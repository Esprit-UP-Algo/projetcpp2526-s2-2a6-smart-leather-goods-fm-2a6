# Script PowerShell pour corriger l'encodage du fichier mainwindow.cpp

$content = Get-Content -Path "mainwindow.cpp" -Raw -Encoding UTF8

# Corrections d'encodage
$corrections = @{
    'âš–ï¸' = '⚖️'
    'â€"' = '—'
    'â' ' = '①'
    'â'¡' = '②'
    'â'¢' = '③'
    'ðŸ"Š' = '📊'
    'AperÃ§u produit sÃ©lectionnÃ©' = 'Aperçu produit sélectionné'
    'RÃ©sultats du Calcul' = 'Résultats du Calcul'
    'MathÃ©matique' = 'Mathématique'
    'rÃ©duire' = 'réduire'
    'automatisÃ©' = 'automatisé'
    'Verifiez' = 'Vérifiez'
    'Ã©lÃ©gante' = 'élégante'
    'Ã©levÃ©e' = 'élevée'
    'prÃ©cision' = 'précision'
    'rÃ©gularitÃ©' = 'régularité'
    'rÃ©sistance' = 'résistance'
}

foreach ($key in $corrections.Keys) {
    $content = $content.Replace($key, $corrections[$key])
}

# Corriger les chemins d'images
$content = $content.Replace('chemin = ":/sac main.webp";', 'chemin = "sac main";')
$content = $content.Replace('chemin = ":/portfeuille.webp";', 'chemin = "portefeuille";')
$content = $content.Replace('chemin = ":/ceinture.webp";', 'chemin = "ceinture";')
$content = $content.Replace('chemin = ":/sac voyage.jpg";', 'chemin = "sac voyage";')

# Corriger le chargement d'image
$oldLoad = 'QPixmap pix(chemin);'
$newLoad = 'QPixmap pix = loadIllustration(chemin);'
$content = $content.Replace($oldLoad, $newLoad)

# Sauvegarder
$content | Out-File -FilePath "mainwindow.cpp" -Encoding UTF8 -NoNewline

Write-Host "✅ Corrections appliquées!" -ForegroundColor Green
