"""Charge fashion_oracle/.env en premier (override=True) pour eviter variables vides heritees du process parent."""

from __future__ import annotations

import os
from pathlib import Path

from dotenv import dotenv_values, load_dotenv

# app/ -> racine du projet fashion_oracle
FASHION_ORACLE_ROOT = Path(__file__).resolve().parents[1]
DOTENV_PATH = FASHION_ORACLE_ROOT / ".env"


def _backfill_empty_keys_from_file(path: Path) -> None:
    """Complete uniquement les cles absentes ou vides (ex. parent .env ou cwd)."""
    if not path.is_file():
        return
    data = dotenv_values(path)
    for key, val in data.items():
        if not key or val is None:
            continue
        v = str(val).strip()
        if not v:
            continue
        cur = os.environ.get(key)
        if cur is None or not str(cur).strip():
            os.environ[key] = v


def load_fashion_oracle_dotenv(*, override: bool = True) -> bool:
    """
    Retourne True si un fichier .env a ete trouve et charge.
    override=True : les cles du .env remplacent l environnement (indispensable si IMAGE_MODEL vide dans le parent).
    """
    if DOTENV_PATH.is_file():
        load_dotenv(DOTENV_PATH, override=override)
    else:
        load_dotenv(override=override)
    # Si IMAGE_MODEL toujours vide : second fichier .env (racine integration ou cwd).
    if not (os.getenv("FASHION_ORACLE_IMAGE_MODEL") or "").strip():
        _backfill_empty_keys_from_file(FASHION_ORACLE_ROOT.parent / ".env")
        _backfill_empty_keys_from_file(Path.cwd() / ".env")
    return bool(DOTENV_PATH.is_file())


def log_image_env_at_boot() -> None:
    import logging

    log = logging.getLogger("fashion_oracle.env_bootstrap")
    prov = (os.getenv("FASHION_ORACLE_IMAGE_PROVIDER") or "").strip() or "(unset)"
    img = (os.getenv("FASHION_ORACLE_IMAGE_MODEL") or "").strip() or "(empty)"
    log.info(
        "ENV_BOOT dotenv_path=%s exists=%s IMAGE_PROVIDER=%s IMAGE_MODEL=%s TEXT_MODEL=%s",
        DOTENV_PATH,
        DOTENV_PATH.is_file(),
        prov,
        img,
        (os.getenv("FASHION_ORACLE_MODEL") or "").strip()[:48] or "(empty)",
    )
