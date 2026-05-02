#include "etape.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

Etape::Etape() {}

Etape::Etape(int idPlanif, int idEmp, QString etape, double tempsReel, double d, int alerte)
{
    id_planification = idPlanif;
    id_employe = idEmp;
    etape_actuelle = etape;
    temps_reel_passe = tempsReel;
    delta = d;
    alerte_active = alerte;
}

// =============================================
// AJOUTER UNE ÉTAPE
// =============================================
bool Etape::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO ETAPES "
                  "(ID_SUIVI, ID_PLANIFICATION, ID_EMPLOYE, ETAPE_ACTUELLE, TEMPS_REEL_PASSE, DELTA, ALERTE_ACTIVE) "
                  "VALUES (SEQ_ETAPE.NEXTVAL, :idp, :ide, :etape, :temps, :delta, :alerte)");

    query.bindValue(":idp", id_planification);
    query.bindValue(":ide", id_employe);
    query.bindValue(":etape", etape_actuelle);
    query.bindValue(":temps", temps_reel_passe);
    query.bindValue(":delta", delta);
    query.bindValue(":alerte", alerte_active);

    if (!query.exec()) {
        qDebug() << "Erreur Ajout Étape :" << query.lastError().text();
        return false;
    }

    QSqlQuery().exec("COMMIT");
    return true;
}

// =============================================
// MODIFIER UNE ÉTAPE
// =============================================
bool Etape::modifier(int idSuivi)
{
    QSqlQuery query;
    query.prepare("UPDATE ETAPES SET "
                  "ID_PLANIFICATION = :idp, "
                  "ID_EMPLOYE = :ide, "
                  "ETAPE_ACTUELLE = :etape, "
                  "TEMPS_REEL_PASSE = :temps, "
                  "DELTA = :delta, "
                  "ALERTE_ACTIVE = :alerte "
                  "WHERE ID_SUIVI = :id");

    query.bindValue(":idp", id_planification);
    query.bindValue(":ide", id_employe);
    query.bindValue(":etape", etape_actuelle);
    query.bindValue(":temps", temps_reel_passe);
    query.bindValue(":delta", delta);
    query.bindValue(":alerte", alerte_active);
    query.bindValue(":id", idSuivi);

    if (!query.exec()) {
        qDebug() << "Erreur Modification Étape :" << query.lastError().text();
        return false;
    }

    QSqlQuery().exec("COMMIT");
    return true;
}

// =============================================
// SUPPRIMER UNE ÉTAPE
// =============================================
bool Etape::supprimer(int idSuivi)
{
    QSqlQuery query;
    query.prepare("DELETE FROM ETAPES WHERE ID_SUIVI = :id");
    query.bindValue(":id", idSuivi);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Étape :" << query.lastError().text();
        return false;
    }

    QSqlQuery().exec("COMMIT");
    return true;
}

// =============================================
// AFFICHER TOUTES LES ÉTAPES (avec jointures)
// =============================================
QSqlQueryModel* Etape::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT e.ID_SUIVI, e.ID_PLANIFICATION, "
        "pr.DESIGNATION AS PRODUIT, "
        "emp.NOM || ' ' || emp.PRENOM AS EMPLOYE, "
        "e.ETAPE_ACTUELLE, "
        "e.TEMPS_REEL_PASSE, "
        "e.DELTA, "
        "e.ALERTE_ACTIVE "
        "FROM ETAPES e "
        "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "LEFT JOIN EMPLOYES emp ON e.ID_EMPLOYE = emp.ID_EMPLOYE "
        "ORDER BY e.ID_PLANIFICATION, "
        "CASE e.ETAPE_ACTUELLE "
        "  WHEN 'Coupe' THEN 1 "
        "  WHEN 'Assemblage' THEN 2 "
        "  WHEN 'Couture' THEN 3 "
        "  WHEN 'Finition' THEN 4 "
        "  ELSE 5 END"
        );

    if (model->lastError().isValid())
        qDebug() << "Erreur Affichage Étapes :" << model->lastError().text();

    return model;
}

