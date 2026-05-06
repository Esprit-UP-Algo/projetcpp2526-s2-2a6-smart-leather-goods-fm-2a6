# Script de correction de l'erreur NMake
# Force l'utilisation de MinGW au lieu de NMake/MSVC

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "CORRECTION ERREUR NMAKE" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Vérifier que Qt Creator est fermé
Write-Host "[1/7] Vérification de Qt Creator..." -ForegroundColor Yellow
$qtcreator = Get-Process -Name "qtcreator" -ErrorAction SilentlyContinue
if ($qtcreator) {
    Write-Host "  ⚠️  Qt Creator est en cours d'exécution!" -ForegroundColor Red
    Write-Host "  Veuillez fermer Qt Creator et relancer ce script." -ForegroundColor Red
    Write-Host ""
    Read-Host "Appuyez sur Entrée pour quitter"
    exit 1
} else {
    Write-Host "  ✓ Qt Creator fermé" -ForegroundColor Green
}

# 2. Nettoyer complètement
Write-Host ""
Write-Host "[2/7] Nettoyage complet..." -ForegroundColor Yellow
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force .qtcreator -ErrorAction SilentlyContinue
Remove-Item CMakeLists.txt.user* -Force -ErrorAction SilentlyContinue
Write-Host "  ✓ Projet nettoyé" -ForegroundColor Green

# 3. Vérifier MinGW
Write-Host ""
Write-Host "[3/7] Vérification de MinGW..." -ForegroundColor Yellow

$mingw_paths = @(
    "C:\Qt\Tools\mingw1310_64\bin\g++.exe",
    "C:\Qt\Tools\mingw1120_64\bin\g++.exe",
    "C:\Qt\Tools\mingw900_64\bin\g++.exe"
)

$mingw_found = $false
$mingw_path = ""
foreach ($path in $mingw_paths) {
    if (Test-Path $path) {
        $mingw_found = $true
        $mingw_path = Split-Path (Split-Path $path)
        Write-Host "  ✓ MinGW trouvé: $mingw_path" -ForegroundColor Green
        break
    }
}

if (-not $mingw_found) {
    Write-Host "  ✗ MinGW NON trouvé!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Solution:" -ForegroundColor Yellow
    Write-Host "1. Lancer C:\Qt\MaintenanceTool.exe" -ForegroundColor White
    Write-Host "2. Ajouter/Supprimer des composants" -ForegroundColor White
    Write-Host "3. Cocher: Qt 6.7.3 → MinGW 13.1.0 64-bit" -ForegroundColor White
    Write-Host "4. Installer" -ForegroundColor White
    Write-Host ""
    Read-Host "Appuyez sur Entrée pour quitter"
    exit 1
}

# 4. Vérifier Qt
Write-Host ""
Write-Host "[4/7] Vérification de Qt..." -ForegroundColor Yellow

$qt_paths = @(
    "C:\Qt\6.7.3\mingw_64",
    "C:\Qt\6.7.2\mingw_64",
    "C:\Qt\6.7.1\mingw_64"
)

$qt_found = $false
$qt_path = ""
foreach ($path in $qt_paths) {
    if (Test-Path $path) {
        $qt_found = $true
        $qt_path = $path
        Write-Host "  ✓ Qt trouvé: $qt_path" -ForegroundColor Green
        break
    }
}

if (-not $qt_found) {
    Write-Host "  ✗ Qt NON trouvé!" -ForegroundColor Red
    exit 1
}

# 5. Créer CMakePresets avec MinGW
Write-Host ""
Write-Host "[5/7] Configuration de CMakePresets..." -ForegroundColor Yellow

$gcc_path = Join-Path $mingw_path "bin\gcc.exe"
$gxx_path = Join-Path $mingw_path "bin\g++.exe"
$make_path = Join-Path $mingw_path "bin\mingw32-make.exe"

$presets = @"
{
  "version": 3,
  "configurePresets": [
    {
      "name": "mingw-debug",
      "displayName": "MinGW Debug",
      "description": "Configuration Debug avec MinGW",
      "generator": "MinGW Makefiles",
      "binaryDir": "`${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_PREFIX_PATH": "$($qt_path -replace '\\', '/')",
        "CMAKE_C_COMPILER": "$($gcc_path -replace '\\', '/')",
        "CMAKE_CXX_COMPILER": "$($gxx_path -replace '\\', '/')",
        "CMAKE_MAKE_PROGRAM": "$($make_path -replace '\\', '/')"
      }
    }
  ]
}
"@

# Sauvegarder l'ancien CMakePresets s'il existe
if (Test-Path "CMakePresets.json") {
    Copy-Item "CMakePresets.json" "CMakePresets.json.backup" -Force
    Write-Host "  ℹ️  Ancien CMakePresets.json sauvegardé" -ForegroundColor Gray
}

$presets | Out-File -FilePath "CMakePresets.json" -Encoding UTF8
Write-Host "  ✓ CMakePresets.json créé avec MinGW" -ForegroundColor Green

# 6. Créer un dossier build vide
Write-Host ""
Write-Host "[6/7] Création du dossier build..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path build -Force | Out-Null
Write-Host "  ✓ Dossier build créé" -ForegroundColor Green

# 7. Instructions finales
Write-Host ""
Write-Host "[7/7] Configuration terminée!" -ForegroundColor Yellow
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✅ CORRECTION APPLIQUÉE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Prochaines étapes:" -ForegroundColor Yellow
Write-Host ""
Write-Host "1. Lancer Qt Creator" -ForegroundColor White
Write-Host ""
Write-Host "2. Fichier → Ouvrir un fichier ou projet" -ForegroundColor White
Write-Host "   Sélectionner: CMakeLists.txt" -ForegroundColor Gray
Write-Host ""
Write-Host "3. IMPORTANT: Sélectionner le kit MinGW" -ForegroundColor White
Write-Host "   ☑ Desktop Qt 6.7.3 MinGW 64-bit  ← COCHER" -ForegroundColor Green
Write-Host "   ☐ Desktop Qt 6.7.3 MSVC 64-bit   ← NE PAS COCHER" -ForegroundColor Red
Write-Host ""
Write-Host "4. Cliquer: Configure Project" -ForegroundColor White
Write-Host ""
Write-Host "5. Attendre la configuration (1-3 min)" -ForegroundColor White
Write-Host ""
Write-Host "6. Build → Rebuild All (Ctrl+Shift+B)" -ForegroundColor White
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "📖 Pour plus d'aide: CORRIGER_ERREUR_NMAKE.md" -ForegroundColor Cyan
Write-Host ""

# Pause
Read-Host "Appuyez sur Entrée pour continuer"
