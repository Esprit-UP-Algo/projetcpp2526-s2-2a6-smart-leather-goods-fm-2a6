#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QSerialPort>
#include <QString>

class Arduino : public QObject
{
    Q_OBJECT
public:
    explicit Arduino(QObject *parent = nullptr);
    ~Arduino();

    bool connectToBoard(const QString &portName = "COM3");
    void disconnectFromBoard();
    bool isConnected() const;

public slots:
    void beep();

private:
    QSerialPort *serial;
};

#endif // ARDUINO_H