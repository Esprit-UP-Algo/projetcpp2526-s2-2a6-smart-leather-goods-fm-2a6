from __future__ import annotations
from dataclasses import dataclass
from typing import Dict, List
import numpy as np
import pandas as pd


@dataclass
class TemporalForecast:
    scores: Dict[str, float]
    confidence: float
    similar_decades: List[str]


def _fourier_regression(years: np.ndarray, values: np.ndarray, target_year: int) -> float:
    t = years.astype(float)
    X = np.column_stack(
        [
            np.ones_like(t),
            t,
            np.sin(2 * np.pi * t / 20.0),
            np.cos(2 * np.pi * t / 20.0),
            np.sin(2 * np.pi * t / 8.0),
            np.cos(2 * np.pi * t / 8.0),
        ]
    )
    beta = np.linalg.pinv(X) @ values
    x_t = np.array(
        [
            1.0,
            float(target_year),
            np.sin(2 * np.pi * target_year / 20.0),
            np.cos(2 * np.pi * target_year / 20.0),
            np.sin(2 * np.pi * target_year / 8.0),
            np.cos(2 * np.pi * target_year / 8.0),
        ]
    )
    return float(x_t @ beta)


def forecast_styles(df: pd.DataFrame, target_year: int) -> TemporalForecast:
    style_scores: Dict[str, float] = {}
    errors = []

    for style, grp in df.groupby("style"):
        grp = grp.sort_values("year")
        y = grp["score"].to_numpy(dtype=float)
        x = grp["year"].to_numpy(dtype=float)
        if len(y) < 6:
            pred = float(np.mean(y) if len(y) else 0.0)
            style_scores[style] = pred
            continue

        pred = _fourier_regression(x, y, target_year)
        backtest_pred = _fourier_regression(x[:-1], y[:-1], int(x[-1]))
        errors.append(abs(backtest_pred - y[-1]) / max(1.0, y[-1]))
        style_scores[style] = max(0.0, pred)

    mae = float(np.mean(errors)) if errors else 0.3
    confidence = float(max(0.15, min(0.99, 1.0 - mae)))

    decade = (target_year // 10) * 10
    similar_decades = [f"{decade-20}s", f"{decade-10}s", f"{decade}s"]
    return TemporalForecast(scores=style_scores, confidence=confidence, similar_decades=similar_decades)
