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
                      "UID=Nafissatou;"
                      "PWD=esprit18;"
                      "SERVER=localhost;"
                      "PORT=1521;";

    db.setDatabaseName(connStr);
    db.setUserName("");
    db.setPassword("");

    qDebug() << "Connection string:" << connStr;

    if (db.open()) {
        qDebug() << "✅ SUCCÈS!";
        // Créer la table après connexion
        createClientTable();
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

// CORRECTION: Le nom de la classe est Connexion (avec x), pas Connection
bool Connexion::createClientTable()
{
    QSqlQuery query;

    // Pour Oracle - utiliser ID_CLIENT comme nom de colonne
    QString createTable = "CREATE TABLE client ("
                          "id_client NUMBER PRIMARY KEY, "  // ← ID_CLIENT au lieu de id
                          "cin VARCHAR2(50) UNIQUE NOT NULL, "
                          "nom VARCHAR2(100) NOT NULL, "
                          "tel VARCHAR2(20), "
                          "email VARCHAR2(100), "
                          "adresse VARCHAR2(200))";

    QSqlQuery checkQuery;
    checkQuery.exec("SELECT COUNT(*) FROM user_tables WHERE table_name = 'CLIENT'");

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "Table CLIENT existe déjà";
        return true;
    }

    if (!query.exec(createTable)) {
        qDebug() << "Erreur création table client: " << query.lastError().text();
        return false;
    }

    // Créer une séquence pour l'auto-incrémentation
    QString createSequence = "CREATE SEQUENCE client_seq START WITH 1 INCREMENT BY 1";
    if (!query.exec(createSequence)) {
        qDebug() << "Erreur création séquence: " << query.lastError().text();
    }

    // Créer un trigger pour auto-incrémenter l'ID_CLIENT
    QString createTrigger = "CREATE OR REPLACE TRIGGER client_trigger "
                            "BEFORE INSERT ON client "
                            "FOR EACH ROW "
                            "BEGIN "
                            "    SELECT client_seq.NEXTVAL INTO :NEW.id_client FROM dual; "  // ← id_client
                            "END;";
    if (!query.exec(createTrigger)) {
        qDebug() << "Erreur création trigger: " << query.lastError().text();
    }

    qDebug() << "Table CLIENT créée avec succès";
    return true;
}
