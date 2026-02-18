#ifndef ORDREFABRICATION_H
#define ORDREFABRICATION_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class OrdreFabrication
{
    QString id_produit;
    int quantite;
    QString id_matiere;
    QDate date_lancement;
    QDate date_fin_prevue;
    QString statut;

public:
    // Constructeurs
    OrdreFabrication();
    OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut);

    // CRUD
    bool ajouter();
    QSqlQueryModel * afficher();
    bool supprimer(int id);
    bool modifier(int id); // <--- AJOUTEZ CECI
};

#endif // ORDREFABRICATION_H
