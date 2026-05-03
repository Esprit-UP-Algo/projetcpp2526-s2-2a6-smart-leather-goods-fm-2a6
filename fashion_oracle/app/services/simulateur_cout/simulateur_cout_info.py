"""
Fichier compagnon du simulateur de coût : métadonnées API, version, chemins documentés.

Le moteur chiffré reste dans ``simulation_service.py`` ; ce module sert aux clients
(Qt, passerelles) pour découverte / contrat sans importer tout le pipeline.
"""

from __future__ import annotations

from typing import Any, Dict, List

# Incrémenter lors d'un changement de contrat JSON ou de comportement observable côté API.
SIMULATEUR_COUT_API_VERSION = "1.0.0"


def build_api_meta() -> Dict[str, Any]:
    """Description stable des routes exposées (même logique sur les deux préfixes)."""
    prefixes: List[str] = ["/api/simulateur-cout", "/api/cout-simulateur"]
    routes: List[Dict[str, str]] = [
        {"method": "POST", "suffix": "/analyser", "summary": "Analyse complète (corps SimulateurCoutAnalyserRequest)"},
        {"method": "GET", "suffix": "/exemple-reponse", "summary": "Exemple JSON (contrat UI)"},
        {"method": "GET", "suffix": "/meta", "summary": "Métadonnées service / routes (ce fichier)"},
    ]
    return {
        "service": "simulateur-cout",
        "api_version": SIMULATEUR_COUT_API_VERSION,
        "prefixes": prefixes,
        "routes": routes,
        "note_fr": "Les deux préfixes partagent le même contrat ; choisir l’un ou l’autre selon l’intégration.",
    }
