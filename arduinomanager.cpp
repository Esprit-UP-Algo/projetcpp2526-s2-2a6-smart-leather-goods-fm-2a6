#include "arduinomanager.h"
#include <QDebug>
#include <QRegularExpression>

ArduinoManager::ArduinoManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_timerReconnexion(new QTimer(this))
    , m_tentativesReconnexion(0)
{
    // Configuration du timer de reconnexion (5 secondes)
    m_timerReconnexion->setInterval(5000);
    m_timerReconnexion->setSingleShot(false);
    connect(m_timerReconnexion, &QTimer::timeout, this, &ArduinoManager::tentativeReconnexion);

    // Connexion des signaux du port série
    connect(m_serialPort, &QSerialPort::readyRead, this, &ArduinoManager::lireDonnees);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &ArduinoManager::gererErreur);
}

ArduinoManager::~ArduinoManager()
{
    deconnecter();
}

bool ArduinoManager::connecter(const QString &portName)
{
    // Si déjà connecté, déconnecter d'abord
    if (m_serialPort->isOpen()) {
        deconnecter();
    }

    // Configuration du port série
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // Tentative d'ouverture
    if (m_serialPort->open(QIODevice::ReadOnly)) {
        m_dernierPortUtilise = portName;
        m_buffer.clear();
        m_tentativesReconnexion = 0;
        m_timerReconnexion->stop();
        
        qDebug() << "✅ Arduino connecté sur" << portName;
        emit statutChange(true);
        return true;
    } else {
        QString errMsg = QString("Impossible d'ouvrir le port %1: %2")
                            .arg(portName)
                            .arg(m_serialPort->errorString());
        qDebug() << "❌" << errMsg;
        emit erreurConnexion(errMsg);
        emit statutChange(false);
        return false;
    }
}

void ArduinoManager::deconnecter()
{
    m_timerReconnexion->stop();
    
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        qDebug() << "🔌 Arduino déconnecté";
    }
    
    m_buffer.clear();
    emit statutChange(false);
}

bool ArduinoManager::estConnecte() const
{
    return m_serialPort->isOpen();
}

QStringList ArduinoManager::listerPorts() const
{
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &info : infos) {
        // Afficher des informations détaillées pour aider l'utilisateur
        QString description = info.portName();
        if (!info.description().isEmpty()) {
            description += " - " + info.description();
        }
        if (!info.manufacturer().isEmpty()) {
            description += " (" + info.manufacturer() + ")";
        }
        ports << info.portName();
        qDebug() << "Port disponible:" << description;
    }
    
    return ports;
}

void ArduinoManager::lireDonnees()
{
    // Lire toutes les données disponibles
    QByteArray data = m_serialPort->readAll();
    m_buffer.append(QString::fromUtf8(data));

    // Traiter toutes les lignes complètes dans le buffer
    int pos;
    while ((pos = m_buffer.indexOf('\n')) != -1) {
        QString ligne = m_buffer.left(pos).trimmed();
        m_buffer.remove(0, pos + 1);

        // Ignorer les lignes vides ou les messages de debug Arduino
        if (ligne.isEmpty() || ligne.startsWith("Test") || ligne.startsWith("ERREUR")) {
            continue;
        }

        // Parser la ligne
        double humidite, temperature;
        if (parserLigne(ligne, humidite, temperature)) {
            // Filtrer les données aberrantes
            if (donneesValides(humidite, temperature)) {
                emit donneesRecues(humidite, temperature);
            } else {
                qDebug() << "⚠️ Données filtrées (aberrantes):" << ligne;
            }
        } else {
            qDebug() << "⚠️ Format invalide:" << ligne;
        }
    }

    // Limiter la taille du buffer pour éviter les fuites mémoire
    if (m_buffer.size() > 1024) {
        m_buffer.clear();
    }
}

void ArduinoManager::gererErreur(QSerialPort::SerialPortError error)
{
    // Ignorer les erreurs "NoError" et "TimeoutError" (normales)
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }

    QString errMsg;
    
    switch (error) {
        case QSerialPort::DeviceNotFoundError:
            errMsg = "Arduino non trouvé - Vérifiez la connexion USB";
            break;
        case QSerialPort::PermissionError:
            errMsg = "Port déjà utilisé par une autre application";
            break;
        case QSerialPort::ResourceError:
            errMsg = "Arduino débranché - Tentative de reconnexion...";
            // Démarrer la reconnexion automatique
            if (!m_timerReconnexion->isActive()) {
                m_timerReconnexion->start();
            }
            break;
        default:
            errMsg = QString("Erreur port série: %1").arg(m_serialPort->errorString());
            break;
    }

    qDebug() << "❌" << errMsg;
    emit erreurConnexion(errMsg);
    
    // Fermer le port en cas d'erreur critique
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit statutChange(false);
    }
}

void ArduinoManager::tentativeReconnexion()
{
    m_tentativesReconnexion++;
    
    qDebug() << "🔄 Tentative de reconnexion" << m_tentativesReconnexion << "sur" << m_dernierPortUtilise;
    
    if (connecter(m_dernierPortUtilise)) {
        m_timerReconnexion->stop();
        m_tentativesReconnexion = 0;
        emit erreurConnexion("✅ Reconnexion réussie !");
    } else {
        // Arrêter après 10 tentatives (50 secondes)
        if (m_tentativesReconnexion >= 10) {
            m_timerReconnexion->stop();
            emit erreurConnexion("❌ Reconnexion échouée après 10 tentatives");
        }
    }
}

bool ArduinoManager::parserLigne(const QString &ligne, double &humidite, double &temperature)
{
    // Format attendu: "H:33.00;T:40.30" ou "H:33.00;T:40.30;G:185"
    static QRegularExpression regex(R"(H:([\d.]+);T:([\d.-]+))");
    QRegularExpressionMatch match = regex.match(ligne);

    if (match.hasMatch()) {
        bool okH, okT;
        humidite = match.captured(1).toDouble(&okH);
        temperature = match.captured(2).toDouble(&okT);
        
        if (okH && okT) {
            return true;
        }
    }

    return false;
}

bool ArduinoManager::donneesValides(double humidite, double temperature) const
{
    // Filtrer les lectures à 0.00 (erreur capteur DHT11)
    if (qAbs(humidite) < 0.01 && qAbs(temperature) < 0.01) {
        return false;
    }

    // Vérifier les plages réalistes pour DHT11
    // Humidité: 20-90% (plage typique DHT11)
    // Température: -40 à 80°C (plage DHT11)
    if (humidite < 0.0 || humidite > 100.0) {
        return false;
    }
    
    if (temperature < -40.0 || temperature > 80.0) {
        return false;
    }

    return true;
}
