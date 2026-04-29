#ifndef ORDREFABRICATION_H
#define ORDREFABRICATION_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class OrdreFabrication
{
    QString id_produit; // Attention, maintenant c'est un ID numérique dans Oracle, on le garde en QString pour Qt
    int quantite;
    QString id_matiere;
    QDate date_lancement;
    QDate date_fin_prevue;
    QString statut;
    QString id_employe; // NOUVEAU
    QString m_derniereErreurSaisie;

public:
    OrdreFabrication();
    OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut, QString id_employe);

    static QString messageSiSaisieInvalide(const QString &idProduit,
                                           const QString &idMatiere,
                                           const QString &idEmploye,
                                           int quantite,
                                           const QDate &dateLancement,
                                           const QDate &dateFinPrevue);
    QString derniereErreurSaisie() const { return m_derniereErreurSaisie; }

    // CRUD
    bool ajouter();
    QSqlQueryModel * afficher();
    bool supprimer(int id);
    bool modifier(int id);
};

#endif // ORDREFABRICATION_H
