# Mock API IA Decoupe (FastAPI)

Serveur local de test pour le module `IA Découpe` (Qt/C++).

## 1) Installation

Dans PowerShell, depuis le dossier `cut_api_mock`:

```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

## 2) Lancement

```powershell
uvicorn server:app --host 127.0.0.1 --port 8000
```

## 3) Config côté application Qt

Dans le terminal où tu lances l'app Qt:

```powershell
$env:FIL_DOR_CUT_API_URL="http://127.0.0.1:8000/api/v1/generate-cut-plan"
```

Puis lance ton application.

## 4) Vérification rapide

- Health:
  - `http://127.0.0.1:8000/health`
- Swagger:
  - `http://127.0.0.1:8000/docs`

Le format de réponse est compatible avec ton parsing actuel (`placements`, `sheets_used`, `waste_pct`, etc. + clé `plan` miroir).
