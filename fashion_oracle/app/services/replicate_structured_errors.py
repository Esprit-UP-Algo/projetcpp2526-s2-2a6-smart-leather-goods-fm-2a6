"""
Erreurs Replicate structurees (codes stables + detail) pour API / UI.
"""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
ERR_REPLICATE_RATE_LIMITED = "replicate_rate_limited"
ERR_REPLICATE_TIMEOUT = "replicate_timeout"
ERR_REPLICATE_INSUFFICIENT_CREDIT = "replicate_insufficient_credit"
ERR_REPLICATE_INVALID_MODEL = "replicate_invalid_model"
ERR_REPLICATE_INVALID_VERSION = "replicate_invalid_version"
ERR_REPLICATE_BAD_PAYLOAD = "replicate_bad_payload"
ERR_REPLICATE_NETWORK = "replicate_network_error"
ERR_REPLICATE_BOTH_FAILED = "replicate_both_models_failed"


@dataclass(frozen=True)
class ReplicateRunMeta:
    used_fallback: bool = False
    primary_model: str = ""
    fallback_model: str = ""


class StructuredReplicateImageError(Exception):
    """Exception avec metadonnees pour GeneratedVisualItem."""

    def __init__(
        self,
        *,
        code: str,
        detail: str,
        primary_model: str = "",
        fallback_model: str = "",
        used_fallback: bool = False,
        primary_error: str = "",
        fallback_error: str = "",
    ) -> None:
        super().__init__(detail)
        self.code = code
        self.detail = detail
        self.primary_model = primary_model
        self.fallback_model = fallback_model
        self.used_fallback = used_fallback
        self.primary_error = primary_error
        self.fallback_error = fallback_error


def _http_status_from_exc(exc: BaseException) -> int | None:
    cur: BaseException | None = exc
    seen: set[int] = set()
    while cur is not None and id(cur) not in seen:
        seen.add(id(cur))
        st = getattr(cur, "status", None)
        if st is None:
            st = getattr(cur, "status_code", None)
        if isinstance(st, int) and st > 0:
            return st
        resp = getattr(cur, "response", None)
        if resp is not None:
            sc = getattr(resp, "status_code", None)
            if isinstance(sc, int) and sc > 0:
                return sc
        nxt = cur.__cause__
        cur = nxt if isinstance(nxt, BaseException) else None
    return None


def _extract_payload(exc: BaseException) -> str:
    try:
        from replicate.exceptions import ModelError, ReplicateError

        if isinstance(exc, ModelError):
            pred = getattr(exc, "prediction", None)
            err = getattr(pred, "error", None) if pred is not None else None
            if err is not None:
                return json.dumps({"prediction_error": str(err)}, ensure_ascii=False)
        if isinstance(exc, ReplicateError):
            return json.dumps(exc.to_dict(), ensure_ascii=False)
    except ImportError:
        pass
    cur: BaseException | None = exc
    seen: set[int] = set()
    while cur is not None and id(cur) not in seen:
        seen.add(id(cur))
        resp = getattr(cur, "response", None)
        if resp is not None and hasattr(resp, "text"):
            try:
                t = getattr(resp, "text", None)
                if t:
                    return str(t)[:4000]
            except Exception:
                pass
        nxt = cur.__cause__
        cur = nxt if isinstance(nxt, BaseException) else None
    return str(exc)


def _is_429(exc: BaseException) -> bool:
    if isinstance(exc, TimeoutError):
        return False
    msg = str(exc).lower()
    if "429" in str(exc) or "throttl" in msg or "rate limit" in msg or "too many requests" in msg:
        return True
    st = getattr(exc, "status", None)
    if st is None:
        st = getattr(exc, "status_code", None)
    return st == 429


def _is_402(exc: BaseException) -> bool:
    if _http_status_from_exc(exc) == 402:
        return True
    return "insufficient credit" in str(exc).lower()


def _detail_for_status(st: int | None, payload: str, exc: BaseException) -> str:
    if st == 402:
        return "Replicate returned 402 insufficient credit; billing required at replicate.com/account/billing"
    if st == 429:
        return "Replicate returned 429 rate limit; wait or add payment method per account policy"
    if st == 404:
        return "Replicate returned 404; model or version not found"
    if st == 422:
        return "Replicate returned 422; input validation failed"
    if isinstance(exc, TimeoutError):
        return "Image generation timed out"
    one = " ".join(str(payload).split())[:500]
    return one or type(exc).__name__


