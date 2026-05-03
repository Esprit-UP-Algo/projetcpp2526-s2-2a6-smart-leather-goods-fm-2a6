#include "produit.h"
#include "connexion.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QVariant>
#include <QMetaType>
#include <utility>

namespace {
bool ensureMoteurLogsTable(QSqlDatabase &db)
{
    if (!db.isOpen())
        return false;
    QSqlQuery check(db);
    check.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TABLES "
        "WHERE UPPER(TABLE_NAME) = UPPER('MOTEUR_LOGS')"));
    if (!check.exec() || !check.next())
        return false;
    if (check.value(0).toInt() > 0)
        return true;
    QSqlQuery ddl(db);
    const bool ok = ddl.exec(QStringLiteral(
        "CREATE TABLE MOTEUR_LOGS ("
        "PRODUCT_ID NUMBER NOT NULL, "
        "CHOIX NUMBER(1) NOT NULL, "
        "MOUVEMENT VARCHAR2(64), "
        "COMMANDE VARCHAR2(32), "
        "ARDUINO_RESPONSE VARCHAR2(128), "
        "PORT_COM VARCHAR2(32), "
        "DUREE_MS NUMBER, "
        "STATUT VARCHAR2(16) DEFAULT 'PENDING', "
        "ID_COMMANDE NUMBER NULL, "
        "CREATED_AT TIMESTAMP DEFAULT SYSTIMESTAMP)"));
    if (ok)
        return true;
    const QString err = ddl.lastError().text();
    if (err.contains(QStringLiteral("ORA-00955")))
        return true;
    QSqlQuery recheck(db);
    recheck.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TABLES "
        "WHERE UPPER(TABLE_NAME) = UPPER('MOTEUR_LOGS')"));
    if (recheck.exec() && recheck.next() && recheck.value(0).toInt() > 0)
        return true;
    qDebug() << "[MoteurSmart] create MOTEUR_LOGS:" << err;
    return false;
}

bool ensureMoteurLogsColumn(QSqlDatabase &db, const QString &columnName, const QString &definition)
{
    QSqlQuery check(db);
    check.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TAB_COLUMNS "
        "WHERE UPPER(TABLE_NAME)=UPPER('MOTEUR_LOGS') "
        "AND UPPER(COLUMN_NAME)=UPPER(:col)"));
    check.bindValue(QStringLiteral(":col"), columnName);
    if (!check.exec() || !check.next())
        return false;
    if (check.value(0).toInt() > 0)
        return true;
    QSqlQuery alter(db);
    const QString sql = QStringLiteral("ALTER TABLE MOTEUR_LOGS ADD (%1 %2)").arg(columnName, definition);
    const bool ok = alter.exec(sql);
    if (!ok)
        qDebug() << "[MoteurSmart] ALTER MOTEUR_LOGS" << columnName << ":" << alter.lastError().text();
    return ok;
}

bool ensureProduitsColumn(QSqlDatabase &db, const QString &columnName, const QString &definition)
{
    QSqlQuery check(db);
    check.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TAB_COLUMNS "
        "WHERE UPPER(TABLE_NAME)=UPPER('PRODUITS') "
        "AND UPPER(COLUMN_NAME)=UPPER(:col)"));
    check.bindValue(QStringLiteral(":col"), columnName);
    if (!check.exec() || !check.next())
        return false;
    if (check.value(0).toInt() > 0)
        return true;
    QSqlQuery alter(db);
    const QString sql = QStringLiteral("ALTER TABLE PRODUITS ADD (%1 %2)").arg(columnName, definition);
    const bool ok = alter.exec(sql);
    if (!ok)
        qDebug() << "[MoteurSmart] ALTER PRODUITS" << columnName << ":" << alter.lastError().text();
    return ok;
}

bool ensureArduinoConfigTable(QSqlDatabase &db)
{
    QSqlQuery check(db);
    check.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TABLES "
        "WHERE UPPER(TABLE_NAME)=UPPER('ARDUINO_CONFIG')"));
    if (!check.exec() || !check.next())
        return false;
    if (check.value(0).toInt() == 0) {
        QSqlQuery ddl(db);
        const bool ok = ddl.exec(QStringLiteral(
            "CREATE TABLE ARDUINO_CONFIG ("
            "CLE VARCHAR2(64) PRIMARY KEY, "
            "VALEUR VARCHAR2(128) NOT NULL, "
            "UPDATED_AT TIMESTAMP DEFAULT SYSTIMESTAMP)"));
        if (!ok) {
            qDebug() << "[MoteurSmart] CREATE ARDUINO_CONFIG:" << ddl.lastError().text();
            return false;
        }
    }
    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
        "MERGE INTO ARDUINO_CONFIG c "
        "USING (SELECT :k AS CLE, :v AS VALEUR FROM dual) s "
        "ON (c.CLE = s.CLE) "
        "WHEN NOT MATCHED THEN INSERT (CLE, VALEUR) VALUES (s.CLE, s.VALEUR)"));
    auto upsertDefault = [&](const QString &k, const QString &v) {
        ins.bindValue(QStringLiteral(":k"), k);
        ins.bindValue(QStringLiteral(":v"), v);
        return ins.exec();
    };
    upsertDefault(QStringLiteral("port_com"), QStringLiteral("COM5"));
    upsertDefault(QStringLiteral("baudrate"), QStringLiteral("9600"));
    upsertDefault(QStringLiteral("auto_trigger"), QStringLiteral("1"));
    return true;
}
} // namespace

