"""Verifie que le port HTTP est libre avant de lancer Uvicorn (evite WinError 10013)."""
from __future__ import annotations

import os
import socket
import subprocess
import sys


def _decode_netstat_output(stdout: bytes | None, stderr: bytes | None) -> str:
    """Decode la sortie console Windows sans lever UnicodeDecodeError."""
    raw = (stdout or b"") + (stderr or b"")
    if not raw:
        return ""
    for enc in ("utf-8", "cp850", "cp437", "latin-1"):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            continue
    return raw.decode("utf-8", errors="replace")


def _windows_netstat_hint(port: int) -> None:
    """Affiche les lignes netstat pour ce port (ecoute, TIME_WAIT, plage reservee, etc.)."""
    if sys.platform != "win32":
        return
    try:
        r = subprocess.run(
            ["netstat", "-ano"],
            capture_output=True,
            timeout=8,
            check=False,
        )
        txt = _decode_netstat_output(r.stdout, r.stderr)
        needle = f":{port}"
        all_hits = [ln.strip() for ln in txt.splitlines() if needle in ln and ln.strip().upper().startswith("TCP")]
        listenish = [
            ln
            for ln in all_hits
            if "LISTENING" in ln.upper()
            or "ECOUTE" in ln.upper().replace("É", "E")
            or "TIME_WAIT" in ln.upper()
        ]
        if listenish:
            print("Lignes netstat pertinentes (ecoute / TIME_WAIT) :")
            for ln in listenish[:12]:
                print(" ", ln)
        elif all_hits:
            print("Lignes netstat contenant ce port (toutes etats TCP) :")
            for ln in all_hits[:15]:
                print(" ", ln)
        else:
            print("(Aucune ligne TCP netstat pour ce port.)")
            print("  Si le bind echoue quand meme : plage Hyper-V / reservee ?")
            print('  CMD admin : netsh interface ipv4 show excludedportrange protocol=tcp')
    except OSError as ex:
        print(f"(netstat indisponible : {ex})")


def main() -> int:
    raw = os.environ.get("FASHION_ORACLE_PORT", "8010").strip()
    try:
        port = int(raw)
    except ValueError:
        print(f"[ERREUR] FASHION_ORACLE_PORT invalide : {raw!r}")
        return 1
    if not (1 <= port < 65536):
        print(f"[ERREUR] Port hors plage : {port}")
        return 1

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Aligne le test sur Uvicorn : evite les faux positifs TIME_WAIT sur Windows.
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        except OSError:
            pass
        s.bind(("127.0.0.1", port))
    except OSError as e:
        print(f"[ERREUR] Impossible d'utiliser le port {port} : {e}")
        print()
        _windows_netstat_hint(port)
        print()
        print("Causes frequentes :")
        print("  - Une instance Fashion Oracle / Uvicorn tourne deja sur ce port.")
        print("  - Dans ce cas : ouvrez http://127.0.0.1:%d/docs (pas besoin de relancer)." % port)
        print("  - Sinon : dans ce dossier, lancez stop_dev.bat puis run_dev.bat.")
        print("  - Ou utilisez un autre port :")
        print("      set FASHION_ORACLE_PORT=8011")
        print("      run_dev.bat")
        print("    (definissez la meme variable FASHION_ORACLE_PORT pour l'application Qt.)")
        return 1
    finally:
        s.close()

    print(f"[OK] Port {port} disponible sur 127.0.0.1.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
