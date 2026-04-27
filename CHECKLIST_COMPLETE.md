# ✅ Checklist Complète - Module Arduino Smart

## 📋 Vue d'ensemble

Cette checklist vous guide pas à pas pour l'installation, la configuration et le test du Module Arduino Smart.

---

## 🔧 Phase 1 : Préparation du Matériel

### Arduino et Capteur

- [ ] **Arduino Uno** disponible et fonctionnel
- [ ] **Capteur DHT11** disponible
- [ ] **3 câbles Dupont** (mâle-femelle)
- [ ] **Câble USB A-B** pour connexion PC
- [ ] **Résistance 10kΩ** (optionnelle, pour pull-up)

### Câblage

- [ ] DHT11 VCC connecté à Arduino 5V
- [ ] DHT11 GND connecté à Arduino GND
- [ ] DHT11 DATA connecté à Arduino Pin Digital 3
- [ ] Résistance pull-up installée (si nécessaire)
- [ ] Vérification visuelle du câblage

### Test du Capteur

- [ ] Arduino branché sur le PC via USB
- [ ] LED d'alimentation Arduino allumée
- [ ] Capteur DHT11 alimenté (vérifier avec multimètre si doute)

---

## 💻 Phase 2 : Installation Logicielle Arduino

### IDE Arduino

- [ ] **Arduino IDE** installé (version 1.8.x ou 2.x)
- [ ] Port série détecté (Outils > Port)
- [ ] Type de carte sélectionné : **Arduino Uno**

### Bibliothèques

- [ ] Ouvrir : **Croquis** > **Inclure une bibliothèque** > **Gérer les bibliothèques**
- [ ] Rechercher : **"DHT sensor library"**
- [ ] Installer : **DHT sensor library** par **Adafruit**
- [ ] Installer : **Adafruit Unified Sensor** (dépendance)

### Code Arduino

- [ ] Ouvrir le fichier **`arduino_dht11_code.ino`**
- [ ] Vérifier que `DHTPIN` est bien défini à **3**
- [ ] Vérifier que `DHTTYPE` est bien **DHT11**
- [ ] Compiler le code (Ctrl+R) - Aucune erreur
- [ ] Téléverser le code (Ctrl+U) - "Téléversement terminé"

### Test du Moniteur Série

- [ ] Ouvrir : **Outils** > **Moniteur série**
- [ ] Régler la vitesse à **9600 bauds**
- [ ] Vérifier l'affichage du message de démarrage
- [ ] Vérifier l'affichage des données au format : `H:xx.xx;T:yy.yy`
- [ ] Vérifier que les valeurs sont réalistes (H: 20-90%, T: 15-35°C)
- [ ] Vérifier la fréquence d'envoi (toutes les 2 secondes)

**Exemple de sortie attendue :**
```
========================================
FIL D'OR - Module Arduino Smart
Capteur DHT11 - Surveillance Stockage
========================================
Initialisation...
Capteur pret !
Format: H:xx.xx;T:yy.yy
========================================
H:45.50;T:22.30
H:45.60;T:22.40
H:45.70;T:22.50
```

---

## 🏗️ Phase 3 : Préparation du Projet Qt

### Fichiers Créés

- [ ] **`arduinomanager.h`** présent dans le dossier du projet
- [ ] **`arduinomanager.cpp`** présent dans le dossier du projet
- [ ] **`arduinowidget.h`** présent dans le dossier du projet
- [ ] **`arduinowidget.cpp`** présent dans le dossier du projet
- [ ] **`arduinowidget.ui`** présent dans le dossier du projet

### Fichiers Modifiés

- [ ] **`CMakeLists.txt`** modifié :
  - [ ] `find_package` inclut **SerialPort**
  - [ ] `target_link_libraries` inclut **SerialPort**
  - [ ] `set(PROJECT_SOURCES ...)` inclut les 5 fichiers Arduino

