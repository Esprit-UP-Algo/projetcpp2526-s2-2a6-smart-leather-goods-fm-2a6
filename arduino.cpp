#include "arduino.h"
#include <QDebug>

Arduino::Arduino(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
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

    if (serial->open(QIODevice::WriteOnly)) {
        qDebug() << "Arduino connected on" << portName;
        return true;
    } else {
        qDebug() << "Failed to open" << portName << ":" << serial->errorString();
        return false;
    }
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

void Arduino::beep()
{
    if (!serial->isOpen()) {
        qDebug() << "Cannot beep: Arduino not connected";
        return;
    }
    serial->write("B");
    serial->flush();
}