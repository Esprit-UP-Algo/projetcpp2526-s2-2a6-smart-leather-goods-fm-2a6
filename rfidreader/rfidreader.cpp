#include "rfidreader.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSerialPortInfo>

namespace {
constexpr int kMaxBuffer = 2048;
}

RFIDReader::RFIDReader(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_buffer()
    , m_currentPort()
    , m_connected(false)
    , m_lastEmittedUid()
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &RFIDReader::readData);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &RFIDReader::handleError);
}

RFIDReader::~RFIDReader()
{
    disconnectReader();
}

bool RFIDReader::connectToReader(const QString &portName)
{
    disconnectReader();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serialPort->errorString());
        emit connectionChanged(false);
        return false;
    }

    m_currentPort = portName;
    m_connected = true;
    m_buffer.clear();
    emit connectionChanged(true);
    return true;
}

void RFIDReader::disconnectReader()
{
    const bool wasOpen = m_serialPort->isOpen();
    if (wasOpen)
        m_serialPort->close();
    m_connected = false;
    m_currentPort.clear();
    m_buffer.clear();
    if (wasOpen)
        emit connectionChanged(false);
}

bool RFIDReader::isConnected() const
{
    return m_serialPort && m_serialPort->isOpen();
}

QStringList RFIDReader::availablePorts()
{
    QStringList ports;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ports.append(info.portName());
    return ports;
}

QString RFIDReader::autoDetectPort()
{
    const QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        const QString desc = info.description().toLower();
        const QString manu = info.manufacturer().toLower();
        if (manu.contains(QStringLiteral("arduino")) || desc.contains(QStringLiteral("arduino"))
            || desc.contains(QStringLiteral("ch340")) || desc.contains(QStringLiteral("cp210"))
            || desc.contains(QStringLiteral("usb-serial")) || desc.contains(QStringLiteral("usb serial")))
            return info.portName();
    }
    if (!infos.isEmpty())
        return infos.first().portName();
    return QString();
}

void RFIDReader::readData()
{
    const QByteArray chunk = m_serialPort->readAll();
    if (!chunk.isEmpty())
        emit rawSerialReceived(QString::fromUtf8(chunk));
    processReceivedData(chunk);
}

void RFIDReader::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;
    emit errorOccurred(m_serialPort->errorString());
    if (error == QSerialPort::ResourceError || error == QSerialPort::PermissionError
        || error == QSerialPort::DeviceNotFoundError) {
        m_connected = false;
        if (m_serialPort->isOpen())
            m_serialPort->close();
        emit connectionChanged(false);
    }
}

void RFIDReader::maybeEmitBadge(const QString &uid)
{
    const QString u = uid.trimmed();
    if (u.isEmpty())
        return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (u.compare(m_lastEmittedUid, Qt::CaseInsensitive) == 0 && (now - m_lastEmittedMs) < 900)
        return;
    m_lastEmittedUid = u;
    m_lastEmittedMs = now;
    emit badgeDetected(u);
}

bool RFIDReader::tryExtractVerboseUidLine()
{
    const QString s = QString::fromLatin1(m_buffer);
    static const QRegularExpression re(
        QStringLiteral(R"(UID\s*\(\s*HEX\s*\)\s*:\s*([0-9a-fA-F]{2}(?:-[0-9a-fA-F]{2})+))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(s);
    if (!m.hasMatch())
        return false;
    const QString uid = m.captured(1).trimmed();
    const int from = m.capturedStart(0);
    const int len = m.capturedLength(0);
    m_buffer.remove(from, len);
    maybeEmitBadge(uid);
    return true;
}

void RFIDReader::processReceivedData(const QByteArray &data)
{
    m_buffer.append(data);

    for (;;) {
        const int tag = m_buffer.indexOf("#RFID:");
        if (tag < 0)
            break;
        if (tag > 0)
            m_buffer.remove(0, tag);

        const int uidStart = 6;
        const int endHash = m_buffer.indexOf('#', uidStart);
        if (endHash < 0)
            break;

        const QString uid = QString::fromLatin1(m_buffer.mid(uidStart, endHash - uidStart));
        m_buffer.remove(0, endHash + 1);
        maybeEmitBadge(uid);
    }

    while (tryExtractVerboseUidLine()) {}

    // Handle "UID:xx:xx:xx:xx" format (unified FIL D'OR sketch via Serial.print("UID:"))
    for (;;) {
        const int nl = m_buffer.indexOf('\n');
        if (nl < 0)
            break;
        QByteArray rawLine = m_buffer.left(nl);
        m_buffer.remove(0, nl + 1);
        if (!rawLine.isEmpty() && rawLine.back() == '\r')
            rawLine.chop(1);
        const QString line = QString::fromUtf8(rawLine).trimmed();
        if (line.startsWith(QStringLiteral("UID:"))) {
            const QString uid = line.mid(4).trimmed();
            if (!uid.isEmpty())
                maybeEmitBadge(uid);
        }
    }

    if (m_buffer.size() > kMaxBuffer)
        m_buffer.remove(0, m_buffer.size() - kMaxBuffer);
}
