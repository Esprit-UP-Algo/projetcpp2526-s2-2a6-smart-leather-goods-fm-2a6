"""Simulateur de coûts maroquinerie / confection : moteur déterministe + enrichissement LLM (OpenRouter)."""
from __future__ import annotations

import json
import math
import re
from typing import Any, Optional

import httpx

from app.config import settings


def _strip_json_fence(content: str) -> str:
    cleaned = content.strip()
    if cleaned.startswith("```"):
        cleaned = re.sub(r"^```[a-zA-Z]*\s*", "", cleaned)
        cleaned = re.sub(r"\s*```$", "", cleaned).strip()
    return cleaned


def _famille_industrialisation(key: str) -> dict[str, Any]:
    k = (key or "maroquinerie_petite").lower().strip().replace(" ", "_").replace("-", "_")
    profiles: dict[str, dict[str, Any]] = {
        "sac_main": {
            "code": "sac_main",
            "libelle": "Sac à main — assemblages & renforts multiples",
            "coeff_temps": 1.14,
            "rebut_pct_points": 0.35,
            "outillage_lot_conseille": 220.0,
        },
        "maroquinerie_petite": {
            "code": "maroquinerie_petite",
            "libelle": "Petite maroquinerie (porte-monnaie, étui, accessoire plat)",
            "coeff_temps": 1.0,
            "rebut_pct_points": 0.0,
            "outillage_lot_conseille": 150.0,
        },
        "ceinture": {
            "code": "ceinture",
            "libelle": "Ceinture — gamme courte, boucle / perforation",
            "coeff_temps": 0.84,
            "rebut_pct_points": -0.12,
            "outillage_lot_conseille": 90.0,
        },
        "accessoire_metal": {
            "code": "accessoire_metal",
            "libelle": "Accessoire forte quincaillerie / pièce métal",
            "coeff_temps": 1.06,
            "rebut_pct_points": 0.22,
            "outillage_lot_conseille": 130.0,
        },
    }
    return profiles.get(k, profiles["maroquinerie_petite"])


def _finition_params(key: str) -> dict[str, Any]:
    k = (key or "premium").lower().strip()
    if k == "luxe":
        return {"code": "luxe", "libelle": "Luxe", "cq_mult": 1.32, "emb_mult": 1.45}
    if k == "standard":
        return {"code": "standard", "libelle": "Standard", "cq_mult": 0.88, "emb_mult": 0.95}
    return {"code": "premium", "libelle": "Premium", "cq_mult": 1.0, "emb_mult": 1.08}


def _tension_marche_params(key: str) -> dict[str, Any]:
    k = (key or "base").lower().strip()
    if "seren" in k:
        return {
            "code": "serenite",
            "libelle": "Sérénité fournisseurs",
            "choc_matiere_pessim": 0.08,
            "gain_matiere_optim": 0.06,
            "rebut_pct_points": -0.18,
        }
    if "tens" in k:
        return {
            "code": "tension",
            "libelle": "Tension matières & volatilité change",
            "choc_matiere_pessim": 0.19,
            "gain_matiere_optim": 0.02,
            "rebut_pct_points": 0.55,
        }
    return {
        "code": "base",
        "libelle": "Base",
        "choc_matiere_pessim": 0.12,
        "gain_matiere_optim": 0.05,
        "rebut_pct_points": 0.0,
    }


def _productivite_serie(q: int) -> float:
    """Courbe d'efficience série : petites séries moins productives, plateau progressif."""
    q = max(1, int(q))
    return float(min(1.12, max(0.88, 0.91 + 0.024 * math.log(q))))


