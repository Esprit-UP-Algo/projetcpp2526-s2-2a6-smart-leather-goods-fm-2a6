"""Schémas API Simulateur de Coût Avancé (hors CRUD, orientation décision)."""

from __future__ import annotations

from typing import Any, Dict, List, Optional

from pydantic import BaseModel, Field


class AdvancedCostSimulateRequest(BaseModel):
    designation: str = Field(..., min_length=1, max_length=500)
    raw_material_type: str = Field(default="", max_length=200)
    material_unit_cost: float = Field(..., ge=0, le=1_000_000)
    material_quantity_per_unit: float = Field(default=1.0, gt=0, le=10_000)
    labor_hours: float = Field(default=0, ge=0, le=10_000)
    labor_hourly_rate: float = Field(default=15.5, gt=0, le=10_000)
    labor_charges_sociales_pct: float = Field(
        default=0.0,
        ge=0,
        le=80,
        description="Charges sociales en % sur MOD brute — intégrées au taux horaire effectif côté moteur.",
    )
    finishing_level: str = Field(default="premium", max_length=32)
    packaging_level: str = Field(default="standard", max_length=32)
    transport_cost_per_unit: float = Field(default=0.0, ge=0, le=100_000)
    supplier_risk: str = Field(default="medium", max_length=32)
    market_volatility: str = Field(default="medium", max_length=32)
    production_volume: int = Field(default=100, ge=1, le=10_000_000)
    urgency_level: str = Field(default="normal", max_length=32)
    sustainability_level: str = Field(default="standard", max_length=32)
    target_quality_level: str = Field(default="high", max_length=32)
    target_margin_pct: float = Field(default=58.0, ge=5, le=92)
    supplier_reliability_score: float = Field(default=0.78, ge=0, le=1)
    defect_waste_rate_pct: float = Field(default=2.5, ge=0, le=40)
    overhead_rate_pct: float = Field(default=22.0, ge=0, le=80)
    currency_fluctuation_factor: float = Field(default=1.0, ge=0.5, le=1.5)
    premium_branding_level: str = Field(default="medium", max_length=32)
    packaging_unit_cost_floor: float = Field(
        default=0.0,
        ge=0,
        le=5_000,
        description="Plancher emballage TND/u (saisie atelier) — max avec le coût dérivé du niveau packaging.",
    )
    collection: str = Field(default="", max_length=300)
    notes: str = Field(default="", max_length=4000)
    skip_llm: bool = False
    cost_model: Optional[str] = Field(default=None, max_length=120)


class ScenarioBreakdown(BaseModel):
    matiere: float = 0.0
    main_oeuvre: float = 0.0
    charges_structure: float = 0.0
    emballage_logistique: float = 0.0
    risque_volatilite: float = 0.0
    durabilite_branding: float = 0.0


class ScenarioResult(BaseModel):
    code: str
    label_fr: str
    total_cost_per_unit: float
    estimated_margin_pct: float
    risk_level: str
    positioning_fr: str
    breakdown: ScenarioBreakdown
    recommended_selling_price: float


class RiskItem(BaseModel):
    code: str
    severity: str
    titre_fr: str
    detail_fr: str


class RecommendationItem(BaseModel):
    priority: int = Field(..., ge=1, le=10)
    categorie: str
    titre_fr: str
    impact_fr: str


class ScoreBlock(BaseModel):
    cost_efficiency: float = Field(..., ge=0, le=100)
    risk_exposure: float = Field(..., ge=0, le=100)
    margin_health: float = Field(..., ge=0, le=100)
    product_viability: float = Field(..., ge=0, le=100)


class CostAggregates(BaseModel):
    optimistic_cost: float
    realistic_cost: float
    pessimistic_cost: float
    cost_confidence_score: float = Field(..., ge=0, le=100)


class AdvancedCostSimulateResponse(BaseModel):
    designation: str
    currency: str = "TND"
    reference_scenario: str = "balanced"
    scenarios: List[ScenarioResult]
    aggregates: CostAggregates
    scores: ScoreBlock
    risks: List[RiskItem]
    recommendations: List[RecommendationItem]
    executive_summary_fr: str
    meta: Dict[str, Any] = Field(default_factory=dict)


class AdvancedCostOptimizeRequest(AdvancedCostSimulateRequest):
    objective: str = Field(default="balance", max_length=64)


class AdvancedCostOptimizeResponse(BaseModel):
    best_variant_label_fr: str
    request_snapshot: Dict[str, Any]
    simulation: AdvancedCostSimulateResponse
    alternatives_evaluated: int = 0


class CostHealthResponse(BaseModel):
    status: str = "ok"
    service: str = "advanced-cost-engine"
    llm_configured: bool = False


class CostIntelligenceRequest(AdvancedCostSimulateRequest):
    """Même corps que la simulation + paramètres Monte Carlo (exécution async côté serveur)."""

    monte_carlo_runs: int = Field(default=750, ge=500, le=2000)
    monte_carlo_seed: Optional[int] = Field(default=None)


class CostIntelligenceBatchRequest(BaseModel):
    items: List[AdvancedCostSimulateRequest] = Field(..., min_length=1, max_length=20)
    monte_carlo_runs: int = Field(default=750, ge=500, le=2000)
    monte_carlo_seed: Optional[int] = Field(default=None)


class CostIntelligenceBatchResponse(BaseModel):
    results: List[Dict[str, Any]]
