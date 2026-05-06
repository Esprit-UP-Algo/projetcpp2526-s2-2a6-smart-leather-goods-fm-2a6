# Script de préparation pour Qt Creator
# Ce script nettoie et prépare le projet pour une configuration propre

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "PRÉPARATION POUR QT CREATOR" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Vérifier que Qt Creator est fermé
Write-Host "[1/6] Vérification de Qt Creator..." -ForegroundColor Yellow
$qtcreator = Get-Process -Name "qtcreator" -ErrorAction SilentlyContinue
if ($qtcreator) {
    Write-Host "  ⚠️  Qt Creator est en cours d'exécution!" -ForegroundColor Red
    Write-Host "  Veuillez fermer Qt Creator et relancer ce script." -ForegroundColor Red
    Write-Host ""
    Read-Host "Appuyez sur Entrée pour quitter"
    exit 1
} else {
    Write-Host "  ✓ Qt Creator n'est pas en cours d'exécution" -ForegroundColor Green
}

# 2. Nettoyer le dossier build
Write-Host ""
Write-Host "[2/6] Nettoyage du dossier build..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
    Write-Host "  ✓ Dossier build supprimé" -ForegroundColor Green
} else {
    Write-Host "  ℹ️  Pas de dossier build à supprimer" -ForegroundColor Gray
}

# 3. Nettoyer les fichiers Qt Creator
Write-Host ""
Write-Host "[3/6] Nettoyage des fichiers Qt Creator..." -ForegroundColor Yellow
if (Test-Path ".qtcreator") {
    Remove-Item -Recurse -Force .qtcreator -ErrorAction SilentlyContinue
    Write-Host "  ✓ Dossier .qtcreator supprimé" -ForegroundColor Green
} else {
    Write-Host "  ℹ️  Pas de dossier .qtcreator à supprimer" -ForegroundColor Gray
}

$userFiles = Get-ChildItem -Filter "CMakeLists.txt.user*" -ErrorAction SilentlyContinue
if ($userFiles) {
    $userFiles | Remove-Item -Force
    Write-Host "  ✓ Fichiers CMakeLists.txt.user supprimés" -ForegroundColor Green
} else {
    Write-Host "  ℹ️  Pas de fichiers .user à supprimer" -ForegroundColor Gray
}

# 4. Vérifier les fichiers sources
Write-Host ""
Write-Host "[4/6] Vérification des fichiers sources..." -ForegroundColor Yellow

$fichiers_requis = @(
    "matierepremiere.h",
    "matierepremiere.cpp",
    "connexion.h",
    "connexion.cpp",
    "mainwindow.h",
    "mainwindow.cpp",
    "CMakeLists.txt"
)

$fichiers_manquants = @()
foreach ($fichier in $fichiers_requis) {
    if (Test-Path $fichier) {
        Write-Host "  ✓ $fichier" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $fichier MANQUANT!" -ForegroundColor Red
        $fichiers_manquants += $fichier
    }
}

# 5. Vérifier CMakeLists.txt
Write-Host ""
Write-Host "[5/6] Vérification de CMakeLists.txt..." -ForegroundColor Yellow

if (Test-Path "CMakeLists.txt") {
    $contenu = Get-Content "CMakeLists.txt" -Raw
    
    if ($contenu -match "matierepremiere\.h matierepremiere\.cpp") {
        Write-Host "  ✓ matierepremiere inclus dans CMakeLists.txt" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  matierepremiere NON inclus dans CMakeLists.txt!" -ForegroundColor Yellow
        Write-Host "     Vérifiez la ligne 34 de CMakeLists.txt" -ForegroundColor Yellow
    }
}

# 6. Créer un nouveau dossier build vide
Write-Host ""
Write-Host "[6/6] Création d'un nouveau dossier build..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path build -Force | Out-Null
Write-Host "  ✓ Nouveau dossier build créé" -ForegroundColor Green

# Résumé
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "RÉSUMÉ" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($fichiers_manquants.Count -eq 0) {
    Write-Host "✅ PROJET PRÊT POUR QT CREATOR!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Prochaines étapes:" -ForegroundColor Yellow
    Write-Host "1. Lancer Qt Creator" -ForegroundColor White
    Write-Host "2. Fichier → Ouvrir un fichier ou projet" -ForegroundColor White
    Write-Host "3. Sélectionner: CMakeLists.txt" -ForegroundColor White
    Write-Host "4. Cocher: Desktop Qt 6.7.3 MinGW 64-bit" -ForegroundColor White
    Write-Host "5. Cliquer: Configure Project" -ForegroundColor White
    Write-Host "6. Build → Rebuild All (Ctrl+Shift+B)" -ForegroundColor White
    Write-Host ""
    Write-Host "📖 Consultez: ETAPES_VISUELLES_QT_CREATOR.md" -ForegroundColor Cyan
} else {
    Write-Host "⚠️  FICHIERS MANQUANTS DÉTECTÉS!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Fichiers manquants:" -ForegroundColor Yellow
    foreach ($fichier in $fichiers_manquants) {
        Write-Host "  - $fichier" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "Solution:" -ForegroundColor Yellow
    Write-Host "Copiez les fichiers depuis le dossier d'intégration:" -ForegroundColor White
    Write-Host '  Copy-Item "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.*" .' -ForegroundColor Gray
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Pause
Read-Host "Appuyez sur Entrée pour continuer"
