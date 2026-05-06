# ⚡ EXÉCUTION RAPIDE DE L'INSTALLATION

## 🎯 POUR COMMENCER IMMÉDIATEMENT

**Temps requis:** 10 minutes  
**Prérequis:** Oracle installé et démarré

---

## 📋 CHECKLIST AVANT DE COMMENCER

- [ ] Oracle est démarré (`net start OracleServiceXE`)
- [ ] Vous connaissez vos credentials de connexion
- [ ] SQL Developer ou SQL*Plus est installé
- [ ] Qt Creator est installé (pour compilation C++)

---

## 🚀 ÉTAPE 1: EXÉCUTER LES SCRIPTS SQL (5 min)

### Option A: Via SQL Developer (Recommandé)

1. **Ouvrir SQL Developer**
   - Démarrer SQL Developer
   - Créer/Ouvrir une connexion vers `PROJET_CPP`

2. **Exécuter le premier script**
   - Fichier → Ouvrir → `sql/integration_tables_arduino_gaz.sql`
   - Cliquer sur l'icône "Exécuter le script" (F5)
   - Attendre la fin de l'exécution (~2 min)
   - Vérifier les messages: "✓ INSTALLATION TERMINÉE AVEC SUCCÈS!"

3. **Exécuter le deuxième script**
   - Fichier → Ouvrir → `sql/integration_complete_matieres_premieres.sql`
   - Cliquer sur "Exécuter le script" (F5)
   - Attendre la fin (~2 min)
   - Vérifier: "✓ INSTALLATION TERMINÉE AVEC SUCCÈS!"

4. **Vérifier l'installation**
   - Fichier → Ouvrir → `sql/verification_installation.sql`
   - Exécuter le script (F5)
   - **Résultat attendu:** Score 4/4 ✓

### Option B: Via SQL*Plus (Ligne de commande)

```bash
# Ouvrir un terminal/invite de commandes
cd chemin/vers/votre/projet

# Se connecter à Oracle
sqlplus PROJET_CPP/votre_mot_de_passe@localhost:1521/XE

# Exécuter les scripts
@sql/integration_tables_arduino_gaz.sql
@sql/integration_complete_matieres_premieres.sql
@sql/verification_installation.sql

# Quitter
exit
```

---

## 🔨 ÉTAPE 2: COMPILER L'APPLICATION (3 min)

### Option A: Via Qt Creator (Recommandé)

1. **Ouvrir le projet**
   - Lancer Qt Creator
   - Fichier → Ouvrir un fichier ou projet
   - Sélectionner `CMakeLists.txt`

2. **Configurer le projet**
   - Sélectionner le kit MinGW 64-bit
   - Cliquer sur "Configure Project"

3. **Compiler**
   - Build → Rebuild All (Ctrl+Shift+B)
   - Attendre la fin de la compilation
   - Vérifier: "Build successful"

4. **Lancer**
   - Cliquer sur le bouton "Play" (Ctrl+R)
   - L'application devrait démarrer

### Option B: Via ligne de commande

```bash
# Créer le dossier build s'il n'existe pas
mkdir build
cd build

# Configurer avec CMake
cmake ..

# Compiler
cmake --build .

# Lancer (Windows)
./ProjetCpp.exe

# Lancer (Linux/Mac)
./ProjetCpp
```

---

## ✅ ÉTAPE 3: TESTER L'APPLICATION (2 min)

### Test 1: Interface Matières Premières

1. **Naviguer vers le module**
   - Dans l'application, cliquer sur l'onglet "Stock"
   - Cliquer sur "Matières Premières"

2. **Vérifier l'affichage**
   - Vous devriez voir 5 matières premières de test
   - Colonnes: ID, Code, Catégorie, Lot, État, Couleur, Quantité, Stockage, Qualité