- [ ] **`mainwindow.h`** modifié :
  - [ ] `#include "arduinowidget.h"` ajouté
  - [ ] `void ouvrirModuleArduino();` déclaré

- [ ] **`mainwindow.cpp`** modifié :
  - [ ] Fonction `ouvrirModuleArduino()` implémentée

### Vérification des Dépendances

- [ ] **Qt SerialPort** installé (vérifier dans Qt Maintenance Tool)
- [ ] **QtCharts** installé (déjà présent dans le projet)
- [ ] **Oracle Database** accessible

---

## 🔨 Phase 4 : Compilation du Projet

### Configuration CMake

- [ ] Ouvrir **Qt Creator**
- [ ] Ouvrir le fichier **`CMakeLists.txt`**
- [ ] Configurer le projet (sélectionner le kit)
- [ ] Attendre la fin de la configuration CMake

### Compilation

- [ ] Compiler le projet (Ctrl+B ou Build > Build Project)
- [ ] Vérifier qu'il n'y a **aucune erreur de compilation**
- [ ] Vérifier qu'il n'y a **aucun warning critique**

**Erreurs courantes et solutions :**

| Erreur | Solution |
|--------|----------|
| `Qt5SerialPort not found` | Installer Qt SerialPort via Qt Maintenance Tool |
| `arduinowidget.h: No such file` | Vérifier que les fichiers sont dans le bon dossier |
| `undefined reference to ArduinoWidget` | Vérifier que les fichiers sont dans `CMakeLists.txt` |

---

## 🎨 Phase 5 : Intégration du Bouton

### Choix de l'Option

Choisir **UNE** des options suivantes :

- [ ] **Option A** : Bouton dans la barre de navigation latérale
- [ ] **Option B** : Carte dans le dashboard d'accueil
- [ ] **Option C** : Bouton flottant en haut à droite
- [ ] **Option D** : Menu déroulant

### Implémentation

- [ ] Code du bouton ajouté dans **`mainwindow.cpp`**
- [ ] Signal `clicked()` connecté au slot `ouvrirModuleArduino()`
- [ ] Bouton ajouté au layout approprié
- [ ] Style CSS appliqué au bouton

### Recompilation

- [ ] Recompiler le projet après ajout du bouton
- [ ] Vérifier qu'il n'y a aucune erreur

---

## 🚀 Phase 6 : Test du Module

### Lancement de l'Application

- [ ] Lancer l'application (Ctrl+R ou Run)
- [ ] Application démarre sans crash
- [ ] Interface principale s'affiche correctement

### Vérification du Bouton

- [ ] Bouton **"🔌 Arduino Smart"** visible dans l'interface
- [ ] Bouton cliquable (curseur change au survol)
- [ ] Style du bouton correct (couleur, taille, police)

### Ouverture du Module

- [ ] Cliquer sur le bouton **"Arduino Smart"**
- [ ] Fenêtre **ArduinoWidget** s'ouvre
- [ ] Titre de la fenêtre : **"FIL D'OR - Module Arduino Smart"**
- [ ] Taille de la fenêtre : minimum 900x700 pixels

### Interface du Module

- [ ] **Titre principal** affiché : "🔌 MODULE ARDUINO SMART"
- [ ] **Section Connexion** visible :
  - [ ] ComboBox pour sélection du port
  - [ ] Bouton "🔄 Rafraîchir"
  - [ ] Bouton "Connecter"
  - [ ] Label "Statut : ⚫ Déconnecté"

- [ ] **Section Données** visible :
  - [ ] Label "💧 HUMIDITÉ"
  - [ ] Label "🌡️ TEMPÉRATURE"
  - [ ] Valeurs initiales : "-- %" et "-- °C"
  - [ ] Barre de progression (humidité)

- [ ] **Label Alerte** visible : "En attente de connexion..."

- [ ] **Section Graphique** visible :
  - [ ] Titre : "Graphique Temps Réel (50 dernières mesures)"
  - [ ] Graphique vide (axes visibles)