// =============================================
// GÉNÉRER AUTOMATIQUEMENT LES 4 ÉTAPES
// pour une commande planifiée
// =============================================
bool Etape::genererEtapesCommande(int idPlanification, int idEmploye)
{
    QStringList etapes = {"Coupe", "Assemblage", "Couture", "Finition"};

    // Vérifier si les étapes existent déjà pour cette commande
    QSqlQuery qCheck;
    qCheck.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id");
    qCheck.bindValue(":id", idPlanification);
    if (qCheck.exec() && qCheck.next()) {
        if (qCheck.value(0).toInt() > 0) {
            qDebug() << "⚠️ Étapes déjà générées pour la commande" << idPlanification;
            return false; // Déjà générées
        }
    }

    for (const QString &etape : etapes) {
        QSqlQuery query;
        query.prepare("INSERT INTO ETAPES "
                      "(ID_SUIVI, ID_PLANIFICATION, ID_EMPLOYE, ETAPE_ACTUELLE, "
                      "TEMPS_REEL_PASSE, DELTA, ALERTE_ACTIVE) "
                      "VALUES (SEQ_ETAPE.NEXTVAL, :idp, :ide, :etape, 0, 0, 0)");

        query.bindValue(":idp", idPlanification);
        query.bindValue(":ide", idEmploye);
        query.bindValue(":etape", etape);

        if (!query.exec()) {
            qDebug() << "Erreur génération étape" << etape << ":" << query.lastError().text();
            return false;
        }
    }

    QSqlQuery().exec("COMMIT");
    qDebug() << "✅ 4 étapes générées pour commande" << idPlanification;
    return true;
}

// =============================================
// GÉNÉRER LES 4 ÉTAPES POUR UNE COMMANDE (MULTI-EMPLOYÉS)
// =============================================
bool Etape::genererEtapesCommandeMulti(int idPlanification,
                                       const QVector<int> &idEmployesFallback,
                                       const QHash<QString, QVector<int>> *manuelParEtape)
{
    QStringList etapes = {"Coupe", "Assemblage", "Couture", "Finition"};

    // Vérifier si les étapes existent déjà pour cette commande
    QSqlQuery qCheck;
    qCheck.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id");
    qCheck.bindValue(":id", idPlanification);
    if (qCheck.exec() && qCheck.next()) {
        if (qCheck.value(0).toInt() > 0) {
            qDebug() << "⚠️ Étapes déjà générées pour la commande" << idPlanification;
            return false; // Déjà générées
        }
    }

    int fallbackIdx = 0;
    for (const QString &etape : etapes) {
        int idEmploye = -1;

        // Chercher un employé pour cette étape dans le hash manuel
        if (manuelParEtape && manuelParEtape->contains(etape)) {
            const QVector<int> &employes = manuelParEtape->value(etape);
            if (!employes.isEmpty()) {
                idEmploye = employes.first();
            }
        }

        // Fallback si pas d'employé assigné à cette étape
        if (idEmploye <= 0 && fallbackIdx < idEmployesFallback.size()) {
            idEmploye = idEmployesFallback.at(fallbackIdx);
            fallbackIdx++;
        }

        // Si pas d'employé du tout, passer cette étape
        if (idEmploye <= 0) {
            qDebug() << "⚠️ Pas d'employé assigné pour l'étape" << etape;
            continue;
        }

        QSqlQuery query;
        query.prepare("INSERT INTO ETAPES "
                      "(ID_SUIVI, ID_PLANIFICATION, ID_EMPLOYE, ETAPE_ACTUELLE, "
                      "TEMPS_REEL_PASSE, DELTA, ALERTE_ACTIVE) "
                      "VALUES (SEQ_ETAPE.NEXTVAL, :idp, :ide, :etape, 0, 0, 0)");

        query.bindValue(":idp", idPlanification);
        query.bindValue(":ide", idEmploye);
        query.bindValue(":etape", etape);

        if (!query.exec()) {
            qDebug() << "Erreur génération étape" << etape << ":" << query.lastError().text();
            return false;
        }
    }

    QSqlQuery().exec("COMMIT");
    qDebug() << "✅ 4 étapes multi-employés générées pour commande" << idPlanification;
    return true;
}

