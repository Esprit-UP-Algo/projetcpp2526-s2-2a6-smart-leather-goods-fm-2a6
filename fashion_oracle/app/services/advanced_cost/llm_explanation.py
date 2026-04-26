"""Synthèse exécutive via LLM (OpenRouter) avec repli déterministe."""
from __future__ import annotations

import json
import re
from typing import Any, Dict, List, Optional

import httpx

from app.config import settings


def _strip_fence(content: str) -> str:
    c = content.strip()
    if c.startswith("```"):
        c = re.sub(r"^```[a-zA-Z]*\s*", "", c)
        c = re.sub(r"\s*```$", "", c).strip()
    return c


def deterministic_executive_summary(
    designation: str,
    scenarios: List[Dict[str, Any]],
    risks: List[Dict[str, Any]],
    scores: Dict[str, float],
) -> str:
    bal = next((s for s in scenarios if s.get("code") == "balanced"), scenarios[1] if len(scenarios) > 1 else scenarios[0])
    eco = next((s for s in scenarios if s.get("code") == "eco"), None)
    prem = next((s for s in scenarios if s.get("code") == "premium"), None)
    parts = [
        f"Analyse stratégique pour « {designation} ».",
        f"Le scénario équilibré ({bal.get('label_fr', '')}) sert de référence décisionnelle avec un coût de revient indicatif de "
        f"{bal.get('total_cost_per_unit')} TND / unité et une marge brute cible d'environ {bal.get('estimated_margin_pct')} % au prix recommandé.",
    ]
    if eco and prem:
        parts.append(
            f"L'écart entre scénario éco ({eco.get('total_cost_per_unit')} TND) et premium ({prem.get('total_cost_per_unit')} TND) "
            "matérialise le compromis qualité / coût / risque."
        )
    top_risk = next((r for r in risks if r.get("severity") == "high"), risks[0] if risks else None)
    if top_risk:
        parts.append(f"Point d'attention : {top_risk.get('titre_fr', '')} — {top_risk.get('detail_fr', '')}")
    parts.append(
        f"Scores indicatifs : efficacité coût {scores.get('cost_efficiency', 0):.0f}/100, "
        f"santé marge {scores.get('margin_health', 0):.0f}/100, viabilité produit {scores.get('product_viability', 0):.0f}/100."
    )
    return " ".join(parts)


async def llm_executive_summary(
    payload: Dict[str, Any],
    model: Optional[str] = None,
) -> Optional[str]:
    if not settings.api_key:
        return None
    mdl = (model or settings.model or "").strip() or "openai/gpt-4o-mini"
    system = (
        "Tu es un directeur général industrie luxe / maroquinerie. Rédige UNE synthèse exécutive en français, "
        "5 à 7 phrases, ton professionnel et consultant, sans liste à puces, sans markdown. "
        "Tu t'appuies sur les chiffres fournis (scénarios, scores, risques) sans les contredire."
    )
    user = json.dumps(payload, ensure_ascii=False)
    headers = {
        "Authorization": f"Bearer {settings.api_key}",
        "Content-Type": "application/json",
    }
    body = {
        "model": mdl,
        "temperature": 0.4,
        "max_tokens": 700,
        "messages": [
            {"role": "system", "content": system},
            {"role": "user", "content": user},
        ],
    }
    url = f"{settings.base_url.rstrip('/')}/chat/completions"
    try:
        async with httpx.AsyncClient(timeout=httpx.Timeout(50.0, connect=12.0)) as client:
            r = await client.post(url, headers=headers, json=body)
            r.raise_for_status()
            data = r.json()
            content = data["choices"][0]["message"].get("content", "")
            if isinstance(content, str) and content.strip():
                return content.strip()
    except Exception:
        return None
    return None
