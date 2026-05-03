"""Orchestration BI : Monte Carlo + scénarios + règles décision + meilleure configuration."""

from __future__ import annotations

import asyncio
from copy import deepcopy
from typing import Any, Dict, List, Optional, Tuple

from app.models.advanced_cost import AdvancedCostSimulateRequest
from app.services.advanced_cost.monte_carlo_engine import (
    monte_carlo_risk_score,
    run_monte_carlo_balanced,
    summarize_monte_carlo,
)
from app.services.advanced_cost.orchestrator import _to_sim_inputs, run_advanced_simulation
from app.services.advanced_cost.recommendation_engine import build_recommendations
from app.services.advanced_cost.risk_engine import analyze_risks, risk_exposure_score


def _rule_flags(risk_score: float, margin_pct: float, margin_threshold: float) -> Dict[str, Any]:
    return {
        "highRisk": bool(risk_score > 70.0),
        "notViableMargin": bool(margin_pct < margin_threshold),
        "marginThresholdPct": margin_threshold,
    }


def _rule_recommendations(flags: Dict[str, Any]) -> List[Dict[str, Any]]:
    out: List[Dict[str, Any]] = []
    if flags["highRisk"]:
        out.append(
            {
                "priority": 1,
                "code": "HIGH_RISK",
                "categorie": "risque",
                "titre_fr": "Risque coût élevé (Monte Carlo + exposition)",
                "impact_fr": "Envisager changement de fournisseur, avenant prix matière ou réduction des volumes engageants.",
            }
        )
    if flags["notViableMargin"]:
        out.append(
            {
                "priority": 2,
                "code": "NOT_VIABLE",
                "categorie": "marge",
                "titre_fr": "Marge inférieure au seuil de viabilité",
                "impact_fr": "Ajuster prix de vente, réduire niveau de finition/packaging, ou négocier coût matière.",
            }
        )
    if not out:
        out.append(
            {
                "priority": 5,
                "code": "OK",
                "categorie": "pilotage",
                "titre_fr": "Profil acceptable sous hypothèses actuelles",
                "impact_fr": "Maintenir le suivi fournisseur et un point mensuel sur dérive matière / rebut.",
            }
        )
    return out


def _balanced_cost(sim: Dict[str, Any]) -> float:
    return float(next(s for s in sim["scenarios"] if s["code"] == "balanced")["total_cost_per_unit"])


async def _pick_best_configuration(
    base: AdvancedCostSimulateRequest,
    base_sim: Dict[str, Any],
) -> Dict[str, Any]:
    """Grille courte : coût équilibré minimal parmi les profils à risque / marge acceptables, sinon meilleur compromis."""
    variants: List[Tuple[str, AdvancedCostSimulateRequest]] = [
        ("reference", deepcopy(base)),
    ]

    v1 = deepcopy(base)
    v1.production_volume = min(10_000_000, int(v1.production_volume * 1.2))
    v1.notes = (v1.notes or "") + " [BI: volume +20%]"
    variants.append(("volume_plus_20", v1))

    v2 = deepcopy(base)
    v2.defect_waste_rate_pct = max(0.5, v2.defect_waste_rate_pct - 0.5)
    v2.notes = (v2.notes or "") + " [BI: rebut -0.5pt]"
    variants.append(("rebut_moins_05", v2))

    v3 = deepcopy(base)
    if v3.finishing_level.lower() in ("luxe", "premium"):
        v3.finishing_level = "standard"
        v3.notes = (v3.notes or "") + " [BI: finition standard]"
    variants.append(("finition_rationalisee", v3))

    evaluated: List[Tuple[str, float, float, float, float, Dict[str, Any]]] = []
    ref_cost = _balanced_cost(base_sim)
    sc0 = base_sim["scores"]
    evaluated.append(
        (
            "reference",
            ref_cost,
            float(sc0["product_viability"]),
            float(sc0["risk_exposure"]),
            float(sc0["margin_health"]),
            base_sim,
        )
    )

    for lab, r in variants[1:]:
        sim = await run_advanced_simulation(r)
        sc = sim["scores"]
        evaluated.append(
            (
                lab,
                _balanced_cost(sim),
                float(sc["product_viability"]),
                float(sc["risk_exposure"]),
                float(sc["margin_health"]),
                sim,
            )
        )

    risk_cap = 72.0
    margin_floor = 38.0

    def acceptable(t: Tuple[str, float, float, float, float, Dict[str, Any]]) -> bool:
        return t[3] <= risk_cap and t[4] >= margin_floor

    pool = [t for t in evaluated if acceptable(t)]
    if not pool:
        pool = evaluated

    best = min(pool, key=lambda t: (t[1], -t[2]))
    lab, cost, viab, rex, mhealth, _sim = best
    rationale = (
        f"Configuration « {lab} » : CRv équilibré {cost:.2f} TND, viabilité {viab:.1f}/100, "
        f"exposition risque {rex:.1f}/100, santé marge {mhealth:.1f}/100."
    )
    return {
        "label": lab,
        "balancedCostPerUnit": round(cost, 4),
        "productViabilityScore": round(viab, 2),
        "riskExposure": round(rex, 2),
        "marginHealth": round(mhealth, 2),
        "rationale_fr": rationale,
        "variantsEvaluated": len(evaluated),
    }


