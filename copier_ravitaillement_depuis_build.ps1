# Script PowerShell pour copier la fonction showStockRavitaillementTab complète
# Exécution depuis le dossier build/

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "COPIE FONCTION RAVITAILLEMENT COMPLETE" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Chemins des fichiers (relatifs au dossier build)
$sourceFile = "../projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)/mainwindow.cpp"
$destFile = "../mainwindow.cpp"
$backupFile = "../mainwindow.cpp.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

# Vérifier que les fichiers existent
if (-not (Test-Path $sourceFile)) {
    Write-Host "ERREUR: Fichier source introuvable: $sourceFile" -ForegroundColor Red
    Write-Host "Chemin absolu attendu: $(Resolve-Path $sourceFile -ErrorAction SilentlyContinue)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Assurez-vous que le dossier Integration est présent dans le répertoire parent." -ForegroundColor Yellow
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
$sourceLines = $sourceContent -split "`r?`n"
Write-Host "  Lignes totales dans le fichier source: $($sourceLines.Count)" -ForegroundColor Gray

# PowerShell utilise des index 0-based, donc ligne 5897 = index 5896
$startIndex = 5896
$endIndex = 8760
$functionLines = $sourceLines[$startIndex..$endIndex]
$completeFunction = $functionLines -join "`n"

Write-Host "✓ Fonction extraite: $($functionLines.Count) lignes" -ForegroundColor Green
Write-Host ""

# Trouver et remplacer la fonction incomplète dans le fichier destination
Write-Host "Recherche de la fonction incomplète dans mainwindow.cpp..." -ForegroundColor Yellow

# Trouver le début de la fonction
$startPattern = [regex]::Escape("void MainWindow::showStockRavitaillementTab() {")
# Trouver la fin (la fonction suivante)
$endPattern = [regex]::Escape("void MainWindow::showStockCalculTab() {")

# Vérifier que les patterns existent
if ($destContent -match $startPattern) {
    Write-Host "✓ Début de fonction trouvé" -ForegroundColor Green
} else {
    Write-Host "ERREUR: Début de fonction introuvable" -ForegroundColor Red
    exit 1
}

if ($destContent -match $endPattern) {
    Write-Host "✓ Fin de fonction trouvée (showStockCalculTab)" -ForegroundColor Green
} else {
    Write-Host "ERREUR: Fonction suivante (showStockCalculTab) introuvable" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Remplacement de la fonction..." -ForegroundColor Yellow

# Utiliser une regex pour capturer tout entre les deux fonctions
$pattern = "(?s)($startPattern.*?)($endPattern)"

if ($destContent -match $pattern) {
    # Remplacer la fonction incomplète par la fonction complète
    $newContent = $destContent -replace $pattern, ($completeFunction + "`n`n" + $endPattern)
    
    # Écrire le nouveau contenu
    Write-Host "Écriture du nouveau contenu..." -ForegroundColor Yellow
    [System.IO.File]::WriteAllText((Resolve-Path $destFile).Path, $newContent, [System.Text.Encoding]::UTF8)
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✓ COPIE RÉUSSIE!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "La fonction showStockRavitaillementTab() complète a été copiée." -ForegroundColor White
    Write-Host "Taille: $($functionLines.Count) lignes" -ForegroundColor White
    Write-Host ""
    Write-Host "Prochaines étapes:" -ForegroundColor Cyan
    Write-Host "1. Compiler le projet: mingw32-make" -ForegroundColor White
    Write-Host "2. Exécuter le script SQL: ../sql/creer_tables_ravitaillement.sql" -ForegroundColor White
    Write-Host "3. Tester le module dans l'application" -ForegroundColor White
    Write-Host ""
    Write-Host "En cas de problème, restaurez depuis: $backupFile" -ForegroundColor Yellow
    
} else {
    Write-Host "ERREUR: Impossible de trouver la fonction dans le fichier destination" -ForegroundColor Red
    Write-Host "Pattern recherché: $startPattern ... $endPattern" -ForegroundColor Yellow
    exit 1
}
