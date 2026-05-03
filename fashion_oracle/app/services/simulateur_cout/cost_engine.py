"""
CostEngine — baseline economics from minimal inputs.

Pure functions: unit cost proxy, implied margin vs recommended price,
and a structural risk index (volume / volatility heuristics).
"""

from __future__ import annotations

import math
from dataclasses import dataclass


@dataclass(frozen=True)
class BaselineEconomics:
    cout_unitaire: float
    marge_pct: float
    risque_base: float
    prix_conseille: float


def _productivite_volume(volume: int) -> float:
    q = max(1, int(volume))
    return float(min(1.12, max(0.88, 0.91 + 0.024 * math.log(q))))


def compute_baseline(*, cout_matiere: float, volume: int) -> BaselineEconomics:
    """
    Deterministic cost stack from material + volume-driven labour/overhead proxy.
    Not a full industrial BOM: enough signal for optimisation / sensitivity layers.
    """
    mat = max(0.01, float(cout_matiere))
    q = max(1, int(volume))
    prod = _productivite_volume(q)
    # Labour + structure proxy scales with sqrt(material) and inverse productivity
    mo_proxy = 6.5 + math.sqrt(mat) * 2.8
    structure = mat * 0.18 + (12.0 / prod)
    cout = mat * 1.14 + mo_proxy / prod + structure
    coeff_prix = 2.42 if q < 350 else 2.28
    prix = max(cout * 1.05, cout * coeff_prix)
    marge_pct = max(0.0, min(92.0, 100.0 * (1.0 - cout / prix)))
    # Risk: low volume + high material share => higher exposure
    part_mp = mat / max(cout, 1e-6)
    risque = min(
        100.0,
        18.0 + 42.0 * part_mp + max(0.0, 520 - q) * 0.06 + (100.0 - marge_pct) * 0.22,
    )
    return BaselineEconomics(
        cout_unitaire=round(cout, 4),
        marge_pct=round(marge_pct, 2),
        risque_base=round(risque, 2),
        prix_conseille=round(prix, 2),
    )


def apply_variant_multipliers(
    baseline_mat: float,
    volume: int,
    *,
    mat_mult: float = 1.0,
    efficacite_mult: float = 1.0,
    volume_effectif: int | None = None,
    overhead_mult: float = 1.0,
) -> BaselineEconomics:
    """Re-evaluate baseline with internal variant knobs (used by VariantGenerator)."""
    v = int(volume if volume_effectif is None else volume_effectif)
    mat = max(0.01, baseline_mat * mat_mult)
    base = compute_baseline(cout_matiere=mat, volume=max(1, v))
    adj_cout = base.cout_unitaire * (1.0 / max(0.75, efficacite_mult)) * overhead_mult
    prix = base.prix_conseille * (0.98 + 0.02 * min(1.2, efficacite_mult))
    marge_pct = max(0.0, min(92.0, 100.0 * (1.0 - adj_cout / max(prix, 1e-6))))
    risque = min(100.0, base.risque_base * (0.92 + 0.08 * overhead_mult))
    return BaselineEconomics(
        cout_unitaire=round(adj_cout, 4),
        marge_pct=round(marge_pct, 2),
        risque_base=round(risque, 2),
        prix_conseille=round(prix, 2),
    )
