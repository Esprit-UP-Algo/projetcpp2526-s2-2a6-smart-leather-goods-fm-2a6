from __future__ import annotations
import asyncio
import hashlib
import logging
import os
import time
from typing import Any, AsyncIterator, Dict, List, Optional
import numpy as np
from sqlalchemy import select

from app.data.simulator import build_simulated_corpus
from app.db.repository import SessionLocal, PredictionCache
from app.models.schemas import FashionPrediction
from app.services.temporal_model import forecast_styles
from app.services.trend_engine import build_style_features, compute_trend_snapshot
from app.services.recommender import recommend_product_attributes
from app.services.visualization import trend_chart, cycle_chart
from app.services.generative import (
    build_fashion_look_prompt,
    capsule_provider_image_url_fast,
    fetch_image_base64_with_fallback_urls,
    generate_visual_prompt,
    generate_visual_url,
    generate_visual_url_with_diagnostics,
    llm_enrich_prediction,
    pollinations_client_fallback_url,
    refine_bag_visual_prompt,
    ultra_short_pollinations_prompt,
    _is_probably_image_bytes,
)
from app.services.concept_visual_prompts import (
    CAPSULE_SLOTS,
    batch_slots_slice,
    build_capsule_visual_prompt,
    capsule_cache_key,
)
from app.services.generated_image_store import (
    try_read_cached_disk,
    write_png_atomic,
    public_image_url,
    decode_base64_to_bytes,
    download_bytes,
)
from app.config import settings
from app.services.forecast_phases import (
    FORECAST_MIN_YEAR,
    forecast_phase_for_year,
    phase_editorial_mood,
    phase_label_fr,
    phase_style_multipliers,
    stable_style_id,
    year_score_jitter,
)
from app.services.image_generation_service import ImageGenerationService


