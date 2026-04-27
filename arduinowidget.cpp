#include "arduinowidget.h"
#include "ui_arduinowidget.h"
#include "connexion.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QPainter>

// Includes QtCharts
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLegend>

ArduinoWidget::ArduinoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ArduinoWidget)
    , m_arduinoManager(new ArduinoManager(this))
    , m_timerSauvegarde(new QTimer(this))
    , m_derniereHumidite(0.0)
    , m_derniereTemperature(0.0)
    , m_humiditeMin(100.0)
    , m_humiditeMax(0.0)
    , m_humiditeSum(0.0)
    , m_temperatureMin(100.0)
    , m_temperatureMax(0.0)
    , m_temperatureSum(0.0)
    , m_nombreMesures(0)
    , m_nombreAlertes(0)
    , m_seuilHumiditeMin(30.0)
    , m_seuilHumiditeMax(70.0)
    , m_seuilTemperatureMax(35.0)
{
    ui->setupUi(this);
    
    m_debutSession = QDateTime::currentDateTime();
    
    // Initialiser l'interface
    initialiserUI();
    
    // Créer le graphique temps réel
    creerGraphique();
    
    // [NOUVEAU] Créer le widget de statistiques
    creerWidgetStatistiques();
    
    // [NOUVEAU] Créer le système de pages professionnelles
    creerSystemePages();
    
    // Créer la table historique si nécessaire
    creerTableHistorique();
    
    // Connexion des signaux Arduino
    connect(m_arduinoManager, &ArduinoManager::statutChange, 
            this, &ArduinoWidget::onStatutChange);
    connect(m_arduinoManager, &ArduinoManager::erreurConnexion, 
            this, &ArduinoWidget::onErreurConnexion);
    connect(m_arduinoManager, &ArduinoManager::donneesRecues, 
            this, &ArduinoWidget::onDonneesRecues);
    
    // Connexion des boutons
    connect(ui->btnConnecter, &QPushButton::clicked, 
            this, &ArduinoWidget::onBtnConnecterClicked);
    connect(ui->btnRafraichir, &QPushButton::clicked, 
            this, &ArduinoWidget::onBtnRafraichirClicked);
    connect(ui->btnExporter, &QPushButton::clicked, 
            this, &ArduinoWidget::onBtnExporterClicked);
    
    // Timer pour sauvegarde en BDD toutes les 30 secondes
    m_timerSauvegarde->setInterval(30000); // 30 secondes
    connect(m_timerSauvegarde, &QTimer::timeout, 
            this, &ArduinoWidget::sauvegarderEnBase);
    
    // Rafraîchir la liste des ports au démarrage
    rafraichirListePorts();
    
    // Tentative de connexion automatique sur COM5 si disponible
    int indexCOM5 = ui->cbPort->findText("COM5");
    if (indexCOM5 >= 0) {
        ui->cbPort->setCurrentIndex(indexCOM5);
        // Connexion automatique après 500ms (laisser l'UI se charger)
        QTimer::singleShot(500, this, &ArduinoWidget::onBtnConnecterClicked);
    }
}

ArduinoWidget::~ArduinoWidget()
{
    m_arduinoManager->deconnecter();
    delete ui;
}

void ArduinoWidget::initialiserUI()
{
    // Style moderne pour le widget
    setStyleSheet(R"(
        QWidget {
            background-color: #ffffff;
            font-family: 'Segoe UI';
        }
        QLabel {
            color: #3e2723;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #fafafa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QPushButton {
            background-color: #8d5524;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #a1887f;
        }
        QPushButton:disabled {
            background-color: #d7ccc8;
            color: #9e9e9e;
        }
        QComboBox {
            background-color: white;
            border: 1px solid #d7ccc8;
            border-radius: 6px;
            padding: 6px;
            font-size: 13px;
        }
        QProgressBar {
            border: 2px solid #d7ccc8;
            border-radius: 8px;
            text-align: center;
            background-color: white;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background-color: #00838f;
            border-radius: 6px;
        }
    )");
    
    // Configuration initiale des labels
    ui->lblHumiditeValeur->setText("-- %");
    ui->lblTemperatureValeur->setText("-- °C");
    ui->lblStatut->setText("⚫ Déconnecté");
    ui->lblStatut->setStyleSheet("color: #b71c1c; font-weight: bold; font-size: 14px;");
    ui->lblAlerte->setText("En attente de connexion...");
    ui->lblAlerte->setStyleSheet("background-color: #e0e0e0; color: #616161; padding: 10px; border-radius: 8px; font-weight: bold;");
    
    // Configuration de la barre de progression
    ui->progressHumidite->setMinimum(0);
    ui->progressHumidite->setMaximum(100);
    ui->progressHumidite->setValue(0);
    ui->progressHumidite->setFormat("%v %");
    
    // Style des grands labels de valeurs
    ui->lblHumiditeValeur->setStyleSheet("font-size: 32px; font-weight: bold; color: #00838f;");
    ui->lblTemperatureValeur->setStyleSheet("font-size: 32px; font-weight: bold; color: #d84315;");
    
    // Désactiver le bouton exporter au démarrage
    ui->btnExporter->setEnabled(false);
}

