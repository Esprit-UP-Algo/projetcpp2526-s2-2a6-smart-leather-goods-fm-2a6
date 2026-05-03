from __future__ import annotations

import asyncio
import base64
import hashlib
import json
import logging
import os
import random
import requests
import shutil
import traceback
import time
from pathlib import Path
from typing import Any, List

import httpx

from app.config import settings
from app.demo_assets import (
    DEMO_CONCEPT_FILES,
    DEMO_VISUALS_DIR,
    demo_visual_relative_path,
    demo_visual_tier_for_year,
)
from app.services.concept_prep import build_concepts_for_year
from app.services import replicate_structured_errors as rse
try:
    from app.services.local_visual_generator import (
        ERR_MISSING_DEMO_SOURCE_IMAGES,
        generate_local_year_visual_item,
        generate_local_year_visual_items,
    )
    _LOCAL_YEAR_GENERATOR_IMPORT_ERROR: BaseException | None = None
except Exception as _ex:  # noqa: BLE001
    ERR_MISSING_DEMO_SOURCE_IMAGES = "missing_demo_source_images"
    _LOCAL_YEAR_GENERATOR_IMPORT_ERROR = _ex

    def generate_local_year_visual_item(c: ProductConcept, host_base: str) -> GeneratedVisualItem:
        return GeneratedVisualItem(
            year=c.year,
            concept_index=c.concept_index,
            product_type=c.product_type,
            variant=c.variant,
            style=c.style,
            palette=c.palette,
            material=c.material,
            silhouette=c.silhouette,
            status="error",
            image_url=None,
            error_reason=ERR_MISSING_DEMO_SOURCE_IMAGES,
            error_detail=f"local_year_generator_unavailable:{type(_LOCAL_YEAR_GENERATOR_IMPORT_ERROR).__name__}",
            generation_mode=DEMO_GENERATION_MODE,
            prompt_hash="",
            cache_key="",
        )

    def generate_local_year_visual_items(concepts: list[ProductConcept], host_base: str) -> list[GeneratedVisualItem]:
        return [generate_local_year_visual_item(c, host_base) for c in concepts]
from app.services.replicate_adapters import (
    build_replicate_input_for_ref,
    extract_positive_prompt_for_run,
    json_for_log,
    redact_input_for_log,
)
from app.generation_profile import (
    DEMO_GENERATION_MODE,
    GENERATION_VERSION,
    get_visual_response_generation_mode,
)
from app.image_provider import get_image_provider_resolution
from app.models.schemas import ProductConcept, GeneratedVisualItem

logger = logging.getLogger(__name__)
logger.info(
    "GENERATIVE_MODULE_PATH=%s GENERATION_VERSION=%s",
    Path(__file__).resolve(),
    GENERATION_VERSION,
)
logger.info(
    "FO_GENERATIVE_BOOT file=%s pid=%s generation_version=%s FASHION_ORACLE_IMAGE_PROVIDER=%r "
    "FASHION_ORACLE_IMAGE_MODEL=%r",
    str(Path(__file__).resolve()),
    os.getpid(),
    GENERATION_VERSION,
    os.getenv("FASHION_ORACLE_IMAGE_PROVIDER", ""),
    os.getenv("FASHION_ORACLE_IMAGE_MODEL", ""),
)

# --- Demo / perf ---------------------------------------------------------------------------
# Desactive par defaut : la detection OpenCV sur chaque cache hit est tres lente.
STRICT_CACHE_HUMAN_CHECK = os.getenv("FASHION_ORACLE_STRICT_CACHE_HUMAN_CHECK", "0").strip().lower() in (
    "1",
    "true",
    "yes",
    "on",
)

FAST_IMAGE_DEMO = os.getenv("FASHION_ORACLE_IMAGE_FAST_DEMO", "1").strip().lower() in (
    "1",
    "true",
    "yes",
    "on",
)
# OpenAI / cache profile (Replicate resolution vient du modele ; steps + prompt changent).
DEMO_IMAGE_SIZE = os.getenv("FASHION_ORACLE_IMAGE_SIZE", "512x512")
DEMO_IMAGE_SIZE_FAST = os.getenv("FASHION_ORACLE_IMAGE_SIZE_FAST", "512x512")
HTTPX_TIMEOUT = httpx.Timeout(120.0, connect=30.0)

# Concurrence image (Replicate + OpenAI).
generation_semaphore = asyncio.Semaphore(2)

# Budget temps total par concept (tout le flux Replicate hors cache, plusieurs essais / 429).
PER_CONCEPT_TOTAL_TIMEOUT_S = float(os.getenv("FASHION_ORACLE_PER_CONCEPT_TIMEOUT_S", "150"))
# Timeout d un seul appel run + download Replicate (aligne demande perf : 40s par defaut).
REPLICATE_SINGLE_CALL_TIMEOUT_S = float(os.getenv("FASHION_ORACLE_REPLICATE_GEN_TIMEOUT_S", "40"))
REPLICATE_429_BACKOFF_S = (2.0, 4.0, 8.0)

# Espacement proactif entre deux creations Replicate (429 vs latence : defaut 2.0s).
MIN_REPLICATE_INTERVAL_S = float(os.getenv("FASHION_ORACLE_REPLICATE_MIN_INTERVAL_S", "2.0"))
_replicate_spacing_lock = asyncio.Lock()
_last_replicate_request_mono: float = 0.0

# Pre-generation apres /predict (tache de fond).
PREWARM_AFTER_PREDICT = os.getenv("FASHION_ORACLE_PREWARM_AFTER_PREDICT", "0").strip().lower() in (
    "1",
    "true",
    "yes",
    "on",
)
_prewarm_tasks: dict[int, asyncio.Task[None]] = {}

_nuclear_reset_done = False

try:
    import cv2  # type: ignore
    import numpy as np  # type: ignore
except Exception:  # pragma: no cover
    cv2 = None
    np = None

# Erreurs normalisees (contrat UI / debug). Codes Replicate : module rse.
ERR_REPLICATE_RATE_LIMITED = rse.ERR_REPLICATE_RATE_LIMITED
ERR_REPLICATE_TIMEOUT = rse.ERR_REPLICATE_TIMEOUT
ERR_REPLICATE_INSUFFICIENT_CREDIT = rse.ERR_REPLICATE_INSUFFICIENT_CREDIT
ERR_REPLICATE_BOTH_FAILED = rse.ERR_REPLICATE_BOTH_FAILED
ERR_REPLICATE_BAD_PAYLOAD = rse.ERR_REPLICATE_BAD_PAYLOAD
ERR_REPLICATE_NETWORK = rse.ERR_REPLICATE_NETWORK
ERR_REPLICATE_NO_OUTPUT = "replicate_no_output"
ERR_CACHE_INVALID = "cache_invalid"
ERR_LOCAL_SAVE_FAILED = "local_save_failed"
ERR_INVALID_PROVIDER = "invalid_image_provider"
ERR_BATCH_INTERNAL = "batch_internal_error"
ERR_IMAGE_PROVIDER = "image_provider_error"
ERR_OPENAI_IMAGE_TIMEOUT = "openai_image_timeout"
ERR_MISSING_IMAGE_MODEL = "missing_image_model_config"
ERR_MISSING_REPLICATE_TOKEN = "missing_replicate_api_token"
ERR_REPLICATE_PROVIDER = "replicate_provider_error"
ERR_MISSING_DEMO_LOCAL_SOURCES = ERR_MISSING_DEMO_SOURCE_IMAGES

# Erreurs Replicate : inutile de retenter un autre prompt avec le meme compte / config.
_REPLICATE_NON_RETRYABLE_CODES = frozenset(
    {
        ERR_REPLICATE_INSUFFICIENT_CREDIT,
        ERR_MISSING_IMAGE_MODEL,
        "invalid_model_or_provider",
        ERR_REPLICATE_BOTH_FAILED,
        rse.ERR_REPLICATE_INVALID_MODEL,
        rse.ERR_REPLICATE_INVALID_VERSION,
    }
)

BASE_DIR = Path(__file__).resolve().parents[2]
GENERATED_DIR = BASE_DIR / "generated_visuals"
GENERATED_DIR.mkdir(parents=True, exist_ok=True)
LEGACY_STATIC_GENERATED_DIR = BASE_DIR / "static" / "generated"
GENERATED_OPENAI_DIR = BASE_DIR / "static" / "generated_openai"
GENERATED_OPENAI_DIR.mkdir(parents=True, exist_ok=True)
GENERATED_HF_DIR = BASE_DIR / "static" / "generated_hf"
GENERATED_HF_DIR.mkdir(parents=True, exist_ok=True)

# Compat : ancien nom interne.
_generation_semaphore = generation_semaphore

# --- Mode DEMO : visuels IA pre-generes (fichiers locaux), sans Replicate / OpenAI ----------
# Chemins / graines PNG : app.demo_assets (demarrage main sans importer ce module lourd).


