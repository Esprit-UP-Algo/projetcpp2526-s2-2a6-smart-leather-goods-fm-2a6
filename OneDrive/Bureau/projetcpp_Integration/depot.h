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
    QString m_derniereErreurSaisie;

public:
    Depot();
    Depot(QString etagere, double capMax, double qteActuelle, QString typeStock);

    static QString messageSiSaisieInvalide(const QString &etagere,
                                          double capaciteMax,
                                          double quantiteActuelle);
    QString derniereErreurSaisie() const { return m_derniereErreurSaisie; }

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