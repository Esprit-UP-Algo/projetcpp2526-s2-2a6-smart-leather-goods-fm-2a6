import streamlit as st


st.set_page_config(page_title="Historique de Mode — Luxe", layout="wide")


def get_fashion_image(query: str) -> str:
    safe_query = query.replace(" ", ",")
    return f"https://source.unsplash.com/1200x1600/?{safe_query},fashion,editorial"


TRENDS = [
    {
        "name": "conceptual",
        "material": "bio-fiber satin · ink blue",
        "badge": "PEAK",
        "badge_class": "badge-peak",
        "palette": ["#1C1C2E", "#4A5568", "#C4A882", "#F0E6D3"],
        "query": "conceptual fashion jacket editorial",
    },
    {
        "name": "athlux",
        "material": "tech mesh · graphite",
        "badge": "RISING",
        "badge_class": "badge-rising",
        "palette": ["#0F172A", "#334155", "#8B7355", "#E2D8C8"],
        "query": "athlux sport fashion editorial",
    },
    {
        "name": "artisan-leather",
        "material": "velvet leather · cognac",
        "badge": "FADING",
        "badge_class": "badge-fading",
        "palette": ["#3B2416", "#8B5E3C", "#C4A882", "#F5E6D3"],
        "query": "artisan coat fashion editorial",
    },
]

SEASON_COLORS = [
    ("Midnight Slate", "#1C1C2E"),
    ("Ink Blue", "#4A5568"),
    ("Cognac Sand", "#8B7355"),
    ("Gold Beige", "#C4A882"),
    ("Silk Ivory", "#F0E6D3"),
    ("Runway Black", "#0A0A0A"),
]

MOODBOARD_QUERIES = [
    "fashion editorial coat",
    "luxury leather belt",
    "minimal tailored jacket",
    "runway handbag closeup",
    "silk dress editorial",
    "street luxury fashion",
    "couture fabric texture",
    "fashion portrait monochrome",
]

TEXTURE_SWATCHES = [
    ("organza", "Light that floats between structure and breath."),
    ("tech mesh", "Precision woven for movement and modern rhythm."),
    ("bio-fiber satin", "Sustainable glow with liquid sophistication."),
    ("velvet", "A deep tactile shadow, rich and cinematic."),
]

if "selected_color" not in st.session_state:
    st.session_state.selected_color = None