def classify_replicate_api_exception(
    exc: BaseException,
    *,
    primary_model: str = "",
    fallback_model: str = "",
    used_fallback: bool = False,
    fallback_error: str = "",
) -> StructuredReplicateImageError:
    """Mappe une exception Replicate/httpx vers un code stable + detail."""
    if isinstance(exc, StructuredReplicateImageError):
        return exc

    payload = _extract_payload(exc)
    st = _http_status_from_exc(exc)
    detail = _detail_for_status(st, payload, exc)
    perr = " ".join(str(payload).split())[:1200]

    if isinstance(exc, TimeoutError):
        return StructuredReplicateImageError(
            code=ERR_REPLICATE_TIMEOUT,
            detail=detail,
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )
    if _is_429(exc):
        return StructuredReplicateImageError(
            code=ERR_REPLICATE_RATE_LIMITED,
            detail=detail,
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )
    if _is_402(exc):
        return StructuredReplicateImageError(
            code=ERR_REPLICATE_INSUFFICIENT_CREDIT,
            detail=detail,
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )

    try:
        import httpx

        if isinstance(exc, (httpx.ConnectError, httpx.ConnectTimeout, httpx.ReadTimeout, httpx.WriteTimeout)):
            return StructuredReplicateImageError(
                code=ERR_REPLICATE_NETWORK,
                detail=f"Network error ({type(exc).__name__})",
                primary_model=primary_model,
                fallback_model=fallback_model,
                used_fallback=used_fallback,
                primary_error=perr,
                fallback_error=fallback_error,
            )
        if isinstance(exc, httpx.HTTPStatusError):
            sc = exc.response.status_code if exc.response is not None else None
            if sc == 429:
                return StructuredReplicateImageError(
                    code=ERR_REPLICATE_RATE_LIMITED,
                    detail=_detail_for_status(429, payload, exc),
                    primary_model=primary_model,
                    fallback_model=fallback_model,
                    used_fallback=used_fallback,
                    primary_error=perr,
                    fallback_error=fallback_error,
                )
    except ImportError:
        pass

    low = (payload + str(exc)).lower()
    if st == 404 or "not found" in low or "unknown model" in low:
        code = ERR_REPLICATE_INVALID_VERSION if re.search(
            r"version|versions/", low
        ) else ERR_REPLICATE_INVALID_MODEL
        return StructuredReplicateImageError(
            code=code,
            detail=detail,
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )
    if st == 422 or "invalid input" in low or "validation" in low or "unexpected keyword" in low:
        return StructuredReplicateImageError(
            code=ERR_REPLICATE_BAD_PAYLOAD,
            detail=detail,
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )

    if "connection" in low or "network" in low or "ssl" in low or "timed out" in low:
        return StructuredReplicateImageError(
            code=ERR_REPLICATE_NETWORK,
            detail="Network or transport failure",
            primary_model=primary_model,
            fallback_model=fallback_model,
            used_fallback=used_fallback,
            primary_error=perr,
            fallback_error=fallback_error,
        )

    return StructuredReplicateImageError(
        code=ERR_REPLICATE_BAD_PAYLOAD if st in (400, None) and ("json" in low or "input" in low) else ERR_REPLICATE_NETWORK,
        detail=detail if st else f"{type(exc).__name__}: {' '.join(str(exc).split())[:400]}",
        primary_model=primary_model,
        fallback_model=fallback_model,
        used_fallback=used_fallback,
        primary_error=perr,
        fallback_error=fallback_error,
    )


def both_models_failed(
    *,
    primary_model: str,
    fallback_model: str,
    primary_exc: BaseException,
    fallback_exc: BaseException,
) -> StructuredReplicateImageError:
    pe = _extract_payload(primary_exc)
    fe = _extract_payload(fallback_exc)
    detail = (
        f"SDXL failed ({type(primary_exc).__name__}); Flux failed ({type(fallback_exc).__name__}). "
        f"See primary_model_error and fallback_model_error."
    )
    return StructuredReplicateImageError(
        code=ERR_REPLICATE_BOTH_FAILED,
        detail=detail,
        primary_model=primary_model,
        fallback_model=fallback_model,
        used_fallback=True,
        primary_error=" ".join(pe.split())[:1500],
        fallback_error=" ".join(fe.split())[:1500],
    )
