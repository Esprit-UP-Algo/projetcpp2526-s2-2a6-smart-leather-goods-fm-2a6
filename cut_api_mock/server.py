from __future__ import annotations

from typing import List

from fastapi import FastAPI
from pydantic import BaseModel, Field


class PieceIn(BaseModel):
    name: str
    width_mm: float = Field(gt=0)
    height_mm: float = Field(gt=0)
    qty: int = Field(ge=1, default=1)
    can_rotate: bool = True


class CutPlanRequest(BaseModel):
    order_id: int
    id_produit: int
    id_matiere: int
    spacing_mm: float = Field(ge=0, default=4.0)
    sheet_width_mm: float = Field(gt=0)
    sheet_height_mm: float = Field(gt=0)
    pieces: List[PieceIn]


app = FastAPI(title="FIL D'OR - Mock IA Decoupe")


@app.get("/health")
def health() -> dict:
    return {"ok": True}


@app.post("/api/v1/generate-cut-plan")
def generate_cut_plan(req: CutPlanRequest) -> dict:
    spacing = req.spacing_mm
    sheet_w = req.sheet_width_mm
    sheet_h = req.sheet_height_mm

    units: List[PieceIn] = []
    for p in req.pieces:
        units.extend([p] * p.qty)

    # Simple placement "shelf" (mock) pour test E2E Qt
    placements = []
    sheet_index = 0
    x = 0.0
    y = 0.0
    shelf_h = 0.0

    used_area = 0.0

    def to_color(name: str) -> str:
        n = name.lower()
        if "face" in n:
            return "#5b6ee1"
        if "dos" in n:
            return "#ef6a8b"
        if "rabat" in n:
            return "#ffaf5f"
        if "poche" in n:
            return "#48e38f"
        return "#76a7ff"

    for p in sorted(units, key=lambda u: max(u.width_mm, u.height_mm), reverse=True):
        w = p.width_mm
        h = p.height_mm
        rotated = False

        # rotation simple si utile
        if p.can_rotate and w > sheet_w and h <= sheet_w and w <= sheet_h:
            w, h = h, w
            rotated = True

        # nouvelle ligne (shelf)
        if x + w > sheet_w:
            x = 0.0
            y += shelf_h + spacing
            shelf_h = 0.0

        # nouvelle feuille
        if y + h > sheet_h:
            sheet_index += 1
            x = 0.0
            y = 0.0
            shelf_h = 0.0

        placements.append(
            {
                "name": p.name,
                "sheet_index": sheet_index,
                "x_mm": round(x, 3),
                "y_mm": round(y, 3),
                "w_mm": round(w, 3),
                "h_mm": round(h, 3),
                "rotated": rotated,
                "color": to_color(p.name),
            }
        )
        used_area += w * h
        x += w + spacing
        shelf_h = max(shelf_h, h)

    sheets_used = sheet_index + 1
    total_area = sheet_w * sheet_h * sheets_used
    waste_area = max(0.0, total_area - used_area)
    waste_pct = (waste_area * 100.0 / total_area) if total_area > 0 else 0.0

    # Format compatible avec ton parsing Qt (root direct + "plan")
    plan = {
        "placements": placements,
        "sheets_used": sheets_used,
        "sheet_width_mm": sheet_w,
        "sheet_height_mm": sheet_h,
        "used_area_mm2": round(used_area, 3),
        "total_area_mm2": round(total_area, 3),
        "waste_area_mm2": round(waste_area, 3),
        "waste_pct": round(waste_pct, 3),
        "waste_percentage": round(waste_pct, 3),
    }
    out = dict(plan)
    out["plan"] = plan
    return out
