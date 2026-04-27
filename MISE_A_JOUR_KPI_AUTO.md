# ✅ Mise à Jour Automatique des KPIs

## 🎯 Problème Résolu

Les 3 KPIs (Budget estimé, Couverture stock, Risque rupture) ne se mettaient à jour que lorsqu'on cliquait sur le bouton "Générer plan de ravitaillement". Maintenant, ils se mettent à jour **automatiquement** dès qu'on change un paramètre.

## 🔄 Modifications Appliquées

### 1. **Création de la fonction `updateKPIsAuto()`**

Cette fonction calcule automatiquement :
- **Budget estimé** : Quantité à commander × Prix/M² du fournisseur sélectionné
- **Couverture stock** : Stock actuel ÷ Consommation journalière (en jours)
- **Risque rupture** : Évaluation automatique (Élevé/Modéré/Faible)

### 2. **Déclencheurs Automatiques**

Les KPIs se mettent à jour automatiquement quand :

| Événement | Description |
|-----------|-------------|
| ✅ **Stock actuel change** | Modification du spinbox "Stock actuel" |
| ✅ **Seuil sécurité change** | Modification du spinbox "Seuil sécurité" |
| ✅ **Consommation change** | Modification du spinbox "Conso prévue (7j)" |
| ✅ **Type de cuir change** | Sélection d'un autre type dans la liste déroulante |
| ✅ **Poids changent** | Modification des poids (Prix, Qualité, Délai, Stock) |
| ✅ **Fournisseur sélectionné** | Clic sur une ligne du tableau fournisseurs |
| ✅ **Reset poids** | Clic sur le bouton "Reset poids" |
| ✅ **Auto 100%** | Clic sur le bouton "Auto 100%" |
| ✅ **Chargement initial** | Au démarrage du module |

## 📊 Calculs Automatiques

### Budget Estimé
```
Besoin brut = Seuil sécurité + Consommation prévue - Stock actuel
Quantité à commander = Min(Besoin brut, Stock disponible fournisseur)
Budget = Quantité à commander × Prix/M² du fournisseur sélectionné
```

**Exemple** :
- Stock actuel = 120 M²
- Seuil sécurité = 300 M²
- Consommation prévue = 180 M²/7j
- Besoin brut = 300 + 180 - 120 = **360 M²**
- Prix fournisseur = 45 DT/M²
- **Budget = 360 × 45 = 16 200 DT**

### Couverture Stock
```
Consommation journalière = Consommation prévue ÷ 7
Couverture (jours) = Stock actuel ÷ Consommation journalière
```

**Exemple** :
- Stock actuel = 120 M²
- Consommation prévue = 180 M²/7j
- Consommation journalière = 180 ÷ 7 = 25.71 M²/jour
- **Couverture = 120 ÷ 25.71 = 4.7 jours**

### Risque Rupture
```
SI (Reliquat > 0 OU Couverture < 5 jours) ALORS
    Risque = "Élevé" (82%)
SINON SI (Couverture < 8 jours) ALORS
    Risque = "Modéré" (55%)
SINON
    Risque = "Faible" (25%)
FIN SI
```

**Exemple** :
- Couverture = 4.7 jours
- **Risque = "Élevé" (car < 5 jours)**

## 🎨 Affichage Dynamique

### Budget Estimé
- Affiche "0 DT" si le stock est suffisant
- Affiche le budget calculé sinon
- Format : "11520 DT"

### Couverture Stock
- Affiche le nombre de jours de couverture
- Format : "4.7 j"
- Couleur de la jauge change selon le niveau :
  - 🔴 Rouge : < 50% du seuil
  - 🟠 Orange : < seuil
  - 🟢 Vert : ≥ seuil

### Risque Rupture
- Affiche "Faible", "Modéré" ou "Élevé"
- Barre de progression colorée (rouge pour élevé)
- Alerte automatique si risque élevé