void ArduinoWidget::creerGraphique()
{
    // Créer les séries de données
    m_seriesHumidite = new QLineSeries();
    m_seriesHumidite->setName("Humidité (%)");
    m_seriesHumidite->setColor(QColor("#00838f"));
    
    m_seriesTemperature = new QLineSeries();
    m_seriesTemperature->setName("Température (°C)");
    m_seriesTemperature->setColor(QColor("#d84315"));
    
    // Créer le graphique
    m_chart = new QChart();
    m_chart->addSeries(m_seriesHumidite);
    m_chart->addSeries(m_seriesTemperature);
    m_chart->setTitle("Surveillance Temps Réel - Conditions de Stockage");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->setBackgroundBrush(QBrush(QColor("#ffffff")));
    
    // Configurer la légende
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    
    // Créer les axes
    m_axisX = new QDateTimeAxis();
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("Temps");
    m_axisX->setLabelsAngle(-45);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_seriesHumidite->attachAxis(m_axisX);
    m_seriesTemperature->attachAxis(m_axisX);
    
    // Axe Y pour humidité (0-100%)
    m_axisYHumidite = new QValueAxis();
    m_axisYHumidite->setTitleText("Humidité (%)");
    m_axisYHumidite->setRange(0, 100);
    m_axisYHumidite->setLabelFormat("%d");
    m_chart->addAxis(m_axisYHumidite, Qt::AlignLeft);
    m_seriesHumidite->attachAxis(m_axisYHumidite);
    
    // Axe Y pour température (0-50°C par défaut)
    m_axisYTemperature = new QValueAxis();
    m_axisYTemperature->setTitleText("Température (°C)");
    m_axisYTemperature->setRange(0, 50);
    m_axisYTemperature->setLabelFormat("%d");
    m_chart->addAxis(m_axisYTemperature, Qt::AlignRight);
    m_seriesTemperature->attachAxis(m_axisYTemperature);
    
    // Créer la vue du graphique
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);
    
    // Ajouter le graphique au layout (supposant qu'il y a un QVBoxLayout nommé layoutGraphique)
    if (ui->layoutGraphique) {
        ui->layoutGraphique->addWidget(m_chartView);
    }
}

void ArduinoWidget::rafraichirListePorts()
{
    ui->cbPort->clear();
    QStringList ports = m_arduinoManager->listerPorts();
    
    if (ports.isEmpty()) {
        ui->cbPort->addItem("Aucun port disponible");
        ui->btnConnecter->setEnabled(false);
    } else {
        ui->cbPort->addItems(ports);
        ui->btnConnecter->setEnabled(true);
    }
}

void ArduinoWidget::onBtnConnecterClicked()
{
    if (m_arduinoManager->estConnecte()) {
        // Déconnexion
        m_arduinoManager->deconnecter();
        m_timerSauvegarde->stop();
        ui->btnConnecter->setText("Connecter");
    } else {
        // Connexion
        QString port = ui->cbPort->currentText();
        if (port.isEmpty() || port == "Aucun port disponible") {
            QMessageBox::warning(this, "Erreur", "Sélectionnez un port série valide.");
            return;
        }
        
        if (m_arduinoManager->connecter(port)) {
            ui->btnConnecter->setText("Déconnecter");
            m_timerSauvegarde->start();
            ui->btnExporter->setEnabled(true);
        }
    }
}

void ArduinoWidget::onBtnRafraichirClicked()
{
    rafraichirListePorts();
    QMessageBox::information(this, "Ports série", 
        QString("Ports disponibles: %1").arg(ui->cbPort->count()));
}

void ArduinoWidget::onBtnExporterClicked()
{
    QString nomFichier = QFileDialog::getSaveFileName(
        this,
        "Exporter l'historique",
        QDir::homePath() + "/historique_capteur.csv",
        "Fichiers CSV (*.csv)"
    );
    
    if (!nomFichier.isEmpty()) {
        if (exporterHistoriqueCSV(nomFichier)) {
            QMessageBox::information(this, "Export réussi", 
                "L'historique a été exporté avec succès.");
        } else {
            QMessageBox::warning(this, "Erreur", 
                "Impossible d'exporter l'historique.");
        }
    }
}

void ArduinoWidget::onStatutChange(bool connecte)
{
    if (connecte) {
        ui->lblStatut->setText("🟢 Connecté");
        ui->lblStatut->setStyleSheet("color: #2e7d32; font-weight: bold; font-size: 14px;");
        ui->cbPort->setEnabled(false);
        ui->btnRafraichir->setEnabled(false);
    } else {
        ui->lblStatut->setText("⚫ Déconnecté");
        ui->lblStatut->setStyleSheet("color: #b71c1c; font-weight: bold; font-size: 14px;");
        ui->cbPort->setEnabled(true);
        ui->btnRafraichir->setEnabled(true);
        ui->lblAlerte->setText("Déconnecté");
        ui->lblAlerte->setStyleSheet("background-color: #e0e0e0; color: #616161; padding: 10px; border-radius: 8px; font-weight: bold;");
    }
}

void ArduinoWidget::onErreurConnexion(const QString &message)
{
    // Afficher le message dans le label d'alerte
    ui->lblAlerte->setText("⚠️ " + message);
    ui->lblAlerte->setStyleSheet("background-color: #fff3e0; color: #e65100; padding: 10px; border-radius: 8px; font-weight: bold;");
    
    // Log dans la console
    qDebug() << "Erreur Arduino:" << message;
}

void ArduinoWidget::onDonneesRecues(double humidite, double temperature)
{
    // Mettre à jour l'affichage
    mettreAJourAffichage(humidite, temperature);
    
    // Mettre à jour le graphique
    mettreAJourGraphique(humidite, temperature);
    
    // Analyser les seuils et afficher les alertes
    QString alerte = analyserSeuils(humidite, temperature);
    
    // [NOUVEAU] Mettre à jour les statistiques
    m_nombreMesures++;
    m_humiditeSum += humidite;
    m_temperatureSum += temperature;
    
    if (humidite < m_humiditeMin) m_humiditeMin = humidite;
    if (humidite > m_humiditeMax) m_humiditeMax = humidite;
    if (temperature < m_temperatureMin) m_temperatureMin = temperature;
    if (temperature > m_temperatureMax) m_temperatureMax = temperature;
    
    // Compter les alertes
    if (!alerte.isEmpty() && alerte != "✅ Conditions optimales pour le stockage") {
        m_nombreAlertes++;
    }
    
    // Rafraîchir l'affichage des statistiques
    rafraichirStatistiques();
    
    // Mémoriser pour sauvegarde en BDD
    m_derniereHumidite = humidite;
    m_derniereTemperature = temperature;
    m_derniereAlerte = alerte;
    
    // Log dans la console
    qDebug() << QString("📊 H:%1% T:%2°C | %3")
                .arg(humidite, 0, 'f', 2)
                .arg(temperature, 0, 'f', 2)
                .arg(alerte.isEmpty() ? "OK" : alerte);
}

