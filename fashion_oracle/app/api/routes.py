import json
import logging
import os
import random
import time
import asyncio
from fastapi import APIRouter, HTTPException, Query, Request
from fastapi.responses import StreamingResponse

from app.models.schemas import FashionPrediction, VisualGenerationResponse
from app.services.prediction_service import PREDICTION_ENGINE_VERSION, compute_fashion_prediction

router = APIRouter()
_log = logging.getLogger("fashion_oracle.routes")


@router.get("/health")
async def health():
    port_hint = os.getenv("FASHION_ORACLE_HTTP_PORT") or os.getenv("PORT", "not_set")
    return {
        "status": "ok",
        "service": "Fashion Oracle",
        "process_id": os.getpid(),
        "http_port_hint": port_hint,
        "mode": "forecast-cockpit",
        "prediction_engine_version": PREDICTION_ENGINE_VERSION,
    }


@router.get("/debug-config")
async def debug_image_config():
    """Diagnostic runtime du cockpit prévisionnel."""
    return {
        "prediction_engine_version": PREDICTION_ENGINE_VERSION,
        "cache_enabled": os.getenv("FASHION_ORACLE_PREDICTION_CACHE", "1"),
        "cache_max": os.getenv("FASHION_ORACLE_PREDICTION_CACHE_MAX", "128"),
        "image_generation": "disabled",
    }


@router.get("/predict", response_model=FashionPrediction)
async def predict(
    request: Request,
    year: int = Query(..., ge=1960, le=2050),
    nocache: bool = Query(False, description="Force le recalcul (ignore le cache prediction)"),
    scenario: str = Query("balanced", description="Mode scénario: conservative|balanced|disruptive"),
) -> FashionPrediction:
    """Prévision serveur (séries + taxonomie), cache par annee + version moteur — sans generation image."""
    t0 = time.perf_counter()
    _log.info("PREDICT_ROUTE_START year=%s engine=%s nocache=%s scenario=%s", year, PREDICTION_ENGINE_VERSION, nocache, scenario)
    try:
        pred = compute_fashion_prediction(year, bypass_cache=bool(nocache), scenario=scenario)
        _log.info(
            "PREDICT_ROUTE_END year=%s ms=%.0f from_cache=%s",
            year,
            (time.perf_counter() - t0) * 1000.0,
            pred.from_cache,
        )
        return pred
    except Exception as ex:  # noqa: BLE001
        _log.exception("PREDICT_FAIL year=%s err=%s", year, ex)
        raise


@router.get("/generate-visuals", response_model=VisualGenerationResponse)
async def generate_visuals(
    year: int,
    request: Request,
    fast_mode: bool | None = Query(
        None,
        description="Si absent : variable d environnement FASHION_ORACLE_IMAGE_FAST_DEMO",
    ),
):
    raise HTTPException(
        status_code=410,
        detail="Image generation has been removed. Use /predict for advanced capsule projection.",
    )


@router.get("/generate-visuals-capsule")
async def generate_visuals_capsule(
    request: Request,
    year: int = Query(..., ge=1960, le=2050),
    count: int = Query(3, ge=1, le=3),
    fast_mode: bool | None = Query(
        None,
        description="Si absent : FASHION_ORACLE_IMAGE_FAST_DEMO",
    ),
):
    raise HTTPException(
        status_code=410,
        detail="Capsule visual endpoint removed. Use /predict to get concepts_projection.",
    )


@router.get("/generate-visuals-capsule/stream")
async def generate_visuals_capsule_stream(
    request: Request,
    year: int = Query(..., ge=1960, le=2050),
    count: int = Query(3, ge=1, le=3),
    fast_mode: bool = Query(True),
):
    async def ndjson_body():
        yield json.dumps(
            {
                "event": "error",
                "year": year,
                "message": "Image stream removed. Query /predict for advanced projection.",
            },
            ensure_ascii=False,
        ) + "\n"
        yield json.dumps({"event": "done", "year": year}, ensure_ascii=False) + "\n"

    return StreamingResponse(ndjson_body(), media_type="application/x-ndjson")