def build_demo_visual_item_for_concept(c: ProductConcept, host_base: str) -> GeneratedVisualItem:
    tier = demo_visual_tier_for_year(c.year)
    rel = demo_visual_relative_path(c.concept_index, c.year)
    url = f"{host_base.rstrip('/')}/static/demo_visuals/{rel}"
    salt = f"{time.time_ns()}|{random.randint(1000, 9999)}"
    style_opts = [c.style, f"{c.style}-atelier", f"{c.style}-editorial", f"{c.style}-runway"]
    palette_opts = [c.palette, "sand", "espresso", "charcoal", "taupe", "pearl"]
    material_opts = [c.material, "cuir lisse", "nubuck", "lin premium", "coton technique"]
    silhouette_opts = [c.silhouette, "tailored", "minimal-structured", "soft-volume"]
    style_v = random.choice(style_opts)
    palette_v = random.choice(palette_opts)
    material_v = random.choice(material_opts)
    silhouette_v = random.choice(silhouette_opts)
    ph = prompt_hash(f"demo_cached|{tier}|ci{c.concept_index}|y{c.year}|{salt}")
    ck = f"demo|{tier}|ci{c.concept_index}|{prompt_hash(salt)}"
    return GeneratedVisualItem(
        year=c.year,
        concept_index=c.concept_index,
        product_type=c.product_type,
        variant=c.variant,
        style=style_v,
        palette=palette_v,
        material=material_v,
        silhouette=silhouette_v,
        status="success",
        image_url=url,
        cached=True,
        generation_mode=DEMO_GENERATION_MODE,
        prompt_hash=ph,
        cache_key=ck,
    )


def build_demo_visual_items(year: int, host_base: str) -> List[GeneratedVisualItem]:
    return [build_demo_visual_item_for_concept(c, host_base) for c in build_concepts_for_year(year)]


def build_image_runtime_debug_config() -> dict[str, object]:
    """Expose la config image + texte (diagnostic /debug-config), sans secrets."""
    from app.text_runtime import text_debug_slice

    r = get_image_provider_resolution()
    try:
        resolved_ref = _replicate_model_ref()
    except Exception as e:  # noqa: BLE001
        resolved_ref = f"(unresolved:{type(e).__name__}:{e})"
    out: dict[str, object] = {
        "generation_mode": get_visual_response_generation_mode(),
        "generation_version": GENERATION_VERSION,
        "http_port_hint": os.getenv("FASHION_ORACLE_HTTP_PORT")
        or os.getenv("PORT", "not_set"),
        "image_provider": r.provider,
        "image_model": replicate_image_model_from_env(),
        "replicate_diag_safe_run": os.getenv("FASHION_ORACLE_REPLICATE_DIAG_SAFE_RUN", "")
        .strip()
        .lower()
        in ("1", "true", "yes", "on"),
        "replicate_resolved_ref": resolved_ref,
        "replicate_flux_fallback_ref": (os.getenv("FASHION_ORACLE_REPLICATE_FLUX_FALLBACK_REF") or "").strip()
        or _REPLICATE_FLUX_SCHNELL_REF,
        "timeout": float(REPLICATE_SINGLE_CALL_TIMEOUT_S),
        "per_concept_budget_s": float(PER_CONCEPT_TOTAL_TIMEOUT_S),
        "throttle": float(MIN_REPLICATE_INTERVAL_S),
        "has_replicate_token": bool((r.replicate_api_token or "").strip())
        if r.provider == "replicate"
        else False,
        "image_provider_failure": (r.failure or "") if r.provider == "invalid" else "",
        "generative_file": str(Path(__file__).resolve()),
        "process_id": int(os.getpid()),
    }
    out.update(text_debug_slice())
    return out


# =============================================================================
# Cache key & chemins
# =============================================================================


def _cache_profile_tag(*, fast_demo: bool) -> str:
    steps = max(2, int(settings.replicate_num_inference_steps) - 1) if fast_demo else int(
        settings.replicate_num_inference_steps
    )
    size = DEMO_IMAGE_SIZE_FAST if fast_demo else DEMO_IMAGE_SIZE
    return f"{'fast' if fast_demo else 'std'}|steps={steps}|img={size}"


def concept_cache_key(c: ProductConcept, provider_tag: str, *, fast_demo: bool = False) -> str:
    raw = "|".join(
        [
            GENERATION_VERSION,
            provider_tag,
            _cache_profile_tag(fast_demo=fast_demo),
            str(c.year),
            str(c.concept_index),
            c.product_type,
            c.variant,
            c.style,
            c.palette,
            c.material,
            c.silhouette,
        ]
    )
    return hashlib.sha1(raw.encode("utf-8")).hexdigest()


def prompt_hash(prompt: str) -> str:
    return hashlib.sha1((prompt or "").encode("utf-8")).hexdigest()[:12]


def _local_url_for(host_base: str, filename: str) -> str:
    return f"{host_base}/static/generated/{filename}"


def _local_url_for_openai(host_base: str, filename: str) -> str:
    return f"{host_base}/static/generated_openai/{filename}"


def _local_url_for_hf(host_base: str, filename: str) -> str:
    return f"{host_base}/static/generated_hf/{filename}"


def _png_magic_ok(data: bytes) -> bool:
    return len(data) >= 8 and data[:8] == b"\x89PNG\r\n\x1a\n"


def validate_cached_image_file(
    file_path: Path,
    *,
    host_base: str,
    filename: str,
) -> tuple[bool, str | None]:
    """
    Fichier cache valide : existe, extension .png, taille > 0, en-tete PNG, URL locale coherente.
    Retourne (ok, code_erreur si invalide).
    """
    if file_path.suffix.lower() != ".png":
        return False, ERR_CACHE_INVALID
    if not file_path.exists() or not file_path.is_file():
        return False, ERR_CACHE_INVALID
    if file_path.stat().st_size <= 0:
        return False, ERR_CACHE_INVALID
    try:
        head = file_path.read_bytes()[:32]
    except OSError:
        return False, ERR_CACHE_INVALID
    if not _png_magic_ok(head):
        return False, ERR_CACHE_INVALID
    expected = _local_url_for(host_base, filename)
    if not expected.endswith(filename):
        return False, ERR_CACHE_INVALID
    return True, None


def nuclear_reset_cache() -> None:
    global _nuclear_reset_done
    if _nuclear_reset_done:
        return
    for folder in (GENERATED_DIR, LEGACY_STATIC_GENERATED_DIR):
        if folder.exists():
            for p in folder.glob("*"):
                try:
                    if p.is_file():
                        p.unlink()
                    elif p.is_dir():
                        shutil.rmtree(p, ignore_errors=True)
                except OSError:
                    pass
    GENERATED_DIR.mkdir(parents=True, exist_ok=True)
    _nuclear_reset_done = True
    logger.warning("NUCLEAR_CACHE_RESET applied generation_mode=%s", GENERATION_VERSION)


# =============================================================================
# Prompts (product-only)
# =============================================================================


def build_negative_prompt(*, fast_demo: bool = False) -> str:
    if fast_demo:
        return "human, face, model, body, mannequin, portrait, blurry, low quality."
    # Court et efficace (tokens / stabilite SDXL).
    return (
        "human, face, model, body, mannequin, portrait, hands, skin, "
        "blurry, low quality, deformed, cropped product."
    )


def build_compact_replicate_prompt(c: ProductConcept) -> str:
    return (
        "Product photo only, no human, studio white background, centered. "
        f"{c.product_type} {c.variant}: {c.style}, {c.palette}, {c.material}, {c.year}."
    )


def build_product_only_prompt(c: ProductConcept, *, fast_demo: bool = False) -> str:
    # Prompts compacts product-only (moins de tokens, generations plus stables).
    if c.product_type == "veste":
        core = (
            f"Isolated luxury jacket product photo, studio white-gray backdrop, no person, no face, "
            f"full garment visible, 3/4 view, sharp focus. Style {c.style}, color {c.palette}, "
            f"fabric {c.material}, variant {c.variant}, year {c.year}."
        )
        if fast_demo:
            return core
        return core + f" Silhouette: {c.silhouette}."
    if c.product_type == "ceinture":
        core = (
            f"Isolated luxury belt product photo, studio backdrop, no person, no face, "
            f"buckle visible, macro texture. Style {c.style}, color {c.palette}, material "
            f"{c.material}, variant {c.variant}, year {c.year}."
        )
        if fast_demo:
            return core
        return core + f" Silhouette: {c.silhouette}."
    raise ValueError(f"Unsupported product_type: {c.product_type}")


def _hf_prompt_for_concept(c: ProductConcept) -> str:
    if c.concept_index == 1:
        return (
            "futuristic luxury leather jacket, premium black leather couture outerwear, "
            "high-end product photography, clean studio background, dramatic editorial lighting, "
            "ultra-detailed material texture, fashion campaign quality, no human, no mannequin"
        )
    if c.concept_index == 2:
        return (
            "futuristic technical leather jacket, advanced techwear couture, engineered seams, "
            "premium product photography, minimal clean studio background, sharp editorial lighting, "
            "high-detail leather and hardware, no human, no mannequin"
        )
    return (
        "futuristic luxury leather belt, premium accessory product photo, sculptural buckle detail, "
        "clean luxury studio background, cinematic fashion lighting, ultra-detailed leather grain, "
        "high-end editorial campaign quality, no human"
    )


# =============================================================================
# Replicate : modele, sync run, bytes  (source unique : os.environ, pas de fallback modele)
# =============================================================================


