#include "connection.h"

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if(db.isOpen())
        db.close();
}

Connection& Connection::getInstance()
{
    static Connection instance;   // Singleton
    return instance;
}

bool Connection::createconnect()
{
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("Maram");
    db.setPassword("esprit18");

    if(db.open())
    {
        qDebug() << "Connexion établie";
        return true;
    }
    else
    {
        qDebug() << "Erreur :" << db.lastError().text();
        return false;
    }
}
