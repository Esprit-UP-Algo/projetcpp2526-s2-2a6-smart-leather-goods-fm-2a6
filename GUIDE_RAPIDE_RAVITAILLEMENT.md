# 🚀 Guide Rapide - Intégration Module Ravitaillement

## ⚡ Installation en 3 Étapes

### Étape 1: Créer les Tables Oracle (5 minutes)

```bash
# Ouvrir SQL Developer ou SQL*Plus
# Exécuter le script:
@sql/creer_tables_ravitaillement.sql
```

**Ce script va créer:**
- ✅ Table FOURNISSEURS (avec 9 fournisseurs de test)
- ✅ Table RAVITAILLEMENT_LOG (historique des commandes)
- ✅ Table CALCUL_BESOINS_LOG (historique des calculs)
- ✅ 3 Vues utiles
- ✅ 2 Procédures stockées

### Étape 2: Copier le Code (2 minutes)

**Option A - Automatique (Recommandé)**
```powershell
# Exécuter le script PowerShell:
.\copier_module_ravitaillement.ps1
```

**Option B - Manuel**
1. Ouvrir les deux fichiers:
   - Source: `projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)/mainwindow.cpp`
   - Destination: `mainwindow.cpp`

2. Dans le fichier source, copier la fonction `showStockRavitaillementTab()` complète (ligne ~5897 à ~11000)

3. Dans le fichier destination, remplacer la fonction existante (ligne 13315+)

### Étape 3: Compiler et Tester (3 minutes)

```bash
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/mingw_64" ..
mingw32-make
./ProjetCpp.exe
```

## 🎯 Vérification Rapide

Une fois l'application lancée:

1. ✅ Aller dans **Stock** → **🚚 Ravitaillement**
2. ✅ Vérifier que le stepper visuel s'affiche (①→②→③)
3. ✅ Sélectionner "Cuir Vachette"
4. ✅ Cliquer sur "Analyser fournisseurs"
5. ✅ Vérifier que les 3 fournisseurs s'affichent dans le tableau
6. ✅ Cliquer sur "Générer plan de ravitaillement"
7. ✅ Vérifier que la carte de synthèse s'affiche
8. ✅ Vérifier que le graphique de projection s'affiche

## 📊 Fonctionnalités Principales

### 1. Workflow Visuel
- **① Constat**: Stock actuel & seuil de sécurité
- **② Analyse**: Scoring automatique des fournisseurs
- **③ Décision**: Plan d'achat optimal avec budget

### 2. Alertes Intelligentes
- 🚨 **Critique**: Stock < 3 jours (rouge)
- ⚠️ **Modéré**: Stock < 7 jours (orange)
- ✅ **Normal**: Stock > 7 jours (vert)

### 3. Analyse Fournisseurs
- Chargement automatique depuis Oracle
- Scoring multi-critères (Prix 35% + Qualité 30% + Délai 20% + Stock 15%)
- Sélection manuelle possible

### 4. KPIs Temps Réel
- Fournisseur sélectionné
- Budget estimé (DT)
- Couverture stock (jours)
- Risque rupture (%)

### 5. Scénarios d'Achat
- **Conservateur**: 70% du besoin
- **Optimal**: 100% du besoin (recommandé)
- **Anticipatif**: 140% du besoin

### 6. Historique & Export
- Tableau filtrable par date et matière
- Graphique d'évolution du budget
- Export PDF des rapports
- Sauvegarde automatique en base

## 🔧 Dépannage

### Problème: "Table FOURNISSEURS n'existe pas"
**Solution**: Exécuter le script SQL `creer_tables_ravitaillement.sql`

### Problème: "Aucun fournisseur trouvé"
**Solution**: Vérifier que les fournisseurs sont bien insérés:
```sql
SELECT * FROM FOURNISSEURS WHERE ACTIF = '1';
```

### Problème: Graphiques ne s'affichent pas
**Solution**: Vérifier que Qt Charts est lié dans CMakeLists.txt:
```cmake
find_package(Qt6 REQUIRED COMPONENTS Charts)
target_link_libraries(ProjetCpp PRIVATE Qt6::Charts)
```

### Problème: Images ne s'affichent pas
**Solution**: Vérifier que les ressources sont dans le fichier .qrc

### Problème: Erreur de compilation
**Solution**: Vérifier que tous les includes sont présents:
```cpp
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QPrinter>
#include <QTextDocument>
```

## 📁 Fichiers Créés

- ✅ `INTEGRATION_RAVITAILLEMENT_COMPLETE.md` - Documentation complète
- ✅ `copier_module_ravitaillement.ps1` - Script de copie automatique
- ✅ `sql/creer_tables_ravitaillement.sql` - Script SQL complet
- ✅ `GUIDE_RAPIDE_RAVITAILLEMENT.md` - Ce guide

## 🎓 Utilisation Avancée

### Optimisation EOQ/Wilson
Cliquez sur "⚙️ Paramètres Avancés" pour accéder à:
- Calcul de la quantité économique de commande
- Stock de sécurité optimal
- Coût de passation
- Taux de stockage annuel

### Module Calcul Besoins
Onglet séparé pour calculer les besoins en matière:
- Sélection produit (Sac, Portefeuille, Ceinture)
- Scénarios (Normal, Pic saisonnier, Urgent VIP)
- Qualité requise (A/B/C/Toutes)
- Analyse de substitution
- Export PDF

### Historique Détaillé
- Filtrage par date et matière
- Graphique d'évolution
- Export CSV/PDF
- Statistiques agrégées

## 💡 Conseils d'Utilisation

1. **Vérifiez le stock régulièrement** (au moins 1x/semaine)
2. **Utilisez les scénarios** pour anticiper les pics de demande
3. **Consultez l'historique** pour optimiser les commandes futures
4. **Exportez les rapports** pour la traçabilité
5. **Activez les alertes** pour ne jamais être en rupture

## 📞 Support

Pour toute question ou problème:
1. Consultez `INTEGRATION_RAVITAILLEMENT_COMPLETE.md`
2. Vérifiez les logs de compilation
3. Vérifiez la connexion Oracle
4. Vérifiez que toutes les tables existent

---

**Version**: 1.0
**Date**: 05/05/2026
**Auteur**: Kiro AI Assistant

🎉 **Bon travail avec le module Ravitaillement!**
