"""
Helpers legacy : Pollinations, prompts capsule, enrichissement LLM (OpenRouter).
Utilise par oracle.py, image_generation_service.py, concept_visual_prompts.py.
"""
from __future__ import annotations

import base64
import hashlib
import json
import logging
import random
import re
from typing import Any, Dict, List, Optional
from urllib.parse import quote

import httpx

from app.config import settings

logger = logging.getLogger(__name__)


def _is_probably_image_bytes(data: bytes | None) -> bool:
    if not data or len(data) < 12:
        return False
    if data[:8] == b"\x89PNG\r\n\x1a\n":
        return True
    if data[:3] == b"\xff\xd8\xff":
        return True
    if data[:4] in (b"RIFF",) and b"WEBP" in data[:16]:
        return True
    return False


def build_fashion_look_prompt(
    *,
    year: int,
    style: str,
    palette: str,
    material: str,
    silhouette: str,
    product_type: str,
    forecast_phase: str,
    era_mood: str,
) -> str:
    pt = "jacket" if product_type == "jacket" else "belt" if product_type == "belt" else product_type
    return (
        f"Editorial luxury fashion product hero {year}, phase {forecast_phase}, mood {era_mood}. "
        f"Style {style}, palette {palette}, material {material}, silhouette {silhouette}. "
        f"Product type focus: {pt}. Isolated product, premium studio lighting, no human, no face."
    )


def refine_bag_visual_prompt(prompt: str) -> str:
    p = (prompt or "").strip()
    if not p:
        return p
    return re.sub(r"\bbag\b", "belt accessory", p, flags=re.I)


def generate_visual_prompt(
    year: int,
    top_style: str,
    colors: List[str],
    fabrics: List[str],
    silhouettes: List[str],
) -> str:
    c = ", ".join(colors[:4]) if colors else "neutrals"
    f = ", ".join(fabrics[:3]) if fabrics else "mixed materials"
    s = ", ".join(silhouettes[:3]) if silhouettes else "structured"
    return (
        f"Fashion forecast moodboard {year}, dominant style {top_style}, palette {c}, "
        f"fabrics {f}, silhouettes {s}. Editorial collage, soft lighting, premium magazine."
    )


def ultra_short_pollinations_prompt(
    *,
    product_type: str,
    year: int,
    forecast_phase: str,
    model_fingerprint: str,
) -> str:
    h = hashlib.sha1(model_fingerprint.encode("utf-8")).hexdigest()[:10]
    return f"{product_type}|{year}|{forecast_phase}|{h}"


def _pollinations_url(
    prompt: str,
    *,
    width: int,
    height: int,
    seed: int,
) -> str:
    safe = (prompt or "").strip()[:1800]
    enc = quote(safe, safe="")
    return (
        f"https://image.pollinations.ai/prompt/{enc}"
        f"?width={width}&height={height}&seed={seed}&nologo=true&enhance=false"
    )


def pollinations_client_fallback_url(attempt_prompt: str, *, url_prompt: str | None = None) -> str:
    del url_prompt
    seed = random.randint(1, 2_000_000_000)
    return _pollinations_url(attempt_prompt, width=512, height=512, seed=seed)


async def capsule_provider_image_url_fast(
    attempt_prompt: str,
    *,
    url_prompt: str,
    product_type_for_image: str,
) -> tuple[Optional[str], Optional[str]]:
    del url_prompt, product_type_for_image
    seed = random.randint(1, 2_000_000_000)
    url = _pollinations_url(attempt_prompt, width=384, height=384, seed=seed)
    logger.debug("pollinations_fast url_len=%s", len(url))
    return url, None


