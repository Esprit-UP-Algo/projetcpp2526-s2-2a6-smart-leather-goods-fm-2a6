# 🔌 MODULE ARDUINO SMART - Documentation Complète

## 📋 Vue d'ensemble

Le **Module Arduino Smart** est un système de surveillance en temps réel des conditions de stockage des matières premières (cuir, tissus, etc.) pour l'atelier FIL D'OR. Il utilise un capteur DHT11 connecté à un Arduino Uno pour mesurer la température et l'humidité.

---

## 🎯 Fonctionnalités

### ✅ Surveillance Temps Réel
- **Affichage en direct** de l'humidité (%) et température (°C)
- **Graphique dynamique** des 50 dernières mesures
- **Barre de progression** visuelle pour l'humidité
- **Rafraîchissement automatique** toutes les 2 secondes

### 🚨 Alertes Intelligentes
- **Humidité < 30%** : ⚠️ Air trop sec (risque de dessèchement du cuir)
- **Humidité 30-70%** : ✅ Conditions optimales
- **Humidité > 70%** : ❌ Humidité élevée (risque de moisissure)
- **Température > 35°C** : 🔥 Température critique

### 💾 Historique en Base de Données
- **Sauvegarde automatique** toutes les 30 secondes
- **Table Oracle** : `historique_capteur`
- **Export CSV** de l'historique complet
- **Horodatage précis** de chaque mesure

### 🔄 Reconnexion Automatique
- **Détection de déconnexion** (Arduino débranché)
- **Tentatives automatiques** toutes les 5 secondes
- **Maximum 10 tentatives** (50 secondes)

---

## 🛠️ Installation Matérielle

### Composants Requis
- **1x Arduino Uno** (ou compatible)
- **1x Capteur DHT11** (température + humidité)
- **3x Câbles Dupont** (mâle-femelle)
- **1x Câble USB A-B** (pour connexion PC)

### Schéma de Câblage

```
DHT11          Arduino Uno
-----          -----------
VCC   -------> 5V
GND   -------> GND
DATA  -------> Pin Digital 3
```

### Code Arduino

Téléversez ce code sur votre Arduino Uno :

```cpp
#include <DHT.h>

#define DHTPIN 3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("Test DHT11 demarre...");
}

void loop() {
  delay(2000);  // Lecture toutes les 2 secondes
  
  float humidite = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidite) || isnan(temperature)) {
    Serial.println("ERREUR de lecture du capteur !");
    return;
  }
  
  // Format: H:xx.xx;T:yy.yy
  Serial.print("H:");
  Serial.print(humidite);
  Serial.print(";T:");
  Serial.println(temperature);
}
```

---

## 💻 Installation Logicielle

### 1. Bibliothèque Qt SerialPort

Le module utilise **Qt SerialPort** pour la communication série. Cette bibliothèque est déjà ajoutée dans `CMakeLists.txt` :

```cmake
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ... SerialPort)
target_link_libraries(ProjetCpp PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
```

### 2. Fichiers du Module

Les fichiers suivants ont été créés :

```
arduinomanager.h         # Classe de gestion Arduino
arduinomanager.cpp       # Implémentation
arduinowidget.h          # Interface utilisateur
arduinowidget.cpp        # Implémentation UI
arduinowidget.ui         # Fichier Qt Designer
```

### 3. Intégration dans MainWindow

Ajoutez dans `mainwindow.h` :

```cpp
#include "arduinowidget.h"

// Dans la section private:
void ouvrirModuleArduino();
```

La fonction `ouvrirModuleArduino()` est déjà implémentée dans `mainwindow.cpp`.

### 4. Ajouter un Bouton dans l'Interface

**Option A : Bouton dans la barre de navigation**

Ajoutez un bouton dans le fichier `.ui` (Qt Designer) ou créez-le dynamiquement :

```cpp
// Dans MainWindow::MainWindow() ou construireDashboardAccueil()
QPushButton *btnArduino = new QPushButton("🔌 Arduino Smart");
btnArduino->setStyleSheet(
    "QPushButton { background-color: #1565c0; color: white; "
    "padding: 10px 20px; border-radius: 8px; font-weight: bold; }"
    "QPushButton:hover { background-color: #0d47a1; }"
);
connect(btnArduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);

// Ajoutez le bouton à votre layout principal
// Par exemple : ui->layoutPrincipal->addWidget(btnArduino);
```

**Option B : Menu ou Dashboard**

Ajoutez une carte dans le dashboard d'accueil :

```cpp
// Dans construireDashboardAccueil()
QFrame *carteArduino = creerCarteStat("🔌", "Arduino", "Surveillance", "#1565c0");
connect(carteArduino, &QFrame::clicked, this, &MainWindow::ouvrirModuleArduino);
```

---

## 🗄️ Structure de la Base de Données

### Table `historique_capteur`

```sql
CREATE TABLE historique_capteur (
    id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    date_mesure TIMESTAMP NOT NULL,
    humidite NUMBER(5,2) NOT NULL,
    temperature NUMBER(5,2) NOT NULL,
    alerte VARCHAR2(200)
);
```

**Colonnes :**
- `id` : Identifiant unique (auto-incrémenté)
- `date_mesure` : Date et heure de la mesure
- `humidite` : Valeur d'humidité (0-100%)
- `temperature` : Valeur de température (°C)
- `alerte` : Message d'alerte (si applicable)

