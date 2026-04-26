"""
DecisionEngine — coarse GO / ADJUST / RISK gate from composite metrics.

Uses margin bands, risk saturation, and unit cost pressure.
Output tokens match the Qt simulator badges (French, ASCII-safe RISQUE).
"""

from __future__ import annotations


def decide_from_metrics(*, marge_pct: float, risque: float, cout: float) -> str:
    """
    Rule stack (explicit, auditable):
    - Very low margin or very high risk => RISQUE
    - Moderate tension => AJUSTER
    - Healthy margin + contained risk => LANCER
    Cost pressure modulates risk perception when margin is borderline.
    """
    pression_cout = min(100.0, max(0.0, (cout - 35.0) * 1.15))
    risque_eff = min(100.0, risque + pression_cout * 0.12)

    if marge_pct < 22.0 or risque_eff > 72.0:
        return "RISQUE"
    if marge_pct < 30.0 or risque_eff > 58.0:
        return "AJUSTER"
    if marge_pct >= 40.0 and risque_eff <= 48.0:
        return "LANCER"
    if marge_pct >= 34.0 and risque_eff <= 55.0:
        return "LANCER"
    return "AJUSTER"


def decide_from_scores(
    *,
    rentabilite: int,
    robustesse: int,
    coherence: int,
    confiance: int,
    marge_pct: float,
    risque: float,
) -> str:
    """Second reading: composite scores can override marginal numeric cases."""
    comp = 0.28 * rentabilite + 0.30 * robustesse + 0.22 * coherence + 0.20 * confiance
    if comp < 48 or marge_pct < 24:
        return "RISQUE" if comp < 40 or marge_pct < 20 else "AJUSTER"
    if comp >= 68 and marge_pct >= 36 and risque <= 52:
        return "LANCER"
    if comp >= 58:
        return "AJUSTER"
    return "AJUSTER"
