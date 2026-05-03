"""Recommandations d'optimisation déterministes (extensible ML)."""
from __future__ import annotations

from typing import Any, Dict, List

from app.services.advanced_cost.simulation_engine import SimInputs


def _norm(s: str) -> str:
    return (s or "").strip().lower()


def build_recommendations(
    inp: SimInputs,
    balanced_total: float,
    balanced_breakdown: Dict[str, float],
) -> List[Dict[str, Any]]:
    recs: List[Dict[str, Any]] = []
    prio = 1

    mat = balanced_breakdown.get("matiere", 0)
    if balanced_total > 0 and mat / balanced_total > 0.48:
        recs.append(
            {
                "priority": prio,
                "categorie": "matiere",
                "titre_fr": "Sécuriser le coût matière (contrat, rendement coupe, alternative)",
                "impact_fr": "Potentiellement -3 % à -9 % sur le CRv si rendement + négociation volume ou substitution partielle.",
            }
        )
        prio += 1

    if inp.defect_waste_rate_pct > 2.2:
        recs.append(
            {
                "priority": prio,
                "categorie": "atelier",
                "titre_fr": "Réduire le taux de rebut et retouches",
                "impact_fr": "Chaque point de rebut en moins se répercute directement sur la marge à prix constant.",
            }
        )
        prio += 1

    if _norm(inp.finishing_level) in ("luxe", "premium") and _norm(inp.target_quality_level) in ("medium", "moyen"):
        recs.append(
            {
                "priority": prio,
                "categorie": "positionnement",
                "titre_fr": "Aligner finition premium vs exigence qualité cible",
                "impact_fr": "Éviter sur-spécification : ajuster finition ou gamme pour protéger le prix sans fragiliser la perception.",
            }
        )
        prio += 1

    if inp.production_volume < 80:
        recs.append(
            {
                "priority": prio,
                "categorie": "volume",
                "titre_fr": "Augmenter la taille de série ou mutualiser les lancements",
                "impact_fr": "Les coûts fixes (outillage, logistique) se diluent avec le volume : effet visible sur le CRv unitaire.",
            }
        )
        prio += 1

    if _norm(inp.packaging_level) in ("premium", "luxe"):
        recs.append(
            {
                "priority": prio,
                "categorie": "packaging",
                "titre_fr": "Revoir le niveau d'emballage vs canal de vente",
                "impact_fr": "Sur B2B ou réassort, un packaging intermédiaire peut préserver la marge sans dégrader l'image.",
            }
        )
        prio += 1

    if inp.currency_fluctuation_factor > 1.08:
        recs.append(
            {
                "priority": prio,
                "categorie": "finance",
                "titre_fr": "Couvrir le risque de change sur approvisionnement import",
                "impact_fr": "Clause de révision, couverture partielle ou sourcing local pour stabiliser le CRv.",
            }
        )
        prio += 1

    if _norm(inp.urgency_level) != "normal":
        recs.append(
            {
                "priority": prio,
                "categorie": "planification",
                "titre_fr": "Lisser la charge pour sortir du mode urgence",
                "impact_fr": "Réduction des surcoûts implicites MOD et baisse du rebut lié au stress de cadence.",
            }
        )
        prio += 1

    recs.append(
        {
            "priority": min(prio, 10),
            "categorie": "prix",
            "titre_fr": "Tester un repositionnement prix sur le scénario équilibré",
            "impact_fr": "Valider l'élasticité : le simulateur donne un prix public indicatif cohérent avec la marge cible.",
        }
    )
    return recs[:10]
