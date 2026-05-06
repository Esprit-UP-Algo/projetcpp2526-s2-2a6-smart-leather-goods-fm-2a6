# Fashion Oracle - Metier avance Historique de Mode

Fashion Oracle est un service IA avance pour le module Produit. Il reconstruit les tendances historiques et predit les styles futurs.

## Fonctionnalites
- Prediction par annee: styles, palettes, matieres, silhouettes
- Score de confiance et similarite avec decades passees
- Graphiques d'evolution temporelle et cycles mode
- Recommandations produit exploitables par le module Produit
- Endpoint REST FastAPI (`/predict?year=XXXX`)
- Mode generatif image (si API key disponible)

## Lancement rapide
```bash
cd fashion_oracle
python -m venv .venv
. .venv/Scripts/activate
pip install -r requirements.txt
copy .env.example .env
uvicorn app.main:app --reload --port 8010
```

## Activer la vraie IA (obligatoire pour LLM)
Dans `.env`, renseigner une cle valide:
```env
FASHION_ORACLE_API_KEY=YOUR_REAL_KEY
FASHION_ORACLE_MODEL=openai/gpt-4o-mini
FASHION_ORACLE_BASE_URL=https://openrouter.ai/api/v1
```

Si la cle est absente/invalide, le service fonctionne en mode local (forecast statistique) et l'UI Qt affichera un badge de fallback.

## Endpoints
- `GET /health`
- `GET /predict?year=2031`
- `GET /similarity?year_a=2031&year_b=1998`
- `GET /reverse?style=minimal-tailoring`

## Integration Qt (module Produit)
1. Depuis C++ Qt, appeler `http://127.0.0.1:8010/predict?year=YYYY` via `QNetworkAccessManager`.
2. Parser JSON (`QJsonDocument`).
3. Afficher les champs `summary`, `top_styles`, `recommended_product_attributes` dans l'onglet Historique du mode.

## Securite
- Ne jamais ecrire une cle API dans le code source.
- Utiliser uniquement `.env` local ou variables d'environnement.