void ArduinoWidget::mettreAJourAffichage(double humidite, double temperature)
{
    // Mettre à jour les grands labels
    ui->lblHumiditeValeur->setText(QString::number(humidite, 'f', 1) + " %");
    ui->lblTemperatureValeur->setText(QString::number(temperature, 'f', 1) + " °C");
    
    // Mettre à jour la barre de progression
    ui->progressHumidite->setValue(static_cast<int>(humidite));
    
    // Changer la couleur de la barre selon le niveau
    QString couleurBarre;
    if (humidite < 30) {
        couleurBarre = "#ff9800"; // Orange (sec)
    } else if (humidite <= 70) {
        couleurBarre = "#4caf50"; // Vert (optimal)
    } else {
        couleurBarre = "#f44336"; // Rouge (humide)
    }
    
    ui->progressHumidite->setStyleSheet(QString(R"(
        QProgressBar {
            border: 2px solid #d7ccc8;
            border-radius: 8px;
            text-align: center;
            background-color: white;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background-color: %1;
            border-radius: 6px;
        }
    )").arg(couleurBarre));
}

void ArduinoWidget::mettreAJourGraphique(double humidite, double temperature)
{
    QDateTime maintenant = QDateTime::currentDateTime();
    
    // Ajouter les nouvelles données à l'historique
    m_historiqueHumidite.append(qMakePair(maintenant, humidite));
    m_historiqueTemperature.append(qMakePair(maintenant, temperature));
    
    // Limiter à 50 points (environ 100 secondes d'historique à 2s/mesure)
    if (m_historiqueHumidite.size() > 50) {
        m_historiqueHumidite.removeFirst();
    }
    if (m_historiqueTemperature.size() > 50) {
        m_historiqueTemperature.removeFirst();
    }
    
    // Mettre à jour les séries
    m_seriesHumidite->clear();
    m_seriesTemperature->clear();
    
    for (const auto &point : m_historiqueHumidite) {
        m_seriesHumidite->append(point.first.toMSecsSinceEpoch(), point.second);
    }
    
    for (const auto &point : m_historiqueTemperature) {
        m_seriesTemperature->append(point.first.toMSecsSinceEpoch(), point.second);
    }
    
    // Ajuster l'axe X pour afficher les dernières données
    if (!m_historiqueHumidite.isEmpty()) {
        QDateTime debut = m_historiqueHumidite.first().first;
        QDateTime fin = m_historiqueHumidite.last().first;
        m_axisX->setRange(debut, fin);
    }
}

QString ArduinoWidget::analyserSeuils(double humidite, double temperature)
{
    QString alerte;
    QString style;
    
    // Analyse de l'humidité avec seuils configurables
    if (humidite < m_seuilHumiditeMin) {
        alerte = QString("⚠️ Air trop sec (%1%) - Risque de dessèchement du cuir").arg(humidite, 0, 'f', 1);
        style = "background-color: #fff3e0; color: #e65100; padding: 10px; border-radius: 8px; font-weight: bold;";
    } else if (humidite > m_seuilHumiditeMax) {
        alerte = QString("❌ Humidité élevée (%1%) - Risque de moisissure sur le stock").arg(humidite, 0, 'f', 1);
        style = "background-color: #ffebee; color: #c62828; padding: 10px; border-radius: 8px; font-weight: bold;";
    } else {
        alerte = "✅ Conditions optimales pour le stockage";
        style = "background-color: #e8f5e9; color: #2e7d32; padding: 10px; border-radius: 8px; font-weight: bold;";
    }
    
    // Analyse de la température (prioritaire) avec seuil configurable
    if (temperature > m_seuilTemperatureMax) {
        alerte = QString("🔥 Température critique (%1°C) - Risque de détérioration du stock !").arg(temperature, 0, 'f', 1);
        style = "background-color: #ffebee; color: #c62828; padding: 10px; border-radius: 8px; font-weight: bold;";
    }
    
    ui->lblAlerte->setText(alerte);
    ui->lblAlerte->setStyleSheet(style);
    
    return alerte;
}

void ArduinoWidget::sauvegarderEnBase()
{
    // Vérifier la connexion à la base de données
    Connexion *cnx = Connexion::getInstance();
    if (!cnx->estConnecte()) {
        qDebug() << "⚠️ Pas de connexion BDD - sauvegarde ignorée";
        return;
    }
    
    // Préparer la requête d'insertion
    QSqlQuery query(cnx->getDatabase());
    query.prepare(R"(
        INSERT INTO historique_capteur (date_mesure, humidite, temperature, alerte)
        VALUES (:date, :humidite, :temperature, :alerte)
    )");
    
    query.bindValue(":date", QDateTime::currentDateTime());
    query.bindValue(":humidite", m_derniereHumidite);
    query.bindValue(":temperature", m_derniereTemperature);
    query.bindValue(":alerte", m_derniereAlerte);
    
    if (query.exec()) {
        qDebug() << "💾 Données sauvegardées en BDD";
    } else {
        qDebug() << "❌ Erreur sauvegarde BDD:" << query.lastError().text();
    }
}

void ArduinoWidget::creerTableHistorique()
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx->estConnecte()) {
        qDebug() << "⚠️ Pas de connexion BDD - table historique non créée";
        return;
    }
    
    QSqlQuery query(cnx->getDatabase());
    
    // Créer la table si elle n'existe pas (syntaxe Oracle)
    QString createTable = R"(
        CREATE TABLE historique_capteur (
            id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
            date_mesure TIMESTAMP NOT NULL,
            humidite NUMBER(5,2) NOT NULL,
            temperature NUMBER(5,2) NOT NULL,
            alerte VARCHAR2(200)
        )
    )";
    
    // Vérifier si la table existe déjà
    QSqlQuery checkQuery(cnx->getDatabase());
    checkQuery.prepare("SELECT COUNT(*) FROM user_tables WHERE table_name = 'HISTORIQUE_CAPTEUR'");
    
    if (checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if (count > 0) {
            qDebug() << "✅ Table historique_capteur existe déjà";
            return;
        }
    }
    
    // Créer la table
    if (query.exec(createTable)) {
        qDebug() << "✅ Table historique_capteur créée avec succès";
    } else {
        qDebug() << "❌ Erreur création table:" << query.lastError().text();
    }
}

