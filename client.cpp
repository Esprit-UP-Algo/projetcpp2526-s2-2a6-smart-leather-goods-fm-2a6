#include "client.h"
#include "connexion.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <utility>

/// Valide la session Oracle (QODBC gère mal QSqlDatabase::transaction/commit pour Oracle).
static bool validerSessionOracle(QSqlDatabase &db, QString *erreur)
{
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("COMMIT")))
        return true;
    if (erreur)
        *erreur = q.lastError().text();
    return false;
}

Client::Client() = default;

Client::Client(int id, QString nom, QString tel, QString adr, QString mail, int pts)
    : id_client(id)
    , nom(std::move(nom))
    , telephone(std::move(tel))
    , adresse(std::move(adr))
    , email(std::move(mail))
    , points_fidelite(pts)
{
}

static QSqlDatabase clientDatabase()
{
    Connexion *cx = Connexion::getInstance();
    return cx ? cx->getDatabase() : QSqlDatabase();
}

bool Client::ajouter()
{
    m_derniereErreurSaisie.clear();
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }

    QSqlQuery q(db);
    if (id_client <= 0) {
        q.prepare(QStringLiteral(
            "INSERT INTO CLIENTS (ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE) "
            "VALUES (SEQ_CLIENT.NEXTVAL, :nom, :tel, :adr, :mail, :pts)"));
    } else {
        q.prepare(QStringLiteral(
            "INSERT INTO CLIENTS (ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE) "
            "VALUES (:id, :nom, :tel, :adr, :mail, :pts)"));
        q.bindValue(QStringLiteral(":id"), id_client);
    }
    q.bindValue(QStringLiteral(":nom"), nom.trimmed());
    q.bindValue(QStringLiteral(":tel"), telephone.trimmed());
    q.bindValue(QStringLiteral(":adr"), adresse.trimmed());
    q.bindValue(QStringLiteral(":mail"), email.trimmed());
    q.bindValue(QStringLiteral(":pts"), points_fidelite);

    if (!q.exec()) {
        m_derniereErreurSaisie = q.lastError().text();
        if (m_derniereErreurSaisie.contains(QStringLiteral("ORA-02289"), Qt::CaseInsensitive)
            || m_derniereErreurSaisie.contains(QStringLiteral("sequence does not exist"), Qt::CaseInsensitive)) {
            m_derniereErreurSaisie += QStringLiteral(
                "\n\nLa séquence Oracle SEQ_CLIENT est absente. "
                "Exécute une fois le script oracle_seq_client.sql dans SQL Developer "
                "(même schéma / utilisateur que la connexion de l’application).");
        }
        qDebug() << "Erreur Client::ajouter:" << m_derniereErreurSaisie;
        return false;
    }

    if (!validerSessionOracle(db, &m_derniereErreurSaisie)) {
        qDebug() << "Erreur Client::ajouter COMMIT:" << m_derniereErreurSaisie;
        return false;
    }
    qDebug() << "Client::ajouter OK (COMMIT)";
    return true;
}

bool Client::modifier(int id)
{
    m_derniereErreurSaisie.clear();
    if (id <= 0) {
        m_derniereErreurSaisie = QStringLiteral("Identifiant client invalide.");
        return false;
    }
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "UPDATE CLIENTS SET "
        "NOM = :nom, TELEPHONE = :tel, ADRESSE = :adr, EMAIL = :mail, POINTS_FIDELITE = :pts "
        "WHERE ID_CLIENT = :id"));
    q.bindValue(QStringLiteral(":nom"), nom.trimmed());
    q.bindValue(QStringLiteral(":tel"), telephone.trimmed());
    q.bindValue(QStringLiteral(":adr"), adresse.trimmed());
    q.bindValue(QStringLiteral(":mail"), email.trimmed());
    q.bindValue(QStringLiteral(":pts"), points_fidelite);
    q.bindValue(QStringLiteral(":id"), id);

    if (!q.exec()) {
        m_derniereErreurSaisie = q.lastError().text();
        qDebug() << "Erreur Client::modifier:" << m_derniereErreurSaisie;
        return false;
    }

    if (!validerSessionOracle(db, &m_derniereErreurSaisie))
        return false;
    return true;
}

bool Client::supprimer(int id)
{
    m_derniereErreurSaisie.clear();
    if (id <= 0) {
        m_derniereErreurSaisie = QStringLiteral("Identifiant client invalide.");
        return false;
    }
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        m_derniereErreurSaisie = QStringLiteral("Base de données non connectée.");
        return false;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM CLIENTS WHERE ID_CLIENT = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!q.exec()) {
        m_derniereErreurSaisie = q.lastError().text();
        qDebug() << "Erreur Client::supprimer:" << m_derniereErreurSaisie;
        return false;
    }

    if (!validerSessionOracle(db, &m_derniereErreurSaisie))
        return false;
    return true;
}

QSqlQueryModel *Client::afficher()
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        qDebug() << "Client::afficher: base non ouverte";
        return model;
    }
    model->setQuery(QStringLiteral(
                        "SELECT ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE "
                        "FROM CLIENTS ORDER BY ID_CLIENT DESC"),
                    db);
    if (model->lastError().isValid())
        qDebug() << "Client::afficher:" << model->lastError().text();
    return model;
}

QSqlQueryModel *Client::rechercher(const QString &critere)
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        qDebug() << "Client::rechercher: base non ouverte";
        return model;
    }

    const QString w = critere.trimmed();
    if (w.isEmpty()) {
        model->setQuery(QStringLiteral(
                            "SELECT ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE "
                            "FROM CLIENTS ORDER BY ID_CLIENT DESC"),
                        db);
        return model;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE "
        "FROM CLIENTS "
        "WHERE UPPER(NOM) LIKE UPPER(:p) "
        "   OR UPPER(TELEPHONE) LIKE UPPER(:p) "
        "   OR UPPER(EMAIL) LIKE UPPER(:p) "
        "   OR UPPER(ADRESSE) LIKE UPPER(:p) "
        "   OR TO_CHAR(ID_CLIENT) LIKE :p2 "
        "ORDER BY ID_CLIENT DESC"));
    const QString pattern = QStringLiteral("%") + w + QStringLiteral("%");
    q.bindValue(QStringLiteral(":p"), pattern);
    q.bindValue(QStringLiteral(":p2"), pattern);
    if (!q.exec())
        qDebug() << "Client::rechercher:" << q.lastError().text();
    model->setQuery(std::move(q));
    return model;
}

QSqlQueryModel *Client::trierParNom()
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = clientDatabase();
    if (!db.isOpen()) {
        qDebug() << "Client::trierParNom: base non ouverte";
        return model;
    }
    model->setQuery(QStringLiteral(
                        "SELECT ID_CLIENT, NOM, TELEPHONE, ADRESSE, EMAIL, POINTS_FIDELITE "
                        "FROM CLIENTS ORDER BY NOM ASC"),
                    db);
    if (model->lastError().isValid())
        qDebug() << "Client::trierParNom:" << model->lastError().text();
    return model;
}
