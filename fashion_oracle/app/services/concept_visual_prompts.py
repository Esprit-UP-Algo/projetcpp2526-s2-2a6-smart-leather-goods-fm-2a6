"""Prompts capsule deterministes : Veste 1 (premium), Veste 2 (technique), Ceinture (accessoire)."""

from __future__ import annotations

from typing import Any, Dict, List

from app.services.generative import build_fashion_look_prompt


# Matrice etendue batch : 12 concepts distincts (validation/demo).
VISUAL_BATCH_SLOTS: List[Dict[str, Any]] = [
    {
        "concept_index": 1,
        "product_type": "jacket",
        "variant": "premium",
        "category_label": "Concept 1 — Veste premium",
    },
    {
        "concept_index": 2,
        "product_type": "jacket",
        "variant": "technical",
        "category_label": "Concept 2 — Veste technique",
    },
    {
        "concept_index": 3,
        "product_type": "belt",
        "variant": "accessory",
        "category_label": "Concept 3 — Ceinture luxe",
    },
    {
        "concept_index": 4,
        "product_type": "jacket",
        "variant": "editorial",
        "category_label": "Concept 4 — Veste editorial",
    },
    {
        "concept_index": 5,
        "product_type": "belt",
        "variant": "minimal",
        "category_label": "Concept 5 — Ceinture minimal",
    },
    {
        "concept_index": 6,
        "product_type": "jacket",
        "variant": "sport_luxe",
        "category_label": "Concept 6 — Veste sport-luxe",
    },
    {
        "concept_index": 7,
        "product_type": "jacket",
        "variant": "premium_dark",
        "category_label": "Concept 7 — Veste premium dark",
    },
    {
        "concept_index": 8,
        "product_type": "belt",
        "variant": "couture",
        "category_label": "Concept 8 — Ceinture couture",
    },
    {
        "concept_index": 9,
        "product_type": "jacket",
        "variant": "utility",
        "category_label": "Concept 9 — Veste utility",
    },
    {
        "concept_index": 10,
        "product_type": "belt",
        "variant": "structured",
        "category_label": "Concept 10 — Ceinture structurée",
    },
    {
        "concept_index": 11,
        "product_type": "jacket",
        "variant": "soft_tailoring",
        "category_label": "Concept 11 — Veste soft tailoring",
    },
    {
        "concept_index": 12,
        "product_type": "belt",
        "variant": "premium_detail",
        "category_label": "Concept 12 — Ceinture premium detail",
    },
]

# Retro-compat : les 3 premiers slots = capsule historique.
CAPSULE_SLOTS: List[Dict[str, Any]] = VISUAL_BATCH_SLOTS[:3]


def batch_slots_slice(count: int) -> List[Dict[str, Any]]:
    n = max(1, min(12, int(count)))
    return list(VISUAL_BATCH_SLOTS[: min(n, len(VISUAL_BATCH_SLOTS))])


def _variant_clause(product_type: str, variant: str, concept_index: int) -> str:
    product_only_lock = (
        "PRODUCT-ONLY lock: isolated product only, no human, no face, no model, no body, no mannequin. "
        "NEGATIVE: human, face, model, body, mannequin, portrait, person, skin, hands."
    )
    if product_type == "jacket" and variant == "premium":
        return (
            f"CAPSULE SLOT {concept_index} — PREMIUM OUTERWEAR: first hero jacket product only, "
            "structured luxury silhouette, refined craftsmanship, couture-level seams, soft studio light, "
            "full jacket fully visible in frame, front or 3/4 angle, floating garment or hanger only, "
            "product-centered packshot composition, isolated jacket only. "
            f"{product_only_lock} NOT a belt, NOT a bag."
        )
    if product_type == "jacket" and variant == "technical":
        return (
            f"CAPSULE SLOT {concept_index} — TECHNICAL OUTERWEAR: second hero jacket MUST be visually DISTINCT "
            "from any premium tailored jacket: performance-luxury utility, modular hardware, athlux-utility mood, "
            "technical materials, sharper contrast studio lighting. Same capsule coherence but clearly a different garment "
            "from slot 1. Full jacket visible in frame, no face crop, no portrait framing, "
            "clear utility silhouette and construction details, zipper/pocket/panel details visible. "
            "Centered product-first composition, isolated jacket only, hanger or invisible support. "
            f"{product_only_lock} NOT a belt, NOT a bag."
        )
    if product_type == "jacket" and variant in {"editorial", "sport_luxe", "premium_dark", "utility", "soft_tailoring"}:
        return (
            f"CAPSULE SLOT {concept_index} — JACKET PRODUCT HERO: one jacket object only, visually distinct variant={variant}, "
            "premium studio packshot, jacket-first composition, full garment readability, isolated jacket only. "
            f"{product_only_lock} NOT a belt, NOT a bag."
        )
    if product_type == "belt":
        return (
            f"CAPSULE SLOT {concept_index} — BELT ACCESSORY ONLY: single luxury belt product hero, zero humans, "
            "premium product photography on clean luxury background, sharp leather and buckle macro detail, "
            "close-up accessory framing. NO jacket as hero, NO handbag, NO model, NO hands holding the belt. "
            f"{product_only_lock}"
        )
    return f"CAPSULE SLOT {concept_index} — luxury fashion product hero, isolated."


def build_capsule_visual_prompt(
    *,
    year: int,
    forecast_phase: str,
    era_mood: str,
    concept_index: int,
    product_type: str,
    variant: str,
    style: str,
    palette: str,
    material: str,
    silhouette: str,
    image_prompt_override: str | None,
) -> str:
    if image_prompt_override and image_prompt_override.strip():
        return (
            f"{image_prompt_override.strip()} | {_variant_clause(product_type, variant, concept_index)} "
            f"| year={year} phase={forecast_phase}"
        )
    base = build_fashion_look_prompt(
        year=year,
        style=style,
        palette=palette,
        material=material,
        silhouette=silhouette,
        product_type=product_type,
        forecast_phase=forecast_phase,
        era_mood=era_mood,
    )
    clause = _variant_clause(product_type, variant, concept_index)
    return f"{base} {clause} (concept_index={concept_index}, variant={variant})."


def capsule_cache_key(
    *,
    year: int,
    forecast_phase: str,
    concept_index: int,
    product_type: str,
    variant: str,
    style: str,
    palette: str,
    material: str,
    silhouette: str,
    prompt_fp: str,
) -> str:
    """Cle disque rapide : annee + slot + type + attributs tendance (variant distingue les 2 vestes)."""
    ph = (forecast_phase or "").replace("|", "_")[:48]
    sil = (silhouette or "").strip()[:60]
    fp = (prompt_fp or "").strip()[:16]
    # v23: face detection fix + Pollinations/editorial removal + post-save human reject.
    return f"v23|{year}|{ph}|ci{concept_index}|{product_type}|{variant}|{style}|{palette}|{material}|{sil}|fp{fp}"
