#ifndef ARDUINOWIDGET_H
#define ARDUINOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDateEdit>
#include "arduinomanager.h"

// Includes QtCharts
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

namespace Ui {
class ArduinoWidget;
}

/**
 * @brief Widget d'interface pour le module Arduino Smart
 * 
 * Affiche en temps réel les données du capteur DHT11 (température et humidité)
 * avec graphiques, alertes intelligentes et historique en base de données
 */
class ArduinoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArduinoWidget(QWidget *parent = nullptr);
    ~ArduinoWidget();

private slots:
    // Slots pour la gestion de la connexion Arduino
    void onBtnConnecterClicked();
    void onBtnRafraichirClicked();
    void onStatutChange(bool connecte);
    void onErreurConnexion(const QString &message);
    
    // Slot pour traiter les données reçues
    void onDonneesRecues(double humidite, double temperature);
    
    // Slot pour sauvegarder en base de données (toutes les 30 secondes)
    void sauvegarderEnBase();
    
    // Slot pour exporter l'historique en CSV
    void onBtnExporterClicked();
    
    // [NOUVEAU] Slots pour export PDF
    void onBtnExporterPDFClicked();
    
    // [NOUVEAU] Slots pour configuration
    void onBtnConfigClicked();
    
    // [NOUVEAU] Slot pour rafraîchir les statistiques
    void rafraichirStatistiques();

private:
    Ui::ArduinoWidget *ui;
    ArduinoManager *m_arduinoManager;
    QTimer *m_timerSauvegarde;           // Timer pour sauvegarde BDD toutes les 30s
    
    // [NOUVEAU] Système de navigation par pages
    QStackedWidget *m_stackedPages;
    QWidget *m_pageDashboard;
    QWidget *m_pageConnexion;
    QWidget *m_pageGraphiques;
    QWidget *m_pageHistorique;
    QWidget *m_pageAlertes;
    
    // Données pour le graphique temps réel
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_seriesHumidite;
    QLineSeries *m_seriesTemperature;
    QDateTimeAxis *m_axisX;
    QValueAxis *m_axisYHumidite;
    QValueAxis *m_axisYTemperature;
    
    // Historique des 50 dernières mesures pour le graphique
    QVector<QPair<QDateTime, double>> m_historiqueHumidite;
    QVector<QPair<QDateTime, double>> m_historiqueTemperature;
    
    // Dernières valeurs pour sauvegarde en BDD
    double m_derniereHumidite;
    double m_derniereTemperature;
    QString m_derniereAlerte;
    
    // [NOUVEAU] Statistiques avancées
    double m_humiditeMin;
    double m_humiditeMax;
    double m_humiditeSum;
    double m_temperatureMin;
    double m_temperatureMax;
    double m_temperatureSum;
    int m_nombreMesures;
    int m_nombreAlertes;
    QDateTime m_debutSession;
    
    // [NOUVEAU] Configuration des seuils
    double m_seuilHumiditeMin;
    double m_seuilHumiditeMax;
    double m_seuilTemperatureMax;
    
    // [NOUVEAU] Widgets pour statistiques
    QLabel *m_lblHumiditeMin;
    QLabel *m_lblHumiditeMax;
    QLabel *m_lblHumidityMoy;
    QLabel *m_lblTemperatureMin;
    QLabel *m_lblTemperatureMax;
    QLabel *m_lblTemperatureMoy;
    QLabel *m_lblNombreMesures;
    QLabel *m_lblNombreAlertes;
    QLabel *m_lblDureeSession;
    
    /**
     * @brief Initialise les composants de l'interface
     */
    void initialiserUI();
    
    /**
     * @brief Crée et configure le graphique temps réel
     */
    void creerGraphique();
    
    /**
     * @brief Rafraîchit la liste des ports série disponibles
     */
    void rafraichirListePorts();
    
    /**
     * @brief Met à jour l'affichage des valeurs (labels, progressbar)
     */
    void mettreAJourAffichage(double humidite, double temperature);
    
    /**
     * @brief Met à jour le graphique avec les nouvelles données
     */
    void mettreAJourGraphique(double humidite, double temperature);
    
    /**
     * @brief Analyse les seuils et affiche les alertes
     * @return Message d'alerte (vide si conditions optimales)
     */
    QString analyserSeuils(double humidite, double temperature);
    
    /**
     * @brief Crée la table historique_capteur si elle n'existe pas
     */
    void creerTableHistorique();
    
    /**
     * @brief Exporte l'historique en fichier CSV
     * @param nomFichier Chemin du fichier CSV
     * @return true si l'export réussit, false sinon
     */
    bool exporterHistoriqueCSV(const QString &nomFichier);
    
    /**
     * @brief [NOUVEAU] Exporte un rapport PDF professionnel
     * @param nomFichier Chemin du fichier PDF
     * @return true si l'export réussit, false sinon
     */
    bool exporterRapportPDF(const QString &nomFichier);
    
    /**
     * @brief [NOUVEAU] Crée le widget de statistiques avancées
     */
    void creerWidgetStatistiques();
    
    /**
     * @brief [NOUVEAU] Réinitialise les statistiques
     */
    void reinitialiserStatistiques();
    
    /**
     * @brief [NOUVEAU] Ouvre la fenêtre de configuration
     */
    void ouvrirConfiguration();
    
    /**
     * @brief [NOUVEAU] Crée le système de pages professionnelles
     */
    void creerSystemePages();
    
    /**
     * @brief [NOUVEAU] Crée la page Dashboard ultra-professionnelle
     */
    QWidget* creerPageDashboard();
    
    /**
     * @brief [NOUVEAU] Crée la page Connexion avancée
     */
    QWidget* creerPageConnexion();
    
    /**
     * @brief [NOUVEAU] Crée la page Graphiques interactive
     */
    QWidget* creerPageGraphiques();
    
    /**
     * @brief [NOUVEAU] Crée la page Historique avec filtres
     */
    QWidget* creerPageHistorique();
    
    /**
     * @brief [NOUVEAU] Crée la page Alertes intelligente
     */
    QWidget* creerPageAlertes();
    
    /**
     * @brief [NOUVEAU] Affiche une page spécifique
     */
    void afficherPage(int index);
};

#endif // ARDUINOWIDGET_H
