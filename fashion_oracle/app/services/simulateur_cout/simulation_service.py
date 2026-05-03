"""
SimulationService — single orchestration entry for the Simulateur de cout.

Layers invoked in order:
  CostEngine (baseline) → VariantGenerator+OptimizationEngine → SensitivityEngine
  → ScoringEngine → DecisionEngine (metrics + scores) → InsightEngine → DTO mapping.
"""

from __future__ import annotations

from app.models.simulateur_cout import (
    AlternativeDTO,
    BaselineDTO,
    ObjectifOptimisation,
    OptimiseDTO,
    ScoresDTO,
    SensibiliteDTO,
    SimulateurCoutAnalyserRequest,
    SimulateurCoutAnalyserResponse,
    StressPointDTO,
)
from app.services.simulateur_cout.cost_engine import compute_baseline
from app.services.simulateur_cout.decision_engine import decide_from_metrics, decide_from_scores
from app.services.simulateur_cout.insight_engine import build_insights
from app.services.simulateur_cout.optimization_engine import evaluate_variants
from app.services.simulateur_cout.scoring_engine import compute_scores, stress_collection_for_scoring
from app.services.simulateur_cout.sensitivity_engine import (
    find_critical_material_pct,
    run_material_stress,
    run_time_proxy_stress,
    run_volume_stress,
    stability_index,
)


def _merge_decisions(dm: str, ds: str) -> str:
    """Conservative merge: retain the most cautious (highest severity) signal."""
    order = {"LANCER": 0, "AJUSTER": 1, "RISQUE": 2}
    return dm if order[dm] >= order[ds] else ds


def run_analyse_simulateur(req: SimulateurCoutAnalyserRequest) -> SimulateurCoutAnalyserResponse:
    mat = float(req.coutMatiere)
    vol = int(req.volume)
    obj = req.objectif

    baseline = compute_baseline(cout_matiere=mat, volume=vol)
    dm = decide_from_metrics(
        marge_pct=baseline.marge_pct,
        risque=baseline.risque_base,
        cout=baseline.cout_unitaire,
    )

    mat_stress = run_material_stress(cout_matiere=mat, volume=vol, pcts=(5.0, 10.0, 15.0, 20.0))
    vol_stress = run_volume_stress(cout_matiere=mat, volume=vol, deltas=(-10.0, 10.0))
    tim_stress = run_time_proxy_stress(cout_matiere=mat, volume=vol)
    merged = {**mat_stress, **vol_stress, **tim_stress}
    stabilite = stability_index(merged, dm)
    seuil = find_critical_material_pct(
        cout_matiere=mat, volume=vol, baseline_decision=dm, step=0.5, max_pct=60.0
    )

    best, alts = evaluate_variants(cout_matiere=mat, volume=vol, objectif=obj)
    gain_cout = round(best.cout - baseline.cout_unitaire, 4)
    gain_marge = round(best.marge - baseline.marge_pct, 2)

    stress_for_scores = stress_collection_for_scoring(cout_matiere=mat, volume=vol, extra=merged)
    scores = compute_scores(
        baseline=baseline,
        optimised_marge=best.marge,
        optimised_cout=best.cout,
        stress_values=stress_for_scores,
        seuil_critique_pct=seuil,
    )
    ds = decide_from_scores(
        rentabilite=scores.rentabilite,
        robustesse=scores.robustesse,
        coherence=scores.coherence,
        confiance=scores.confiance,
        marge_pct=baseline.marge_pct,
        risque=baseline.risque_base,
    )
    decision = _merge_decisions(dm, ds)

    insights = build_insights(
        baseline=baseline,
        optimised_marge=best.marge,
        optimised_cout=best.cout,
        seuil_critique_pct=seuil,
        stabilite=stabilite,
        decision=decision,
        max_items=3,
    )

    stress_compact: dict[str, StressPointDTO] = {}
    for key in ("matiere_10", "matiere_15"):
        if key in mat_stress:
            v = mat_stress[key]
            stress_compact[key] = StressPointDTO(cout=v.cout, marge=v.marge, decision=v.decision)
    vol_dto = {k: StressPointDTO(cout=v.cout, marge=v.marge, decision=v.decision) for k, v in vol_stress.items()}
    tim_dto = {k: StressPointDTO(cout=v.cout, marge=v.marge, decision=v.decision) for k, v in tim_stress.items()}

    sens = SensibiliteDTO(
        seuilCritiqueMatiere=seuil,
        stabilite=stabilite,
        stress={**stress_compact, **vol_dto, **tim_dto},
    )

    return SimulateurCoutAnalyserResponse(
        decision=decision,
        baseline=BaselineDTO(
            cout=baseline.cout_unitaire,
            marge=baseline.marge_pct,
            risque=baseline.risque_base,
            prixConseille=baseline.prix_conseille,
        ),
        optimise=OptimiseDTO(
            cout=best.cout,
            marge=best.marge,
            gainCout=gain_cout,
            gainMarge=gain_marge,
        ),
        scores=ScoresDTO(
            rentabilite=scores.rentabilite,
            robustesse=scores.robustesse,
            coherence=scores.coherence,
            confiance=scores.confiance,
        ),
        sensibilite=sens,
        insights=insights,
        alternatives=[
            AlternativeDTO(nom=a.spec.nom, cout=a.cout, marge=a.marge, risque=a.risque) for a in alts[:3]
        ],
    )