// =============================================
// SAISIR TEMPS RÉEL + CALCUL DELTA + ALERTE
// =============================================
bool Etape::saisirTempsReel(int idSuivi, double tempsReel, double tempsPrevue)
{
    double delta = tempsReel - tempsPrevue;
    int alerte = (delta > 0) ? 1 : 0; // Alerte si temps réel > prévu

    QSqlQuery query;
    query.prepare("UPDATE ETAPES SET "
                  "TEMPS_REEL_PASSE = :temps, "
                  "DELTA = :delta, "
                  "ALERTE_ACTIVE = :alerte "
                  "WHERE ID_SUIVI = :id");

    query.bindValue(":temps", tempsReel);
    query.bindValue(":delta", delta);
    query.bindValue(":alerte", alerte);
    query.bindValue(":id", idSuivi);

    if (!query.exec()) {
        qDebug() << "Erreur saisie temps :" << query.lastError().text();
        return false;
    }

    QSqlQuery().exec("COMMIT");
    qDebug() << "✅ Temps saisi:" << tempsReel << "h | Delta:" << delta << "h | Alerte:" << alerte;
    return true;
}

// =============================================
// RECHERCHER PAR COMMANDE
// =============================================
QSqlQueryModel* Etape::rechercherParCommande(int idPlanification)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare(
        "SELECT e.ID_SUIVI, e.ID_PLANIFICATION, "
        "pr.DESIGNATION AS PRODUIT, "
        "emp.NOM || ' ' || emp.PRENOM AS EMPLOYE, "
        "e.ETAPE_ACTUELLE, e.TEMPS_REEL_PASSE, e.DELTA, e.ALERTE_ACTIVE "
        "FROM ETAPES e "
        "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "LEFT JOIN EMPLOYES emp ON e.ID_EMPLOYE = emp.ID_EMPLOYE "
        "WHERE e.ID_PLANIFICATION = :id "
        "ORDER BY CASE e.ETAPE_ACTUELLE "
        "  WHEN 'Coupe' THEN 1 WHEN 'Assemblage' THEN 2 "
        "  WHEN 'Couture' THEN 3 WHEN 'Finition' THEN 4 ELSE 5 END"
        );
    query.bindValue(":id", idPlanification);
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

// =============================================
// RECHERCHER PAR ÉTAPE
// =============================================
QSqlQueryModel* Etape::rechercherParEtape(const QString &etape)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare(
        "SELECT e.ID_SUIVI, e.ID_PLANIFICATION, "
        "pr.DESIGNATION AS PRODUIT, "
        "emp.NOM || ' ' || emp.PRENOM AS EMPLOYE, "
        "e.ETAPE_ACTUELLE, e.TEMPS_REEL_PASSE, e.DELTA, e.ALERTE_ACTIVE "
        "FROM ETAPES e "
        "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "LEFT JOIN EMPLOYES emp ON e.ID_EMPLOYE = emp.ID_EMPLOYE "
        "WHERE UPPER(e.ETAPE_ACTUELLE) LIKE UPPER(:et) "
        "ORDER BY e.ID_PLANIFICATION"
        );
    query.bindValue(":et", "%" + etape + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

// =============================================
// TRIER PAR COMMANDE
// =============================================
QSqlQueryModel* Etape::trierParCommande()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT e.ID_SUIVI, e.ID_PLANIFICATION, "
        "pr.DESIGNATION AS PRODUIT, "
        "emp.NOM || ' ' || emp.PRENOM AS EMPLOYE, "
        "e.ETAPE_ACTUELLE, e.TEMPS_REEL_PASSE, e.DELTA, e.ALERTE_ACTIVE "
        "FROM ETAPES e "
        "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "LEFT JOIN EMPLOYES emp ON e.ID_EMPLOYE = emp.ID_EMPLOYE "
        "ORDER BY e.ID_PLANIFICATION ASC, "
        "CASE e.ETAPE_ACTUELLE "
        "  WHEN 'Coupe' THEN 1 WHEN 'Assemblage' THEN 2 "
        "  WHEN 'Couture' THEN 3 WHEN 'Finition' THEN 4 ELSE 5 END"
        );
    return model;
}

// =============================================
// AFFICHER ALERTES SEULEMENT
// =============================================
QSqlQueryModel* Etape::afficherAlertes()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT e.ID_SUIVI, e.ID_PLANIFICATION, "
        "pr.DESIGNATION AS PRODUIT, "
        "emp.NOM || ' ' || emp.PRENOM AS EMPLOYE, "
        "e.ETAPE_ACTUELLE, e.TEMPS_REEL_PASSE, e.DELTA, e.ALERTE_ACTIVE "
        "FROM ETAPES e "
        "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "LEFT JOIN EMPLOYES emp ON e.ID_EMPLOYE = emp.ID_EMPLOYE "
        "WHERE e.ALERTE_ACTIVE = 1 "
        "ORDER BY e.DELTA DESC"
        );
    return model;
}
