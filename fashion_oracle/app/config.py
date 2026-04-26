import os
from dataclasses import dataclass, field

from app.env_bootstrap import load_fashion_oracle_dotenv

# Avant tout os.getenv utilise par Settings (importe depuis plusieurs modules).
load_fashion_oracle_dotenv()


def _env_bool(name: str, default: bool = True) -> bool:
    v = os.getenv(name)
    if v is None or not str(v).strip():
        return default
    return str(v).strip().lower() in ("1", "true", "yes", "on")


@dataclass
class Settings:
    # Texte / cout LLM (OpenRouter). Les images /generate-visuals utilisent app.image_provider (priorite stricte).
    api_key: str = os.getenv("FASHION_ORACLE_API_KEY", "")
    model: str = os.getenv("FASHION_ORACLE_MODEL", "openai/gpt-4o-mini")
    # Vide par defaut : la resolution Replicate lit os.environ dans generative.py (source unique).
    image_model: str = os.getenv("FASHION_ORACLE_IMAGE_MODEL", "")
    db_url: str = os.getenv("FASHION_ORACLE_DB", "sqlite:///./fashion_oracle.db")
    base_url: str = os.getenv("FASHION_ORACLE_BASE_URL", "https://openrouter.ai/api/v1")
    # Replicate (image) : token https://replicate.com/account/api-tokens (prefixe r8_)
    replicate_api_token: str = os.getenv(
        "FASHION_ORACLE_REPLICATE_API_TOKEN",
        os.getenv("REPLICATE_API_TOKEN", ""),
    )
    # Modèle OpenRouter dédié au simulateur de coûts (sinon FASHION_ORACLE_MODEL)
    cost_model: str = os.getenv("FASHION_ORACLE_COST_MODEL", "")
    # URL publique absolue pour les visuels servis sous /static/ (sinon la route utilise l hote de la requete).
    public_base_url: str = os.getenv("FASHION_ORACLE_PUBLIC_URL", "").strip()
    # Capsule : timeout global par slot (telechargement Pollinations peut depasser 90s sous charge).
    capsule_slot_timeout_s: float = float(os.getenv("FASHION_ORACLE_CAPSULE_SLOT_TIMEOUT_S", "165"))
    # Capsule : 1 = pas d attente Replicate/OpenRouter (beaucoup plus rapide). 0 = chemin complet qualite.
    capsule_fast_providers: bool = field(default_factory=lambda: _env_bool("FASHION_ORACLE_CAPSULE_FAST", True))
    # 1 = slots capsule en serie (lent). 0 = parallele borne par batch_max_parallel (defaut).
    capsule_serial_when_fast: bool = field(default_factory=lambda: _env_bool("FASHION_ORACLE_CAPSULE_FAST_SERIAL", False))
    # Parallele borne : nombre max de jobs image simultanes (fournisseur + telechargement).
    batch_max_parallel: int = int(os.getenv("FASHION_ORACLE_BATCH_PARALLEL", "3"))
    # Mode batch rapide : cote image Pollinations plus petit (multiple de 64).
    fast_batch_image_edge_px: int = int(os.getenv("FASHION_ORACLE_FAST_BATCH_EDGE_PX", "384"))
    # Timeout par slot en mode batch rapide (secondes).
    fast_batch_slot_timeout_s: float = float(os.getenv("FASHION_ORACLE_FAST_BATCH_SLOT_TIMEOUT_S", "25"))
    # Nombre max d images par requete batch (1..12).
    batch_max_count: int = int(os.getenv("FASHION_ORACLE_BATCH_MAX_COUNT", "12"))
    # Cote Pollinations / OpenRouter images : compromis rapidite / nettete visage.
    image_edge_px: int = int(os.getenv("FASHION_ORACLE_IMAGE_EDGE_PX", "512"))
    # Replicate : SDXL Lightning ~4 steps ; Flux schnell peut rester bas aussi.
    replicate_num_inference_steps: int = int(os.getenv("FASHION_ORACLE_REPLICATE_STEPS", "4"))
    # Modes visuels:
    # - safe_product: produit uniquement (aucun humain) -> mode validation recommandé
    # - safe_model: humain autorisé mais cadrage produit + anti-portrait
    visual_mode: str = os.getenv("FASHION_ORACLE_VISUAL_MODE", "safe_product").strip().lower()
    # Steps qualité quand on autorise un modèle humain (safe_model).
    replicate_quality_steps: int = int(os.getenv("FASHION_ORACLE_REPLICATE_QUALITY_STEPS", "8"))


settings = Settings()
