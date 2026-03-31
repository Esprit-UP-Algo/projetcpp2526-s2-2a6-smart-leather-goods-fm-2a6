#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Produit
{
    int id_produit;              // NUMBER (Not Null)
    QString designation;         // VARCHAR2(100)
    double cout;                 // NUMBER(10,2)
    QString collection;          // VARCHAR2(50)
    QString type_cuir_requis;    // VARCHAR2(50)
    int temps_fabrication;       // NUMBER
    int id_client;               // NUMBER
    int id_emplacement;          // NUMBER

public:
    Produit();

    Produit(int id_produit,
            QString designation,
            double cout,
            QString collection,
            QString type_cuir_requis,
            int temps_fabrication,
            int id_client,
            int id_emplacement);

    // CRUD
    bool ajouter();
    QSqlQueryModel * afficher();
    /// Supprime d'abord les lignes PLANIFICATION liées, puis le produit. \a messageErreur reçoit le texte SQL si échec.
    bool supprimer(int id, QString *messageErreur = nullptr);
    bool modifier(int id);
};

#endif // PRODUIT_H
