# 🔧 Correction du CRUD Matières Premières

## ❌ Problème

Le CRUD des matières premières ne fonctionne pas correctement. Possible causes :
1. Tables manquantes dans Oracle (`MATIERES_PREMIERES` et `STOCK_MP`)
2. Séquences manquantes (`SEQ_MAT`, `SEQ_STOCK_MP`)
3. Validations trop strictes dans le formulaire

## ✅ Solution

### Étape 1 : Créer les Tables Oracle

Exécutez les scripts SQL dans l'ordre suivant :

#### 1.1 Table MATIERES_PREMIERES (CRUD principal)
```sql
@oracle_matieres_premieres.sql
```

Cette table contient :
- `ID_STOCK_MP` : Identifiant unique (auto-incrémenté via `SEQ_MAT`)
- `CODE_MP` : Code unique (format: CUIR-2024-001)
- `CATEGORIE_MP` : Catégorie (Cuir, Fil)
- `NUM_LOT` : Numéro de lot (format: LOT-2024-A)
- `ETAT_MP` : État (BRUT, TEINT)
- `COULEUR` : Couleur (texte libre, 3-20 caractères)
- `QUANTITE` : Quantité en M² ou unités
- `TYPE_STOCKAGE` : Type (Sec, Froid)
- `QUALITE` : Qualité (A, B, C)

#### 1.2 Table STOCK_MP (Vue agrégée)
```sql
@oracle_stock_mp.sql
```

Cette table contient le stock agrégé par catégorie pour le module Ravitaillement.

### Étape 2 : Vérifier les Données

```sql
-- Vérifier MATIERES_PREMIERES
SELECT * FROM MATIERES_PREMIERES ORDER BY ID_STOCK_MP;

-- Vérifier STOCK_MP
SELECT * FROM STOCK_MP ORDER BY CATEGORIE_MP;

-- Vérifier les séquences
SELECT SEQ_MAT.CURRVAL FROM DUAL;
SELECT SEQ_STOCK_MP.CURRVAL FROM DUAL;
```

### Étape 3 : Formats de Saisie Requis

Pour ajouter une matière première, respectez ces formats :

| Champ | Format | Exemple | Règle |
|-------|--------|---------|-------|
| **Code MP** | `XX-20YY-NNN` | `CUIR-2024-001` | 2-4 lettres majuscules, année 20xx, 3 chiffres |
| **Catégorie** | Liste déroulante | `Cuir` ou `Fil` | Sélection obligatoire |
| **Numéro de Lot** | `LOT-20YY-X` | `LOT-2024-A` | Format fixe avec 1 lettre majuscule |
| **État** | Liste déroulante | `BRUT` ou `TEINT` | Sélection obligatoire |
| **Couleur** | Texte libre | `Marron`, `Noir` | 3-20 caractères, lettres et espaces |
| **Quantité** | Nombre décimal | `150.50` | Doit être > 0 et < 100000 |
| **Type Stockage** | Liste déroulante | `Sec` ou `Froid` | Sélection obligatoire |
| **Qualité** | Liste déroulante | `A`, `B` ou `C` | Sélection obligatoire |

### Étape 4 : Exemples de Saisie Valides

#### Exemple 1 : Cuir Vachette
```
Code MP:         CUIR-2024-001
Catégorie:       Cuir
Numéro de Lot:   LOT-2024-A
État:            BRUT
Couleur:         Marron
Quantité:        150.50
Type Stockage:   Sec
Qualité:         A
```

#### Exemple 2 : Fil
```
Code MP:         FIL-2024-001
Catégorie:       Fil
Numéro de Lot:   LOT-2024-B
État:            BRUT
Couleur:         Blanc
Quantité:        50.00
Type Stockage:   Sec
Qualité:         B
```

#### Exemple 3 : Cuir Agneau
```
Code MP:         CUIR-2024-002
Catégorie:       Cuir
Numéro de Lot:   LOT-2024-C
État:            TEINT
Couleur:         Noir
Quantité:        200.00
Type Stockage:   Froid
Qualité:         A
```

### Étape 5 : Messages d'Erreur Courants

| Erreur | Cause | Solution |
|--------|-------|----------|
| "Code MP invalide" | Format incorrect | Utilisez `XX-2024-NNN` (ex: `CUIR-2024-001`) |
| "Numéro de lot invalide" | Format incorrect | Utilisez `LOT-2024-X` (ex: `LOT-2024-A`) |
| "Couleur invalide" | Caractères spéciaux | Utilisez uniquement lettres et espaces |
| "La quantité doit être > 0" | Quantité nulle ou négative | Entrez une valeur positive |
| "Données invalides ou insertion impossible" | Erreur Oracle | Vérifiez que les tables existent |