async def run_cost_intelligence(
    req: AdvancedCostSimulateRequest,
    *,
    monte_carlo_runs: int,
    seed: Optional[int],
) -> Dict[str, Any]:
    base_sim = await run_advanced_simulation(req)
    inp = _to_sim_inputs(req)

    def _mc_sync() -> Tuple[Any, Dict[str, Any], Dict[str, Any]]:
        costs, mc_meta = run_monte_carlo_balanced(inp, n_runs=monte_carlo_runs, seed=seed)
        mc_kpis = summarize_monte_carlo(costs, req.target_margin_pct)
        return costs, mc_meta, mc_kpis

    _, mc_meta, mc_kpis = await asyncio.to_thread(_mc_sync)

    bal = next(s for s in base_sim["scenarios"] if s["code"] == "balanced")
    risks = analyze_risks(inp, bal["total_cost_per_unit"], bal["breakdown"])
    rex = risk_exposure_score(risks)
    spread_ratio = mc_meta["spread_p95_p05"] / max(1e-9, mc_kpis["avgCost"])
    risk_score = monte_carlo_risk_score(mc_meta["coefficient_of_variation"], spread_ratio, rex)

    margin_threshold = 28.0
    flags = _rule_flags(risk_score, float(mc_kpis["expectedMarginPct"]), margin_threshold)
    rule_recs = _rule_recommendations(flags)
    engine_recs = build_recommendations(inp, bal["total_cost_per_unit"], bal["breakdown"])
    merged_recs: List[Dict[str, Any]] = list(rule_recs)
    for r in engine_recs:
        merged_recs.append(
            {
                "priority": int(r.get("priority", 5)),
                "code": "ENGINE",
                "categorie": r.get("categorie", ""),
                "titre_fr": r.get("titre_fr", ""),
                "impact_fr": r.get("impact_fr", ""),
            }
        )

    best_cfg = await _pick_best_configuration(req, base_sim)
    profitability = float(base_sim["scores"]["margin_health"])

    kpis = {
        "monteCarloRuns": mc_meta["runs"],
        "avgCost": mc_kpis["avgCost"],
        "worstCaseCost": mc_kpis["worstCaseCost"],
        "bestCaseCost": mc_kpis["bestCaseCost"],
        "medianCost": mc_kpis["medianCost"],
        "p01Cost": mc_kpis["p01Cost"],
        "p99Cost": mc_kpis["p99Cost"],
        "riskScore": round(risk_score, 2),
        "profitabilityScore": round(profitability, 2),
        "minimumViablePrice": mc_kpis["minimumViablePrice"],
        "recommendedSellingPrice": mc_kpis["recommendedSellingPrice"],
        "expectedMarginPct": mc_kpis["expectedMarginPct"],
        "monteCarloMeta": mc_meta,
        "decisionFlags": flags,
    }

    risk_analysis = {
        "riskScore": round(risk_score, 2),
        "level": "HIGH" if risk_score > 70 else ("MEDIUM" if risk_score > 45 else "LOW"),
        "monteCarloSpread": mc_meta["spread_p95_p05"],
        "coefficientOfVariation": mc_meta["coefficient_of_variation"],
        "legacyRisks": risks,
        "recommendationText": (base_sim.get("executive_summary_fr") or "")[:1200],
    }

    return {
        "kpis": kpis,
        "scenarios": base_sim["scenarios"],
        "riskAnalysis": risk_analysis,
        "recommendations": merged_recs,
        "bestConfiguration": best_cfg,
        "aggregates": base_sim["aggregates"],
        "scores": base_sim["scores"],
        "risks": base_sim["risks"],
        "executive_summary_fr": base_sim["executive_summary_fr"],
        "meta": {
            **base_sim.get("meta", {}),
            "engine": "cost-intelligence-v1",
            "monte_carlo": True,
        },
    }


async def run_cost_intelligence_batch(
    items: List[AdvancedCostSimulateRequest],
    *,
    monte_carlo_runs: int,
    seed: Optional[int],
) -> List[Dict[str, Any]]:
    sem = asyncio.Semaphore(4)

    async def one(r: AdvancedCostSimulateRequest) -> Dict[str, Any]:
        async with sem:
            return await run_cost_intelligence(r, monte_carlo_runs=monte_carlo_runs, seed=seed)

    return list(await asyncio.gather(*[one(r) for r in items]))
