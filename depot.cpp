#include "depot.h"
#include <QDebug>
#include <QSqlError>
#include <climits>
#include <cmath>
#include <utility>

static constexpr double kMaxUnitesDepotFormulaire = 100000.0;

namespace {

QString fusionEmplacementEtagere(const QString &emplacement, const QString &etagere)
{
    const QString e = emplacement.trimmed();
    const QString t = etagere.trimmed();
    if (e.isEmpty())
        return t;
    if (t.isEmpty())
        return e;
    return e + QStringLiteral(" - ") + t;
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

void assurerColonneValeurGaz()
{
    QSqlQuery query;
    if (!query.exec("ALTER TABLE DEPOTS ADD VALEUR_GAZ NUMBER(12,2)")) {
        const QString err = query.lastError().text();
        if (!err.contains("ORA-01430") && !err.contains("ORA-01442")) {
            qDebug() << "ALTER TABLE DEPOTS (VALEUR_GAZ):" << err;
        }
    }
}

} // namespace

QString Depot::messageIdAjoutNeDoitPasEtreSaisi(const QString &idTexteBrut)
{
    if (idTexteBrut.trimmed().isEmpty())
        return {};
    return QStringLiteral("L'identifiant est genere automatiquement : ne renseignez pas le champ ID a l'ajout.");
}

QString Depot::messageIdModificationTexteInvalide(const QString &idTexteBrut)
{
    const QString s = idTexteBrut.trimmed();
    if (s.isEmpty())
        return QStringLiteral("L'identifiant de l'emplacement est obligatoire pour la modification.");
    bool ok = false;
    const qlonglong v = s.toLongLong(&ok);
    if (!ok || v < 1 || v > INT_MAX)
        return QStringLiteral("L'identifiant doit etre un entier strictement positif (maximum %1).").arg(INT_MAX);
    return {};
}

Depot::Depot() {}

Depot::Depot(QString emplacement, QString etagere, double capMax, double qteActuelle, double valeurGaz, QString typeStock)
{
    m_emplacementSaisi = emplacement.trimmed();
    m_etagereSaisi = etagere.trimmed();
    if (m_emplacementSaisi.isEmpty())
        this->etagere = m_etagereSaisi;
    else
        this->etagere = fusionEmplacementEtagere(m_emplacementSaisi, m_etagereSaisi);
    capacite_max = capMax;
    quantite_actuelle = qteActuelle;
    valeur_gaz = valeurGaz;
    type_stockage = typeStock.trimmed();
}

QString Depot::messageSiSaisieInvalide(const QString &emplacement,
                                       const QString &etagere,
                                       double capaciteMax,
                                       double quantiteActuelle,
                                       double valeurGaz,
                                       const QString &typeStockage)
{
    const QString emp = emplacement.trimmed();
    const QString et = etagere.trimmed();

    if (emp.isEmpty() && et.isEmpty())
        return QStringLiteral("Renseignez l'emplacement et l'etagere.");

    if (!emp.isEmpty()) {
        if (emp.length() > 80)
            return QStringLiteral("L'emplacement ne doit pas depasser 80 caracteres.");
        if (et.isEmpty())
            return QStringLiteral("L'etagere est obligatoire lorsque l'emplacement est renseigne.");
        if (et.length() > 80)
            return QStringLiteral("L'etagere ne doit pas depasser 80 caracteres.");
        const QString fusion = fusionEmplacementEtagere(emp, et);
        if (fusion.length() > 100)
            return QStringLiteral("Emplacement + etagere : 100 caracteres maximum.");
    } else if (et.length() > 100) {
        return QStringLiteral("La description etagere / emplacement ne doit pas depasser 100 caracteres.");
    }

    const QString typ = typeStockage.trimmed();
    if (typ.compare(QStringLiteral("Sec"), Qt::CaseInsensitive) != 0
        && typ.compare(QStringLiteral("Froid"), Qt::CaseInsensitive) != 0) {
        return QStringLiteral("Le type de stockage doit etre Sec ou Froid.");
    }

    if (!std::isfinite(capaciteMax) || !std::isfinite(quantiteActuelle) || !std::isfinite(valeurGaz))
        return QStringLiteral("Capacite, quantite ou valeur gaz : valeur numerique invalide.");

    if (capaciteMax > kMaxUnitesDepotFormulaire + 1e-6)
        return QStringLiteral("La capacite maximale ne peut pas depasser 100000.");

    if (quantiteActuelle > kMaxUnitesDepotFormulaire + 1e-6)
        return QStringLiteral("La quantite actuelle ne peut pas depasser 100000.");

    if (valeurGaz > kMaxUnitesDepotFormulaire + 1e-6)
        return QStringLiteral("La valeur gaz ne peut pas depasser 100000.");

    if (capaciteMax <= 0.0)
        return QStringLiteral("La capacite maximale doit etre strictement positive.");

    if (quantiteActuelle < 0.0)
        return QStringLiteral("La quantite actuelle ne peut pas etre negative.");

    if (valeurGaz < 0.0)
        return QStringLiteral("La valeur gaz ne peut pas etre negative.");

    if (quantiteActuelle > capaciteMax + 1e-6)
        return QStringLiteral("La quantite actuelle ne peut pas depasser la capacite maximale.");

    return {};
}

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
        valeur_gaz,
        type_stockage);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    assurerColonneValeurGaz();
    const QString typeOk = normaliserTypeStockage(type_stockage);

    QSqlQuery query;
    query.prepare("INSERT INTO DEPOTS "
                  "(ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, VALEUR_GAZ, TYPE_STOCKAGE) "
                  "VALUES (SEQ_DEP.NEXTVAL, :et, :cap, :qte, :gaz, :type)");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":gaz", valeur_gaz);
    query.bindValue(":type", typeOk);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "Erreur Ajout Depot:" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