st.markdown(
    """
<style>
@import url('https://fonts.googleapis.com/css2?family=Cormorant+Garamond:ital,wght@0,300;0,400;1,300&family=Inter:wght@300;400&display=swap');

.stApp { background: #0A0A0A; color: #F5F0E8; }
.block-container { padding-top: 0.5rem; max-width: 1400px; }
h1, h2, h3 { font-family: 'Cormorant Garamond', serif; font-weight: 300; letter-spacing: 1px; }
p, span, div, button { font-family: 'Inter', sans-serif; }

.hero {
  min-height: 75vh;
  display: flex;
  flex-direction: column;
  justify-content: center;
  border-bottom: 1px solid rgba(200,151,42,0.45);
  background:
    radial-gradient(circle at 20% 30%, rgba(255,255,255,0.04), transparent 40%),
    radial-gradient(circle at 80% 70%, rgba(200,151,42,0.12), transparent 32%),
    #0A0A0A;
}
.hero-title {
  font-size: clamp(38px, 8vw, 92px);
  line-height: 0.95;
  color: #F5F0E8;
  margin-bottom: 18px;
}
.hero-sub { color: #C4A882; letter-spacing: 2px; text-transform: uppercase; font-size: 12px; }

.ticker-wrap {
  overflow: hidden;
  white-space: nowrap;
  border-top: 1px solid rgba(200,151,42,0.6);
  border-bottom: 1px solid rgba(200,151,42,0.6);
  margin-top: 28px;
  padding: 12px 0;
}
.ticker {
  display: inline-block;
  min-width: 200%;
  animation: ticker 26s linear infinite;
  font-size: 11px;
  letter-spacing: 4px;
  color: #F5F0E8;
  text-transform: uppercase;
}
@keyframes ticker { from { transform: translateX(0); } to { transform: translateX(-50%); } }

.section-light {
  margin-top: 42px;
  padding: 28px 26px 36px 26px;
  background: #F5F0E8;
  color: #16120f;
  border-top: 1px solid #C8972A;
  border-bottom: 1px solid #C8972A;
}

.fashion-card {
  position: relative;
  overflow: hidden;
  border-radius: 2px;
  cursor: pointer;
  transition: transform 0.6s cubic-bezier(0.25, 0.46, 0.45, 0.94);
}
.fashion-card:hover { transform: scale(1.03); }
.fashion-card img { width: 100%; height: 540px; object-fit: cover; display: block; }
.card-overlay {
  position: absolute; bottom: 0; left: 0; right: 0;
  background: linear-gradient(transparent, rgba(0,0,0,0.90));
  padding: 40px 24px 22px;
}
.trend-name {
  font-family: 'Cormorant Garamond', serif;
  font-size: 34px; font-weight: 300; color: #F5F0E8; letter-spacing: 2px;
}
.matter { color: #D8CCBA; font-style: italic; font-size: 12px; margin: 6px 0 10px 0; }
.badge { display: inline-block; padding: 3px 10px; font-size: 9px; letter-spacing: 2px; text-transform: uppercase; }
.badge-peak { background: #C8972A; color: #0A0A0A; }
.badge-rising { background: #2D6A4F; color: #F5F0E8; }
.badge-fading { background: #6B2D2D; color: #F5F0E8; }

.palette-row { display: flex; gap: 0; margin-top: 18px; }
.swatch-col { flex: 1; text-align: center; }
.swatch {
  width: 100%; height: 112px; border: 1px solid rgba(0,0,0,0.08);
  transition: transform .35s ease;
}
.swatch:hover { transform: scaleY(1.05); }
.swatch-name { margin-top: 8px; font-size: 11px; letter-spacing: 1.3px; text-transform: uppercase; color: #3B2F24; }

.masonry {
  column-count: 4;
  column-gap: 16px;
}
.masonry-item {
  break-inside: avoid;
  margin-bottom: 16px;
  position: relative;
  overflow: hidden;
}
.masonry-item img { width: 100%; border-radius: 2px; display: block; }
.masonry-caption {
  position: absolute; left: 10px; bottom: 10px;
  background: rgba(10,10,10,0.65); color: #F5F0E8;
  padding: 5px 10px; font-size: 10px; letter-spacing: 1px; opacity: 0; transition: opacity .25s ease;
}
.masonry-item:hover .masonry-caption { opacity: 1; }

.sil-grid { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 22px; }
.sil-card { text-align: center; padding: 24px 12px; border: 1px solid rgba(200,151,42,0.3); }
.silhouette {
  width: 100px; height: 200px; margin: 0 auto 14px auto; position: relative; opacity: 0;
  animation: fadeInUp 1.1s ease forwards;
  background: linear-gradient(180deg, rgba(245,240,232,.95), rgba(196,168,130,.65));
  clip-path: polygon(50% 0%, 67% 8%, 62% 24%, 72% 46%, 66% 100%, 34% 100%, 28% 46%, 38% 24%, 33% 8%);
}
.sil-1 { animation-delay: .1s; } .sil-2 { animation-delay: .4s; } .sil-3 { animation-delay: .7s; }

.texture-grid { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 16px; }
.texture-item { position: relative; overflow: hidden; }
.texture-item img { width: 100%; height: 200px; object-fit: cover; display: block; }
.texture-name { margin-top: 8px; font-family: 'Cormorant Garamond', serif; font-size: 22px; }
.texture-poem {
  position: absolute; left: 0; right: 0; bottom: 0;
  background: linear-gradient(transparent, rgba(10,10,10,0.88));
  color: #F5F0E8; font-size: 11px; padding: 36px 12px 10px 12px;
  opacity: 0; transition: opacity .3s ease;
}
.texture-item:hover .texture-poem { opacity: 1; }

.reveal { animation: fadeInUp 1.0s ease both; }
@keyframes fadeInUp { from { opacity: 0; transform: translateY(28px);} to {opacity:1; transform: translateY(0);} }
</style>
""",
    unsafe_allow_html=True,
)

