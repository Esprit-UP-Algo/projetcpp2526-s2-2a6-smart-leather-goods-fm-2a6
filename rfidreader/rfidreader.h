#ifndef RFIDREADER_H
#define RFIDREADER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDebug>

/**
 * @brief Classe pour gérer la communication avec le lecteur RFID Arduino
 * 
 * Cette classe lit les données série envoyées par l'Arduino et émet
 * un signal quand un badge RFID est détecté.
 */
class RFIDReader : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur
     * @param parent Objet parent
     */
    explicit RFIDReader(QObject *parent = nullptr);
    
    /**
     * @brief Destructeur
     */
    ~RFIDReader();

    /**
     * @brief Tente de se connecter au lecteur RFID
     * @param portName Nom du port série (ex: "COM3" ou "/dev/ttyUSB0")
     * @return true si connexion réussie
     */
    bool connectToReader(const QString &portName);
    
    /**
     * @brief Déconnecte le lecteur RFID
     */
    void disconnectReader();
    
    /**
     * @brief Vérifie si le lecteur est connecté
     * @return true si connecté
     */
    bool isConnected() const;
    
    /**
     * @brief Liste les ports série disponibles
     * @return Liste des noms de ports
     */
    static QStringList availablePorts();
    
    /**
     * @brief Recherche automatiquement un port avec Arduino RFID
     * @return Nom du port trouvé ou vide si non trouvé
     */
    static QString autoDetectPort();

signals:
    /**
     * @brief Signal émis quand un badge RFID est détecté
     * @param uid UID du badge lu
     */
    void badgeDetected(const QString &uid);
    
    /**
     * @brief Signal émis en cas d'erreur
     * @param error Message d'erreur
     */
    void errorOccurred(const QString &error);
    
    /**
     * @brief Signal émis quand le lecteur est connecté/déconnecté
     * @param connected État de connexion
     */
    void connectionChanged(bool connected);

    /**
     * @brief Données brutes reçues sur le port (pour journal / diagnostic dans l’UI)
     */
    void rawSerialReceived(const QString &text);

private slots:
    /**
     * @brief Slot appelé quand des données sont disponibles sur le port série
     */
    void readData();
    
    /**
     * @brief Slot appelé en cas d'erreur série
     * @param error Erreur série
     */
    void handleError(QSerialPort::SerialPortError error);

private:
    /**
     * @brief Traite les données reçues et extrait l'UID RFID
     * @param data Données brutes reçues
     */
    void processReceivedData(const QByteArray &data);

    void maybeEmitBadge(const QString &uid);
    /** Ancien sketch Arduino : ligne « UID (HEX) : aa-bb-cc-dd ». */
    bool tryExtractVerboseUidLine();

    QSerialPort *m_serialPort;
    QByteArray m_buffer;
    QString m_currentPort;
    bool m_connected;
    QString m_lastEmittedUid;
    qint64 m_lastEmittedMs = 0;
};

#endif // RFIDREADER_H