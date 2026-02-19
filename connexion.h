#ifndef CONNEXION_H
#define CONNEXION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Connexion
{
private:
    static Connexion* instance;
    Connexion();
    Connexion(const Connexion&) = delete;
    Connexion& operator=(const Connexion&) = delete;
    QSqlDatabase db;

public:
    static Connexion* getInstance();
    bool etablirConnexion();
    void fermerConnexion();
    bool estConnecte();
    QSqlDatabase getDatabase() { return db; }
    ~Connexion();
};

#endif // CONNEXION_H