def replicate_image_model_from_env() -> str:
    """FASHION_ORACLE_IMAGE_MODEL uniquement depuis l environnement (ref Replicate tel quel, ex. owner/model ou owner/model:version)."""
    return os.getenv("FASHION_ORACLE_IMAGE_MODEL", "").strip()


def _replicate_image_slug_lower() -> str:
    return replicate_image_model_from_env().lower()


def _replicate_uses_flux_api() -> bool:
    """Heuristique sur FASHION_ORACLE_IMAGE_MODEL (env)."""
    s = _replicate_image_slug_lower()
    return "flux" in s or s.startswith("black-forest-labs/")


def _sdxl_square_side_px() -> int:
    return int(os.getenv("FASHION_ORACLE_SDXL_SIDE_PX", "768"))


def _sdxl_guidance_scale() -> float:
    """SDXL sur Replicate : plage typique 1–50 ; 0 est rejeté (422) → replicate_provider_error."""
    try:
        g = float(os.getenv("FASHION_ORACLE_SDXL_GUIDANCE_SCALE", "7.5").strip())
    except ValueError:
        g = 7.5
    return max(1.0, min(50.0, g))


def _replicate_model_ref() -> str:
    """Ref unique : FASHION_ORACLE_IMAGE_MODEL (pas de FASHION_ORACLE_REPLICATE_VERSION)."""
    m = replicate_image_model_from_env()
    if not m:
        raise RuntimeError("missing_image_model_config")
    if "/" not in m:
        raise RuntimeError("invalid_model_or_provider")
    return m


def _replicate_inference_steps(*, fast_demo: bool) -> int:
    base = int(settings.replicate_num_inference_steps)
    if _replicate_uses_flux_api():
        if fast_demo:
            return max(2, base - 1)
        return max(1, base)
    return min(6, max(4, base if base >= 4 else 4))


def _log_replicate_call_context(
    *,
    concept_index: int,
    prompt_label: str,
    attempt: int,
    max_attempts: int,
    resolved_ref: str,
    single_call_timeout_s: float,
    fast_demo: bool | None = None,
) -> None:
    r = get_image_provider_resolution()
    has_tok = bool((r.replicate_api_token or "").strip()) if r.provider == "replicate" else False
    logger.info("[IMAGE_PROVIDER] %s", r.provider)
    logger.info("[IMAGE_MODEL_ENV] %s", replicate_image_model_from_env() or "(empty)")
    logger.info("[REPLICATE_REF] %s", resolved_ref)
    logger.info("[REPLICATE_TOKEN] %s", "present" if has_tok else "missing")
    logger.info("[IMAGE_TIMEOUT] %s", single_call_timeout_s)
    logger.info("[IMAGE_THROTTLE] %s", MIN_REPLICATE_INTERVAL_S)
    logger.info("[IMAGE_FAST_DEMO_CTX] %s", fast_demo)
    logger.info(
        "REPLICATE_CALL_CTX ci=%s label=%s attempt=%s/%s steps_env=%s fast_demo=%s",
        concept_index,
        prompt_label,
        attempt,
        max_attempts,
        int(settings.replicate_num_inference_steps),
        fast_demo,
    )


# Ref flux-schnell versionnee (diagnostic + fallback si stability-ai/sdxl echoue).
_REPLICATE_FLUX_SCHNELL_REF = (
    "black-forest-labs/flux-schnell:"
    "5d891b49a17e1cdd6786d441e1e3a2f0f76daeeee0c59868bd598b4951d3c58a"
)
_REPLICATE_DIAG_DEFAULT_REF = _REPLICATE_FLUX_SCHNELL_REF


def _replicate_diag_safe_enabled() -> bool:
    return os.getenv("FASHION_ORACLE_REPLICATE_DIAG_SAFE_RUN", "").strip().lower() in (
        "1",
        "true",
        "yes",
        "on",
    )


def _extract_replicate_error_payload(exc: BaseException) -> str | None:
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
    return None


def _emit_replicate_pre_run_logs(
    *,
    logical_model: str,
    effective_ref: str,
    adapter_family: str,
    inp: dict[str, Any],
    fast_demo: bool,
    diag: bool,
) -> None:
    safe_inp = redact_input_for_log(inp)
    in_json = json_for_log(safe_inp)
    print("[REPLICATE] provider=replicate", flush=True)
    print(f"[REPLICATE] pre_run model={effective_ref!r}", flush=True)
    print(f"[REPLICATE] pre_run adapter_family={adapter_family!r}", flush=True)
    print(f"[REPLICATE] pre_run input={in_json}", flush=True)
    print(f"[REPLICATE] FASHION_ORACLE_IMAGE_MODEL={logical_model!r}", flush=True)
    print(f"[REPLICATE] fast_demo={fast_demo}", flush=True)
    print(f"[REPLICATE] diag_safe_run={diag}", flush=True)
    logger.info(
        "[REPLICATE] provider=replicate effective_ref=%r env_image_model=%r fast_demo=%s diag=%s "
        "adapter_family=%r input=%s",
        effective_ref,
        logical_model,
        fast_demo,
        diag,
        adapter_family,
        in_json,
    )


def _replicate_http_status_from_exc(exc: BaseException) -> int | None:
    """Remonte __cause__ pour retrouver un status HTTP (Replicate / httpx)."""
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


def _is_sdxl_flux_fallback_candidate(exc: BaseException) -> bool:
    """
    Echecs type fournisseur / modele / version / payload (un essai flux peut reussir).
    Exclut timeout et 429.
    """
    if isinstance(exc, TimeoutError):
        return False
    if _exception_is_replicate_429(exc):
        return False
    try:
        from replicate.exceptions import ModelError, ReplicateError

        if isinstance(exc, ModelError):
            return True
        if isinstance(exc, ReplicateError):
            st = getattr(exc, "status", None)
            if isinstance(st, int) and st in (400, 404, 422):
                return True
    except ImportError:
        pass
    st2 = _replicate_http_status_from_exc(exc)
    if isinstance(st2, int) and st2 in (400, 404, 422):
        return True
    msg = str(exc).lower()
    needles = (
        "model not found",
        "unknown model",
        "invalid version",
        "invalid input",
        "validation error",
        "does not exist",
        "no such version",
        "model error",
        "schema",
        "unexpected keyword",
        "422",
        "404",
    )
    if any(n in msg for n in needles):
        return True
    return False


def _emit_replicate_error_logs(
    *,
    effective_ref: str,
    inp: dict[str, Any],
    fast_demo: bool,
    exc: BaseException,
    diag: bool,
) -> None:
    payload = _extract_replicate_error_payload(exc)
    emsg = str(exc).replace("\n", " ")[:2000]
    print(f"[REPLICATE] error_type={type(exc).__name__}", flush=True)
    print(f"[REPLICATE] error_message={emsg!r}", flush=True)
    if payload:
        print(f"[REPLICATE] response_body={payload[:3200]}", flush=True)
    logger.error(
        "[REPLICATE] error_type=%s error_message=%r response_body=%r ref=%r fast_demo=%s diag=%s input=%s",
        type(exc).__name__,
        str(exc)[:2000],
        (payload or "")[:4000],
        effective_ref,
        fast_demo,
        diag,
        json_for_log(redact_input_for_log(inp)),
        exc_info=True,
    )


