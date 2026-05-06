#!/usr/bin/env bash
# Git Bash / MSYS : ne pas utiliser les backslashes Windows (.\.venv\...).
# Si 8010 refuse de demarrer (WinError 10013), exportez FASHION_ORACLE_PORT=8011 (meme valeur que le client Qt).
set -e
cd "$(dirname "$0")"
: "${FASHION_ORACLE_PORT:=8010}"
exec ./.venv/Scripts/python.exe -m uvicorn app.main:app --host 127.0.0.1 --port "$FASHION_ORACLE_PORT" --reload
