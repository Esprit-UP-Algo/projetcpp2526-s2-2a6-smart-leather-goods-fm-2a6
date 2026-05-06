# Script pour corriger TOUS les accents dans mainwindow.cpp

$content = Get-Content mainwindow.cpp -Raw -Encoding UTF8

# Corrections des labels principaux
$content = $content -replace 'Parametres de Production', 'Paramètres de Production'
$content = $content -replace 'Quantite a produire', 'Quantité à produire'
$content = $content -replace 'Type de Matiere', 'Type de Matière'
$content = $content -replace 'Scenario Demande', 'Scénario Demande'
$content = $content -replace 'Qualite requise', 'Qualité requise'
$content = $content -replace 'Apercu produit selectionne', 'Aperçu produit sélectionné'
$content = $content -replace 'Resultats du Calcul', 'Résultats du Calcul'
$content = $content -replace 'Analyse Mathematique', 'Analyse Mathématique'

# Corrections des textes dans les résultats
$content = $content -replace 'Surface necessaire par unite', 'Surface nécessaire par unité'
$content = $content -replace 'Scenario applique', 'Scénario appliqué'
$content = $content -replace 'Surface totale justifiee', 'Surface totale justifiée'
$content = $content -replace 'Deficit estime', 'Déficit estimé'
$content = $content -replace 'Stock utilise', 'Stock utilisé'
$content = $content -replace 'Qualite A', 'Qualité A'
$content = $content -replace 'Qualite B', 'Qualité B'
$content = $content -replace 'Qualite C', 'Qualité C'
$content = $content -replace 'Toutes qualites', 'Toutes qualités'
$content = $content -replace 'Valider le Lancement Reserver la Matiere', 'Valider le Lancement Réserver la Matière'

# Sauvegarder
$content | Out-File mainwindow.cpp -Encoding UTF8 -NoNewline

Write-Host "✅ Tous les accents corriges!" -ForegroundColor Green
