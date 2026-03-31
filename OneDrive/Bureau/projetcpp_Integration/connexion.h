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
    Connexion();

    // Supprimer copie et affectation
    Connexion(const Connexion&) = delete;
    Connexion& operator=(const Connexion&) = delete;

    // Instance unique (statique)
    static Connexion* instance;

public:
    // Point d'accès global unique
    static Connexion* getInstance();

    // Méthodes
    bool ouvrir();
    void fermer();
    bool estConnecte();
    QSqlDatabase getDatabase();

    // Destructeur
    ~Connexion();
};

#endif // CONNEXION_H