ticker_items = "  •  ".join(
    [
        "conceptual",
        "athlux",
        "artisan-leather",
        "minimal-tailoring",
        "romantic-fluid",
        "annual",
    ]
)

st.markdown(
    f"""
<section class="hero reveal">
  <div class="hero-sub">Historique de mode — Maison Vision</div>
  <div class="hero-title">Runway Narrative<br/>Seasonal Intelligence</div>
  <div class="ticker-wrap"><div class="ticker">{ticker_items}  •  {ticker_items}</div></div>
</section>
""",
    unsafe_allow_html=True,
)

st.markdown('<div class="section-light reveal">', unsafe_allow_html=True)
st.markdown("## Éditorial des tendances")

color_cols = st.columns(len(SEASON_COLORS) + 1)
for i, (name, color) in enumerate(SEASON_COLORS):
    if color_cols[i].button(name, key=f"swatch_{name}"):
        st.session_state.selected_color = color
if color_cols[-1].button("Tout afficher", key="swatch_all"):
    st.session_state.selected_color = None

st.markdown('<div class="palette-row">', unsafe_allow_html=True)
for name, color in SEASON_COLORS:
    st.markdown(
        f"""
        <div class="swatch-col">
          <div class="swatch" style="background:{color};"></div>
          <div class="swatch-name">{name}</div>
        </div>
        """,
        unsafe_allow_html=True,
    )
st.markdown("</div>", unsafe_allow_html=True)

selected = st.session_state.selected_color
if selected:
    filtered = [t for t in TRENDS if selected in t["palette"]]
    if not filtered:
        filtered = TRENDS
else:
    filtered = TRENDS

cards = st.columns(3)
for i, trend in enumerate(filtered[:3]):
    with cards[i]:
        dots = "".join(
            [f'<span style="display:inline-block;width:16px;height:16px;border-radius:50%;background:{c};margin-right:-4px;border:1px solid rgba(255,255,255,.25)"></span>' for c in trend["palette"][:4]]
        )
        st.markdown(
            f"""
            <div class="fashion-card reveal">
              <img src="{get_fashion_image(trend['query'])}" alt="{trend['name']}">
              <div class="card-overlay">
                <div class="trend-name">{trend['name']}</div>
                <div class="matter">{trend['material']}</div>
                <div style="margin-bottom:10px;">{dots}</div>
                <span class="badge {trend['badge_class']}">{trend['badge']}</span>
                <span style="float:right;color:#F5F0E8;font-size:11px;letter-spacing:1.2px;">Explorer</span>
              </div>
            </div>
            """,
            unsafe_allow_html=True,
        )
st.markdown("</div>", unsafe_allow_html=True)

st.markdown("## Moodboard atelier")
masonry_html = '<div class="masonry reveal">'
for q in MOODBOARD_QUERIES:
    masonry_html += f"""
    <div class="masonry-item">
      <img src="{get_fashion_image(q)}" alt="{q}">
      <div class="masonry-caption">{q.title()}</div>
    </div>
    """
masonry_html += "</div>"
st.markdown(masonry_html, unsafe_allow_html=True)

st.markdown("## Silhouettes iconiques")
sil_html = '<div class="sil-grid reveal">'
for i, t in enumerate(TRENDS[:3], start=1):
    sil_html += f"""
    <div class="sil-card">
      <div class="silhouette sil-{i}"></div>
      <div style="font-family:'Cormorant Garamond',serif;font-size:30px;">{t['name']}</div>
      <div style="font-size:12px;color:#C4A882;">{t['material']}</div>
    </div>
    """
sil_html += "</div>"
st.markdown(sil_html, unsafe_allow_html=True)

st.markdown("## Tissus & matières")
tex_html = '<div class="texture-grid reveal">'
for mat, poetry in TEXTURE_SWATCHES:
    tex_html += f"""
    <div class="texture-item">
      <img src="{get_fashion_image(f'{mat} fabric texture macro')}" alt="{mat}">
      <div class="texture-poem">{poetry}</div>
      <div class="texture-name">{mat}</div>
    </div>
    """
tex_html += "</div>"
st.markdown(tex_html, unsafe_allow_html=True)

st.markdown("</div>", unsafe_allow_html=True)
