#include "depot.h"
#include "connexion.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include <climits>
#include <cmath>
#include <utility>

static constexpr double kMaxUnitesDepotFormulaire = 100000.0; // aligné sur sb_depot_* (mainwindow.ui)

namespace {

QString fusionEmplacementEtagere(const QString &emplacement, const QString &etagere)
{
    const QString e = emplacement.trimmed();
    const QString t = etagere.trimmed();
    if (e.isEmpty())
        return t;
    if (t.isEmpty())
        return e;
    return e + QStringLiteral(" — ") + t;
}

QString normaliserTypeStockage(const QString &typeBrut)
{
    const QString t = typeBrut.trimmed();
    if (t.compare(QStringLiteral("Sec"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("Sec");
    if (t.compare(QStringLiteral("Froid"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("Froid");
    return t;
}

} // namespace

static QSqlDatabase depotDatabase()
{
    Connexion *cx = Connexion::getInstance();
    return cx ? cx->getDatabase() : QSqlDatabase();
}

QString Depot::messageIdAjoutNeDoitPasEtreSaisi(const QString &idTexteBrut)
{
    if (idTexteBrut.trimmed().isEmpty())
        return {};
    return QStringLiteral("L'identifiant est généré automatiquement : ne renseignez pas le champ « ID » à l'ajout.");
}

QString Depot::messageIdModificationTexteInvalide(const QString &idTexteBrut)
{
    const QString s = idTexteBrut.trimmed();
    if (s.isEmpty())
        return QStringLiteral("L'identifiant de l'emplacement est obligatoire pour la modification.");
    bool ok = false;
    const qlonglong v = s.toLongLong(&ok);
    if (!ok || v < 1 || v > INT_MAX)
        return QStringLiteral("L'identifiant doit être un entier strictement positif (maximum %1).").arg(INT_MAX);
    return {};
}

Depot::Depot() {}

Depot::Depot(QString emplacement, QString etagere, double capMax, double qteActuelle, QString typeStock)
{
    m_emplacementSaisi = emplacement.trimmed();
    m_etagereSaisi = etagere.trimmed();
    if (m_emplacementSaisi.isEmpty())
        this->etagere = m_etagereSaisi;
    else
        this->etagere = fusionEmplacementEtagere(m_emplacementSaisi, m_etagereSaisi);
    capacite_max = capMax;
    quantite_actuelle = qteActuelle;
    type_stockage = typeStock.trimmed();
}

QString Depot::messageSiSaisieInvalide(const QString &emplacement,
                                       const QString &etagere,
                                       double capaciteMax,
                                       double quantiteActuelle,
                                       const QString &typeStockage)
{
    const QString emp = emplacement.trimmed();
    const QString et = etagere.trimmed();

    if (emp.isEmpty() && et.isEmpty())
        return QStringLiteral("Renseignez l'emplacement et l'étagère (ou au minimum la description dans « Étagère » pour les fiches existantes).");

    if (!emp.isEmpty()) {
        if (emp.length() > 80)
            return QStringLiteral("L'emplacement ne doit pas dépasser 80 caractères.");
        if (et.isEmpty())
            return QStringLiteral("L'étagère est obligatoire lorsque l'emplacement est renseigné.");
        if (et.length() > 80)
            return QStringLiteral("L'étagère ne doit pas dépasser 80 caractères.");
        const QString fusion = fusionEmplacementEtagere(emp, et);
        if (fusion.length() > 100)
            return QStringLiteral("Emplacement + étagère : au total 100 caractères maximum (limite base de données).");
    } else {
        if (et.length() > 100)
            return QStringLiteral("La description (étagère / emplacement) ne doit pas dépasser 100 caractères.");
    }

    const QString typ = typeStockage.trimmed();
    if (typ.compare(QStringLiteral("Sec"), Qt::CaseInsensitive) != 0
        && typ.compare(QStringLiteral("Froid"), Qt::CaseInsensitive) != 0) {
        return QStringLiteral("Le type de stockage doit être « Sec » ou « Froid ».");
    }

    if (!std::isfinite(capaciteMax) || !std::isfinite(quantiteActuelle))
        return QStringLiteral("Capacité ou quantité : valeur numérique invalide (infini ou non numérique).");

    if (capaciteMax > kMaxUnitesDepotFormulaire + 1e-6)
        return QStringLiteral("La capacité maximale ne peut pas dépasser 100 000 (limite du formulaire).");

    if (quantiteActuelle > kMaxUnitesDepotFormulaire + 1e-6)
        return QStringLiteral("La quantité actuelle ne peut pas dépasser 100 000 (limite du formulaire).");

    if (capaciteMax <= 0.0)
        return QStringLiteral("La capacité maximale doit être strictement positive.");

    if (quantiteActuelle < 0.0)
        return QStringLiteral("La quantité actuelle ne peut pas être négative.");

    if (quantiteActuelle > capaciteMax + 1e-6)
        return QStringLiteral("La quantité actuelle ne peut pas dépasser la capacité maximale.");

    return {};
}

// =============================================
// AJOUTER
// =============================================
bool Depot::ajouter()
{
    m_derniereErreurSaisie.clear();
    if (m_emplacementSaisi.isEmpty())
        this->etagere = m_etagereSaisi;
    else
        this->etagere = fusionEmplacementEtagere(m_emplacementSaisi, m_etagereSaisi);
    m_derniereErreurSaisie = messageSiSaisieInvalide(
        m_emplacementSaisi,
        m_etagereSaisi,
        capacite_max,
        quantite_actuelle,
        type_stockage);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    const QString typeOk = normaliserTypeStockage(type_stockage);

    QSqlDatabase db = depotDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO DEPOTS "
                  "(ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE) "
                  "VALUES (SEQ_DEP.NEXTVAL, :et, :cap, :qte, :type)");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":type", typeOk);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "Erreur Ajout Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery(db);
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// MODIFIER
// =============================================
bool Depot::modifier(int id)
{
    m_derniereErreurSaisie.clear();
    if (id <= 0) {
        m_derniereErreurSaisie = QStringLiteral("L'identifiant de l'emplacement doit être un entier strictement positif.");
        return false;
    }
    if (m_emplacementSaisi.isEmpty())
        this->etagere = m_etagereSaisi;
    else
        this->etagere = fusionEmplacementEtagere(m_emplacementSaisi, m_etagereSaisi);
    m_derniereErreurSaisie = messageSiSaisieInvalide(
        m_emplacementSaisi,
        m_etagereSaisi,
        capacite_max,
        quantite_actuelle,
        type_stockage);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    const QString typeOk = normaliserTypeStockage(type_stockage);

    QSqlDatabase db = depotDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE DEPOTS SET "
                  "ETAGERE = :et, "
                  "CAPACITE_MAX = :cap, "
                  "QUANTITE_ACTUELLE = :qte, "
                  "TYPE_STOCKAGE = :type "
                  "WHERE ID_EMPLACEMENT = :id");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":type", typeOk);
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "Erreur Modification Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery(db);
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// SUPPRIMER
// =============================================
bool Depot::supprimer(int id)
{
    QSqlDatabase db = depotDatabase();
    if (!db.isOpen())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM DEPOTS WHERE ID_EMPLACEMENT = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Depot :" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery(db);
    commitQuery.exec("COMMIT");
    return true;
}

// =============================================
// AFFICHER
// =============================================
QSqlQueryModel* Depot::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = depotDatabase();
    if (!db.isOpen()) {
        qDebug() << "Depot::afficher: base non ouverte";
        return model;
    }
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ID_EMPLACEMENT DESC",
                    db);

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
    QSqlDatabase db = depotDatabase();
    if (!db.isOpen())
        return model;
    QSqlQuery query(db);
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
    QSqlDatabase db = depotDatabase();
    if (!db.isOpen())
        return model;
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ETAGERE ASC",
                    db);

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Tri Depots :" << model->lastError().text();
    }
    return model;
}