def _sync_replicate_client_run(
    prompt: str,
    token: str,
    *,
    num_inference_steps: int | None = None,
    model_ref: str | None = None,
    fast_demo: bool = False,
) -> tuple[object, rse.ReplicateRunMeta]:
    import replicate as replicate_sdk

    env_model = replicate_image_model_from_env()
    ref = model_ref if model_ref else _replicate_model_ref()
    client = replicate_sdk.Client(api_token=token)

    if _replicate_diag_safe_enabled():
        dref = os.getenv("FASHION_ORACLE_REPLICATE_DIAG_MODEL", "").strip() or _REPLICATE_DIAG_DEFAULT_REF
        inp_diag: dict[str, Any] = {
            "prompt": (
                "studio product photo, single luxury fashion accessory on neutral surface, "
                "no people, no hands"
            ),
            "aspect_ratio": "1:1",
            "num_inference_steps": 4,
        }
        _emit_replicate_pre_run_logs(
            logical_model=env_model,
            effective_ref=dref,
            adapter_family="diag_flux_schnell",
            inp=inp_diag,
            fast_demo=fast_demo,
            diag=True,
        )
        try:
            out = client.run(dref, input=inp_diag)
            return out, rse.ReplicateRunMeta(used_fallback=False, primary_model=dref, fallback_model="")
        except Exception as exc:  # noqa: BLE001
            _emit_replicate_error_logs(
                effective_ref=dref,
                inp=inp_diag,
                fast_demo=fast_demo,
                exc=exc,
                diag=True,
            )
            raise rse.classify_replicate_api_exception(exc, primary_model=dref) from exc

    steps = int(num_inference_steps) if num_inference_steps is not None else int(
        settings.replicate_num_inference_steps
    )
    side = _sdxl_square_side_px()
    guidance = _sdxl_guidance_scale()
    inp, family = build_replicate_input_for_ref(
        ref,
        full_prompt=prompt,
        num_inference_steps=steps,
        side_px=side,
        guidance_scale=guidance,
    )
    _emit_replicate_pre_run_logs(
        logical_model=env_model,
        effective_ref=ref,
        adapter_family=str(family),
        inp=inp,
        fast_demo=fast_demo,
        diag=False,
    )
    try:
        out = client.run(ref, input=inp)
        return out, rse.ReplicateRunMeta(used_fallback=False, primary_model=ref, fallback_model="")
    except Exception as exc:  # noqa: BLE001
        head = (ref.split(":", 1)[0] if ref else "").strip().lower()
        sdxl_provider_detail = _extract_replicate_error_payload(exc) or str(exc)
        logger.info(
            "REPLICATE_SDXL_FAILED primary_model=%r error_type=%s http_status=%s provider_detail=%r",
            ref,
            type(exc).__name__,
            _replicate_http_status_from_exc(exc),
            (sdxl_provider_detail or "")[:4000],
        )
        if not head.startswith("stability-ai/sdxl"):
            _emit_replicate_error_logs(
                effective_ref=ref,
                inp=inp,
                fast_demo=fast_demo,
                exc=exc,
                diag=False,
            )
            raise rse.classify_replicate_api_exception(exc, primary_model=ref) from exc
        if not _is_sdxl_flux_fallback_candidate(exc):
            logger.info(
                "REPLICATE_SDXL_FLUX_FALLBACK_SKIPPED primary_model=%r reason=not_provider_model_or_payload_class",
                ref,
            )
            _emit_replicate_error_logs(
                effective_ref=ref,
                inp=inp,
                fast_demo=fast_demo,
                exc=exc,
                diag=False,
            )
            raise rse.classify_replicate_api_exception(exc, primary_model=ref) from exc
        flux_ref = (os.getenv("FASHION_ORACLE_REPLICATE_FLUX_FALLBACK_REF") or "").strip() or _REPLICATE_FLUX_SCHNELL_REF
        logger.warning(
            "REPLICATE_SDXL_WILL_RETRY_FLUX primary_model=%r fallback_model=%r first_error_type=%s",
            ref,
            flux_ref,
            type(exc).__name__,
        )
        _emit_replicate_error_logs(
            effective_ref=ref,
            inp=inp,
            fast_demo=fast_demo,
            exc=exc,
            diag=False,
        )
        flux_inp: dict[str, Any] = {
            "prompt": extract_positive_prompt_for_run(prompt),
            "aspect_ratio": "1:1",
            "num_inference_steps": 4,
        }
        _emit_replicate_pre_run_logs(
            logical_model=env_model,
            effective_ref=flux_ref,
            adapter_family="flux_schnell_fallback",
            inp=flux_inp,
            fast_demo=fast_demo,
            diag=False,
        )
        try:
            out2 = client.run(flux_ref, input=flux_inp)
            logger.info(
                "REPLICATE_FINAL_OK used_fallback=true primary_model=%r fallback_model=%r",
                ref,
                flux_ref,
            )
            return out2, rse.ReplicateRunMeta(used_fallback=True, primary_model=ref, fallback_model=flux_ref)
        except Exception as exc2:  # noqa: BLE001
            flux_provider_detail = _extract_replicate_error_payload(exc2) or str(exc2)
            logger.error(
                "REPLICATE_IMAGE_GEN_BOTH_FAILED primary_model=%r fallback_model=%r "
                "sdxl_error_type=%s sdxl_provider_detail=%r flux_error_type=%s flux_provider_detail=%r",
                ref,
                flux_ref,
                type(exc).__name__,
                (sdxl_provider_detail or "")[:4000],
                type(exc2).__name__,
                (flux_provider_detail or "")[:4000],
            )
            _emit_replicate_error_logs(
                effective_ref=flux_ref,
                inp=flux_inp,
                fast_demo=fast_demo,
                exc=exc2,
                diag=False,
            )
            both = rse.both_models_failed(
                primary_model=ref,
                fallback_model=flux_ref,
                primary_exc=exc,
                fallback_exc=exc2,
            )
            logger.error(
                "REPLICATE_FINAL_MAP error_reason=%s used_fallback=true primary=%r fallback=%r",
                both.code,
                ref,
                flux_ref,
            )
            raise both from exc2


def _replicate_result_to_bytes(out: object) -> bytes:
    if out is None:
        raise RuntimeError("replicate_empty_output")
    if isinstance(out, (bytes, bytearray)):
        return bytes(out)
    if isinstance(out, str) and out.startswith("http"):
        with httpx.Client(timeout=120.0) as c:
            r = c.get(out)
            r.raise_for_status()
            return r.content
    if isinstance(out, list) and out:
        first = out[0]
        if isinstance(first, str) and first.startswith("http"):
            with httpx.Client(timeout=120.0) as c:
                r = c.get(first)
                r.raise_for_status()
                return r.content
    if hasattr(out, "__iter__") and not isinstance(out, (str, bytes, dict, list)):
        for chunk in out:  # pragma: no cover
            if isinstance(chunk, str) and chunk.startswith("http"):
                with httpx.Client(timeout=120.0) as c:
                    r = c.get(chunk)
                    r.raise_for_status()
                    return r.content
            if hasattr(chunk, "read"):
                data = chunk.read()
                if isinstance(data, bytes):
                    return data
    raise RuntimeError(f"invalid_model_or_provider: sortie Replicate inattendue type={type(out).__name__}")


def _sync_replicate_prompt_to_bytes(
    prompt: str,
    token: str,
    *,
    num_inference_steps: int | None = None,
    model_ref: str | None = None,
    fast_demo: bool = False,
) -> tuple[bytes, rse.ReplicateRunMeta]:
    out, meta = _sync_replicate_client_run(
        prompt,
        token,
        num_inference_steps=num_inference_steps,
        model_ref=model_ref,
        fast_demo=fast_demo,
    )
    try:
        return _replicate_result_to_bytes(out), meta
    except Exception as ex:  # noqa: BLE001
        if isinstance(ex, rse.StructuredReplicateImageError):
            raise
        msg = str(ex).lower()
        if "invalid_model_or_provider" in msg or "replicate_empty_output" in msg or "inattendue" in msg:
            raise rse.StructuredReplicateImageError(
                code=ERR_REPLICATE_BAD_PAYLOAD,
                detail=str(ex),
                primary_model=meta.primary_model,
                fallback_model=meta.fallback_model,
                used_fallback=meta.used_fallback,
                primary_error=str(ex),
            ) from ex
        raise rse.classify_replicate_api_exception(
            ex,
            primary_model=meta.primary_model,
            fallback_model=meta.fallback_model,
            used_fallback=meta.used_fallback,
        ) from ex


def _replicate_model_ref_for_log() -> str:
    try:
        return _replicate_model_ref()
    except Exception:
        return "(modele_replicate_incomplet)"


# =============================================================================
# Erreurs & 429
# =============================================================================


def _exception_is_replicate_429(exc: BaseException) -> bool:
    if isinstance(exc, TimeoutError):
        return False
    msg = str(exc).lower()
    if "429" in str(exc) or "throttl" in msg or "rate limit" in msg or "too many requests" in msg:
        return True
    st = getattr(exc, "status", None)
    if st is None:
        st = getattr(exc, "status_code", None)
    return st == 429


def _exception_is_replicate_insufficient_credit(exc: BaseException) -> bool:
    """Replicate 402 / billing — inutile de retenter flux avec le meme compte."""
    st = _replicate_http_status_from_exc(exc)
    if st == 402:
        return True
    return "insufficient credit" in str(exc).lower()


def map_exception_to_structured_error(
    exc: BaseException,
    *,
    provider: str = "replicate",
) -> rse.StructuredReplicateImageError:
    """Exception -> StructuredReplicateImageError (source unique pour reason + detail API)."""
    if isinstance(exc, rse.StructuredReplicateImageError):
        return exc
    pm = replicate_image_model_from_env() or ""
    if isinstance(exc, RuntimeError):
        s = str(exc).strip()
        if s == "missing_image_model_config":
            return rse.StructuredReplicateImageError(
                code=ERR_MISSING_IMAGE_MODEL,
                detail=s,
                primary_model=pm,
            )
        if s == "invalid_model_or_provider":
            return rse.StructuredReplicateImageError(
                code="invalid_model_or_provider",
                detail=s,
                primary_model=pm,
            )
    if isinstance(exc, TimeoutError):
        code = ERR_OPENAI_IMAGE_TIMEOUT if provider == "openai" else ERR_REPLICATE_TIMEOUT
        return rse.StructuredReplicateImageError(
            code=code,
            detail="Image generation timed out",
            primary_model=pm,
        )
    if _exception_is_replicate_429(exc):
        return rse.classify_replicate_api_exception(exc, primary_model=pm)
    if provider == "replicate" and _exception_is_replicate_insufficient_credit(exc):
        return rse.classify_replicate_api_exception(exc, primary_model=pm)
    if provider == "replicate":
        return rse.classify_replicate_api_exception(exc, primary_model=pm)
    msg = str(exc).lower()
    if "invalid_model_or_provider" in msg or "404" in str(exc) or "422" in str(exc):
        if msg.startswith("invalid_model") or "invalid_model_or_provider" in msg:
            return rse.StructuredReplicateImageError(
                code="invalid_model_or_provider",
                detail=str(exc)[:2000],
                primary_model=pm,
            )
    return rse.StructuredReplicateImageError(
        code=ERR_IMAGE_PROVIDER,
        detail=str(exc)[:2000],
        primary_model=pm,
    )


