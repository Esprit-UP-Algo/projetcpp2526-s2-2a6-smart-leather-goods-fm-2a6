from dataclasses import dataclass
from typing import Dict, List
import numpy as np
import pandas as pd


@dataclass
class SimulatedCorpus:
    timeline: pd.DataFrame
    style_taxonomy: Dict[str, Dict[str, List[str]]]


def _style_catalog() -> Dict[str, Dict[str, List[str]]]:
    return {
        "minimal-tailoring": {
            "colors": ["beige", "taupe", "graphite", "off-white"],
            "fabrics": ["wool", "cashmere", "gabardine"],
            "silhouettes": ["structured coat", "wide-leg trouser", "clean blazer"],
        },
        "neo-vintage": {
            "colors": ["terracotta", "olive", "burnt-orange", "cream"],
            "fabrics": ["corduroy", "denim", "suede"],
            "silhouettes": ["cropped jacket", "high waist", "retro trench"],
        },
        "athlux-utility": {
            "colors": ["charcoal", "neon-lime", "steel-blue", "black"],
            "fabrics": ["technical-nylon", "mesh", "stretch-cotton"],
            "silhouettes": ["cargo overshirt", "utility vest", "hybrid hoodie"],
        },
        "romantic-fluid": {
            "colors": ["rose-dust", "lavender", "pearl", "sage"],
            "fabrics": ["silk", "chiffon", "lace"],
            "silhouettes": ["bias-cut dress", "soft drape", "flowing sleeve"],
        },
        "artisan-leather": {
            "colors": ["cognac", "espresso", "sand", "oxblood"],
            "fabrics": ["full-grain leather", "nubuck", "raw linen"],
            "silhouettes": ["boxy jacket", "modular bag", "belted tunic"],
        },
    }


_DEFAULT_CORPUS: SimulatedCorpus | None = None


def build_simulated_corpus(start: int = 1990, end: int = 2038, seed: int = 42) -> SimulatedCorpus:
    """Corpus simule (memoise pour les parametres par defaut — prediction rapide)."""
    global _DEFAULT_CORPUS
    if start == 1990 and end == 2038 and seed == 42 and _DEFAULT_CORPUS is not None:
        return _DEFAULT_CORPUS

    rng = np.random.default_rng(seed)
    styles = list(_style_catalog().keys())
    years = np.arange(start, end + 1)

    rows = []
    for year in years:
        t = (year - start) / max(1, (end - start))
        cycle20 = np.sin(2 * np.pi * (year - start) / 20.0)
        cycle8 = np.sin(2 * np.pi * (year - start) / 8.0)

        for idx, style in enumerate(styles):
            base = 45 + 9 * idx
            drift = 24 * t * (1 if idx % 2 == 0 else -0.6)
            cyc = (12 + 3 * idx) * cycle20 + (6 - idx) * cycle8
            noise = rng.normal(0, 4)
            score = max(1.0, base + drift + cyc + noise)

            image_signal = score + rng.normal(0, 2)
            text_signal = score * (0.8 + rng.normal(0, 0.04))

            rows.append(
                {
                    "year": int(year),
                    "style": style,
                    "score": float(score),
                    "image_signal": float(image_signal),
                    "text_signal": float(text_signal),
                }
            )

    corpus = SimulatedCorpus(timeline=pd.DataFrame(rows), style_taxonomy=_style_catalog())
    if start == 1990 and end == 2038 and seed == 42:
        _DEFAULT_CORPUS = corpus
    return corpus