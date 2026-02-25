#include "depot.h"
#include <QSqlError>
#include <QDebug>

Depot::Depot() {}

Depot::Depot(QString et, double capMax, double qteActuelle, QString typeStock)
{
    etagere = et;
    capacite_max = capMax;
    quantite_actuelle = qteActuelle;
    type_stockage = typeStock;
}

// =============================================
// AJOUTER
// =============================================
bool Depot::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO DEPOTS "
                  "(ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE) "
                  "VALUES (SEQ_DEP.NEXTVAL, :et, :cap, :qte, :type)");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":type", type_stockage);

    if (!query.exec()) {
        qDebug() << "Erreur Ajout Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// MODIFIER
// =============================================
bool Depot::modifier(int id)
{
    QSqlQuery query;
    query.prepare("UPDATE DEPOTS SET "
                  "ETAGERE = :et, "
                  "CAPACITE_MAX = :cap, "
                  "QUANTITE_ACTUELLE = :qte, "
                  "TYPE_STOCKAGE = :type "
                  "WHERE ID_EMPLACEMENT = :id");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":type", type_stockage);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Modification Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// SUPPRIMER
// =============================================
bool Depot::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM DEPOTS WHERE ID_EMPLACEMENT = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// AFFICHER
// =============================================
QSqlQueryModel* Depot::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ID_EMPLACEMENT DESC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage Depots :" << model->lastError().text();
    }
    return model;
}

// =============================================
// RECHERCHER
// =============================================
QSqlQueryModel* Depot::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE "
                  "FROM DEPOTS "
                  "WHERE UPPER(ETAGERE) LIKE UPPER(:c1) "
                  "OR UPPER(TYPE_STOCKAGE) LIKE UPPER(:c2) "
                  "ORDER BY ID_EMPLACEMENT DESC");

    QString pattern = "%" + critere + "%";
    query.bindValue(":c1", pattern);
    query.bindValue(":c2", pattern);
    query.exec();

    model->setQuery(std::move(query));
    return model;
}

// =============================================
// TRI
// =============================================
QSqlQueryModel* Depot::trierParEtagere()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ETAGERE ASC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Tri Depots :" << model->lastError().text();
    }
    return model;
}