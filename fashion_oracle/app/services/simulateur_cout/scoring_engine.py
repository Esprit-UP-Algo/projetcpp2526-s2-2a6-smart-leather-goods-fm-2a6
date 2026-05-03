"""
ScoringEngine — four 0-100 indices for dashboard / Qt binding.

Combines baseline economics, optimisation headroom, and stress dispersion.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterable

from app.services.simulateur_cout.cost_engine import BaselineEconomics, compute_baseline
from app.services.simulateur_cout.sensitivity_engine import StressResult


@dataclass(frozen=True)
class Scores:
    rentabilite: int
    robustesse: int
    coherence: int
    confiance: int


def _clamp_int(x: float) -> int:
    return int(max(0, min(100, round(x))))


def compute_scores(
    *,
    baseline: BaselineEconomics,
    optimised_marge: float,
    optimised_cout: float,
    stress_values: Iterable[StressResult],
    seuil_critique_pct: float,
) -> Scores:
    gain_marge = max(0.0, optimised_marge - baseline.marge_pct)
    gain_cout = baseline.cout_unitaire - optimised_cout
    rent = 52.0 + 0.85 * baseline.marge_pct + 1.4 * gain_marge + 0.35 * gain_cout - 0.22 * baseline.risque_base

    decisions = [s.decision for s in stress_values]
    maj = len([d for d in decisions if d == "LANCER"])
    ris = len([d for d in decisions if d == "RISQUE"])
    dispersion = max(maj, ris) / max(1, len(decisions))
    robust = 48.0 + 40.0 * (1.0 - dispersion) + min(18.0, seuil_critique_pct * 0.22)

    coh = 50.0 + 0.45 * baseline.marge_pct - 0.28 * baseline.risque_base + 0.15 * (100.0 - abs(baseline.marge_pct - optimised_marge))

    conf = 45.0 + 0.35 * baseline.marge_pct + 0.25 * (100.0 - baseline.risque_base)
    if seuil_critique_pct < 18.0:
        conf -= 10.0
    elif seuil_critique_pct > 38.0:
        conf += 6.0

    return Scores(
        rentabilite=_clamp_int(rent),
        robustesse=_clamp_int(robust),
        coherence=_clamp_int(coh),
        confiance=_clamp_int(conf),
    )


def stress_collection_for_scoring(
    *,
    cout_matiere: float,
    volume: int,
    extra: Dict[str, StressResult],
) -> list[StressResult]:
    """Flatten stress maps into one iterable for scoring."""
    base = compute_baseline(cout_matiere=cout_matiere, volume=volume)
    from app.services.simulateur_cout.decision_engine import decide_from_metrics

    base_d = decide_from_metrics(
        marge_pct=base.marge_pct, risque=base.risque_base, cout=base.cout_unitaire
    )
    core = StressResult(cout=base.cout_unitaire, marge=base.marge_pct, decision=base_d)
    return [core, *list(extra.values())]