def map_exception_to_structured_error_reason(
    exc: BaseException,
    *,
    provider: str = "replicate",
) -> str:
    return map_exception_to_structured_error(exc, provider=provider).code


# =============================================================================
# Throttle proactif Replicate
# =============================================================================


async def _replicate_spacing_throttle(concept_index: int) -> None:
    global _last_replicate_request_mono
    if MIN_REPLICATE_INTERVAL_S <= 0:
        return
    async with _replicate_spacing_lock:
        now = time.monotonic()
        wait = MIN_REPLICATE_INTERVAL_S - (now - _last_replicate_request_mono)
        if wait > 0:
            logger.info(
                "REPLICATE_SPACING_WAIT ci=%s wait_s=%.3f (anti-429)",
                concept_index,
                wait,
            )
            await asyncio.sleep(wait)
        _last_replicate_request_mono = time.monotonic()


# =============================================================================
# Retry 429 + timeout par appel
# =============================================================================


async def replicate_run_with_429_backoff_and_timeout(
    full_prompt: str,
    token: str,
    *,
    concept_index: int,
    prompt_label: str,
    num_inference_steps: int | None,
    single_call_timeout_s: float,
    model_ref: str | None = None,
    fast_demo: bool = False,
) -> tuple[bytes, rse.ReplicateRunMeta]:
    """Appels Replicate avec backoff 429 ; ref = FASHION_ORACLE_IMAGE_MODEL (ou model_ref explicite)."""
    max_attempts = 1 + len(REPLICATE_429_BACKOFF_S)
    last_exc: BaseException | None = None
    primary_ref = model_ref or _replicate_model_ref()

    async def _run_stack(ref: str, *, stack_label: str) -> tuple[bytes, rse.ReplicateRunMeta]:
        nonlocal last_exc
        le2: BaseException | None = None
        for attempt in range(max_attempts):
            await _replicate_spacing_throttle(concept_index)
            t_api0 = time.perf_counter()
            _log_replicate_call_context(
                concept_index=concept_index,
                prompt_label=f"{prompt_label}|{stack_label}",
                attempt=attempt + 1,
                max_attempts=max_attempts,
                resolved_ref=ref,
                single_call_timeout_s=single_call_timeout_s,
                fast_demo=fast_demo,
            )
            logger.info(
                "REPLICATE_REQ_START ci=%s label=%s attempt=%s/%s steps=%s timeout_s=%.1f ref=%s fast_demo=%s",
                concept_index,
                prompt_label,
                attempt + 1,
                max_attempts,
                num_inference_steps,
                single_call_timeout_s,
                ref,
                fast_demo,
            )

            def _thread_replicate_bytes() -> tuple[bytes, rse.ReplicateRunMeta]:
                return _sync_replicate_prompt_to_bytes(
                    full_prompt,
                    token,
                    num_inference_steps=num_inference_steps,
                    model_ref=ref,
                    fast_demo=fast_demo,
                )

            try:
                data, rmeta = await asyncio.wait_for(
                    asyncio.to_thread(_thread_replicate_bytes),
                    timeout=single_call_timeout_s,
                )
                logger.info(
                    "REPLICATE_REQ_END ci=%s label=%s attempt=%s/%s ms=%.0f ok=1 ref=%s used_fallback=%s primary=%r fallback=%r",
                    concept_index,
                    prompt_label,
                    attempt + 1,
                    max_attempts,
                    (time.perf_counter() - t_api0) * 1000.0,
                    ref,
                    rmeta.used_fallback,
                    rmeta.primary_model,
                    rmeta.fallback_model,
                )
                return data, rmeta
            except TimeoutError as te:
                le2 = te
                last_exc = te
                logger.warning(
                    "REPLICATE_TIMEOUT ci=%s label=%s attempt=%s/%s single_timeout_s=%.1f ref=%s",
                    concept_index,
                    prompt_label,
                    attempt + 1,
                    max_attempts,
                    single_call_timeout_s,
                    ref,
                )
                if attempt + 1 >= max_attempts:
                    break
                continue
            except Exception as ex:  # noqa: BLE001
                if isinstance(ex, rse.StructuredReplicateImageError):
                    raise
                le2 = ex
                last_exc = ex
                logger.info(
                    "REPLICATE_REQ_END ci=%s label=%s attempt=%s/%s ms=%.0f ok=0 err=%s ref=%s",
                    concept_index,
                    prompt_label,
                    attempt + 1,
                    max_attempts,
                    (time.perf_counter() - t_api0) * 1000.0,
                    type(ex).__name__,
                    ref,
                )
                if not _exception_is_replicate_429(ex):
                    raise
                if attempt + 1 >= max_attempts:
                    break
                delay = REPLICATE_429_BACKOFF_S[attempt]
                logger.warning(
                    "RATE LIMITED - retrying ci=%s label=%s attempt=%s/%s backoff_s=%.1f ref=%s",
                    concept_index,
                    prompt_label,
                    attempt + 1,
                    max_attempts,
                    delay,
                    ref,
                )
                await asyncio.sleep(delay)
        assert le2 is not None
        raise le2

    return await _run_stack(primary_ref, stack_label="primary")


# =============================================================================
# Persistance locale
# =============================================================================


def save_png_bytes_atomic(file_path: Path, image_bytes: bytes) -> bool:
    try:
        tmp = file_path.with_suffix(".tmp.png")
        tmp.write_bytes(image_bytes)
        tmp.replace(file_path)
        logger.info("IMAGE_SAVED path=%s bytes=%s", file_path, file_path.stat().st_size)
        return True
    except OSError as ex:
        logger.error("IMAGE_SAVE_FAIL path=%s err=%s", file_path, ex)
        return False


# =============================================================================
# OpenCV humain
# =============================================================================


def _extract_b64_or_url(result) -> tuple[str | None, str | None]:
    try:
        if getattr(result, "data", None):
            item = result.data[0]
            b64_data = getattr(item, "b64_json", None)
            url = getattr(item, "url", None)
            return b64_data, url
    except Exception:
        pass
    return None, None


def _detect_human_in_image(image_bytes: bytes) -> bool:
    if not image_bytes:
        return True
    if cv2 is None or np is None:
        return False
    try:
        arr = np.frombuffer(image_bytes, dtype=np.uint8)
        img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
        if img is None:
            return True
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        frontal = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
        profile = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_profileface.xml")
        if not frontal.empty():
            if len(frontal.detectMultiScale(gray, scaleFactor=1.08, minNeighbors=4, minSize=(24, 24))) > 0:
                return True
        if not profile.empty():
            if len(profile.detectMultiScale(gray, scaleFactor=1.08, minNeighbors=4, minSize=(24, 24))) > 0:
                return True
        hog = cv2.HOGDescriptor()
        hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
        rects, _ = hog.detectMultiScale(img, winStride=(8, 8), padding=(8, 8), scale=1.05)
        return len(rects) > 0
    except Exception:
        return False


async def _download_to_disk(url: str, file_path: Path) -> None:
    async with httpx.AsyncClient(timeout=HTTPX_TIMEOUT) as http:
        resp = await http.get(url)
        resp.raise_for_status()
        file_path.write_bytes(resp.content)


# =============================================================================
# Item d erreur structure (lot toujours 3 lignes)
# =============================================================================


def generated_visual_error_item(
    c: ProductConcept,
    *,
    error_reason: str,
    prompt_hash_val: str = "",
    cache_key_val: str = "",
    error_detail: str | None = None,
    primary_model: str | None = None,
    fallback_model: str | None = None,
    used_fallback: bool = False,
    primary_model_error: str | None = None,
    fallback_model_error: str | None = None,
) -> GeneratedVisualItem:
    return GeneratedVisualItem(
        year=c.year,
        concept_index=c.concept_index,
        product_type=c.product_type,
        variant=c.variant,
        style=c.style,
        palette=c.palette,
        material=c.material,
        silhouette=c.silhouette,
        status="error",
        error_reason=error_reason,
        error_detail=error_detail,
        primary_model=primary_model,
        fallback_model=fallback_model,
        used_fallback=used_fallback,
        primary_model_error=primary_model_error,
        fallback_model_error=fallback_model_error,
        image_url=None,
        generation_mode=GENERATION_VERSION,
        prompt_hash=prompt_hash_val,
        cache_key=cache_key_val,
    )


def generated_visual_error_from_structured(
    c: ProductConcept,
    structured: rse.StructuredReplicateImageError,
    *,
    prompt_hash_val: str = "",
    cache_key_val: str = "",
) -> GeneratedVisualItem:
    return generated_visual_error_item(
        c,
        error_reason=structured.code,
        prompt_hash_val=prompt_hash_val,
        cache_key_val=cache_key_val,
        error_detail=structured.detail or None,
        primary_model=structured.primary_model or None,
        fallback_model=structured.fallback_model or None,
        used_fallback=structured.used_fallback,
        primary_model_error=structured.primary_error or None,
        fallback_model_error=structured.fallback_error or None,
    )


