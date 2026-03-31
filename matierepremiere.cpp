#include "matierepremiere.h"
#include <QSqlError>
#include <QDebug>

MatierePremiere::MatierePremiere() {}

MatierePremiere::MatierePremiere(QString code, QString cat, QString lot, QString etat,
                                 QString coul, double qte, QString typeStock, QString qual)
{
    code_mp = code;
    categorie_mp = cat;
    num_lot = lot;
    etat_mp = etat;
    couleur = coul;
    quantite = qte;
    type_stockage = typeStock;
    qualite = qual;
}

// =============================================
// AJOUTER
// =============================================
bool MatierePremiere::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO MATIERES_PREMIERES "
                  "(ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE) "
                  "VALUES (SEQ_MAT.NEXTVAL, :code, :cat, :lot, :etat, :coul, :qte, :type, :qual)");

    query.bindValue(":code", code_mp);
    query.bindValue(":cat", categorie_mp);
    query.bindValue(":lot", num_lot);
    query.bindValue(":etat", etat_mp);
    query.bindValue(":coul", couleur);
    query.bindValue(":qte", quantite);
    query.bindValue(":type", type_stockage);
    query.bindValue(":qual", qualite);

    if (!query.exec()) {
        qDebug() << "Erreur Ajout Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// MODIFIER
// =============================================
bool MatierePremiere::modifier(int id)
{
    QSqlQuery query;
    query.prepare("UPDATE MATIERES_PREMIERES SET "
                  "CODE_MP = :code, "
                  "CATEGORIE_MP = :cat, "
                  "NUM_LOT = :lot, "
                  "ETAT_MP = :etat, "
                  "COULEUR = :coul, "
                  "QUANTITE = :qte, "
                  "TYPE_STOCKAGE = :type, "
                  "QUALITE = :qual "
                  "WHERE ID_STOCK_MP = :id");

    query.bindValue(":code", code_mp);
    query.bindValue(":cat", categorie_mp);
    query.bindValue(":lot", num_lot);
    query.bindValue(":etat", etat_mp);
    query.bindValue(":coul", couleur);
    query.bindValue(":qte", quantite);
    query.bindValue(":type", type_stockage);
    query.bindValue(":qual", qualite);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Modification Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// SUPPRIMER
// =============================================
bool MatierePremiere::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM MATIERES_PREMIERES WHERE ID_STOCK_MP = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// AFFICHER
// =============================================
QSqlQueryModel* MatierePremiere::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                    "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                    "FROM MATIERES_PREMIERES "
                    "ORDER BY ID_STOCK_MP DESC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage Matières :" << model->lastError().text();
    }
    return model;
}

// =============================================
// RECHERCHER
// =============================================
QSqlQueryModel* MatierePremiere::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                  "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                  "FROM MATIERES_PREMIERES "
                  "WHERE UPPER(CODE_MP) LIKE UPPER(:c1) "
                  "OR UPPER(CATEGORIE_MP) LIKE UPPER(:c2) "
                  "OR UPPER(COULEUR) LIKE UPPER(:c3) "
                  "OR UPPER(NUM_LOT) LIKE UPPER(:c4) "
                  "ORDER BY ID_STOCK_MP DESC");

    QString pattern = "%" + critere + "%";
    query.bindValue(":c1", pattern);
    query.bindValue(":c2", pattern);
    query.bindValue(":c3", pattern);
    query.bindValue(":c4", pattern);
    query.exec();

    model->setQuery(std::move(query));
    return model;
}

// =============================================
// TRI ALPHABÉTIQUE
// =============================================
QSqlQueryModel* MatierePremiere::trierParCode()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                    "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                    "FROM MATIERES_PREMIERES "
                    "ORDER BY CODE_MP ASC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Tri Matières :" << model->lastError().text();
    }
    return model;
}
