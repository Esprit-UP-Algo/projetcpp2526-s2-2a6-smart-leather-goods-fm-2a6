#ifndef DEPOT_H
#define DEPOT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Depot
{
    QString m_emplacementSaisi;
    QString m_etagereSaisi;
    QString etagere; ///< fusion (reprise en base dans ETAGERE)
    double capacite_max = 0.0;
    double quantite_actuelle = 0.0;
    double valeur_gaz = 0.0;
    QString type_stockage;
    QString m_derniereErreurSaisie;

public:
    Depot();
    /// \a emplacement et \a etagere sont fusionnés en base dans la colonne ETAGERE (ex. « Zone A — E1 »).
    Depot(QString emplacement, QString etagere, double capMax, double qteActuelle, double valeurGaz, QString typeStock);

    static QString messageSiSaisieInvalide(const QString &emplacement,
                                          const QString &etagere,
                                          double capaciteMax,
                                          double quantiteActuelle,
                                          double valeurGaz,
                                          const QString &typeStockage);
    /// À l'ajout : l'ID est généré par la séquence — le champ doit rester vide.
    static QString messageIdAjoutNeDoitPasEtreSaisi(const QString &idTexteBrut);
    /// À la modification : entier strictement positif (Oracle NUMBER).
    static QString messageIdModificationTexteInvalide(const QString &idTexteBrut);
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