- [ ] **Bouton Export** visible : "📊 Exporter Historique CSV"

### Test de Connexion

- [ ] **Fermer l'IDE Arduino** (libérer le port série)
- [ ] Cliquer sur **"🔄 Rafraîchir"**
- [ ] Vérifier que le port apparaît dans la liste (ex: COM5)
- [ ] Sélectionner le port dans la ComboBox
- [ ] Cliquer sur **"Connecter"**

**Résultat attendu :**
- [ ] Statut change à : **"🟢 Connecté"** (vert)
- [ ] Bouton change à : **"Déconnecter"**
- [ ] ComboBox et bouton Rafraîchir désactivés

### Test de Réception des Données

Après connexion, vérifier que :

- [ ] **Humidité** s'affiche (ex: "45.5 %")
- [ ] **Température** s'affiche (ex: "22.3 °C")
- [ ] **Barre de progression** se remplit selon l'humidité
- [ ] **Couleur de la barre** change selon le niveau :
  - [ ] Orange si < 30%
  - [ ] Vert si 30-70%
  - [ ] Rouge si > 70%

- [ ] **Label Alerte** affiche un message :
  - [ ] "⚠️ Air trop sec" si H < 30%
  - [ ] "✅ Conditions optimales" si H 30-70%
  - [ ] "❌ Humidité élevée" si H > 70%
  - [ ] "🔥 Température critique" si T > 35°C

- [ ] **Graphique** se met à jour :
  - [ ] Courbe bleue (Humidité) apparaît
  - [ ] Courbe rouge (Température) apparaît
  - [ ] Axes X (temps) et Y (valeurs) s'ajustent
  - [ ] Légende visible en bas

### Test de Rafraîchissement

- [ ] Attendre 10 secondes
- [ ] Vérifier que les valeurs se mettent à jour (toutes les 2 secondes)
- [ ] Vérifier que le graphique s'étend progressivement
- [ ] Vérifier que les alertes changent selon les valeurs

### Test de Déconnexion

- [ ] Cliquer sur **"Déconnecter"**
- [ ] Statut change à : **"⚫ Déconnecté"** (rouge)
- [ ] Bouton change à : **"Connecter"**
- [ ] ComboBox et bouton Rafraîchir réactivés
- [ ] Valeurs restent affichées (dernières valeurs)

### Test de Reconnexion

- [ ] Cliquer à nouveau sur **"Connecter"**
- [ ] Vérifier que la connexion se rétablit
- [ ] Vérifier que les données reprennent

---

## 🗄️ Phase 7 : Test de la Base de Données

### Vérification de la Table

- [ ] Ouvrir **SQL Developer** ou **SQL*Plus**
- [ ] Se connecter à la base de données Oracle
- [ ] Exécuter la requête :
  ```sql
  SELECT table_name FROM user_tables WHERE table_name = 'HISTORIQUE_CAPTEUR';
  ```
- [ ] Vérifier que la table existe

**Si la table n'existe pas :**
- [ ] Lancer le module Arduino Smart
- [ ] Connecter au port série
- [ ] Attendre 5 secondes
- [ ] Revérifier l'existence de la table

### Vérification des Données

- [ ] Connecter le module Arduino Smart
- [ ] Attendre **30 secondes** (première sauvegarde)
- [ ] Exécuter la requête :
  ```sql
  SELECT * FROM historique_capteur ORDER BY date_mesure DESC;
  ```
- [ ] Vérifier qu'au moins **1 enregistrement** est présent
- [ ] Vérifier les colonnes :
  - [ ] `id` : Nombre entier (auto-incrémenté)
  - [ ] `date_mesure` : Date et heure récente
  - [ ] `humidite` : Valeur réaliste (0-100)
  - [ ] `temperature` : Valeur réaliste (-40 à 80)
  - [ ] `alerte` : Message d'alerte (ou NULL)

### Test de Sauvegarde Continue

