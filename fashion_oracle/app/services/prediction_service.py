"""Moteur de prediction Fashion Oracle: mathematique temporelle + selection competitive."""

from __future__ import annotations

import logging
import math
import os
import time
from collections import OrderedDict
from dataclasses import dataclass
from typing import Final

from app.models.schemas import ConceptPreview, FashionPrediction, ForecastConceptInsight
from app.services.forecast_phases import FORECAST_MIN_YEAR, forecast_phase_for_year
from app.services.recommender import recommend_product_attributes

_log = logging.getLogger("fashion_oracle.prediction")
PREDICTION_ENGINE_VERSION: Final[str] = "forecast_cockpit_v8"
_MAX_CACHE_ENTRIES = int(os.getenv("FASHION_ORACLE_PREDICTION_CACHE_MAX", "128"))
_prediction_lru: OrderedDict[tuple[int, str, str], FashionPrediction] = OrderedDict()

_TREND_FAMILIES = (
    "artisan-leather",
    "romantic-fluid",
    "athlux-utility",
    "minimal-tailoring",
    "conceptual-futurism",
)

_TREND_CURVE_CONFIG: dict[str, dict[str, float | str]] = {
    "artisan-leather": {"shape": "gaussian", "base": 16.0, "amplitude": 74.0, "pivot": 2028.0, "speed": 3.2},       # early peak
    "romantic-fluid": {"shape": "gaussian", "base": 14.0, "amplitude": 70.0, "pivot": 2034.0, "speed": 3.8},        # mid peak
    "conceptual-futurism": {"shape": "gaussian", "base": 10.0, "amplitude": 88.0, "pivot": 2045.0, "speed": 3.9},   # late peak
    "athlux-utility": {"shape": "long_rise", "base": 12.0, "amplitude": 78.0, "pivot": 2034.0, "speed": 5.4},       # long rise
    "minimal-tailoring": {"shape": "decline", "base": 22.0, "amplitude": 64.0, "pivot": 2026.0, "speed": 8.2},      # decline
}

_SCENARIO_CONFIG: dict[str, dict[str, float]] = {
    "conservative": {"volatility": 0.35, "shock_intensity": 0.55, "conflict_intensity": 0.75},
    "balanced": {"volatility": 0.60, "shock_intensity": 1.0, "conflict_intensity": 1.0},
    "disruptive": {"volatility": 0.95, "shock_intensity": 1.35, "conflict_intensity": 1.25},
}

_TREND_CONFLICT_MATRIX: dict[str, dict[str, float]] = {
    "artisan-leather": {"athlux-utility": -0.12, "conceptual-futurism": -0.14, "minimal-tailoring": 0.05},
    "romantic-fluid": {"minimal-tailoring": -0.10, "conceptual-futurism": 0.08},
    "athlux-utility": {"artisan-leather": -0.10, "romantic-fluid": -0.07, "conceptual-futurism": 0.10},
    "minimal-tailoring": {"romantic-fluid": -0.11, "conceptual-futurism": -0.09},
    "conceptual-futurism": {"minimal-tailoring": -0.16, "artisan-leather": -0.08, "athlux-utility": 0.10},
}

_TREND_SHOCK_EVENTS: list[dict[str, object]] = [
    {
        "year": 2030,
        "label": "Supply chain compression",
        "effects": {"artisan-leather": -10.0, "athlux-utility": 5.0, "minimal-tailoring": -4.0},
    },
    {
        "year": 2036,
        "label": "Smart textile breakthrough",
        "effects": {"conceptual-futurism": 11.0, "athlux-utility": 8.0, "artisan-leather": -5.0},
    },
    {
        "year": 2042,
        "label": "Luxury austerity cycle",
        "effects": {"conceptual-futurism": -8.0, "minimal-tailoring": 6.0, "romantic-fluid": 4.0},
    },
]

_STYLE_VECTORS: dict[str, dict[str, float]] = {
    "artisan-leather": {"family": "craft", "w": {"artisan-leather": 0.62, "minimal-tailoring": 0.18, "athlux-utility": 0.08, "romantic-fluid": 0.08, "conceptual-futurism": 0.04}},
    "romantic-fluid": {"family": "fluid", "w": {"romantic-fluid": 0.62, "minimal-tailoring": 0.14, "athlux-utility": 0.06, "artisan-leather": 0.08, "conceptual-futurism": 0.10}},
    "athlux-utility": {"family": "technical", "w": {"athlux-utility": 0.68, "minimal-tailoring": 0.14, "conceptual-futurism": 0.10, "artisan-leather": 0.04, "romantic-fluid": 0.04}},
    "minimal-tailoring": {"family": "minimal", "w": {"minimal-tailoring": 0.66, "athlux-utility": 0.16, "artisan-leather": 0.10, "romantic-fluid": 0.04, "conceptual-futurism": 0.04}},
    "conceptual-futurism": {"family": "experimental", "w": {"conceptual-futurism": 0.70, "athlux-utility": 0.14, "minimal-tailoring": 0.10, "romantic-fluid": 0.04, "artisan-leather": 0.02}},
    "neo-vintage": {"family": "heritage", "w": {"artisan-leather": 0.34, "minimal-tailoring": 0.24, "romantic-fluid": 0.20, "athlux-utility": 0.12, "conceptual-futurism": 0.10}},
}