async def generate_visual_url_with_diagnostics(
    attempt_prompt: str,
    *,
    url_prompt: str,
    product_type_for_image: str,
) -> tuple[Optional[str], Optional[str]]:
    del url_prompt, product_type_for_image
    seed = random.randint(1, 2_000_000_000)
    url = _pollinations_url(attempt_prompt, width=768, height=768, seed=seed)
    err: Optional[str] = None
    try:
        async with httpx.AsyncClient(timeout=httpx.Timeout(12.0, connect=6.0)) as client:
            r = await client.get(url, follow_redirects=True)
            if r.status_code >= 400:
                err = f"pollinations_head_{r.status_code}"
    except Exception as ex:  # noqa: BLE001
        err = f"pollinations_probe:{type(ex).__name__}"
        logger.info("generate_visual_url probe soft_fail err=%s", err)
    return url, err


async def generate_visual_url(visual_prompt_full: str) -> Optional[str]:
    url, _ = await generate_visual_url_with_diagnostics(
        visual_prompt_full,
        url_prompt="chart",
        product_type_for_image="jacket",
    )
    return url


async def fetch_image_base64_with_fallback_urls(
    provider_url: str,
    attempt_prompt: str,
    *,
    url_prompt: str | None = None,
    quick: bool = False,
) -> Optional[str]:
    del url_prompt
    urls = [provider_url]
    for w, h in ((512, 512), (384, 384)) if quick else ((768, 768), (640, 640), (512, 512)):
        urls.append(
            _pollinations_url(
                attempt_prompt,
                width=w,
                height=h,
                seed=random.randint(1, 2_000_000_000),
            )
        )
    timeout = httpx.Timeout(25.0 if quick else 45.0, connect=10.0)
    async with httpx.AsyncClient(timeout=timeout, follow_redirects=True) as client:
        for u in urls:
            try:
                r = await client.get(u)
                if r.status_code >= 400:
                    continue
                data = r.content
                if _is_probably_image_bytes(data):
                    return base64.b64encode(data).decode("ascii")
            except Exception:  # noqa: BLE001
                continue
    return None


def _strip_json_fence(content: str) -> str:
    c = content.strip()
    if c.startswith("```"):
        c = re.sub(r"^```[a-zA-Z]*\s*", "", c)
        c = re.sub(r"\s*```$", "", c).strip()
    return c


async def llm_enrich_prediction(
    *,
    year: int,
    forecast_phase: str,
    phase_summary: str,
    top_styles: List[str],
    color_palette: List[str],
    fabrics_materials: List[str],
    silhouettes: List[str],
    confidence: float,
    similar_decades: List[str],
) -> Optional[Dict[str, Any]]:
    if not (settings.api_key or "").strip():
        return None
    model = (settings.model or "openai/gpt-4o-mini").strip()
    system = (
        "Tu es un styliste senior mode. Reponds UNIQUEMENT par un JSON valide avec les cles: "
        "summary (string court FR), similar_decades (liste de 3 a 4 strings FR), "
        "confidence (nombre 0..1), recommended_product_attributes (objet cle/valeur string)."
    )
    user = json.dumps(
        {
            "year": year,
            "forecast_phase": forecast_phase,
            "phase_summary": phase_summary,
            "top_styles": top_styles,
            "color_palette": color_palette,
            "fabrics_materials": fabrics_materials,
            "silhouettes": silhouettes,
            "confidence": confidence,
            "similar_decades": similar_decades,
        },
        ensure_ascii=False,
    )
    headers = {
        "Authorization": f"Bearer {settings.api_key}",
        "Content-Type": "application/json",
    }
    body = {
        "model": model,
        "temperature": 0.35,
        "max_tokens": 700,
        "messages": [
            {"role": "system", "content": system},
            {"role": "user", "content": user},
        ],
    }
    url = f"{settings.base_url.rstrip('/')}/chat/completions"
    try:
        async with httpx.AsyncClient(timeout=httpx.Timeout(55.0, connect=12.0)) as client:
            r = await client.post(url, headers=headers, json=body)
            r.raise_for_status()
            data = r.json()
            content = (data.get("choices") or [{}])[0].get("message", {}).get("content") or ""
            raw = _strip_json_fence(content)
            return json.loads(raw)
    except Exception as ex:  # noqa: BLE001
        logger.warning("llm_enrich_prediction_fail err=%s", type(ex).__name__)
        return None