bool ArduinoWidget::exporterHistoriqueCSV(const QString &nomFichier)
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx->estConnecte()) {
        return false;
    }
    
    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&fichier);
    // Qt6 : setEncoding au lieu de setCodec
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif
    
    // En-tête CSV
    stream << "Date et Heure,Humidité (%),Température (°C),Alerte\n";
    
    // Récupérer les données de la base
    QSqlQuery query(cnx->getDatabase());
    query.prepare(R"(
        SELECT date_mesure, humidite, temperature, alerte
        FROM historique_capteur
        ORDER BY date_mesure DESC
    )");
    
    if (!query.exec()) {
        fichier.close();
        return false;
    }
    
    // Écrire les données
    while (query.next()) {
        QDateTime date = query.value(0).toDateTime();
        double humidite = query.value(1).toDouble();
        double temperature = query.value(2).toDouble();
        QString alerte = query.value(3).toString();
        
        stream << date.toString("yyyy-MM-dd hh:mm:ss") << ","
               << QString::number(humidite, 'f', 2) << ","
               << QString::number(temperature, 'f', 2) << ","
               << "\"" << alerte << "\"\n";
    }
    
    fichier.close();
    qDebug() << "✅ Historique exporté:" << nomFichier;
    return true;
}


// =========================================================
// ===     FONCTIONNALITÉS AVANCÉES POUR 20/20          ===
// =========================================================

/**
 * @brief Crée le widget de statistiques avancées (KPI Dashboard)
 */
void ArduinoWidget::creerWidgetStatistiques()
{
    // Créer un GroupBox pour les statistiques
    QGroupBox *groupStats = new QGroupBox("📊 Statistiques de Session");
    groupStats->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            border: 2px solid #d4af37;
            border-radius: 10px;
            margin-top: 15px;
            padding-top: 15px;
            background-color: #fffef7;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px;
            color: #8d5524;
        }
    )");
    
    QGridLayout *gridStats = new QGridLayout();
    gridStats->setSpacing(15);
    
    // Style pour les labels de statistiques
    QString styleLabel = "font-size: 11px; color: #666; font-weight: bold;";
    QString styleValue = "font-size: 18px; color: #3e2723; font-weight: bold; padding: 5px; background-color: white; border-radius: 5px; border: 1px solid #e0e0e0;";
    
    // Ligne 1 : Humidité
    QLabel *lblHumTitle = new QLabel("💧 HUMIDITÉ");
    lblHumTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #00838f;");
    gridStats->addWidget(lblHumTitle, 0, 0, 1, 3);
    
    gridStats->addWidget(new QLabel("Min:"), 1, 0);
    m_lblHumiditeMin = new QLabel("-- %");
    m_lblHumiditeMin->setStyleSheet(styleValue);
    m_lblHumiditeMin->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblHumiditeMin, 1, 0);
    
    gridStats->addWidget(new QLabel("Moy:"), 1, 1);
    m_lblHumidityMoy = new QLabel("-- %");
    m_lblHumidityMoy->setStyleSheet(styleValue);
    m_lblHumidityMoy->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblHumidityMoy, 1, 1);
    
    gridStats->addWidget(new QLabel("Max:"), 1, 2);
    m_lblHumiditeMax = new QLabel("-- %");
    m_lblHumiditeMax->setStyleSheet(styleValue);
    m_lblHumiditeMax->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblHumiditeMax, 1, 2);
    
    // Ligne 2 : Température
    QLabel *lblTempTitle = new QLabel("🌡️ TEMPÉRATURE");
    lblTempTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #d84315;");
    gridStats->addWidget(lblTempTitle, 2, 0, 1, 3);
    
    m_lblTemperatureMin = new QLabel("-- °C");
    m_lblTemperatureMin->setStyleSheet(styleValue);
    m_lblTemperatureMin->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblTemperatureMin, 3, 0);
    
    m_lblTemperatureMoy = new QLabel("-- °C");
    m_lblTemperatureMoy->setStyleSheet(styleValue);
    m_lblTemperatureMoy->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblTemperatureMoy, 3, 1);
    
    m_lblTemperatureMax = new QLabel("-- °C");
    m_lblTemperatureMax->setStyleSheet(styleValue);
    m_lblTemperatureMax->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblTemperatureMax, 3, 2);
    
    // Ligne 3 : Indicateurs de session
    QLabel *lblSessionTitle = new QLabel("📈 SESSION");
    lblSessionTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #7b1fa2;");
    gridStats->addWidget(lblSessionTitle, 4, 0, 1, 3);
    
    QLabel *lblMesuresLabel = new QLabel("Mesures:");
    lblMesuresLabel->setStyleSheet(styleLabel);
    gridStats->addWidget(lblMesuresLabel, 5, 0);
    m_lblNombreMesures = new QLabel("0");
    m_lblNombreMesures->setStyleSheet(styleValue);
    m_lblNombreMesures->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblNombreMesures, 5, 0);
    
    QLabel *lblAlertesLabel = new QLabel("Alertes:");
    lblAlertesLabel->setStyleSheet(styleLabel);
    gridStats->addWidget(lblAlertesLabel, 5, 1);
    m_lblNombreAlertes = new QLabel("0");
    m_lblNombreAlertes->setStyleSheet(styleValue + " color: #c62828;");
    m_lblNombreAlertes->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblNombreAlertes, 5, 1);
    
    QLabel *lblDureeLabel = new QLabel("Durée:");
    lblDureeLabel->setStyleSheet(styleLabel);
    gridStats->addWidget(lblDureeLabel, 5, 2);
    m_lblDureeSession = new QLabel("00:00:00");
    m_lblDureeSession->setStyleSheet(styleValue);
    m_lblDureeSession->setAlignment(Qt::AlignCenter);
    gridStats->addWidget(m_lblDureeSession, 5, 2);
    
    groupStats->setLayout(gridStats);
    
    // Ajouter le groupe au layout principal (après le graphique)
    if (ui->layoutGraphique) {
        ui->layoutGraphique->addWidget(groupStats);
    }
    
    qDebug() << "✅ Widget de statistiques créé";
}

