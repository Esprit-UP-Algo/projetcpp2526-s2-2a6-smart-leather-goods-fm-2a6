#ifndef ARDUINOMANAGER_H
#define ARDUINOMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>
#include <QTimer>

/**
 * @brief Classe de gestion de la communication avec l'Arduino via port série
 * 
 * Cette classe gère la connexion avec un Arduino Uno équipé d'un capteur DHT11
 * qui envoie des données de température et d'humidité au format "H:xx.xx;T:yy.yy\n"
 */
class ArduinoManager : public QObject
{
    Q_OBJECT

public:
    explicit ArduinoManager(QObject *parent = nullptr);
    ~ArduinoManager();

    /**
     * @brief Connecte au port série spécifié
     * @param portName Nom du port (ex: "COM5")
     * @return true si la connexion réussit, false sinon
     */
    bool connecter(const QString &portName);

    /**
     * @brief Déconnecte le port série
     */
    void deconnecter();

    /**
     * @brief Vérifie si le port est connecté
     * @return true si connecté, false sinon
     */
    bool estConnecte() const;

    /**
     * @brief Liste tous les ports série disponibles
     * @return Liste des noms de ports disponibles
     */
    QStringList listerPorts() const;

signals:
    /**
     * @brief Signal émis quand des données valides sont reçues
     * @param humidite Valeur d'humidité (0-100%)
     * @param temperature Valeur de température (°C)
     */
    void donneesRecues(double humidite, double temperature);

    /**
     * @brief Signal émis en cas d'erreur de connexion
     * @param message Message d'erreur descriptif
     */
    void erreurConnexion(const QString &message);

    /**
     * @brief Signal émis quand le statut de connexion change
     * @param connecte true si connecté, false si déconnecté
     */
    void statutChange(bool connecte);

private slots:
    /**
     * @brief Slot appelé quand des données sont disponibles sur le port série
     * Parse le format "H:xx.xx;T:yy.yy" et émet le signal donneesRecues
     */
    void lireDonnees();

    /**
     * @brief Slot appelé en cas d'erreur sur le port série
     */
    void gererErreur(QSerialPort::SerialPortError error);

    /**
     * @brief Tente une reconnexion automatique
     */
    void tentativeReconnexion();

private:
    QSerialPort *m_serialPort;           // Port série pour communication Arduino
    QString m_buffer;                     // Buffer pour accumuler les données reçues
    QString m_dernierPortUtilise;        // Mémorise le dernier port pour reconnexion
    QTimer *m_timerReconnexion;          // Timer pour reconnexion automatique
    int m_tentativesReconnexion;         // Compteur de tentatives de reconnexion

    /**
     * @brief Parse une ligne de données au format "H:xx.xx;T:yy.yy"
     * @param ligne Ligne à parser
     * @param humidite [out] Valeur d'humidité extraite
     * @param temperature [out] Valeur de température extraite
     * @return true si le parsing réussit, false sinon
     */
    bool parserLigne(const QString &ligne, double &humidite, double &temperature);

    /**
     * @brief Filtre les données aberrantes (H:0.00;T:0.00)
     * @param humidite Valeur d'humidité
     * @param temperature Valeur de température
     * @return true si les données sont valides, false sinon
     */
    bool donneesValides(double humidite, double temperature) const;
};

#endif // ARDUINOMANAGER_H
