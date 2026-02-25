#ifndef DEPOT_H
#define DEPOT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Depot
{
    QString etagere;
    double capacite_max;
    double quantite_actuelle;
    QString type_stockage;

public:
    Depot();
    Depot(QString etagere, double capMax, double qteActuelle, QString typeStock);

    // CRUD
    bool ajouter();
    bool modifier(int id);
    bool supprimer(int id);
    QSqlQueryModel* afficher();

    // Recherche & Tri
    QSqlQueryModel* rechercher(const QString &critere);
    QSqlQueryModel* trierParEtagere();
};

#endif // DEPOT_H