- [ ] Laisser le module connecté pendant **2 minutes**
- [ ] Exécuter la requête :
  ```sql
  SELECT COUNT(*) FROM historique_capteur;
  ```
- [ ] Vérifier qu'il y a **au moins 4 enregistrements** (2 min / 30s = 4)

---

## 📊 Phase 8 : Test de l'Export CSV

### Export de l'Historique

- [ ] Dans le module Arduino Smart, cliquer sur **"📊 Exporter Historique CSV"**
- [ ] Choisir un emplacement (ex: Bureau)
- [ ] Nommer le fichier : **`historique_capteur.csv`**
- [ ] Cliquer sur **"Enregistrer"**
- [ ] Vérifier le message : **"L'historique a été exporté avec succès"**

### Vérification du Fichier CSV

- [ ] Ouvrir le fichier **`historique_capteur.csv`** avec Excel ou un éditeur de texte
- [ ] Vérifier l'en-tête :
  ```
  Date et Heure,Humidité (%),Température (°C),Alerte
  ```
- [ ] Vérifier les données :
  - [ ] Format de date : `2026-04-26 14:30:15`
  - [ ] Humidité : Nombre avec 2 décimales
  - [ ] Température : Nombre avec 2 décimales
  - [ ] Alerte : Message entre guillemets

**Exemple de ligne :**
```
2026-04-26 14:30:15,45.50,22.30,"✅ Conditions optimales pour le stockage"
```

---

## 🔄 Phase 9 : Test de Robustesse

### Test de Déconnexion Arduino

- [ ] Module connecté et fonctionnel
- [ ] **Débrancher le câble USB** de l'Arduino
- [ ] Vérifier que le statut passe à **"⚫ Déconnecté"**
- [ ] Vérifier l'affichage d'un message d'erreur dans le label d'alerte
- [ ] Vérifier que la reconnexion automatique démarre

### Test de Reconnexion Automatique

- [ ] Attendre **5 secondes** (première tentative)
- [ ] Vérifier le message : **"🔄 Tentative de reconnexion 1 sur COM5"**
- [ ] **Rebrancher le câble USB** de l'Arduino
- [ ] Vérifier que la connexion se rétablit automatiquement
- [ ] Vérifier le message : **"✅ Reconnexion réussie !"**

### Test de Données Aberrantes

- [ ] Modifier temporairement le code Arduino pour envoyer : `H:0.00;T:0.00`
- [ ] Téléverser le code modifié
- [ ] Connecter le module
- [ ] Vérifier que les données **ne s'affichent pas** (filtrées)
- [ ] Vérifier le log dans la console Qt : **"⚠️ Données filtrées (aberrantes)"**
- [ ] Restaurer le code Arduino original

### Test de Port Déjà Utilisé

- [ ] Module connecté
- [ ] Ouvrir l'**IDE Arduino**
- [ ] Ouvrir le **Moniteur Série** (9600 bauds)
- [ ] Dans le module, cliquer sur **"Déconnecter"** puis **"Connecter"**
- [ ] Vérifier le message d'erreur : **"Port déjà utilisé par une autre application"**
- [ ] Fermer le Moniteur Série Arduino
- [ ] Reconnecter le module

---

## 🎨 Phase 10 : Test de l'Interface

### Test de Redimensionnement

- [ ] Redimensionner la fenêtre ArduinoWidget
- [ ] Vérifier que les éléments s'adaptent correctement
- [ ] Vérifier que le graphique reste lisible

### Test de Fermeture

- [ ] Fermer la fenêtre ArduinoWidget (croix rouge)
- [ ] Vérifier que la fenêtre se ferme sans crash
- [ ] Vérifier que l'application principale reste ouverte
- [ ] Rouvrir le module Arduino Smart
- [ ] Vérifier que tout fonctionne à nouveau

### Test de Multiples Instances