static QSqlDatabase produitDatabase()
{
    Connexion *cx = Connexion::getInstance();
    return cx ? cx->getDatabase() : QSqlDatabase();
}

bool Produit::ensureChoixColumn()
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return false;
    QSqlQuery check(db);
    check.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM USER_TAB_COLUMNS "
        "WHERE TABLE_NAME='PRODUITS' AND COLUMN_NAME='CHOIX'"));
    if (!check.exec() || !check.next())
        return false;
    if (check.value(0).toInt() > 0)
        return true;
    QSqlQuery alter(db);
    const bool ok = alter.exec(QStringLiteral("ALTER TABLE PRODUITS ADD (CHOIX NUMBER(1) DEFAULT 0 NOT NULL)"));
    if (!ok)
        qDebug() << "[MoteurSmart] ALTER TABLE PRODUITS CHOIX:" << alter.lastError().text();
    else
        qDebug() << "[MoteurSmart] Colonne CHOIX ajoutée à PRODUITS.";
    return ok;
}

int Produit::getProductChoix(int productId)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return -1;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT NVL(CHOIX, 0) FROM PRODUITS WHERE ID_PRODUIT = :id"));
    q.bindValue(QStringLiteral(":id"), productId);
    if (q.exec() && q.next())
        return q.value(0).toInt();
    qDebug() << "[MoteurSmart] getProductChoix erreur:" << q.lastError().text();
    return -1;
}

bool Produit::setProductChoix(int productId, int choix)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return false;
    ensureProduitsColumn(db, QStringLiteral("CHOIX_UPDATED_AT"), QStringLiteral("TIMESTAMP"));
    ensureProduitsColumn(db, QStringLiteral("MOTEUR_COUNT"), QStringLiteral("NUMBER DEFAULT 0"));
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE PRODUITS SET CHOIX = :choix, "
                             "CHOIX_UPDATED_AT = SYSTIMESTAMP, "
                             "MOTEUR_COUNT = NVL(MOTEUR_COUNT, 0) + 1 "
                             "WHERE ID_PRODUIT = :id"));
    q.bindValue(QStringLiteral(":choix"), choix);
    q.bindValue(QStringLiteral(":id"), productId);
    const bool ok = q.exec();
    if (!ok)
        qDebug() << "[MoteurSmart] setProductChoix err:" << q.lastError().text();
    return ok;
}

QString Produit::getProductDesignation(int productId)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return QString();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT DESIGNATION FROM PRODUITS WHERE ID_PRODUIT=:id"));
    q.bindValue(QStringLiteral(":id"), productId);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return QString();
}

QString Produit::getArduinoConfig(const QString &cle, const QString &defaut)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return defaut;
    if (!ensureArduinoConfigTable(db))
        return defaut;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT VALEUR FROM ARDUINO_CONFIG WHERE CLE = :cle"));
    q.bindValue(QStringLiteral(":cle"), cle);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return defaut;
}

bool Produit::setArduinoConfig(const QString &cle, const QString &valeur)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return false;
    if (!ensureArduinoConfigTable(db))
        return false;
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "MERGE INTO ARDUINO_CONFIG c "
        "USING (SELECT :cle AS CLE, :val AS VALEUR FROM dual) s "
        "ON (c.CLE = s.CLE) "
        "WHEN MATCHED THEN UPDATE SET c.VALEUR = s.VALEUR, c.UPDATED_AT = SYSTIMESTAMP "
        "WHEN NOT MATCHED THEN INSERT (CLE, VALEUR, UPDATED_AT) VALUES (s.CLE, s.VALEUR, SYSTIMESTAMP)"));
    q.bindValue(QStringLiteral(":cle"), cle);
    q.bindValue(QStringLiteral(":val"), valeur);
    return q.exec();
}

