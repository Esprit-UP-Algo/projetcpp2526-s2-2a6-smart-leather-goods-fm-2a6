from __future__ import annotations
from dataclasses import dataclass
from typing import Dict, List
import numpy as np
import pandas as pd
from sklearn.cluster import KMeans
from sklearn.decomposition import PCA


@dataclass
class TrendSnapshot:
    style_scores: Dict[str, float]
    cluster_labels: Dict[str, int]
    pca_components: Dict[str, List[float]]


def build_style_features(df: pd.DataFrame, target_year: int) -> pd.DataFrame:
    window = df[(df["year"] >= target_year - 7) & (df["year"] <= target_year)]
    if window.empty:
        window = df.copy()

    feats = []
    for style, grp in window.groupby("style"):
        y = grp.sort_values("year")["score"].to_numpy()
        slope = float(np.polyfit(np.arange(len(y)), y, 1)[0]) if len(y) >= 2 else 0.0
        recency = float(np.mean(y[-3:])) if len(y) else 0.0
        vol = float(np.std(y)) if len(y) else 0.0
        img = float(grp["image_signal"].mean())
        txt = float(grp["text_signal"].mean())
        cyc = float(np.sin(2 * np.pi * target_year / 20.0) * np.mean(y) if len(y) else 0.0)
        feats.append(
            {
                "style": style,
                "recency": recency,
                "slope": slope,
                "volatility": vol,
                "image_emb": img,
                "text_emb": txt,
                "cyclic_signal": cyc,
            }
        )
    return pd.DataFrame(feats)


def compute_trend_snapshot(feature_df: pd.DataFrame) -> TrendSnapshot:
    X = feature_df[["recency", "slope", "volatility", "image_emb", "text_emb", "cyclic_signal"]].to_numpy()

    n_clusters = min(3, len(feature_df)) if len(feature_df) else 1
    km = KMeans(n_clusters=n_clusters, n_init=10, random_state=7)
    labels = km.fit_predict(X) if len(feature_df) else np.array([])

    pca = PCA(n_components=2)
    pcs = pca.fit_transform(X) if len(feature_df) >= 2 else np.zeros((len(feature_df), 2))

    score = (
        0.42 * feature_df["recency"]
        + 0.28 * feature_df["slope"]
        + 0.18 * feature_df["cyclic_signal"]
        - 0.12 * feature_df["volatility"]
    )

    return TrendSnapshot(
        style_scores={k: float(v) for k, v in zip(feature_df["style"], score)},
        cluster_labels={k: int(v) for k, v in zip(feature_df["style"], labels)},
        pca_components={k: [float(x), float(y)] for k, (x, y) in zip(feature_df["style"], pcs)},
    )
