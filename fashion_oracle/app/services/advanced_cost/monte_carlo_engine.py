"""Monte Carlo sur le coût unitaire (scénario équilibré) — variations stochastiques métier."""

from __future__ import annotations

from dataclasses import replace
from typing import Any, Dict, Optional, Tuple

import numpy as np

from app.services.advanced_cost.simulation_engine import (
    SimInputs,
    compute_unit_cost_for_scenario,
    estimate_margin_at_price,
    selling_price_for_margin,
)


def _supplier_delay_factor(rng: np.random.Generator, supplier_risk: str) -> float:
    r = (supplier_risk or "").strip().lower()
    base = 0.03 if r in ("low", "faible") else 0.07 if r in ("high", "eleve", "élevé") else 0.05
    return float(1.0 + rng.uniform(0.0, base))


def _perturb_inputs(base: SimInputs, rng: np.random.Generator) -> SimInputs:
    """Variations : matière, délai fournisseur (via temps), rebut, temps de production."""
    mat_mult = float(rng.lognormal(mean=0.0, sigma=0.055))
    hrs_mult = float(rng.normal(1.0, 0.035))
    defect_delta = float(rng.uniform(-0.35, 0.75))
    delay_f = _supplier_delay_factor(rng, base.supplier_risk)
    mat_mult = max(0.65, min(1.45, mat_mult))
    hrs_mult = max(0.55, min(1.45, hrs_mult * delay_f))
    new_defect = max(0.0, min(40.0, base.defect_waste_rate_pct + defect_delta))
    new_hours = max(0.0, base.labor_hours * hrs_mult)
    new_mat = max(0.0, base.material_unit_cost * mat_mult)
    return replace(
        base,
        material_unit_cost=new_mat,
        labor_hours=new_hours,
        defect_waste_rate_pct=new_defect,
    )


def run_monte_carlo_balanced(
    base: SimInputs,
    *,
    n_runs: int,
    seed: Optional[int] = None,
) -> Tuple[np.ndarray, Dict[str, Any]]:
    n = max(500, min(2000, int(n_runs)))
    rng = np.random.default_rng(seed)
    costs = np.empty(n, dtype=np.float64)
    for i in range(n):
        p = _perturb_inputs(base, rng)
        total, _, _ = compute_unit_cost_for_scenario(p, "balanced")
        costs[i] = total
    spread = float(np.percentile(costs, 95) - np.percentile(costs, 5))
    cv = float(np.std(costs) / max(1e-9, np.mean(costs)))
    meta = {
        "runs": n,
        "spread_p95_p05": round(spread, 4),
        "coefficient_of_variation": round(cv, 4),
        "seed": seed,
    }
    return costs, meta


def summarize_monte_carlo(costs: np.ndarray, target_margin_pct: float) -> Dict[str, Any]:
    mean_c = float(np.mean(costs))
    p1 = float(np.percentile(costs, 1))
    p99 = float(np.percentile(costs, 99))
    median_c = float(np.median(costs))
    worst = float(np.max(costs))
    best = float(np.min(costs))
    reco_price = selling_price_for_margin(mean_c, target_margin_pct)
    mvp = selling_price_for_margin(p99, max(5.0, target_margin_pct - 8.0))
    exp_margin = estimate_margin_at_price(mean_c, reco_price)
    return {
        "avgCost": round(mean_c, 4),
        "worstCaseCost": round(worst, 4),
        "bestCaseCost": round(best, 4),
        "medianCost": round(median_c, 4),
        "p01Cost": round(p1, 4),
        "p99Cost": round(p99, 4),
        "minimumViablePrice": round(mvp, 4),
        "recommendedSellingPrice": round(reco_price, 4),
        "expectedMarginPct": exp_margin,
    }


def monte_carlo_risk_score(cv: float, spread_ratio: float, base_risk_exposure: float) -> float:
    """0–100 : combine variabilité MC et score exposition existant."""
    vol = min(55.0, cv * 420.0 + spread_ratio * 28.0)
    return float(min(100.0, max(0.0, 0.45 * base_risk_exposure + 0.55 * vol)))
