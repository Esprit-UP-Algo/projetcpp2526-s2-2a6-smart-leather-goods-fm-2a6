"""Contexte texte / OpenRouter (strictement separe de la generation image)."""

from __future__ import annotations

from app.config import settings


def text_provider_name() -> str:
    """Convention : cle FASHION_ORACLE_API_KEY + base OpenRouter => openrouter."""
    if (settings.api_key or "").strip() and "openrouter" in (settings.base_url or "").lower():
        return "openrouter"
    if (settings.api_key or "").strip():
        return "custom_http"
    return "none"


def log_text_router_config() -> None:
    import logging

    log = logging.getLogger("fashion_oracle.text_router")
    log.info(
        "[TEXT_PROVIDER] %s [TEXT_MODEL] %s [TEXT_BASE_URL] %s [TEXT_API_KEY] %s",
        text_provider_name(),
        (settings.model or "").strip() or "(empty)",
        (settings.base_url or "").strip() or "(empty)",
        "present" if (settings.api_key or "").strip() else "missing",
    )


def text_debug_slice() -> dict[str, object]:
    return {
        "text_provider": text_provider_name(),
        "text_model": (settings.model or "").strip(),
        "text_base_url": (settings.base_url or "").strip(),
        "has_text_api_key": bool((settings.api_key or "").strip()),
    }