# =============================================================================
# Cache hit (avant tout appel provider)
# =============================================================================


def try_return_valid_cache(
    c: ProductConcept,
    *,
    host_base: str,
    provider_tag: str,
    fast_demo: bool,
    primary_prompt_hash: str,
) -> GeneratedVisualItem | None:
    t0 = time.perf_counter()
    key = concept_cache_key(c, provider_tag, fast_demo=fast_demo)
    file_name = f"{key}.png"
    file_path = GENERATED_DIR / file_name
    local_url = _local_url_for(host_base, file_name)
    ok, err = validate_cached_image_file(file_path, host_base=host_base, filename=file_name)
    if not ok:
        if not file_path.exists():
            logger.info(
                "CACHE_MISS ci=%s year=%s key_tail=%s",
                c.concept_index,
                c.year,
                key[-48:] if len(key) > 48 else key,
            )
        else:
            logger.info(
                "CACHE_MISS ci=%s year=%s invalid_on_disk=1 reason=%s",
                c.concept_index,
                c.year,
                err,
            )
        if file_path.exists():
            logger.warning(
                "CACHE_INVALID_REMOVE ci=%s path=%s reason=%s",
                c.concept_index,
                file_path,
                err,
            )
            try:
                file_path.unlink()
            except OSError:
                pass
        return None
    data = file_path.read_bytes()
    if STRICT_CACHE_HUMAN_CHECK and _detect_human_in_image(data):
        logger.warning("CACHE_REJECT_HUMAN ci=%s path=%s", c.concept_index, file_path)
        try:
            file_path.unlink()
        except OSError:
            pass
        return None
    logger.info(
        "CACHE_HIT ci=%s year=%s path=%s url=%s validate_ms=%.1f",
        c.concept_index,
        c.year,
        file_path,
        local_url,
        (time.perf_counter() - t0) * 1000.0,
    )
    return GeneratedVisualItem(
        year=c.year,
        concept_index=c.concept_index,
        product_type=c.product_type,
        variant=c.variant,
        style=c.style,
        palette=c.palette,
        material=c.material,
        silhouette=c.silhouette,
        status="cached",
        image_url=local_url,
        cached=True,
        generation_mode=GENERATION_VERSION,
        prompt_hash=primary_prompt_hash,
        cache_key=key,
    )


# =============================================================================
# Flux Replicate pour un concept (hors cache)
# =============================================================================


async def _replicate_product_flow_for_concept(
    c: ProductConcept,
    *,
    host_base: str,
    token: str,
    fast_demo: bool,
    file_path: Path,
    local_url: str,
    cache_key: str,
    primary_ph: str,
) -> GeneratedVisualItem:
    negative = build_negative_prompt(fast_demo=fast_demo)
    strict_suffix = "Product only: no human, no face, no model, no body, no mannequin."
    prompt = build_product_only_prompt(c, fast_demo=fast_demo)
    # Deux passes max : vitesse + stabilite (retry 429 reste actif dans replicate_run_*).
    attempts = [prompt, f"{prompt} {strict_suffix}"]
    steps = _replicate_inference_steps(fast_demo=fast_demo)
    single_to = float(REPLICATE_SINGLE_CALL_TIMEOUT_S)

    last_reason = ERR_REPLICATE_PROVIDER
    last_structured: rse.StructuredReplicateImageError | None = None

    def _success_visual_item(
        image_bytes: bytes,
        rmeta: rse.ReplicateRunMeta,
        *,
        prompt_hash_val: str,
    ) -> GeneratedVisualItem:
        return GeneratedVisualItem(
            year=c.year,
            concept_index=c.concept_index,
            product_type=c.product_type,
            variant=c.variant,
            style=c.style,
            palette=c.palette,
            material=c.material,
            silhouette=c.silhouette,
            status="success",
            image_url=local_url,
            cached=False,
            generation_mode=GENERATION_VERSION,
            prompt_hash=prompt_hash_val,
            cache_key=cache_key,
            primary_model=rmeta.primary_model or None,
            fallback_model=rmeta.fallback_model or None,
            used_fallback=rmeta.used_fallback,
        )

    try:
        _replicate_model_ref()
    except RuntimeError as e:
        se = map_exception_to_structured_error(e, provider="replicate")
        if se.code in (ERR_MISSING_IMAGE_MODEL, "invalid_model_or_provider"):
            return generated_visual_error_from_structured(
                c,
                se,
                prompt_hash_val=primary_ph,
                cache_key_val=cache_key,
            )
        raise

    async def _inner() -> GeneratedVisualItem | None:
        nonlocal last_reason, last_structured
        for idx, active_prompt in enumerate(attempts):
            full_p = f"{active_prompt}\nAvoid: {negative}"
            try:
                image_bytes, rmeta = await replicate_run_with_429_backoff_and_timeout(
                    full_p,
                    token,
                    concept_index=c.concept_index,
                    prompt_label=f"try_{idx}",
                    num_inference_steps=steps,
                    single_call_timeout_s=single_to,
                    fast_demo=fast_demo,
                )
            except TimeoutError:
                last_reason = ERR_REPLICATE_TIMEOUT
                last_structured = map_exception_to_structured_error(
                    TimeoutError("replicate_call"), provider="replicate"
                )
                logger.error("REPLICATE_FLOW_TIMEOUT ci=%s try=%s", c.concept_index, idx)
                continue
            except Exception as ex:  # noqa: BLE001
                se = map_exception_to_structured_error(ex, provider="replicate")
                last_structured = se
                last_reason = se.code
                logger.error("REPLICATE_FLOW_FAIL ci=%s try=%s err=%s", c.concept_index, idx, ex)
                if se.code in _REPLICATE_NON_RETRYABLE_CODES:
                    return generated_visual_error_from_structured(
                        c,
                        se,
                        prompt_hash_val=primary_ph,
                        cache_key_val=cache_key,
                    )
                continue
            if not image_bytes:
                last_reason = ERR_REPLICATE_PROVIDER
                continue
            if _detect_human_in_image(image_bytes):
                last_reason = "human_detected_invalid_output"
                logger.warning("REJECT_HUMAN ci=%s try=%s", c.concept_index, idx)
                continue
            if not save_png_bytes_atomic(file_path, image_bytes):
                last_reason = ERR_LOCAL_SAVE_FAILED
                return generated_visual_error_item(
                    c,
                    error_reason=last_reason,
                    prompt_hash_val=primary_ph,
                    cache_key_val=cache_key,
                )
            return _success_visual_item(image_bytes, rmeta, prompt_hash_val=primary_ph)

        compact = build_compact_replicate_prompt(c)
        full_compact = f"{compact}\nAvoid: human, face, model, body."
        try:
            image_bytes_fb, rmeta_fb = await replicate_run_with_429_backoff_and_timeout(
                full_compact,
                token,
                concept_index=c.concept_index,
                prompt_label="compact_fallback",
                num_inference_steps=steps,
                single_call_timeout_s=single_to,
                fast_demo=fast_demo,
            )
        except TimeoutError:
            last_reason = ERR_REPLICATE_TIMEOUT
            last_structured = map_exception_to_structured_error(
                TimeoutError("replicate_compact"), provider="replicate"
            )
            image_bytes_fb = None
            rmeta_fb = rse.ReplicateRunMeta()
        except Exception as ex:  # noqa: BLE001
            se = map_exception_to_structured_error(ex, provider="replicate")
            last_structured = se
            last_reason = se.code
            image_bytes_fb = None
            rmeta_fb = rse.ReplicateRunMeta()
            logger.error("REPLICATE_COMPACT_FAIL ci=%s err=%s", c.concept_index, ex)
            if se.code in _REPLICATE_NON_RETRYABLE_CODES:
                return generated_visual_error_from_structured(
                    c,
                    se,
                    prompt_hash_val=prompt_hash(compact),
                    cache_key_val=cache_key,
                )

        if image_bytes_fb and not _detect_human_in_image(image_bytes_fb):
            if save_png_bytes_atomic(file_path, image_bytes_fb):
                return _success_visual_item(
                    image_bytes_fb,
                    rmeta_fb,
                    prompt_hash_val=prompt_hash(compact),
                )
            last_reason = ERR_LOCAL_SAVE_FAILED
        elif image_bytes_fb:
            last_reason = "human_detected_invalid_output"

        return None

    try:
        result = await asyncio.wait_for(_inner(), timeout=PER_CONCEPT_TOTAL_TIMEOUT_S)
    except TimeoutError:
        logger.error(
            "PER_CONCEPT_BUDGET_TIMEOUT ci=%s budget_s=%.0f",
            c.concept_index,
            PER_CONCEPT_TOTAL_TIMEOUT_S,
        )
        return generated_visual_error_item(
            c,
            error_reason=ERR_REPLICATE_TIMEOUT,
            prompt_hash_val=primary_ph,
            cache_key_val=cache_key,
            error_detail=f"Dépassement du budget {PER_CONCEPT_TOTAL_TIMEOUT_S:.0f}s par concept.",
        )

    if result is not None:
        return result
    if last_structured is not None:
        return generated_visual_error_from_structured(
            c,
            last_structured,
            prompt_hash_val=primary_ph,
            cache_key_val=cache_key,
        )
    return generated_visual_error_item(
        c,
        error_reason=last_reason,
        prompt_hash_val=primary_ph,
        cache_key_val=cache_key,
    )


