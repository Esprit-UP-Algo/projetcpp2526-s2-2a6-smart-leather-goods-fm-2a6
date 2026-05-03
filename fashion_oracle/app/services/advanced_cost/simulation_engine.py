"""Moteur de simulation multi-scénarios (éco / équilibré / premium) — logique pondérée métier."""
from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Any, Dict, List, Tuple


def _norm(s: str) -> str:
    return (s or "").strip().lower()


def _risk_rank(s: str) -> int:
    m = _norm(s)
    if m in ("low", "faible", "bas"):
        return 0
    if m in ("high", "eleve", "élevé", "haut"):
        return 2
    return 1


def _vol_rank(s: str) -> int:
    return _risk_rank(s)


def _urgency_mult(s: str) -> float:
    m = _norm(s)
    if m in ("critical", "critique", "crit"):
        return 1.28
    if m in ("rush", "urgent", "urgence"):
        return 1.14
    return 1.0


def _finishing_mult(s: str) -> float:
    m = _norm(s)
    if m in ("luxe", "luxury"):
        return 1.18
    if m in ("premium",):
        return 1.08
    return 1.0


def _packaging_slice(base: float, level: str) -> float:
    m = _norm(level)
    factors = {"basic": 0.55, "economy": 0.55, "standard": 1.0, "premium": 1.55, "luxe": 2.1}
    return base * factors.get(m, 1.0)


def _sustainability_premium(direct_core: float, level: str) -> float:
    m = _norm(level)
    if m in ("max", "maximum", "carbon_neutral"):
        return direct_core * 0.045
    if m in ("elevated", "eleve", "élevé", "high"):
        return direct_core * 0.022
    return direct_core * 0.008


def _branding_premium(direct_core: float, level: str) -> float:
    m = _norm(level)
    if m in ("high", "haut", "fort"):
        return direct_core * 0.035
    if m in ("medium", "moyen"):
        return direct_core * 0.018
    return direct_core * 0.006


def _volume_efficiency(volume: int) -> float:
    v = max(1, int(volume))
    return 1.0 - min(0.12, 0.018 * math.log10(max(v, 10)))


def _quality_labor_mult(level: str) -> float:
    m = _norm(level)
    if m in ("max", "excellence", "highest"):
        return 1.12
    if m in ("high", "eleve", "élevé"):
        return 1.06
    if m in ("medium", "moyen"):
        return 1.0
    return 0.96


@dataclass
class SimInputs:
    material_unit_cost: float
    material_quantity_per_unit: float
    labor_hours: float
    labor_hourly_rate: float
    labor_charges_sociales_pct: float
    finishing_level: str
    packaging_level: str
    transport_cost_per_unit: float
    supplier_risk: str
    market_volatility: str
    production_volume: int
    urgency_level: str
    sustainability_level: str
    target_quality_level: str
    target_margin_pct: float
    supplier_reliability_score: float
    defect_waste_rate_pct: float
    overhead_rate_pct: float
    currency_fluctuation_factor: float
    premium_branding_level: str
    packaging_unit_cost_floor: float


def _scenario_weights(code: str) -> Dict[str, float]:
    """Coefficients relatifs par scénario business."""
    c = _norm(code)
    if c in ("eco", "economique", "low_cost"):
        return {
            "mat_use": 0.96,
            "labor_speed": 1.05,
            "overhead": 0.94,
            "quality": 0.94,
            "risk_buffer": 0.92,
            "vol_buffer": 0.94,
        }
    if c in ("premium", "haut_de_gamme"):
        return {
            "mat_use": 1.02,
            "labor_speed": 0.94,
            "overhead": 1.05,
            "quality": 1.08,
            "risk_buffer": 1.06,
            "vol_buffer": 1.05,
        }
    return {
        "mat_use": 1.0,
        "labor_speed": 1.0,
        "overhead": 1.0,
        "quality": 1.0,
        "risk_buffer": 1.0,
        "vol_buffer": 1.0,
    }