**Note :** La table est créée automatiquement au premier lancement du module.

---

## 🚀 Utilisation

### 1. Connexion

1. **Branchez l'Arduino** sur le port USB (généralement COM5 sous Windows)
2. **Ouvrez le module** via le bouton "Arduino Smart"
3. **Sélectionnez le port** dans la liste déroulante
4. **Cliquez sur "Connecter"**

Le module tente une **connexion automatique sur COM5** au démarrage.

### 2. Surveillance

Une fois connecté :
- Les **valeurs s'affichent en temps réel** (toutes les 2 secondes)
- Le **graphique se met à jour** automatiquement
- Les **alertes apparaissent** selon les seuils
- Les **données sont sauvegardées** en BDD toutes les 30 secondes

### 3. Export de l'Historique

1. Cliquez sur **"📊 Exporter Historique CSV"**
2. Choisissez l'emplacement du fichier
3. Le fichier CSV contient toutes les mesures enregistrées

**Format CSV :**
```
Date et Heure,Humidité (%),Température (°C),Alerte
2026-04-26 14:30:15,45.50,22.30,"✅ Conditions optimales"
2026-04-26 14:30:45,46.20,22.50,"✅ Conditions optimales"
```

---

## 🔧 Dépannage

### ❌ "Aucun port disponible"

**Causes possibles :**
- Arduino non branché
- Pilotes USB non installés
- Port utilisé par une autre application

**Solutions :**
1. Vérifiez le branchement USB
2. Installez les pilotes Arduino (CH340 ou FTDI)
3. Fermez l'IDE Arduino ou tout autre logiciel utilisant le port série

### ❌ "Port déjà utilisé"

**Solution :**
- Fermez l'IDE Arduino
- Fermez le moniteur série
- Redémarrez l'application

### ❌ "Données corrompues" (H:0.00;T:0.00)

**Causes :**
- Capteur DHT11 défectueux
- Mauvais câblage
- Alimentation insuffisante

**Solutions :**
1. Vérifiez les connexions (VCC, GND, DATA)
2. Testez avec un autre capteur DHT11
3. Utilisez une alimentation externe 5V si nécessaire

### ❌ "Arduino débranché en cours de session"

Le module détecte automatiquement la déconnexion et tente une **reconnexion automatique** toutes les 5 secondes (max 10 tentatives).

---

## 📊 Seuils Recommandés

### Pour le Stockage du Cuir

| Paramètre | Plage Optimale | Risques |
|-----------|----------------|---------|
| **Humidité** | 40-60% | < 30% : Dessèchement, craquelures<br>> 70% : Moisissure, déformation |
| **Température** | 15-25°C | > 35°C : Détérioration accélérée |

### Personnalisation des Seuils

Modifiez la fonction `analyserSeuils()` dans `arduinowidget.cpp` :

```cpp
QString ArduinoWidget::analyserSeuils(double humidite, double temperature)
{
    // Personnalisez les seuils ici
    if (humidite < 40) {  // Au lieu de 30
        alerte = "⚠️ Humidité trop basse";
    }
    // ...
}
```

---

## 🎨 Personnalisation de l'Interface

### Couleurs

Modifiez le stylesheet dans `initialiserUI()` :

```cpp
ui->lblHumiditeValeur->setStyleSheet(
    "font-size: 32px; font-weight: bold; color: #00838f;"  // Changez la couleur
);
```

### Fréquence de Sauvegarde

Modifiez l'intervalle du timer (par défaut 30 secondes) :

```cpp
m_timerSauvegarde->setInterval(60000);  // 60 secondes
```

### Nombre de Points sur le Graphique

Modifiez la limite dans `mettreAJourGraphique()` :

```cpp
if (m_historiqueHumidite.size() > 100) {  // Au lieu de 50
    m_historiqueHumidite.removeFirst();
}
```

---

## 📈 Évolutions Futures

### Fonctionnalités Possibles

- [ ] **Multi-capteurs** : Surveiller plusieurs zones de stockage
- [ ] **Notifications push** : Alertes par email/SMS
- [ ] **Contrôle automatique** : Activer ventilation/déshumidificateur
- [ ] **Prédictions IA** : Anticiper les variations de conditions
- [ ] **Rapports PDF** : Génération automatique de rapports mensuels
- [ ] **Dashboard web** : Accès distant via navigateur

### Capteurs Additionnels

- **DHT22** : Précision améliorée (±2% humidité, ±0.5°C)
- **BME280** : Température + humidité + pression atmosphérique
- **DS18B20** : Température haute précision (±0.5°C)

---

## 📞 Support

Pour toute question ou problème :

1. **Vérifiez cette documentation**
2. **Consultez les logs** dans la console Qt (qDebug)
3. **Testez le capteur** avec l'IDE Arduino
4. **Vérifiez la base de données** Oracle

---

## 📝 Licence

Ce module fait partie du projet **FIL D'OR** - Atelier de Maroquinerie de Luxe.

© 2026 FIL D'OR — Tous droits réservés

---

## 🙏 Remerciements

- **Qt Framework** : Interface graphique et communication série
- **Arduino** : Plateforme matérielle
- **DHT Library** : Bibliothèque pour capteur DHT11
- **Oracle Database** : Stockage des données

---

**Version :** 1.0.0  
**Date :** 26 avril 2026  
**Auteur :** Équipe FIL D'OR
