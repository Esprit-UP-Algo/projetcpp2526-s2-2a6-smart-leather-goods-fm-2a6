#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QSerialPort>

class Arduino : public QObject
{
    Q_OBJECT

public:
    explicit Arduino(QObject *parent = nullptr);

    bool connectArduino();
    void disconnectArduino();

signals:
    void dataReceived(QString data); // 🔥 send data to Qt

private:
    QSerialPort *serial;
};

#endif // ARDUINO_H