_PALETTES = [
    ("sand", "warm", {"warm": 0.92, "cool": 0.08, "neutral": 0.20, "exp": 0.05}),
    ("cognac", "warm", {"warm": 0.94, "cool": 0.06, "neutral": 0.16, "exp": 0.04}),
    ("espresso", "warm", {"warm": 0.86, "cool": 0.14, "neutral": 0.22, "exp": 0.08}),
    ("taupe", "warm", {"warm": 0.70, "cool": 0.30, "neutral": 0.58, "exp": 0.14}),
    ("charcoal", "cool", {"warm": 0.10, "cool": 0.92, "neutral": 0.55, "exp": 0.20}),
    ("slate", "cool", {"warm": 0.12, "cool": 0.90, "neutral": 0.50, "exp": 0.24}),
    ("steel", "cool", {"warm": 0.14, "cool": 0.88, "neutral": 0.46, "exp": 0.32}),
    ("graphite", "cool", {"warm": 0.10, "cool": 0.90, "neutral": 0.56, "exp": 0.28}),
    ("off-white", "neutral", {"warm": 0.42, "cool": 0.42, "neutral": 0.90, "exp": 0.26}),
    ("smoke", "neutral", {"warm": 0.34, "cool": 0.56, "neutral": 0.88, "exp": 0.28}),
    ("ink-blue", "neutral", {"warm": 0.24, "cool": 0.72, "neutral": 0.80, "exp": 0.34}),
    ("greige", "neutral", {"warm": 0.52, "cool": 0.40, "neutral": 0.86, "exp": 0.20}),
    ("neon-lime", "accent", {"warm": 0.08, "cool": 0.46, "neutral": 0.26, "exp": 0.98}),
    ("chrome-accent", "accent", {"warm": 0.06, "cool": 0.62, "neutral": 0.32, "exp": 0.96}),
]
_MATERIALS = [
    ("full-grain leather", "craft", {"craft": 0.96, "technical": 0.18, "minimal": 0.44, "experimental": 0.12}),
    ("nubuck", "craft", {"craft": 0.90, "technical": 0.24, "minimal": 0.46, "experimental": 0.16}),
    ("wool gabardine", "craft", {"craft": 0.84, "technical": 0.34, "minimal": 0.58, "experimental": 0.20}),
    ("technical nylon", "technical", {"craft": 0.10, "technical": 0.96, "minimal": 0.44, "experimental": 0.52}),
    ("structured mesh", "technical", {"craft": 0.08, "technical": 0.90, "minimal": 0.38, "experimental": 0.64}),
    ("bonded wool", "technical", {"craft": 0.34, "technical": 0.84, "minimal": 0.52, "experimental": 0.48}),
    ("matte leather", "minimal", {"craft": 0.64, "technical": 0.42, "minimal": 0.88, "experimental": 0.28}),
    ("brushed cotton", "minimal", {"craft": 0.54, "technical": 0.36, "minimal": 0.84, "experimental": 0.24}),
    ("clean twill", "minimal", {"craft": 0.46, "technical": 0.44, "minimal": 0.90, "experimental": 0.26}),
    ("reactive textile", "experimental", {"craft": 0.06, "technical": 0.64, "minimal": 0.30, "experimental": 0.98}),
    ("bio-fiber satin", "experimental", {"craft": 0.18, "technical": 0.52, "minimal": 0.38, "experimental": 0.94}),
    ("coated linen", "experimental", {"craft": 0.26, "technical": 0.56, "minimal": 0.42, "experimental": 0.90}),
]
_SILHOUETTES = [
    ("structured coat", "craft", {"craft": 0.90, "technical": 0.24, "minimal": 0.54, "experimental": 0.18}),
    ("clean shoulder", "minimal", {"craft": 0.52, "technical": 0.36, "minimal": 0.92, "experimental": 0.22}),
    ("boxy jacket", "minimal", {"craft": 0.46, "technical": 0.48, "minimal": 0.88, "experimental": 0.30}),
    ("modular shell", "technical", {"craft": 0.08, "technical": 0.96, "minimal": 0.38, "experimental": 0.60}),
    ("utility-tailored line", "technical", {"craft": 0.18, "technical": 0.90, "minimal": 0.52, "experimental": 0.52}),
    ("active structured fit", "technical", {"craft": 0.12, "technical": 0.88, "minimal": 0.48, "experimental": 0.56}),
    ("architectural line", "experimental", {"craft": 0.14, "technical": 0.64, "minimal": 0.44, "experimental": 0.96}),
    ("directional volume", "experimental", {"craft": 0.08, "technical": 0.56, "minimal": 0.34, "experimental": 0.98}),
    ("concept volume", "experimental", {"craft": 0.06, "technical": 0.52, "minimal": 0.28, "experimental": 0.99}),
    ("elongated precision", "luxury", {"craft": 0.58, "technical": 0.32, "minimal": 0.72, "experimental": 0.44}),
]


def _clip(v: float, lo: int = 0, hi: int = 100) -> int:
    return int(max(lo, min(hi, round(v))))


def _prediction_cache_enabled() -> bool:
    return os.getenv("FASHION_ORACLE_PREDICTION_CACHE", "1").strip().lower() not in ("0", "false", "no", "off")


def _cache_get(year: int, scenario: str) -> FashionPrediction | None:
    if not _prediction_cache_enabled():
        return None
    key = (year, PREDICTION_ENGINE_VERSION, scenario)
    if key not in _prediction_lru:
        return None
    _prediction_lru.move_to_end(key)
    base = _prediction_lru[key]
    _log.info("PREDICT_CACHE_HIT year=%s engine=%s total_ms=0", year, PREDICTION_ENGINE_VERSION)
    return base.model_copy(update={"from_cache": True, "prediction_engine_version": PREDICTION_ENGINE_VERSION})


