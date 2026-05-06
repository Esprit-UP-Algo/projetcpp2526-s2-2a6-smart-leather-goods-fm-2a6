# Script simplifié pour réparer l'encodage UTF-8

Write-Host "Réparation encodage UTF-8..." -ForegroundColor Cyan

# Lire en UTF-8
$content = Get-Content mainwindow.cpp -Raw -Encoding UTF8

# Remplacer les caractères mal encodés
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

# Sauvegarder en UTF-8 sans BOM
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllText("$PWD\mainwindow.cpp", $content, $utf8NoBom)

Write-Host "✅ Terminé! Fichier sauvegardé en UTF-8" -ForegroundColor Green
