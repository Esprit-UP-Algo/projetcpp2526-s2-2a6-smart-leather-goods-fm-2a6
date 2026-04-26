"""
Selection stricte du fournisseur IMAGE pour /generate-visuals.

Variable : FASHION_ORACLE_IMAGE_PROVIDER (defaut: replicate)
  - replicate : uniquement Replicate (FASHION_ORACLE_REPLICATE_API_TOKEN), jamais OPENAI_API_KEY
  - huggingface : uniquement HuggingFace (HF_API_KEY)
  - demo      : visuels statiques locaux (aucune API), presentation / validation UI
  - demo_local_year : generation locale par annee (Pillow), deterministe

Toute autre valeur -> invalid_image_provider
"""

from __future__ import annotations

import logging
import os
from dataclasses import dataclass
from typing import Literal

log = logging.getLogger("fashion_oracle.image_provider")

OPENAI_PLATFORM_BASE_URL = "https://api.openai.com/v1"

ImageProviderName = Literal["huggingface", "replicate", "demo", "demo_local_year", "invalid"]


def _strip(name: str, default: str = "") -> str:
    return (os.getenv(name) or default).strip()


def mask_secret(value: str, head: int = 4) -> str:
    if not value:
        return "absent"
    v = value.strip()
    if len(v) <= head:
        return f"present ({len(v)} chars)"
    return f"present prefix={v[:head]}… len={len(v)}"


@dataclass(frozen=True)
class ImageProviderResolution:
    """provider=invalid => consulter failure (aucune generation)."""

    provider: ImageProviderName
    openai_api_key: str = ""
    hf_api_key: str = ""
    replicate_api_token: str = ""
    failure: str = ""

    @property
    def is_configured(self) -> bool:
        return self.provider in ("huggingface", "replicate", "demo", "demo_local_year")


_resolved: ImageProviderResolution | None = None


def _resolve_image_provider() -> ImageProviderResolution:
    # Compat: accepte aussi IMAGE_PROVIDER=demo pour un basculement rapide.
    prov = (os.getenv("FASHION_ORACLE_IMAGE_PROVIDER") or os.getenv("IMAGE_PROVIDER") or "").strip()
    if not prov:
        log.error("missing_image_provider: definir FASHION_ORACLE_IMAGE_PROVIDER (ex. replicate)")
        return ImageProviderResolution(provider="invalid", failure="missing_image_provider")
    raw = prov.lower()
    if raw == "demo":
        return ImageProviderResolution(provider="demo")
    if raw == "demo_local_year":
        return ImageProviderResolution(provider="demo_local_year")

    if raw not in ("replicate", "huggingface"):
        log.error("invalid_image_provider valeur=%r (replicate|huggingface|demo|demo_local_year)", raw)
        return ImageProviderResolution(provider="invalid", failure="invalid_image_provider")

    if raw == "replicate":
        tok = _strip("FASHION_ORACLE_REPLICATE_API_TOKEN") or _strip("REPLICATE_API_TOKEN")
        if not tok:
            log.error("missing_replicate_api_token: definir FASHION_ORACLE_REPLICATE_API_TOKEN (r8_...)")
            return ImageProviderResolution(provider="invalid", failure="missing_replicate_api_token")
        img_model = _strip("FASHION_ORACLE_IMAGE_MODEL")
        if not img_model:
            log.error("missing_image_model_config: definir FASHION_ORACLE_IMAGE_MODEL (ex. stability-ai/sdxl)")
            return ImageProviderResolution(provider="invalid", failure="missing_image_model_config")
        return ImageProviderResolution(provider="replicate", replicate_api_token=tok)

    key = _strip("HF_API_KEY")
    if not key:
        log.error("missing_hf_api_key: definir HF_API_KEY pour FASHION_ORACLE_IMAGE_PROVIDER=huggingface")
        return ImageProviderResolution(provider="invalid", failure="missing_hf_api_key")
    return ImageProviderResolution(provider="huggingface", hf_api_key=key)


def get_image_provider_resolution() -> ImageProviderResolution:
    global _resolved
    if _resolved is None:
        _resolved = _resolve_image_provider()
    return _resolved


def reset_image_provider_cache_for_tests() -> None:
    global _resolved
    _resolved = None


def log_image_provider_at_startup() -> None:
    r = get_image_provider_resolution()
    env_raw = (os.getenv("FASHION_ORACLE_IMAGE_PROVIDER") or "").strip()
    env_img = _strip("FASHION_ORACLE_IMAGE_MODEL")
    print("IMAGE_PROVIDER =", env_raw or "(empty)", flush=True)
    print("IMAGE_MODEL =", env_img or "(empty)", flush=True)
    log.info(
        "[IMAGE_PROVIDER] %s [IMAGE_MODEL] %s [REPLICATE_TOKEN] %s",
        r.provider,
        env_img or "(empty)",
        mask_secret(r.replicate_api_token),
    )
    if r.provider == "invalid":
        log.warning("IMAGE_PROVIDER_INVALID failure=%s", r.failure[:200])
