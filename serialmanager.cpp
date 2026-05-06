#include "serialmanager.h"

#include <QDebug>
#include <QSerialPort>
#include <QTimer>

static bool isMotorCommand(const QString &cmd)
{
    const QString c = cmd.trimmed();
    return c.startsWith(QStringLiteral("MOTOR:"))
        || c == QStringLiteral("1")
        || c == QStringLiteral("2");
}

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_busyTimer(new QTimer(this))
{
    m_busyTimer->setSingleShot(true);
    connect(m_busyTimer, &QTimer::timeout, this, &SerialManager::onBusyTimeout);
    connect(m_serial, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);

    // Bug fix 1: handle hardware errors so the port doesn't silently die.
    connect(m_serial, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError err) {
        if (err == QSerialPort::NoError)
            return;
        qDebug() << "[Serial] errorOccurred:" << err << m_serial->errorString();
        m_rxBuffer.clear();
        m_busy = false;
        m_busyTimer->stop();
        // On resource/device error (Arduino reset, USB disconnect): close port so
        // isConnected() returns false and callers can detect the loss and reconnect.
        if (err == QSerialPort::ResourceError || err == QSerialPort::DeviceNotFoundError) {
            if (m_serial->isOpen())
                m_serial->close();
            emit connectionLost();
        }
    });
}

bool SerialManager::connectPort(const QString &portName)
{
    const QString name = portName.trimmed();
    if (name.isEmpty())
        return false;

    if (m_serial->isOpen())
        m_serial->close();

    m_rxBuffer.clear();
    m_busy = false;
    m_busyTimer->stop();

    m_serial->setPortName(name);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    const bool ok = m_serial->open(QIODevice::ReadWrite);
    if (!ok) {
        qDebug() << "[Serial] Ouverture impossible:" << name << m_serial->errorString();
        return false;
    }
    m_serial->readAll(); // flush stale bytes from OS buffer
    m_lastPort = name;
    return true;
}

bool SerialManager::reconnect()
{
    if (m_lastPort.isEmpty())
        return false;
    qDebug() << "[Serial] Reconnexion sur" << m_lastPort;
    return connectPort(m_lastPort);
}

// Bug fix 2: PING must NOT consume the motor-busy lock.
// autoDetectArduino manages its own QEventLoop+QTimer for the PING/PONG handshake;
// SerialManager must not interfere by setting m_busy for PING.
void SerialManager::reallySend(const QString &cmd)
{
    const bool isPing = cmd.compare(QStringLiteral("PING"), Qt::CaseInsensitive) == 0;
    if (!isPing) {
        m_busy = true;
        int timeout = 5000;
        if (cmd == QStringLiteral("MOTOR:1") || cmd == QStringLiteral("1"))
            timeout = 4000;
        else if (cmd == QStringLiteral("MOTOR:2") || cmd == QStringLiteral("2"))
            timeout = 3000;
        m_busyTimer->start(timeout);
    }
    m_serial->write((cmd + QStringLiteral("\n")).toUtf8());
    m_serial->flush();
}

void SerialManager::sendCommand(const QString &cmd)
{
    if (!m_serial || !m_serial->isOpen()) {
        qDebug() << "[Serial] Port fermé !";
        return;
    }

    const bool motor = isMotorCommand(cmd);

    if (m_busy && motor) {
        m_busyTimer->stop();
        m_busy = false;
    } else if (m_busy && !motor) {
        if (!m_busyTimer->isActive())
            m_busy = false;
        else
            return;
    }

    if (m_busy) {
        qDebug() << "[Serial] Occupé, ignoré:" << cmd;
        return;
    }

    reallySend(cmd);
}

bool SerialManager::isConnected() const
{
    return m_serial && m_serial->isOpen();
}

void SerialManager::disconnectPort()
{
    m_busyTimer->stop();
    m_busy = false;
    m_rxBuffer.clear();
    if (m_serial && m_serial->isOpen())
        m_serial->close();
}

void SerialManager::sendBuzzerCommand(const QString &cmd)
{
    if (!m_serial || !m_serial->isOpen())
        return;
    m_serial->write((cmd.trimmed() + QStringLiteral("\n")).toUtf8());
    m_serial->flush();
}

void SerialManager::processLine(const QString &line)
{
    const QString t = line.trimmed();
    if (t.isEmpty())
        return;

    emit dataReceived(t);

    if (t == QStringLiteral("READY")) {
        m_busy = false;
        m_busyTimer->stop();
        emit arduinoReady();
    } else if (t == QStringLiteral("MOVING:1")) {
        emit motorMoving(1);
    } else if (t == QStringLiteral("MOVING:2")) {
        emit motorMoving(2);
    } else if (t == QStringLiteral("DONE:1")) {
        m_busy = false;
        m_busyTimer->stop();
        emit motorDone(1);
    } else if (t == QStringLiteral("DONE:2")) {
        m_busy = false;
        m_busyTimer->stop();
        emit motorDone(2);
    } else if (t == QStringLiteral("PONG")) {
        // PONG is only used during autoDetect PING; no busy state to clear.
        emit pongReceived();
    } else if (t.startsWith(QStringLiteral("UID:"))) {
        const QString uid = t.mid(4).trimmed();
        if (!uid.isEmpty())
            emit badgeDetected(uid);
    }
}

void SerialManager::onReadyRead()
{
    m_rxBuffer.append(m_serial->readAll());

    for (;;) {
        const int nl = m_rxBuffer.indexOf('\n');
        if (nl < 0)
            break;
        const QByteArray rawLine = m_rxBuffer.left(nl);
        m_rxBuffer.remove(0, nl + 1);

        QString ln = QString::fromUtf8(rawLine).trimmed();
        if (ln.endsWith(QLatin1Char('\r')))
            ln.chop(1);
        processLine(ln);
    }

    if (m_rxBuffer.size() > 4096)
        m_rxBuffer.clear();
}

// Bug fix 3: do NOT drain the serial buffer on timeout.
// The old code called readAll() + rxBuffer.clear() here, which silently discarded
// buffered RFID UID lines that arrived during a motor move — causing RFID to
// "only read one card" and then stop responding.
void SerialManager::onBusyTimeout()
{
    const bool wasBusy = m_busy;
    m_busy = false;
    m_busyTimer->stop();
    if (wasBusy)
        emit motorTimedOut();
}