def compute_engineering_breakdown(
    *,
    cout_matiere_dt: float,
    temps_fabrication_h: float,
    quantite_serie: int,
    taux_horaire_dt: float,
    charges_sociales_pct: float,
    frais_generaux_pct: float,
    defaut_rebut_pct: float,
    emballage_unitaire_dt: float,
    logistique_lot_dt: float,
    amortissement_outillage_lot_dt: float,
    controle_qualite_pct: float,
    canal: str,
    famille_article: str = "maroquinerie_petite",
    niveau_finition: str = "premium",
    tension_marche: str = "base",
    delai_rotation_stock_j: int = 42,
    taux_financement_annuel_pct: float = 9.5,
    pct_energie_maintenance: float = 1.85,
) -> dict[str, Any]:
    """Coût de revient unitaire (TND) + couche métier industrialisation (hors simple CRUD)."""
    q = max(1, int(quantite_serie))
    canal_u = (canal or "B2C").strip().upper()
    if canal_u.startswith("B2B") or "WHOLESALE" in canal_u or "EXPORT" in canal_u:
        canal_mult = 1.12
        canal_label = "B2B"
    else:
        canal_mult = 1.0
        canal_label = "B2C"

    fam = _famille_industrialisation(famille_article)
    fin = _finition_params(niveau_finition)
    tens = _tension_marche_params(tension_marche)

    prod_serie = _productivite_serie(q)
    temps_effectif_h = max(0.0, float(temps_fabrication_h)) * float(fam["coeff_temps"]) / prod_serie

    main_oeuvre_brute = temps_effectif_h * max(0.01, float(taux_horaire_dt))
    charges_sociales = main_oeuvre_brute * (max(0.0, float(charges_sociales_pct)) / 100.0)
    main_oeuvre_chargee = main_oeuvre_brute + charges_sociales

    mat = max(0.0, float(cout_matiere_dt))
    rebut_pct_eff = max(
        0.0,
        float(defaut_rebut_pct) + float(fam["rebut_pct_points"]) + float(tens["rebut_pct_points"]),
    )
    cq_pct_eff = max(0.0, float(controle_qualite_pct) * float(fin["cq_mult"]))

    energie_maint = (mat + main_oeuvre_brute) * (max(0.0, float(pct_energie_maintenance)) / 100.0)
    d_rot = max(0, int(delai_rotation_stock_j))
    finance_stock_mp = mat * (max(0.0, float(taux_financement_annuel_pct)) / 100.0) * (d_rot / 360.0)

    base_core = mat + main_oeuvre_chargee + energie_maint + finance_stock_mp
    frais_generaux = base_core * (max(0.0, float(frais_generaux_pct)) / 100.0)
    rebut = base_core * (rebut_pct_eff / 100.0)
    cq = base_core * (cq_pct_eff / 100.0)

    emballage = max(0.0, float(emballage_unitaire_dt)) * float(fin["emb_mult"])
    logistique_par_unite = max(0.0, float(logistique_lot_dt)) / q
    amort_outillage_par_unite = max(0.0, float(amortissement_outillage_lot_dt)) / q

    base_sans_canal = base_core + frais_generaux + rebut + cq
    base_sans_canal += emballage + logistique_par_unite + amort_outillage_par_unite
    ajustement_canal = base_sans_canal * (canal_mult - 1.0)
    sous_total = base_sans_canal + ajustement_canal

    coeff_marge_conseille = 2.35 if canal_mult > 1.0 else 2.55

    line_items: list[dict[str, Any]] = [
        {
            "code": "MP",
            "label_fr": "Matières & approvisionnement (valeur stock)",
            "montant_dt": round(mat, 4),
            "detail": "Cuir, quincaillerie, consommables — hors négociation volume.",
        },
        {
            "code": "MOD",
            "label_fr": "Main-d'œuvre directe (temps gamme × taux)",
            "montant_dt": round(main_oeuvre_brute, 4),
            "detail": (
                f"Temps nominal {temps_fabrication_h} h → effectif {temps_effectif_h:.2f} h "
                f"(coeff. gamme {fam['coeff_temps']:.2f} ÷ productivité série {prod_serie:.2f})."
            ),
        },
        {
            "code": "CHG_SOC",
            "label_fr": "Charges sociales & employeur (estimation)",
            "montant_dt": round(charges_sociales, 4),
            "detail": f"{charges_sociales_pct}% sur MOD brute.",
        },
        {
            "code": "ENER_MAINT",
            "label_fr": "Énergie, maintenance & petit outillage courant",
            "montant_dt": round(energie_maint, 4),
            "detail": f"{pct_energie_maintenance}% sur (matière + MOD brute).",
        },
        {
            "code": "FIN_STOCK",
            "label_fr": "Coût de détention stock MP (financement)",
            "montant_dt": round(finance_stock_mp, 4),
            "detail": f"{taux_financement_annuel_pct}% annuel × {d_rot} j / 360 sur matière.",
        },
        {
            "code": "FG",
            "label_fr": "Frais généraux atelier (structure)",
            "montant_dt": round(frais_generaux, 4),
            "detail": f"{frais_generaux_pct}% sur socle direct (MP+MOD+énergie+fin. stock).",
        },
        {
            "code": "REBUT",
            "label_fr": "Provision rebut / retouches (métier + tension marché)",
            "montant_dt": round(rebut, 4),
            "detail": f"Taux effectif {rebut_pct_eff:.2f}% (saisie + gamme + scénario marché).",
        },
        {
            "code": "CQ",
            "label_fr": "Contrôle qualité & métrologie",
            "montant_dt": round(cq, 4),
            "detail": f"{cq_pct_eff:.2f}% effectif (finition {fin['libelle']}).",
        },
        {
            "code": "EMB",
            "label_fr": "Emballage & présentation unitaire",
            "montant_dt": round(emballage, 4),
            "detail": f"Coefficient finition ×{fin['emb_mult']:.2f}.",
        },
        {
            "code": "LOG",
            "label_fr": "Logistique (lot / série)",
            "montant_dt": round(logistique_par_unite, 4),
            "detail": f"{logistique_lot_dt} TND / {q} unités.",
        },
        {
            "code": "OUT",
            "label_fr": "Amortissement outillage & gabarits (lot)",
            "montant_dt": round(amort_outillage_par_unite, 4),
            "detail": f"{amortissement_outillage_lot_dt} TND / {q} unités.",
        },
    ]
    if abs(ajustement_canal) > 1e-9:
        line_items.append(
            {
                "code": "CANAL",
                "label_fr": "Ajustement canal (B2B : docs, échantillons, conformité export)",
                "montant_dt": round(ajustement_canal, 4),
                "detail": f"Facteur ×{canal_mult:.2f} sur sous-total logistique & industrialisation.",
            }
        )

    cout_revient_unitaire = round(sous_total, 4)
    prix_public_conseille = round(cout_revient_unitaire * coeff_marge_conseille, 4)
    marge_brute_pct = (
        round(100.0 * (prix_public_conseille - cout_revient_unitaire) / prix_public_conseille, 2)
        if prix_public_conseille > 0
        else 0.0
    )

    dmc_ratio = round(100.0 * mat / cout_revient_unitaire, 2) if cout_revient_unitaire > 0 else 0.0
    va_unitaire = round(cout_revient_unitaire - mat, 4)
    taux_va = round(100.0 * va_unitaire / cout_revient_unitaire, 2) if cout_revient_unitaire > 0 else 0.0

    choc_p = float(tens["choc_matiere_pessim"])
    gain_o = float(tens["gain_matiere_optim"])
    pessim_core = base_sans_canal + mat * choc_p + main_oeuvre_brute * 0.07
    optim_core = base_sans_canal - mat * gain_o - main_oeuvre_brute * 0.035
    pessim = pessim_core * canal_mult
    optim = optim_core * canal_mult

    metier = {
        "reference_gamme": fam["libelle"],
        "famille_code": fam["code"],
        "finition": fin["libelle"],
        "tension_marche": tens["libelle"],
        "productivite_serie": round(prod_serie, 4),
        "temps_effectif_h": round(temps_effectif_h, 4),
        "kpis": {
            "part_matiere_dans_crv_pct": dmc_ratio,
            "valeur_ajoutee_unitaire_dt": va_unitaire,
            "taux_valeur_ajoutee_pct": taux_va,
        },
        "coefficients": {
            "famille_temps": fam["coeff_temps"],
            "rebut_points_ajoute": fam["rebut_pct_points"] + tens["rebut_pct_points"],
        },
        "recommandations_outillage_lot_dt": float(fam["outillage_lot_conseille"]),
    }

    return {
        "devise": "TND",
        "quantite_serie": q,
        "canal": canal_label,
        "lignes": line_items,
        "metier": metier,
        "totaux": {
            "cout_revient_unitaire_dt": cout_revient_unitaire,
            "cout_revient_serie_dt": round(cout_revient_unitaire * q, 4),
            "prix_public_conseille_unitaire_dt": prix_public_conseille,
            "coefficient_marge_multiplicateur": round(coeff_marge_conseille, 3),
            "marge_brute_indicative_pct": marge_brute_pct,
        },
        "scenarios": {
            "pessimiste_unitaire_dt": round(max(0.0, pessim), 4),
            "attendu_unitaire_dt": cout_revient_unitaire,
            "optimiste_unitaire_dt": round(max(0.0, optim), 4),
        },
        "hypotheses": {
            "taux_horaire_dt": taux_horaire_dt,
            "temps_fabrication_h": temps_fabrication_h,
            "charges_sociales_pct": charges_sociales_pct,
            "frais_generaux_pct": frais_generaux_pct,
            "defaut_rebut_pct": defaut_rebut_pct,
            "controle_qualite_pct": controle_qualite_pct,
            "famille_article": fam["code"],
            "niveau_finition": fin["code"],
            "tension_marche": tens["code"],
            "delai_rotation_stock_j": d_rot,
            "taux_financement_annuel_pct": taux_financement_annuel_pct,
            "pct_energie_maintenance": pct_energie_maintenance,
        },
    }


