from pydantic import BaseModel, Field
from typing import Any, Dict, List, Optional


class ConceptPreview(BaseModel):
    """Resume concept produit pour l UI (post-prediction, pre-generation)."""

    concept_index: int
    category_label_fr: str
    product_type: str
    variant: str
    style: str
    palette: str
    material: str
    silhouette: str


class ForecastConceptInsight(BaseModel):
    concept_index: int
    product_name: str
    style: str
    palette: str
    material: str
    silhouette: str
    innovation_score: int
    market_score: int
    feasibility_score: int
    trend_timing_score: int
    trend_maturity_score: int
    differentiation_score: int
    saturation_risk: str
    strategic_explanation: str
    evolution_vs_previous: str
    direction_badge: str


class ProductConcept(BaseModel):
    year: int
    concept_index: int
    product_type: str  # veste / ceinture
    variant: str  # premium / technical / luxe
    style: str
    palette: str
    material: str
    silhouette: str


class GeneratedVisualItem(BaseModel):
    year: int
    concept_index: int
    product_type: str
    variant: str
    style: str
    palette: str
    material: str
    silhouette: str
    status: str  # success | error | cached (success = image generee)
    image_url: Optional[str] = None
    error_reason: Optional[str] = Field(
        default=None,
        description="Code stable (ex. replicate_insufficient_credit, replicate_both_models_failed).",
    )
    error_detail: Optional[str] = Field(default=None, description="Message lisible pour UI / debug.")
    primary_model: Optional[str] = Field(default=None, description="Ref Replicate du premier essai.")
    fallback_model: Optional[str] = Field(default=None, description="Ref Replicate du repli flux si tente.")
    used_fallback: bool = False
    primary_model_error: Optional[str] = Field(default=None, description="Erreur API / message modele principal.")
    fallback_model_error: Optional[str] = Field(default=None, description="Erreur API / message repli flux.")
    cached: bool = False
    generation_mode: str = "product_only_v7"
    prompt_hash: str = ""
    cache_key: str = ""


class VisualGenerationResponse(BaseModel):
    year: int
    items: List[GeneratedVisualItem]
    generation_mode: str = "product_only_v7"


class FashionPrediction(BaseModel):
    """Prévision cockpit (série temporelle) — résultat calculé côté serveur, sans repli « faux » client."""

    year: int
    top_styles: List[str]
    color_palette: List[str]
    fabrics_materials: List[str]
    silhouettes: List[str]
    confidence: float
    similar_decades: List[str]
    summary: str
    raw_scores: Dict[str, Any]
    recommended_product_attributes: Dict[str, str]
    chart_trend_base64: Optional[str] = None
    chart_cycle_base64: Optional[str] = None
    generated_visual_prompt: Optional[str] = None
    generated_visual_url: Optional[str] = None
    inference_mode: str = "hybrid-timeseries"
    forecast_eligible: bool = True
    forecast_phase: str = ""
    business_notice: Optional[str] = None
    # Performance / UX : cache prediction cote serveur ; metadonnees concepts sans generation image.
    from_cache: bool = False
    prediction_engine_version: str = "timeseries_v1"
    concepts_preview: List[ConceptPreview] = Field(default_factory=list)
    concepts_projection: List[ForecastConceptInsight] = Field(default_factory=list)
    direction_title: str = ""
    dominant_shift: str = ""
    consumer_signal: str = ""
    strategic_opportunity: str = ""
    year_over_year_evolution: str = ""
    trend_trajectory: Dict[str, Any] = Field(default_factory=dict)
    dominant_direction: str = ""
    strongest_signal: str = ""
    market_posture: str = ""
    risk_level: str = ""
    recommended_capsule: str = ""
    rejected_concepts: List[str] = Field(default_factory=list)