3. **Tester l'ajout**
   - Cliquer sur "Ajouter"
   - Remplir le formulaire:
     ```
     Code:      CUIR-2026-TEST
     Catégorie: Cuir
     Lot:       LOT-2026-T
     État:      BRUT
     Couleur:   Rouge
     Quantité:  100.00
     Stockage:  Sec
     Qualité:   A
     ```
   - Cliquer sur "Valider"
   - **Résultat attendu:** Message de succès + nouvelle ligne dans le tableau

4. **Tester la recherche**
   - Dans le champ de recherche, taper "TEST"
   - **Résultat attendu:** Seule la matière TEST s'affiche

5. **Tester la modification**
   - Sélectionner la ligne TEST
   - Cliquer sur "Modifier"
   - Changer la quantité à 150.00
   - Valider
   - **Résultat attendu:** Quantité mise à jour

6. **Tester la suppression**
   - Sélectionner la ligne TEST
   - Cliquer sur "Supprimer"
   - Confirmer
   - **Résultat attendu:** Ligne supprimée

### Test 2: Module Ravitaillement (Si disponible dans l'interface)

1. **Naviguer vers Ravitaillement**
   - Onglet "Stock" → "Ravitaillement"

2. **Vérifier les fournisseurs**
   - Vous devriez voir 9 fournisseurs
   - Triés par type de cuir et prix

3. **Calculer un besoin**
   - Sélectionner une matière
   - Entrer les paramètres:
     ```
     Stock actuel:      150 M²
     Seuil sécurité:    100 M²
     Consommation:      200 M²
     ```
   - Cliquer sur "Calculer"
   - **Résultat attendu:** Besoin calculé + fournisseur recommandé

---

## 🎯 VÉRIFICATION FINALE

### Checklist de succès

- [ ] Scripts SQL exécutés sans erreur
- [ ] Score de vérification: 4/4 ✓
- [ ] Application compilée sans erreur
- [ ] Application démarre correctement
- [ ] Module Matières Premières accessible
- [ ] 5 matières de test affichées
- [ ] Ajout d'une matière fonctionne
- [ ] Recherche fonctionne
- [ ] Modification fonctionne
- [ ] Suppression fonctionne

### Si tous les points sont cochés: ✅ **INSTALLATION RÉUSSIE!**

---

## ⚠️ EN CAS DE PROBLÈME

### Problème 1: Erreur SQL "Table already exists"

**Solution:**
```sql
-- Supprimer les tables existantes
DROP TABLE MATIERES_PREMIERES CASCADE CONSTRAINTS;
DROP TABLE FOURNISSEURS CASCADE CONSTRAINTS;
DROP TABLE STOCK_MP CASCADE CONSTRAINTS;
DROP TABLE RAVITAILLEMENT_LOG CASCADE CONSTRAINTS;
DROP TABLE ARDUINO_CONFIG CASCADE CONSTRAINTS;
DROP TABLE MOTEUR_LOGS CASCADE CONSTRAINTS;
DROP TABLE GAZ_ALERTS CASCADE CONSTRAINTS;

-- Réexécuter les scripts
@sql/integration_tables_arduino_gaz.sql
@sql/integration_complete_matieres_premieres.sql
```

### Problème 2: Erreur de compilation "matierepremiere.h not found"

**Solution:**
```bash
# Vérifier que les fichiers existent
ls matierepremiere.h
ls matierepremiere.cpp

# Si absents, les copier depuis le dossier d'intégration
cp "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)/matierepremiere.h" .
cp "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)/matierepremiere.cpp" .

# Recompiler
cd build
cmake ..
cmake --build .
```

### Problème 3: Application ne démarre pas

**Solution:**
1. Vérifier que Oracle est démarré:
   ```bash
   net start OracleServiceXE
   ```

2. Vérifier la connexion dans `connexion.cpp`:
   ```cpp
   db.setDatabaseName("localhost:1521/XE");
   db.setUserName("PROJET_CPP");
   db.setPassword("votre_mot_de_passe");
   ```

3. Tester la connexion avec SQL Developer

