"""Persistance locale des visuels IA — URLs stables servies par /static/generated/."""

from __future__ import annotations

import asyncio
import hashlib
import logging
import re
from pathlib import Path
from typing import Optional, Tuple

import httpx

from app.config import settings

logger = logging.getLogger("fashion_oracle.generated_assets")

# Racine fashion_oracle/static/generated
GENERATED_ROOT = Path(__file__).resolve().parents[2] / "static" / "generated"


def _safe_filename_from_cache_key(cache_key: str) -> str:
    h = hashlib.sha256(cache_key.encode("utf-8")).hexdigest()[:32]
    return f"fo_{h}.png"


def local_file_path(cache_key: str) -> Path:
    GENERATED_ROOT.mkdir(parents=True, exist_ok=True)
    return GENERATED_ROOT / _safe_filename_from_cache_key(cache_key)


def public_image_url(cache_key: str, *, base_url: str | None = None) -> str:
    """URL absolue servie par FastAPI StaticFiles (/static/...)."""
    base = (base_url or settings.public_base_url or "http://127.0.0.1:8010").rstrip("/")
    rel = f"/static/generated/{_safe_filename_from_cache_key(cache_key)}"
    return f"{base}{rel}"


def try_read_cached_disk(cache_key: str) -> Optional[bytes]:
    p = local_file_path(cache_key)
    if not p.is_file():
        return None
    try:
        data = p.read_bytes()
        if len(data) < 200:
            return None
        return data
    except OSError:
        return None


def write_png_atomic(cache_key: str, image_bytes: bytes) -> Tuple[bool, str]:
    """Ecrit le PNG sur disque. Retourne (ok, chemin absolu ou message)."""
    if not image_bytes or len(image_bytes) < 200:
        return False, "bytes_trop_courts"
    if image_bytes[:8] == b"<html" or image_bytes[:2] == b"<!" or image_bytes[:1] == b"<":
        return False, "contenu_non_image"
    GENERATED_ROOT.mkdir(parents=True, exist_ok=True)
    dest = local_file_path(cache_key)
    tmp = dest.with_suffix(".tmp.png")
    try:
        tmp.write_bytes(image_bytes)
        tmp.replace(dest)
        logger.info(
            "generated_image SAVED path=%s bytes=%s key_prefix=%s",
            dest,
            len(image_bytes),
            cache_key[:80],
        )
        return True, str(dest.resolve())
    except OSError as ex:
        logger.warning("generated_image WRITE_FAIL %s", ex)
        if tmp.exists():
            try:
                tmp.unlink()
            except OSError:
                pass
        return False, str(ex)


async def download_bytes(
    url: str,
    *,
    timeout_s: float = 100.0,
    max_attempts: int = 5,
) -> Optional[bytes]:
    if not url or not url.startswith("http"):
        return None
    headers = {
        "User-Agent": "Mozilla/5.0 FashionOracleBackend/2.0 (local-persist)",
        "Accept": "image/avif,image/webp,image/apng,image/*,*/*;q=0.8",
    }
    try:
        async with httpx.AsyncClient(
            timeout=httpx.Timeout(timeout_s, connect=18.0),
            follow_redirects=True,
            http2=False,
        ) as client:
            for attempt in range(max(1, max_attempts)):
                r = await client.get(url, headers=headers)
                body = r.content or b""
                if r.status_code >= 400 or len(body) < 200:
                    if attempt < max_attempts - 1:
                        await asyncio.sleep(0.15 + attempt * 0.12)
                    continue
                if body.startswith(b"<") or body.startswith(b"<!"):
                    if attempt < max_attempts - 1:
                        await asyncio.sleep(0.15 + attempt * 0.12)
                    continue
                return body
    except Exception as ex:
        logger.warning("download_bytes FAIL url=%s err=%s", url[:120], type(ex).__name__)
    return None


def decode_base64_to_bytes(b64: str) -> Optional[bytes]:
    import base64

    raw = (b64 or "").strip()
    if "base64," in raw:
        raw = raw.split("base64,", 1)[1].strip()
    raw = re.sub(r"\s+", "", raw)
    if not raw:
        return None
    try:
        out = base64.b64decode(raw, validate=False)
        return out if len(out) > 200 else None
    except Exception:
        return None
