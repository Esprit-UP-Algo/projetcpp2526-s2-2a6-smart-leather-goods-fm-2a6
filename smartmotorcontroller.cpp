#include "smartmotorcontroller.h"

#include <QSerialPort>
#include <QDebug>
#include <QDateTime>
#include <QThread>

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
    m_ready = false;
    m_readyAtMs = 0;
    m_busy = false;
    m_busyUntilMs = 0;
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
    const bool ok = m_serial->open(QIODevice::ReadWrite);
    if (ok) {
        m_ready = false;
        m_readyAtMs = QDateTime::currentMSecsSinceEpoch() + 2000;
    }
    return ok;
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
    if (!m_ready) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now < m_readyAtMs) {
            const unsigned long waitMs = static_cast<unsigned long>(m_readyAtMs - now);
            qDebug() << "[SmartMoteur] Attente initialisation Arduino:" << waitMs << "ms";
            QThread::msleep(waitMs);
        }
        m_ready = true;
        qDebug() << "[SmartMoteur] Arduino pret a recevoir.";
    }
    if (m_busy) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now < m_busyUntilMs) {
            if (errorOut)
                *errorOut = QStringLiteral("Moteur occupe, commande ignoree.");
            qDebug() << "[SmartMoteur] Occupe, ignore:" << cmd;
            return false;
        }
        m_busy = false;
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
    m_busy = true;
    m_busyUntilMs = QDateTime::currentMSecsSinceEpoch() + 3000;
    return true;
}

QString SmartMotorController::readLine(int timeoutMs, QString *errorOut)
{
    if (!m_serial || !m_serial->isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Port serie non ouvert.");
        return QString();
    }

    if (!m_serial->canReadLine()) {
        if (!m_serial->waitForReadyRead(timeoutMs)) {
            if (errorOut)
                *errorOut = QStringLiteral("Aucune reponse Arduino (timeout).");
            return QString();
        }
    }

    const QByteArray line = m_serial->readLine().trimmed();
    const QString resp = QString::fromUtf8(line);
    qDebug() << "[SmartMoteur][RX]" << resp;
    return resp;
}

bool SmartMotorController::sendCommandWithResponse(const QString &cmd,
                                                   QString *responseOut,
                                                   QString *errorOut,
                                                   int timeoutMs)
{
    if (responseOut)
        responseOut->clear();
    if (!sendCommand(cmd, errorOut))
        return false;

    QString localErr;
    const QString resp = readLine(timeoutMs, &localErr);
    if (resp.isEmpty()) {
        if (errorOut)
            *errorOut = localErr;
        return false;
    }
    if (responseOut)
        *responseOut = resp;
    m_busy = false;
    m_busyUntilMs = 0;
    return true;
}
