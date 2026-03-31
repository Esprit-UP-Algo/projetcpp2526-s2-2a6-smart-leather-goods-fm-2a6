#include "employe.h"
#include <QSqlError>
#include <QSqlDatabase>

employe::employe() {}

employe::employe(int id_employe, QString nom, QString prenom, QString poste,
                 QString email, QString telephone, QString departement,
                 QDate date_embauche, double salaire, QString rfid_tag)
{
    this->id_employe = id_employe;
    this->nom = nom;
    this->prenom = prenom;
    this->poste = poste;
    this->email = email;
    this->telephone = telephone;
    this->departement = departement;
    this->date_embauche = date_embauche;
    this->salaire = salaire;
    this->rfid_tag = rfid_tag;
}

// ✅ AJOUTER
bool employe::ajouter()
{
    QSqlDatabase db = QSqlDatabase::database();

    if(!db.isOpen()){
        if(!db.open()){
            qDebug() << "DB OPEN ERROR:" << db.lastError().text();
            return false;
        }
    }

    const QStringList insertSqls = {
        "INSERT INTO EMPLOYES "
        "(ID_EMPLOYE, NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE, RFID_TAG) "
        "VALUES (PROJET_CPP.SEQ_EMPLOYE.NEXTVAL, :nom, :prenom, :poste, :email, :tel, :dep, :dateEmb, :sal, :rfid)",

        "INSERT INTO EMPLOYES "
        "(ID_EMPLOYE, NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE, RFID_TAG) "
        "VALUES (SEQ_EMPLOYE.NEXTVAL, :nom, :prenom, :poste, :email, :tel, :dep, :dateEmb, :sal, :rfid)",

        "INSERT INTO EMPLOYES "
        "(ID_EMPLOYE, NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE, RFID_TAG) "
        "VALUES (SEQ_EMP.NEXTVAL, :nom, :prenom, :poste, :email, :tel, :dep, :dateEmb, :sal, :rfid)",

        // Fallback utile si ID est généré par trigger.
        "INSERT INTO EMPLOYES "
        "(NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE, RFID_TAG) "
        "VALUES (:nom, :prenom, :poste, :email, :tel, :dep, :dateEmb, :sal, :rfid)"
    };

    QString lastErr;
    for (const QString &sql : insertSqls) {
        QSqlQuery query;
        query.prepare(sql);
        query.bindValue(":nom", nom);
        query.bindValue(":prenom", prenom);
        query.bindValue(":poste", poste);
        query.bindValue(":email", email);
        query.bindValue(":tel", telephone);
        query.bindValue(":dep", departement);
        query.bindValue(":dateEmb", date_embauche);
        query.bindValue(":sal", salaire);
        query.bindValue(":rfid", rfid_tag);

        if (query.exec()) {
            return true;
        }
        lastErr = query.lastError().text();
    }

    qDebug() << "Erreur ajouter employe:" << lastErr;
    return false;
}


// ✅ AFFICHER
QSqlQueryModel * employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    // Inclure tous les champs nécessaires à l'édition dans l'UI (email/tel/RFID).
    model->setQuery(
        "SELECT ID_EMPLOYE, NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE, RFID_TAG "
        "FROM EMPLOYES "
        "ORDER BY ID_EMPLOYE DESC"
    );
    if(model->lastError().isValid()){
        qDebug() << "Erreur afficher employe:" << model->lastError().text();
    }
    return model;
}

// ✅ SUPPRIMER
bool employe::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM EMPLOYES WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "Erreur supprimer employe:" << query.lastError().text();
        return false;
    }
    return true;
}

// ✅ MODIFIER  (هذا هو اللي ناقص عندك وسبب undefined reference)
bool employe::modifier(int id)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE EMPLOYES SET "
        "NOM=:nom, PRENOM=:prenom, POSTE=:poste, EMAIL=:email, "
        "TELEPHONE=:tel, DEPARTEMENT=:dep, DATE_EMBAUCHE=:dateEmb, "
        "SALAIRE=:sal, RFID_TAG=:rfid "
        "WHERE ID_EMPLOYE=:id"
        );

    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":poste", poste);
    query.bindValue(":email", email);
    query.bindValue(":tel", telephone);
    query.bindValue(":dep", departement);
    query.bindValue(":dateEmb", date_embauche);
    query.bindValue(":sal", salaire);
    query.bindValue(":rfid", rfid_tag);
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "Erreur modifier employe:" << query.lastError().text();
        return false;
    }
    return true;
}
