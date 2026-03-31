#include "produit.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMetaType>

Produit::Produit() {}

Produit::Produit(int id_produit,
                 QString designation,
                 double cout,
                 QString collection,
                 QString type_cuir_requis,
                 int temps_fabrication,
                 int id_client,
                 int id_emplacement)
{
    this->id_produit = id_produit;
    this->designation = designation;
    this->cout = cout;
    this->collection = collection;
    this->type_cuir_requis = type_cuir_requis;
    this->temps_fabrication = temps_fabrication;
    this->id_client = id_client;
    this->id_emplacement = id_emplacement;
}
bool Produit::ajouter()
{
    QSqlQuery query;

    query.prepare(
        "INSERT INTO PRODUITS "
        "(ID_PRODUIT, DESIGNATION, COUT, COLLECTION, TYPE_CUIR_REQUIS, TEMPS_FABRICATION, ID_CLIENT, ID_EMPLACEMENT) "
        "VALUES (SEQ_PRODUIT.NEXTVAL, :des, :cout, :coll, :cuir, :temps, :client, :empl)"
        );

    query.bindValue(":des", designation);
    query.bindValue(":cout", cout);
    query.bindValue(":coll", collection);
    query.bindValue(":cuir", type_cuir_requis);
    query.bindValue(":temps", temps_fabrication);

    // NULL typed as NUMBER for ODBC
    if (id_client <= 0)
        query.bindValue(":client", QVariant(QMetaType::fromType<int>()));
    else
        query.bindValue(":client", id_client);

    if (id_emplacement <= 0)
        query.bindValue(":empl", QVariant(QMetaType::fromType<int>()));
    else
        query.bindValue(":empl", id_emplacement);

    if(!query.exec()){
        qDebug() << "ERROR INSERT PRODUIT:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Produit::modifier(int id)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE PRODUITS SET "
        "DESIGNATION = :des, "
        "COUT = :cout, "
        "COLLECTION = :coll, "
        "TYPE_CUIR_REQUIS = :cuir, "
        "TEMPS_FABRICATION = :temps "
        "WHERE ID_PRODUIT = :id"
        );

    query.bindValue(":des", designation);
    query.bindValue(":cout", cout);
    query.bindValue(":coll", collection);
    query.bindValue(":cuir", type_cuir_requis);
    query.bindValue(":temps", temps_fabrication);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "ERROR UPDATE PRODUIT:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Produit::supprimer(int id, QString *messageErreur)
{
    QSqlQuery query;

    query.prepare("DELETE FROM PLANIFICATION WHERE ID_PRODUIT = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        const QString err = query.lastError().text();
        qDebug() << "Erreur DELETE PLANIFICATION (produit):" << err;
        if (messageErreur)
            *messageErreur = err;
        return false;
    }

    query.prepare("DELETE FROM PRODUITS WHERE ID_PRODUIT = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        const QString err = query.lastError().text();
        qDebug() << "Erreur Supprimer Produit :" << err;
        if (messageErreur)
            *messageErreur = err;
        return false;
    }
    return true;
}
