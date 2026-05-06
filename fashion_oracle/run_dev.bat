@echo off
cd /d "%~dp0"
if not defined FASHION_ORACLE_PORT set FASHION_ORACLE_PORT=8010

REM Python : .venv par defaut, sinon .venv-new (si .venv absent)
set "PYEXE=%~dp0.venv\Scripts\python.exe"
if not exist "%PYEXE%" set "PYEXE=%~dp0.venv-new\Scripts\python.exe"
if not exist "%PYEXE%" (
  echo [ERREUR] Aucun interpreteur trouve :
  echo   %~dp0.venv\Scripts\python.exe
  echo   %~dp0.venv-new\Scripts\python.exe
  echo Creez-en un : python -m venv .venv
  echo   ou si .venv est bloque : python -m venv .venv-new
  echo Puis : .venv\Scripts\pip install -r requirements.txt  ^(ou .venv-new\Scripts\pip ...^)
  pause
  exit /b 1
)

REM Si .venv existe mais sans uvicorn ^(paquets installes dans .venv-new seulement^)
if exist "%~dp0.venv\Scripts\python.exe" (
  if not exist "%~dp0.venv\Scripts\uvicorn.exe" (
    if exist "%~dp0.venv-new\Scripts\python.exe" (
      set "PYEXE=%~dp0.venv-new\Scripts\python.exe"
      echo [Fashion Oracle] .venv sans uvicorn — utilisation de .venv-new
    )
  )
)

echo [Fashion Oracle] Python utilise : %PYEXE%

echo [Fashion Oracle] Verification du port %FASHION_ORACLE_PORT%...
"%PYEXE%" dev_check_port.py
if errorlevel 1 (
  echo.
  pause
  exit /b 1
)

echo [Fashion Oracle] Demarrage Uvicorn - http://127.0.0.1:%FASHION_ORACLE_PORT%/docs  ^(Ctrl+C pour arreter^)
echo.
"%PYEXE%" -m uvicorn app.main:app --host 127.0.0.1 --port %FASHION_ORACLE_PORT% --reload --reload-exclude dev_check_port.py
