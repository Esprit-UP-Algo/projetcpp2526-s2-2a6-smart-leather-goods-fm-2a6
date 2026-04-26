"""Scores décisionnels 0–100 (efficacité, risque, marge, viabilité)."""
from __future__ import annotations

from typing import Any, Dict, List

from app.services.advanced_cost.simulation_engine import SimInputs, selling_price_for_margin


def compute_scores(
    inp: SimInputs,
    balanced_total: float,
    balanced_breakdown: Dict[str, float],
    scenarios: List[Dict[str, Any]],
    risk_exposure: float,
) -> Dict[str, float]:
    price = selling_price_for_margin(balanced_total, inp.target_margin_pct)
    margin = (100.0 * (price - balanced_total) / price) if price > 0 else 0.0
    mat_share = (balanced_breakdown.get("matiere", 0) / balanced_total) if balanced_total > 0 else 0.0

    # Efficacité : plus la part valeur ajoutée (hors matière) est élevée, mieux c'est
    va_share = 1.0 - mat_share
    cost_efficiency = min(100.0, max(15.0, 42.0 + 58.0 * va_share - 12.0 * mat_share))

    # Marge : autour de la cible
    gap = abs(margin - inp.target_margin_pct)
    margin_health = min(100.0, max(10.0, 100.0 - 2.2 * gap - 5.0 * max(0, 35 - margin)))

    # Viabilité : cohérence des 3 scénarios (écart modéré = mieux)
    costs = [s["total_cost_per_unit"] for s in scenarios]
    spread = (max(costs) - min(costs)) / max(1e-6, balanced_total)
    viability = min(100.0, max(20.0, 92.0 - 55.0 * spread - 0.35 * risk_exposure))

    return {
        "cost_efficiency": round(cost_efficiency, 2),
        "risk_exposure": round(risk_exposure, 2),
        "margin_health": round(margin_health, 2),
        "product_viability": round(viability, 2),
    }


def confidence_score(scenarios: List[Dict[str, Any]], inp: SimInputs) -> float:
    """Score de confiance du forecast (données + stabilité inter-scénarios)."""
    costs = [s["total_cost_per_unit"] for s in scenarios]
    spread = (max(costs) - min(costs)) / max(1e-6, costs[1]) if len(costs) > 2 else 0.1
    base = 78.0
    base -= min(25.0, spread * 120.0)
    if inp.labor_hours <= 0 or inp.material_unit_cost <= 0:
        base -= 18.0
    if inp.supplier_reliability_score < 0.45:
        base -= 10.0
    return round(max(32.0, min(96.0, base)), 2)