# =============================================================================
# API publique : un concept / une annee
# =============================================================================


async def generate_one_image_local(
    c: ProductConcept,
    host_base: str = "http://127.0.0.1:8010",
    *,
    request_fast_mode: bool | None = None,
) -> GeneratedVisualItem:
    resolved = get_image_provider_resolution()
    if resolved.provider == "demo":
        logger.info(
            "IMAGE_DEMO_LOCAL ci=%s year=%s (cache statique, pas d API)",
            c.concept_index,
            c.year,
        )
        return build_demo_visual_item_for_concept(c, host_base)
    if resolved.provider == "demo_local_year":
        logger.info(
            "IMAGE_DEMO_LOCAL_YEAR ci=%s year=%s (generation locale deterministic)",
            c.concept_index,
            c.year,
        )
        return generate_local_year_visual_item(c, host_base)

    fast_demo = bool(request_fast_mode) if request_fast_mode is not None else FAST_IMAGE_DEMO
    prompt = build_product_only_prompt(c, fast_demo=fast_demo)
    ph = prompt_hash(prompt)

    logger.info(
        "IMAGE_PROVIDER_SELECTED provider=%s ci=%s year=%s fast_demo=%s",
        resolved.provider,
        c.concept_index,
        c.year,
        fast_demo,
    )

    if resolved.provider == "invalid":
        fail = (resolved.failure or ERR_INVALID_PROVIDER).strip()
        return generated_visual_error_item(
            c,
            error_reason=fail if fail else ERR_INVALID_PROVIDER,
            prompt_hash_val=ph,
            cache_key_val="",
        )

    if resolved.provider == "replicate":
        print("IMAGE_PROVIDER =", resolved.provider, flush=True)
        print("IMAGE_MODEL =", replicate_image_model_from_env() or "(empty)", flush=True)
        logger.info(
            "[IMAGE_PIPELINE] provider=replicate image_model=%r has_replicate_token=%s",
            replicate_image_model_from_env(),
            bool((resolved.replicate_api_token or "").strip()),
        )
        try:
            _replicate_model_ref()
        except RuntimeError as e:
            return generated_visual_error_item(
                c,
                error_reason=map_exception_to_structured_error_reason(e, provider="replicate"),
                prompt_hash_val=ph,
                cache_key_val="",
            )
    elif resolved.provider != "huggingface":
        return generated_visual_error_item(
            c,
            error_reason=ERR_INVALID_PROVIDER,
            prompt_hash_val=ph,
            cache_key_val="",
        )

    provider_tag = resolved.provider
    key = concept_cache_key(c, provider_tag, fast_demo=fast_demo)
    file_name = f"{key}.png"
    file_path = GENERATED_DIR / file_name
    local_url = _local_url_for(host_base, file_name)

    if resolved.provider == "huggingface":
        hf_name = f"hf_{c.year}_{c.concept_index}.png"
        hf_path = GENERATED_HF_DIR / hf_name
        hf_url = _local_url_for_hf(host_base, hf_name)
        ok_hf_cache, _ = validate_cached_image_file(
            hf_path,
            host_base=host_base,
            filename=hf_name,
        )
        if ok_hf_cache:
            return GeneratedVisualItem(
                year=c.year,
                concept_index=c.concept_index,
                product_type=c.product_type,
                variant=c.variant,
                style=c.style,
                palette=c.palette,
                material=c.material,
                silhouette=c.silhouette,
                status="cached",
                image_url=hf_url,
                cached=True,
                generation_mode=GENERATION_VERSION,
                prompt_hash=ph,
                cache_key=key,
            )
        file_name = hf_name
        file_path = hf_path
        local_url = hf_url
    else:
        cached = try_return_valid_cache(
            c,
            host_base=host_base,
            provider_tag=provider_tag,
            fast_demo=fast_demo,
            primary_prompt_hash=ph,
        )
        if cached is not None:
            return cached

    logger.info(
        "CACHE_MISS ci=%s year=%s cache_key_prefix=%s",
        c.concept_index,
        c.year,
        key[:16],
    )

    negative = build_negative_prompt(fast_demo=fast_demo)
    strict_suffix = "Product only: no human, no face, no model, no body, no mannequin."
    attempts = [prompt, f"{prompt} {strict_suffix}"]

    t_req0 = time.perf_counter()

    async with generation_semaphore:
        logger.info(
            "IMAGE_REQ_START provider=%s model=%s replicate_ref=%s ci=%s year=%s type=%s variant=%s openai_size=%s",
            resolved.provider,
            ("stabilityai/stable-diffusion-2-1" if resolved.provider == "huggingface" else replicate_image_model_from_env()),
            _replicate_model_ref_for_log() if resolved.provider == "replicate" else "-",
            c.concept_index,
            c.year,
            c.product_type,
            c.variant,
            ("512x512" if resolved.provider == "huggingface" else DEMO_IMAGE_SIZE),
        )

        try:
            if resolved.provider == "replicate":
                out = await _replicate_product_flow_for_concept(
                    c,
                    host_base=host_base,
                    token=resolved.replicate_api_token or "",
                    fast_demo=fast_demo,
                    file_path=file_path,
                    local_url=local_url,
                    cache_key=key,
                    primary_ph=ph,
                )
                logger.info(
                    "IMAGE_CONCEPT_DONE provider=replicate ci=%s status=%s total_ms=%.0f",
                    c.concept_index,
                    out.status,
                    (time.perf_counter() - t_req0) * 1000.0,
                )
                return out

            # HuggingFace only flow
            hf_model = (os.getenv("FASHION_ORACLE_IMAGE_MODEL", "").strip() or "black-forest-labs/FLUX.1-schnell")
            api_urls = [
                f"https://api-inference.huggingface.co/models/{hf_model}",
                f"https://router.huggingface.co/hf-inference/models/{hf_model}",
            ]
            headers = {"Authorization": f"Bearer {resolved.hf_api_key}"}
            prompt_hf = _hf_prompt_for_concept(c)
            print("HF PROMPT:", prompt_hf, flush=True)

            def _hf_sync_generate() -> tuple[requests.Response | None, str]:
                last_error = ""
                params = {
                    "negative_prompt": (
                        "low quality, blurry, watermark, text, logo, cartoon, illustration, "
                        "deformed, duplicated object, extra sleeves, bad anatomy, person, face, mannequin"
                    ),
                    "guidance_scale": 7.0 if fast_demo else 8.0,
                    "num_inference_steps": 14 if fast_demo else 20,
                    "width": 512,
                    "height": 512,
                }
                payload = {
                    "inputs": prompt_hf,
                    "parameters": params,
                    "options": {"wait_for_model": True},
                }
                for url in api_urls:
                    try:
                        r = requests.post(
                            url,
                            headers=headers,
                            json=payload,
                            timeout=16 if fast_demo else 35,
                        )
                        if r.status_code == 200:
                            return r, ""
                        last_error = f"url={url} status={r.status_code} body={(r.text or '')[:500]}"
                    except Exception as ex:  # noqa: BLE001
                        last_error = f"url={url} ex={type(ex).__name__}:{ex}"
                return None, last_error

            resp, hf_err = await asyncio.to_thread(_hf_sync_generate)
            if resp is None:
                print("HF ERROR:", hf_err, flush=True)
                logger.warning(
                    "HF_PROVIDER_FAIL_FALLBACK ci=%s year=%s err=%s",
                    c.concept_index,
                    c.year,
                    hf_err[:240],
                )
                return build_demo_visual_item_for_concept(c, host_base)

            image_bytes = resp.content
            if not image_bytes:
                logger.warning(
                    "HF_EMPTY_RESPONSE_FALLBACK ci=%s year=%s",
                    c.concept_index,
                    c.year,
                )
                return build_demo_visual_item_for_concept(c, host_base)
            if not save_png_bytes_atomic(file_path, image_bytes):
                logger.warning(
                    "HF_WRITE_FAIL_FALLBACK ci=%s year=%s path=%s",
                    c.concept_index,
                    c.year,
                    str(file_path),
                )
                return build_demo_visual_item_for_concept(c, host_base)
            logger.info(
                "IMAGE_REQ_OK provider=huggingface ci=%s ms=%.0f",
                c.concept_index,
                (time.perf_counter() - t_req0) * 1000.0,
            )
            return GeneratedVisualItem(
                year=c.year,
                concept_index=c.concept_index,
                product_type=c.product_type,
                variant=c.variant,
                style=c.style,
                palette=c.palette,
                material=c.material,
                silhouette=c.silhouette,
                status="success",
                image_url=local_url,
                cached=False,
                generation_mode=GENERATION_VERSION,
                prompt_hash=ph,
                cache_key=key,
            )

        except Exception as e:
            logger.exception("IMAGE_REQ_EXCEPTION ci=%s err=%s", c.concept_index, e)
            if resolved.provider == "huggingface":
                print("========== HF ERROR ==========", flush=True)
                print(str(e), flush=True)
                traceback.print_exc()
                print("===============================", flush=True)
            se = map_exception_to_structured_error(e, provider=resolved.provider)
            return generated_visual_error_from_structured(
                c,
                se,
                prompt_hash_val=ph,
                cache_key_val=key,
            )