async def llm_enrich_cost_analysis(
    *,
    designation: str,
    collection: str,
    type_cuir: str,
    notes_atelier: str,
    engineering: dict[str, Any],
    model: Optional[str] = None,
) -> Optional[dict[str, Any]]:
    if not settings.api_key:
        return None

    mdl = (model or settings.model or "").strip() or "openai/gpt-4o-mini"
    system_msg = (
        "Tu es un directeur industrialisation et sourcing maroquinerie luxe (Tunisie / UE). "
        "Tu raisonnes en gamme de fabrication, goulets d'étranglement atelier, politique prix canal, "
        "et risques cuir/quincaillerie. Tu réponds UNIQUEMENT en JSON UTF-8 valide, sans markdown. "
        "Clés obligatoires : "
        "resume_executif (string, 3-5 phrases en français, ton conseil direction), "
        "leviers_optimisation (array d'objets {action, impact_estime, priorite 1-5}), "
        "risques_supply_chain (array de strings), "
        "indices_benchmark (objet avec optional prix_marche_baseline_dt, commentaire), "
        "checklist_qualite (array de strings courts), "
        "message_conformite (string courte : normes cuir, REACH, étiquetage), "
        "sequence_operationnelle (array max 6 strings : ordre type coupe → parage → assemblage…). "
        "N'invente pas de chiffres contradictoires avec engineering.totaux et engineering.lignes ; "
        "ne recalcule pas un coût de revient : commente les écarts possibles vs marché."
    )
    user_payload = {
        "produit": {
            "designation": designation,
            "collection": collection,
            "type_cuir": type_cuir,
            "notes_atelier": notes_atelier,
        },
        "contexte_industrialisation": engineering.get("metier"),
        "engineering": engineering,
    }

    headers = {
        "Authorization": f"Bearer {settings.api_key}",
        "Content-Type": "application/json",
    }
    payload = {
        "model": mdl,
        "temperature": 0.35,
        "max_tokens": 1800,
        "messages": [
            {"role": "system", "content": system_msg},
            {"role": "user", "content": json.dumps(user_payload, ensure_ascii=False)},
        ],
    }
    url = f"{settings.base_url.rstrip('/')}/chat/completions"
    try:
        async with httpx.AsyncClient(timeout=httpx.Timeout(55.0, connect=15.0)) as client:
            resp = await client.post(url, headers=headers, json=payload)
            resp.raise_for_status()
            data = resp.json()
            content = data["choices"][0]["message"].get("content", "")
            if not isinstance(content, str) or not content.strip():
                return None
            cleaned = _strip_json_fence(content)
            parsed = json.loads(cleaned)
            return parsed if isinstance(parsed, dict) else None
    except Exception:
        return None


