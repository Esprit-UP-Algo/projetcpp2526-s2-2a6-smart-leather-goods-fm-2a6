/*
 * ========================================
 * EXEMPLE D'INTÉGRATION RAPIDE
 * Module Arduino Smart dans MainWindow
 * ========================================
 * 
 * Ce fichier montre comment intégrer rapidement le bouton
 * Arduino Smart dans votre interface FIL D'OR.
 * 
 * Copiez-collez les sections appropriées dans votre code.
 */

// ========================================
// 1. DANS mainwindow.h
// ========================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
// ... autres includes ...

// ✅ AJOUTER CET INCLUDE
#include "arduinowidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // ... vos variables existantes ...
    
    // ✅ AJOUTER CETTE DÉCLARATION
    void ouvrirModuleArduino();
};

#endif // MAINWINDOW_H


// ========================================
// 2. DANS mainwindow.cpp - CONSTRUCTEUR
// ========================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // ... votre code d'initialisation existant ...
    
    // ========================================
    // ✅ OPTION A : BOUTON DANS LA SIDEBAR
    // ========================================
    
    // Si vous avez une sidebar avec des boutons de navigation
    QPushButton *btnArduino = new QPushButton("🔌 Arduino Smart");
    btnArduino->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #1565c0, stop:1 #0d47a1);"
        "  color: white;"
        "  padding: 12px 20px;"
        "  border-radius: 10px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #1976d2, stop:1 #1565c0);"
        "}"
    );
    btnArduino->setMinimumHeight(45);
    btnArduino->setCursor(Qt::PointingHandCursor);
    
    // Connecter le signal au slot
    connect(btnArduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);
    
    // Ajouter le bouton à votre layout de sidebar
    // (Adaptez selon votre structure)
    // ui->sidebarLayout->addWidget(btnArduino);
    
    
    // ========================================
    // ✅ OPTION B : BOUTON FLOTTANT EN HAUT À DROITE
    // ========================================
    
    QPushButton *btnArduinoFloat = new QPushButton("🔌 Arduino");
    btnArduinoFloat->setStyleSheet(
        "QPushButton {"
        "  background-color: #1565c0;"
        "  color: white;"
        "  padding: 10px 20px;"
        "  border-radius: 8px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #1976d2; }"
    );
    btnArduinoFloat->setFixedSize(140, 40);
    btnArduinoFloat->setCursor(Qt::PointingHandCursor);
    connect(btnArduinoFloat, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);
    
    // Positionner en haut à droite (coordonnées absolues)
    // btnArduinoFloat->move(width() - 160, 20);
    // btnArduinoFloat->setParent(this);
    
    
    // ========================================
    // ✅ OPTION C : CARTE DANS LE DASHBOARD
    // ========================================
    
    // Créer une carte cliquable
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
    
    QVBoxLayout *layoutCarte = new QVBoxLayout(carteArduino);
    layoutCarte->setSpacing(12);
    layoutCarte->setContentsMargins(20, 20, 20, 20);
    
    // Icône
    QLabel *icone = new QLabel("🔌");
    icone->setStyleSheet("font-size: 48px; border: none; background: transparent;");
    icone->setAlignment(Qt::AlignCenter);
    layoutCarte->addWidget(icone);
    
    // Titre
    QLabel *titre = new QLabel("Arduino Smart");
    titre->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: white; "
        "border: none; background: transparent;"
    );
    titre->setAlignment(Qt::AlignCenter);
    layoutCarte->addWidget(titre);
    
    // Description
    QLabel *desc = new QLabel("Surveillance Temps Réel");
    desc->setStyleSheet(
        "font-size: 12px; color: rgba(255,255,255,0.8); "
        "border: none; background: transparent;"
    );
    desc->setAlignment(Qt::AlignCenter);
    layoutCarte->addWidget(desc);
    
    layoutCarte->addStretch();
    
    // Rendre la carte cliquable avec un event filter
    carteArduino->installEventFilter(this);
    carteArduino->setProperty("action", "arduino");
    
    // Ajouter la carte au layout du dashboard
    // ui->dashboardLayout->addWidget(carteArduino);
}


// ========================================
// 3. DANS mainwindow.cpp - FONCTION D'OUVERTURE
// ========================================

void MainWindow::ouvrirModuleArduino()
{
    // Créer une nouvelle fenêtre pour le module Arduino
    ArduinoWidget *arduinoWindow = new ArduinoWidget();
    
    // Configurer la fenêtre
    arduinoWindow->setWindowTitle("FIL D'OR - Module Arduino Smart");
    arduinoWindow->setWindowIcon(windowIcon());
    arduinoWindow->setAttribute(Qt::WA_DeleteOnClose); // Libérer la mémoire à la fermeture
    arduinoWindow->setMinimumSize(900, 700);
    
    // Afficher la fenêtre
    arduinoWindow->show();
    
    qDebug() << "✅ Module Arduino Smart ouvert";
}


// ========================================
// 4. EVENT FILTER POUR CARTE CLIQUABLE (OPTIONNEL)
// ========================================

