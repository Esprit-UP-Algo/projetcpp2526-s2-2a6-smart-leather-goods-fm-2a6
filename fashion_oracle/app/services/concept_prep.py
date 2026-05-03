"""Preparation des concepts produit (deterministe, leger — sans generation image)."""

from __future__ import annotations

from app.models.schemas import ConceptPreview, ProductConcept

# Mapping cockpit fixe (premium / professionnel).
_CONCEPT_LABEL_FR: dict[int, str] = {
    1: "Concept 1 — Veste premium",
    2: "Concept 2 — Veste technique",
    3: "Concept 3 — Ceinture luxe",
}


def build_concepts_for_year(year: int) -> list[ProductConcept]:
    """Trois concepts produit stables (pas de visage / pas de mannequin dans les prompts amont)."""
    return [
        ProductConcept(
            year=year,
            concept_index=1,
            product_type="veste",
            variant="premium",
            style="artisan-leather",
            palette="sand",
            material="raw linen",
            silhouette="structured premium outerwear",
        ),
        ProductConcept(
            year=year,
            concept_index=2,
            product_type="veste",
            variant="technical",
            style="athlux-utility",
            palette="charcoal",
            material="mesh",
            silhouette="technical utility outerwear",
        ),
        ProductConcept(
            year=year,
            concept_index=3,
            product_type="ceinture",
            variant="luxe",
            style="minimal-tailoring",
            palette="off-white",
            material="wool",
            silhouette="premium belt accessory",
        ),
    ]


def build_concept_previews(year: int) -> list[ConceptPreview]:
    """Metadonnees pretes pour l UI apres prediction (aucun appel fournisseur)."""
    out: list[ConceptPreview] = []
    for c in build_concepts_for_year(year):
        out.append(
            ConceptPreview(
                concept_index=c.concept_index,
                category_label_fr=_CONCEPT_LABEL_FR.get(c.concept_index, f"Concept {c.concept_index}"),
                product_type=c.product_type,
                variant=c.variant,
                style=c.style,
                palette=c.palette,
                material=c.material,
                silhouette=c.silhouette,
            )
        )
    return out
