"""Proxy HTTP images (Pollinations, Replicate, etc.) pour le client Qt."""

from __future__ import annotations

import asyncio
import logging
from urllib.parse import urlparse

import httpx
from fastapi import HTTPException, Query
from fastapi.responses import Response
from pydantic import BaseModel, Field

from app.services.generative import _is_probably_image_bytes

logger = logging.getLogger(__name__)


class ProxyImageJsonBody(BaseModel):
    """POST /proxy-image — evite les URLs GET trop longues (Pollinations)."""

    src: str = Field(..., min_length=12, max_length=32768)


def _proxy_src_host_allowed(hostname: str) -> bool:
    """Limite le proxy (anti-SSRF) aux CDN utilisés pour les visuels Fashion Oracle."""
    h = (hostname or "").lower().strip(".")
    if not h:
        return False
    if h in ("image.pollinations.ai", "pollinations.ai"):
        return True
    if h.endswith(".pollinations.ai"):
        return True
    if "replicate.delivery" in h:
        return True
    if "blob.core.windows.net" in h and ("openai" in h or "oai" in h):
        return True
    if "openaipublic" in h:
        return True
    return False


def _validate_src(raw: str) -> str:
    raw = (raw or "").strip()
    try:
        pu = urlparse(raw)
    except Exception:
        raise HTTPException(status_code=400, detail="URL invalide")
    if pu.scheme not in ("http", "https"):
        raise HTTPException(status_code=400, detail="Scheme non autorise")
    host = (pu.hostname or "").lower()
    if not _proxy_src_host_allowed(host):
        raise HTTPException(status_code=403, detail="Hote non autorise pour le proxy")
    return raw


async def _proxy_fetch_response(raw: str) -> Response:
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) FashionOracleBackend/1.2",
        "Accept": "image/avif,image/webp,image/apng,image/*,*/*;q=0.8",
        "Accept-Language": "fr-FR,fr;q=0.9,en;q=0.8",
    }
    try:
        timeout = httpx.Timeout(75.0, connect=22.0)
        async with httpx.AsyncClient(timeout=timeout, follow_redirects=True, http2=False) as client:
            for attempt in range(5):
                resp = await client.get(raw, headers=headers)
                body = resp.content or b""
                if resp.status_code >= 400 or len(body) < 200:
                    await asyncio.sleep(0.25 + attempt * 0.18)
                    continue
                if body.startswith(b"<") or body.startswith(b"<!"):
                    await asyncio.sleep(0.25 + attempt * 0.18)
                    continue
                ct = (resp.headers.get("Content-Type") or "").lower()
                if "image" in ct or _is_probably_image_bytes(body):
                    mt = (ct.split(";")[0].strip() or "image/png") if ct else "image/png"
                    return Response(content=body, media_type=mt)
                await asyncio.sleep(0.25 + attempt * 0.18)
    except Exception:
        pass
    raise HTTPException(status_code=502, detail="Image non recuperable via proxy")


async def proxy_image(
    src: str = Query(..., min_length=12, max_length=16384),
) -> Response:
    """GET : pratique pour tests navigateur ; les longues URLs preferer POST."""
    raw = _validate_src(src)
    return await _proxy_fetch_response(raw)


async def proxy_image_post(body: ProxyImageJsonBody) -> Response:
    """POST JSON {\"src\": \"https://...\"} — recommande pour le client Qt (URLs Pollinations longues)."""
    raw = _validate_src(body.src)
    return await _proxy_fetch_response(raw)