// Si vous utilisez l'Option C (carte cliquable), ajoutez ceci :

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *frame = qobject_cast<QFrame*>(obj);
        if (frame && frame->property("action").toString() == "arduino") {
            ouvrirModuleArduino();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


// ========================================
// 5. ALTERNATIVE : MENU DÉROULANT
// ========================================

// Dans le constructeur, si vous avez une barre de menu :

void MainWindow::creerMenus()
{
    // Menu Outils
    QMenu *menuOutils = menuBar()->addMenu("Outils");
    
    // Action Arduino Smart
    QAction *actionArduino = new QAction("🔌 Arduino Smart", this);
    actionArduino->setShortcut(QKeySequence("Ctrl+Shift+A"));
    actionArduino->setStatusTip("Ouvrir le module de surveillance Arduino");
    connect(actionArduino, &QAction::triggered, this, &MainWindow::ouvrirModuleArduino);
    
    menuOutils->addAction(actionArduino);
}


// ========================================
// 6. ALTERNATIVE : TOOLBAR
// ========================================

// Dans le constructeur, si vous avez une toolbar :

void MainWindow::creerToolbar()
{
    QToolBar *toolbar = addToolBar("Outils");
    
    // Action Arduino Smart
    QAction *actionArduino = new QAction(QIcon(":/icons/arduino.png"), "Arduino Smart", this);
    actionArduino->setStatusTip("Ouvrir le module de surveillance Arduino");
    connect(actionArduino, &QAction::triggered, this, &MainWindow::ouvrirModuleArduino);
    
    toolbar->addAction(actionArduino);
}


// ========================================
// 7. INTÉGRATION DANS LA PAGE DE LOGIN
// ========================================

// Si vous voulez ajouter un bouton après la connexion :

void MainWindow::onLoginSuccess()
{
    // ... votre code de connexion existant ...
    
    // Activer le bouton Arduino (si désactivé par défaut)
    if (btnArduino) {
        btnArduino->setEnabled(true);
    }
    
    // Ou afficher une notification
    QMessageBox::information(this, "Module Arduino", 
        "Le module Arduino Smart est maintenant disponible !");
}


// ========================================
// 8. STYLE PERSONNALISÉ POUR LE BOUTON
// ========================================

QString styleArduinoButton()
{
    return R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1565c0, stop:1 #0d47a1);
            color: white;
            padding: 12px 24px;
            border-radius: 10px;
            font-weight: bold;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1976d2, stop:1 #1565c0);
        }
        QPushButton:pressed {
            padding: 13px 23px;
        }
        QPushButton:disabled {
            background: #bdbdbd;
            color: #757575;
        }
    )";
}


// ========================================
// 9. EXEMPLE COMPLET MINIMAL
// ========================================

/*
 * Voici un exemple complet minimal pour intégrer rapidement
 * le module Arduino Smart dans votre application.
 */

// Dans mainwindow.h :
// #include "arduinowidget.h"
// void ouvrirModuleArduino();

// Dans mainwindow.cpp (constructeur) :
QPushButton *btnArduino = new QPushButton("🔌 Arduino Smart");
btnArduino->setStyleSheet(
    "QPushButton { background: #1565c0; color: white; padding: 12px 24px; "
    "border-radius: 10px; font-weight: bold; }"
    "QPushButton:hover { background: #1976d2; }"
);
connect(btnArduino, &QPushButton::clicked, this, &MainWindow::ouvrirModuleArduino);
// Ajoutez btnArduino à votre layout

// Dans mainwindow.cpp (fonction) :
void MainWindow::ouvrirModuleArduino()
{
    ArduinoWidget *w = new ArduinoWidget();
    w->setWindowTitle("FIL D'OR - Arduino Smart");
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setMinimumSize(900, 700);
    w->show();
}


// ========================================
// 10. CHECKLIST D'INTÉGRATION
// ========================================

/*
 * ✅ Fichiers créés :
 *    - arduinomanager.h
 *    - arduinomanager.cpp
 *    - arduinowidget.h
 *    - arduinowidget.cpp
 *    - arduinowidget.ui
 * 
 * ✅ CMakeLists.txt modifié :
 *    - SerialPort ajouté
 *    - Fichiers Arduino ajoutés
 * 
 * ✅ mainwindow.h modifié :
 *    - #include "arduinowidget.h"
 *    - void ouvrirModuleArduino();
 * 
 * ✅ mainwindow.cpp modifié :
 *    - Fonction ouvrirModuleArduino() implémentée
 *    - Bouton ajouté et connecté
 * 
 * ✅ Compilation :
 *    - Projet compilé sans erreur
 * 
 * ✅ Test :
 *    - Bouton visible dans l'interface
 *    - Clic ouvre la fenêtre Arduino
 *    - Connexion au port série fonctionne
 */


// ========================================
// FIN DE L'EXEMPLE
// ========================================

/*
 * Pour plus d'informations, consultez :
 * - MODULE_ARDUINO_README.md
 * - INTEGRATION_BOUTON_ARDUINO.md
 * - COMPILATION_ET_TEST.md
 */
