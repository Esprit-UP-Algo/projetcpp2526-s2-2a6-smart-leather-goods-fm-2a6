"""REST API — Simulateur de cout (moteur metier avance, sortie simple pour Qt).

Deux emplacements URL exposent le meme contrat (meme corps / meme reponse) :
- ``/api/simulateur-cout/*`` — chemin historique
- ``/api/cout-simulateur/*`` — alias dedie (integration, passerelles, clients separes)
"""

from __future__ import annotations

from fastapi import APIRouter

from app.models.simulateur_cout import (
    SimulateurCoutAnalyserRequest,
    SimulateurCoutAnalyserResponse,
    response_example,
)
from app.services.simulateur_cout.simulateur_cout_info import build_api_meta
from app.services.simulateur_cout.simulation_service import run_analyse_simulateur

router = APIRouter(prefix="/api/simulateur-cout", tags=["simulateur-cout"])
router_cout_simulateur = APIRouter(prefix="/api/cout-simulateur", tags=["cout-simulateur"])


def _run_analyse(body: SimulateurCoutAnalyserRequest) -> SimulateurCoutAnalyserResponse:
    return run_analyse_simulateur(body)


@router.post(
    "/analyser",
    response_model=SimulateurCoutAnalyserResponse,
    summary="Analyse complete (baseline, optimisation, sensibilite, scores, insights)",
)
async def analyser_simulateur_cout(body: SimulateurCoutAnalyserRequest) -> SimulateurCoutAnalyserResponse:
    """
    Point d entree principal pour le frontend : orchestration synchrone, sans etat serveur.
    """
    return _run_analyse(body)


@router_cout_simulateur.post(
    "/analyser",
    response_model=SimulateurCoutAnalyserResponse,
    summary="[Alias] Meme analyse que POST /api/simulateur-cout/analyser",
)
async def analyser_cout_simulateur(body: SimulateurCoutAnalyserRequest) -> SimulateurCoutAnalyserResponse:
    """Emplacement API separe : meme charge utile et meme reponse que ``/api/simulateur-cout/analyser``."""
    return _run_analyse(body)


@router.get("/exemple-reponse", summary="Exemple JSON documente (contrat UI)")
async def exemple_reponse() -> dict:
    return response_example()


@router.get("/meta", summary="Metadonnees API simulateur cout (version, prefixes, routes)")
async def meta_simulateur_cout() -> dict:
    """Expose les infos de ``simulateur_cout_info.py`` pour integration / discovery."""
    return build_api_meta()


@router_cout_simulateur.get("/exemple-reponse", summary="[Alias] Exemple JSON (contrat UI)")
async def exemple_reponse_cout_simulateur() -> dict:
    return response_example()


@router_cout_simulateur.get("/meta", summary="[Alias] Meme meta que GET /api/simulateur-cout/meta")
async def meta_cout_simulateur() -> dict:
    return build_api_meta()
