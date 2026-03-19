#include "ordrefabrication.h"
#include <QDebug>
#include <QSqlError>

OrdreFabrication::OrdreFabrication() {}

OrdreFabrication::OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut, QString id_employe)
{
    this->id_produit = id_produit; this->quantite = quantite; this->id_matiere = id_matiere;
    this->date_lancement = date_lancement; this->date_fin_prevue = date_fin_prevue;
    this->statut = statut; this->id_employe = id_employe;
}
bool OrdreFabrication::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO PLANIFICATION (ID_COMMANDE, ID_PRODUIT, QUANTITE, ID_STOCK_MP, DATE_LANCEMENT, DATE_FIN_PREVUE, STATUT, ID_EMPLOYE) "
                  "VALUES (SEQ_PLAN.NEXTVAL, :prod, :qte, :mat, :deb, :fin, :stat, :emp)");

    // On convertit les QString en Int car la BDD attend des nombres
    query.bindValue(":prod", id_produit.toInt());
    query.bindValue(":qte", quantite);
    query.bindValue(":mat", id_matiere.toInt());
    query.bindValue(":deb", date_lancement);
    query.bindValue(":fin", date_fin_prevue);
    query.bindValue(":stat", statut);
    query.bindValue(":emp", id_employe.toInt());

    return query.exec();
}

bool OrdreFabrication::modifier(int id) {
    QSqlQuery query;
    query.prepare("UPDATE PLANIFICATION SET ID_PRODUIT = :prod, QUANTITE = :qte, ID_STOCK_MP = :mat, DATE_LANCEMENT = :deb, DATE_FIN_PREVUE = :fin, ID_EMPLOYE = :emp WHERE ID_COMMANDE = :id");
    query.bindValue(":prod", id_produit.toInt());
    query.bindValue(":qte", quantite);
    query.bindValue(":mat", id_matiere.toInt());
    query.bindValue(":deb", date_lancement);
    query.bindValue(":fin", date_fin_prevue);
    query.bindValue(":emp", id_employe.toInt());
    query.bindValue(":id", id);
    return query.exec();
}


QSqlQueryModel * OrdreFabrication::afficher()
{
    QSqlQueryModel * model = new QSqlQueryModel();
    qDebug() << "DEFAULT conn name =" << QSqlDatabase::database().connectionName();
    qDebug() << "DEFAULT driver =" << QSqlDatabase::database().driverName();
    qDebug() << "ALL connections =" << QSqlDatabase::connectionNames();
    // Jointure pour récupérer les noms au lieu des ID numériques
    model->setQuery("SELECT p.ID_COMMANDE, pr.DESIGNATION, p.QUANTITE, m.CODE_MP, "
                    "p.DATE_LANCEMENT, p.DATE_FIN_PREVUE, p.STATUT, e.NOM "
                    "FROM PLANIFICATION p "
                    "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
                    "LEFT JOIN MATIERES_PREMIERES m ON p.ID_STOCK_MP = m.ID_STOCK_MP "
                    "LEFT JOIN EMPLOYES e ON p.ID_EMPLOYE = e.ID_EMPLOYE "
                    "ORDER BY p.ID_COMMANDE DESC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage Planif :" << model->lastError().text();
    }
    return model;
}
bool OrdreFabrication::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM PLANIFICATION WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id);
    return query.exec();
}

