# Script PowerShell pour copier le module Ravitaillement complet
# Date: 05/05/2026

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  COPIE MODULE RAVITAILLEMENT COMPLET  " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$sourceFolder = "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)"
$sourceFile = Join-Path $sourceFolder "mainwindow.cpp"
$destFile = "mainwindow.cpp"

# Vérifier que le fichier source existe
if (-not (Test-Path $sourceFile)) {
    Write-Host "ERREUR: Fichier source introuvable: $sourceFile" -ForegroundColor Red
    Write-Host "Assurez-vous que le dossier Integration (2) est présent." -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ Fichier source trouvé: $sourceFile" -ForegroundColor Green

# Lire le fichier source
Write-Host "Lecture du fichier source..." -ForegroundColor Yellow
$sourceContent = Get-Content $sourceFile -Raw -Encoding UTF8

# Extraire la fonction showStockRavitaillementTab complète
Write-Host "Extraction de la fonction showStockRavitaillementTab()..." -ForegroundColor Yellow

# Trouver le début de la fonction
$startPattern = 'void MainWindow::showStockRavitaillementTab\(\) \{'
$startMatch = [regex]::Match($sourceContent, $startPattern)

if (-not $startMatch.Success) {
    Write-Host "ERREUR: Impossible de trouver la fonction showStockRavitaillementTab() dans le fichier source" -ForegroundColor Red
    exit 1
}

$startIndex = $startMatch.Index

# Trouver la fin de la fonction (accolade fermante correspondante)
$braceCount = 0
$inFunction = $false
$endIndex = $startIndex

for ($i = $startIndex; $i -lt $sourceContent.Length; $i++) {
    $char = $sourceContent[$i]
    
    if ($char -eq '{') {
        $braceCount++
        $inFunction = $true
    }
    elseif ($char -eq '}') {
        $braceCount--
        if ($inFunction -and $braceCount -eq 0) {
            $endIndex = $i + 1
            break
        }
    }
}

if ($endIndex -eq $startIndex) {
    Write-Host "ERREUR: Impossible de trouver la fin de la fonction" -ForegroundColor Red
    exit 1
}

$functionContent = $sourceContent.Substring($startIndex, $endIndex - $startIndex)
$functionLines = ($functionContent -split "`n").Count

Write-Host "✓ Fonction extraite: $functionLines lignes" -ForegroundColor Green

# Lire le fichier destination
Write-Host "Lecture du fichier destination..." -ForegroundColor Yellow
$destContent = Get-Content $destFile -Raw -Encoding UTF8

# Trouver et remplacer la fonction dans le fichier destination
$destStartMatch = [regex]::Match($destContent, $startPattern)

if (-not $destStartMatch.Success) {
    Write-Host "ERREUR: Impossible de trouver la fonction showStockRavitaillementTab() dans le fichier destination" -ForegroundColor Red
    exit 1
}

$destStartIndex = $destStartMatch.Index

# Trouver la fin de la fonction dans le fichier destination
$braceCount = 0
$inFunction = $false
$destEndIndex = $destStartIndex

for ($i = $destStartIndex; $i -lt $destContent.Length; $i++) {
    $char = $destContent[$i]
    
    if ($char -eq '{') {
        $braceCount++
        $inFunction = $true
    }
    elseif ($char -eq '}') {
        $braceCount--
        if ($inFunction -and $braceCount -eq 0) {
            $destEndIndex = $i + 1
            break
        }
    }
}

# Créer une sauvegarde
$backupFile = "mainwindow.cpp.backup_ravitaillement_" + (Get-Date -Format "yyyyMMdd_HHmmss")
Write-Host "Création d'une sauvegarde: $backupFile" -ForegroundColor Yellow
Copy-Item $destFile $backupFile

Write-Host "✓ Sauvegarde créée" -ForegroundColor Green

# Remplacer la fonction
Write-Host "Remplacement de la fonction..." -ForegroundColor Yellow
$newContent = $destContent.Substring(0, $destStartIndex) + $functionContent + $destContent.Substring($destEndIndex)

# Écrire le nouveau contenu
Set-Content -Path $destFile -Value $newContent -Encoding UTF8 -NoNewline

Write-Host "✓ Fonction remplacée avec succès!" -ForegroundColor Green
Write-Host ""

# Statistiques
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  STATISTIQUES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Lignes copiées: $functionLines" -ForegroundColor White
Write-Host "Fichier sauvegardé: $backupFile" -ForegroundColor White
Write-Host ""

Write-Host "========================================" -ForegroundColor Green
Write-Host "  COPIE TERMINÉE AVEC SUCCÈS!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

Write-Host "PROCHAINES ÉTAPES:" -ForegroundColor Yellow
Write-Host "1. Vérifier que les tables Oracle existent (FOURNISSEURS, MATIERES_PREMIERES)" -ForegroundColor White
Write-Host "2. Compiler le projet:" -ForegroundColor White
Write-Host "   cd build" -ForegroundColor Gray
Write-Host "   cmake -G `"MinGW Makefiles`" -DCMAKE_PREFIX_PATH=`"C:/Qt/6.7.3/mingw_64`" .." -ForegroundColor Gray
Write-Host "   mingw32-make" -ForegroundColor Gray
Write-Host "3. Tester le module Ravitaillement dans l'application" -ForegroundColor White
Write-Host ""

Write-Host "En cas de problème, restaurez la sauvegarde:" -ForegroundColor Yellow
Write-Host "   Copy-Item $backupFile mainwindow.cpp -Force" -ForegroundColor Gray
Write-Host ""
