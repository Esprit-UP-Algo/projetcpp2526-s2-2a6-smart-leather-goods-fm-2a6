# Script PowerShell pour copier la fonction showStockRavitaillementTab complète
# depuis le dossier Integration vers le projet principal

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "COPIE FONCTION RAVITAILLEMENT COMPLETE" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Chemins des fichiers
$sourceFile = "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)/mainwindow.cpp"
$destFile = "mainwindow.cpp"
$backupFile = "mainwindow.cpp.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

# Vérifier que les fichiers existent
if (-not (Test-Path $sourceFile)) {
    Write-Host "ERREUR: Fichier source introuvable: $sourceFile" -ForegroundColor Red
    Write-Host "Assurez-vous que le dossier Integration est présent." -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $destFile)) {
    Write-Host "ERREUR: Fichier destination introuvable: $destFile" -ForegroundColor Red
    exit 1
}

Write-Host "✓ Fichiers trouvés" -ForegroundColor Green
Write-Host "  Source: $sourceFile" -ForegroundColor Gray
Write-Host "  Destination: $destFile" -ForegroundColor Gray
Write-Host ""

# Créer une sauvegarde
Write-Host "Création de la sauvegarde..." -ForegroundColor Yellow
Copy-Item $destFile $backupFile -Force
Write-Host "✓ Sauvegarde créée: $backupFile" -ForegroundColor Green
Write-Host ""

# Lire les fichiers
Write-Host "Lecture des fichiers..." -ForegroundColor Yellow
$sourceContent = Get-Content $sourceFile -Raw -Encoding UTF8
$destContent = Get-Content $destFile -Raw -Encoding UTF8

# Extraire la fonction complète du fichier source (lignes 5897-8761)
Write-Host "Extraction de la fonction complète depuis Integration..." -ForegroundColor Yellow
$sourceLines = $sourceContent -split "`n"
$functionLines = $sourceLines[5896..8760]  # PowerShell utilise des index 0-based
$completeFunction = $functionLines -join "`n"

Write-Host "✓ Fonction extraite: $($functionLines.Count) lignes" -ForegroundColor Green
Write-Host ""

# Trouver et remplacer la fonction incomplète dans le fichier destination
Write-Host "Recherche de la fonction incomplète dans mainwindow.cpp..." -ForegroundColor Yellow

# Pattern pour trouver le début de la fonction
$startPattern = "void MainWindow::showStockRavitaillementTab\(\) \{"
# Pattern pour trouver la fin (la fonction suivante)
$endPattern = "void MainWindow::showStockCalculTab\(\) \{"

# Trouver les positions
if ($destContent -match "(?s)($startPattern.*?)($endPattern)") {
    Write-Host "✓ Fonction incomplète trouvée" -ForegroundColor Green
    
    # Remplacer la fonction incomplète par la fonction complète
    $newContent = $destContent -replace "(?s)$startPattern.*?(?=$endPattern)", $completeFunction
    
    # Écrire le nouveau contenu
    Write-Host "Écriture du nouveau contenu..." -ForegroundColor Yellow
    $newContent | Out-File -FilePath $destFile -Encoding UTF8 -NoNewline
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✓ COPIE RÉUSSIE!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "La fonction showStockRavitaillementTab() complète a été copiée." -ForegroundColor White
    Write-Host ""
    Write-Host "Prochaines étapes:" -ForegroundColor Cyan
    Write-Host "1. Vérifier que le fichier compile sans erreurs" -ForegroundColor White
    Write-Host "2. Exécuter le script SQL: sql/creer_tables_ravitaillement.sql" -ForegroundColor White
    Write-Host "3. Tester le module dans l'application" -ForegroundColor White
    Write-Host ""
    Write-Host "En cas de problème, restaurez depuis: $backupFile" -ForegroundColor Yellow
    
} else {
    Write-Host "ERREUR: Impossible de trouver la fonction dans le fichier destination" -ForegroundColor Red
    Write-Host "Vérifiez que la fonction showStockRavitaillementTab existe dans mainwindow.cpp" -ForegroundColor Yellow
    exit 1
}