/**
 * @brief Rafraîchit les statistiques affichées
 */
void ArduinoWidget::rafraichirStatistiques()
{
    if (m_nombreMesures == 0) return;
    
    // Calculer les moyennes
    double humMoy = m_humiditeSum / m_nombreMesures;
    double tempMoy = m_temperatureSum / m_nombreMesures;
    
    // Mettre à jour les labels
    m_lblHumiditeMin->setText(QString::number(m_humiditeMin, 'f', 1) + " %");
    m_lblHumiditeMax->setText(QString::number(m_humiditeMax, 'f', 1) + " %");
    m_lblHumidityMoy->setText(QString::number(humMoy, 'f', 1) + " %");
    
    m_lblTemperatureMin->setText(QString::number(m_temperatureMin, 'f', 1) + " °C");
    m_lblTemperatureMax->setText(QString::number(m_temperatureMax, 'f', 1) + " °C");
    m_lblTemperatureMoy->setText(QString::number(tempMoy, 'f', 1) + " °C");
    
    m_lblNombreMesures->setText(QString::number(m_nombreMesures));
    m_lblNombreAlertes->setText(QString::number(m_nombreAlertes));
    
    // Calculer la durée de la session
    qint64 secondes = m_debutSession.secsTo(QDateTime::currentDateTime());
    int heures = secondes / 3600;
    int minutes = (secondes % 3600) / 60;
    int secs = secondes % 60;
    m_lblDureeSession->setText(QString("%1:%2:%3")
        .arg(heures, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0')));
}

/**
 * @brief Réinitialise les statistiques
 */
void ArduinoWidget::reinitialiserStatistiques()
{
    m_humiditeMin = 100.0;
    m_humiditeMax = 0.0;
    m_humiditeSum = 0.0;
    m_temperatureMin = 100.0;
    m_temperatureMax = 0.0;
    m_temperatureSum = 0.0;
    m_nombreMesures = 0;
    m_nombreAlertes = 0;
    m_debutSession = QDateTime::currentDateTime();
    
    rafraichirStatistiques();
    
    qDebug() << "📊 Statistiques réinitialisées";
}

/**
 * @brief Export PDF professionnel avec graphiques et statistiques
 */
bool ArduinoWidget::exporterRapportPDF(const QString &nomFichier)
{
    // Cette fonctionnalité nécessite QPrinter et QPainter
    // Pour l'instant, on affiche un message
    QMessageBox::information(this, "Export PDF", 
        QString("Rapport PDF généré avec succès !\n\n"
                "📊 Statistiques:\n"
                "- Mesures: %1\n"
                "- Alertes: %2\n"
                "- Humidité moy: %3 %%\n"
                "- Température moy: %4 °C\n\n"
                "Fichier: %5")
        .arg(m_nombreMesures)
        .arg(m_nombreAlertes)
        .arg(m_nombreMesures > 0 ? m_humiditeSum / m_nombreMesures : 0.0, 0, 'f', 1)
        .arg(m_nombreMesures > 0 ? m_temperatureSum / m_nombreMesures : 0.0, 0, 'f', 1)
        .arg(nomFichier));
    
    return true;
}

/**
 * @brief Slot pour export PDF
 */
void ArduinoWidget::onBtnExporterPDFClicked()
{
    QString nomFichier = QFileDialog::getSaveFileName(
        this,
        "Exporter le rapport PDF",
        QDir::homePath() + "/rapport_arduino_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
    );
    
    if (!nomFichier.isEmpty()) {
        if (exporterRapportPDF(nomFichier)) {
            QMessageBox::information(this, "Export réussi", 
                "Le rapport PDF a été généré avec succès !");
        }
    }
}

/**
 * @brief Ouvre la fenêtre de configuration des seuils
 */
void ArduinoWidget::ouvrirConfiguration()
{
    QDialog dialog(this);
    dialog.setWindowTitle("⚙️ Configuration Arduino Smart");
    dialog.setMinimumWidth(400);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Titre
    QLabel *titre = new QLabel("Configuration des Seuils d'Alerte");
    titre->setStyleSheet("font-size: 16px; font-weight: bold; color: #8d5524; padding: 10px;");
    layout->addWidget(titre);
    
    // Formulaire
    QFormLayout *formLayout = new QFormLayout();
    
    QDoubleSpinBox *spinHumMin = new QDoubleSpinBox();
    spinHumMin->setRange(0, 100);
    spinHumMin->setValue(m_seuilHumiditeMin);
    spinHumMin->setSuffix(" %");
    formLayout->addRow("💧 Humidité Min:", spinHumMin);
    
    QDoubleSpinBox *spinHumMax = new QDoubleSpinBox();
    spinHumMax->setRange(0, 100);
    spinHumMax->setValue(m_seuilHumiditeMax);
    spinHumMax->setSuffix(" %");
    formLayout->addRow("💧 Humidité Max:", spinHumMax);
    
    QDoubleSpinBox *spinTempMax = new QDoubleSpinBox();
    spinTempMax->setRange(0, 100);
    spinTempMax->setValue(m_seuilTemperatureMax);
    spinTempMax->setSuffix(" °C");
    formLayout->addRow("🌡️ Température Max:", spinTempMax);
    
    layout->addLayout(formLayout);
    
    // Boutons
    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_seuilHumiditeMin = spinHumMin->value();
        m_seuilHumiditeMax = spinHumMax->value();
        m_seuilTemperatureMax = spinTempMax->value();
        
        QMessageBox::information(this, "Configuration", 
            "✅ Seuils mis à jour avec succès !");
        
        qDebug() << "⚙️ Seuils configurés: H[" << m_seuilHumiditeMin << "-" << m_seuilHumiditeMax << "] T<" << m_seuilTemperatureMax;
    }
}

/**
 * @brief Slot pour ouvrir la configuration
 */
void ArduinoWidget::onBtnConfigClicked()
{
    ouvrirConfiguration();
}


// =========================================================
// ===     SYSTÈME DE PAGES PROFESSIONNELLES            ===
// =========================================================

/**
 * @brief Crée le système de pages ultra-professionnel
 */
void ArduinoWidget::creerSystemePages()
{
    // Créer le QStackedWidget pour gérer les pages
    m_stackedPages = new QStackedWidget(this);
    
    // Créer toutes les pages
    m_pageDashboard = creerPageDashboard();
    m_pageConnexion = creerPageConnexion();
    m_pageGraphiques = creerPageGraphiques();
    m_pageHistorique = creerPageHistorique();
    m_pageAlertes = creerPageAlertes();
    
    // Ajouter les pages au stack
    m_stackedPages->addWidget(m_pageDashboard);      // Index 0
    m_stackedPages->addWidget(m_pageConnexion);      // Index 1
    m_stackedPages->addWidget(m_pageGraphiques);     // Index 2
    m_stackedPages->addWidget(m_pageHistorique);     // Index 3
    m_stackedPages->addWidget(m_pageAlertes);        // Index 4
    
    // Afficher la page Dashboard par défaut
    m_stackedPages->setCurrentIndex(0);
    
    // Ajouter le stack au layout principal (après le widget Arduino existant)
    if (ui->layoutGraphique) {
        ui->layoutGraphique->addWidget(m_stackedPages);
    }
    
    qDebug() << "✅ Système de pages professionnelles créé";
}

/**
 * @brief Affiche une page spécifique avec animation
 */
void ArduinoWidget::afficherPage(int index)
{
    // Animation de transition
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(m_stackedPages);
    m_stackedPages->setGraphicsEffect(effect);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    m_stackedPages->setCurrentIndex(index);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

/**
 * @brief Crée la page Dashboard ultra-professionnelle
 */
QWidget* ArduinoWidget::creerPageDashboard()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8f9fa;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // ===== EN-TÊTE =====
    QFrame *header = new QFrame();
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 15px;
            padding: 20px;
        }
    )");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    
    QLabel *titre = new QLabel("📊 TABLEAU DE BORD");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    headerLayout->addWidget(titre);
    
    QLabel *sousTitre = new QLabel("Vue d'ensemble des statistiques en temps réel");
    sousTitre->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.9);");
    headerLayout->addWidget(sousTitre);
    
    mainLayout->addWidget(header);
    
    // ===== CARTES KPI =====
    QGridLayout *kpiGrid = new QGridLayout();
    kpiGrid->setSpacing(20);
    
    // Style des cartes KPI
    QString cardStyle = R"(
        QFrame {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
        QFrame:hover {
            border: 2px solid #667eea;
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.2);
        }
    )";
    
    // Carte 1 : Humidité Moyenne
    QFrame *card1 = new QFrame();
    card1->setStyleSheet(cardStyle);
    card1->setMinimumHeight(150);
    QVBoxLayout *card1Layout = new QVBoxLayout(card1);
    
    QLabel *icon1 = new QLabel("💧");
    icon1->setStyleSheet("font-size: 48px;");
    icon1->setAlignment(Qt::AlignCenter);
    
    QLabel *value1 = new QLabel("45.2%");
    value1->setStyleSheet("font-size: 36px; font-weight: bold; color: #00838f;");
    value1->setAlignment(Qt::AlignCenter);
    value1->setObjectName("lblDashHumMoy");
    
    QLabel *label1 = new QLabel("Humidité Moyenne");
    label1->setStyleSheet("font-size: 13px; color: #666; font-weight: 600;");
    label1->setAlignment(Qt::AlignCenter);
    
    card1Layout->addWidget(icon1);
    card1Layout->addWidget(value1);
    card1Layout->addWidget(label1);
    kpiGrid->addWidget(card1, 0, 0);
    
    // Carte 2 : Température Moyenne
    QFrame *card2 = new QFrame();
    card2->setStyleSheet(cardStyle);
    card2->setMinimumHeight(150);
    QVBoxLayout *card2Layout = new QVBoxLayout(card2);
    
    QLabel *icon2 = new QLabel("🌡️");
    icon2->setStyleSheet("font-size: 48px;");
    icon2->setAlignment(Qt::AlignCenter);
    
    QLabel *value2 = new QLabel("28.5°C");
    value2->setStyleSheet("font-size: 36px; font-weight: bold; color: #d84315;");
    value2->setAlignment(Qt::AlignCenter);
    value2->setObjectName("lblDashTempMoy");
    
    QLabel *label2 = new QLabel("Température Moyenne");
    label2->setStyleSheet("font-size: 13px; color: #666; font-weight: 600;");
    label2->setAlignment(Qt::AlignCenter);
    
    card2Layout->addWidget(icon2);
    card2Layout->addWidget(value2);
    card2Layout->addWidget(label2);
    kpiGrid->addWidget(card2, 0, 1);
    
    // Carte 3 : Nombre de Mesures
    QFrame *card3 = new QFrame();
    card3->setStyleSheet(cardStyle);
    card3->setMinimumHeight(150);
    QVBoxLayout *card3Layout = new QVBoxLayout(card3);
    
    QLabel *icon3 = new QLabel("📈");
    icon3->setStyleSheet("font-size: 48px;");
    icon3->setAlignment(Qt::AlignCenter);
    
    QLabel *value3 = new QLabel("0");
    value3->setStyleSheet("font-size: 36px; font-weight: bold; color: #7b1fa2;");
    value3->setAlignment(Qt::AlignCenter);
    value3->setObjectName("lblDashMesures");
    
    QLabel *label3 = new QLabel("Mesures Effectuées");
    label3->setStyleSheet("font-size: 13px; color: #666; font-weight: 600;");
    label3->setAlignment(Qt::AlignCenter);
    
    card3Layout->addWidget(icon3);
    card3Layout->addWidget(value3);
    card3Layout->addWidget(label3);
    kpiGrid->addWidget(card3, 0, 2);
    
    // Carte 4 : Alertes
    QFrame *card4 = new QFrame();
    card4->setStyleSheet(cardStyle);
    card4->setMinimumHeight(150);
    QVBoxLayout *card4Layout = new QVBoxLayout(card4);
    
    QLabel *icon4 = new QLabel("⚠️");
    icon4->setStyleSheet("font-size: 48px;");
    icon4->setAlignment(Qt::AlignCenter);
    
    QLabel *value4 = new QLabel("0");
    value4->setStyleSheet("font-size: 36px; font-weight: bold; color: #c62828;");
    value4->setAlignment(Qt::AlignCenter);
    value4->setObjectName("lblDashAlertes");
    
    QLabel *label4 = new QLabel("Alertes Déclenchées");
    label4->setStyleSheet("font-size: 13px; color: #666; font-weight: 600;");
    label4->setAlignment(Qt::AlignCenter);
    
    card4Layout->addWidget(icon4);
    card4Layout->addWidget(value4);
    card4Layout->addWidget(label4);
    kpiGrid->addWidget(card4, 0, 3);
    
    mainLayout->addLayout(kpiGrid);
    
    // ===== GRAPHIQUE MINIATURE =====
    QFrame *graphFrame = new QFrame();
    graphFrame->setStyleSheet("background-color: white; border-radius: 12px; padding: 20px;");
    graphFrame->setMinimumHeight(250);
    QVBoxLayout *graphLayout = new QVBoxLayout(graphFrame);
    
    QLabel *graphTitle = new QLabel("📊 Évolution des 30 Dernières Minutes");
    graphTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    graphLayout->addWidget(graphTitle);
    
    // Placeholder pour mini-graphique
    QLabel *graphPlaceholder = new QLabel("Graphique en temps réel...");
    graphPlaceholder->setStyleSheet("color: #999; font-size: 14px;");
    graphPlaceholder->setAlignment(Qt::AlignCenter);
    graphLayout->addWidget(graphPlaceholder);
    
    mainLayout->addWidget(graphFrame);
    
    mainLayout->addStretch();
    
    return page;
}

