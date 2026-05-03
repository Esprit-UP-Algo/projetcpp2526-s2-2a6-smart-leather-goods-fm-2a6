from __future__ import annotations

import asyncio
import hashlib
import logging
import time
from typing import Any, AsyncIterator, Awaitable, Callable, Dict, Optional

import numpy as np

try:
    import cv2  # type: ignore
except Exception:  # pragma: no cover
    cv2 = None

from app.config import settings
from app.models.schemas import FashionPrediction
from app.services.concept_visual_prompts import (
    batch_slots_slice,
    build_capsule_visual_prompt,
    capsule_cache_key,
)
from app.services.forecast_phases import (
    FORECAST_MIN_YEAR,
    forecast_phase_for_year,
    phase_editorial_mood,
)
from app.services.generative import (
    _is_probably_image_bytes,
    capsule_provider_image_url_fast,
    fetch_image_base64_with_fallback_urls,
    generate_visual_url_with_diagnostics,
    pollinations_client_fallback_url,
    ultra_short_pollinations_prompt,
)
from app.services.generated_image_store import (
    decode_base64_to_bytes,
    download_bytes,
    local_file_path,
    public_image_url,
    try_read_cached_disk,
    write_png_atomic,
)


class ImageGenerationService:
    """Service dédié à la génération d'images (batch + stream + cache + persistance locale)."""

    def __init__(self) -> None:
        self._logger = logging.getLogger("fashion_oracle.image_generation")
        self._memory_cache: Dict[str, str] = {}
        self._cache_lock = asyncio.Lock()

    @staticmethod
    def _product_type_fr(product_type_en: str) -> str:
        pt = (product_type_en or "").strip().lower()
        if pt == "belt":
            return "ceinture"
        return "veste"

    @staticmethod
    def _quick_image_sanity(data: Optional[bytes]) -> bool:
        if not data or not _is_probably_image_bytes(data):
            return False
        return len(data) >= 1200

    @staticmethod
    def _has_visible_face(image_bytes: Optional[bytes]) -> bool:
        """Reject images with visible human faces in PRODUCT-ONLY mode."""
        if cv2 is None or not image_bytes:
            return False
        try:
            arr = np.frombuffer(image_bytes, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
            if img is None:
                return False
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            frontal = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
            profile = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_profileface.xml")
            if frontal.empty():
                return False
            faces_f = frontal.detectMultiScale(
                gray,
                scaleFactor=1.08,
                minNeighbors=4,
                minSize=(26, 26),
            )
            if len(faces_f) > 0:
                return True
            if not profile.empty():
                faces_p = profile.detectMultiScale(
                    gray,
                    scaleFactor=1.08,
                    minNeighbors=4,
                    minSize=(26, 26),
                )
                if len(faces_p) > 0:
                    return True
            return False
        except Exception:
            return False

    @staticmethod
    def _has_person_silhouette(image_bytes: Optional[bytes]) -> bool:
        """Detect obvious human silhouettes (full/upper body) to enforce product-only mode."""
        if cv2 is None or not image_bytes:
            return False
        try:
            arr = np.frombuffer(image_bytes, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
            if img is None:
                return False
            h, w = img.shape[:2]
            if h < 80 or w < 80:
                return False
            hog = cv2.HOGDescriptor()
            hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
            # Resize for stable detector behavior and speed.
            target_w = 640
            scale = target_w / float(w)
            rs = cv2.resize(img, (target_w, int(h * scale)))
            rects, _ = hog.detectMultiScale(
                rs,
                winStride=(8, 8),
                padding=(8, 8),
                scale=1.05,
            )
            return len(rects) > 0
        except Exception:
            return False

    @staticmethod
    def _product_only_fallback_png(
        product_type_en: str,
        *,
        style: str,
        palette: str,
        material: str,
    ) -> Optional[bytes]:
        """Always-safe fallback image (no human, no face, no body)."""
        if cv2 is None:
            return None
        try:
            h, w = 768, 768
            img = np.full((h, w, 3), 245, dtype=np.uint8)
            # Soft neutral gradient.
            for y in range(h):
                v = 245 - int(18 * (y / float(h)))
                img[y, :, :] = (v, v, v)

            # Header text
            cv2.putText(img, "FASHION ORACLE - PRODUCT ONLY", (36, 54), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (60, 60, 60), 2, cv2.LINE_AA)
            cv2.putText(img, f"Style: {style[:42]}", (36, 96), cv2.FONT_HERSHEY_SIMPLEX, 0.58, (90, 90, 90), 1, cv2.LINE_AA)
            cv2.putText(img, f"Palette: {palette[:42]}", (36, 122), cv2.FONT_HERSHEY_SIMPLEX, 0.58, (90, 90, 90), 1, cv2.LINE_AA)
            cv2.putText(img, f"Matiere: {material[:42]}", (36, 148), cv2.FONT_HERSHEY_SIMPLEX, 0.58, (90, 90, 90), 1, cv2.LINE_AA)

            if product_type_en == "belt":
                # Belt product fallback.
                cv2.ellipse(img, (410, 430), (210, 95), -10, 0, 360, (45, 45, 55), 26, cv2.LINE_AA)
                cv2.rectangle(img, (495, 360), (620, 455), (200, 180, 110), -1)
                cv2.rectangle(img, (512, 377), (592, 438), (70, 70, 80), 6)
                cv2.putText(img, "CEINTURE LUXE - PRODUCT FALLBACK", (180, 660), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (55, 55, 55), 2, cv2.LINE_AA)
            else:
                # Jacket product fallback (ghost-mannequin style, no human parts).
                base = (58, 60, 68)
                cv2.rectangle(img, (250, 220), (520, 590), base, -1)
                cv2.rectangle(img, (185, 260), (255, 530), base, -1)  # left sleeve
                cv2.rectangle(img, (520, 260), (590, 530), base, -1)  # right sleeve
                cv2.rectangle(img, (315, 175), (455, 240), (72, 75, 84), -1)  # collar
                cv2.rectangle(img, (315, 310), (382, 380), (46, 48, 54), -1)  # pocket
                cv2.rectangle(img, (390, 310), (457, 380), (46, 48, 54), -1)  # pocket
                cv2.line(img, (385, 220), (385, 590), (36, 36, 42), 4)  # zipper seam
                cv2.putText(img, "VESTE - PRODUCT FALLBACK", (214, 660), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (55, 55, 55), 2, cv2.LINE_AA)

            ok, enc = cv2.imencode(".png", img)
            if not ok or enc is None:
                return None
            return enc.tobytes()
        except Exception:
            return None

    @staticmethod
    def _safe_retry_prompt(prompt: str, product_type_en: str) -> str:
        p = (prompt or "").strip()
        if product_type_en == "belt":
            return (
                f"{p} SAFE_RETRY: premium belt product photography, centered accessory close-up, "
                "studio clean background, no person, no face, no body."
            )
        if product_type_en == "jacket":
            return (
                f"{p} SAFE_RETRY_PRODUCT_ONLY: premium product photography, isolated jacket only, "
                "floating garment or hanger only, full jacket visible, front or 3/4 angle, "
                "clean studio background, sharp material detail. "
                "NEGATIVE: human, face, model, body, mannequin, portrait, person, skin, hands."
            )
        return (
            f"{p} SAFE_RETRY_PRODUCT_ONLY: premium product photography, isolated object, "
            "clean studio background, no human, no face, no model, no body, no mannequin."
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

    async def _build_slot(
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
            retry_count: int = 0,
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
                "retry_count": retry_count,
            }

        disk_bytes = try_read_cached_disk(cache_key)
        if self._quick_image_sanity(disk_bytes):
            if product_type_en in ("jacket", "belt") and (
                ImageGenerationService._has_visible_face(disk_bytes)
                or ImageGenerationService._has_person_silhouette(disk_bytes)
            ):
                self._logger.warning("image cache INVALIDATE disk human ci=%s key=%s", concept_index, cache_key[:80])
                try:
                    pth = local_file_path(cache_key)
                    if pth.exists():
                        pth.unlink()
                except OSError:
                    pass
            else:
                local_url = public_image_url(cache_key, base_url=public_base_url)
                async with self._cache_lock:
                    self._memory_cache[cache_key] = local_url
                self._logger.info("image cache HIT disk ci=%s key=%s", concept_index, cache_key[:100])
                return _item(status="ok", image_url=local_url, cache_hit=True)

        async with self._cache_lock:
            mem_url = self._memory_cache.get(cache_key)
        if mem_url:
            self._logger.info("image cache HIT memory ci=%s key=%s", concept_index, cache_key[:100])
            return _item(status="ok", image_url=mem_url, cache_hit=True)

        attempts = [prompt, self._safe_retry_prompt(prompt, product_type_en)]
        provider_error: Optional[str] = None
        image_bytes: Optional[bytes] = None
        retry_count = 0
        face_detected = False
        for i, attempt_prompt in enumerate(attempts):
            # Veste: prioriser qualite en tentative 1, fallback rapide en tentative 2.
            if product_type_en == "jacket":
                use_fast_provider = fast_mode and i > 0
            else:
                use_fast_provider = fast_mode
            provider_t0 = time.perf_counter()
            if use_fast_provider:
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
            provider_ms = int((time.perf_counter() - provider_t0) * 1000)

            self._logger.info(
                "image provider year=%s ci=%s variant=%s has_url=%s retry=%s provider_ms=%s err=%s url=%s",
                year,
                concept_index,
                variant,
                bool(provider_url and provider_url.startswith("http")),
                i,
                provider_ms,
                (provider_error or "")[:180],
                (provider_url or "")[:140],
            )
            retry_count = i
            image_bytes = None
            if provider_url:
                image_bytes = await download_bytes(
                    provider_url,
                    timeout_s=24.0 if use_fast_provider else 110.0,
                    max_attempts=2 if use_fast_provider else 5,
                )
                if not self._quick_image_sanity(image_bytes):
                    b64 = await fetch_image_base64_with_fallback_urls(
                        provider_url,
                        attempt_prompt,
                        url_prompt=url_prompt,
                        quick=use_fast_provider,
                    )
                    image_bytes = decode_base64_to_bytes(b64) if b64 else None
            if not self._quick_image_sanity(image_bytes):
                fb = pollinations_client_fallback_url(attempt_prompt, url_prompt=url_prompt)
                image_bytes = await download_bytes(
                    fb,
                    timeout_s=20.0 if use_fast_provider else 115.0,
                    max_attempts=2 if use_fast_provider else 4,
                )
            if product_type_en in ("jacket", "belt") and self._quick_image_sanity(image_bytes):
                has_face = self._has_visible_face(image_bytes)
                has_person = self._has_person_silhouette(image_bytes)
                if has_face or has_person:
                    face_detected = True
                    self._logger.warning(
                        "image REJECT_HUMAN year=%s ci=%s variant=%s retry=%s has_face=%s has_person=%s",
                        year,
                        concept_index,
                        variant,
                        i,
                        has_face,
                        has_person,
                    )
                    image_bytes = None
                    provider_error = "human_detected_product_only_mode"
            if self._quick_image_sanity(image_bytes):
                break
            image_bytes = None

        if not self._quick_image_sanity(image_bytes):
            reason = "backend_download_error" if provider_url else "provider_error"
            if face_detected:
                reason = "human_detected_error"
            self._logger.warning(
                "image FAIL year=%s ci=%s variant=%s retry=%s status=%s err=%s",
                year,
                concept_index,
                variant,
                retry_count,
                reason,
                provider_error,
            )
            fb_png = self._product_only_fallback_png(
                product_type_en,
                style=style,
                palette=palette,
                material=material,
            )
            if self._quick_image_sanity(fb_png):
                ok_write, wmsg = write_png_atomic(cache_key, fb_png or b"")
                if ok_write:
                    local_url = public_image_url(cache_key, base_url=public_base_url)
                    async with self._cache_lock:
                        self._memory_cache[cache_key] = local_url
                    return _item(
                        status="ok_fallback_product_only",
                        image_url=local_url,
                        error=reason,
                        provider_error=provider_error,
                        retry_count=retry_count,
                    )
                self._logger.warning("fallback WRITE_FAIL ci=%s msg=%s", concept_index, wmsg)

            return _item(
                status=reason,
                error=reason,
                provider_error=provider_error,
                retry_count=retry_count,
            )

        ok_write, wmsg = write_png_atomic(cache_key, image_bytes)
        if not ok_write:
            return _item(
                status="local_save_error",
                error=f"local_save_error:{wmsg}",
                provider_error=provider_error,
                retry_count=retry_count,
            )
        saved = try_read_cached_disk(cache_key)
        if not self._quick_image_sanity(saved):
            return _item(
                status="local_save_error",
                error="local_file_invalid_after_write",
                provider_error=provider_error,
                retry_count=retry_count,
            )
        if product_type_en in ("jacket", "belt") and (
            self._has_visible_face(saved) or self._has_person_silhouette(saved)
        ):
            self._logger.warning("image REJECT_AFTER_SAVE human ci=%s -> product fallback", concept_index)
            try:
                pth = local_file_path(cache_key)
                if pth.exists():
                    pth.unlink()
            except OSError:
                pass
            fb_png = self._product_only_fallback_png(
                product_type_en, style=style, palette=palette, material=material
            )
            if self._quick_image_sanity(fb_png):
                ok_fb, wmsg_fb = write_png_atomic(cache_key, fb_png or b"")
                if ok_fb:
                    saved2 = try_read_cached_disk(cache_key)
                    if self._quick_image_sanity(saved2):
                        local_url = public_image_url(cache_key, base_url=public_base_url)
                        async with self._cache_lock:
                            self._memory_cache[cache_key] = local_url
                        return _item(
                            status="ok_fallback_product_only",
                            image_url=local_url,
                            error="human_detected_replaced",
                            provider_error=provider_error,
                            retry_count=retry_count,
                        )
            return _item(
                status="human_detected_error",
                error="human_detected_after_save",
                provider_error=provider_error,
                retry_count=retry_count,
            )

        local_url = public_image_url(cache_key, base_url=public_base_url)
        async with self._cache_lock:
            self._memory_cache[cache_key] = local_url

        self._logger.info(
            "image OK year=%s ci=%s variant=%s cache=%s local_url=%s bytes=%s retry=%s",
            year,
            concept_index,
            variant,
            cache_key[:90],
            local_url[:160],
            len(saved or b""),
            retry_count,
        )
        return _item(
            status="ok",
            image_url=local_url,
            provider_error=provider_error,
            retry_count=retry_count,
        )

    async def _run_slot_timed(
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
            await asyncio.sleep(0.05 * stagger_idx)
        ci = int(slot["concept_index"])
        t0 = time.perf_counter()
        try:
            out = await asyncio.wait_for(
                self._build_slot(
                    year=year,
                    pred=pred,
                    slot=slot,
                    public_base_url=public_base_url,
                    fast_mode=fast_mode,
                ),
                timeout=slot_timeout,
            )
        except asyncio.TimeoutError:
            out = {
                "year": year,
                "concept_index": ci,
                "product_type": self._product_type_fr(str(slot["product_type"])),
                "variant": str(slot["variant"]),
                "category_label": str(slot["category_label"]),
                "style": "",
                "palette": "",
                "material": "",
                "silhouette": "",
                "forecast_phase": phase,
                "status": "backend_download_error",
                "image_url": None,
                "error": f"backend_download_error:timeout_{int(slot_timeout)}s",
                "cache_key": None,
                "cache_hit": False,
                "prompt": "",
                "provider_error": "timeout",
                "retry_count": 0,
            }
        out["perf_ms"] = int((time.perf_counter() - t0) * 1000)
        return out

    async def generate_batch(
        self,
        *,
        year: int,
        public_base_url: str,
        count: int,
        fast_mode: bool,
        predict_fn: Callable[[int], Awaitable[FashionPrediction]],
    ) -> list[Dict[str, Any]]:
        phase = forecast_phase_for_year(year)
        slots = batch_slots_slice(min(count, int(getattr(settings, "batch_max_count", 12) or 12)))
        if year < FORECAST_MIN_YEAR:
            notice = "La génération de visuels IA est disponible uniquement pour les prévisions à partir de 2026."
            return [
                {
                    "year": year,
                    "concept_index": int(slot["concept_index"]),
                    "product_type": self._product_type_fr(str(slot["product_type"])),
                    "variant": str(slot["variant"]),
                    "category_label": str(slot["category_label"]),
                    "style": "",
                    "palette": "",
                    "material": "",
                    "silhouette": "",
                    "forecast_phase": phase,
                    "status": "blocked",
                    "image_url": None,
                    "error": notice,
                    "cache_key": None,
                    "cache_hit": False,
                    "prompt": "",
                    "provider_error": "year_before_forecast_min",
                    "retry_count": 0,
                    "perf_ms": 0,
                }
                for slot in slots
            ]

        t0 = time.perf_counter()
        pred = await predict_fn(year)
        slot_timeout = (
            float(getattr(settings, "fast_batch_slot_timeout_s", 90.0) or 90.0)
            if fast_mode
            else float(getattr(settings, "capsule_slot_timeout_s", 120.0) or 120.0)
        )
        # Anti-mauvaise config .env: en mode fast, borne dure pour la validation demo.
        if fast_mode:
            slot_timeout = max(10.0, min(30.0, slot_timeout))
        else:
            slot_timeout = max(12.0, min(360.0, slot_timeout))
        max_parallel = max(1, min(6, int(getattr(settings, "batch_max_parallel", 3) or 3)))
        sem = asyncio.Semaphore(max_parallel)
        self._logger.info(
            "batch start year=%s count=%s fast=%s timeout=%.1f parallel=%s",
            year,
            len(slots),
            fast_mode,
            slot_timeout,
            max_parallel,
        )

        async def _run(slot: Dict[str, Any], i: int) -> Dict[str, Any]:
            async with sem:
                return await self._run_slot_timed(
                    year=year,
                    phase=phase,
                    pred=pred,
                    slot=slot,
                    public_base_url=public_base_url,
                    stagger_idx=i,
                    slot_timeout=slot_timeout,
                    fast_mode=fast_mode,
                )

        out = await asyncio.gather(*[_run(s, i) for i, s in enumerate(slots)])
        self._logger.info(
            "batch done year=%s count=%s total_ms=%s ok=%s err=%s",
            year,
            len(out),
            int((time.perf_counter() - t0) * 1000),
            sum(1 for it in out if it.get("status") == "ok"),
            sum(1 for it in out if it.get("status") != "ok"),
        )
        return out

    async def stream_batch(
        self,
        *,
        year: int,
        public_base_url: str,
        count: int,
        fast_mode: bool,
        predict_fn: Callable[[int], Awaitable[FashionPrediction]],
    ) -> AsyncIterator[Dict[str, Any]]:
        t0 = time.perf_counter()
        phase = forecast_phase_for_year(year)
        slots = batch_slots_slice(min(count, int(getattr(settings, "batch_max_count", 12) or 12)))
        if year < FORECAST_MIN_YEAR:
            notice = "La génération de visuels IA est disponible uniquement pour les prévisions à partir de 2026."
            for slot in slots:
                yield {
                    "year": year,
                    "concept_index": int(slot["concept_index"]),
                    "product_type": self._product_type_fr(str(slot["product_type"])),
                    "variant": str(slot["variant"]),
                    "category_label": str(slot["category_label"]),
                    "style": "",
                    "palette": "",
                    "material": "",
                    "silhouette": "",
                    "forecast_phase": phase,
                    "status": "blocked",
                    "image_url": None,
                    "error": notice,
                    "cache_key": None,
                    "cache_hit": False,
                    "prompt": "",
                    "provider_error": "year_before_forecast_min",
                    "retry_count": 0,
                    "perf_ms": 0,
                    "stream_batch_ms": int((time.perf_counter() - t0) * 1000),
                }
            return

        pred = await predict_fn(year)
        slot_timeout = (
            float(getattr(settings, "fast_batch_slot_timeout_s", 90.0) or 90.0)
            if fast_mode
            else float(getattr(settings, "capsule_slot_timeout_s", 120.0) or 120.0)
        )
        if fast_mode:
            slot_timeout = max(10.0, min(30.0, slot_timeout))
        else:
            slot_timeout = max(12.0, min(360.0, slot_timeout))
        max_parallel = max(1, min(6, int(getattr(settings, "batch_max_parallel", 3) or 3)))
        sem = asyncio.Semaphore(max_parallel)

        async def _run(slot: Dict[str, Any], i: int) -> Dict[str, Any]:
            async with sem:
                return await self._run_slot_timed(
                    year=year,
                    phase=phase,
                    pred=pred,
                    slot=slot,
                    public_base_url=public_base_url,
                    stagger_idx=i,
                    slot_timeout=slot_timeout,
                    fast_mode=fast_mode,
                )

        tasks = [asyncio.create_task(_run(s, i)) for i, s in enumerate(slots)]
        try:
            for fut in asyncio.as_completed(tasks):
                item = await fut
                item["stream_batch_ms"] = int((time.perf_counter() - t0) * 1000)
                yield item
        finally:
            for t in tasks:
                if not t.done():
                    t.cancel()

