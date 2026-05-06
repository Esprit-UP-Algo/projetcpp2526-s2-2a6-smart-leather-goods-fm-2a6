# Script pour réparer TOUS les caractères mal encodés et forcer UTF-8

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  RÉPARATION ENCODAGE UTF-8" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Lire le fichier en UTF-8
Write-Host "[1/3] Lecture du fichier..." -ForegroundColor Yellow
$content = Get-Content mainwindow.cpp -Raw -Encoding UTF8

# Compter les problèmes avant
$problemesAvant = ([regex]::Matches($content, 'Ã©|Ã¨|Ã |Ã´|Ã§|Ãª|Ã«|Ã®|Ã¯|Ã¹|Ã»|É|À')).Count
Write-Host "   Problèmes détectés: $problemesAvant" -ForegroundColor Red

# Remplacer TOUS les caractères mal encodés
Write-Host "[2/3] Réparation des caractères..." -ForegroundColor Yellow

# Voyelles avec accents
$content = $content -replace 'Ã©', 'é'
$content = $content -replace 'Ã¨', 'è'
$content = $content -replace 'Ã ', 'à'
$content = $content -replace 'Ã´', 'ô'
$content = $content -replace 'Ã§', 'ç'
$content = $content -replace 'Ãª', 'ê'
$content = $content -replace 'Ã«', 'ë'
$content = $content -replace 'Ã®', 'î'
$content = $content -replace 'Ã¯', 'ï'
$content = $content -replace 'Ã¹', 'ù'
$content = $content -replace 'Ã»', 'û'

# Majuscules
$content = $content -replace 'É', 'É'
$content = $content -replace 'À', 'À'

# Symboles spéciaux
$content = $content -replace 'â€"', '—'
$content = $content -replace 'â€™', "'"
$content = $content -replace 'â€œ', '"'
$content = $content -replace 'â€', '"'

Write-Host "   ✅ Caractères réparés!" -ForegroundColor Green

# Sauvegarder en UTF-8 SANS BOM
Write-Host "[3/3] Sauvegarde en UTF-8..." -ForegroundColor Yellow
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllText("$PWD\mainwindow.cpp", $content, $utf8NoBom)

Write-Host "   ✅ Fichier sauvegardé en UTF-8!" -ForegroundColor Green
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  RÉPARATION TERMINÉE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Résultat:" -ForegroundColor White
Write-Host "  - Problèmes avant: $problemesAvant" -ForegroundColor Yellow
Write-Host "  - Encodage: UTF-8 (sans BOM)" -ForegroundColor Green
Write-Host "  - Fichier: mainwindow.cpp" -ForegroundColor Cyan
Write-Host ""
