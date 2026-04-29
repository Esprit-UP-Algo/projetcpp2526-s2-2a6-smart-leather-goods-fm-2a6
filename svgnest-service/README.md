# svgnest-service

Service Node.js (Express + Playwright) pour generer des plans de decoupage avec un endpoint JSON compatible Qt.

## 1) Installation

```bash
cd svgnest-service
npm install
npm run install:browsers
```

## 2) Lancement

Mode Playwright (par defaut):

```bash
npm start
```

Mode mock pur Node (sans navigateur):

```bash
set NEST_ENGINE=mock
npm start
```

Healthcheck:

- `GET http://127.0.0.1:8010/health`

Endpoint principal:

- `POST http://127.0.0.1:8010/generate-cut-plan`

## 3) Payload attendu (compatible Qt)

```json
{
  "order_id": 101,
  "id_produit": 12,
  "id_matiere": 3,
  "spacing_mm": 4,
  "sheet_width_mm": 1400,
  "sheet_height_mm": 1000,
  "pieces": [
    { "name": "face_avant", "width_mm": 300, "height_mm": 220, "qty": 2, "can_rotate": true },
    { "name": "face_arriere", "width_mm": 300, "height_mm": 220, "qty": 2, "can_rotate": true },
    { "name": "rabat", "width_mm": 250, "height_mm": 150, "qty": 2, "can_rotate": true }
  ]
}
```

## 4) Reponse JSON (structure utile cote Qt)

Le service renvoie:

- `success`
- `algorithm`
- `placements[]` avec `name`, `sheet_index`, `x_mm`, `y_mm`, `w_mm`, `h_mm`, `rotated`, `color`
- `sheets_used`
- `sheet_width_mm`, `sheet_height_mm`
- `used_area_mm2`, `total_area_mm2`, `waste_area_mm2`
- `waste_pct` **et** `waste_percentage`

## 5) Branchement avec ton appli Qt

Dans la session de lancement de ton app Qt:

```bash
set FIL_DOR_CUT_API_URL=http://127.0.0.1:8010/generate-cut-plan
```

Ensuite ton slot Qt `tryRunCutPlanViaApi(...)` peut appeler directement cet endpoint.

## 6) Integration SVGnest (active)

Le service utilise deja le moteur SVGnest dans:

- `src/worker-page.html` -> `window.computeNesting(...)`

Pipeline actuel:

1. generation d'un SVG d'entree (bin + pieces),
2. execution de `SvgNest.start(...)` dans Playwright,
3. extraction des `transform` pour mapper vers `placements[]` Qt.

En cas d'erreur SVGnest, un fallback shelf est applique automatiquement pour garder le service disponible.
