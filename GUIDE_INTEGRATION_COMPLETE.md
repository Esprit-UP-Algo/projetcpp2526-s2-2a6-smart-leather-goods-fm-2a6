# 📋 GUIDE D'INTÉGRATION COMPLÈTE - MATIÈRES PREMIÈRES
## Projet FIL D'OR - Smart Leather Goods

**Date:** 05 Mai 2026  
**Version:** 1.0  
**Statut:** ✅ Prêt pour déploiement

---

## 🎯 OBJECTIF

Intégrer complètement le module de gestion des matières premières avec:
- ✅ Tables de base de données (MATIERES_PREMIERES, FOURNISSEURS, STOCK_MP)
- ✅ Tables Arduino et Gaz (ARDUINO_CONFIG, MOTEUR_LOGS, GAZ_ALERTS)
- ✅ Système de ravitaillement intelligent
- ✅ Calcul automatique des besoins
- ✅ Classes C++ complètes

---

## 📦 FICHIERS CRÉÉS

### 1. Scripts SQL

#### `sql/integration_tables_arduino_gaz.sql`
**Contenu:**
- Table `ARDUINO_CONFIG` - Configuration des ports COM et paramètres Arduino
- Table `MOTEUR_LOGS` - Logs des opérations moteur avec capteurs
- Table `GAZ_ALERTS` - Alertes de détection de gaz
- Vues: `V_ALERTES_GAZ_ACTIVES`, `V_MONITORING_ARDUINO`
- Procédures: `SP_LOG_MOTEUR`, `SP_CREER_ALERTE_GAZ`, `SP_TRAITER_ALERTE_GAZ`
- Triggers automatiques pour auto-incrémentation

**Utilisation:**
```sql
-- Exécuter dans SQL Developer ou SQL*Plus
@sql/integration_tables_arduino_gaz.sql
```

#### `sql/integration_complete_matieres_premieres.sql`
**Contenu:**
- Table `MATIERES_PREMIERES` - Gestion des matières premières
- Table `FOURNISSEURS` - 9 fournisseurs pré-configurés (Vachette, Agneau, Veau)
- Table `STOCK_MP` - Vue agrégée du stock
- Table `RAVITAILLEMENT_LOG` - Historique des décisions de ravitaillement
- Vues: `V_STOCK_AGREGE`, `V_FOURNISSEURS_ACTIFS`, `V_ANALYSE_RAVITAILLEMENT`
- Procédures: `SP_AJOUTER_MATIERE`, `SP_LOG_RAVITAILLEMENT`, `SP_CALCULER_BESOIN`

**Utilisation:**
```sql
-- Exécuter dans SQL Developer ou SQL*Plus
@sql/integration_complete_matieres_premieres.sql
```

### 2. Classes C++ (Déjà présentes)

#### `matierepremiere.h` / `matierepremiere.cpp`
**Fonctionnalités:**
- ✅ CRUD complet (Ajouter, Modifier, Supprimer, Afficher)
- ✅ Validation des données (formats, contraintes)
- ✅ Recherche multi-critères
- ✅ Tri alphabétique
- ✅ Gestion des erreurs

---

## 🚀 PROCÉDURE D'INSTALLATION

### ÉTAPE 1: Exécuter les scripts SQL

#### Option A: Via SQL Developer
1. Ouvrir SQL Developer
2. Se connecter à la base `PROJET_CPP`
3. Ouvrir le fichier `sql/integration_tables_arduino_gaz.sql`
4. Cliquer sur "Exécuter le script" (F5)
5. Vérifier les messages de succès
6. Répéter pour `sql/integration_complete_matieres_premieres.sql`

#### Option B: Via SQL*Plus
```bash
sqlplus PROJET_CPP/votre_mot_de_passe@localhost:1521/XE
@sql/integration_tables_arduino_gaz.sql
@sql/integration_complete_matieres_premieres.sql
exit
```

### ÉTAPE 2: Vérifier l'installation

```sql
-- Vérifier les tables créées
SELECT table_name FROM user_tables 
WHERE table_name IN (
    'ARDUINO_CONFIG', 'MOTEUR_LOGS', 'GAZ_ALERTS',
    'MATIERES_PREMIERES', 'FOURNISSEURS', 'STOCK_MP', 'RAVITAILLEMENT_LOG'
)
ORDER BY table_name;

-- Vérifier les données
SELECT COUNT(*) AS NB_MATIERES FROM MATIERES_PREMIERES;
SELECT COUNT(*) AS NB_FOURNISSEURS FROM FOURNISSEURS;
SELECT COUNT(*) AS NB_ALERTES FROM GAZ_ALERTS;

-- Tester une vue
SELECT * FROM V_STOCK_AGREGE;
SELECT * FROM V_FOURNISSEURS_ACTIFS;
```

