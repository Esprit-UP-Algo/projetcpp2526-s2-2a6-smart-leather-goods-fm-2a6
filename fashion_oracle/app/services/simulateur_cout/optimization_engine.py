"""
OptimizationEngine — ranks generated variants under a business objective.

Pure ranking: no persistence. Returns best variant + top-3 alternatives.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import List, Tuple

from app.models.simulateur_cout import ObjectifOptimisation
from app.services.simulateur_cout.cost_engine import apply_variant_multipliers, compute_baseline
from app.services.simulateur_cout.variant_generator import VariantSpec, generate_variants


@dataclass(frozen=True)
class EvaluatedVariant:
    spec: VariantSpec
    cout: float
    marge: float
    risque: float
    score_objectif: float


def _objective_score(o: ObjectifOptimisation, marge: float, risque: float, cout: float) -> float:
    if o == ObjectifOptimisation.marge_max:
        return marge * 1.35 - risque * 0.25 - cout * 0.08
    if o == ObjectifOptimisation.risque_min:
        return -risque * 1.4 + marge * 0.55 - cout * 0.05
    # equilibre
    return marge * 0.9 - risque * 0.55 - cout * 0.06


def evaluate_variants(
    *,
    cout_matiere: float,
    volume: int,
    objectif: ObjectifOptimisation,
) -> Tuple[EvaluatedVariant, List[EvaluatedVariant]]:
    specs = generate_variants(cout_matiere=cout_matiere, volume=volume)
    evaluated: List[EvaluatedVariant] = []
    base = compute_baseline(cout_matiere=cout_matiere, volume=volume)
    for sp in specs:
        v_eff = int(round(volume * (1.0 + sp.volume_delta_pct / 100.0)))
        econ = apply_variant_multipliers(
            cout_matiere,
            volume,
            mat_mult=sp.mat_mult,
            efficacite_mult=sp.efficacite_mult,
            volume_effectif=max(1, v_eff),
            overhead_mult=sp.overhead_mult,
        )
        sc = _objective_score(objectif, econ.marge_pct, econ.risque_base, econ.cout_unitaire)
        evaluated.append(
            EvaluatedVariant(
                spec=sp,
                cout=round(econ.cout_unitaire, 4),
                marge=round(econ.marge_pct, 2),
                risque=round(econ.risque_base, 2),
                score_objectif=round(sc, 4),
            )
        )
    # Baseline as implicit candidate
    baseline_ev = EvaluatedVariant(
        spec=VariantSpec("BASE", "Reference saisie", 1.0, 1.0, 0.0, 1.0),
        cout=base.cout_unitaire,
        marge=base.marge_pct,
        risque=base.risque_base,
        score_objectif=_objective_score(objectif, base.marge_pct, base.risque_base, base.cout_unitaire),
    )
    evaluated.append(baseline_ev)
    evaluated.sort(key=lambda x: x.score_objectif, reverse=True)
    # Second pass: among top candidates, prefer tangible economic uplift (marge / cout),
    # not only the composite score (which may overweight risk noise).
    pool = evaluated[: min(10, len(evaluated))]

    def _tradeoff(ev: EvaluatedVariant) -> float:
        return (ev.marge - base.marge_pct) * 1.45 + (base.cout_unitaire - ev.cout) * 0.52 - (ev.risque - base.risque_base) * 0.09

    best = max(pool, key=_tradeoff)
    if _tradeoff(best) <= 0.0 and best.spec.code != "BASE":
        best = baseline_ev
    alts = [e for e in evaluated if e.spec.code != best.spec.code][:3]
    return best, alts