- [ ] Ouvrir le module Arduino Smart
- [ ] Cliquer à nouveau sur le bouton "Arduino Smart"
- [ ] Vérifier qu'une **nouvelle fenêtre** s'ouvre
- [ ] Vérifier que les deux fenêtres fonctionnent indépendamment
- [ ] Fermer les deux fenêtres

---

## 📝 Phase 11 : Documentation

### Vérification des Fichiers de Documentation

- [ ] **`MODULE_ARDUINO_README.md`** présent et lisible
- [ ] **`INTEGRATION_BOUTON_ARDUINO.md`** présent et lisible
- [ ] **`COMPILATION_ET_TEST.md`** présent et lisible
- [ ] **`RESUME_MODULE_ARDUINO.md`** présent et lisible
- [ ] **`EXEMPLE_INTEGRATION_RAPIDE.cpp`** présent et lisible
- [ ] **`README_MODULE_ARDUINO.txt`** présent et lisible
- [ ] **`ARCHITECTURE_SYSTEME.txt`** présent et lisible
- [ ] **`CHECKLIST_COMPLETE.md`** présent et lisible (ce fichier)

### Lecture de la Documentation

- [ ] Lire **`README_MODULE_ARDUINO.txt`** (vue d'ensemble)
- [ ] Consulter **`MODULE_ARDUINO_README.md`** (documentation complète)
- [ ] Consulter **`ARCHITECTURE_SYSTEME.txt`** (architecture technique)

---

## ✅ Phase 12 : Validation Finale

### Checklist Globale

- [ ] **Matériel** : Arduino + DHT11 câblés et fonctionnels
- [ ] **Code Arduino** : Téléversé et testé
- [ ] **Projet Qt** : Compilé sans erreur
- [ ] **Bouton** : Intégré et fonctionnel
- [ ] **Module** : Ouvre correctement
- [ ] **Connexion** : Fonctionne (COM5 ou autre)
- [ ] **Données** : Affichées en temps réel
- [ ] **Graphique** : Se met à jour
- [ ] **Alertes** : Fonctionnent selon les seuils
- [ ] **Base de données** : Sauvegarde automatique
- [ ] **Export CSV** : Fonctionne
- [ ] **Reconnexion** : Automatique après déconnexion
- [ ] **Documentation** : Complète et accessible

### Test de Scénario Complet

**Scénario : Surveillance d'une journée de stockage**

1. [ ] Démarrer l'application FIL D'OR
2. [ ] Ouvrir le module Arduino Smart
3. [ ] Connecter au port série
4. [ ] Vérifier l'affichage des données
5. [ ] Laisser tourner pendant 5 minutes
6. [ ] Vérifier que le graphique se remplit
7. [ ] Vérifier que les données sont sauvegardées en BDD
8. [ ] Exporter l'historique en CSV
9. [ ] Ouvrir le fichier CSV et vérifier les données
10. [ ] Débrancher l'Arduino (simuler une panne)
11. [ ] Vérifier la détection de déconnexion
12. [ ] Rebrancher l'Arduino
13. [ ] Vérifier la reconnexion automatique
14. [ ] Fermer le module
15. [ ] Rouvrir le module
16. [ ] Vérifier que tout fonctionne à nouveau

**Résultat attendu :** ✅ Tous les points validés

---

## 🎉 Félicitations !

Si vous avez coché **toutes les cases** de cette checklist, votre **Module Arduino Smart** est **100% opérationnel** ! 🚀

### Prochaines Étapes

- [ ] Former les utilisateurs à l'utilisation du module
- [ ] Définir les seuils d'alerte selon vos besoins spécifiques
- [ ] Planifier la maintenance (vérification capteur, nettoyage)
- [ ] Envisager les évolutions futures (multi-capteurs, notifications, etc.)

### Support

En cas de problème, consultez :
- **`MODULE_ARDUINO_README.md`** (section Dépannage)
- **`COMPILATION_ET_TEST.md`** (section Dépannage)
- Les logs de la console Qt (qDebug)

---

**Bon développement ! 💻**

© 2026 FIL D'OR — Tous droits réservés