### ÉTAPE 3: Compiler l'application C++

#### Via Qt Creator
1. Ouvrir Qt Creator
2. Ouvrir le projet `CMakeLists.txt`
3. Vérifier que `matierepremiere.cpp` est dans la liste des sources
4. Build → Rebuild All
5. Vérifier qu'il n'y a pas d'erreurs

#### Via ligne de commande
```bash
cd build
cmake ..
cmake --build .
```

### ÉTAPE 4: Tester l'application

1. Lancer l'application
2. Aller dans l'onglet "Stock" → "Matières Premières"
3. Tester l'ajout d'une matière:
   - Code: `CUIR-2026-001`
   - Catégorie: `Cuir`
   - Lot: `LOT-2026-A`
   - État: `BRUT`
   - Couleur: `Marron`
   - Quantité: `100.00`
   - Stockage: `Sec`
   - Qualité: `A`
4. Vérifier l'affichage dans le tableau
5. Tester la recherche
6. Tester la modification
7. Tester la suppression

---

## 📊 STRUCTURE DES TABLES

### MATIERES_PREMIERES
| Colonne | Type | Description |
|---------|------|-------------|
| ID_STOCK_MP | NUMBER | Clé primaire (auto-incrémentée) |
| CODE_MP | VARCHAR2(50) | Code unique (ex: CUIR-2024-001) |
| CATEGORIE_MP | VARCHAR2(50) | Cuir, Fil, etc. |
| NUM_LOT | VARCHAR2(50) | Numéro de lot (ex: LOT-2024-A) |
| ETAT_MP | VARCHAR2(20) | BRUT, TEINT, TRAITE |
| COULEUR | VARCHAR2(50) | Couleur de la matière |
| QUANTITE | NUMBER(10,2) | Quantité en M² |
| TYPE_STOCKAGE | VARCHAR2(20) | Sec, Froid, Humide |
| QUALITE | VARCHAR2(10) | A, B, C |
| DATE_CREATION | DATE | Date de création |
| DATE_MODIFICATION | DATE | Date de modification |

### FOURNISSEURS
| Colonne | Type | Description |
|---------|------|-------------|
| ID_FOURNISSEUR | NUMBER | Clé primaire |
| NOM_FOURNISSEUR | VARCHAR2(100) | Nom du fournisseur |
| TYPE_CUIR | VARCHAR2(50) | Cuir Vachette, Agneau, Veau |
| PRIX_PAR_M2 | NUMBER(10,2) | Prix unitaire |
| QUALITE | VARCHAR2(50) | A (Premium), B (Standard), C (Économique) |
| DELAI_JOURS | NUMBER | Délai de livraison |
| STOCK_DISPO | NUMBER(10,2) | Stock disponible |
| EMAIL | VARCHAR2(100) | Email de contact |
| TELEPHONE | VARCHAR2(20) | Téléphone |
| ADRESSE | VARCHAR2(200) | Adresse complète |
| ACTIF | CHAR(1) | 0=Inactif, 1=Actif |

### ARDUINO_CONFIG
| Colonne | Type | Description |
|---------|------|-------------|
| CLE | VARCHAR2(50) | Clé de configuration (PK) |
| VALEUR | VARCHAR2(255) | Valeur de la configuration |
| DESCRIPTION | VARCHAR2(500) | Description |
| DATE_CREATION | DATE | Date de création |
| DATE_MODIFICATION | DATE | Date de modification |

### MOTEUR_LOGS
| Colonne | Type | Description |
|---------|------|-------------|
| LOG_ID | NUMBER | Clé primaire |
| PRODUCT_ID | NUMBER | Référence au produit (FK) |
| TIMESTAMP | TIMESTAMP | Date et heure du log |
| ACTION | VARCHAR2(100) | Type d'action |
| STATUT | VARCHAR2(50) | EN_COURS, TERMINE, ERREUR, ANNULE |
| TEMPERATURE | NUMBER(5,2) | Température mesurée |
| HUMIDITE | NUMBER(5,2) | Humidité mesurée |
| VALEUR_GAZ | NUMBER(10,2) | Valeur du gaz |
| MESSAGE | VARCHAR2(500) | Message descriptif |
| UTILISATEUR | VARCHAR2(100) | Utilisateur |

### GAZ_ALERTS
| Colonne | Type | Description |
|---------|------|-------------|
| ID | NUMBER | Clé primaire |
| EMPLACEMENT_ID | NUMBER | Référence à l'emplacement |
| VALEUR_GAZ | NUMBER(10,2) | Valeur mesurée |
| MESSAGE | VARCHAR2(255) | Message d'alerte |
| DATE_ALERT | DATE | Date de l'alerte |
| NIVEAU_ALERTE | VARCHAR2(20) | BAS, MOYEN, ELEVE, CRITIQUE |
| STATUT_TRAITEMENT | VARCHAR2(50) | NON_TRAITE, EN_COURS, TRAITE, IGNORE |
| TRAITE_PAR | VARCHAR2(100) | Personne ayant traité |
| DATE_TRAITEMENT | DATE | Date de traitement |
| COMMENTAIRE | VARCHAR2(500) | Commentaire |

