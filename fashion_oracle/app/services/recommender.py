from typing import Dict


def recommend_product_attributes(top_style: str, palette: list[str], fabrics: list[str], silhouettes: list[str]) -> Dict[str, str]:
    return {
        "collection_name": f"Oracle-{top_style.replace('-', ' ').title()}",
        "primary_color": palette[0] if palette else "black",
        "secondary_color": palette[1] if len(palette) > 1 else "white",
        "hero_material": fabrics[0] if fabrics else "leather",
        "signature_silhouette": silhouettes[0] if silhouettes else "structured",
        "positioning": "premium-smart" if "minimal" in top_style or "artisan" in top_style else "urban-trend",
    }
