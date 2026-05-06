# Script de diagnostic des problèmes de compilation
# Projet: FIL D'OR

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DIAGNOSTIC DES PROBLÈMES DE COMPILATION" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$erreurs = 0
$avertissements = 0

# 1. Vérifier les fichiers sources
Write-Host "[1/8] Vérification des fichiers sources..." -ForegroundColor Yellow

$fichiers_requis = @(
    "matierepremiere.h",
    "matierepremiere.cpp",
    "connexion.h",
    "connexion.cpp",
    "mainwindow.h",
    "mainwindow.cpp",
    "CMakeLists.txt"
)

foreach ($fichier in $fichiers_requis) {
    if (Test-Path $fichier) {
        Write-Host "  ✓ $fichier" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $fichier MANQUANT!" -ForegroundColor Red
        $erreurs++
    }
}

# 2. Vérifier CMakeLists.txt
Write-Host ""
Write-Host "[2/8] Vérification de CMakeLists.txt..." -ForegroundColor Yellow

if (Test-Path "CMakeLists.txt") {
    $contenu = Get-Content "CMakeLists.txt" -Raw
    
    if ($contenu -match "matierepremiere\.h matierepremiere\.cpp") {
        Write-Host "  ✓ matierepremiere inclus dans CMakeLists.txt" -ForegroundColor Green
    } else {
        Write-Host "  ✗ matierepremiere NON inclus dans CMakeLists.txt!" -ForegroundColor Red
        $erreurs++
    }
    
    if ($contenu -match "Qt\$\{QT_VERSION_MAJOR\}::Sql") {
        Write-Host "  ✓ Module Qt Sql lié" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Module Qt Sql peut-être manquant" -ForegroundColor Yellow
        $avertissements++
    }
} else {
    Write-Host "  ✗ CMakeLists.txt MANQUANT!" -ForegroundColor Red
    $erreurs++
}

# 3. Vérifier l'installation Qt
Write-Host ""
Write-Host "[3/8] Vérification de l'installation Qt..." -ForegroundColor Yellow

$qt_paths = @(
    "C:\Qt\6.7.3\mingw_64",
    "C:\Qt\6.7.2\mingw_64",
    "C:\Qt\6.7.1\mingw_64",
    "C:\Qt\6.7.0\mingw_64"
)

$qt_trouve = $false
foreach ($path in $qt_paths) {
    if (Test-Path $path) {
        Write-Host "  ✓ Qt trouvé: $path" -ForegroundColor Green
        $qt_trouve = $true
        $env:QT_DIR = $path
        break
    }
}

if (-not $qt_trouve) {
    Write-Host "  ✗ Qt NON trouvé!" -ForegroundColor Red
    Write-Host "    Installez Qt depuis https://www.qt.io/download" -ForegroundColor Red
    $erreurs++
}

# 4. Vérifier MinGW
Write-Host ""
Write-Host "[4/8] Vérification de MinGW..." -ForegroundColor Yellow

$mingw_paths = @(
    "C:\Qt\Tools\mingw1310_64\bin",
    "C:\Qt\Tools\mingw1120_64\bin",
    "C:\Qt\Tools\mingw900_64\bin"
)

$mingw_trouve = $false
foreach ($path in $mingw_paths) {
    if (Test-Path "$path\g++.exe") {
        Write-Host "  ✓ MinGW trouvé: $path" -ForegroundColor Green
        $mingw_trouve = $true
        $env:MINGW_DIR = $path
        break
    }
}

if (-not $mingw_trouve) {
    Write-Host "  ✗ MinGW NON trouvé!" -ForegroundColor Red
    $erreurs++
}

# 5. Vérifier CMake
Write-Host ""
Write-Host "[5/8] Vérification de CMake..." -ForegroundColor Yellow

$cmake_paths = @(
    "C:\Qt\Tools\CMake_64\bin\cmake.exe",
    "C:\Program Files\CMake\bin\cmake.exe",
    "C:\Program Files (x86)\CMake\bin\cmake.exe"
)

