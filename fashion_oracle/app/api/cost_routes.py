"""API Simulateur de Coût Avancé — /cost/*"""

from fastapi import APIRouter

from app.config import settings
from typing import Any, Dict, List

from app.models.advanced_cost import (
    AdvancedCostOptimizeRequest,
    AdvancedCostOptimizeResponse,
    AdvancedCostSimulateRequest,
    AdvancedCostSimulateResponse,
    CostHealthResponse,
    CostIntelligenceBatchRequest,
    CostIntelligenceBatchResponse,
    CostIntelligenceRequest,
)
from app.services.advanced_cost.intelligence_engine import run_cost_intelligence, run_cost_intelligence_batch
from app.services.advanced_cost.orchestrator import run_advanced_simulation, run_optimize

router = APIRouter(prefix="/cost", tags=["advanced-cost"])


@router.get("/health", response_model=CostHealthResponse)
async def cost_health() -> CostHealthResponse:
    return CostHealthResponse(
        status="ok",
        service="advanced-cost-engine",
        llm_configured=bool(settings.api_key),
    )


@router.post("/simulate", response_model=AdvancedCostSimulateResponse)
async def cost_simulate(body: AdvancedCostSimulateRequest) -> AdvancedCostSimulateResponse:
    data = await run_advanced_simulation(body)
    return AdvancedCostSimulateResponse.model_validate(data)


@router.post("/optimize", response_model=AdvancedCostOptimizeResponse)
async def cost_optimize(body: AdvancedCostOptimizeRequest) -> AdvancedCostOptimizeResponse:
    data = await run_optimize(body)
    sim = AdvancedCostSimulateResponse.model_validate(data["simulation"])
    return AdvancedCostOptimizeResponse(
        best_variant_label_fr=data["best_variant_label_fr"],
        request_snapshot=data["request_snapshot"],
        simulation=sim,
        alternatives_evaluated=data["alternatives_evaluated"],
    )


@router.post("/intelligence")
async def cost_intelligence(body: CostIntelligenceRequest) -> Dict[str, Any]:
    """Monte Carlo + KPIs décisionnels + meilleure configuration (JSON BI, hors CRUD)."""
    d = body.model_dump()
    runs = int(d.pop("monte_carlo_runs", 750))
    seed = d.pop("monte_carlo_seed", None)
    base = AdvancedCostSimulateRequest.model_validate(d)
    return await run_cost_intelligence(base, monte_carlo_runs=runs, seed=seed)


@router.post("/intelligence/batch", response_model=CostIntelligenceBatchResponse)
async def cost_intelligence_batch(body: CostIntelligenceBatchRequest) -> CostIntelligenceBatchResponse:
    out: List[Dict[str, Any]] = await run_cost_intelligence_batch(
        body.items,
        monte_carlo_runs=body.monte_carlo_runs,
        seed=body.monte_carlo_seed,
    )
    return CostIntelligenceBatchResponse(results=out)