def _cache_put(year: int, scenario: str, pred: FashionPrediction) -> None:
    if not _prediction_cache_enabled():
        return
    key = (year, PREDICTION_ENGINE_VERSION, scenario)
    _prediction_lru[key] = pred.model_copy(update={"from_cache": False, "prediction_engine_version": PREDICTION_ENGINE_VERSION})
    _prediction_lru.move_to_end(key)
    while len(_prediction_lru) > _MAX_CACHE_ENTRIES:
        _prediction_lru.popitem(last=False)


def _logistic(t: float, k: float, t0: float) -> float:
    return 1.0 / (1.0 + math.exp(-k * (t - t0)))


def _gaussian(t: float, mu: float, sigma: float) -> float:
    return math.exp(-((t - mu) ** 2) / (2.0 * sigma * sigma))


def _exp_peak(year: float, pivot: float, speed: float) -> float:
    """Forme gaussienne demandée: amplitude * exp(-((year - pivot)/speed)^2)."""
    return math.exp(-((year - pivot) / max(0.1, speed)) ** 2)


def _non_linear_curve(year: int, *, shape: str, base: float, amplitude: float, pivot: float, speed: float) -> float:
    y = float(year)
    if shape == "gaussian":
        raw = _exp_peak(y, pivot, speed)
    elif shape == "long_rise":
        # Croissance non-linéaire progressive puis plateau.
        raw = 1.0 - math.exp(-max(0.0, y - (pivot - 9.0)) / max(1.0, speed))
    elif shape == "decline":
        # Déclin non-linéaire depuis un niveau initial élevé.
        raw = math.exp(-max(0.0, y - pivot) / max(1.0, speed))
    else:
        raw = _exp_peak(y, pivot, speed)
    return base + amplitude * max(0.0, min(1.0, raw))


def _scenario_name(raw: str | None) -> str:
    if not raw:
        return "balanced"
    key = raw.strip().lower()
    if key in _SCENARIO_CONFIG:
        return key
    return "balanced"


def _base_trend_intensities(year: int) -> dict[str, float]:
    intensities: dict[str, float] = {}
    for trend, cfg in _TREND_CURVE_CONFIG.items():
        intensities[trend] = _non_linear_curve(
            year,
            shape=str(cfg["shape"]),
            base=float(cfg["base"]),
            amplitude=float(cfg["amplitude"]),
            pivot=float(cfg["pivot"]),
            speed=float(cfg["speed"]),
        )
    return intensities


def _scenario_jitter(year: int, trend: str, volatility: float) -> float:
    # Jitter déterministe (pas d'aléatoire runtime), dépendant de l'année et de la tendance.
    h = (year * 1009 + sum(ord(c) for c in trend) * 313) % 9973
    wave = math.sin(h * 0.071) + 0.7 * math.cos(h * 0.049)
    return wave * 2.6 * volatility


def _shock_effects(year: int, scenario: str) -> tuple[dict[str, float], list[dict[str, object]]]:
    conf = _SCENARIO_CONFIG[scenario]
    effects: dict[str, float] = {k: 0.0 for k in _TREND_FAMILIES}
    events_for_year: list[dict[str, object]] = []
    for evt in _TREND_SHOCK_EVENTS:
        if int(evt["year"]) != year:
            continue
        ev_eff = dict(evt["effects"])  # type: ignore[arg-type]
        scaled: dict[str, float] = {}
        for trend in _TREND_FAMILIES:
            delta = float(ev_eff.get(trend, 0.0)) * conf["shock_intensity"]
            effects[trend] += delta
            if abs(delta) >= 0.5:
                scaled[trend] = round(delta, 2)
        events_for_year.append({"year": year, "label": str(evt["label"]), "effects": scaled})
    return effects, events_for_year


def _apply_conflicts(base: dict[str, float], scenario: str) -> tuple[dict[str, float], list[str]]:
    conf_gain = _SCENARIO_CONFIG[scenario]["conflict_intensity"]
    adjusted = dict(base)
    notes: list[str] = []
    for src in _TREND_FAMILIES:
        src_norm = max(0.0, min(1.0, base[src] / 100.0))
        for dst, weight in _TREND_CONFLICT_MATRIX.get(src, {}).items():
            delta = src_norm * float(weight) * 16.0 * conf_gain
            adjusted[dst] += delta
            if abs(delta) >= 1.25:
                direction = "suppresses" if delta < 0 else "supports"
                notes.append(f"{src} {direction} {dst} ({delta:+.1f})")
    return adjusted, notes[:6]


def _trend_intensities(year: int, scenario: str = "balanced") -> tuple[dict[str, float], list[dict[str, object]], list[str], float]:
    """Courbes non-linéaires + conflit inter-trend + shocks + volatilité scénario."""
    scenario_key = _scenario_name(scenario)
    conf = _SCENARIO_CONFIG[scenario_key]
    base = _base_trend_intensities(year)
    shocks, events = _shock_effects(year, scenario_key)
    with_shocks = {k: base[k] + shocks[k] for k in _TREND_FAMILIES}
    conflicted, conflict_notes = _apply_conflicts(with_shocks, scenario_key)
    out: dict[str, float] = {}
    for trend in _TREND_FAMILIES:
        jitter = _scenario_jitter(year, trend, conf["volatility"])
        out[trend] = max(0.0, min(100.0, conflicted[trend] + jitter))
    volatility_proxy = conf["volatility"] * 100.0 + sum(abs(_scenario_jitter(year, t, conf["volatility"])) for t in _TREND_FAMILIES) / 5.0
    return out, events, conflict_notes, volatility_proxy


