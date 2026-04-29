#include "ordrefabrication.h"
#include <QDebug>
#include <QSqlError>

QString OrdreFabrication::messageSiSaisieInvalide(const QString &idProduit,
                                                  const QString &idMatiere,
                                                  const QString &idEmploye,
                                                  int quantite,
                                                  const QDate &dateLancement,
                                                  const QDate &dateFinPrevue)
{
    bool okP = false, okM = false, okE = false;
    const int p = idProduit.trimmed().toInt(&okP);
    const int m = idMatiere.trimmed().toInt(&okM);
    const int e = idEmploye.trimmed().toInt(&okE);

    if (!okP || p <= 0)
        return QStringLiteral("Sélectionnez un produit.");
    if (!okM || m <= 0)
        return QStringLiteral("Sélectionnez une matière première.");
    if (!okE || e <= 0)
        return QStringLiteral("Sélectionnez un employé responsable.");
    if (quantite < 1)
        return QStringLiteral("La quantité doit être au moins 1.");
    if (quantite > 10000)
        return QStringLiteral("La quantité dépasse la limite autorisée (10 000).");
    if (!dateLancement.isValid())
        return QStringLiteral("Date de lancement invalide.");
    if (!dateFinPrevue.isValid())
        return QStringLiteral("Date de fin prévue invalide.");
    if (dateFinPrevue < dateLancement)
        return QStringLiteral("La date de fin doit être le même jour ou après la date de lancement.");
    return {};
}

OrdreFabrication::OrdreFabrication() {}

OrdreFabrication::OrdreFabrication(QString id_produit, int quantite, QString id_matiere, QDate date_lancement, QDate date_fin_prevue, QString statut, QString id_employe)
{
    this->id_produit = id_produit;
    this->quantite = quantite;
    this->id_matiere = id_matiere;
    this->date_lancement = date_lancement;
    this->date_fin_prevue = date_fin_prevue;
    this->statut = statut;
    this->id_employe = id_employe;
}
bool OrdreFabrication::ajouter() {
    m_derniereErreurSaisie.clear();
    m_derniereErreurSaisie = messageSiSaisieInvalide(id_produit, id_matiere, id_employe,
                                                     quantite, date_lancement, date_fin_prevue);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

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
    m_derniereErreurSaisie.clear();
    m_derniereErreurSaisie = messageSiSaisieInvalide(id_produit, id_matiere, id_employe,
                                                     quantite, date_lancement, date_fin_prevue);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    QSqlQuery query;
    query.prepare("UPDATE PLANIFICATION SET ID_PRODUIT = :prod, QUANTITE = :qte, ID_STOCK_MP = :mat, DATE_LANCEMENT = :deb, DATE_FIN_PREVUE = :fin, STATUT = :stat, ID_EMPLOYE = :emp WHERE ID_COMMANDE = :id");
    query.bindValue(":prod", id_produit.toInt());
    query.bindValue(":qte", quantite);
    query.bindValue(":mat", id_matiere.toInt());
    query.bindValue(":deb", date_lancement);
    query.bindValue(":fin", date_fin_prevue);
    query.bindValue(":stat", statut);
    query.bindValue(":emp", id_employe.toInt());
    query.bindValue(":id", id);
    return query.exec();
}


QSqlQueryModel * OrdreFabrication::afficher()
{
    QSqlQueryModel * model = new QSqlQueryModel();
    // Jointure pour récupérer les noms au lieu des ID numériques
    model->setQuery("SELECT p.ID_COMMANDE, pr.DESIGNATION AS PRODUIT, p.QUANTITE, m.CODE_MP AS MATIERE, "
                    "p.DATE_LANCEMENT, p.DATE_FIN_PREVUE, p.STATUT, (e.NOM || ' ' || e.PRENOM) AS EMPLOYE "
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

