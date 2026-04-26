"""API métier annexe — même serveur Fashion Oracle, chemin séparé du simulateur coût.

Préfixe : ``/api/metiers-annexe`` (aucun mélange avec ``/api/simulateur-cout``).
À étendre : RH, planning, autre domaine métier, sans toucher au contrat simulateur coût.
"""

from __future__ import annotations

from typing import Any, Dict

from fastapi import APIRouter
from pydantic import BaseModel, Field

router = APIRouter(prefix="/api/metiers-annexe", tags=["metiers-annexe"])


class MetiersAnnexeHealth(BaseModel):
    status: str
    service: str
    message_fr: str
    voisinage_api: str


class MetiersAnnexeEchoRequest(BaseModel):
    """Test de connectivité / gabarit pour futurs corps JSON métier."""

    message: str = Field(default="", max_length=2000, description="Texte libre (echo).")


class MetiersAnnexeEchoResponse(BaseModel):
    recu: str
    note_fr: str


@router.get(
    "/health",
    response_model=MetiersAnnexeHealth,
    summary="Santé du module métier annexe",
)
async def health_metiers_annexe() -> MetiersAnnexeHealth:
    return MetiersAnnexeHealth(
        status="ok",
        service="metiers-annexe",
        message_fr="Module API séparé du simulateur de coût ; prêt à accueillir de nouveaux endpoints métier.",
        voisinage_api="Le simulateur coût reste sur /api/simulateur-cout et /api/cout-simulateur.",
    )


@router.get(
    "/capabilities",
    summary="Liste indicative des extensions prévues (documentation)",
)
async def capabilities() -> Dict[str, Any]:
    return {
        "prefix": "/api/metiers-annexe",
        "endpoints_actifs": ["/health", "/capabilities", "/echo"],
        "extensions_suggerees_fr": [
            "Ressources humaines (fiches, agrégats)",
            "Indicateurs atelier / production",
            "Rapports décisionnels hors coût unitaire",
        ],
        "non_couvert": "Aucune logique partagée avec POST .../simulateur-cout/analyser",
    }


@router.post(
    "/echo",
    response_model=MetiersAnnexeEchoResponse,
    summary="Echo JSON (test client / gabarit)",
)
async def echo_metiers_annexe(body: MetiersAnnexeEchoRequest) -> MetiersAnnexeEchoResponse:
    return MetiersAnnexeEchoResponse(
        recu=body.message.strip(),
        note_fr="Réponse de démonstration : remplacer par un vrai cas d'usage métier.",
    )


def openapi_example() -> Dict[str, Any]:
    """Exemple pour documentation / tests."""
    return {
        "health": {"status": "ok", "service": "metiers-annexe"},
        "echo_request": {"message": "ping"},
        "echo_response": {"recu": "ping", "note_fr": "..."},
    }


@router.get("/exemple", summary="Exemple JSON documenté")
async def exemple() -> Dict[str, Any]:
    return openapi_example()