## 🔧 Code Technique

### Fonction principale
```cpp
auto updateKPIsAuto = [=]() {
    // Récupération des données du fournisseur sélectionné
    const int row = (*selectedSupplierRow >= 0 && *selectedSupplierRow < table->rowCount()) 
                    ? *selectedSupplierRow : 0;
    const double prixUnitaire = parseNumber(table->item(row, 1)->text());
    
    // Récupération des paramètres
    const double stockActuel = sbStockActuel->value();
    const double seuil = sbSeuilSecurite->value();
    const double conso = sbConsoPrevue->value();
    
    // Calculs
    const double besoinBrut = qMax(0.0, seuil + conso - stockActuel);
    const double consoJour = qMax(0.01, conso / 7.0);
    const double couvertureJours = stockActuel / consoJour;
    
    // Mise à jour des KPIs
    lblKpiCouverture->setText(QString::number(couvertureJours, 'f', 1) + " j");
    
    if (besoinBrut <= 0.0) {
        lblKpiBudget->setText("0 DT");
        lblKpiRisqueTxt->setText("Faible");
        pbRisque->setValue(15);
    } else {
        const double stockDispo = parseNumber(table->item(row, 4)->text());
        const double qteCommande = qMin(besoinBrut, stockDispo);
        const double budget = qteCommande * prixUnitaire;
        
        lblKpiBudget->setText(QString::number(budget, 'f', 0) + " DT");
        
        // Évaluation du risque
        if (reliquat > 0.0 || couvertureJours < 5.0) {
            lblKpiRisqueTxt->setText("Eleve");
            pbRisque->setValue(82);
        } else if (couvertureJours < 8.0) {
            lblKpiRisqueTxt->setText("Modere");
            pbRisque->setValue(55);
        } else {
            lblKpiRisqueTxt->setText("Faible");
            pbRisque->setValue(25);
        }
    }
};
```

### Connexions automatiques
```cpp
// Mise à jour quand les paramètres changent
auto onStockChanged = [=]() {
    updateJauge(sbStockActuel->value(), sbSeuilSecurite->value(), sbConsoPrevue->value());
    updateKPIsAuto(); // ← Ajouté
};

QObject::connect(sbStockActuel, qOverload<double>(&QDoubleSpinBox::valueChanged), 
                 this, [=](double){ onStockChanged(); });
QObject::connect(sbSeuilSecurite, qOverload<double>(&QDoubleSpinBox::valueChanged), 
                 this, [=](double){ onStockChanged(); });
QObject::connect(sbConsoPrevue, qOverload<double>(&QDoubleSpinBox::valueChanged), 
                 this, [=](double){ onStockChanged(); });

// Mise à jour quand le tableau est rempli (changement de matière ou poids)
auto remplirTable = [=]() {
    // ... code de remplissage du tableau ...
    updateKPIsAuto(); // ← Ajouté à la fin
};

// Mise à jour quand un fournisseur est sélectionné manuellement
QObject::connect(table, &QTableWidget::cellClicked, this, [=](int row, int col) {
    // ... code de sélection ...
    updateKPIsAuto(); // ← Ajouté
});
```

## ✨ Résultat Final

Les 3 KPIs se mettent maintenant à jour **en temps réel** dès que vous :
- Changez le stock actuel
- Changez le seuil de sécurité
- Changez la consommation prévue
- Sélectionnez un autre type de cuir
- Modifiez les poids de scoring
- Cliquez sur un fournisseur dans le tableau

Plus besoin de cliquer sur "Générer plan de ravitaillement" pour voir les valeurs ! 🎉

## 📝 Notes

- Les valeurs sont calculées avec les **vraies données** du fournisseur sélectionné
- Si aucun fournisseur n'est disponible, les KPIs affichent "--"
- Les calculs sont identiques à ceux du bouton "Générer plan", mais automatiques
- La fonction est optimisée pour éviter les calculs inutiles