def _phase_from_window(prev_v: float, curr_v: float, next_v: float) -> str:
    momentum = curr_v - prev_v
    forward = next_v - curr_v
    flat = abs(momentum) <= 1.1 and abs(forward) <= 1.1
    if curr_v >= 90.0 and momentum >= 0.0 and forward < -0.8:
        return "Peak"
    if curr_v <= 22.0 and momentum <= 0.6 and forward > 0.8:
        return "Bottom"
    if flat:
        return "Stable"
    if momentum > 1.0 or (momentum >= 0 and forward > 0.8):
        return "Rising"
    if momentum < -1.0 or (momentum <= 0 and forward < -0.8):
        return "Declining"
    return "Stable"


def _trend_dynamics(year: int, scenario: str = "balanced") -> dict[str, dict[str, float | str]]:
    prev_t, _, _, _ = _trend_intensities(max(2026, year - 1), scenario)
    curr_t, _, _, _ = _trend_intensities(year, scenario)
    next_t, _, _, _ = _trend_intensities(year + 1, scenario)
    out: dict[str, dict[str, float | str]] = {}
    for trend in _TREND_FAMILIES:
        prev_v = prev_t[trend]
        curr_v = curr_t[trend]
        next_v = next_t[trend]
        out[trend] = {
            "prev": prev_v,
            "current": curr_v,
            "next": next_v,
            "momentum": curr_v - prev_v,
            "phase": _phase_from_window(prev_v, curr_v, next_v),
        }
    stable = [k for k, v in out.items() if v["phase"] == "Stable"]
    if len(stable) > 2:
        # Réduction stricte du "Stable" : on conserve les 2 plus plats, les autres basculent selon la pente.
        flat_rank = sorted(
            stable,
            key=lambda k: abs(float(out[k]["momentum"])) + abs(float(out[k]["next"]) - float(out[k]["current"])),
        )
        for trend in flat_rank[2:]:
            slope = float(out[trend]["next"]) - float(out[trend]["current"])
            out[trend]["phase"] = "Rising" if slope >= 0 else "Declining"
    rising_count = sum(1 for v in out.values() if v["phase"] == "Rising")
    declining_count = sum(1 for v in out.values() if v["phase"] == "Declining")
    if rising_count > 0 and declining_count == 0:
        weakest = min(_TREND_FAMILIES, key=lambda k: float(out[k]["momentum"]))
        out[weakest]["phase"] = "Declining"
    return out


def _year_profile(year: int, trends: dict[str, float]) -> dict[str, int]:
    t = float(year - 2025)
    artisan = trends["artisan-leather"]
    romantic = trends["romantic-fluid"]
    athlux = trends["athlux-utility"]
    minimal = trends["minimal-tailoring"]
    conceptual = trends["conceptual-futurism"]

    luxury = 74.0 - 0.75 * t + 0.22 * artisan + 0.10 * minimal - 0.14 * conceptual
    technical = 18.0 + 0.30 * athlux + 0.26 * conceptual + 0.08 * minimal
    minimal_idx = 30.0 + 0.46 * minimal + 0.10 * athlux
    craft = 66.0 - 0.55 * t + 0.34 * artisan
    experimental = 10.0 + 0.55 * conceptual + 0.12 * romantic
    commercial = 82.0 - 0.65 * t + 0.24 * athlux + 0.14 * minimal - 0.16 * conceptual
    risk = 18.0 + 0.44 * conceptual + 0.16 * romantic + 0.06 * t

    # warm -> cool -> neutral progression (dépend des forces, pas mapping fixe par année)
    warm_pressure = 0.42 * artisan + 0.24 * craft + 0.18 * luxury
    cool_pressure = 0.46 * athlux + 0.24 * conceptual + 0.16 * technical
    neutral_pressure = 0.36 * minimal + 0.18 * romantic
    palette_temp = 50.0 + 0.18 * (warm_pressure - cool_pressure) - 0.06 * (neutral_pressure - 40.0)

    return {
        "luxury_index": _clip(luxury),
        "technical_index": _clip(technical),
        "minimal_index": _clip(minimal_idx),
        "craft_index": _clip(craft),
        "experimental_index": _clip(experimental),
        "commercial_index": _clip(commercial),
        "risk_index": _clip(risk),
        "palette_temperature": _clip(palette_temp),
    }


@dataclass
class Candidate:
    concept_id: str
    style: str
    style_family: str
    palette: str
    palette_family: str
    material: str
    material_family: str
    silhouette: str
    silhouette_family: str
    trend_fit_score: int = 0
    innovation_score: int = 0
    desirability_score: int = 0
    feasibility_score: int = 0
    timing_score: int = 0
    differentiation_score: int = 0
    trend_timing_score: int = 0
    trend_maturity_score: int = 0
    market_score: int = 0
    saturation_risk: str = "Medium"
    final_score: float = 0.0
    reasoning: str = ""
    reject_reason: str = ""
    duplicate_penalty: float = 0.0


def _palette_axes(profile: dict[str, int]) -> dict[str, float]:
    warm = max(0.0, min(1.0, profile["palette_temperature"] / 100.0))
    cool = 1.0 - warm
    neutral = max(0.0, 1.0 - abs(warm - 0.5) * 1.9)
    exp = max(0.0, min(1.0, profile["experimental_index"] / 100.0))
    return {"warm": warm, "cool": cool, "neutral": neutral, "exp": exp}


def _material_axes(profile: dict[str, int]) -> dict[str, float]:
    return {
        "craft": profile["craft_index"] / 100.0,
        "technical": profile["technical_index"] / 100.0,
        "minimal": profile["minimal_index"] / 100.0,
        "experimental": profile["experimental_index"] / 100.0,
    }


def _score_named_vectors(catalog: list[tuple[str, str, dict[str, float]]], axes: dict[str, float]) -> list[tuple[float, str, str]]:
    scored: list[tuple[float, str, str]] = []
    for name, family, vec in catalog:
        s = 0.0
        for k, w in vec.items():
            s += w * axes.get(k, 0.0)
        scored.append((s, name, family))
    scored.sort(reverse=True, key=lambda x: x[0])
    return scored


