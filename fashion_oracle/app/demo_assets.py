"""Assets PNG demo (stdlib uniquement) — demarrage rapide sans importer generative / replicate."""

from __future__ import annotations

import logging
import struct
import zlib
from pathlib import Path

log = logging.getLogger(__name__)

_FASHION_ORACLE_ROOT = Path(__file__).resolve().parent.parent
DEMO_VISUALS_DIR = _FASHION_ORACLE_ROOT / "static" / "demo_visuals"

DEMO_CONCEPT_FILES = {
    1: "veste_premium.png",
    2: "veste_technique.png",
    3: "ceinture_luxe.png",
}

_DEMO_RGB: dict[tuple[str, int], tuple[int, int, int]] = {
    ("set_a", 1): (188, 168, 138),
    ("set_a", 2): (72, 98, 128),
    ("set_a", 3): (132, 96, 68),
    ("set_b", 1): (178, 148, 188),
    ("set_b", 2): (88, 118, 108),
    ("set_b", 3): (118, 88, 72),
    ("set_c", 1): (158, 178, 198),
    ("set_c", 2): (98, 82, 108),
    ("set_c", 3): (108, 92, 78),
}


def demo_visual_tier_for_year(year: int) -> str:
    """Palier visuel demo : 2026–2028 A, 2029–2032 B, 2033+ C."""
    if year <= 2028:
        return "set_a"
    if year <= 2032:
        return "set_b"
    return "set_c"


def demo_visual_relative_path(concept_index: int, year: int) -> str:
    tier = demo_visual_tier_for_year(year)
    fn = DEMO_CONCEPT_FILES[concept_index]
    return f"{tier}/{fn}"


def _write_flat_color_png(path: Path, rgb: tuple[int, int, int], w: int = 512, h: int = 512) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)

    def chunk(tag: bytes, data: bytes) -> bytes:
        crc = zlib.crc32(tag + data) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", crc)

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    row = bytes([0]) + bytes(rgb) * w
    raw = row * h
    compressed = zlib.compress(raw, 9)
    png = sig + chunk(b"IHDR", ihdr) + chunk(b"IDAT", compressed) + chunk(b"IEND", b"")
    path.write_bytes(png)


def ensure_demo_visual_png_files() -> None:
    """Cree les 9 PNG (3 concepts x 3 paliers annee) si absents — aucune API."""
    # Fichiers "vitrine" racine demandes par le produit (set A par defaut).
    for ci in (1, 2, 3):
        root_name = DEMO_CONCEPT_FILES[ci]
        root_file = DEMO_VISUALS_DIR / root_name
        if not (root_file.exists() and root_file.stat().st_size > 500):
            rgb_root = _DEMO_RGB.get(("set_a", ci), (160, 160, 160))
            _write_flat_color_png(root_file, rgb_root, 512, 512)
            log.info("DEMO_ASSET_SEEDED path=%s", root_file)

    for tier in ("set_a", "set_b", "set_c"):
        for ci in (1, 2, 3):
            rel = f"{tier}/{DEMO_CONCEPT_FILES[ci]}"
            p = DEMO_VISUALS_DIR / rel
            if p.exists() and p.stat().st_size > 500:
                continue
            rgb = _DEMO_RGB.get((tier, ci), (160, 160, 160))
            _write_flat_color_png(p, rgb, 512, 512)
            log.info("DEMO_ASSET_SEEDED path=%s", p)
