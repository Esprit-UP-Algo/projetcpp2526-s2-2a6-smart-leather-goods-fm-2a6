"""
Adaptateurs d entree Replicate par famille de modele (pas de payload generique unique).

Chaque famille construit le dict `input` attendu par l API du modele cible.
"""

from __future__ import annotations

import json
import logging
from typing import Any, Literal

logger = logging.getLogger(__name__)

ModelFamily = Literal["flux", "stability_sdxl", "sdxl_lightning", "unknown"]


def _head(ref: str) -> str:
    return (ref.split(":", 1)[0] if ref else "").strip().lower()


def classify_replicate_model(ref: str) -> ModelFamily:
    h = _head(ref)
    if "flux" in h or h.startswith("black-forest-labs/"):
        return "flux"
    if "lucataco" in h and "lightning" in h:
        return "sdxl_lightning"
    if h.startswith("stability-ai/sdxl"):
        return "stability_sdxl"
    if "sdxl-lightning" in h or "sdxl_lightning" in h:
        return "sdxl_lightning"
    return "unknown"


def _split_prompt_positive_negative(full: str) -> tuple[str, str]:
    if "\nAvoid:" in full:
        pos, neg = full.split("\nAvoid:", 1)
        return pos.strip(), (neg.strip() or "worst quality, low quality, blurry")
    return full.strip(), "worst quality, low quality, blurry"


def extract_positive_prompt_for_run(full_prompt: str) -> str:
    """Partie positive (avant Avoid) pour flux / fallback."""
    pos, _ = _split_prompt_positive_negative(full_prompt)
    t = pos.strip() or full_prompt.strip()
    return t[:4000]


def build_replicate_input_for_ref(
    ref: str,
    *,
    full_prompt: str,
    num_inference_steps: int,
    side_px: int,
    guidance_scale: float,
) -> tuple[dict[str, Any], ModelFamily]:
    """
    Retourne (input_dict, famille_declaree).

    famille == unknown : on retombe sur le schema stability-ai/sdxl (large compatibilite)
    tout en loggant un avertissement — ajouter une branche explicite si besoin.
    """
    family = classify_replicate_model(ref)
    effective: ModelFamily = family
    if family == "unknown":
        h = _head(ref)
        if "sdxl" in h:
            effective = "stability_sdxl"
        elif "flux" in h or h.startswith("black-forest-labs/"):
            effective = "flux"
        else:
            effective = "stability_sdxl"
        logger.warning(
            "[REPLICATE_ADAPTER] unknown family for ref head=%r — using effective=%s",
            h,
            effective,
        )

    steps = max(1, int(num_inference_steps))

    if effective == "flux":
        inp: dict[str, Any] = {
            "prompt": full_prompt[:4000],
            "num_inference_steps": max(1, min(12, steps)),
            "aspect_ratio": "1:1",
        }
        return inp, family

    if effective == "sdxl_lightning":
        # lucataco / SDXL-Lightning : pas de champ scheduler (souvent absent du schema Cog).
        pos, neg = _split_prompt_positive_negative(full_prompt)
        lt_steps = max(2, min(8, min(steps, 4)))
        return (
            {
                "prompt": pos[:2000],
                "negative_prompt": neg[:800],
                "width": int(side_px),
                "height": int(side_px),
                "num_inference_steps": lt_steps,
                "guidance_scale": max(1.0, min(50.0, float(guidance_scale))),
            },
            family,
        )

    # stability-ai/sdxl : schéma minimal validé par l API (pas scheduler, pas aspect_ratio).
    pos, _ = _split_prompt_positive_negative(full_prompt)
    prompt_text = pos.strip()[:4000] if pos.strip() else full_prompt.strip()[:4000]
    return (
        {
            "prompt": prompt_text,
            "negative_prompt": "blurry, low quality, distorted",
            "width": 1024,
            "height": 1024,
            "num_inference_steps": 25,
            "guidance_scale": 7.5,
        },
        family,
    )


def json_for_log(obj: Any, *, max_len: int = 4000) -> str:
    """Serialisation JSON pour logs (tronquee)."""
    try:
        s = json.dumps(obj, ensure_ascii=False, default=str)
    except TypeError:
        s = repr(obj)
    if len(s) > max_len:
        return s[: max_len - 3] + "..."
    return s


def redact_input_for_log(inp: dict[str, Any], *, prompt_clip: int = 220) -> dict[str, Any]:
    """Copie legere pour logs (prompt tronque)."""
    out = dict(inp)
    if "prompt" in out and isinstance(out["prompt"], str) and len(out["prompt"]) > prompt_clip:
        out["prompt"] = out["prompt"][:prompt_clip] + "…"
    return out
