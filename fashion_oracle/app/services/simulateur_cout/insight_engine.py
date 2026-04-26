"""
InsightEngine — short French labels for UI chips (max 3 returned upstream).

Rule-based, no LLM: deterministic mapping from economics + optimisation delta.
"""

from __future__ import annotations

from typing import List

from app.services.simulateur_cout.cost_engine import BaselineEconomics


def build_insights(
    *,
    baseline: BaselineEconomics,
    optimised_marge: float,
    optimised_cout: float,
    seuil_critique_pct: float,
    stabilite: int,
    decision: str,
    max_items: int = 3,
) -> List[str]:
    out: List[str] = []
    if optimised_marge - baseline.marge_pct >= 3.0:
        out.append("Marge ameliorable")
    if baseline.marge_pct < 32.0:
        out.append("Pression marge")
    if baseline.risque_base > 55.0:
        out.append("Risque eleve")
    elif baseline.risque_base > 42.0:
        out.append("Risque modere")
    else:
        out.append("Risque contenu")
    if seuil_critique_pct <= 15.0:
        out.append("Seuil matiere serre")
    elif seuil_critique_pct >= 35.0:
        out.append("Marge de manoeuvre matiere")
    if stabilite >= 72:
        out.append("Scenario stable")
    elif stabilite < 50:
        out.append("Sensibilite forte")
    if optimised_cout < baseline.cout_unitaire - 1.0:
        out.append("Levier cout actif")
    # Volume signal from risk model proxy
    if baseline.risque_base < 40.0 and baseline.marge_pct >= 34.0:
        out.append("Volume favorable")

    # Dedup while preserving order
    seen = set()
    deduped: List[str] = []
    for x in out:
        if x not in seen:
            seen.add(x)
            deduped.append(x)
    if decision == "LANCER" and "Marge ameliorable" not in deduped[:2]:
        deduped.insert(0, "Lancement coherent")
    return deduped[:max_items]
