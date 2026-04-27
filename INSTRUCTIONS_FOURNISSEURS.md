# 📋 Instructions - Module Fournisseurs avec Données Réelles

## ✅ Modifications Appliquées

### 1. **Création de la table FOURNISSEURS dans Oracle**
   - Fichier SQL créé : `oracle_fournisseurs.sql`
   - Structure de la table avec tous les champs nécessaires
   - Données initiales pour 3 types de cuir (Vachette, Agneau, Veau)
   - 3 fournisseurs par type de cuir (9 fournisseurs au total)

### 2. **Modification du code C++ (mainwindow.cpp)**
   - La fonction `buildData()` lit maintenant les données depuis Oracle
   - Requête SQL : `SELECT NOM_FOURNISSEUR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, STOCK_DISPO FROM FOURNISSEURS WHERE TYPE_CUIR = :type_cuir AND ACTIF = '1'`
   - Les calculs utilisent les vraies données :
     - **Prix/M²** : lu depuis la colonne `PRIX_PAR_M2`
     - **Qualité** : lu depuis la colonne `QUALITE`
     - **Délai** : lu depuis la colonne `DELAI_JOURS`
     - **Stock Dispo** : lu depuis la colonne `STOCK_DISPO`

### 3. **Calculs Automatiques Réels**
   Les KPIs suivants sont calculés automatiquement avec les vraies données :
   
   #### 📊 **Top Fournisseur**
   - Calculé par scoring pondéré basé sur :
     - Poids Prix (%)
     - Poids Qualité (%)
     - Poids Délai (%)
     - Poids Stock (%)
   - Le meilleur fournisseur est affiché automatiquement
   
   #### 💰 **Budget Estimé**
   - Formule : `Budget = Quantité à commander × Prix/M² du fournisseur sélectionné`
   - Exemple : Si besoin = 150 M² et prix = 45 DT/M² → Budget = 6750 DT
   
   #### 📦 **Couverture Stock**
   - Formule : `Couverture (jours) = Stock actuel ÷ Consommation journalière`
   - Consommation journalière = Consommation prévue (7j) ÷ 7
   - Exemple : Stock = 200 M², Conso = 50 M²/semaine → Couverture = 28 jours
   
   #### ⚠️ **Risque Rupture**
   - **Élevé** : Si reliquat > 0 OU couverture < 5 jours
   - **Modéré** : Si couverture < 8 jours
   - **Faible** : Si couverture ≥ 8 jours

## 🚀 Étapes d'Installation

### Étape 1 : Exécuter le script SQL
```sql
-- Connectez-vous à Oracle SQL Developer ou SQL*Plus
-- Exécutez le fichier : oracle_fournisseurs.sql
@oracle_fournisseurs.sql
```

### Étape 2 : Vérifier les données
```sql
-- Vérifier que les fournisseurs sont bien insérés
SELECT * FROM FOURNISSEURS ORDER BY TYPE_CUIR, PRIX_PAR_M2 DESC;

-- Devrait afficher 9 fournisseurs (3 par type de cuir)
```

### Étape 3 : Compiler le projet
- Ouvrez le projet dans Qt Creator
- Compilez (Ctrl+B)
- Exécutez l'application

### Étape 4 : Tester le module
1. Lancez l'application
2. Allez dans le module **Stock & Ravitaillement**
3. Sélectionnez un type de cuir dans la liste déroulante
4. Les fournisseurs s'affichent automatiquement depuis Oracle
5. Ajustez les poids (Prix, Qualité, Délai, Stock)
6. Le scoring se recalcule automatiquement
7. Cliquez sur **"Générer plan de ravitaillement"**
8. Le budget estimé est calculé avec les vraies données

## 📊 Structure de la Table FOURNISSEURS

