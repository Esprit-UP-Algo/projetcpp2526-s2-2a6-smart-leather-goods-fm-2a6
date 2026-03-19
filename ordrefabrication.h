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

public:
    OrdreFabrication();
    // Ajoutez id_employe à la fin du constructeur :
    OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut, QString id_employe);

    // CRUD
    bool ajouter();
    QSqlQueryModel * afficher();
    bool supprimer(int id);
    bool modifier(int id); // <--- AJOUTEZ CECI
};

#endif // ORDREFABRICATION_H
