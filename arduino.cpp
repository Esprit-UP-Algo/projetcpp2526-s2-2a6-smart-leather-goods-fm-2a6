#include "arduino.h"
#include <QSerialPortInfo>
#include <QDebug>

Arduino::Arduino(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);

    // 🔥 Read data automatically
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

bool Arduino::connectArduino()
{
    serial->setPortName("COM7");  // change to your actual port
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "✅ Arduino connected on COM3";
        return true;
    }
    qDebug() << "❌ Failed:" << serial->errorString();
    return false;
}

void Arduino::disconnectArduino()
{
    if (serial->isOpen()) {
        serial->close();
        qDebug() << "Arduino disconnected";
    }
}