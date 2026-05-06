# Script complet pour corriger TOUS les textes mal encodés

$c = Get-Content mainwindow.cpp -Raw -Encoding UTF8

# Corrections principales déjà faites
$c = $c -creplace 'Parametres de Production', 'Paramètres de Production'
$c = $c -creplace 'Quantite a produire', 'Quantité à produire'
$c = $c -creplace 'Type de Matiere', 'Type de Matière'
$c = $c -creplace 'Scenario Demande', 'Scénario Demande'
$c = $c -creplace 'Qualite requise', 'Qualité requise'
$c = $c -creplace 'Apercu produit selectionne', 'Aperçu produit sélectionné'
$c = $c -creplace 'Resultats du Calcul', 'Résultats du Calcul'
$c = $c -creplace 'Analyse Mathematique', 'Analyse Mathématique'

# Corrections des textes restants
$c = $c -creplace 'QuantitÃ© Ã  produire', 'Quantité à produire'
$c = $c -creplace 'Type de MatiÃ¨re', 'Type de Matière'
$c = $c -creplace 'ScÃ©nario Demande', 'Scénario Demande'
$c = $c -creplace 'QualitÃ© requise', 'Qualité requise'
$c = $c -creplace 'qualitÃ©', 'qualité'
$c = $c -creplace 'QualitÃ©', 'Qualité'
$c = $c -creplace 'QUALITÃ‰', 'QUALITÉ'
$c = $c -creplace 'mÃªme', 'même'
$c = $c -creplace 'MatiÃ¨re', 'Matière'
$c = $c -creplace 'matiÃ¨re', 'matière'
$c = $c -creplace 'ModÃ¨le', 'Modèle'
$c = $c -creplace 'RÃ©server', 'Réserver'
$c = $c -creplace 'rÃ©server', 'réserver'
$c = $c -creplace 'RÃ©servation', 'Réservation'
$c = $c -creplace 'rÃ©servÃ©e', 'réservée'
$c = $c -creplace 'scÃ©nario', 'scénario'
$c = $c -creplace 'ScÃ©nario', 'Scénario'
$c = $c -creplace 'scÃ©narios', 'scénarios'
$c = $c -creplace 'commandÃ©e', 'commandée'
$c = $c -creplace 'estimÃ©', 'estimé'
$c = $c -creplace 'EstimÃ©', 'Estimé'
$c = $c -creplace 'recommandÃ©', 'recommandé'
$c = $c -creplace 'ajoutÃ©s', 'ajoutés'
$c = $c -creplace 'passÃ©e', 'passée'
$c = $c -creplace 'auprÃ¨s', 'auprès'
$c = $c -creplace 'dÃ©duite', 'déduite'
$c = $c -creplace 'dÃ©ficit', 'déficit'
$c = $c -creplace 'DÃ©ficit', 'Déficit'
$c = $c -creplace 'rÃ©el', 'réel'
$c = $c -creplace 'RÃ©el', 'Réel'
$c = $c -creplace 'RÃ©duire', 'Réduire'
$c = $c -creplace 'dÃ©coupe', 'découpe'
$c = $c -creplace 'CoÃ»t', 'Coût'
$c = $c -creplace 'investissement', 'investissement'
$c = $c -creplace 'DÃ©duction', 'Déduction'
$c = $c -creplace 'sÃ©lectionnÃ©e', 'sélectionnée'
$c = $c -creplace 'sÃ©lectionnÃ©s', 'sélectionnés'
$c = $c -creplace 'ModÃ©rÃ©', 'Modéré'
$c = $c -creplace 'sÃ©curiser', 'sécuriser'
$c = $c -creplace 'ProcÃ©dez', 'Procédez'
$c = $c -creplace 'rÃ©approvisionnement', 'réapprovisionnement'
$c = $c -creplace 'quantitÃ©', 'quantité'
$c = $c -creplace 'QuantitÃ©', 'Quantité'
$c = $c -creplace 'meilleure qualitÃ©', 'meilleure qualité'
$c = $c -creplace 'toutes qualitÃ©s', 'toutes qualités'

# Corrections des symboles
$c = $c -creplace 'â†'', '→'
$c = $c -creplace 'â€"', '—'
$c = $c -creplace 'Ã ', 'à'
$c = $c -creplace 'Ã©', 'é'
$c = $c -creplace 'Ã¨', 'è'
$c = $c -creplace 'Ãª', 'ê'

# Sauvegarder
$c | Out-File mainwindow.cpp -Encoding UTF8 -NoNewline

Write-Host "✅ TOUS les textes corrigés!" -ForegroundColor Green