## 🔍 Diagnostic des Problèmes

### Problème 1 : "Données invalides ou insertion impossible"

**Cause** : Table `MATIERES_PREMIERES` n'existe pas

**Solution** :
```sql
-- Vérifier si la table existe
SELECT table_name FROM user_tables WHERE table_name = 'MATIERES_PREMIERES';

-- Si vide, exécuter le script
@oracle_matieres_premieres.sql
```

### Problème 2 : "Erreur de séquence"

**Cause** : Séquence `SEQ_MAT` n'existe pas

**Solution** :
```sql
-- Vérifier si la séquence existe
SELECT sequence_name FROM user_sequences WHERE sequence_name = 'SEQ_MAT';

-- Si vide, créer la séquence
CREATE SEQUENCE SEQ_MAT START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
```

### Problème 3 : Code MP déjà existant

**Cause** : Le code MP doit être unique

**Solution** : Changez le numéro dans le code (ex: `CUIR-2024-002` au lieu de `CUIR-2024-001`)

## 📊 Structure des Tables

### Table MATIERES_PREMIERES
```sql
CREATE TABLE MATIERES_PREMIERES (
    ID_STOCK_MP NUMBER PRIMARY KEY,
    CODE_MP VARCHAR2(50) NOT NULL UNIQUE,
    CATEGORIE_MP VARCHAR2(50) NOT NULL,
    NUM_LOT VARCHAR2(50) NOT NULL,
    ETAT_MP VARCHAR2(20) NOT NULL,
    COULEUR VARCHAR2(50) NOT NULL,
    QUANTITE NUMBER(10,2) NOT NULL CHECK (QUANTITE > 0),
    TYPE_STOCKAGE VARCHAR2(20) NOT NULL,
    QUALITE VARCHAR2(10) NOT NULL,
    DATE_CREATION DATE DEFAULT SYSDATE,
    DATE_MODIFICATION DATE DEFAULT SYSDATE
);
```

### Table STOCK_MP (Agrégée)
```sql
CREATE TABLE STOCK_MP (
    ID_STOCK NUMBER PRIMARY KEY,
    CATEGORIE_MP VARCHAR2(50) NOT NULL,
    QUANTITE NUMBER(10,2) NOT NULL CHECK (QUANTITE >= 0),
    DATE_MAJ DATE DEFAULT SYSDATE
);
```

## 🎯 Fonctionnalités CRUD

### ✅ CREATE (Ajouter)
1. Cliquez sur l'onglet **"Ajouter Matière"**
2. Remplissez tous les champs selon les formats requis
3. Cliquez sur **"✅ Valider l'ajout"**
4. Message de succès : "La matière première XXX a été enregistrée avec succès !"

### 📝 READ (Lire)
- L'onglet **"Liste du Stock"** affiche toutes les matières premières
- Utilisez la barre de recherche pour filtrer
- Cliquez sur **"Tri Alpha"** pour trier par code

### ✏️ UPDATE (Modifier)
1. Dans **"Liste du Stock"**, cliquez sur une ligne
2. Cliquez sur le bouton **"Modifier"**
3. L'onglet **"Modifier Matière"** s'ouvre avec les données pré-remplies
4. Modifiez les champs souhaités
5. Cliquez sur **"✅ Valider la modification"**

### ❌ DELETE (Supprimer)
1. Dans **"Liste du Stock"**, cliquez sur une ligne
2. Cliquez sur le bouton **"Supprimer"**
3. Confirmez la suppression
4. La matière est supprimée de la base de données

## 🎉 Résultat Final

Après avoir exécuté les scripts SQL, le CRUD des matières premières fonctionnera correctement :
- ✅ Ajout de nouvelles matières
- ✅ Affichage de la liste
- ✅ Modification des matières existantes
- ✅ Suppression de matières
- ✅ Recherche et tri
- ✅ Validation des formats
- ✅ Messages d'erreur clairs

## 📝 Notes Importantes

1. **Codes MP uniques** : Chaque code MP doit être unique. Incrémentez le numéro pour chaque nouvelle matière.
2. **Formats stricts** : Les formats sont stricts pour garantir la cohérence des données.
3. **Séquences automatiques** : L'ID est généré automatiquement, ne le saisissez pas manuellement.
4. **Synchronisation** : La table `STOCK_MP` doit être mise à jour manuellement ou via trigger pour refléter les changements dans `MATIERES_PREMIERES`.

## 🔗 Fichiers Créés

- `oracle_matieres_premieres.sql` - Script de création de la table principale
- `oracle_stock_mp.sql` - Script de création de la table agrégée
- `CORRECTION_CRUD_MATIERES.md` - Ce document d'instructions
