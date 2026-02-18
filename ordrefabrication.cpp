#include "ordrefabrication.h"
#include <QDebug>
#include <QSqlError>

OrdreFabrication::OrdreFabrication() {}

OrdreFabrication::OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut)
{
    this->id_produit = id_produit;
    this->quantite = quantite;
    this->id_matiere = id_matiere;
    this->date_lancement = date_lancement;
    this->date_fin_prevue = date_fin_prevue;
    this->statut = statut;
}

bool OrdreFabrication::ajouter()
{
    QSqlQuery query;

    // 1. Préparation de la requête selon VOTRE base de données "Groupe"
    // Notez bien les noms : REF_PRODUIT (texte) et ID_STOCK_MP (nombre)
    query.prepare("INSERT INTO ORDRE_FABRICATION (ID_COMMANDE, REF_PRODUIT, QUANTITE, ID_STOCK_MP, DATE_LANCEMENT, DATE_FIN_PREVUE, STATUT) "
                  "VALUES (SEQ_OF.NEXTVAL, :prod, :qte, :mat, :deb, :fin, :stat)");

    // 2. Liaison des valeurs
    query.bindValue(":prod", id_produit); // Doit exister dans la table PRODUITS !
    query.bindValue(":qte", quantite);

    // --- CORRECTION CRITIQUE ---
    // Votre base attend un ID (Nombre) pour la matière.
    // L'interface envoie un NOM. Pour l'instant, on force l'ID '1' pour tester.
    // (Plus tard, il faudra faire une requête SELECT pour trouver l'ID à partir du nom).
    int idMatiereForce = 1;
    query.bindValue(":mat", idMatiereForce);

    query.bindValue(":deb", date_lancement);
    query.bindValue(":fin", date_fin_prevue);
    query.bindValue(":stat", statut);

    // 3. Exécution et Debug
    if (query.exec()) {
        qDebug() << "Ajout réussi !";
        return true;
    } else {
        // C'est ICI que l'erreur s'affichera dans la console de Qt Creator
        qDebug() << "--------------------------------------------";
        qDebug() << "ERREUR SQL :" << query.lastError().text();
        qDebug() << "ERREUR ORACLE :" << query.lastError().databaseText();
        qDebug() << "--------------------------------------------";
        return false;
    }
}

QSqlQueryModel * OrdreFabrication::afficher()
{
    QSqlQueryModel * model = new QSqlQueryModel();

    // REQUÊTE AVEC JOINTURE (JOIN)
    // On sélectionne les infos de l'Ordre (o) ET le Code Matière (s.CODE_MP) de la table Stock (s)
    model->setQuery("SELECT o.ID_COMMANDE, o.REF_PRODUIT, o.QUANTITE, s.CODE_MP, o.DATE_LANCEMENT, o.DATE_FIN_PREVUE, o.STATUT "
                    "FROM ORDRE_FABRICATION o "
                    "LEFT JOIN STOCK_MATIERES s ON o.ID_STOCK_MP = s.ID_STOCK_MP "
                    "ORDER BY o.ID_COMMANDE DESC");

    // Vérification d'erreur
    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage (Join) :" << model->lastError().text();
    }

    return model;
}

bool OrdreFabrication::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM ORDRE_FABRICATION WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id);
    return query.exec();
}
bool OrdreFabrication::modifier(int id)
{
    QSqlQuery query;
    // Requête UPDATE standard
    query.prepare("UPDATE ORDRE_FABRICATION SET REF_PRODUIT = :prod, QUANTITE = :qte, "
                  "ID_STOCK_MP = :mat, DATE_LANCEMENT = :deb, DATE_FIN_PREVUE = :fin "
                  "WHERE ID_COMMANDE = :id");

    query.bindValue(":id", id);
    query.bindValue(":prod", id_produit);
    query.bindValue(":qte", quantite);

    // Note : Comme pour l'ajout, on force l'ID matière à 1 pour l'instant
    // car l'interface envoie du texte.
    query.bindValue(":mat", 1);

    query.bindValue(":deb", date_lancement);
    query.bindValue(":fin", date_fin_prevue);

    if(query.exec()) {
        return true;
    } else {
        qDebug() << "Erreur Update :" << query.lastError().text();
        return false;
    }
}
