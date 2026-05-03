from __future__ import annotations

import hashlib
import logging
import random
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageEnhance, ImageFilter, ImageStat

from app.models.schemas import GeneratedVisualItem, ProductConcept

LOCAL_GENERATION_VERSION = "year_demo_v3_product_first"
ERR_MISSING_DEMO_SOURCE_IMAGES = "missing_demo_source_images"
ERR_INVALID_DEMO_SOURCE_ASSET = "invalid_demo_source_asset"

log = logging.getLogger("fashion_oracle.local_visual_generator")

_FASHION_ORACLE_ROOT = Path(__file__).resolve().parents[2]
DEMO_SOURCES_ROOT = _FASHION_ORACLE_ROOT / "static" / "demo_sources"
GENERATED_DEMO_ROOT = _FASHION_ORACLE_ROOT / "static" / "generated_demo"
GENERATED_DEMO_ROOT.mkdir(parents=True, exist_ok=True)

_SOURCE_EXTS = (".png", ".jpg", ".jpeg", ".webp")
_SOURCE_SPECS: dict[int, tuple[str, str, str]] = {
    1: ("veste", "premium", "veste_premium"),
    2: ("veste", "technique", "veste_technique"),
    3: ("ceinture", "luxe", "ceinture_luxe"),
}

@dataclass(frozen=True)
class LocalVisualResult:
    ok: bool
    file_path: Path | None
    error_code: str = ""
    error_detail: str = ""


def _prompt_hash(s: str) -> str:
    return hashlib.sha1(s.encode("utf-8")).hexdigest()[:12]


def _source_folder(concept_index: int) -> Path:
    cat, variant, _ = _SOURCE_SPECS.get(concept_index, ("misc", "default", "concept"))
    return DEMO_SOURCES_ROOT / cat / variant


def _iter_source_images(folder: Path) -> list[Path]:
    out: list[Path] = []
    for ext in _SOURCE_EXTS:
        out.extend(sorted(folder.glob(f"*{ext}")))
    return [p for p in out if p.is_file()]


def _validate_source_asset(path: Path) -> tuple[bool, str]:
    n = path.name.lower()
    banned_markers = ("_src_", "placeholder", "abstract", "poster", "concept_board")
    if any(m in n for m in banned_markers):
        return False, "filename_marker_abstract"
    try:
        with Image.open(path) as im:
            rgb = im.convert("RGB")
            st = ImageStat.Stat(rgb)
            if not st.stddev:
                return False, "invalid_image_stats"
            avg_std = sum(float(v) for v in st.stddev[:3]) / 3.0
            if avg_std < 14.0:
                return False, "low_texture_or_flat_asset"
    except Exception as ex:  # noqa: BLE001
        return False, f"unreadable_asset:{type(ex).__name__}"
    return True, ""


def _ensure_source_folders() -> None:
    # Product-first: on cree seulement les dossiers, jamais d'images synthetiques.
    for ci in (1, 2, 3):
        _source_folder(ci).mkdir(parents=True, exist_ok=True)


def select_source_image(year: int, concept_index: int) -> Path | None:
    folder = _source_folder(concept_index)
    imgs = _iter_source_images(folder)
    if not imgs:
        return None
    rng = random.Random(year + concept_index * 97)
    return imgs[rng.randrange(len(imgs))]


