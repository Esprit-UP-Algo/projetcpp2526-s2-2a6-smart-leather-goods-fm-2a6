"""DTOs REST pour le Simulateur de cout (couche presentation — labels metier en francais)."""

from __future__ import annotations

from enum import Enum
from typing import Any, Dict, List, Optional

from pydantic import BaseModel, Field


class ObjectifOptimisation(str, Enum):
    marge_max = "marge_max"
    risque_min = "risque_min"
    equilibre = "equilibre"


class SimulateurCoutAnalyserRequest(BaseModel):
    produit: str = Field(default="", max_length=500)
    coutMatiere: float = Field(..., ge=0.01, le=1_000_000, description="Cout matiere unitaire (TND)")
    volume: int = Field(..., ge=1, le=1_000_000)
    objectif: ObjectifOptimisation = Field(
        default=ObjectifOptimisation.equilibre,
        description="Priorite d optimisation interne",
    )


class BaselineDTO(BaseModel):
    cout: float
    marge: float
    risque: float
    prixConseille: float


class OptimiseDTO(BaseModel):
    cout: float
    marge: float
    gainCout: float
    gainMarge: float


class ScoresDTO(BaseModel):
    rentabilite: int = Field(..., ge=0, le=100)
    robustesse: int = Field(..., ge=0, le=100)
    coherence: int = Field(..., ge=0, le=100)
    confiance: int = Field(..., ge=0, le=100)


class StressPointDTO(BaseModel):
    cout: float
    marge: float
    decision: str


class SensibiliteDTO(BaseModel):
    seuilCritiqueMatiere: float = Field(..., description="Pourcentage matiere ou la decision bascule (estimation)")
    stabilite: int = Field(..., ge=0, le=100)
    stress: Dict[str, StressPointDTO]


class AlternativeDTO(BaseModel):
    nom: str
    cout: float
    marge: float
    risque: float


class SimulateurCoutAnalyserResponse(BaseModel):
    decision: str
    baseline: BaselineDTO
    optimise: OptimiseDTO
    scores: ScoresDTO
    sensibilite: SensibiliteDTO
    insights: List[str] = Field(default_factory=list, max_length=8)
    alternatives: List[AlternativeDTO] = Field(default_factory=list)


def response_example() -> Dict[str, Any]:
    """Exemple documente pour OpenAPI / equipe front."""
    return {
        "decision": "AJUSTER",
        "baseline": {"cout": 56.21, "marge": 33.0, "risque": 28.0, "prixConseille": 138.0},
        "optimise": {"cout": 52.84, "marge": 39.0, "gainCout": -3.37, "gainMarge": 6.0},
        "scores": {"rentabilite": 68, "robustesse": 64, "coherence": 72, "confiance": 67},
        "sensibilite": {
            "seuilCritiqueMatiere": 28.0,
            "stabilite": 61,
            "stress": {
                "matiere_10": {"cout": 59.1, "marge": 30.4, "decision": "AJUSTER"},
                "matiere_15": {"cout": 61.5, "marge": 27.8, "decision": "RISQUE"},
            },
        },
        "insights": ["Marge ameliorable", "Volume favorable", "Risque modere"],
        "alternatives": [
            {"nom": "Variant A", "cout": 53.4, "marge": 38.2, "risque": 27.0},
            {"nom": "Variant B", "cout": 54.1, "marge": 37.5, "risque": 24.0},
        ],
    }
