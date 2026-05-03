"""
SensitivityEngine — stress grid on material, volume, and time proxies.

Detects critical material threshold (decision flip), stability index,
and a compact stress map for the API contract.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Tuple

from app.services.simulateur_cout.cost_engine import apply_variant_multipliers, compute_baseline
from app.services.simulateur_cout.decision_engine import decide_from_metrics


@dataclass(frozen=True)
class StressResult:
    cout: float
    marge: float
    decision: str


def run_material_stress(*, cout_matiere: float, volume: int, pcts: Tuple[float, ...]) -> Dict[str, StressResult]:
    out: Dict[str, StressResult] = {}
    for p in pcts:
        mat = cout_matiere * (1.0 + p / 100.0)
        b = compute_baseline(cout_matiere=mat, volume=volume)
        d = decide_from_metrics(marge_pct=b.marge_pct, risque=b.risque_base, cout=b.cout_unitaire)
        key = f"matiere_{int(p)}" if p == int(p) else f"matiere_{p:.1f}".replace(".", "_")
        out[key] = StressResult(cout=b.cout_unitaire, marge=b.marge_pct, decision=d)
    return out


def run_volume_stress(*, cout_matiere: float, volume: int, deltas: Tuple[float, ...]) -> Dict[str, StressResult]:
    out: Dict[str, StressResult] = {}
    for dlt in deltas:
        v = max(1, int(round(volume * (1.0 + dlt / 100.0))))
        b = compute_baseline(cout_matiere=cout_matiere, volume=v)
        d = decide_from_metrics(marge_pct=b.marge_pct, risque=b.risque_base, cout=b.cout_unitaire)
        if dlt < 0:
            tag = "moins_10"
        else:
            tag = "plus_10"
        out[f"volume_{tag}"] = StressResult(cout=b.cout_unitaire, marge=b.marge_pct, decision=d)
    return out


def run_time_proxy_stress(*, cout_matiere: float, volume: int) -> Dict[str, StressResult]:
    """Time +/-8% modeled as efficiency multiplier on labour proxy."""
    out: Dict[str, StressResult] = {}
    for label, eff in (("temps_plus_8", 0.92), ("temps_moins_8", 1.08)):
        e = apply_variant_multipliers(
            cout_matiere, volume, mat_mult=1.0, efficacite_mult=eff, overhead_mult=1.0
        )
        d = decide_from_metrics(marge_pct=e.marge_pct, risque=e.risque_base, cout=e.cout_unitaire)
        out[label] = StressResult(cout=e.cout_unitaire, marge=e.marge_pct, decision=d)
    return out


def find_critical_material_pct(
    *,
    cout_matiere: float,
    volume: int,
    baseline_decision: str,
    step: float = 0.5,
    max_pct: float = 60.0,
) -> float:
    """First +X% material (vs baseline) that changes the coarse decision."""
    for s in range(1, int(max_pct / step) + 2):
        p = s * step
        mat = cout_matiere * (1.0 + p / 100.0)
        b = compute_baseline(cout_matiere=mat, volume=volume)
        d = decide_from_metrics(marge_pct=b.marge_pct, risque=b.risque_base, cout=b.cout_unitaire)
        if d != baseline_decision:
            return round(p, 1)
    return round(max_pct, 1)


def stability_index(stress_map: Dict[str, StressResult], baseline_decision: str) -> int:
    """0-100: how often stress scenarios keep the same decision class as baseline."""
    if not stress_map:
        return 50
    same = sum(1 for v in stress_map.values() if v.decision == baseline_decision)
    return int(round(100.0 * same / len(stress_map)))