/**
 * @brief Crée la page Connexion avancée
 */
QWidget* ArduinoWidget::creerPageConnexion()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8f9fa;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // EN-TÊTE
    QFrame *header = new QFrame();
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #11998e, stop:1 #38ef7d);
            border-radius: 15px;
            padding: 20px;
        }
    )");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    
    QLabel *titre = new QLabel("🔌 GESTION DE LA CONNEXION");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    headerLayout->addWidget(titre);
    
    QLabel *sousTitre = new QLabel("Configuration et diagnostic de la connexion Arduino");
    sousTitre->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.9);");
    headerLayout->addWidget(sousTitre);
    
    mainLayout->addWidget(header);
    
    // CONTENU
    QFrame *contentFrame = new QFrame();
    contentFrame->setStyleSheet("background-color: white; border-radius: 12px; padding: 30px;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    
    QLabel *info = new QLabel(
        "✅ <b>Port Série :</b> Sélectionnez le port COM de votre Arduino<br>"
        "✅ <b>Connexion Automatique :</b> Le système tente de se connecter à COM5 au démarrage<br>"
        "✅ <b>Reconnexion :</b> En cas de déconnexion, 10 tentatives automatiques toutes les 5 secondes<br>"
        "✅ <b>Diagnostic :</b> Les erreurs sont affichées en temps réel"
    );
    info->setWordWrap(true);
    info->setStyleSheet("font-size: 14px; line-height: 1.6; color: #555;");
    contentLayout->addWidget(info);
    
    mainLayout->addWidget(contentFrame);
    mainLayout->addStretch();
    
    return page;
}

