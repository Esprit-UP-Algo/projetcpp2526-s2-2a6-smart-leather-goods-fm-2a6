from __future__ import annotations
from typing import Dict, List
import matplotlib.pyplot as plt
import io
import base64


def _fig_to_base64() -> str:
    buf = io.BytesIO()
    plt.savefig(buf, format="png", bbox_inches="tight", dpi=130)
    plt.close()
    return base64.b64encode(buf.getvalue()).decode("utf-8")


def trend_chart(style_year_series: Dict[str, List[float]], years: List[int]) -> str:
    plt.figure(figsize=(8, 3.4))
    for style, vals in style_year_series.items():
        plt.plot(years, vals, label=style)
    plt.title("Fashion Oracle - Evolution temporelle")
    plt.xlabel("Annee")
    plt.ylabel("Trend score")
    plt.grid(alpha=0.25)
    plt.legend(loc="upper left", fontsize=7)
    return _fig_to_base64()


def cycle_chart(years: List[int], values: List[float]) -> str:
    plt.figure(figsize=(8, 3.2))
    plt.plot(years, values, color="#8d5524", linewidth=2)
    plt.fill_between(years, values, alpha=0.15, color="#d4af37")
    plt.title("Fashion Oracle - Cycle mode")
    plt.xlabel("Annee")
    plt.ylabel("Cycle index")
    plt.grid(alpha=0.25)
    return _fig_to_base64()
