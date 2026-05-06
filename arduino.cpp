#include "arduino.h"
#include <QSerialPortInfo>
#include <QDebug>

Arduino::Arduino(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);

    connect(serial, &QSerialPort::readyRead, this, [=]() {
        while (serial->canReadLine()) {
            QString data = QString(serial->readLine()).trimmed();
            if (!data.isEmpty()) {
                qDebug() << "Arduino:" << data;
                emit dataReceived(data);
            }
        }
    });
}

Arduino::~Arduino()
{
    disconnectFromBoard();
}

bool Arduino::connectToBoard(const QString &portName)
{
    if (serial->isOpen())
        serial->close();

    serial->setPortName(portName);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Arduino connected on" << portName;
        return true;
    }
    qDebug() << "Failed to open" << portName << ":" << serial->errorString();
    return false;
}

void Arduino::disconnectFromBoard()
{
    if (serial->isOpen())
        serial->close();
}

bool Arduino::isConnected() const
{
    return serial->isOpen();
}

bool Arduino::connectArduino()
{
    return connectToBoard("COM5");
}

void Arduino::disconnectArduino()
{
    disconnectFromBoard();
}

void Arduino::beep()
{
    if (!serial->isOpen()) {
        qDebug() << "Cannot beep: Arduino not connected";
        return;
    }
    serial->write("B");
    serial->flush();
}
