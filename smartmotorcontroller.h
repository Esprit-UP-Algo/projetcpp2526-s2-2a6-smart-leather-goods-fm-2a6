#ifndef SMARTMOTORCONTROLLER_H
#define SMARTMOTORCONTROLLER_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

class QSerialPort;

// Pilotage d'un Arduino en UART (ex: "COM5" sur Windows) pour un servo 3 fils.
// Convention attendue côté Arduino :
//  MOTOR:1 => 90°, MOTOR:2 => 180° (2e et 3e choix Qt).
//  PING    => PONG
class SmartMotorController
{
public:
    SmartMotorController();
    ~SmartMotorController();

    void setPortName(const QString &portName) { m_portName = portName; }
    QString portName() const { return m_portName; }

    bool ensureOpen();
    void close();

    bool isOpen() const;

    bool sendCommand(const QString &cmd, QString *errorOut = nullptr); // ajoute '\n'
    QString readLine(int timeoutMs = 1200, QString *errorOut = nullptr);
    bool sendCommandWithResponse(const QString &cmd,
                                 QString *responseOut,
                                 QString *errorOut = nullptr,
                                 int timeoutMs = 1200);

private:
    QSerialPort *m_serial = nullptr;
    QString m_portName;
    bool m_triedOpen = false;
    bool m_ready = false;
    qint64 m_readyAtMs = 0;
    bool m_busy = false;
    qint64 m_busyUntilMs = 0;
};

#endif
