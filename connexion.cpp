#include "connexion.h"

Connexion* Connexion::instance = nullptr;

Connexion::Connexion() {
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("Projet_cpp");
    db.setPassword("1234");
}

Connexion::~Connexion() {
    fermer();
}

Connexion* Connexion::getInstance() {
    if (instance == nullptr) {
        instance = new Connexion();
    }
    return instance;
}

bool Connexion::ouvrir() {
    if (db.isOpen()) {
        return true;
    }

    if (db.open()) {
        qDebug() << "✅ Connexion Oracle via ODBC réussie (Singleton)";
        return true;
    } else {
        qDebug() << "❌ Échec connexion Oracle :" << db.lastError().text();
        return false;
    }
}

void Connexion::fermer() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "🔒 Connexion Oracle fermée.";
    }
}

bool Connexion::estConnecte() {
    return db.isOpen();
}

QSqlDatabase Connexion::getDatabase() {
    return db;
}