$cmake_trouve = $false
foreach ($path in $cmake_paths) {
    if (Test-Path $path) {
        Write-Host "  ✓ CMake trouvé: $path" -ForegroundColor Green
        $cmake_trouve = $true
        $env:CMAKE_DIR = Split-Path $path
        break
    }
}

if (-not $cmake_trouve) {
    Write-Host "  ✗ CMake NON trouvé!" -ForegroundColor Red
    $erreurs++
}

# 6. Vérifier le dossier build
Write-Host ""
Write-Host "[6/8] Vérification du dossier build..." -ForegroundColor Yellow

if (Test-Path "build") {
    Write-Host "  ✓ Dossier build existe" -ForegroundColor Green
    
    if (Test-Path "build\CMakeCache.txt") {
        Write-Host "  ✓ Projet CMake configuré" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Projet CMake non configuré" -ForegroundColor Yellow
        $avertissements++
    }
} else {
    Write-Host "  ⚠ Dossier build n'existe pas" -ForegroundColor Yellow
    $avertissements++
}

# 7. Vérifier les includes dans mainwindow.cpp
Write-Host ""
Write-Host "[7/8] Vérification des includes..." -ForegroundColor Yellow

if (Test-Path "mainwindow.cpp") {
    $contenu = Get-Content "mainwindow.cpp" -Raw
    
    if ($contenu -match '#include\s+"matierepremiere\.h"') {
        Write-Host "  ✓ matierepremiere.h inclus dans mainwindow.cpp" -ForegroundColor Green
    } else {
        Write-Host "  ✗ matierepremiere.h NON inclus dans mainwindow.cpp!" -ForegroundColor Red
        $erreurs++
    }
}

# 8. Vérifier la syntaxe de matierepremiere.h
Write-Host ""
Write-Host "[8/8] Vérification de la syntaxe..." -ForegroundColor Yellow

if (Test-Path "matierepremiere.h") {
    $contenu = Get-Content "matierepremiere.h" -Raw
    
    if ($contenu -match "#ifndef MATIEREPREMIERE_H") {
        Write-Host "  ✓ Header guard correct" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Header guard peut-être incorrect" -ForegroundColor Yellow
        $avertissements++
    }
    
    if ($contenu -match "class MatierePremiere") {
        Write-Host "  ✓ Classe MatierePremiere définie" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Classe MatierePremiere NON définie!" -ForegroundColor Red
        $erreurs++
    }
}

# Résumé
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "RÉSUMÉ DU DIAGNOSTIC" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Erreurs critiques: $erreurs" -ForegroundColor $(if ($erreurs -eq 0) { "Green" } else { "Red" })
Write-Host "Avertissements: $avertissements" -ForegroundColor $(if ($avertissements -eq 0) { "Green" } else { "Yellow" })
Write-Host ""

# Recommandations
if ($erreurs -gt 0) {
    Write-Host "❌ PROBLÈMES DÉTECTÉS!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Actions recommandées:" -ForegroundColor Yellow
    Write-Host "1. Corriger les erreurs ci-dessus" -ForegroundColor White
    Write-Host "2. Exécuter: .\reconfigurer_et_compiler.bat" -ForegroundColor White
    Write-Host "3. Consulter: DEPANNAGE_QT_CREATOR.md" -ForegroundColor White
} elseif ($avertissements -gt 0) {
    Write-Host "⚠️ AVERTISSEMENTS DÉTECTÉS" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Le projet devrait compiler, mais vérifiez les avertissements." -ForegroundColor White
} else {
    Write-Host "✅ AUCUN PROBLÈME DÉTECTÉ!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Le projet devrait compiler correctement." -ForegroundColor White
    Write-Host ""
    Write-Host "Pour compiler:" -ForegroundColor Yellow
    Write-Host "  Option 1: Ouvrir Qt Creator et Build → Rebuild All" -ForegroundColor White
    Write-Host "  Option 2: Exécuter .\reconfigurer_et_compiler.bat" -ForegroundColor White
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Pause pour lire les résultats
Read-Host "Appuyez sur Entrée pour continuer"
