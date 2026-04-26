#include "produit.h"
#include "connexion.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QVariant>
#include <utility>

Produit::Produit() {}

Produit::Produit(int id_produit,
                 QString designation,
                 double cout,
                 QString collection,
                 QString type_cuir_requis,
                 int temps_fabrication,
                 int id_client,
                 int id_emplacement)
    : id_produit(id_produit)
    , designation(std::move(designation))
    , cout(cout)
    , collection(std::move(collection))
    , type_cuir_requis(std::move(type_cuir_requis))
    , temps_fabrication(temps_fabrication)
    , id_client(id_client)
    , id_emplacement(id_emplacement)
{
}

static QSqlDatabase produitDatabase()
{
    Connexion *cx = Connexion::getInstance();
    return cx ? cx->getDatabase() : QSqlDatabase();
}

bool Produit::ajouter()
{
    m_derniereErreurSaisie.clear();
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }

    const bool useTxn = db.transaction();
    QSqlQuery query(db);

    query.prepare(QStringLiteral(
        "INSERT INTO PRODUITS "
        "(ID_PRODUIT, DESIGNATION, COUT, COLLECTION, TYPE_CUIR_REQUIS, TEMPS_FABRICATION, ID_CLIENT, ID_EMPLACEMENT) "
        "VALUES (SEQ_PRODUIT.NEXTVAL, :des, :cout, :coll, :cuir, :temps, :client, :empl)"));

    query.bindValue(QStringLiteral(":des"), designation);
    query.bindValue(QStringLiteral(":cout"), cout);
    query.bindValue(QStringLiteral(":coll"), collection);
    query.bindValue(QStringLiteral(":cuir"), type_cuir_requis);
    query.bindValue(QStringLiteral(":temps"), temps_fabrication);

    if (id_client <= 0)
        query.bindValue(QStringLiteral(":client"), QVariant());
    else
        query.bindValue(QStringLiteral(":client"), id_client);

    if (id_emplacement <= 0)
        query.bindValue(QStringLiteral(":empl"), QVariant());
    else
        query.bindValue(QStringLiteral(":empl"), id_emplacement);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "ERROR INSERT PRODUIT:" << m_derniereErreurSaisie;
        if (useTxn)
            db.rollback();
        return false;
    }

    if (useTxn) {
        if (!db.commit()) {
            m_derniereErreurSaisie = db.lastError().text();
            db.rollback();
            return false;
        }
    } else {
        QSqlQuery c(db);
        if (!c.exec(QStringLiteral("COMMIT"))) {
            m_derniereErreurSaisie = c.lastError().text();
            return false;
        }
    }
    return true;
}

bool Produit::modifier(int id)
{
    m_derniereErreurSaisie.clear();
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }

    const bool useTxn = db.transaction();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE PRODUITS SET "
        "DESIGNATION = :des, "
        "COUT = :cout, "
        "COLLECTION = :coll, "
        "TYPE_CUIR_REQUIS = :cuir, "
        "TEMPS_FABRICATION = :temps, "
        "ID_CLIENT = :client, "
        "ID_EMPLACEMENT = :empl "
        "WHERE ID_PRODUIT = :id"));

    query.bindValue(QStringLiteral(":des"), designation);
    query.bindValue(QStringLiteral(":cout"), cout);
    query.bindValue(QStringLiteral(":coll"), collection);
    query.bindValue(QStringLiteral(":cuir"), type_cuir_requis);
    query.bindValue(QStringLiteral(":temps"), temps_fabrication);
    if (id_client <= 0)
        query.bindValue(QStringLiteral(":client"), QVariant());
    else
        query.bindValue(QStringLiteral(":client"), id_client);

    if (id_emplacement <= 0)
        query.bindValue(QStringLiteral(":empl"), QVariant());
    else
        query.bindValue(QStringLiteral(":empl"), id_emplacement);

    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_derniereErreurSaisie = query.lastError().text();
        qDebug() << "ERROR UPDATE PRODUIT:" << m_derniereErreurSaisie;
        if (useTxn)
            db.rollback();
        return false;
    }

    if (useTxn) {
        if (!db.commit()) {
            m_derniereErreurSaisie = db.lastError().text();
            db.rollback();
            return false;
        }
    } else {
        QSqlQuery c(db);
        if (!c.exec(QStringLiteral("COMMIT"))) {
            m_derniereErreurSaisie = c.lastError().text();
            return false;
        }
    }
    return true;
}

bool Produit::supprimer(int id, QString *messageErreur)
{
    m_derniereErreurSaisie.clear();
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        if (messageErreur)
            *messageErreur = m_derniereErreurSaisie;
        return false;
    }

    const bool useTxn = db.transaction();
    QSqlQuery query(db);

    query.prepare(QStringLiteral("DELETE FROM PLANIFICATION WHERE ID_PRODUIT = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        const QString err = query.lastError().text();
        m_derniereErreurSaisie = err;
        qDebug() << "Erreur DELETE PLANIFICATION (produit):" << err;
        if (messageErreur)
            *messageErreur = err;
        if (useTxn)
            db.rollback();
        return false;
    }

    query.prepare(QStringLiteral("DELETE FROM PRODUITS WHERE ID_PRODUIT = :id"));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        const QString err = query.lastError().text();
        m_derniereErreurSaisie = err;
        qDebug() << "Erreur Supprimer Produit :" << err;
        if (messageErreur)
            *messageErreur = err;
        if (useTxn)
            db.rollback();
        return false;
    }

    if (useTxn) {
        if (!db.commit()) {
            m_derniereErreurSaisie = db.lastError().text();
            db.rollback();
            return false;
        }
    } else {
        QSqlQuery c(db);
        if (!c.exec(QStringLiteral("COMMIT"))) {
            m_derniereErreurSaisie = c.lastError().text();
            return false;
        }
    }
    return true;
}

QSqlQueryModel *Produit::afficher(const QString &filtreCollection)
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen()) {
        qDebug() << "Produit::afficher: base non ouverte";
        return model;
    }

    const QString needle = filtreCollection.trimmed();

    QString sql = QStringLiteral(
        "SELECT p.ID_PRODUIT, p.DESIGNATION, p.COUT, p.COLLECTION, p.TYPE_CUIR_REQUIS, p.TEMPS_FABRICATION, "
        "p.ID_CLIENT, p.ID_EMPLACEMENT, c.NOM AS NOM_CLIENT, d.ETAGERE AS ETA_DEPOT "
        "FROM PRODUITS p "
        "LEFT JOIN CLIENTS c ON c.ID_CLIENT = p.ID_CLIENT "
        "LEFT JOIN DEPOTS d ON d.ID_EMPLACEMENT = p.ID_EMPLACEMENT");

    if (!needle.isEmpty())
        sql += QStringLiteral(" WHERE UPPER(p.COLLECTION) LIKE :needle");
    sql += QStringLiteral(" ORDER BY p.ID_PRODUIT DESC");

    if (needle.isEmpty()) {
        model->setQuery(sql, db);
    } else {
        QSqlQuery query(db);
        query.prepare(sql);
        query.bindValue(QStringLiteral(":needle"), QStringLiteral("%") + needle.toUpper() + QStringLiteral("%"));
        if (!query.exec())
            qDebug() << "Erreur afficher PRODUITS (filtre):" << query.lastError().text();
        model->setQuery(std::move(query));
    }

    if (model->lastError().isValid())
        qDebug() << "Erreur model PRODUITS:" << model->lastError().text();

    return model;
}
