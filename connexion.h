#ifndef CONNEXION_H
#define CONNEXION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QString>

class Connexion
{
private:
    QSqlDatabase db;

    // Constructeur PRIVÉ (Singleton)
    Connexion() {
        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName("Source_Projet2A");
        db.setUserName("Projet_cpp");
        db.setPassword("1234");
    }

    // Supprimer copie et affectation
    Connexion(const Connexion&) = delete;
    Connexion& operator=(const Connexion&) = delete;

    // Instance unique (statique)
    static Connexion* instance;

public:
    // Point d'accès global unique
    static Connexion* getInstance() {
        if (instance == nullptr) {
            instance = new Connexion();
        }
        return instance;
    }

    // Ouvrir la connexion
    bool ouvrir() {
        if (db.isOpen()) {
            return true; // Déjà connecté
        }

        if (db.open()) {
            qDebug() << "✅ Connexion Oracle réussie (Singleton)";
            return true;
        } else {
            qDebug() << "❌ Échec connexion Oracle :" << db.lastError().text();
            return false;
        }
    }

    // Fermer la connexion
    void fermer() {
        if (db.isOpen()) {
            db.close();
            qDebug() << "🔒 Connexion Oracle fermée.";
        }
    }

    // Vérifier si connecté
    bool estConnecte() {
        return db.isOpen();
    }

    // Accéder à la base
    QSqlDatabase getDatabase() {
        return db;
    }

    // Destructeur
    ~Connexion() {
        fermer();
    }
};

#endif // CONNEXION_H
