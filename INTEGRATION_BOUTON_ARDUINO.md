# 🔌 Intégration du Bouton Arduino Smart dans l'Interface

## 📋 Options d'Intégration

Vous avez plusieurs options pour ajouter le bouton "Arduino Smart" dans votre interface FIL D'OR.

---

## ✅ Option 1 : Ajouter dans la Barre de Navigation Latérale

### Étape 1 : Ouvrir le fichier UI dans Qt Designer

1. Ouvrez `mainwindow.ui` dans **Qt Designer**
2. Localisez la **barre de navigation latérale** (sidebar) qui contient les boutons :
   - `btn_nav_dashboard`
   - `btn_nav_planif`
   - `btn_nav_fab`
   - `btn_nav_produit`
   - `btn_nav_rh`
   - `btn_nav_stock`
   - `btn_nav_clients`
   - `btn_nav_depot`

### Étape 2 : Ajouter le Bouton

1. **Glissez-déposez** un `QPushButton` dans le layout de la sidebar
2. **Nommez-le** : `btn_nav_arduino`
3. **Texte** : `🔌 Arduino Smart`
4. **Style** : Copiez le style des autres boutons de navigation

### Étape 3 : Connecter le Signal

Dans `mainwindow.cpp`, dans le constructeur `MainWindow::MainWindow()`, ajoutez après les autres connexions de navigation (ligne ~693) :

```cpp
connect(ui->btn_nav_arduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);
```

### Étape 4 : Activer le Bouton après Connexion

Dans la lambda `setNavigationEnabled` (ligne ~593), ajoutez :

```cpp
auto setNavigationEnabled = [this](bool enabled) {
    ui->btn_nav_dashboard->setEnabled(enabled);
    ui->btn_nav_planif->setEnabled(enabled);
    ui->btn_nav_fab->setEnabled(enabled);
    ui->btn_nav_produit->setEnabled(enabled);
    ui->btn_nav_rh->setEnabled(enabled);
    ui->btn_nav_stock->setEnabled(enabled);
    ui->btn_nav_clients->setEnabled(enabled);
    ui->btn_nav_depot->setEnabled(enabled);
    ui->btn_nav_arduino->setEnabled(enabled);  // ← AJOUTER CETTE LIGNE
};
```

---

## ✅ Option 2 : Ajouter dans le Dashboard d'Accueil

### Méthode : Créer une Carte dans le Dashboard

Ajoutez ce code dans la fonction `construireDashboardAccueil()` (après les autres cartes de modules) :

```cpp
// Carte Arduino Smart
QFrame *carteArduino = new QFrame();
carteArduino->setFixedSize(280, 180);
carteArduino->setCursor(Qt::PointingHandCursor);
carteArduino->setStyleSheet(
    "QFrame {"
    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
    "    stop:0 #1565c0, stop:1 #0d47a1);"
    "  border-radius: 16px;"
    "  border: 2px solid rgba(255,255,255,0.1);"
    "}"
    "QFrame:hover {"
    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
    "    stop:0 #1976d2, stop:1 #1565c0);"
    "  border: 2px solid rgba(255,255,255,0.3);"
    "}"
);

QVBoxLayout *layoutArduino = new QVBoxLayout(carteArduino);
layoutArduino->setSpacing(12);
layoutArduino->setContentsMargins(20, 20, 20, 20);

QLabel *iconeArduino = new QLabel("🔌");
iconeArduino->setStyleSheet("font-size: 48px; border: none; background: transparent;");
iconeArduino->setAlignment(Qt::AlignCenter);
layoutArduino->addWidget(iconeArduino);

QLabel *titreArduino = new QLabel("Arduino Smart");
titreArduino->setStyleSheet(
    "font-size: 18px; font-weight: bold; color: white; "
    "border: none; background: transparent;"
);
titreArduino->setAlignment(Qt::AlignCenter);
layoutArduino->addWidget(titreArduino);

QLabel *descArduino = new QLabel("Surveillance Temps Réel");
descArduino->setStyleSheet(
    "font-size: 12px; color: rgba(255,255,255,0.8); "
    "border: none; background: transparent;"
);
descArduino->setAlignment(Qt::AlignCenter);
layoutArduino->addWidget(descArduino);

layoutArduino->addStretch();

// Rendre la carte cliquable
carteArduino->installEventFilter(new ClickableFrameFilter(this, [this]() {
    ouvrirModuleArduino();
}));

// Ajoutez la carte au layout du dashboard
// (Adaptez selon votre structure de layout)
layoutDashboard->addWidget(carteArduino);
```

**Note :** Vous devrez peut-être créer un `ClickableFrameFilter` ou utiliser `mousePressEvent` pour rendre la carte cliquable.

---

## ✅ Option 3 : Ajouter un Bouton dans la Barre d'Outils Principale

### Méthode : Bouton Flottant en Haut à Droite

Ajoutez ce code dans `MainWindow::MainWindow()` :