bool Depot::modifier(int id)
{
    m_derniereErreurSaisie.clear();
    if (id <= 0) {
        m_derniereErreurSaisie = QStringLiteral("L'identifiant de l'emplacement doit etre un entier strictement positif.");
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
        valeur_gaz,
        type_stockage);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    assurerColonneValeurGaz();
    const QString typeOk = normaliserTypeStockage(type_stockage);

    QSqlQuery query;
    query.prepare("UPDATE DEPOTS SET "
                  "ETAGERE = :et, "
                  "CAPACITE_MAX = :cap, "
                  "QUANTITE_ACTUELLE = :qte, "
                  "VALEUR_GAZ = :gaz, "
                  "TYPE_STOCKAGE = :type "
                  "WHERE ID_EMPLACEMENT = :id");

    query.bindValue(":et", etagere);
    query.bindValue(":cap", capacite_max);
    query.bindValue(":qte", quantite_actuelle);
    query.bindValue(":gaz", valeur_gaz);
    query.bindValue(":type", typeOk);
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "Erreur Modification Depot:" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

bool Depot::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM DEPOTS WHERE ID_EMPLACEMENT = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Depot:" << query.lastError().text();
        return false;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");
    return true;
}

QSqlQueryModel* Depot::afficher()
{
    assurerColonneValeurGaz();
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, VALEUR_GAZ, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ID_EMPLACEMENT DESC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage Depots:" << model->lastError().text();
    }
    return model;
}

QSqlQueryModel* Depot::rechercher(const QString &critere)
{
    assurerColonneValeurGaz();
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, VALEUR_GAZ, TYPE_STOCKAGE "
                  "FROM DEPOTS "
                  "WHERE UPPER(ETAGERE) LIKE UPPER(:c1) "
                  "OR UPPER(TYPE_STOCKAGE) LIKE UPPER(:c2) "
                  "ORDER BY ID_EMPLACEMENT DESC");

    const QString pattern = "%" + critere + "%";
    query.bindValue(":c1", pattern);
    query.bindValue(":c2", pattern);
    query.exec();

    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel* Depot::trierParEtagere()
{
    assurerColonneValeurGaz();
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLACEMENT, ETAGERE, CAPACITE_MAX, QUANTITE_ACTUELLE, VALEUR_GAZ, TYPE_STOCKAGE "
                    "FROM DEPOTS ORDER BY ETAGERE ASC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Tri Depots:" << model->lastError().text();
    }
    return model;
}

