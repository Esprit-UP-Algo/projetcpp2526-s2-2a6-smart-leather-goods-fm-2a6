#include "connexion.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Connexion* Connexion::instance = nullptr;

Connexion::Connexion()
{
    qDebug() << "\n=== DIAGNOSTIC ODBC ===";

    qDebug() << "Drivers disponibles:";
    QStringList drivers = QSqlDatabase::drivers();
    foreach(QString driver, drivers) {
        qDebug() << "  -" << driver;
    }

    if (QSqlDatabase::isDriverAvailable("QODBC")) {
        db = QSqlDatabase::addDatabase("QODBC");
        qDebug() << "✅ Driver QODBC chargé";
    }
    qDebug() << "=====================\n";
}

Connexion* Connexion::getInstance()
{
    if (instance == nullptr) {
        instance = new Connexion();
    }
    return instance;
}

bool Connexion::etablirConnexion()
{
    qDebug() << "\n=== TEST DE CONNEXION ===";

    QString connStr = "DRIVER={Oracle in XE};"
                      "DBQ=XE;"
                      "UID=Nafissatou;"  // Vérifiez votre nom d'utilisateurrrrr
                      "PWD=esprit18;"
                      "SERVER=localhost;"
                      "PORT=1521;";

    db.setDatabaseName(connStr);
    db.setUserName("");
    db.setPassword("");

    qDebug() << "Connection string:" << connStr;

    if (db.open()) {
        qDebug() << "✅ SUCCÈS!";
        return true;
    } else {
        qDebug() << "❌ Échec:" << db.lastError().text();
        return false;
    }
}

void Connexion::fermerConnexion()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Connexion fermée";
    }
}

bool Connexion::estConnecte()
{
    return db.isOpen();
}

Connexion::~Connexion()
{
    fermerConnexion();
}
