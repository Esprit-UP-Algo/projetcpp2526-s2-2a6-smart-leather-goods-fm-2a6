#!/usr/bin/env bash
# Git Bash : ne pas utiliser `taskkill /PID ...` directement (les / sont pris pour des chemins).
set -e
cd "$(dirname "$0")"
PORT="${FASHION_ORACLE_PORT:-8010}"
LINE=$(netstat -ano 2>/dev/null | grep -E ":${PORT}[[:space:]].*LISTENING" | head -1) || true
if [ -z "$LINE" ]; then
  echo "Aucun processus en LISTENING sur le port ${PORT}."
  exit 0
fi
# Dernier champ de la ligne netstat = PID (Windows)
PID=$(echo "$LINE" | awk '{print $NF}')
if [ -z "$PID" ] || ! [[ "$PID" =~ ^[0-9]+$ ]]; then
  echo "Impossible de determiner le PID (ligne: $LINE)"
  exit 1
fi
echo "Arret du processus PID=${PID} (port ${PORT})..."
cmd.exe //c "taskkill /PID ${PID} /F"