```cpp
// Bouton Arduino Smart (barre d'outils)
QPushButton *btnArduinoToolbar = new QPushButton("🔌 Arduino Smart");
btnArduinoToolbar->setStyleSheet(
    "QPushButton {"
    "  background-color: #1565c0;"
    "  color: white;"
    "  padding: 10px 20px;"
    "  border-radius: 8px;"
    "  font-weight: bold;"
    "  font-size: 13px;"
    "}"
    "QPushButton:hover {"
    "  background-color: #0d47a1;"
    "}"
);
btnArduinoToolbar->setMinimumHeight(40);
connect(btnArduinoToolbar, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);

// Ajoutez le bouton à la toolbar (adaptez selon votre layout)
ui->toolbarLayout->addWidget(btnArduinoToolbar);
```

---

## ✅ Option 4 : Menu Déroulant

### Méthode : Ajouter dans la Barre de Menu

Si vous avez une barre de menu (`QMenuBar`), ajoutez :

```cpp
// Dans MainWindow::MainWindow()
QMenu *menuOutils = menuBar()->addMenu("Outils");
QAction *actionArduino = menuOutils->addAction("🔌 Arduino Smart");
connect(actionArduino, &QAction::triggered, this, &MainWindow::ouvrirModuleArduino);
```

---

## 🎨 Styles Recommandés

### Style Bouton Bleu (Technologie)

```cpp
"QPushButton {"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
"    stop:0 #1565c0, stop:1 #0d47a1);"
"  color: white;"
"  padding: 12px 24px;"
"  border-radius: 10px;"
"  font-weight: bold;"
"  font-size: 14px;"
"  border: none;"
"}"
"QPushButton:hover {"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
"    stop:0 #1976d2, stop:1 #1565c0);"
"}"
"QPushButton:pressed {"
"  padding: 13px 23px;"
"}"
```

### Style Bouton Vert (IoT)

```cpp
"QPushButton {"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
"    stop:0 #00838f, stop:1 #006064);"
"  color: white;"
"  padding: 12px 24px;"
"  border-radius: 10px;"
"  font-weight: bold;"
"  font-size: 14px;"
"  border: none;"
"}"
"QPushButton:hover {"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
"    stop:0 #00acc1, stop:1 #00838f);"
"}"
```

---

## 🔧 Code Complet pour Intégration Rapide

### Ajout Dynamique dans le Constructeur

Ajoutez ce code à la fin du constructeur `MainWindow::MainWindow()` (après `ui->setupUi(this);`) :

```cpp
// ========== MODULE ARDUINO SMART ==========
QPushButton *btnArduino = new QPushButton("🔌 Arduino Smart");
btnArduino->setStyleSheet(
    "QPushButton {"
    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #0d47a1);"
    "  color: white; padding: 12px 24px; border-radius: 10px;"
    "  font-weight: bold; font-size: 14px; border: none;"
    "}"
    "QPushButton:hover { background: #1976d2; }"
);
btnArduino->setMinimumHeight(45);
btnArduino->setCursor(Qt::PointingHandCursor);
connect(btnArduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);

// Ajoutez le bouton à votre layout principal
// OPTION A : Dans la sidebar (si vous avez un QVBoxLayout nommé sidebarLayout)
// ui->sidebarLayout->addWidget(btnArduino);

// OPTION B : Dans une toolbar horizontale
// ui->toolbarLayout->addWidget(btnArduino);

// OPTION C : Créer un layout flottant en haut à droite
QHBoxLayout *topRightLayout = new QHBoxLayout();
topRightLayout->addStretch();
topRightLayout->addWidget(btnArduino);
topRightLayout->setContentsMargins(0, 10, 20, 0);
// Ajoutez ce layout à votre layout principal
// ui->mainLayout->insertLayout(0, topRightLayout);
```

---

## 📝 Checklist d'Intégration

- [ ] Fichiers Arduino créés (`arduinomanager.h/cpp`, `arduinowidget.h/cpp/ui`)
- [ ] `CMakeLists.txt` modifié (SerialPort ajouté)
- [ ] `mainwindow.h` modifié (include + déclaration fonction)
- [ ] `mainwindow.cpp` modifié (fonction `ouvrirModuleArduino()` ajoutée)
- [ ] Bouton ajouté dans l'interface (UI ou code)
- [ ] Signal connecté au slot `ouvrirModuleArduino()`
- [ ] Compilation réussie
- [ ] Test de connexion Arduino
- [ ] Vérification de la base de données

---

## 🚀 Test Rapide

1. **Compilez le projet** : `cmake --build build`
2. **Lancez l'application**
3. **Cliquez sur le bouton Arduino Smart**
4. **Vérifiez que la fenêtre s'ouvre**
5. **Testez la connexion** avec l'Arduino

---

## 🐛 Dépannage

### Erreur de Compilation : "ArduinoWidget not found"

**Solution :** Vérifiez que `arduinowidget.h` est bien inclus dans `mainwindow.h` :

```cpp
#include "arduinowidget.h"
```

### Erreur de Compilation : "undefined reference to ArduinoWidget"

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

### Le Bouton n'Apparaît Pas

**Solution :** Vérifiez que le bouton est bien ajouté à un layout visible et que le layout est bien défini dans le widget parent.

---

## 📞 Support

Si vous rencontrez des problèmes, vérifiez :

1. Les logs de compilation
2. Les warnings Qt
3. La structure de votre layout dans Qt Designer
4. Les noms des widgets dans le fichier `.ui`

---

**Bonne intégration ! 🎉**
