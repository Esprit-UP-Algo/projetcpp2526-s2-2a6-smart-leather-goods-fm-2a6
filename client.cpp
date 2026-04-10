#include "client.h"  // ← Correction: "Client.h" → "client.h" (minuscule)

// ============= CONSTRUCTEURS =============
Client::Client() : id(0), cin(""), nom(""), tel(""), email(""), adresse(""), points(0) {}

Client::Client(QString cin, QString nom, QString tel, QString email, QString adresse, int points)
    : id(0), cin(cin), nom(nom), tel(tel), email(email), adresse(adresse), points(points) {}

Client::Client(int id, QString cin, QString nom, QString tel, QString email, QString adresse, int points)
    : id(id), cin(cin), nom(nom), tel(tel), email(email), adresse(adresse), points(points) {}
// ============= GETTERS =============
int Client::getId() const { return id; }
QString Client::getCin() const { return cin; }
QString Client::getNom() const { return nom; }
QString Client::getTel() const { return tel; }
QString Client::getEmail() const { return email; }
QString Client::getAdresse() const { return adresse; }
int Client::getPoints() const { return points; }
// ============= SETTERS =============
void Client::setId(int id) { this->id = id; }
void Client::setCin(const QString& cin) { this->cin = cin; }
void Client::setNom(const QString& nom) { this->nom = nom; }
void Client::setTel(const QString& tel) { this->tel = tel; }
void Client::setEmail(const QString& email) { this->email = email; }
void Client::setAdresse(const QString& adresse) { this->adresse = adresse; }
void Client::setPoints(int points) { this->points = points; }
// ============= CREATE - AJOUTER UN CLIENT =============
bool Client::ajouter()
{
    QSqlQuery query;

    query.prepare("INSERT INTO client (cin, nom, tel, email, adresse, points) "
                  "VALUES (:cin, :nom, :tel, :email, :adresse, :points)");

    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":tel", tel);
    query.bindValue(":email", email);
    query.bindValue(":adresse", adresse);
    query.bindValue(":points", points);  // ← AJOUTER

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'ajout: " << query.lastError().text();
        return false;
    }
    return true;
}

// ============= DELETE - SUPPRIMER UN CLIENT =============
bool Client::supprimer(int id)
{
    QSqlQuery query;
    QString resId = QString::number(id);

    query.prepare("DELETE FROM client WHERE id_client = :id_client");
    query.bindValue(":id_client", resId);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la suppression: " << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ============= UPDATE - MODIFIER UN CLIENT =============
bool Client::modifier(int id)
{
    QSqlQuery query;
    QString resId = QString::number(id);

    query.prepare("UPDATE client SET cin = :cin, nom = :nom, tel = :tel, "
                  "email = :email, adresse = :adresse, points = :points "
                  "WHERE id_client = :id_client");

    query.bindValue(":id_client", resId);
    query.bindValue(":cin", this->cin);
    query.bindValue(":nom", this->nom);
    query.bindValue(":tel", this->tel);
    query.bindValue(":email", this->email);
    query.bindValue(":adresse", this->adresse);
    query.bindValue(":points", this->points);  // ← AJOUTER

    if (!query.exec()) {
        qDebug() << "Erreur lors de la modification: " << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}
// ============= READ - AFFICHER TOUS LES CLIENTS =============
QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    // Utiliser id_client
    model->setQuery("SELECT id_client, cin, nom, tel, email, adresse FROM client ORDER BY id_client");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Adresse"));
     model->setHeaderData(6, Qt::Horizontal, "Points");
    return model;
}

// ============= RECHERCHER UN CLIENT =============
QSqlQueryModel* Client::rechercher(const QString& critere)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QString query = "SELECT id_client, cin, nom, tel, email, adresse FROM client "
                    "WHERE nom LIKE '%" + critere + "%' "
                                "OR cin LIKE '%" + critere + "%' "
                                "OR tel LIKE '%" + critere + "%' "
                                "ORDER BY id_client";

    model->setQuery(query);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("CIN / MF"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom Client"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Adresse"));

    return model;
}


QSqlQueryModel* Client::trierParPoints()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    model->setQuery("SELECT id_client, cin, nom, tel, email, adresse, points FROM client ORDER BY points DESC");

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "CIN / MF");
    model->setHeaderData(2, Qt::Horizontal, "Nom Client");
    model->setHeaderData(3, Qt::Horizontal, "Téléphone");
    model->setHeaderData(4, Qt::Horizontal, "Email");
    model->setHeaderData(5, Qt::Horizontal, "Adresse");
    model->setHeaderData(6, Qt::Horizontal, "Points");

    return model;
}
