#include "connexion.h"
#include <QSqlError>
#include <QDebug>

Connexion::Connexion() {}

bool Connexion::createconnect()
{
    // On utilise QODBC car QOCI est introuvable
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    // --- CHAÎNE DE CONNEXION ---
    // Syntaxe standard pour Oracle XE
    // Si vous avez une erreur "Data source name not found", c'est ici qu'on regardera.
    QString connectionString = "Driver={Oracle in XE};Dbq=//localhost:1521/xe;Uid=PROJET_CPP;Pwd=1234;";

    db.setDatabaseName(connectionString);

    if (db.open()) {
        qDebug() << "--- CONNEXION REUSSIE (ODBC) ---";
        return true;
    } else {
        qDebug() << "--- ECHEC CONNEXION ODBC ---";
        qDebug() << "Erreur :" << db.lastError().text();
        return false;
    }
}

void Connexion::closeConnection() { QSqlDatabase::database().close(); }