def _generate_candidate_pool(year: int, profile: dict[str, int], trends: dict[str, float], target_count: int = 14) -> list[Candidate]:
    style_list = list(_STYLE_VECTORS.keys())
    palette_rank = _score_named_vectors(_PALETTES, _palette_axes(profile))
    material_rank = _score_named_vectors(_MATERIALS, _material_axes(profile))
    silhouette_rank = _score_named_vectors(_SILHOUETTES, _material_axes(profile))
    count = max(12, min(15, target_count))
    out: list[Candidate] = []
    dominant_order = sorted(trends.items(), key=lambda kv: kv[1], reverse=True)
    dominant_styles = [k for k, _ in dominant_order]
    for i in range(count):
        style = style_list[(year * 5 + i * 3 + 1) % len(style_list)]
        if i < len(dominant_styles):
            style = dominant_styles[i % len(dominant_styles)]
        p_name = palette_rank[(i * 2 + year) % len(palette_rank)][1]
        p_family = palette_rank[(i * 2 + year) % len(palette_rank)][2]
        m_name = material_rank[(i * 3 + year + 1) % len(material_rank)][1]
        m_family = material_rank[(i * 3 + year + 1) % len(material_rank)][2]
        s_name = silhouette_rank[(i * 4 + year + 2) % len(silhouette_rank)][1]
        s_family = silhouette_rank[(i * 4 + year + 2) % len(silhouette_rank)][2]
        out.append(
            Candidate(
                concept_id=f"Y{year}-C{i+1}",
                style=style,
                style_family=str(_STYLE_VECTORS[style]["family"]),
                palette=p_name,
                palette_family=p_family,
                material=m_name,
                material_family=m_family,
                silhouette=s_name,
                silhouette_family=s_family,
            )
        )
    return out


def _evaluate_candidates(year: int, profile: dict[str, int], trends: dict[str, float], candidates: list[Candidate], scenario: str = "balanced") -> None:
    style_freq: dict[str, int] = {}
    palette_freq: dict[str, int] = {}
    material_freq: dict[str, int] = {}
    for c in candidates:
        style_freq[c.style] = style_freq.get(c.style, 0) + 1
        palette_freq[c.palette] = palette_freq.get(c.palette, 0) + 1
        material_freq[c.material] = material_freq.get(c.material, 0) + 1

    t = float(year - 2025)
    for c in candidates:
        w = _STYLE_VECTORS[c.style]["w"]
        trend_fit = 0.0
        for fam in _TREND_FAMILIES:
            trend_fit += float(w.get(fam, 0.0)) * trends[fam]
        trend_fit_score = _clip(trend_fit)

        # timing via slope local réel
        tpf, _, _, _ = _trend_intensities(year + 1, scenario)
        tmf, _, _, _ = _trend_intensities(max(2026, year - 1), scenario)
        tp = tpf[c.style if c.style in _TREND_FAMILIES else "minimal-tailoring"]
        tm = tmf[c.style if c.style in _TREND_FAMILIES else "minimal-tailoring"]
        timing = _clip(50.0 + (tp - tm) * 1.8 + 0.20 * profile["technical_index"])

        innovation = _clip(
            0.42 * profile["experimental_index"]
            + 0.24 * trend_fit_score
            + 8.0 * (1.0 if c.material_family == "experimental" else 0.0)
            + 6.0 * (1.0 if c.silhouette_family == "experimental" else 0.0)
        )
        desirability = _clip(0.44 * profile["commercial_index"] + 0.34 * trend_fit_score + 0.22 * timing)
        feasibility = _clip(
            0.52 * profile["commercial_index"]
            + 0.26 * profile["craft_index"]
            + 0.22 * profile["minimal_index"]
            - max(0, innovation - 72) * 0.38
            - profile["risk_index"] * 0.12
            - max(0.0, t - 10.0) * 0.8
        )
        differentiation = _clip(
            76.0
            - 8.0 * (style_freq[c.style] - 1)
            - 5.5 * (palette_freq[c.palette] - 1)
            - 5.5 * (material_freq[c.material] - 1)
            + 8.0 * (1.0 if c.palette_family == "accent" else 0.0)
            + 6.0 * (1.0 if c.material_family == "experimental" else 0.0)
        )
        market = _clip(
            0.48 * desirability
            + 0.20 * trend_fit_score
            + 0.18 * timing
            + 0.14 * profile["luxury_index"]
            - max(0, differentiation - 84) * 0.12
        )
        sat_raw = _clip(0.44 * market + 0.34 * (style_freq[c.style] * 14) + 0.22 * (palette_freq[c.palette] * 10))
        saturation = "High" if sat_raw >= 74 else ("Medium" if sat_raw >= 48 else "Low")

        maturity = _clip(0.50 * timing + 0.30 * market + 0.20 * feasibility)
        duplicate_penalty = (style_freq[c.style] - 1) * 6.0 + (palette_freq[c.palette] - 1) * 3.2 + (material_freq[c.material] - 1) * 3.2

        final_score = (
            0.22 * trend_fit_score
            + 0.18 * innovation
            + 0.20 * desirability
            + 0.20 * feasibility
            + 0.10 * timing
            + 0.10 * differentiation
            - duplicate_penalty
        )

        c.trend_fit_score = trend_fit_score
        c.innovation_score = innovation
        c.desirability_score = desirability
        c.feasibility_score = feasibility
        c.timing_score = timing
        c.differentiation_score = differentiation
        c.market_score = market
        c.trend_timing_score = timing
        c.trend_maturity_score = maturity
        c.saturation_risk = saturation
        c.duplicate_penalty = duplicate_penalty
        c.final_score = final_score

        if market >= 78 and innovation >= 70:
            c.reasoning = "Selected for optimal balance between market demand and innovation"
        elif differentiation >= 78 and feasibility <= 68:
            c.reasoning = "High differentiation but moderate feasibility in this year context"
        elif c.style_family == "technical" and profile["technical_index"] >= 60:
            c.reasoning = "Strong alignment with rising technical luxury signals"
        elif saturation == "High":
            c.reasoning = "Commercially strong but saturation risk needs controlled rollout"
        else:
            c.reasoning = "Balanced strategic profile for current year signals"


