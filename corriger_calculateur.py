#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script pour corriger les problèmes d'encodage et d'images dans le module Calculateur
"""

import re

# Lire le fichier
with open('mainwindow.cpp', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# Dictionnaire de corrections d'encodage
corrections = {
    # Caractères spéciaux mal encodés
    'âš–ï¸': '⚖️',
    'â€"': '—',
    'â' ': '①',
    'â'¡': '②',
    'â'¢': '③',
    'ðŸ"Š': '📊',
    'â†'': '→',
    'Ã©': 'é',
    'Ã¨': 'è',
    'Ã ': 'à',
    'Ã´': 'ô',
    'Ã§': 'ç',
    'Ã«': 'ë',
    'Ã®': 'î',
    'Ã¯': 'ï',
    'Ã¹': 'ù',
    'Ã»': 'û',
    'Ã‰': 'É',
    'Ã€': 'À',
    'Ãª': 'ê',
    'â€™': "'",
    'â€œ': '"',
    'â€': '"',
    
    # Corrections spécifiques
    'AperÃ§u produit sÃ©lectionnÃ©': 'Aperçu produit sélectionné',
    'RÃ©sultats du Calcul': 'Résultats du Calcul',
    'MathÃ©matique': 'Mathématique',
    'rÃ©duire': 'réduire',
    'automatisÃ©': 'automatisé',
    'nÃ©cessaire': 'nécessaire',
    'ScÃ©nario': 'Scénario',
    'qualitÃ©': 'qualité',
    'QUALITÃ‰': 'QUALITÉ',
    'mÃªme': 'même',
    'dÃ©jÃ ': 'déjà',
    'Ã©conomie': 'économie',
    'Ã©conomique': 'économique',
    'prÃ©cision': 'précision',
    'rÃ©gularitÃ©': 'régularité',
    'rÃ©sistance': 'résistance',
    'Ã©lÃ©gante': 'élégante',
    'Ã©levÃ©e': 'élevée',
    'Ã©levÃ©': 'élevé',
    'crÃ©Ã©': 'créé',
    'EstimÃ©': 'Estimé',
    'estimÃ©': 'estimé',
    'estimÃ©e': 'estimée',
    'DÃ©ficit': 'Déficit',
    'dÃ©ficit': 'déficit',
    'MÃ©tier': 'Métier',
    'IntÃ©gration': 'Intégration',
    'OpÃ©ration': 'Opération',
    'VÃ©rifiez': 'Vérifiez',
    'Verifiez': 'Vérifiez',
}

# Appliquer toutes les corrections
for old, new in corrections.items():
    content = content.replace(old, new)

# Corriger les chemins d'images pour utiliser loadIllustration
image_fixes = [
    # Sac Main
    ('chemin = ":/sac main.webp";', 'chemin = "sac main";'),
    # Portefeuille
    ('chemin = ":/portfeuille.webp";', 'chemin = "portefeuille";'),
    # Ceinture
    ('chemin = ":/ceinture.webp";', 'chemin = "ceinture";'),
    # Sac Voyage
    ('chemin = ":/sac voyage.jpg";', 'chemin = "sac voyage";'),
]

for old, new in image_fixes:
    content = content.replace(old, new)

# Corriger le chargement d'image pour utiliser loadIllustration
old_image_load = '''        QPixmap pix(chemin);
        if (pix.isNull()) {
            photoProduit->setText("Image indisponible");
            photoProduitSousTitre->setText("Vérifiez la ressource image du produit dans le projet.");
            return;
        }

        photoProduit->setPixmap(pix.scaled(photoProduit->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));'''

new_image_load = '''        QPixmap pix = loadIllustration(chemin);
        if (pix.isNull()) {
            photoProduit->setText("Image indisponible");
            photoProduitSousTitre->setText("Vérifiez la ressource image du produit dans le projet.");
            return;
        }

        photoProduit->setPixmap(safeScalePixmap(pix, photoProduit->size()));'''

content = content.replace(old_image_load, new_image_load)

# Sauvegarder le fichier corrigé
with open('mainwindow.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("✅ Corrections appliquées avec succès!")
print("   - Encodage UTF-8 corrigé")
print("   - Chemins d'images corrigés")
print("   - Fonction loadIllustration utilisée")
