#include "connexion.h"

// Initialiser le pointeur statique à nullptr
Connexion* Connexion::instance = nullptr;

// =============================================
// CONSTRUCTEUR PRIVÉ
// =============================================
Connexion::Connexion() {
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("Projet_cpp");
    db.setPassword("1234");
}

// =============================================
// DESTRUCTEUR
// =============================================
Connexion::~Connexion() {
    fermer();
}

// =============================================
// GET INSTANCE (Singleton)
// =============================================
Connexion* Connexion::getInstance() {
    if (instance == nullptr) {
        instance = new Connexion();
    }
    return instance;
}

// =============================================
// OUVRIR LA CONNEXION
// =============================================
bool Connexion::ouvrir() {
    if (db.isOpen()) {
        return true; // Déjà connecté
    }

    if (db.open()) {
        qDebug() << "✅ Connexion Oracle via ODBC réussie (Singleton)";
        return true;
    } else {
        qDebug() << "❌ Échec connexion Oracle :" << db.lastError().text();
        return false;
    }
}

// =============================================
// FERMER LA CONNEXION
// =============================================
void Connexion::fermer() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "🔒 Connexion Oracle fermée.";
    }
}

// =============================================
// VÉRIFIER SI CONNECTÉ
// =============================================
bool Connexion::estConnecte() {
    return db.isOpen();
}

// =============================================
// ACCÉDER À LA BASE
// =============================================
QSqlDatabase Connexion::getDatabase() {
    return db;
}