def _vignette_overlay(size: tuple[int, int], strength: int) -> Image.Image:
    w, h = size
    ov = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(ov)
    for i in range(0, min(w, h) // 2, 6):
        a = int((i / max(1, min(w, h) // 2)) * strength)
        draw.rectangle((i, i, w - i, h - i), outline=(20, 20, 20, a), width=3)
    return ov


def _year_band(year: int) -> str:
    if year <= 2028:
        return "near_future"
    if year <= 2032:
        return "tech_future"
    return "concept_future"


def _future_palette(year: int, concept_index: int) -> tuple[int, int, int]:
    band = _year_band(year)
    if band == "near_future":
        base = [(198, 190, 176), (164, 170, 178), (140, 146, 154)]
    elif band == "tech_future":
        base = [(128, 154, 182), (96, 118, 146), (76, 96, 126)]
    else:
        base = [(124, 154, 196), (92, 118, 168), (72, 94, 144)]
    pick = base[(concept_index - 1) % len(base)]
    shift = (year % 7) * 3
    return (min(255, pick[0] + shift), min(255, pick[1] + shift // 2), min(255, pick[2] + shift))


def apply_year_transformations(src: Image.Image, *, year: int, concept_index: int) -> Image.Image:
    rng = random.Random(year * 1000 + concept_index * 37)
    img = src.convert("RGB")
    w, h = img.size

    # Recadrage leger pour conserver la lisibilite produit.
    crop = int(min(w, h) * (0.01 + rng.random() * 0.03))
    img = img.crop((crop, crop, w - crop, h - crop)).resize((640, 640), Image.Resampling.LANCZOS)

    band = _year_band(year)
    if band == "near_future":
        bright = 1.00 + rng.uniform(-0.02, 0.02)
        contrast = 1.06 + concept_index * 0.03 + rng.uniform(-0.02, 0.02)
        color = 0.95 + rng.uniform(-0.01, 0.03)
        sharp = 104
        blur_radius = 0.24
    elif band == "tech_future":
        bright = 0.95 + rng.uniform(-0.02, 0.01)
        contrast = 1.18 + concept_index * 0.05 + rng.uniform(-0.02, 0.03)
        color = 0.90 + rng.uniform(-0.02, 0.02)
        sharp = 132
        blur_radius = 0.30
    else:
        bright = 0.92 + rng.uniform(-0.03, 0.01)
        contrast = 1.28 + concept_index * 0.06 + rng.uniform(-0.02, 0.04)
        color = 0.87 + rng.uniform(-0.03, 0.02)
        sharp = 146
        blur_radius = 0.34
    img = ImageEnhance.Brightness(img).enhance(bright)
    img = ImageEnhance.Contrast(img).enhance(contrast)
    img = ImageEnhance.Color(img).enhance(color)

    tint = _future_palette(year, concept_index)
    overlay = Image.new("RGB", img.size, tint)
    alpha = 14 + (year % 8) * (2 if band == "concept_future" else 1)
    img = Image.blend(img, overlay, alpha / 255.0)

    img = img.filter(ImageFilter.GaussianBlur(radius=blur_radius))
    img = img.filter(ImageFilter.UnsharpMask(radius=1.4, percent=sharp, threshold=3))

    img_rgba = img.convert("RGBA")
    img_rgba.alpha_composite(_vignette_overlay(img_rgba.size, 72 if band == "near_future" else 88))
    if band == "concept_future":
        glow = Image.new("RGBA", img_rgba.size, (128, 172, 232, 8 if concept_index == 2 else 6))
        img_rgba.alpha_composite(glow)
    img = img_rgba.convert("RGB")

    if concept_index == 1:
        frame_color = (42, 44, 48) if band == "near_future" else (32, 36, 42) if band == "tech_future" else (26, 30, 38)
        framed = Image.new("RGB", (680, 680), frame_color)
        framed.paste(img, (20, 20))
        img = framed.resize((640, 640), Image.Resampling.LANCZOS)
    elif concept_index == 2:
        frame_color = (30, 34, 40) if band == "near_future" else (20, 24, 31) if band == "tech_future" else (14, 18, 26)
        framed = Image.new("RGB", (680, 680), frame_color)
        framed.paste(img, (20, 20))
        img = framed.resize((640, 640), Image.Resampling.LANCZOS)
    elif concept_index == 3:
        frame_color = (54, 48, 40) if band == "near_future" else (42, 40, 38) if band == "tech_future" else (34, 36, 40)
        framed = Image.new("RGB", (680, 680), frame_color)
        framed.paste(img, (20, 20))
        img = framed.resize((640, 640), Image.Resampling.LANCZOS)

    return img


def save_generated_visual(year: int, concept_index: int, img: Image.Image) -> Path:
    _, _, slug = _SOURCE_SPECS[concept_index]
    out_name = f"{LOCAL_GENERATION_VERSION}_{year}_{slug}.png"
    out_path = GENERATED_DEMO_ROOT / out_name
    out_path.parent.mkdir(parents=True, exist_ok=True)
    img.save(out_path, format="PNG", optimize=True)
    return out_path


def get_or_create_year_visual(year: int, concept_index: int) -> LocalVisualResult:
    _, _, slug = _SOURCE_SPECS.get(concept_index, ("misc", "default", "concept"))
    out_path = GENERATED_DEMO_ROOT / f"{LOCAL_GENERATION_VERSION}_{year}_{slug}.png"
    src_path = select_source_image(year, concept_index)
    if src_path is None:
        folder = _source_folder(concept_index)
        return LocalVisualResult(
            ok=False,
            file_path=None,
            error_code=ERR_MISSING_DEMO_SOURCE_IMAGES,
            error_detail=f"No source images in {folder}",
        )
    valid, reason = _validate_source_asset(src_path)
    if not valid:
        return LocalVisualResult(
            ok=False,
            file_path=None,
            error_code=ERR_INVALID_DEMO_SOURCE_ASSET,
            error_detail=f"{reason}:{src_path}",
        )
    if out_path.exists() and out_path.stat().st_size > 1000:
        if out_path.stat().st_mtime >= src_path.stat().st_mtime:
            log.info(
                "LOCAL_YEAR_VISUAL_CACHE_HIT concept_index=%s year=%s source=%s output=%s",
                concept_index,
                year,
                src_path,
                out_path,
            )
            return LocalVisualResult(ok=True, file_path=out_path)

    src = Image.open(src_path)
    out = apply_year_transformations(src, year=year, concept_index=concept_index)
    saved = save_generated_visual(year, concept_index, out)
    log.info(
        "LOCAL_YEAR_VISUAL_GENERATED concept_index=%s year=%s source=%s output=%s",
        concept_index,
        year,
        src_path,
        saved,
    )
    return LocalVisualResult(ok=True, file_path=saved)


def _public_url(host_base: str, filename: str) -> str:
    return f"{host_base.rstrip('/')}/static/generated_demo/{filename}"


def generate_local_year_visual_item(c: ProductConcept, host_base: str) -> GeneratedVisualItem:
    ensure_local_visual_workspace()
    res = get_or_create_year_visual(c.year, c.concept_index)
    if not res.ok or res.file_path is None:
        return GeneratedVisualItem(
            year=c.year,
            concept_index=c.concept_index,
            product_type=c.product_type,
            variant=c.variant,
            style=c.style,
            palette=c.palette,
            material=c.material,
            silhouette=c.silhouette,
            status="error",
            image_url=None,
            error_reason=res.error_code or ERR_MISSING_DEMO_SOURCE_IMAGES,
            error_detail=res.error_detail or "Missing local demo sources",
            generation_mode=LOCAL_GENERATION_VERSION,
            prompt_hash=_prompt_hash(f"err|{c.year}|{c.concept_index}"),
            cache_key=f"demo_local_year|{LOCAL_GENERATION_VERSION}|{c.year}|{c.concept_index}",
        )
    return GeneratedVisualItem(
        year=c.year,
        concept_index=c.concept_index,
        product_type=c.product_type,
        variant=c.variant,
        style=c.style,
        palette=c.palette,
        material=c.material,
        silhouette=c.silhouette,
        status="success",
        image_url=_public_url(host_base, res.file_path.name),
        cached=True,
        generation_mode=LOCAL_GENERATION_VERSION,
        prompt_hash=_prompt_hash(f"{LOCAL_GENERATION_VERSION}|{c.year}|{c.concept_index}"),
        cache_key=f"demo_local_year|{LOCAL_GENERATION_VERSION}|{c.year}|{c.concept_index}",
    )


def generate_local_year_visual_items(concepts: Iterable[ProductConcept], host_base: str) -> list[GeneratedVisualItem]:
    ensure_local_visual_workspace()
    return [generate_local_year_visual_item(c, host_base) for c in concepts]


def ensure_local_visual_workspace() -> None:
    _ensure_source_folders()
    GENERATED_DEMO_ROOT.mkdir(parents=True, exist_ok=True)