bool Produit::logMoteurAction(int productId, int choix, const QString &arduinoResponse,
                              const QString &commande, const QString &portCom,
                              const QString &statut, int dureeMs, int idCommande)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen())
        return false;
    if (!ensureMoteurLogsTable(db)) {
        qDebug() << "[MoteurSmart] logMoteurAction: table MOTEUR_LOGS indisponible, insert ignore.";
        return false;
    }
    ensureMoteurLogsColumn(db, QStringLiteral("MOUVEMENT"), QStringLiteral("VARCHAR2(64)"));
    ensureMoteurLogsColumn(db, QStringLiteral("ARDUINO_RESPONSE"), QStringLiteral("VARCHAR2(128)"));
    ensureMoteurLogsColumn(db, QStringLiteral("COMMANDE"), QStringLiteral("VARCHAR2(32)"));
    ensureMoteurLogsColumn(db, QStringLiteral("PORT_COM"), QStringLiteral("VARCHAR2(32)"));
    ensureMoteurLogsColumn(db, QStringLiteral("DUREE_MS"), QStringLiteral("NUMBER"));
    ensureMoteurLogsColumn(db, QStringLiteral("STATUT"), QStringLiteral("VARCHAR2(16)"));
    ensureMoteurLogsColumn(db, QStringLiteral("ID_COMMANDE"), QStringLiteral("NUMBER NULL"));
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare(QStringLiteral("INSERT INTO MOTEUR_LOGS "
                             "(PRODUCT_ID, CHOIX, MOUVEMENT, COMMANDE, ARDUINO_RESPONSE, PORT_COM, DUREE_MS, STATUT, ID_COMMANDE) "
                             "VALUES (:pid, :choix, :mouv, :cmd, :resp, :port, :duree, :statut, :idc)"));
    q.bindValue(QStringLiteral(":pid"), productId);
    q.bindValue(QStringLiteral(":choix"), choix);
    q.bindValue(QStringLiteral(":mouv"),
                choix == 1 ? QStringLiteral("1er choix (90°)")
                           : (choix == 2 ? QStringLiteral("2ème choix (180°)")
                                         : QStringLiteral("3ème choix (180°)")));
    q.bindValue(QStringLiteral(":cmd"), commande);
    q.bindValue(QStringLiteral(":resp"), arduinoResponse);
    q.bindValue(QStringLiteral(":port"), portCom);
    q.bindValue(QStringLiteral(":duree"), dureeMs >= 0 ? QVariant(dureeMs) : QVariant(QMetaType(QMetaType::Int)));
    q.bindValue(QStringLiteral(":statut"), statut.isEmpty() ? QStringLiteral("DONE") : statut);
    q.bindValue(QStringLiteral(":idc"), idCommande > 0 ? QVariant(idCommande) : QVariant(QMetaType(QMetaType::Int)));
    const bool ok = q.exec();
    if (!ok)
        qDebug() << "[MoteurSmart] logMoteurAction erreur:" << q.lastError().text();
    return ok;
}

bool Produit::updateLatestPendingMoteurLog(int productId, const QString &commande,
                                           const QString &arduinoResponse, const QString &statut,
                                           int dureeMs)
{
    QSqlDatabase db = produitDatabase();
    if (!db.isOpen() || productId <= 0 || commande.isEmpty())
        return false;
    if (!ensureMoteurLogsTable(db))
        return false;
    ensureMoteurLogsColumn(db, QStringLiteral("ARDUINO_RESPONSE"), QStringLiteral("VARCHAR2(128)"));
    ensureMoteurLogsColumn(db, QStringLiteral("STATUT"), QStringLiteral("VARCHAR2(16)"));
    ensureMoteurLogsColumn(db, QStringLiteral("DUREE_MS"), QStringLiteral("NUMBER"));
    ensureMoteurLogsColumn(db, QStringLiteral("CREATED_AT"), QStringLiteral("TIMESTAMP"));
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare(QStringLiteral(
        "UPDATE MOTEUR_LOGS SET "
        "STATUT = :statut, "
        "ARDUINO_RESPONSE = :resp, "
        "DUREE_MS = COALESCE(:duree, DUREE_MS) "
        "WHERE ROWID = ( "
        "  SELECT rid FROM ( "
        "    SELECT ROWID AS rid FROM MOTEUR_LOGS "
        "    WHERE PRODUCT_ID = :pid AND COMMANDE = :cmd AND STATUT = 'PENDING' "
        "    ORDER BY CREATED_AT DESC NULLS LAST "
        "  ) WHERE ROWNUM = 1 "
        ")"));
    q.bindValue(QStringLiteral(":statut"), statut);
    q.bindValue(QStringLiteral(":resp"), arduinoResponse);
    if (dureeMs >= 0)
        q.bindValue(QStringLiteral(":duree"), dureeMs);
    else
        q.bindValue(QStringLiteral(":duree"), QVariant());
    q.bindValue(QStringLiteral(":pid"), productId);
    q.bindValue(QStringLiteral(":cmd"), commande);
    if (!q.exec()) {
        qDebug() << "[MoteurSmart] updateLatestPendingMoteurLog:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

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
