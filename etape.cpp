#include "etape.h"
#include <QSqlError>
#include <QDebug>

Etape::Etape(int id_plan, int id_emp, QString etape, double temps, double dlt, int alrt) {
    id_planification = id_plan;
    id_employe = id_emp;
    etape_actuelle = etape;
    temps_reel = temps;
    delta = dlt;
    alerte_active = alrt;
}

bool Etape::ajouter() {
    QSqlQuery query;
    // On enregistre l'étape dans Oracle selon votre diagramme
    query.prepare("INSERT INTO ETAPES (ID_SUIVI, ID_PLANIFICATION, ID_EMPLOYE, ETAPE_ACTUELLE, TEMPS_REEL_PASSE, DELTA, ALERTE_ACTIVE) "
                  "VALUES (SEQ_ETAPE.NEXTVAL, :idp, :ide, :eta, :tr, :dlt, :alrt)");

    query.bindValue(":idp", id_planification);
    query.bindValue(":ide", id_employe);
    query.bindValue(":eta", etape_actuelle);
    query.bindValue(":tr", temps_reel);
    query.bindValue(":dlt", delta);
    query.bindValue(":alrt", alerte_active);

    if(!query.exec()) {
        qDebug() << "Erreur Ajout Etape :" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Etape::afficherHistorique(int idPlanif) {
    QSqlQueryModel *model = new QSqlQueryModel();
    // On demande à Oracle de nous donner les étapes de CETTE commande précise
    // DECODE est une fonction Oracle super pratique pour transformer 1 en 'Retard' et 0 en 'OK'
    QString requete = QString("SELECT ETAPE_ACTUELLE as Etape, TEMPS_REEL_PASSE as Temps_Reel, "
                              "DELTA as Ecart, DECODE(ALERTE_ACTIVE, 1, 'Retard', 'OK') as Statut "
                              "FROM ETAPES WHERE ID_PLANIFICATION = %1 ORDER BY ID_SUIVI ASC").arg(idPlanif);

    model->setQuery(requete);
    return model;
}
