"""Règles métier Fashion Oracle : prévision avancée à partir de 2026 et phases temporelles."""
from __future__ import annotations
from typing import Dict

# Année minimale pour le mode prévision « cockpit » (métier avancé)
FORECAST_MIN_YEAR = 2026

STYLE_KEYS = (
    "minimal-tailoring",
    "neo-vintage",
    "athlux-utility",
    "romantic-fluid",
    "artisan-leather",
)


def forecast_phase_for_year(year: int) -> str:
    if year < FORECAST_MIN_YEAR:
        return "historical_non_forecast"
    if year <= 2028:
        return "phase_1"
    if year <= 2032:
        return "phase_2"
    if year <= 2038:
        return "phase_3"
    return "phase_4"


def phase_label_fr(phase: str) -> str:
    return {
        "historical_non_forecast": "Hors périmètre prévision (avant 2026)",
        "phase_1": "2026–2028 · utilité premium, artisanat cuir, minimalisme structuré",
        "phase_2": "2029–2032 · mode modulaire durable, tech-luxe",
        "phase_3": "2033–2038 · silhouettes adaptatives, matières intelligentes, accessoires affirmés",
        "phase_4": "2039+ · couture utilitaire expérimentale, artisanat hybride",
    }.get(phase, phase)


def phase_editorial_mood(phase: str) -> str:
    """Langage visuel / direction éditoriale par phase (injecté dans les prompts image)."""
    return {
        "phase_1": (
            "near-term luxury editorial realism, premium craftsmanship, grounded sophisticated tones, "
            "structured utility elegance, 2026-2028 fashion forecast aesthetic"
        ),
        "phase_2": (
            "mid-term modular sustainable design, refined future utility, tech-luxury crossover, "
            "clean functional luxury, 2029-2032 directional campaign"
        ),
        "phase_3": (
            "advanced adaptive silhouettes, intelligent material aesthetics, bold premium accessories, "
            "experimental yet wearable, 2033-2038 high-end vision"
        ),
        "phase_4": (
            "long-horizon futuristic couture utility, conceptual premium accessories, hybrid craftsmanship, "
            "avant-garde editorial, 2039+ speculative luxury"
        ),
        "historical_non_forecast": "archival fashion documentation, not a forward forecast",
    }.get(phase, "luxury editorial fashion photography")


def phase_style_multipliers(phase: str) -> Dict[str, float]:
    """Pondérations déterministes par phase pour différencier les classements par fenêtre temporelle."""
    base: Dict[str, float] = {s: 1.0 for s in STYLE_KEYS}
    if phase == "phase_1":
        return {
            **base,
            "artisan-leather": 1.38,
            "minimal-tailoring": 1.22,
            "athlux-utility": 1.18,
            "neo-vintage": 0.92,
            "romantic-fluid": 0.88,
        }
    if phase == "phase_2":
        return {
            **base,
            "athlux-utility": 1.32,
            "minimal-tailoring": 1.15,
            "neo-vintage": 1.12,
            "romantic-fluid": 1.05,
            "artisan-leather": 0.9,
        }
    if phase == "phase_3":
        return {
            **base,
            "romantic-fluid": 1.28,
            "athlux-utility": 1.2,
            "minimal-tailoring": 1.1,
            "neo-vintage": 1.05,
            "artisan-leather": 0.92,
        }
    if phase == "phase_4":
        return {
            **base,
            "neo-vintage": 1.25,
            "romantic-fluid": 1.22,
            "artisan-leather": 1.15,
            "minimal-tailoring": 1.08,
            "athlux-utility": 0.95,
        }
    return base


def stable_style_id(style: str) -> int:
    s = 0
    for c in style:
        s = (s * 31 + ord(c)) % 1_000_000_007
    return s


def year_score_jitter(year: int, style: str) -> float:
    """Variation reproductible par (année, style) — sans dépendre de PYTHONHASHSEED."""
    h = (year * 1315423911 + stable_style_id(style)) % 1_000_000_007
    t = (h % 2000) / 2000.0
    # Écart plus large pour permuter le podium selon l'année
    return 0.78 + 0.38 * t
