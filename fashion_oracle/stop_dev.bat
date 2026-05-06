@echo off
cd /d "%~dp0"

REM Libere les ports dev les plus utilises (8010 defaut, 8011 souvent si 8010 occupe).
echo [Fashion Oracle] Arret des processus en LISTENING sur les ports 8010, 8011, 8012...
for %%P in (8010 8011 8012) do (
  for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%%P " ^| findstr LISTENING') do (
    echo Arret PID %%a sur le port %%P
    taskkill /PID %%a /F 1>nul 2>nul
  )
)
echo [Fashion Oracle] stop_dev termine. Relancez avec:  .\run_dev.bat
