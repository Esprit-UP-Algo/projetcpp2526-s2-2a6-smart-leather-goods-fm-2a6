"""Profil de generation visuelle (leger : pas d import de generative / replicate)."""

from __future__ import annotations

from app.image_provider import get_image_provider_resolution

DEMO_GENERATION_MODE = "demo_cached_ai_v1"
DEMO_LOCAL_YEAR_MODE = "year_demo_v1"
GENERATION_VERSION = "product_only_v7"


def get_visual_response_generation_mode() -> str:
    provider = get_image_provider_resolution().provider
    if provider == "demo":
        return DEMO_GENERATION_MODE
    if provider == "demo_local_year":
        return DEMO_LOCAL_YEAR_MODE
    return GENERATION_VERSION