def _select_top_diverse(candidates: list[Candidate], top_n: int = 3) -> tuple[list[Candidate], list[str]]:
    ranked = sorted(candidates, key=lambda c: c.final_score, reverse=True)
    selected: list[Candidate] = []
    rejected: list[str] = []
    used_styles: set[str] = set()
    used_palettes: set[str] = set()
    used_materials: set[str] = set()

    for c in ranked:
        if c.style in used_styles:
            rejected.append(f"Rejected: {c.style} -> duplicate style")
            continue
        if c.palette in used_palettes:
            rejected.append(f"Rejected: {c.style} -> duplicate palette")
            continue
        if c.material in used_materials:
            rejected.append(f"Rejected: {c.style} -> duplicate material")
            continue
        if c.saturation_risk == "High" and c.market_score < 72:
            rejected.append(f"Rejected: {c.style} -> too saturated this year")
            continue
        selected.append(c)
        used_styles.add(c.style)
        used_palettes.add(c.palette)
        used_materials.add(c.material)
        if len(selected) >= top_n:
            break

    if len(selected) < top_n:
        for c in ranked:
            if c in selected:
                continue
            selected.append(c)
            if len(selected) >= top_n:
                break
    return selected[:top_n], rejected[:8]


def _profile_to_global_summary(profile: dict[str, int], selected: list[Candidate], trends: dict[str, float]) -> dict[str, str]:
    dominant_trend = max(trends.items(), key=lambda kv: kv[1])[0]
    if dominant_trend == "athlux-utility":
        dominant_direction = "Technical Luxury"
    elif dominant_trend == "conceptual-futurism":
        dominant_direction = "Conceptual Experimental"
    elif dominant_trend == "minimal-tailoring":
        dominant_direction = "Minimal Precision"
    elif dominant_trend == "artisan-leather":
        dominant_direction = "Premium Craft"
    else:
        dominant_direction = "Fluid Romantic Direction"

    strongest_signal = f"Tech {profile['technical_index']} / Exp {profile['experimental_index']} / Risk {profile['risk_index']}"
    market_posture = "Offensive expansion" if profile["commercial_index"] >= 72 else ("Balanced selective growth" if profile["commercial_index"] >= 58 else "Niche premium defense")
    risk_level = "High" if profile["risk_index"] >= 68 else ("Medium" if profile["risk_index"] >= 44 else "Low")
    recommended_capsule = " | ".join(f"{c.style} + {c.material}" for c in selected)
    return {
        "dominant_direction": dominant_direction,
        "strongest_signal": strongest_signal,
        "market_posture": market_posture,
        "risk_level": risk_level,
        "recommended_capsule": recommended_capsule,
    }


