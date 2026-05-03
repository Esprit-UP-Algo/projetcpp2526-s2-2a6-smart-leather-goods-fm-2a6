"""Analyse des risques coût / supply / marge (règles métier + scores)."""
from __future__ import annotations

from typing import Any, Dict, List

from app.services.advanced_cost.simulation_engine import SimInputs


def _norm(s: str) -> str:
    return (s or "").strip().lower()


def analyze_risks(inp: SimInputs, balanced_total: float, balanced_breakdown: Dict[str, float]) -> List[Dict[str, Any]]:
    risks: List[Dict[str, Any]] = []

    if _norm(inp.supplier_risk) in ("high", "eleve", "élevé", "haut") or inp.supplier_reliability_score < 0.55:
        risks.append(
            {
                "code": "SUPPLIER",
                "severity": "high" if inp.supplier_reliability_score < 0.5 else "medium",
                "titre_fr": "Dépendance fournisseur & fiabilité",
                "detail_fr": "La combinaison risque fournisseur / score de fiabilité augmente la variabilité du coût matière et le besoin de couverture stocks.",
            }
        )

    if _norm(inp.market_volatility) in ("high", "eleve", "élevé"):
        risks.append(
            {
                "code": "VOLATILITY",
                "severity": "high",
                "titre_fr": "Volatilité marché matières / change",
                "detail_fr": "Un environnement volatil élargit l'enveloppe pessimiste : prévoir clauses de révision prix ou couverture partielle.",
            }
        )

    mat_share = (balanced_breakdown.get("matiere", 0) / balanced_total) if balanced_total > 0 else 0
    if mat_share > 0.52:
        risks.append(
            {
                "code": "MATERIAL_DOMINANCE",
                "severity": "medium",
                "titre_fr": "Sensibilité au coût matière",
                "detail_fr": "La matière domine la structure de coût : toute variation cours ou rendement coupe impacte fortement le CRv.",
            }
        )

    if _norm(inp.urgency_level) in ("rush", "urgent", "critical", "critique"):
        risks.append(
            {
                "code": "URGENCY",
                "severity": "medium",
                "titre_fr": "Pénalité urgence production",
                "detail_fr": "Les cadences compressées dégradent le rendement et augmentent rebut / heures supplémentaires implicites.",
            }
        )

    margin_proxy = 100.0 * (1.0 - balanced_total / max(balanced_total * 2.8, 0.01))
    if margin_proxy < 30:
        risks.append(
            {
                "code": "MARGIN_FRAGILE",
                "severity": "high",
                "titre_fr": "Marge structurellement fragile",
                "detail_fr": "Au prix cible, la marge nette reste exposée aux aléas matière et logistique : revoir mix prix / coût ou volume.",
            }
        )

    if _norm(inp.sustainability_level) in ("max", "maximum") and _norm(inp.packaging_level) in ("premium", "luxe"):
        risks.append(
            {
                "code": "LOGISTICS_PRESSURE",
                "severity": "low",
                "titre_fr": "Pression coût packaging & conformité",
                "detail_fr": "Niveaux durabilité + packaging premium augmentent la pression sur le poste emballage / transport.",
            }
        )

    if not risks:
        risks.append(
            {
                "code": "STABLE",
                "severity": "low",
                "titre_fr": "Profil de risque modéré",
                "detail_fr": "Les leviers principaux restent opérationnels (productivité, rendement matière) plutôt que macro-risque immédiat.",
            }
        )
    return risks


def risk_exposure_score(risks: List[Dict[str, Any]]) -> float:
    sev_map = {"low": 18, "medium": 48, "high": 82}
    if not risks:
        return 25.0
    acc = 0.0
    for r in risks:
        acc += sev_map.get(r.get("severity", "low"), 30)
    return min(100.0, round(acc / max(len(risks), 1), 2))