### RAVITAILLEMENT_LOG
| Colonne | Type | Description |
|---------|------|-------------|
| LOG_ID | VARCHAR2(40) | Clé primaire (format: RAV-YYYYMMDD-HHMMSS) |
| DATE_LOG | TIMESTAMP | Date du log |
| MATIERE | VARCHAR2(100) | Matière concernée |
| FOURNISSEUR | VARCHAR2(100) | Fournisseur sélectionné |
| SCORE_GLOBAL | NUMBER(5,2) | Score de décision |
| QUANTITE_COMMANDE | NUMBER(10,2) | Quantité commandée |
| BUDGET_ESTIME | NUMBER(12,2) | Budget estimé |
| RISQUE | NUMBER(5,2) | Niveau de risque |
| COUVERTURE_JOURS | NUMBER(10,2) | Couverture en jours |
| STRATEGIE | VARCHAR2(50) | Stratégie appliquée |
| UTILISATEUR | VARCHAR2(100) | Utilisateur |
| STATUT | VARCHAR2(50) | PLANIFIE, COMMANDE, RECU, ANNULE |

---

## 🔧 PROCÉDURES STOCKÉES DISPONIBLES

### 1. SP_AJOUTER_MATIERE
Ajoute une nouvelle matière première.

```sql
BEGIN
    SP_AJOUTER_MATIERE(
        p_code_mp => 'CUIR-2026-010',
        p_categorie => 'Cuir',
        p_num_lot => 'LOT-2026-Z',
        p_etat => 'BRUT',
        p_couleur => 'Noir',
        p_quantite => 250.00,
        p_type_stockage => 'Sec',
        p_qualite => 'A'
    );
END;
/
```

### 2. SP_LOG_RAVITAILLEMENT
Enregistre une décision de ravitaillement.

```sql
BEGIN
    SP_LOG_RAVITAILLEMENT(
        p_matiere => 'Cuir Vachette',
        p_fournisseur => 'TanLeather SA',
        p_score => 85.50,
        p_quantite => 100.00,
        p_budget => 4500.00,
        p_risque => 15.00,
        p_couverture => 7.50,
        p_strategie => 'Equilibre'
    );
END;
/
```

### 3. SP_CALCULER_BESOIN
Calcule le besoin de ravitaillement.

```sql
DECLARE
    v_besoin NUMBER;
    v_couverture NUMBER;
BEGIN
    SP_CALCULER_BESOIN(
        p_categorie => 'Cuir Vachette',
        p_stock_actuel => 150.00,
        p_seuil_securite => 100.00,
        p_conso_prevue => 200.00,
        p_besoin => v_besoin,
        p_couverture => v_couverture
    );
    
    DBMS_OUTPUT.PUT_LINE('Besoin: ' || v_besoin || ' M²');
    DBMS_OUTPUT.PUT_LINE('Couverture: ' || v_couverture || ' jours');
END;
/
```

### 4. SP_LOG_MOTEUR
Enregistre un log moteur Arduino.

```sql
BEGIN
    SP_LOG_MOTEUR(
        p_product_id => 1,
        p_action => 'CONTROLE_QUALITE',
        p_statut => 'TERMINE',
        p_temperature => 22.5,
        p_humidite => 45.0,
        p_valeur_gaz => 150.0,
        p_message => 'Contrôle qualité OK',
        p_utilisateur => 'SYSTEM'
    );
END;
/
```

### 5. SP_CREER_ALERTE_GAZ
Crée une alerte de détection de gaz.

```sql
BEGIN
    SP_CREER_ALERTE_GAZ(
        p_emplacement_id => 1,
        p_valeur_gaz => 450.75,
        p_message => 'Alerte gaz élevée - Vérification requise',
        p_niveau_alerte => 'ELEVE'
    );
END;
/
```

### 6. SP_TRAITER_ALERTE_GAZ
Marque une alerte comme traitée.

```sql
BEGIN
    SP_TRAITER_ALERTE_GAZ(
        p_alert_id => 1,
        p_traite_par => 'Jean Dupont',
        p_commentaire => 'Ventilation augmentée, situation normalisée'
    );
END;
/
```

---

## 📈 VUES DISPONIBLES

### V_STOCK_AGREGE
Vue agrégée du stock par catégorie.

```sql
SELECT * FROM V_STOCK_AGREGE;
```

**Colonnes:**
- CATEGORIE_MP
- QUANTITE_TOTALE
- NB_LOTS
- DERNIERE_MAJ
- QUANTITE_MOYENNE