class FashionOracleService:
    def __init__(self) -> None:
        self.corpus = build_simulated_corpus()
        self._visual_cache: Dict[str, Dict[str, str]] = {}
        self._logger = logging.getLogger("fashion_oracle.visuals")
        # 3 requetes /generate-visuals en parallele : une seule execution predict+LLM par annee.
        self._predict_locks: Dict[int, asyncio.Lock] = {}
        self._predict_tasks: Dict[int, asyncio.Task[Any]] = {}
        self.image_generation = ImageGenerationService()
        self._prewarm_tasks: Dict[str, asyncio.Task[Any]] = {}

    @staticmethod
    def _enrich_future_concepts(
        year: int,
        styles: List[str],
        palettes: List[str],
        materials: List[str],
        silhouettes: List[str],
    ) -> tuple[List[str], List[str], List[str], List[str]]:
        """Quality upgrade layer: renforce la direction produit pour les annees futures."""
        if year < 2029:
            return styles, palettes, materials, silhouettes

        style_map = {
            0: "neo-artisan couture outerwear",
            1: "luxury utility-tech tailoring",
            2: "architectural accessories minimalism",
        }
        palette_map = {
            0: "obsidian noir, graphite, sand",
            1: "oxblood, steel-blue, charcoal",
            2: "deep taupe, brushed gold, black pearl",
        }
        material_map = {
            0: "full-grain leather + bonded wool lining",
            1: "technical nylon + structured gabardine + matte hardware",
            2: "vegetable-tanned leather + brushed metal buckle",
        }
        silhouette_map = {
            0: "structured 3/4 tailored outerwear silhouette",
            1: "modular utility outerwear silhouette",
            2: "clean sculpted accessory silhouette",
        }

        out_s = list(styles)
        out_p = list(palettes)
        out_m = list(materials)
        out_si = list(silhouettes)
        for i in range(min(3, len(out_s))):
            out_s[i] = style_map.get(i, out_s[i])
            if i < len(out_p):
                out_p[i] = palette_map.get(i, out_p[i])
            if i < len(out_m):
                out_m[i] = material_map.get(i, out_m[i])
            if i < len(out_si):
                out_si[i] = silhouette_map.get(i, out_si[i])
        return out_s, out_p, out_m, out_si

    @staticmethod
    def _quick_image_sanity(data: Optional[bytes]) -> bool:
        """Validation anti-régression: non-vide, signature image, taille minimale raisonnable."""
        if not data or not _is_probably_image_bytes(data):
            return False
        if len(data) < 1200:
            return False
        return True

    @staticmethod
    def _safe_retry_prompt(prompt: str, product_type_en: str) -> str:
        p = (prompt or "").strip()
        if product_type_en == "belt":
            return (
                f"{p} SAFE_RETRY: premium belt product photography, centered accessory close-up, "
                "studio clean background, no person, no face, no body."
            )
        return (
            f"{p} SAFE_RETRY: studio fashion portrait, upper-body only, clean pose, realistic face, "
            "symmetrical facial features, natural anatomy, no broken hands, no duplicate limbs, "
            "no warped facial geometry."
        )

    @staticmethod
    def _slot_attrs(pred: FashionPrediction, idx: int) -> tuple[str, str, str, str]:
        styles = pred.top_styles or ["style"]
        palettes = pred.color_palette or ["palette"]
        materials = pred.fabrics_materials or ["material"]
        silhouettes = pred.silhouettes or ["silhouette"]
        row = max(0, min(len(styles) - 1, idx % max(1, len(styles))))
        return (
            styles[row],
            palettes[row % len(palettes)],
            materials[row % len(materials)],
            silhouettes[row % len(silhouettes)],
        )

    def _blend_scores(self, temporal: Dict[str, float], structural: Dict[str, float]) -> Dict[str, float]:
        all_styles = sorted(set(temporal.keys()) | set(structural.keys()))
        return {
            s: float(0.68 * temporal.get(s, 0.0) + 0.32 * structural.get(s, 0.0))
            for s in all_styles
        }

    def _normalize(self, scores: Dict[str, float]) -> Dict[str, float]:
        vals = np.array(list(scores.values()), dtype=float)
        lo, hi = float(vals.min()), float(vals.max())
        if hi - lo < 1e-9:
            return {k: 0.5 for k in scores}
        return {k: float((v - lo) / (hi - lo)) for k, v in scores.items()}

    def _rank_fields_for(self, year: int, styles: List[str]) -> tuple[List[str], List[str], List[str]]:
        """Attributs palette / matière / silhouette par rang, calibrés année (taxonomie corpus uniquement)."""
        pal: List[str] = []
        fab: List[str] = []
        sil: List[str] = []
        for i, st in enumerate(styles):
            tax_i = self.corpus.style_taxonomy[st]
            cols = list(tax_i["colors"])
            fabs = list(tax_i["fabrics"])
            sils = list(tax_i["silhouettes"])
            if cols:
                r = (year * 3 + i * 7 + ord(st[0])) % len(cols)
                cols = cols[r:] + cols[:r]
            if fabs:
                r = (year * 5 + i * 11) % len(fabs)
                fabs = fabs[r:] + fabs[:r]
            if sils:
                r = (year + i * 13) % len(sils)
                sils = sils[r:] + sils[:r]
            pal.append(cols[0] if cols else "")
            fab.append(fabs[0] if fabs else "")
            sil.append(sils[0] if sils else "")
        return pal, fab, sil

    async def predict(self, year: int) -> FashionPrediction:
        phase = forecast_phase_for_year(year)

        if year < FORECAST_MIN_YEAR:
            notice = (
                "La prédiction avancée Fashion Oracle est disponible à partir de 2026. "
                "Sélectionnez une année cible 2026 ou ultérieure pour activer la prévision stratégique cockpit."
            )
            return FashionPrediction(
                year=year,
                top_styles=[],
                color_palette=[],
                fabrics_materials=[],
                silhouettes=[],
                confidence=0.0,
                similar_decades=[],
                summary=notice,
                raw_scores={},
                recommended_product_attributes={
                    "perimetre": "Prévision cockpit à partir de 2026 uniquement",
                    "action": "Choisir une année cible ≥ 2026",
                },
                chart_trend_base64=None,
                chart_cycle_base64=None,
                generated_visual_prompt=None,
                generated_visual_url=None,
                inference_mode="historical-non-forecast",
                forecast_eligible=False,
                forecast_phase=phase,
                business_notice=notice,
            )

        with SessionLocal() as db:
            row = db.execute(select(PredictionCache).where(PredictionCache.year == year)).scalar_one_or_none()
            if row:
                try:
                    cached = FashionPrediction.model_validate_json(row.payload)
                    known_styles = set(self.corpus.style_taxonomy.keys())
                    styles_ok = cached.top_styles and all(s in known_styles for s in cached.top_styles)
                    if (
                        cached.forecast_eligible
                        and cached.forecast_phase
                        and cached.forecast_phase != "historical_non_forecast"
                        and styles_ok
                    ):
                        return cached
                except Exception:
                    pass

        lock = self._predict_locks.setdefault(year, asyncio.Lock())
        async with lock:
            with SessionLocal() as db:
                row = db.execute(select(PredictionCache).where(PredictionCache.year == year)).scalar_one_or_none()
                if row:
                    try:
                        cached = FashionPrediction.model_validate_json(row.payload)
                        known_styles = set(self.corpus.style_taxonomy.keys())
                        styles_ok = cached.top_styles and all(s in known_styles for s in cached.top_styles)
                        if (
                            cached.forecast_eligible
                            and cached.forecast_phase
                            and cached.forecast_phase != "historical_non_forecast"
                            and styles_ok
                        ):
                            return cached
                    except Exception:
                        pass
            if year not in self._predict_tasks:
                self._predict_tasks[year] = asyncio.create_task(self._predict_compute_once(year))
            _predict_task = self._predict_tasks[year]

        try:
            return await _predict_task
        finally:
            async with self._predict_locks.setdefault(year, asyncio.Lock()):
                if self._predict_tasks.get(year) is _predict_task and _predict_task.done():
                    self._predict_tasks.pop(year, None)

    async def _predict_compute_once(self, year: int) -> FashionPrediction:
        phase = forecast_phase_for_year(year)
        timeline = self.corpus.timeline
        temporal = forecast_styles(timeline, year)
        feat_df = build_style_features(timeline, year)
        snapshot = compute_trend_snapshot(feat_df)

        blended = self._blend_scores(temporal.scores, snapshot.style_scores)
        normalized = self._normalize(blended)
        mult = phase_style_multipliers(phase)
        horizon = max(0, year - FORECAST_MIN_YEAR)
        modulated = {
            s: float(normalized[s])
            * float(mult.get(s, 1.0))
            * float(year_score_jitter(year, s))
            * (
                1.0
                + 0.09 * float(np.sin((horizon + (stable_style_id(s) % 17)) / 3.4))
            )
            for s in normalized
        }
        mod_norm = self._normalize(modulated)
        ranked = sorted(mod_norm.items(), key=lambda kv: kv[1], reverse=True)

        top_styles = [k for k, _ in ranked[:3]]
        top_style = top_styles[0]

        out_palette, out_fabrics, out_sil = self._rank_fields_for(year, top_styles)

        tax = self.corpus.style_taxonomy[top_style]

        years = list(range(max(1990, year - 14), year + 1))
        style_year_series = {
            s: [float(timeline[(timeline["style"] == s) & (timeline["year"] == y)]["score"].mean()) for y in years]
            for s in top_styles
        }
        cycle_values = [float(np.sin(2 * np.pi * y / 20.0) + 0.4 * np.sin(2 * np.pi * y / 8.0)) for y in years]

        chart_a = trend_chart(style_year_series, years)
        chart_b = cycle_chart(years, cycle_values)

        phase_boost = {"phase_1": 0.0, "phase_2": 0.02, "phase_3": 0.045, "phase_4": 0.07}
        out_conf = round(
            float(
                min(
                    0.95,
                    max(
                        0.52,
                        float(temporal.confidence)
                        + float(phase_boost.get(phase, 0.0))
                        + 0.012 * float(np.sin(year / 3.0)),
                    ),
                )
            ),
            3,
        )

        phase_human = phase_label_fr(phase)
        summary = (
            f"Horizon {year} — {phase_human}. "
            f"Triptyque dominant : « {top_styles[0]} » (pilier), « {top_styles[1]} » (structurant), "
            f"« {top_styles[2]} » (variation créative). "
            f"Dynamique {('haussière' if temporal.confidence > 0.55 else 'à surveiller')} ; "
            f"confiance de scénario {out_conf * 100:.0f} %."
        )

        base_vis_prompt = generate_visual_prompt(year, top_style, tax["colors"], tax["fabrics"], tax["silhouettes"])
        visual_prompt_full = (
            f"{base_vis_prompt} Unique scenario year {year}, {phase}, editorial: {phase_editorial_mood(phase)}."
        )
        visual_url = await generate_visual_url(visual_prompt_full)

        llm_enriched = await llm_enrich_prediction(
            year=year,
            forecast_phase=phase,
            phase_summary=phase_human,
            top_styles=top_styles,
            color_palette=out_palette,
            fabrics_materials=out_fabrics,
            silhouettes=out_sil,
            confidence=out_conf,
            similar_decades=temporal.similar_decades,
        )

        out_styles = top_styles
        out_palette_f: List[str] = list(out_palette)
        out_fabrics_f: List[str] = list(out_fabrics)
        out_sil_f: List[str] = list(out_sil)
        out_summary = summary
        out_conf_f = out_conf
        out_decades = [f"{d} (calibré {year})" for d in (temporal.similar_decades or [])][:4]
        if not out_decades:
            out_decades = [f"Projection {phase_human.split('·')[0].strip()} — année {year}"]
        inference_mode = "hybrid-timeseries+phase"

        if llm_enriched:
            # Ne pas écraser styles / attributs : le moteur année+phase garantit la crédibilité temporelle.
            out_summary = llm_enriched.get("summary", out_summary) or out_summary
            out_decades = llm_enriched.get("similar_decades", out_decades) or out_decades
            try:
                out_conf_f = float(llm_enriched.get("confidence", out_conf_f))
            except Exception:
                pass
            inference_mode = "hybrid-timeseries+phase+llm"

        known = set(self.corpus.style_taxonomy.keys())
        cleaned_styles: List[str] = []
        for s in out_styles:
            if s in known and s not in cleaned_styles:
                cleaned_styles.append(s)
        for s in top_styles:
            if len(cleaned_styles) >= 3:
                break
            if s in known and s not in cleaned_styles:
                cleaned_styles.append(s)
        for s in sorted(known):
            if len(cleaned_styles) >= 3:
                break
            if s not in cleaned_styles:
                cleaned_styles.append(s)
        out_styles = cleaned_styles[:3]
        out_palette_f, out_fabrics_f, out_sil_f = self._rank_fields_for(year, out_styles)
        out_styles, out_palette_f, out_fabrics_f, out_sil_f = self._enrich_future_concepts(
            year, out_styles, out_palette_f, out_fabrics_f, out_sil_f
        )
        top_style = out_styles[0]
        base_attrs = recommend_product_attributes(top_style, out_palette_f, out_fabrics_f, out_sil_f)
        if llm_enriched:
            attrs = llm_enriched.get("recommended_product_attributes")
            if isinstance(attrs, dict) and attrs:
                base_attrs = {**base_attrs, **{str(k): str(v) for k, v in attrs.items()}}
        out_attrs = base_attrs

        strategic_line = (
            f" Positionnement produit {year}: capsule premium orientee desirabilite matiere, "
            "silhouette lisible et execution editoriale produit-centree."
        )
        pred = FashionPrediction(
            year=year,
            top_styles=out_styles,
            color_palette=out_palette_f,
            fabrics_materials=out_fabrics_f,
            silhouettes=out_sil_f,
            confidence=round(float(out_conf_f), 3),
            similar_decades=out_decades,
            summary=(out_summary + strategic_line),
            raw_scores={k: round(v, 5) for k, v in mod_norm.items()},
            recommended_product_attributes=out_attrs,
            chart_trend_base64=chart_a,
            chart_cycle_base64=chart_b,
            generated_visual_prompt=visual_prompt_full,
            generated_visual_url=visual_url,
            inference_mode=inference_mode,
            forecast_eligible=True,
            forecast_phase=phase,
            business_notice=None,
        )

        with SessionLocal() as db:
            row = db.execute(select(PredictionCache).where(PredictionCache.year == year)).scalar_one_or_none()
            payload_json = pred.model_dump_json()
            if row:
                row.payload = payload_json
                row.confidence = pred.confidence
                row.top_style = top_style
            else:
                db.add(
                    PredictionCache(
                        year=year,
                        payload=payload_json,
                        confidence=pred.confidence,
                        top_style=top_style,
                    )
                )
            db.commit()

        return pred

    async def generate_visuals_for_year(
        self,
        year: int,
        limit: int = 3,
        *,
        concept_index: int | None = None,
        product_type: str | None = None,
        image_prompt_override: Optional[str] = None,
        client_style: Optional[str] = None,
        client_palette: Optional[str] = None,
        client_material: Optional[str] = None,
    ) -> List[Dict[str, Any]]:
        # Route legacy /generate-visuals alignée sur le pipeline robuste capsule:
        # provider -> backend download -> local disk -> URL locale stable.
        req_count = max(1, min(int(limit), 3))
        if concept_index is not None:
            req_count = 1
        raw = await self.generate_visual_capsule(
            year,
            public_base_url=(settings.public_base_url or "http://127.0.0.1:8010").rstrip("/"),
            count=req_count,
            fast_mode=True,
        )
        mapped: List[Dict[str, Any]] = []
        for it in raw:
            ci = int(it.get("concept_index") or 1)
            if concept_index is not None and ci != int(concept_index):
                continue
            mapped.append(
                {
                    "year": int(it.get("year") or year),
                    "rank": ci,
                    "forecast_phase": str(it.get("forecast_phase") or ""),
                    "product_type": str(it.get("product_type") or ""),
                    "category_label": str(it.get("category_label") or f"Concept Produit {ci}"),
                    "style": str(it.get("style") or ""),
                    "palette": str(it.get("palette") or ""),
                    "material": str(it.get("material") or ""),
                    "silhouette": str(it.get("silhouette") or ""),
                    "prompt": str(it.get("prompt") or ""),
                    "image_url": it.get("image_url"),
                    "image_base64": None,
                    "status": str(it.get("status") or "error"),
                    "error": it.get("error"),
                    "cache_key": it.get("cache_key"),
                    "cache_hit": bool(it.get("cache_hit")),
                    "provider_error": it.get("provider_error"),
                }
            )
        return mapped[: max(1, min(3, req_count))]

    @staticmethod
    def _product_type_fr(product_type_en: str) -> str:
        pt = (product_type_en or "").strip().lower()
        if pt == "belt":
            return "ceinture"
        return "veste"

    async def _build_capsule_slot(
        self,
        *,
        year: int,
        pred: FashionPrediction,
        slot: Dict[str, Any],
        public_base_url: str,
        fast_mode: bool,
    ) -> Dict[str, Any]:
        phase = pred.forecast_phase or forecast_phase_for_year(year)
        era = phase_editorial_mood(phase)
        concept_index = int(slot["concept_index"])
        product_type_en = str(slot["product_type"])
        variant = str(slot["variant"])
        category_label = str(slot["category_label"])
        style, palette, material, silhouette = self._slot_attrs(pred, concept_index - 1)

        prompt = build_capsule_visual_prompt(
            year=year,
            forecast_phase=phase,
            era_mood=era,
            concept_index=concept_index,
            product_type=product_type_en,
            variant=variant,
            style=style,
            palette=palette,
            material=material,
            silhouette=silhouette,
            image_prompt_override=None,
        )
        fp = hashlib.sha256(prompt.encode("utf-8")).hexdigest()[:16]
        cache_key = capsule_cache_key(
            year=year,
            forecast_phase=phase,
            concept_index=concept_index,
            product_type=product_type_en,
            variant=variant,
            style=style,
            palette=palette,
            material=material,
            silhouette=silhouette,
            prompt_fp=fp,
        )
        url_prompt = ultra_short_pollinations_prompt(
            product_type=product_type_en,
            year=year,
            forecast_phase=phase,
            model_fingerprint=f"{fp}|{variant}|{style}|{palette}|ci{concept_index}",
        )

        def _item(
            *,
            status: str,
            image_url: Optional[str] = None,
            error: Optional[str] = None,
            cache_hit: bool = False,
            provider_error: Optional[str] = None,
        ) -> Dict[str, Any]:
            return {
                "year": year,
                "concept_index": concept_index,
                "product_type": self._product_type_fr(product_type_en),
                "variant": variant,
                "category_label": category_label,
                "style": style,
                "palette": palette,
                "material": material,
                "silhouette": silhouette,
                "forecast_phase": phase,
                "status": status,
                "image_url": image_url,
                "error": error,
                "cache_key": cache_key,
                "cache_hit": cache_hit,
                "prompt": prompt,
                "provider_error": provider_error,
            }

        self._logger.info(
            "capsule_slot START year=%s ci=%s type=%s variant=%s cache_key=%s",
            year,
            concept_index,
            product_type_en,
            variant,
            cache_key[:120],
        )

        disk_bytes = try_read_cached_disk(cache_key)
        if self._quick_image_sanity(disk_bytes):
            local_url = public_image_url(cache_key, base_url=public_base_url)
            self._logger.info(
                "capsule_slot DISK_HIT year=%s ci=%s bytes=%s local_url=%s",
                year,
                concept_index,
                len(disk_bytes),
                local_url[:120],
            )
            return _item(status="ok", image_url=local_url, cache_hit=True)

        provider_url: Optional[str] = None
        provider_error: Optional[str] = None
        fast = bool(getattr(settings, "capsule_fast_providers", True)) or fast_mode
        attempts = [prompt, self._safe_retry_prompt(prompt, product_type_en)]
        image_bytes: Optional[bytes] = None
        final_prompt = prompt
        retry_count = 0
        last_err: Optional[str] = None
        for attempt_idx, attempt_prompt in enumerate(attempts):
            final_prompt = attempt_prompt
            if fast:
                provider_url, provider_error = await capsule_provider_image_url_fast(
                    attempt_prompt,
                    url_prompt=url_prompt,
                    product_type_for_image=product_type_en,
                )
            else:
                provider_url, provider_error = await generate_visual_url_with_diagnostics(
                    attempt_prompt,
                    url_prompt=url_prompt,
                    product_type_for_image=product_type_en,
                )
            self._logger.info(
                "capsule_slot PROVIDER year=%s ci=%s fast=%s has_url=%s retry=%s err=%s",
                year,
                concept_index,
                fast,
                bool(provider_url and provider_url.startswith("http")),
                attempt_idx,
                (provider_error or "")[:200],
            )

            max_fetch = 3
            dl_timeout = 85.0 if fast_mode else (100.0 if fast else 110.0)
            image_bytes = None
            for fetch_attempt in range(max_fetch):
                if provider_url:
                    image_bytes = await download_bytes(
                        provider_url,
                        timeout_s=dl_timeout,
                        max_attempts=4 if fast_mode else (5 if fast else 4),
                    )
                    if not image_bytes:
                        b64 = await fetch_image_base64_with_fallback_urls(
                            provider_url,
                            attempt_prompt,
                            url_prompt=url_prompt,
                            quick=fast_mode or fast,
                        )
                        image_bytes = decode_base64_to_bytes(b64) if b64 else None
                if not image_bytes:
                    fb = pollinations_client_fallback_url(attempt_prompt, url_prompt=url_prompt)
                    image_bytes = await download_bytes(
                        fb,
                        timeout_s=90.0 if fast_mode else 115.0,
                        max_attempts=4 if fast_mode else 5,
                    )
                if self._quick_image_sanity(image_bytes):
                    break
                image_bytes = None
                if fetch_attempt < max_fetch - 1:
                    await asyncio.sleep(0.12 + fetch_attempt * 0.18)

            if self._quick_image_sanity(image_bytes):
                retry_count = attempt_idx
                break
            last_err = provider_error or "invalid_or_empty_image"

        if not self._quick_image_sanity(image_bytes):
            self._logger.warning(
                "capsule_slot FAIL year=%s ci=%s provider_err=%s retries=%s",
                year,
                concept_index,
                last_err,
                len(attempts) - 1,
            )
            return _item(
                status="error",
                error="echec_generation_ou_image_invalide",
                cache_hit=False,
                provider_error=last_err,
            )

        ok_write, wmsg = write_png_atomic(cache_key, image_bytes)
        if not ok_write:
            self._logger.warning("capsule_slot WRITE_FAIL year=%s ci=%s msg=%s", year, concept_index, wmsg)
            return _item(
                status="error",
                error=f"persist_image_echec:{wmsg}",
                cache_hit=False,
                provider_error=provider_error,
            )

        local_url = public_image_url(cache_key, base_url=public_base_url)
        self._logger.info(
            "capsule_slot OK year=%s ci=%s saved local_url=%s prompt_len=%s retry_count=%s",
            year,
            concept_index,
            local_url[:160],
            len(final_prompt),
            retry_count,
        )
        return _item(status="ok", image_url=local_url, cache_hit=False, provider_error=provider_error)

    def _capsule_timeout_item(
        self,
        *,
        year: int,
        phase: str,
        slot: Dict[str, Any],
        slot_timeout_s: float,
    ) -> Dict[str, Any]:
        ci = int(slot["concept_index"])
        pt_en = str(slot["product_type"])
        return {
            "year": year,
            "concept_index": ci,
            "product_type": self._product_type_fr(pt_en),
            "variant": str(slot["variant"]),
            "category_label": str(slot["category_label"]),
            "style": "",
            "palette": "",
            "material": "",
            "silhouette": "",
            "forecast_phase": phase,
            "status": "timeout",
            "image_url": None,
            "error": f"timeout_{int(slot_timeout_s)}s",
            "cache_key": None,
            "cache_hit": False,
            "prompt": "",
            "provider_error": "timeout",
            "perf_ms": int(slot_timeout_s * 1000),
        }

    async def _run_capsule_slot_timed(
        self,
        *,
        year: int,
        phase: str,
        pred: FashionPrediction,
        slot: Dict[str, Any],
        public_base_url: str,
        stagger_idx: int,
        slot_timeout: float,
        fast_mode: bool,
    ) -> Dict[str, Any]:
        if stagger_idx:
            await asyncio.sleep(0.08 * stagger_idx)
        ci = int(slot["concept_index"])
        t0 = time.perf_counter()
        try:
            out = await asyncio.wait_for(
                self._build_capsule_slot(
                    year=year,
                    pred=pred,
                    slot=slot,
                    public_base_url=public_base_url,
                    fast_mode=fast_mode,
                ),
                timeout=slot_timeout,
            )
        except asyncio.TimeoutError:
            self._logger.warning(
                "capsule_slot TIMEOUT year=%s ci=%s after_s=%.1f",
                year,
                ci,
                slot_timeout,
            )
            out = self._capsule_timeout_item(
                year=year,
                phase=pred.forecast_phase or phase,
                slot=slot,
                slot_timeout_s=slot_timeout,
            )
        out["perf_ms"] = int((time.perf_counter() - t0) * 1000)
        self._logger.info(
            "capsule_slot PERF year=%s ci=%s ms=%s status=%s cache_hit=%s",
            year,
            ci,
            out["perf_ms"],
            out.get("status"),
            out.get("cache_hit"),
        )
        return out

    async def generate_visual_capsule(
        self,
        year: int,
        *,
        public_base_url: str,
        count: int = 3,
        fast_mode: bool = True,
    ) -> List[Dict[str, Any]]:
        return await self.image_generation.generate_batch(
            year=year,
            public_base_url=public_base_url,
            count=count,
            fast_mode=fast_mode,
            predict_fn=self.predict,
        )

    async def iter_capsule_visual_slots_stream(
        self,
        year: int,
        *,
        public_base_url: str,
        count: int = 3,
        fast_mode: bool = True,
    ) -> AsyncIterator[Dict[str, Any]]:
        async for item in self.image_generation.stream_batch(
            year=year,
            public_base_url=public_base_url,
            count=count,
            fast_mode=fast_mode,
            predict_fn=self.predict,
        ):
            yield item

    def schedule_capsule_prewarm(self, year: int, *, public_base_url: str) -> None:
        """Lance la capsule en tache de fond apres predict (cache disque -> clic Generer quasi instantane)."""
        if year < FORECAST_MIN_YEAR:
            return
        prewarm_count = 3
        key = f"{year}|fast|{prewarm_count}"
        old = self._prewarm_tasks.get(key)
        if old and not old.done():
            return

        async def _job() -> None:
            try:
                t0 = time.perf_counter()
                await self.generate_visual_capsule(
                    year,
                    public_base_url=public_base_url,
                    count=prewarm_count,
                    fast_mode=True,
                )
                self._logger.info(
                    "capsule_PREWARM_DONE year=%s ms=%s",
                    year,
                    int((time.perf_counter() - t0) * 1000),
                )
            except Exception as ex:
                self._logger.warning("capsule_PREWARM_FAIL year=%s err=%s", year, type(ex).__name__)
            finally:
                self._prewarm_tasks.pop(key, None)

        try:
            self._prewarm_tasks[key] = asyncio.create_task(_job())
        except RuntimeError:
            self._logger.warning("capsule_PREWARM_SKIP year=%s no_running_loop", year)


fashion_oracle_service = FashionOracleService()