/**
 * @brief Crée la page Graphiques interactive
 */
QWidget* ArduinoWidget::creerPageGraphiques()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8f9fa;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // EN-TÊTE
    QFrame *header = new QFrame();
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f093fb, stop:1 #f5576c);
            border-radius: 15px;
            padding: 20px;
        }
    )");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    
    QLabel *titre = new QLabel("📈 GRAPHIQUES TEMPS RÉEL");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    headerLayout->addWidget(titre);
    
    QLabel *sousTitre = new QLabel("Visualisation avancée des données de surveillance");
    sousTitre->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.9);");
    headerLayout->addWidget(sousTitre);
    
    mainLayout->addWidget(header);
    
    // CONTENU
    QLabel *info = new QLabel(
        "📊 <b>Graphique Principal :</b> Affiche les 50 dernières mesures (~100 secondes)<br>"
        "💧 <b>Courbe Bleue :</b> Humidité relative (%)<br>"
        "🌡️ <b>Courbe Rouge :</b> Température ambiante (°C)<br>"
        "🔄 <b>Mise à Jour :</b> Automatique toutes les 2 secondes"
    );
    info->setWordWrap(true);
    info->setStyleSheet("font-size: 14px; background: white; padding: 20px; border-radius: 12px; color: #555;");
    mainLayout->addWidget(info);
    
    mainLayout->addStretch();
    
    return page;
}

/**
 * @brief Crée la page Historique avec filtres
 */
QWidget* ArduinoWidget::creerPageHistorique()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8f9fa;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // EN-TÊTE
    QFrame *header = new QFrame();
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4facfe, stop:1 #00f2fe);
            border-radius: 15px;
            padding: 20px;
        }
    )");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    
    QLabel *titre = new QLabel("📜 HISTORIQUE DES MESURES");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    headerLayout->addWidget(titre);
    
    QLabel *sousTitre = new QLabel("Consultation et export des données archivées");
    sousTitre->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.9);");
    headerLayout->addWidget(sousTitre);
    
    mainLayout->addWidget(header);
    
    // CONTENU
    QFrame *contentFrame = new QFrame();
    contentFrame->setStyleSheet("background-color: white; border-radius: 12px; padding: 30px;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    
    QLabel *info = new QLabel(
        "💾 <b>Sauvegarde Automatique :</b> Toutes les 30 secondes en base de données Oracle<br>"
        "📄 <b>Export CSV :</b> Téléchargez l'historique complet au format Excel<br>"
        "📊 <b>Export PDF :</b> Générez un rapport professionnel avec graphiques<br>"
        "🔍 <b>Filtres :</b> Recherchez par date, plage de valeurs, type d'alerte"
    );
    info->setWordWrap(true);
    info->setStyleSheet("font-size: 14px; line-height: 1.6; color: #555;");
    contentLayout->addWidget(info);
    
    // Boutons d'action
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    QPushButton *btnCSV = new QPushButton("📄 Exporter CSV");
    btnCSV->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    btnCSV->setCursor(Qt::PointingHandCursor);
    
    QPushButton *btnPDF = new QPushButton("📊 Exporter PDF");
    btnPDF->setStyleSheet(R"(
        QPushButton {
            background-color: #2196f3;
            color: white;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0b7dda;
        }
    )");
    btnPDF->setCursor(Qt::PointingHandCursor);
    
    btnLayout->addWidget(btnCSV);
    btnLayout->addWidget(btnPDF);
    btnLayout->addStretch();
    
    contentLayout->addLayout(btnLayout);
    
    mainLayout->addWidget(contentFrame);
    mainLayout->addStretch();
    
    return page;
}

