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
    void sendCommand(const QString &cmd);
    bool isConnected() const;

signals:
    void arduinoReady();
    void motorMoving(int choix);
    void motorDone(int choix);
    void pongReceived();
    void dataReceived(const QString &line);
    /// Émis si aucune ligne DONE/PONG n’a libéré le busy avant la fin du timeout (échec série).
    void motorTimedOut();

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
};

#endif