| Colonne | Type | Description |
|---------|------|-------------|
| ID_FOURNISSEUR | NUMBER | Clé primaire (auto-incrémentée) |
| NOM_FOURNISSEUR | VARCHAR2(100) | Nom du fournisseur |
| TYPE_CUIR | VARCHAR2(50) | Type de cuir (Cuir Vachette, Cuir Agneau, Cuir Veau) |
| PRIX_PAR_M2 | NUMBER(10,2) | Prix par mètre carré en DT |
| QUALITE | VARCHAR2(50) | Qualité (A Premium, B Standard, C Économique) |
| DELAI_JOURS | NUMBER | Délai de livraison en jours |
| STOCK_DISPO | NUMBER(10,2) | Stock disponible en M² |
| EMAIL | VARCHAR2(100) | Email du fournisseur |
| TELEPHONE | VARCHAR2(20) | Téléphone du fournisseur |
| ADRESSE | VARCHAR2(200) | Adresse du fournisseur |
| ACTIF | CHAR(1) | '1' = actif, '0' = inactif |
| DATE_CREATION | DATE | Date de création |
| DATE_MODIFICATION | DATE | Date de dernière modification |

## 🎯 Fournisseurs Initiaux

### Cuir Vachette
1. **TanLeather SA** - 45 DT/M² - A (Premium) - 5 jours - 500 M²
2. **Cuir Elite** - 38 DT/M² - B (Standard) - 7 jours - 300 M²
3. **MegaCuir** - 32 DT/M² - C (Économique) - 10 jours - 1000 M²

### Cuir Agneau
1. **SoftHide Pro** - 52 DT/M² - A (Premium) - 4 jours - 420 M²
2. **Agneau Plus** - 44 DT/M² - B+ (Standard) - 6 jours - 280 M²
3. **Cuir Sud** - 36 DT/M² - C (Économique) - 9 jours - 900 M²

### Cuir Veau
1. **Veau Prestige** - 49 DT/M² - A (Premium) - 5 jours - 380 M²
2. **Elite Veau** - 41 DT/M² - B (Standard) - 7 jours - 260 M²
3. **Market Cuir** - 34 DT/M² - C (Économique) - 11 jours - 1100 M²

## 🔧 Ajouter un Nouveau Fournisseur

```sql
INSERT INTO FOURNISSEURS (
    ID_FOURNISSEUR, 
    NOM_FOURNISSEUR, 
    TYPE_CUIR, 
    PRIX_PAR_M2, 
    QUALITE, 
    DELAI_JOURS, 
    STOCK_DISPO, 
    EMAIL, 
    TELEPHONE, 
    ADRESSE, 
    ACTIF
) VALUES (
    SEQ_FOURNISSEUR.NEXTVAL,
    'Nouveau Fournisseur',
    'Cuir Vachette',
    40.00,
    'B (Standard)',
    6,
    450.00,
    'contact@nouveau.tn',
    '+216 71 XXX XXX',
    'Adresse, Tunisie',
    '1'
);
COMMIT;
```

## ✨ Fonctionnalités Réelles

### ✅ Ce qui fonctionne maintenant avec des données réelles :

1. **Liste des fournisseurs** : Lue depuis Oracle selon le type de cuir sélectionné
2. **Prix/M²** : Valeur réelle de la base de données
3. **Qualité** : Valeur réelle (A, B, C)
4. **Délai** : Valeur réelle en jours
5. **Stock disponible** : Valeur réelle en M²
6. **Scoring automatique** : Calculé avec les vraies valeurs et les poids configurés
7. **Budget estimé** : Calculé avec le prix réel du fournisseur sélectionné
8. **Couverture stock** : Calculée avec le stock actuel et la consommation prévue
9. **Risque rupture** : Évalué automatiquement selon les seuils

### 🎨 Fallback (données par défaut)
Si la connexion Oracle échoue ou si aucun fournisseur n'est trouvé, le système utilise automatiquement les données par défaut (hardcodées) pour éviter les erreurs.

## 📝 Notes Importantes

- Les données sont filtrées par `TYPE_CUIR` et `ACTIF = '1'`
- Le tri est fait par `PRIX_PAR_M2 DESC` (du plus cher au moins cher)
- Le scoring pondéré détermine automatiquement le meilleur fournisseur
- Les couleurs des lignes changent selon la qualité (A=vert, B=jaune, C=rouge)

## 🎉 Résultat Final

Votre module **Analyse & Sélection du Fournisseur** fonctionne maintenant avec des **données réelles** depuis Oracle ! Tous les calculs (Budget, Couverture, Risque) sont automatiques et basés sur les vraies valeurs de la base de données.