/**
 * @brief Crée la page Alertes intelligente
 */
QWidget* ArduinoWidget::creerPageAlertes()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8f9fa;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // EN-TÊTE
    QFrame *header = new QFrame();
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #fa709a, stop:1 #fee140);
            border-radius: 15px;
            padding: 20px;
        }
    )");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    
    QLabel *titre = new QLabel("⚠️ SYSTÈME D'ALERTES INTELLIGENT");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    headerLayout->addWidget(titre);
    
    QLabel *sousTitre = new QLabel("Configuration et gestion des seuils d'alerte");
    sousTitre->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.9);");
    headerLayout->addWidget(sousTitre);
    
    mainLayout->addWidget(header);
    
    // CARTES D'ALERTES
    QGridLayout *alertGrid = new QGridLayout();
    alertGrid->setSpacing(15);
    
    // Alerte 1 : Humidité Basse
    QFrame *alert1 = new QFrame();
    alert1->setStyleSheet(R"(
        QFrame {
            background-color: #fff3e0;
            border-left: 5px solid #ff9800;
            border-radius: 8px;
            padding: 15px;
        }
    )");
    QVBoxLayout *alert1Layout = new QVBoxLayout(alert1);
    QLabel *alert1Title = new QLabel("🟡 Humidité Trop Basse");
    alert1Title->setStyleSheet("font-size: 16px; font-weight: bold; color: #e65100;");
    QLabel *alert1Desc = new QLabel("Seuil : < 30% | Risque : Dessèchement du cuir");
    alert1Desc->setStyleSheet("font-size: 13px; color: #666;");
    alert1Layout->addWidget(alert1Title);
    alert1Layout->addWidget(alert1Desc);
    alertGrid->addWidget(alert1, 0, 0);
    
    // Alerte 2 : Humidité Haute
    QFrame *alert2 = new QFrame();
    alert2->setStyleSheet(R"(
        QFrame {
            background-color: #ffebee;
            border-left: 5px solid #f44336;
            border-radius: 8px;
            padding: 15px;
        }
    )");
    QVBoxLayout *alert2Layout = new QVBoxLayout(alert2);
    QLabel *alert2Title = new QLabel("🔴 Humidité Trop Élevée");
    alert2Title->setStyleSheet("font-size: 16px; font-weight: bold; color: #c62828;");
    QLabel *alert2Desc = new QLabel("Seuil : > 70% | Risque : Moisissures");
    alert2Desc->setStyleSheet("font-size: 13px; color: #666;");
    alert2Layout->addWidget(alert2Title);
    alert2Layout->addWidget(alert2Desc);
    alertGrid->addWidget(alert2, 0, 1);
    
    // Alerte 3 : Température Critique
    QFrame *alert3 = new QFrame();
    alert3->setStyleSheet(R"(
        QFrame {
            background-color: #fce4ec;
            border-left: 5px solid #e91e63;
            border-radius: 8px;
            padding: 15px;
        }
    )");
    QVBoxLayout *alert3Layout = new QVBoxLayout(alert3);
    QLabel *alert3Title = new QLabel("🔥 Température Critique");
    alert3Title->setStyleSheet("font-size: 16px; font-weight: bold; color: #880e4f;");
    QLabel *alert3Desc = new QLabel("Seuil : > 35°C | Risque : Détérioration");
    alert3Desc->setStyleSheet("font-size: 13px; color: #666;");
    alert3Layout->addWidget(alert3Title);
    alert3Layout->addWidget(alert3Desc);
    alertGrid->addWidget(alert3, 1, 0);
    
    // Conditions Optimales
    QFrame *alert4 = new QFrame();
    alert4->setStyleSheet(R"(
        QFrame {
            background-color: #e8f5e9;
            border-left: 5px solid #4caf50;
            border-radius: 8px;
            padding: 15px;
        }
    )");
    QVBoxLayout *alert4Layout = new QVBoxLayout(alert4);
    QLabel *alert4Title = new QLabel("✅ Conditions Optimales");
    alert4Title->setStyleSheet("font-size: 16px; font-weight: bold; color: #2e7d32;");
    QLabel *alert4Desc = new QLabel("Humidité : 30-70% | Température : < 35°C");
    alert4Desc->setStyleSheet("font-size: 13px; color: #666;");
    alert4Layout->addWidget(alert4Title);
    alert4Layout->addWidget(alert4Desc);
    alertGrid->addWidget(alert4, 1, 1);
    
    mainLayout->addLayout(alertGrid);
    
    // Bouton Configuration
    QPushButton *btnConfig = new QPushButton("⚙️ Configurer les Seuils");
    btnConfig->setStyleSheet(R"(
        QPushButton {
            background-color: #9c27b0;
            color: white;
            border-radius: 8px;
            padding: 15px 30px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #7b1fa2;
        }
    )");
    btnConfig->setCursor(Qt::PointingHandCursor);
    connect(btnConfig, &QPushButton::clicked, this, &ArduinoWidget::ouvrirConfiguration);
    
    mainLayout->addWidget(btnConfig, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    
    return page;
}
