@echo off
chcp 65001 >nul
echo ========================================
echo   COPIE AUTOMATIQUE DES IMAGES
echo ========================================
echo.

REM Créer le dossier images s'il n'existe pas
if not exist "images" (
    echo [1/5] Création du dossier images...
    mkdir images
    echo OK
) else (
    echo [1/5] Dossier images existe déjà
    echo OK
)

echo.
echo [2/5] Copie des images depuis assets/historique...

REM Copier les images de sacs
if exist "assets\historique\sac.jpg" (
    copy /Y "assets\historique\sac.jpg" "images\sac main.jpg" >nul
    echo ✅ sac main.jpg copié
)

if exist "assets\historique\sac4.jpg" (
    copy /Y "assets\historique\sac4.jpg" "images\sac voyage.jpg" >nul
    echo ✅ sac voyage.jpg copié
)

REM Copier l'image de ceinture
if exist "ceinture.webp" (
    copy /Y "ceinture.webp" "images\ceinture.webp" >nul
    echo ✅ ceinture.webp copié
) else if exist "assets\historique\belt.jpg" (
    copy /Y "assets\historique\belt.jpg" "images\ceinture.jpg" >nul
    echo ✅ ceinture.jpg copié
)

REM Chercher une image pour portefeuille
if exist "assets\historique\leather.jpg" (
    copy /Y "assets\historique\leather.jpg" "images\portefeuille.jpg" >nul
    echo ✅ portefeuille.jpg copié
)

echo OK

echo.
echo [3/5] Vérification des images copiées...
dir /B images\*.jpg images\*.webp 2>nul
echo OK

echo.
echo [4/5] Résumé:
echo.
if exist "images\sac main.jpg" (echo ✅ Sac Main: OK) else (echo ❌ Sac Main: MANQUANT)
if exist "images\sac voyage.jpg" (echo ✅ Sac Voyage: OK) else (echo ❌ Sac Voyage: MANQUANT)
if exist "images\ceinture.webp" (echo ✅ Ceinture: OK) else if exist "images\ceinture.jpg" (echo ✅ Ceinture: OK) else (echo ❌ Ceinture: MANQUANT)
if exist "images\portefeuille.jpg" (echo ✅ Portefeuille: OK) else (echo ❌ Portefeuille: MANQUANT)

echo.
echo [5/5] Terminé!
echo.
echo ========================================
echo   IMAGES PRÊTES POUR LE CALCULATEUR
echo ========================================
echo.
echo Pour tester:
echo 1. Lancer: build\ProjetCpp.exe
echo 2. Aller dans: Stock ^> Calculateur
echo 3. Sélectionner un produit
echo.
pause