### Problème 4: Score de vérification < 4/4

**Solution:**
```sql
-- Identifier ce qui manque
SELECT 'Tables' AS TYPE, COUNT(*) AS NOMBRE FROM user_tables 
WHERE table_name IN ('ARDUINO_CONFIG', 'MOTEUR_LOGS', 'GAZ_ALERTS', 'MATIERES_PREMIERES', 'FOURNISSEURS', 'STOCK_MP', 'RAVITAILLEMENT_LOG');

SELECT 'Séquences' AS TYPE, COUNT(*) AS NOMBRE FROM user_sequences 
WHERE sequence_name IN ('SEQ_MAT', 'SEQ_FOURNISSEUR', 'SEQ_STOCK_MP', 'SEQ_MOTEUR_LOGS', 'SEQ_GAZ_ALERTS');

SELECT 'Vues' AS TYPE, COUNT(*) AS NOMBRE FROM user_views 
WHERE view_name IN ('V_STOCK_AGREGE', 'V_FOURNISSEURS_ACTIFS', 'V_ANALYSE_RAVITAILLEMENT', 'V_ALERTES_GAZ_ACTIVES', 'V_MONITORING_ARDUINO');

SELECT 'Procédures' AS TYPE, COUNT(*) AS NOMBRE FROM user_objects 
WHERE object_type = 'PROCEDURE' 
AND object_name IN ('SP_AJOUTER_MATIERE', 'SP_LOG_RAVITAILLEMENT', 'SP_CALCULER_BESOIN', 'SP_LOG_MOTEUR', 'SP_CREER_ALERTE_GAZ', 'SP_TRAITER_ALERTE_GAZ');

-- Réexécuter le script correspondant
```

---

## 📞 BESOIN D'AIDE?

### Documentation disponible

1. **Ce fichier** - Guide d'exécution rapide
2. **README_INTEGRATION_MATIERES_PREMIERES.md** - Résumé complet
3. **GUIDE_INTEGRATION_COMPLETE.md** - Guide détaillé avec exemples

### Commandes utiles

```sql
-- Voir toutes les tables
SELECT table_name FROM user_tables ORDER BY table_name;

-- Voir les données d'une table
SELECT * FROM MATIERES_PREMIERES;
SELECT * FROM FOURNISSEURS;
SELECT * FROM GAZ_ALERTS;

-- Voir les procédures
SELECT object_name FROM user_objects WHERE object_type = 'PROCEDURE';

-- Tester une procédure
BEGIN
    SP_AJOUTER_MATIERE('TEST-2026-001', 'Cuir', 'LOT-2026-T', 'BRUT', 'Noir', 100, 'Sec', 'A');
END;
/
```

---

## 🎉 FÉLICITATIONS!

Si vous avez suivi toutes les étapes et que tous les tests passent, vous avez maintenant:

✅ **7 tables** de base de données opérationnelles  
✅ **5 séquences** pour auto-incrémentation  
✅ **5 vues** pour analyses  
✅ **6 procédures stockées** pour automatisation  
✅ **Application C++** compilée et fonctionnelle  
✅ **Module CRUD** complet pour matières premières  
✅ **Système de ravitaillement** intelligent  
✅ **Monitoring Arduino** et alertes gaz  

**Vous êtes prêt pour la production! 🚀**

---

## 📊 PROCHAINES ÉTAPES

1. **Personnaliser les données**
   - Ajouter vos propres matières premières
   - Configurer vos fournisseurs réels
   - Ajuster les paramètres Arduino

2. **Tester en conditions réelles**
   - Utiliser avec de vraies données
   - Tester le ravitaillement
   - Vérifier les alertes

3. **Former les utilisateurs**
   - Montrer l'interface
   - Expliquer les fonctionnalités
   - Distribuer la documentation

4. **Monitorer et optimiser**
   - Suivre les performances
   - Ajuster les paramètres
   - Collecter les retours

---

**Bon travail et bonne utilisation! 💪**
