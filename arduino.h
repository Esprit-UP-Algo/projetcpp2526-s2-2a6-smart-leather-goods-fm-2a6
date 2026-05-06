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

    // Connexion avec nom de port explicite (utilisé dans mainwindow)
    bool connectToBoard(const QString &portName = "COM5");
    void disconnectFromBoard();
    bool isConnected() const;

    // Connexion simplifiée port fixe (COM5 — capteur gaz dépôt)
    bool connectArduino();
    void disconnectArduino();

public slots:
    void beep();

signals:
    void dataReceived(QString data);

private:
    QSerialPort *serial;
};

#endif // ARDUINO_H