async def run_cost_simulation(
    *,
    designation: str,
    cout_matiere_dt: float,
    temps_fabrication_h: float,
    collection: str = "",
    type_cuir: str = "",
    quantite_serie: int = 100,
    taux_horaire_dt: float = 15.5,
    charges_sociales_pct: float = 18.0,
    frais_generaux_pct: float = 22.0,
    defaut_rebut_pct: float = 2.5,
    emballage_unitaire_dt: float = 0.85,
    logistique_lot_dt: float = 40.0,
    amortissement_outillage_lot_dt: float = 150.0,
    controle_qualite_pct: float = 1.2,
    canal: str = "B2C",
    notes_atelier: str = "",
    skip_llm: bool = False,
    cost_model: Optional[str] = None,
    famille_article: str = "maroquinerie_petite",
    niveau_finition: str = "premium",
    tension_marche: str = "base",
    delai_rotation_stock_j: int = 42,
    taux_financement_annuel_pct: float = 9.5,
    pct_energie_maintenance: float = 1.85,
) -> dict[str, Any]:
    fam_key = (famille_article or "maroquinerie_petite").strip() or "maroquinerie_petite"
    fin_key = (niveau_finition or "premium").strip() or "premium"
    ten_key = (tension_marche or "base").strip() or "base"

    engineering = compute_engineering_breakdown(
        cout_matiere_dt=cout_matiere_dt,
        temps_fabrication_h=temps_fabrication_h,
        quantite_serie=quantite_serie,
        taux_horaire_dt=taux_horaire_dt,
        charges_sociales_pct=charges_sociales_pct,
        frais_generaux_pct=frais_generaux_pct,
        defaut_rebut_pct=defaut_rebut_pct,
        emballage_unitaire_dt=emballage_unitaire_dt,
        logistique_lot_dt=logistique_lot_dt,
        amortissement_outillage_lot_dt=amortissement_outillage_lot_dt,
        controle_qualite_pct=controle_qualite_pct,
        canal=canal,
        famille_article=fam_key,
        niveau_finition=fin_key,
        tension_marche=ten_key,
        delai_rotation_stock_j=delai_rotation_stock_j,
        taux_financement_annuel_pct=taux_financement_annuel_pct,
        pct_energie_maintenance=pct_energie_maintenance,
    )

    warnings: list[str] = []
    if cout_matiere_dt <= 0:
        warnings.append("Coût matière nul ou absent : la simulation repose uniquement sur la main-d'œuvre et les charges.")
    if temps_fabrication_h <= 0:
        warnings.append("Temps de fabrication nul : vérifiez la fiche produit (heures atelier).")

    llm_model = (cost_model or "").strip() or None
    llm_part: Optional[dict[str, Any]] = None
    if not skip_llm and settings.api_key:
        llm_part = await llm_enrich_cost_analysis(
            designation=designation,
            collection=collection,
            type_cuir=type_cuir,
            notes_atelier=notes_atelier,
            engineering=engineering,
            model=llm_model,
        )
    elif not settings.api_key:
        warnings.append(
            "Clé API absente (FASHION_ORACLE_API_KEY). Configurez le fichier .env du backend pour activer l'analyse IA."
        )

    meta = {
        "llm_active": llm_part is not None,
        "model": llm_model or settings.model,
    }
    return {
        "designation": designation,
        "collection": collection,
        "type_cuir": type_cuir,
        "famille_article": fam_key,
        "niveau_finition": fin_key,
        "tension_marche": ten_key,
        "engineering": engineering,
        "llm_analysis": llm_part,
        "warnings": warnings,
        "meta": meta,
    }
