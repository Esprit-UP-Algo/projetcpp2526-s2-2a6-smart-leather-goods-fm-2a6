#ifndef MATIEREPREMIERE_H
#define MATIEREPREMIERE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class MatierePremiere
{
    QString code_mp;
    QString categorie_mp;
    QString num_lot;
    QString etat_mp;
    QString couleur;
    double quantite;
    QString type_stockage;
    QString qualite;

public:
    MatierePremiere();
    MatierePremiere(QString code, QString cat, QString lot, QString etat,
                    QString coul, double qte, QString typeStock, QString qual);

    // CRUD
    bool ajouter();
    bool modifier(int id);
    bool supprimer(int id);
    QSqlQueryModel* afficher();

    // Recherche et Tri
    QSqlQueryModel* rechercher(const QString &critere);
    QSqlQueryModel* trierParCode();
};

#endif // MATIEREPREMIERE_H
