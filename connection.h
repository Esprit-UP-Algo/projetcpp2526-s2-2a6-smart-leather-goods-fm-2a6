#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

class Connection
{
public:
    static Connection& getInstance();   // Instance unique
    bool createconnect();               // Ouvrir connexion

private:
    Connection();                       // Constructeur privé
    ~Connection();                      // Destructeur
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    QSqlDatabase db;
};

#endif // CONNECTION_H
