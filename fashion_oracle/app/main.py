from contextlib import asynccontextmanager
import os
from pathlib import Path

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from starlette.responses import RedirectResponse

from app.env_bootstrap import DOTENV_PATH, load_fashion_oracle_dotenv, log_image_env_at_boot

# Charger fashion_oracle/.env avant tout import app.* qui lit os.environ (override=True).
load_fashion_oracle_dotenv()
from app.generation_profile import GENERATION_VERSION
print("DOTENV file =", DOTENV_PATH, "exists =", DOTENV_PATH.is_file(), flush=True)
print("BOOT pid =", os.getpid(), flush=True)
print(
    "BOOT http_port_hint (FASHION_ORACLE_HTTP_PORT|PORT) =",
    os.getenv("FASHION_ORACLE_HTTP_PORT") or os.getenv("PORT", "not_set"),
    flush=True,
)
print("IMAGE_PROVIDER =", os.getenv("FASHION_ORACLE_IMAGE_PROVIDER"), flush=True)
print("IMAGE_MODEL =", repr(os.getenv("FASHION_ORACLE_IMAGE_MODEL")), flush=True)
print("generation_version =", GENERATION_VERSION, flush=True)

from app.api.cost_routes import router as cost_router
from app.api.routes import router
from app.api.metiers_annexe_routes import router as metiers_annexe_router
from app.api.simulateur_cout_routes import (
    router as simulateur_cout_router,
    router_cout_simulateur as cout_simulateur_router,
)
from app.demo_assets import DEMO_VISUALS_DIR, ensure_demo_visual_png_files
from app.image_provider import log_image_provider_at_startup, reset_image_provider_cache_for_tests

_FASHION_ORACLE_ROOT = Path(__file__).resolve().parents[1]
GENERATED_DIR = _FASHION_ORACLE_ROOT / "generated_visuals"
GENERATED_DEMO_ROOT = _FASHION_ORACLE_ROOT / "static" / "generated_demo"
GENERATED_OPENAI_ROOT = _FASHION_ORACLE_ROOT / "static" / "generated_openai"
GENERATED_HF_ROOT = _FASHION_ORACLE_ROOT / "static" / "generated_hf"
GENERATED_DIR.mkdir(parents=True, exist_ok=True)
DEMO_VISUALS_DIR.mkdir(parents=True, exist_ok=True)
GENERATED_DEMO_ROOT.mkdir(parents=True, exist_ok=True)
GENERATED_OPENAI_ROOT.mkdir(parents=True, exist_ok=True)
GENERATED_HF_ROOT.mkdir(parents=True, exist_ok=True)


@asynccontextmanager
async def _lifespan(_app: FastAPI):
    reset_image_provider_cache_for_tests()
    log_image_env_at_boot()
    log_image_provider_at_startup()
    ensure_demo_visual_png_files()
    try:
        from app.services.local_visual_generator import ensure_local_visual_workspace

        ensure_local_visual_workspace()
    except Exception as ex:  # noqa: BLE001
        print("WARN local_visual_workspace init skipped:", type(ex).__name__, ex, flush=True)
    yield


app = FastAPI(lifespan=_lifespan)


@app.get("/docs.", include_in_schema=False)
def redirect_swagger_trailing_dot() -> RedirectResponse:
    """Evite le 404 quand l'URL est copiee avec un point final (/docs.)."""
    return RedirectResponse(url="/docs", status_code=307)


@app.get("/redoc.", include_in_schema=False)
def redirect_redoc_trailing_dot() -> RedirectResponse:
    return RedirectResponse(url="/redoc", status_code=307)


app.include_router(router)
app.include_router(cost_router)
app.include_router(simulateur_cout_router)
app.include_router(cout_simulateur_router)
app.include_router(metiers_annexe_router)
app.mount("/static/generated", StaticFiles(directory=str(GENERATED_DIR)), name="generated")
app.mount(
    "/static/demo_visuals",
    StaticFiles(directory=str(DEMO_VISUALS_DIR)),
    name="demo_visuals",
)
app.mount(
    "/static/generated_demo",
    StaticFiles(directory=str(GENERATED_DEMO_ROOT)),
    name="generated_demo",
)
app.mount(
    "/static/generated_openai",
    StaticFiles(directory=str(GENERATED_OPENAI_ROOT)),
    name="generated_openai",
)
app.mount(
    "/static/generated_hf",
    StaticFiles(directory=str(GENERATED_HF_ROOT)),
    name="generated_hf",
)