def _trajectory(
    year: int,
    profile: dict[str, int],
    *,
    scenario: str = "balanced",
    shock_events_current: list[dict[str, object]] | None = None,
    conflict_notes_current: list[str] | None = None,
) -> dict[str, object]:
    years = [year - 2, year - 1, year, year + 1, year + 2]
    artisan_curve: list[int] = []
    romantic_curve: list[int] = []
    athlux_curve: list[int] = []
    minimal_curve: list[int] = []
    conceptual_curve: list[int] = []
    market_signal: list[int] = []
    risk_curve: list[int] = []
    dominant_by_year: list[str] = []
    for y in years:
        ti, _, _, _ = _trend_intensities(y, scenario)
        yp = _year_profile(y, ti)
        artisan_curve.append(_clip(ti["artisan-leather"]))
        romantic_curve.append(_clip(ti["romantic-fluid"]))
        athlux_curve.append(_clip(ti["athlux-utility"]))
        minimal_curve.append(_clip(ti["minimal-tailoring"]))
        conceptual_curve.append(_clip(ti["conceptual-futurism"]))
        market_signal.append(_clip(yp["commercial_index"]))
        risk_curve.append(_clip(yp["risk_index"]))
        dominant_by_year.append(max(ti.items(), key=lambda kv: kv[1])[0])

    dynamics = _trend_dynamics(year, scenario)
    phase_map = {k: str(v["phase"]) for k, v in dynamics.items()}
    momentum_map = {k: round(float(v["momentum"]), 2) for k, v in dynamics.items()}
    dominance_shifts: list[str] = []
    for idx in range(1, len(dominant_by_year)):
        prev_dom = dominant_by_year[idx - 1]
        curr_dom = dominant_by_year[idx]
        if curr_dom != prev_dom:
            dominance_shifts.append(f"{years[idx - 1]}->{years[idx]}: {prev_dom} -> {curr_dom}")

    overtakes: list[str] = []
    curve_map = {
        "artisan-leather": artisan_curve,
        "romantic-fluid": romantic_curve,
        "athlux-utility": athlux_curve,
        "minimal-tailoring": minimal_curve,
        "conceptual-futurism": conceptual_curve,
    }
    for i in range(1, len(years)):
        for a in _TREND_FAMILIES:
            for b in _TREND_FAMILIES:
                if a >= b:
                    continue
                prev_diff = curve_map[a][i - 1] - curve_map[b][i - 1]
                curr_diff = curve_map[a][i] - curve_map[b][i]
                if prev_diff == 0:
                    continue
                if (prev_diff < 0 and curr_diff > 0) or (prev_diff > 0 and curr_diff < 0):
                    winner = a if curr_diff > 0 else b
                    loser = b if winner == a else a
                    overtakes.append(f"{years[i]}: {winner} overtakes {loser}")

    return {
        "years": years,
        "artisan_leather_curve": artisan_curve,
        "romantic_fluid_curve": romantic_curve,
        "athlux_utility_curve": athlux_curve,
        "minimal_tailoring_curve": minimal_curve,
        "conceptual_futurism_curve": conceptual_curve,
        "style_intensity": athlux_curve,
        "palette_cooling_direction": [_clip(100 - _year_profile(y, _trend_intensities(y, scenario)[0])["palette_temperature"]) for y in years],
        "material_sophistication": conceptual_curve,
        "commercial_risk_curve": risk_curve,
        "market_signal": market_signal,
        "trend_phases": phase_map,
        "trend_momentum": momentum_map,
        "dominant_by_year": dominant_by_year,
        "dominant_trend": dominant_by_year[len(dominant_by_year) // 2],
        "next_dominant_trend": dominant_by_year[min(len(dominant_by_year) // 2 + 1, len(dominant_by_year) - 1)],
        "dominance_shifts": dominance_shifts,
        "overtakes": overtakes[:8],
        "shock_events": shock_events_current or [],
        "conflict_notes": conflict_notes_current or [],
        "scenario_mode": scenario,
    }


def _evolution_text(year: int, profile: dict[str, int], trends: dict[str, float]) -> str:
    prev = year - 1
    rising = max(trends.items(), key=lambda kv: kv[1])[0]
    declining = min(trends.items(), key=lambda kv: kv[1])[0]
    return (
        f"Compared to {prev}, rising={rising}, declining={declining}, "
        f"saturation risk={profile['risk_index']} and technical pull={profile['technical_index']}."
    )


def compute_fashion_prediction(year: int, *, bypass_cache: bool = False, scenario: str = "balanced") -> FashionPrediction:
    from app.text_runtime import log_text_router_config

    log_text_router_config()
    scenario_mode = _scenario_name(scenario)
    t0 = time.perf_counter()
    if not bypass_cache:
        cached = _cache_get(year, scenario_mode)
        if cached is not None:
            return cached

    phase = forecast_phase_for_year(year)
    eligible = year >= FORECAST_MIN_YEAR
    trends, shock_events, conflict_notes, volatility_proxy = _trend_intensities(year, scenario_mode)
    profile = _year_profile(year, trends)
    candidates = _generate_candidate_pool(year, profile, trends, target_count=14)
    _evaluate_candidates(year, profile, trends, candidates, scenario_mode)
    selected, rejected = _select_top_diverse(candidates, top_n=3)
    global_summary = _profile_to_global_summary(profile, selected, trends)

    _log.info(
        "PREDICT_PROFILE year=%s luxury=%s technical=%s minimal=%s craft=%s experimental=%s commercial=%s risk=%s palette_temp=%s",
        year,
        profile["luxury_index"],
        profile["technical_index"],
        profile["minimal_index"],
        profile["craft_index"],
        profile["experimental_index"],
        profile["commercial_index"],
        profile["risk_index"],
        profile["palette_temperature"],
    )
    _log.info("PREDICT_CANDIDATES year=%s total=%s", year, len(candidates))
    for c in sorted(candidates, key=lambda x: x.final_score, reverse=True):
        _log.info(
            "CAND year=%s id=%s style=%s pal=%s mat=%s sil=%s fit=%s innov=%s desir=%s feas=%s timing=%s diff=%s sat=%s dup_pen=%.1f final=%.2f",
            year,
            c.concept_id,
            c.style,
            c.palette,
            c.material,
            c.silhouette,
            c.trend_fit_score,
            c.innovation_score,
            c.desirability_score,
            c.feasibility_score,
            c.timing_score,
            c.differentiation_score,
            c.saturation_risk,
            c.duplicate_penalty,
            c.final_score,
        )
    _log.info("PREDICT_SELECTED year=%s selected=%s rejected=%s", year, [f"{c.style}/{c.palette}/{c.material}" for c in selected], rejected)

    concepts_projection: list[ForecastConceptInsight] = []
    concepts_preview: list[ConceptPreview] = []
    for idx, c in enumerate(selected, start=1):
        name = "Concept 1 - Veste premium" if idx == 1 else ("Concept 2 - Veste technique" if idx == 2 else "Concept 3 - Ceinture luxe")
        concepts_projection.append(
            ForecastConceptInsight(
                concept_index=idx,
                product_name=name,
                style=c.style,
                palette=c.palette,
                material=c.material,
                silhouette=c.silhouette,
                innovation_score=c.innovation_score,
                market_score=c.market_score,
                feasibility_score=c.feasibility_score,
                trend_timing_score=c.timing_score,
                trend_maturity_score=c.trend_maturity_score,
                differentiation_score=c.differentiation_score,
                saturation_risk=c.saturation_risk,
                strategic_explanation=c.reasoning,
                evolution_vs_previous=_evolution_text(year, profile, trends),
                direction_badge="Core Direction" if idx == 1 else ("Growth Lever" if idx == 2 else "Differentiator"),
            )
        )
        concepts_preview.append(
            ConceptPreview(
                concept_index=idx,
                category_label_fr=name,
                product_type="ceinture" if idx == 3 else "veste",
                variant="luxe" if idx == 3 else ("technique" if idx == 2 else "premium"),
                style=c.style,
                palette=c.palette,
                material=c.material,
                silhouette=c.silhouette,
            )
        )

    top_styles = [c.style for c in selected]
    color_palette = [c.palette for c in selected]
    fabrics_materials = [c.material for c in selected]
    silhouettes = [c.silhouette for c in selected]
    attrs = recommend_product_attributes(top_styles[0], color_palette, fabrics_materials, silhouettes)
    # Confidence model (séparé du risque / innovation):
    # confidence = 0.45*temporal_certainty + 0.30*signal_coherence + 0.25*ranking_gap
    temporal_certainty = _clip(max(0.0, 100.0 - 6.0 * float(year - 2026)))
    avg_selected_fit = sum(c.trend_fit_score for c in selected) / max(1, len(selected))
    key = [
        float(profile["luxury_index"]),
        float(profile["technical_index"]),
        float(profile["minimal_index"]),
        float(profile["craft_index"]),
        float(profile["experimental_index"]),
        float(profile["commercial_index"]),
    ]
    pair_diffs = []
    for i in range(len(key)):
        for j in range(i + 1, len(key)):
            pair_diffs.append(abs(key[i] - key[j]))
    avg_conflict = sum(pair_diffs) / max(1, len(pair_diffs))
    signal_coherence = _clip(0.55 * avg_selected_fit + 0.45 * (100.0 - avg_conflict))
    ranked_selected = sorted(selected, key=lambda c: c.final_score, reverse=True)
    if len(ranked_selected) >= 3:
        gap12 = ranked_selected[0].final_score - ranked_selected[1].final_score
        gap23 = ranked_selected[1].final_score - ranked_selected[2].final_score
    elif len(ranked_selected) == 2:
        gap12 = ranked_selected[0].final_score - ranked_selected[1].final_score
        gap23 = gap12 * 0.5
    else:
        gap12 = 0.0
        gap23 = 0.0
    ranking_gap = _clip(50.0 + 6.0 * gap12 + 3.0 * gap23)
    uncertainty_penalty = min(28.0, 0.24 * volatility_proxy + 0.7 * float(len(shock_events)) + 0.35 * float(len(conflict_notes)))
    confidence_score = _clip(
        0.45 * temporal_certainty + 0.30 * signal_coherence + 0.25 * ranking_gap - uncertainty_penalty,
        28,
        98,
    )
    confidence = round(confidence_score / 100.0, 3)
    pred = FashionPrediction(
        year=year,
        top_styles=top_styles,
        color_palette=color_palette,
        fabrics_materials=fabrics_materials,
        silhouettes=silhouettes,
        confidence=confidence,
        similar_decades=[f"Compared benchmark {year - 1}", f"Forward benchmark {year + 2}"],
        summary=f"Year {year}: {len(candidates)} candidates evaluated in {scenario_mode} mode; top 3 retained via competitive multi-factor scoring.",
        raw_scores={
            "luxury_index": profile["luxury_index"],
            "technical_index": profile["technical_index"],
            "minimal_index": profile["minimal_index"],
            "craft_index": profile["craft_index"],
            "experimental_index": profile["experimental_index"],
            "commercial_index": profile["commercial_index"],
            "risk_index": profile["risk_index"],
            "selected_avg_final_score": round(sum(c.final_score for c in selected) / max(1, len(selected)), 3),
            "temporal_certainty": temporal_certainty,
            "signal_coherence": signal_coherence,
            "ranking_gap": ranking_gap,
            "confidence_score": confidence_score,
            "scenario_mode": scenario_mode,
            "volatility_proxy": round(volatility_proxy, 2),
            "uncertainty_penalty": round(uncertainty_penalty, 2),
            "shock_events_count": len(shock_events),
            "conflict_notes_count": len(conflict_notes),
        },
        recommended_product_attributes=attrs,
        chart_trend_base64=None,
        chart_cycle_base64=None,
        generated_visual_prompt=None,
        generated_visual_url=None,
        inference_mode=f"forecast-cockpit-competitive-{scenario_mode}",
        forecast_eligible=eligible,
        forecast_phase=phase,
        business_notice=None if eligible else f"Prévision cockpit recommandée à partir de {FORECAST_MIN_YEAR}.",
        from_cache=False,
        prediction_engine_version=PREDICTION_ENGINE_VERSION,
        concepts_preview=concepts_preview,
        concepts_projection=concepts_projection,
        direction_title=f"Signal dominant IA - {year}",
        dominant_shift=global_summary["dominant_direction"],
        consumer_signal=global_summary["strongest_signal"],
        strategic_opportunity=global_summary["recommended_capsule"],
        year_over_year_evolution=_evolution_text(year, profile, trends),
        trend_trajectory=_trajectory(
            year,
            profile,
            scenario=scenario_mode,
            shock_events_current=shock_events,
            conflict_notes_current=conflict_notes,
        ),
        dominant_direction=global_summary["dominant_direction"],
        strongest_signal=global_summary["strongest_signal"],
        market_posture=global_summary["market_posture"],
        risk_level=global_summary["risk_level"],
        recommended_capsule=global_summary["recommended_capsule"],
        rejected_concepts=rejected,
    )
    _cache_put(year, scenario_mode, pred)
    _log.info(
        "PREDICT_COMPUTE_OK year=%s total_ms=%.0f top_styles=%s palette=%s",
        year,
        (time.perf_counter() - t0) * 1000.0,
        pred.top_styles,
        pred.color_palette,
    )
    _log.info(
        "CONF_DEBUG year=%s temporal_certainty=%s signal_coherence=%s ranking_gap=%s confidence_score=%s",
        year,
        temporal_certainty,
        signal_coherence,
        ranking_gap,
        confidence_score,
    )
    return pred
