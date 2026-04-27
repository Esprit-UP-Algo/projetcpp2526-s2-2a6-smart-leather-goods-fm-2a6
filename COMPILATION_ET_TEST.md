# 🔨 Compilation et Test du Module Arduino Smart

## 📋 Prérequis

### Logiciels Requis

- ✅ **Qt Creator** (version 5 ou 6)
- ✅ **CMake** (version 3.16 ou supérieure)
- ✅ **Compilateur C++17** (MSVC, GCC ou Clang)
- ✅ **Qt SerialPort** (inclus dans Qt)
- ✅ **Oracle Database** (pour l'historique)
- ✅ **Arduino IDE** (pour téléverser le code sur l'Arduino)

### Matériel Requis

- ✅ **Arduino Uno** (ou compatible)
- ✅ **Capteur DHT11**
- ✅ **Câbles Dupont** (3x)
- ✅ **Câble USB A-B**

---

## 🔧 Étape 1 : Préparation de l'Arduino

### 1.1 Installation de la Bibliothèque DHT

1. Ouvrez l'**IDE Arduino**
2. Allez dans : **Croquis** > **Inclure une bibliothèque** > **Gérer les bibliothèques**
3. Recherchez **"DHT sensor library"** par **Adafruit**
4. Installez :
   - **DHT sensor library**
   - **Adafruit Unified Sensor** (dépendance)

### 1.2 Câblage du Capteur DHT11

```
DHT11          Arduino Uno
-----          -----------
VCC   -------> 5V
GND   -------> GND
DATA  -------> Pin Digital 3
```

**Note :** Une résistance pull-up de 10kΩ entre VCC et DATA peut améliorer la stabilité (optionnelle).

### 1.3 Téléversement du Code

1. Ouvrez le fichier **`arduino_dht11_code.ino`** dans l'IDE Arduino
2. Sélectionnez : **Outils** > **Type de carte** > **Arduino Uno**
3. Sélectionnez : **Outils** > **Port** > **COMx** (Windows) ou **/dev/ttyUSBx** (Linux)
4. Cliquez sur le bouton **Téléverser** (flèche droite)
5. Attendez le message : **"Téléversement terminé"**

### 1.4 Test du Capteur

1. Ouvrez le **Moniteur Série** : **Outils** > **Moniteur série**
2. Réglez la vitesse à **9600 bauds**
3. Vérifiez que les données s'affichent au format :
   ```
   H:45.50;T:22.30
   H:45.60;T:22.40
   H:45.70;T:22.50
   ```

**Si vous voyez "ERREUR de lecture" :**
- Vérifiez le câblage
- Vérifiez que le capteur est bien un DHT11 (pas DHT22)
- Essayez avec un autre capteur

---

## 🏗️ Étape 2 : Compilation du Projet Qt

### 2.1 Vérification des Fichiers

Assurez-vous que tous les fichiers sont présents :

```
ProjetCpp/
├── CMakeLists.txt                 ✅ Modifié (SerialPort ajouté)
├── mainwindow.h                   ✅ Modifié (include + fonction)
├── mainwindow.cpp                 ✅ Modifié (fonction ouvrirModuleArduino)
├── arduinomanager.h               ✅ Nouveau
├── arduinomanager.cpp             ✅ Nouveau
├── arduinowidget.h                ✅ Nouveau
├── arduinowidget.cpp              ✅ Nouveau
├── arduinowidget.ui               ✅ Nouveau
├── arduino_dht11_code.ino         📄 Code Arduino
├── MODULE_ARDUINO_README.md       📄 Documentation
├── INTEGRATION_BOUTON_ARDUINO.md  📄 Guide d'intégration
└── COMPILATION_ET_TEST.md         📄 Ce fichier
```

### 2.2 Compilation avec CMake

#### Sous Windows (PowerShell ou CMD)

```bash
# Créer le dossier de build (si nécessaire)
mkdir build
cd build

# Configurer le projet
cmake ..

# Compiler
cmake --build . --config Release

# Ou avec Qt Creator :
# Ouvrir CMakeLists.txt > Configurer le projet > Compiler (Ctrl+B)
```

#### Sous Linux

```bash
mkdir build
cd build
cmake ..
make -j4
```

### 2.3 Vérification de la Compilation

Si la compilation réussit, vous devriez voir :

```
[100%] Built target ProjetCpp
```

**En cas d'erreur :**

#### Erreur : "Qt5SerialPort not found"

**Solution :** Installez Qt SerialPort :

```bash
# Ubuntu/Debian
sudo apt-get install libqt5serialport5-dev

# Fedora
sudo dnf install qt5-qtserialport-devel

# Windows : Vérifiez que SerialPort est coché dans Qt Maintenance Tool
```

#### Erreur : "arduinowidget.h: No such file"

**Solution :** Vérifiez que les fichiers sont bien dans le même dossier que `mainwindow.cpp`.

#### Erreur : "undefined reference to ArduinoWidget"

**Solution :** Vérifiez que les fichiers sont bien ajoutés dans `CMakeLists.txt` :

```cmake
set(PROJECT_SOURCES
    ...
    arduinomanager.h
    arduinomanager.cpp
    arduinowidget.h
    arduinowidget.cpp
    arduinowidget.ui
)
```

---

## 🚀 Étape 3 : Intégration du Bouton

### 3.1 Choisir une Méthode d'Intégration

Consultez le fichier **`INTEGRATION_BOUTON_ARDUINO.md`** pour choisir parmi :

1. **Option 1** : Bouton dans la barre de navigation latérale
2. **Option 2** : Carte dans le dashboard d'accueil
3. **Option 3** : Bouton dans la barre d'outils
4. **Option 4** : Menu déroulant

### 3.2 Exemple Rapide (Bouton Dynamique)

Ajoutez ce code dans `MainWindow::MainWindow()` après `ui->setupUi(this);` :

```cpp
// Bouton Arduino Smart
QPushButton *btnArduino = new QPushButton("🔌 Arduino Smart");
btnArduino->setStyleSheet(
    "QPushButton { background: #1565c0; color: white; padding: 12px 24px; "
    "border-radius: 10px; font-weight: bold; font-size: 14px; }"
    "QPushButton:hover { background: #1976d2; }"
);
btnArduino->setMinimumHeight(45);
connect(btnArduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);

// Ajoutez le bouton à votre layout (adaptez selon votre structure)
// Exemple : ui->layoutPrincipal->addWidget(btnArduino);
```

### 3.3 Recompilation

Après avoir ajouté le bouton, recompilez :

```bash
cd build
cmake --build . --config Release
```

---

## 🧪 Étape 4 : Test du Module

### 4.1 Lancement de l'Application

1. **Lancez l'application** : `./ProjetCpp` (Linux) ou `ProjetCpp.exe` (Windows)
2. **Connectez-vous** (si nécessaire)
3. **Cliquez sur le bouton "Arduino Smart"**

### 4.2 Test de Connexion

1. **Vérifiez que l'Arduino est branché** sur le port USB
2. **Fermez l'IDE Arduino** (pour libérer le port série)
3. Dans le module Arduino Smart :
   - Sélectionnez le port (ex: **COM5**)
   - Cliquez sur **"Connecter"**
4. **Vérifiez que le statut passe à "🟢 Connecté"**

### 4.3 Vérification des Données

Vous devriez voir :

- ✅ **Humidité** : Valeur en % (ex: 45.5 %)
- ✅ **Température** : Valeur en °C (ex: 22.3 °C)
- ✅ **Barre de progression** : Se remplit selon l'humidité
- ✅ **Graphique** : Courbes qui se mettent à jour
- ✅ **Alerte** : Message coloré selon les seuils

### 4.4 Test de l'Historique

1. **Attendez 30 secondes** (première sauvegarde automatique)
2. **Vérifiez dans la base de données** :

```sql
SELECT * FROM historique_capteur ORDER BY date_mesure DESC;
```

Vous devriez voir des enregistrements avec :
- `date_mesure` : Date et heure
- `humidite` : Valeur d'humidité
- `temperature` : Valeur de température
- `alerte` : Message d'alerte (si applicable)

### 4.5 Test de l'Export CSV

1. Cliquez sur **"📊 Exporter Historique CSV"**
2. Choisissez un emplacement (ex: `Bureau/historique_capteur.csv`)
3. Ouvrez le fichier CSV avec Excel ou un éditeur de texte
4. Vérifiez que les données sont présentes

---

## 🐛 Dépannage

### Problème : "Aucun port disponible"

**Causes possibles :**
- Arduino non branché
- Pilotes USB non installés
- Port utilisé par une autre application

**Solutions :**
1. Vérifiez le branchement USB
2. Installez les pilotes Arduino (CH340 ou FTDI)
3. Fermez l'IDE Arduino et tout autre logiciel utilisant le port série
4. Redémarrez l'ordinateur

### Problème : "Port déjà utilisé"

**Solution :**
- Fermez l'IDE Arduino
- Fermez le moniteur série
- Vérifiez qu'aucune autre instance de l'application n'est ouverte

### Problème : "Données corrompues" (H:0.00;T:0.00)

**Causes :**
- Capteur DHT11 défectueux
- Mauvais câblage
- Alimentation insuffisante

**Solutions :**
1. Vérifiez les connexions (VCC, GND, DATA)
2. Testez avec un autre capteur DHT11
3. Ajoutez une résistance pull-up 10kΩ entre VCC et DATA
4. Utilisez une alimentation externe 5V si nécessaire

### Problème : "Erreur BDD : table historique_capteur not found"

**Solution :**
La table est créée automatiquement au premier lancement. Si l'erreur persiste :

```sql
CREATE TABLE historique_capteur (
    id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    date_mesure TIMESTAMP NOT NULL,
    humidite NUMBER(5,2) NOT NULL,
    temperature NUMBER(5,2) NOT NULL,
    alerte VARCHAR2(200)
);
```

### Problème : Le graphique ne s'affiche pas

**Causes :**
- QtCharts non installé
- Erreur de layout

**Solutions :**
1. Vérifiez que QtCharts est installé :
   ```cmake
   find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ... Charts)
   ```
2. Vérifiez que le layout `layoutGraphique` existe dans `arduinowidget.ui`

---

## ✅ Checklist de Test Complète

### Matériel
- [ ] Arduino Uno branché et alimenté
- [ ] Capteur DHT11 câblé correctement
- [ ] Code Arduino téléversé
- [ ] Test du moniteur série réussi

### Logiciel
- [ ] Projet compilé sans erreur
- [ ] Bouton Arduino Smart visible dans l'interface
- [ ] Clic sur le bouton ouvre la fenêtre Arduino
- [ ] Liste des ports affichée
- [ ] Connexion au port réussie
- [ ] Statut passe à "Connecté"

### Données
- [ ] Humidité affichée correctement
- [ ] Température affichée correctement
- [ ] Barre de progression fonctionne
- [ ] Graphique se met à jour
- [ ] Alertes s'affichent selon les seuils

### Base de Données
- [ ] Table `historique_capteur` créée
- [ ] Données sauvegardées toutes les 30 secondes
- [ ] Export CSV fonctionne
- [ ] Fichier CSV contient les bonnes données

### Robustesse
- [ ] Déconnexion détectée (débrancher l'Arduino)
- [ ] Reconnexion automatique fonctionne
- [ ] Données aberrantes filtrées (H:0.00;T:0.00)
- [ ] Pas de crash en cas d'erreur

---

## 📊 Résultats Attendus

### Affichage Normal

```
🟢 Connecté
💧 HUMIDITÉ : 45.5 %
🌡️ TEMPÉRATURE : 22.3 °C
✅ Conditions optimales pour le stockage
```

### Alerte Humidité Basse

```
🟢 Connecté
💧 HUMIDITÉ : 28.0 %
🌡️ TEMPÉRATURE : 23.0 °C
⚠️ Air trop sec - Risque de dessèchement du cuir
```

### Alerte Humidité Élevée

```
🟢 Connecté
💧 HUMIDITÉ : 75.0 %
🌡️ TEMPÉRATURE : 24.0 °C
❌ Humidité élevée - Risque de moisissure sur le stock
```

### Alerte Température Critique

```
🟢 Connecté
💧 HUMIDITÉ : 50.0 %
🌡️ TEMPÉRATURE : 38.0 °C
🔥 Température critique - Risque de détérioration du stock !
```

---

## 🎉 Félicitations !

Si tous les tests sont passés, votre **Module Arduino Smart** est opérationnel ! 🚀

Vous pouvez maintenant :
- Surveiller les conditions de stockage en temps réel
- Recevoir des alertes intelligentes
- Consulter l'historique des mesures
- Exporter les données pour analyse

---

## 📚 Documentation Complémentaire

- **`MODULE_ARDUINO_README.md`** : Documentation complète du module
- **`INTEGRATION_BOUTON_ARDUINO.md`** : Guide d'intégration du bouton
- **`arduino_dht11_code.ino`** : Code Arduino commenté

---

## 📞 Support

En cas de problème persistant :

1. Vérifiez les logs de compilation
2. Consultez la documentation Qt SerialPort
3. Testez le capteur avec l'IDE Arduino
4. Vérifiez la connexion à la base de données Oracle

---

**Bon développement ! 💻**

© 2026 FIL D'OR — Tous droits réservés
