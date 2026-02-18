#include "connexion.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

Connexion::Connexion() {}

bool Connexion::createconnect()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    // ✅ فقط DSN هنا
    db.setDatabaseName("projet2a6");
    db.setUserName("system");
    db.setPassword("ksouriksouri");

    if(db.open())
    {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Erreur :" << db.lastError().text();
    return false;
}
