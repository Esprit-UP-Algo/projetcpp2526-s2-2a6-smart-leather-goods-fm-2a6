#ifndef SMARTMOTORCONTROLLER_H
#define SMARTMOTORCONTROLLER_H

#include <QByteArray>
#include <QString>

class QSerialPort;

// Pilotage d'un Arduino en UART (ex: "COM3" sur Windows) pour un servo 3 fils.
// Convention attendue côté Arduino :
//  MOTOR:1 => 1er choix
//  MOTOR:2 => 2e choix
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

private:
    QSerialPort *m_serial = nullptr;
    QString m_portName;
    bool m_triedOpen = false;
};

#endif