def compute_unit_cost_for_scenario(inp: SimInputs, scenario_code: str) -> Tuple[float, Dict[str, float], str]:
    w = _scenario_weights(scenario_code)
    sc = _norm(scenario_code)
    label_fr = "Scénario équilibré"
    if sc in ("eco", "economique", "low_cost"):
        label_fr = "Scénario éco / maîtrise des coûts"
    elif sc in ("premium", "haut_de_gamme"):
        label_fr = "Scénario premium / exigence qualité"

    mat_base = (
        max(0.0, inp.material_unit_cost)
        * max(0.0, inp.material_quantity_per_unit)
        * max(0.5, inp.currency_fluctuation_factor)
        * w["mat_use"]
    )
    sr = _risk_rank(inp.supplier_risk)
    mat_risk = mat_base * (0.02 + 0.035 * sr) * w["risk_buffer"]
    rel = max(0.05, min(1.0, inp.supplier_reliability_score))
    mat_supplier = mat_base * (1.08 - rel) * 0.25

    vol_eff = _volume_efficiency(inp.production_volume)
    labor = (
        max(0.0, inp.labor_hours)
        * max(0.01, inp.labor_hourly_rate)
        * _urgency_mult(inp.urgency_level)
        * _quality_labor_mult(inp.target_quality_level)
        * w["labor_speed"]
        / max(0.75, vol_eff)
    )

    conv_core = mat_base + labor
    finish = conv_core * (_finishing_mult(inp.finishing_level) - 1.0) * w["quality"]
    pack_base = max(0.15, 0.35 + 0.0012 * math.sqrt(max(inp.production_volume, 1)))
    packaging = max(
        float(inp.packaging_unit_cost_floor),
        _packaging_slice(pack_base, inp.packaging_level) * w["quality"],
    )
    transport = max(0.0, inp.transport_cost_per_unit)

    direct = conv_core + finish + packaging + transport + mat_risk + mat_supplier
    sust = _sustainability_premium(direct, inp.sustainability_level)
    brand = _branding_premium(direct, inp.premium_branding_level)
    direct += sust + brand

    ovh = direct * (max(0.0, inp.overhead_rate_pct) / 100.0) * w["overhead"]
    waste = direct * (max(0.0, inp.defect_waste_rate_pct) / 100.0)
    vr = _vol_rank(inp.market_volatility)
    vol_stress = direct * (0.01 + 0.022 * vr) * w["vol_buffer"]

    total = direct + ovh + waste + vol_stress

    breakdown = {
        "matiere": round(mat_base + mat_risk + mat_supplier, 4),
        "main_oeuvre": round(labor + finish, 4),
        "charges_structure": round(ovh + waste + vol_stress, 4),
        "emballage_logistique": round(packaging + transport, 4),
        "risque_volatilite": round(vol_stress + mat_risk * 0.35, 4),
        "durabilite_branding": round(sust + brand, 4),
    }
    return round(total, 4), breakdown, label_fr


def selling_price_for_margin(cost: float, margin_pct: float) -> float:
    m = max(5.0, min(92.0, margin_pct))
    if cost <= 0:
        return 0.0
    return round(cost / max(0.08, 1.0 - m / 100.0), 4)


def estimate_margin_at_price(cost: float, price: float) -> float:
    if price <= 0:
        return 0.0
    return round(100.0 * (price - cost) / price, 2)


def scenario_risk_level(total: float, mat_share: float, margin: float) -> str:
    if margin < 22 or mat_share > 0.62:
        return "high"
    if margin < 38 or mat_share > 0.48:
        return "medium"
    return "low"


def positioning_text(scenario_code: str, risk: str) -> str:
    sc = _norm(scenario_code)
    if sc in ("eco", "economique", "low_cost"):
        base = "Positionnement volume / entrée de gamme : priorité à la marge brute et à la rotation."
    elif sc in ("premium", "haut_de_gamme"):
        base = "Positionnement haut de gamme : tolérance à un coût plus élevé pour sécuriser la perception qualité."
    else:
        base = "Positionnement équilibré : compromis entre coût industrialisé, qualité perçue et stabilité de marge."
    if risk == "high":
        base += " Attention : exposition au risque coût / matière élevée — surveiller le prix cible."
    return base


def run_three_scenarios(inp: SimInputs) -> List[Dict[str, Any]]:
    out: List[Dict[str, Any]] = []
    for code in ("eco", "balanced", "premium"):
        total, bd, label = compute_unit_cost_for_scenario(inp, code)
        price = selling_price_for_margin(total, inp.target_margin_pct)
        margin = estimate_margin_at_price(total, price)
        mat_share = (bd["matiere"] / total) if total > 0 else 0.0
        risk = scenario_risk_level(total, mat_share, margin)
        out.append(
            {
                "code": code if code != "balanced" else "balanced",
                "label_fr": label,
                "total_cost_per_unit": total,
                "estimated_margin_pct": margin,
                "risk_level": risk,
                "positioning_fr": positioning_text(code, risk),
                "breakdown": bd,
                "recommended_selling_price": price,
            }
        )
    return out
