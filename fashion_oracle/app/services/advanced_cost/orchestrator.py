"""Orchestration : simulation + risques + recommandations + scores + synthèse."""
from __future__ import annotations

from copy import deepcopy
from typing import Any, Dict, List

from app.config import settings
from app.models.advanced_cost import AdvancedCostOptimizeRequest, AdvancedCostSimulateRequest
from app.services.advanced_cost.llm_explanation import deterministic_executive_summary, llm_executive_summary
from app.services.advanced_cost.recommendation_engine import build_recommendations
from app.services.advanced_cost.risk_engine import analyze_risks, risk_exposure_score
from app.services.advanced_cost.scoring import compute_scores, confidence_score
from app.services.advanced_cost.simulation_engine import (
    SimInputs,
    compute_unit_cost_for_scenario,
    run_three_scenarios,
)


def _to_sim_inputs(req: AdvancedCostSimulateRequest) -> SimInputs:
    return SimInputs(
        material_unit_cost=req.material_unit_cost,
        material_quantity_per_unit=req.material_quantity_per_unit,
        labor_hours=req.labor_hours,
        labor_hourly_rate=req.labor_hourly_rate,
        labor_charges_sociales_pct=req.labor_charges_sociales_pct,
        finishing_level=req.finishing_level,
        packaging_level=req.packaging_level,
        transport_cost_per_unit=req.transport_cost_per_unit,
        supplier_risk=req.supplier_risk,
        market_volatility=req.market_volatility,
        production_volume=req.production_volume,
        urgency_level=req.urgency_level,
        sustainability_level=req.sustainability_level,
        target_quality_level=req.target_quality_level,
        target_margin_pct=req.target_margin_pct,
        supplier_reliability_score=req.supplier_reliability_score,
        defect_waste_rate_pct=req.defect_waste_rate_pct,
        overhead_rate_pct=req.overhead_rate_pct,
        currency_fluctuation_factor=req.currency_fluctuation_factor,
        premium_branding_level=req.premium_branding_level,
        packaging_unit_cost_floor=req.packaging_unit_cost_floor,
    )


async def run_advanced_simulation(req: AdvancedCostSimulateRequest) -> Dict[str, Any]:
    inp = _to_sim_inputs(req)
    scenarios = run_three_scenarios(inp)
    bal = next(s for s in scenarios if s["code"] == "balanced")
    _, bd_bal, _ = compute_unit_cost_for_scenario(inp, "balanced")

    risks = analyze_risks(inp, bal["total_cost_per_unit"], bal["breakdown"])
    rex = risk_exposure_score(risks)
    recs = build_recommendations(inp, bal["total_cost_per_unit"], bal["breakdown"])
    scores = compute_scores(inp, bal["total_cost_per_unit"], bal["breakdown"], scenarios, rex)
    conf = confidence_score(scenarios, inp)

    eco_cost = next(s for s in scenarios if s["code"] == "eco")["total_cost_per_unit"]
    prem_cost = next(s for s in scenarios if s["code"] == "premium")["total_cost_per_unit"]

    llm_payload = {
        "designation": req.designation,
        "collection": req.collection,
        "scenarios": scenarios,
        "risques": risks,
        "scores": scores,
        "recommandations": recs,
        "notes": req.notes,
    }
    summary: str
    if req.skip_llm or not settings.api_key:
        summary = deterministic_executive_summary(req.designation, scenarios, risks, scores)
        llm_used = False
    else:
        mdl = (req.cost_model or "").strip() or None
        llm_txt = await llm_executive_summary(llm_payload, model=mdl)
        summary = llm_txt or deterministic_executive_summary(req.designation, scenarios, risks, scores)
        llm_used = llm_txt is not None

    return {
        "designation": req.designation,
        "currency": "TND",
        "reference_scenario": "balanced",
        "scenarios": scenarios,
        "aggregates": {
            "optimistic_cost": eco_cost,
            "realistic_cost": bal["total_cost_per_unit"],
            "pessimistic_cost": prem_cost,
            "cost_confidence_score": conf,
        },
        "scores": scores,
        "risks": risks,
        "recommendations": recs,
        "executive_summary_fr": summary,
        "meta": {
            "engine": "advanced-cost-v1",
            "llm_summary": llm_used,
            "model": (req.cost_model or "").strip() or settings.model,
        },
    }


async def run_optimize(req: AdvancedCostOptimizeRequest) -> Dict[str, Any]:
    """Quelques variantes locales (volume, rebut, finition) pour proposer un meilleur compromis."""

    variants: List[tuple[str, AdvancedCostSimulateRequest]] = []
    base = deepcopy(req)
    variants.append(("Référence", base))

    v2 = deepcopy(req)
    v2.production_volume = min(10_000_000, int(v2.production_volume * 1.25))
    v2.notes = (v2.notes or "") + " [variante volume +25%]"
    variants.append(("Volume +25 %", v2))

    v3 = deepcopy(req)
    v3.defect_waste_rate_pct = max(0.5, v3.defect_waste_rate_pct - 0.6)
    v3.notes = (v3.notes or "") + " [variante rebut -0.6 pts]"
    variants.append(("Rebut optimisé", v3))

    v4 = deepcopy(req)
    if v4.finishing_level.lower() in ("luxe", "premium"):
        v4.finishing_level = "standard"
        v4.notes = (v4.notes or "") + " [finition ramenée standard]"
    variants.append(("Finition rationalisée", v4))

    best_label = variants[0][0]
    best_sim = await run_advanced_simulation(variants[0][1])
    best_score = float(best_sim["scores"]["product_viability"])

    for label, r in variants[1:]:
        sim = await run_advanced_simulation(r)
        sc = float(sim["scores"]["product_viability"])
        if sc > best_score:
            best_score = sc
            best_sim = sim
            best_label = label

    return {
        "best_variant_label_fr": best_label,
        "request_snapshot": {"objective": req.objective, "base": req.model_dump()},
        "simulation": best_sim,
        "alternatives_evaluated": len(variants),
    }
