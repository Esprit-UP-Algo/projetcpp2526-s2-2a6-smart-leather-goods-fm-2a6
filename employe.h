#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QString>
#include <QDate>
#include <QSqlQueryModel>

class employe
{
    private:
    int id_employe;
    QString nom;
    QString prenom;
    QString poste;
    QString email;
    QString telephone;
    QString departement;
    QDate date_embauche;
    double salaire;
    QString rfid_tag;
public:
    employe();
    employe(int id_employe,QString nom,QString prenom,QString poste, QString email,
            QString telephone,
            QString departement,
            QDate date_embauche,
            double salaire,
            QString rfid_tag);

    // CRUD
    bool ajouter();
    QSqlQueryModel * afficher();
    bool supprimer(int id);
    bool modifier(int id);
};

#endif // EMPLOYE_H