### V_FOURNISSEURS_ACTIFS
Liste des fournisseurs actifs.

```sql
SELECT * FROM V_FOURNISSEURS_ACTIFS
WHERE TYPE_CUIR = 'Cuir Vachette'
ORDER BY PRIX_PAR_M2;
```

### V_ANALYSE_RAVITAILLEMENT
Analyse mensuelle des ravitaillements.

```sql
SELECT * FROM V_ANALYSE_RAVITAILLEMENT
WHERE MOIS >= TO_CHAR(ADD_MONTHS(SYSDATE, -3), 'YYYY-MM');
```

### V_ALERTES_GAZ_ACTIVES
Alertes gaz non traitées.

```sql
SELECT * FROM V_ALERTES_GAZ_ACTIVES
WHERE NIVEAU_ALERTE IN ('ELEVE', 'CRITIQUE');
```

### V_MONITORING_ARDUINO
Monitoring des logs Arduino.

```sql
SELECT * FROM V_MONITORING_ARDUINO
WHERE STATUT = 'ERREUR'
ORDER BY DATE_LOG DESC
FETCH FIRST 20 ROWS ONLY;
```

---

## 🧪 TESTS À EFFECTUER

### Test 1: CRUD Matières Premières
```sql
-- Ajouter
INSERT INTO MATIERES_PREMIERES (ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE)
VALUES (SEQ_MAT.NEXTVAL, 'TEST-2026-999', 'Cuir', 'LOT-2026-T', 'BRUT', 'Rouge', 50.00, 'Sec', 'B');

-- Lire
SELECT * FROM MATIERES_PREMIERES WHERE CODE_MP = 'TEST-2026-999';

-- Modifier
UPDATE MATIERES_PREMIERES SET QUANTITE = 75.00 WHERE CODE_MP = 'TEST-2026-999';

-- Supprimer
DELETE FROM MATIERES_PREMIERES WHERE CODE_MP = 'TEST-2026-999';

COMMIT;
```

### Test 2: Ravitaillement
```sql
-- Calculer besoin
DECLARE
    v_besoin NUMBER;
    v_couverture NUMBER;
BEGIN
    SP_CALCULER_BESOIN('Cuir Vachette', 150, 100, 200, v_besoin, v_couverture);
END;
/

-- Enregistrer décision
BEGIN
    SP_LOG_RAVITAILLEMENT('Cuir Vachette', 'TanLeather SA', 85.5, 100, 4500, 15, 7.5, 'Equilibre');
END;
/

-- Vérifier historique
SELECT * FROM RAVITAILLEMENT_LOG ORDER BY DATE_LOG DESC;
```

### Test 3: Arduino et Gaz
```sql
-- Vérifier config Arduino
SELECT * FROM ARDUINO_CONFIG;

-- Créer alerte gaz
BEGIN
    SP_CREER_ALERTE_GAZ(1, 500.00, 'Test alerte', 'ELEVE');
END;
/

-- Vérifier alertes actives
SELECT * FROM V_ALERTES_GAZ_ACTIVES;

-- Traiter alerte
BEGIN
    SP_TRAITER_ALERTE_GAZ(1, 'Admin', 'Test traitement');
END;
/
```

---

## ⚠️ DÉPANNAGE

### Problème: Table déjà existante
**Solution:**
```sql
DROP TABLE nom_table CASCADE CONSTRAINTS;
```

### Problème: Séquence déjà existante
**Solution:**
```sql
DROP SEQUENCE nom_sequence;
```

### Problème: Erreur de compilation C++
**Solution:**
1. Vérifier que `matierepremiere.h` et `matierepremiere.cpp` sont dans le projet
2. Vérifier les includes dans `CMakeLists.txt`
3. Clean et rebuild

### Problème: Connexion base de données
**Solution:**
1. Vérifier que Oracle est démarré
2. Vérifier les credentials dans `connexion.cpp`
3. Tester la connexion avec SQL Developer

---

## 📞 SUPPORT

Pour toute question ou problème:
1. Consulter ce guide
2. Vérifier les logs SQL (DBMS_OUTPUT)
3. Vérifier les logs Qt (qDebug)
4. Consulter la documentation Oracle

---

## ✅ CHECKLIST FINALE

- [ ] Scripts SQL exécutés sans erreur
- [ ] Tables créées et vérifiées
- [ ] Données de test insérées
- [ ] Vues fonctionnelles
- [ ] Procédures stockées testées
- [ ] Application C++ compilée
- [ ] Tests CRUD réussis
- [ ] Module ravitaillement fonctionnel
- [ ] Alertes gaz opérationnelles
- [ ] Logs Arduino enregistrés

---

**🎉 FÉLICITATIONS! L'intégration est complète et fonctionnelle!**
