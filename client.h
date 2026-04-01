#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Client
{
    int id_client = 0;
    QString nom;
    QString telephone;
    QString adresse;
    QString email;
    int points_fidelite = 0;
    QString m_derniereErreurSaisie;

public:
    Client();
    Client(int id, QString nom, QString tel, QString adr, QString mail, int pts);

    QString derniereErreurSaisie() const { return m_derniereErreurSaisie; }

    // CRUD
    bool ajouter();
    bool modifier(int id);
    bool supprimer(int id);
    QSqlQueryModel *afficher();

    // Recherche et Tri
    QSqlQueryModel *rechercher(const QString &critere);
    QSqlQueryModel *trierParNom();
};

#endif // CLIENT_H
