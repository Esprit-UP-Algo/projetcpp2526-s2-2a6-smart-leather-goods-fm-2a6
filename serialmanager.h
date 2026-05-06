#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QByteArray>
#include <QObject>
#include <QString>

class QSerialPort;
class QTimer;

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);
    bool connectPort(const QString &portName);
    bool reconnect();           ///< Re-open the last successfully connected port.
    void disconnectPort();
    void sendCommand(const QString &cmd);
    /// Send A/B/V/R buzzer or RFID feedback commands — bypasses motor-busy lock.
    void sendBuzzerCommand(const QString &cmd);
    bool isConnected() const;
    QString lastPort() const { return m_lastPort; }

signals:
    void arduinoReady();
    void motorMoving(int choix);
    void motorDone(int choix);
    void pongReceived();
    void dataReceived(const QString &line);
    /// Émis si aucune ligne DONE/PONG n’a libéré le busy avant la fin du timeout (échec série).
    void motorTimedOut();
    /// Émis quand une ligne UID:<uid> est reçue depuis le lecteur RFID.
    void badgeDetected(const QString &uid);
    /// Émis quand le port série se ferme suite à une erreur matérielle.
    void connectionLost();

private slots:
    void onReadyRead();
    void onBusyTimeout();

private:
    void reallySend(const QString &cmd);
    void processLine(const QString &line);

    QSerialPort *m_serial;
    bool m_busy = false;
    QTimer *m_busyTimer;
    QByteArray m_rxBuffer;
    QString m_lastPort;
};

#endif