async def generate_visuals_for_year(
    year: int,
    host_base: str = "http://127.0.0.1:8010",
    *,
    request_fast_mode: bool | None = None,
) -> List[GeneratedVisualItem]:
    t_concepts = time.perf_counter()
    concepts = build_concepts_for_year(year)
    logger.info(
        "VISUAL_CONCEPTS_READY year=%s concepts=%s build_ms=%.2f",
        year,
        len(concepts),
        (time.perf_counter() - t_concepts) * 1000.0,
    )
    resolved = get_image_provider_resolution()
    if resolved.provider == "demo":
        logger.info("VISUAL_BATCH_DEMO year=%s items=%s (sans API)", year, len(concepts))
        return build_demo_visual_items(year, host_base)
    if resolved.provider == "demo_local_year":
        logger.info("VISUAL_BATCH_DEMO_LOCAL_YEAR year=%s items=%s (sans API)", year, len(concepts))
        return generate_local_year_visual_items(concepts, host_base)

    t_batch0 = time.perf_counter()
    logger.info(
        "VISUAL_BATCH_START year=%s provider=%s concepts=%s request_fast_mode=%s env_fast_demo=%s",
        year,
        resolved.provider,
        len(concepts),
        request_fast_mode,
        FAST_IMAGE_DEMO,
    )

    if resolved.provider == "replicate":
        n = len(concepts)
        slot_results: list[GeneratedVisualItem | None] = [None] * n
        work: asyncio.Queue[tuple[int, ProductConcept] | None] = asyncio.Queue()
        for slot, c in enumerate(concepts):
            await work.put((slot, c))

        async def _replicate_queue_worker(worker_id: int) -> None:
            while True:
                item = await work.get()
                try:
                    if item is None:
                        return
                    slot, concept = item
                    logger.info(
                        "VISUAL_QUEUE worker=%s slot=%s ci=%s",
                        worker_id,
                        slot,
                        concept.concept_index,
                    )
                    try:
                        slot_results[slot] = await generate_one_image_local(
                            concept,
                            host_base=host_base,
                            request_fast_mode=request_fast_mode,
                        )
                    except Exception as ex:  # noqa: BLE001
                        logger.exception(
                            "VISUAL_QUEUE_ITEM_FAIL worker=%s ci=%s err=%s",
                            worker_id,
                            concept.concept_index,
                            ex,
                        )
                        # Fallback UX: éviter un slot vide si le provider IA tombe.
                        slot_results[slot] = build_demo_visual_item_for_concept(concept, host_base)
                finally:
                    work.task_done()

        workers = [asyncio.create_task(_replicate_queue_worker(wid)) for wid in range(2)]
        await work.join()
        for _ in range(2):
            await work.put(None)
        await asyncio.gather(*workers)
        out: list[GeneratedVisualItem] = []
        for i in range(n):
            r = slot_results[i]
            if r is None:
                r = build_demo_visual_item_for_concept(concepts[i], host_base)
            out.append(r)
            logger.info(
                "VISUAL_BATCH_ITEM year=%s ci=%s status=%s cached=%s err=%s",
                year,
                r.concept_index,
                r.status,
                getattr(r, "cached", False),
                r.error_reason,
            )
        logger.info(
            "VISUAL_BATCH_END year=%s provider=replicate total_ms=%.0f items=%s",
            year,
            (time.perf_counter() - t_batch0) * 1000.0,
            len(out),
        )
        return out

    tasks = [
        generate_one_image_local(c, host_base=host_base, request_fast_mode=request_fast_mode)
        for c in concepts
    ]
    results = await asyncio.gather(*tasks, return_exceptions=True)
    out2: list[GeneratedVisualItem] = []
    for i, r in enumerate(results):
        if isinstance(r, Exception):
            logger.exception("VISUAL_BATCH_PROVIDER_FAIL ci=%s err=%s", concepts[i].concept_index, r)
            out2.append(build_demo_visual_item_for_concept(concepts[i], host_base))
        else:
            out2.append(r)
            logger.info(
                "VISUAL_BATCH_ITEM year=%s ci=%s status=%s",
                year,
                r.concept_index,
                r.status,
            )
    logger.info(
        "VISUAL_BATCH_END year=%s provider=%s total_ms=%.0f",
        year,
        resolved.provider,
        (time.perf_counter() - t_batch0) * 1000.0,
    )
    return out2


async def iter_generate_visuals_for_year(
    year: int,
    host_base: str = "http://127.0.0.1:8010",
    *,
    max_items: int | None = None,
    request_fast_mode: bool | None = None,
):
    """
    Emission progressive des visuels (ordre de fin, pas d attente du lot complet).
    Chaque concept echoue independamment.
    """
    concepts = build_concepts_for_year(year)
    resolved = get_image_provider_resolution()
    if resolved.provider == "demo":
        n = 0
        for it in build_demo_visual_items(year, host_base):
            yield it
            n += 1
            if max_items is not None and n >= max_items:
                return
        return
    if resolved.provider == "demo_local_year":
        n = 0
        for it in generate_local_year_visual_items(concepts, host_base):
            yield it
            n += 1
            if max_items is not None and n >= max_items:
                return
        return

    t_batch = time.perf_counter()
    logger.info(
        "VISUAL_STREAM_BATCH year=%s concepts=%s request_fast_mode=%s host_base=%s",
        year,
        len(concepts),
        request_fast_mode,
        host_base,
    )

    async def _safe(c: ProductConcept) -> GeneratedVisualItem:
        try:
            # Garde-fou stream: un slot ne doit jamais bloquer toute la capsule.
            return await asyncio.wait_for(
                generate_one_image_local(
                    c,
                    host_base=host_base,
                    request_fast_mode=request_fast_mode,
                ),
                timeout=22.0,
            )
        except asyncio.TimeoutError:
            logger.warning(
                "VISUAL_STREAM_ITEM_TIMEOUT ci=%s year=%s -> demo fallback",
                c.concept_index,
                c.year,
            )
            return build_demo_visual_item_for_concept(c, host_base)
        except Exception as ex:  # noqa: BLE001
            logger.exception("VISUAL_STREAM_ITEM_EXCEPTION ci=%s err=%s", c.concept_index, ex)
            # Fallback UX: renvoyer un visuel démo plutôt qu'une erreur bloquante.
            return build_demo_visual_item_for_concept(c, host_base)

    if resolved.provider == "huggingface":
        n = 0
        for c in concepts:
            item = await _safe(c)
            yield item
            n += 1
            if max_items is not None and n >= max_items:
                return
        return

    pending: dict[asyncio.Task, ProductConcept] = {
        asyncio.create_task(_safe(c)): c for c in concepts
    }
    n_yield = 0
    try:
        while pending:
            done, _ = await asyncio.wait(pending.keys(), return_when=asyncio.FIRST_COMPLETED)
            for task in done:
                pending.pop(task, None)
                item = await task
                logger.info(
                    "VISUAL_STREAM_YIELD year=%s ci=%s status=%s cached=%s elapsed_ms=%.0f",
                    year,
                    item.concept_index,
                    item.status,
                    getattr(item, "cached", False),
                    (time.perf_counter() - t_batch) * 1000.0,
                )
                yield item
                n_yield += 1
                if max_items is not None and n_yield >= max_items:
                    return
    finally:
        for t in list(pending.keys()):
            t.cancel()
        if pending:
            await asyncio.gather(*pending.keys(), return_exceptions=True)


# Re-export legacy capsule / Pollinations / LLM (oracle + image_generation_service).
from app.services.generative_pollinations import (  # noqa: E402
    _is_probably_image_bytes,
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
)


def schedule_visual_prewarm_after_predict(year: int, host_base: str) -> None:
    """Optionnel : lancer generate_visuals en fond apres /predict pour remplir le cache."""
    if not PREWARM_AFTER_PREDICT:
        return
    img = get_image_provider_resolution()
    if img.provider != "replicate" or not img.is_configured:
        return
    old = _prewarm_tasks.get(year)
    if old and not old.done():
        logger.info("PREWARM_SKIP year=%s already_running", year)
        return

    async def _job() -> None:
        t0 = time.perf_counter()
        try:
            await generate_visuals_for_year(year, host_base=host_base)
            logger.info(
                "PREWARM_DONE year=%s host_base=%s ms=%.0f",
                year,
                host_base,
                (time.perf_counter() - t0) * 1000.0,
            )
        except Exception as ex:  # noqa: BLE001
            logger.warning("PREWARM_FAIL year=%s err=%s", year, type(ex).__name__)
        finally:
            _prewarm_tasks.pop(year, None)

    try:
        _prewarm_tasks[year] = asyncio.create_task(_job())
        logger.info("PREWARM_SCHEDULED year=%s", year)
    except RuntimeError:
        logger.warning("PREWARM_SKIP year=%s no_event_loop", year)
