#ifndef CONNEXION_H
#define CONNEXION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connexion {
public:
    Connexion();
    bool createconnect();
    void closeConnection();
};
#endif // CONNEXION_H
