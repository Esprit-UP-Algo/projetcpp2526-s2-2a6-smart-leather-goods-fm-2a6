"""
VariantGenerator — explores the configuration space without user CRUD.

Produces >= 20 internal candidates combining material, efficiency,
volume effect, and overhead levers (discrete grid + small perturbations).
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import List


@dataclass(frozen=True)
class VariantSpec:
    code: str
    nom: str
    mat_mult: float
    efficacite_mult: float
    volume_delta_pct: float  # applied as effective volume multiplier intent
    overhead_mult: float


def _volume_from_delta(base_volume: int, delta_pct: float) -> int:
    v = int(round(base_volume * (1.0 + delta_pct / 100.0)))
    return max(1, min(1_000_000, v))


def generate_variants(*, cout_matiere: float, volume: int) -> List[VariantSpec]:
    """At least 20 deterministic variants from one commercial snapshot."""
    base_v = max(1, int(volume))
    mats = [0.94, 0.97, 1.0, 1.03, 1.06, 1.09]
    effs = [0.92, 0.96, 1.0, 1.04, 1.08]
    vols = [-12.0, -6.0, 0.0, 6.0, 12.0, 18.0]
    ovs = [0.94, 0.98, 1.02, 1.06]
    out: List[VariantSpec] = []
    idx = 0
    for mm in mats:
        for em in effs:
            for vd in vols:
                for om in ovs:
                    if len(out) >= 28:
                        return out
                    idx += 1
                    out.append(
                        VariantSpec(
                            code=f"V{idx:03d}",
                            nom=f"Variante {idx} (matx{mm:.2f} effx{em:.2f} vol{vd:+.0f}% ovx{om:.2f})",
                            mat_mult=mm,
                            efficacite_mult=em,
                            volume_delta_pct=vd,
                            overhead_mult=om,
                        )
                    )
    return out
