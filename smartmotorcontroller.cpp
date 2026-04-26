#include "smartmotorcontroller.h"

#include <QSerialPort>
#include <QDebug>

SmartMotorController::SmartMotorController()
    : m_serial(new QSerialPort())
{
}

SmartMotorController::~SmartMotorController()
{
    close();
    delete m_serial;
    m_serial = nullptr;
}

void SmartMotorController::close()
{
    if (m_serial && m_serial->isOpen())
        m_serial->close();
    m_triedOpen = false;
}

bool SmartMotorController::isOpen() const
{
    return m_serial && m_serial->isOpen();
}

bool SmartMotorController::ensureOpen()
{
    if (!m_serial)
        return false;
    if (m_serial->isOpen())
        return true;
    if (m_portName.trimmed().isEmpty())
        return false;

    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    m_triedOpen = true;
    return m_serial->open(QIODevice::ReadWrite);
}

bool SmartMotorController::sendCommand(const QString &cmd, QString *errorOut)
{
    if (!ensureOpen()) {
        if (errorOut) {
            if (m_portName.trimmed().isEmpty())
                *errorOut = QStringLiteral("Port Arduino non configure (variable FIL_DOR_ARDUINO_PORT).");
            else if (m_triedOpen)
                *errorOut = QStringLiteral("Impossible d'ouvrir le port serie : %1").arg(m_portName);
            else
                *errorOut = QStringLiteral("Port serie indisponible.");
        }
        return false;
    }

    const QByteArray payload = (cmd + QStringLiteral("\n")).toUtf8();
    const qint64 w = m_serial->write(payload);
    if (w != payload.size()) {
        if (errorOut)
            *errorOut = QStringLiteral("Ecriture serie incomplete.");
        return false;
    }
    qDebug() << "[SmartMoteur][TX]" << payload;
    m_serial->flush();
    return true;
}
