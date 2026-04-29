#include "matierepremiere.h"
#include "connexion.h"
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include <QRegularExpression>
#include <QStringList>

namespace {

bool valeurDansListe(const QString &v, const QStringList &liste)
{
    const QString t = v.trimmed();
    for (const QString &ref : liste) {
        if (ref.compare(t, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

// Aligné sur les QComboBox du formulaire (mainwindow.ui) — à étendre si vous ajoutez des items.
const QStringList &categoriesMpAutorisees()
{
    static const QStringList L = {
        QStringLiteral("Cuir"),
        QStringLiteral("Fil"),
    };
    return L;
}

const QStringList &etatsMpAutorises()
{
    static const QStringList L = {
        QStringLiteral("BRUT"),
        QStringLiteral("TEINT"),
    };
    return L;
}

const QStringList &typesStockageAutorises()
{
    static const QStringList L = {
        QStringLiteral("Sec"),
        QStringLiteral("Froid"),
    };
    return L;
}

const QStringList &qualitesAutorisees()
{
    static const QStringList L = {
        QStringLiteral("A"),
        QStringLiteral("B"),
        QStringLiteral("C"),
    };
    return L;
}

} // namespace

static QSqlDatabase matiereDatabase()
{
    Connexion *cx = Connexion::getInstance();
    return cx ? cx->getDatabase() : QSqlDatabase();
}

QString MatierePremiere::messageSiSaisieInvalide(const QString &code,
                                                 const QString &categorie,
                                                 const QString &numeroLot,
                                                 const QString &etat,
                                                 const QString &couleur,
                                                 double quantite,
                                                 const QString &typeStockage,
                                                 const QString &qualite)
{
    const QString c = code.trimmed().toUpper();
    static const QRegularExpression reCode(QStringLiteral("^[A-Z]{2,4}-20\\d{2}-\\d{3}$"));
    if (!reCode.match(c).hasMatch()) {
        return QStringLiteral(
            "Le code MP doit respecter le format AA(A)-20YY-NNN (ex. CUI-2024-001) : "
            "2 à 4 lettres, tiret, année 20xx, tiret, 3 chiffres.");
    }

    if (!valeurDansListe(categorie, categoriesMpAutorisees())) {
        return QStringLiteral(
            "La catégorie doit être choisie dans la liste (ex. Cuir, Fil).");
    }

    const QString l = numeroLot.trimmed();
    static const QRegularExpression reLot(QStringLiteral("^LOT-20\\d{2}-[A-Z]$"));
    if (!reLot.match(l).hasMatch()) {
        return QStringLiteral(
            "Le numéro de lot doit respecter le format LOT-20YY-X (ex. LOT-2024-A) : "
            "une seule lettre après le dernier tiret.");
    }

    if (!valeurDansListe(etat, etatsMpAutorises())) {
        return QStringLiteral(
            "L'état doit être choisi dans la liste (BRUT ou TEINT).");
    }

    const QString col = couleur.trimmed();
    static const QRegularExpression reCoul(QStringLiteral("^[A-Za-zÀ-ÿ ]{3,20}$"));
    if (!reCoul.match(col).hasMatch()) {
        return QStringLiteral(
            "La couleur : lettres (y compris accents) et espaces, entre 3 et 20 caractères.");
    }

    if (quantite <= 0.0) {
        return QStringLiteral("La quantité doit être strictement positive.");
    }
    if (quantite > 100000.0) {
        return QStringLiteral("La quantité dépasse la limite autorisée (100 000).");
    }

    if (!valeurDansListe(typeStockage, typesStockageAutorises())) {
        return QStringLiteral(
            "Le type de stockage doit être choisi dans la liste (Sec ou Froid).");
    }

    if (!valeurDansListe(qualite, qualitesAutorisees())) {
        return QStringLiteral(
            "La qualité doit être choisie dans la liste (A, B ou C).");
    }

    return {};
}

MatierePremiere::MatierePremiere() {}

MatierePremiere::MatierePremiere(QString code, QString cat, QString lot, QString etat,
                                 QString coul, double qte, QString typeStock, QString qual)
{
    code_mp = code;
    categorie_mp = cat;
    num_lot = lot;
    etat_mp = etat;
    couleur = coul;
    quantite = qte;
    type_stockage = typeStock;
    qualite = qual;
}

// =============================================
// AJOUTER
// =============================================
bool MatierePremiere::ajouter()
{
    m_derniereErreurSaisie.clear();
    QString cNorm = code_mp.trimmed().toUpper();
    categorie_mp = categorie_mp.trimmed();
    num_lot = num_lot.trimmed();
    etat_mp = etat_mp.trimmed();
    couleur = couleur.trimmed();
    type_stockage = type_stockage.trimmed();
    qualite = qualite.trimmed();

    m_derniereErreurSaisie = messageSiSaisieInvalide(
        cNorm, categorie_mp, num_lot, etat_mp, couleur, quantite, type_stockage, qualite);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    code_mp = cNorm;

    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen()) {
        qDebug() << "MatierePremiere::ajouter: base non ouverte";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO MATIERES_PREMIERES "
                  "(ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE) "
                  "VALUES (SEQ_MAT.NEXTVAL, :code, :cat, :lot, :etat, :coul, :qte, :type, :qual)");

    query.bindValue(":code", code_mp);
    query.bindValue(":cat", categorie_mp);
    query.bindValue(":lot", num_lot);
    query.bindValue(":etat", etat_mp);
    query.bindValue(":coul", couleur);
    query.bindValue(":qte", quantite);
    query.bindValue(":type", type_stockage);
    query.bindValue(":qual", qualite);

    if (!query.exec()) {
        qDebug() << "Erreur Ajout Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// MODIFIER
// =============================================
bool MatierePremiere::modifier(int id)
{
    m_derniereErreurSaisie.clear();
    QString cNorm = code_mp.trimmed().toUpper();
    categorie_mp = categorie_mp.trimmed();
    num_lot = num_lot.trimmed();
    etat_mp = etat_mp.trimmed();
    couleur = couleur.trimmed();
    type_stockage = type_stockage.trimmed();
    qualite = qualite.trimmed();

    m_derniereErreurSaisie = messageSiSaisieInvalide(
        cNorm, categorie_mp, num_lot, etat_mp, couleur, quantite, type_stockage, qualite);
    if (!m_derniereErreurSaisie.isEmpty())
        return false;

    code_mp = cNorm;

    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen()) {
        qDebug() << "MatierePremiere::modifier: base non ouverte";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE MATIERES_PREMIERES SET "
                  "CODE_MP = :code, "
                  "CATEGORIE_MP = :cat, "
                  "NUM_LOT = :lot, "
                  "ETAT_MP = :etat, "
                  "COULEUR = :coul, "
                  "QUANTITE = :qte, "
                  "TYPE_STOCKAGE = :type, "
                  "QUALITE = :qual "
                  "WHERE ID_STOCK_MP = :id");

    query.bindValue(":code", code_mp);
    query.bindValue(":cat", categorie_mp);
    query.bindValue(":lot", num_lot);
    query.bindValue(":etat", etat_mp);
    query.bindValue(":coul", couleur);
    query.bindValue(":qte", quantite);
    query.bindValue(":type", type_stockage);
    query.bindValue(":qual", qualite);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Modification Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// SUPPRIMER
// =============================================
bool MatierePremiere::supprimer(int id)
{
    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen())
        return false;
    QSqlQuery query(db);
    query.prepare("DELETE FROM MATIERES_PREMIERES WHERE ID_STOCK_MP = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur Suppression Matière :" << query.lastError().text();
        return false;
    }
    return true;
}

// =============================================
// AFFICHER
// =============================================
QSqlQueryModel* MatierePremiere::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen()) {
        qDebug() << "MatierePremiere::afficher: base non ouverte";
        return model;
    }
    model->setQuery("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                    "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                    "FROM MATIERES_PREMIERES "
                    "ORDER BY ID_STOCK_MP DESC",
                    db);

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Affichage Matières :" << model->lastError().text();
    }
    return model;
}

// =============================================
// RECHERCHER
// =============================================
QSqlQueryModel* MatierePremiere::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen())
        return model;
    QSqlQuery query(db);
    query.prepare("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                  "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                  "FROM MATIERES_PREMIERES "
                  "WHERE UPPER(CODE_MP) LIKE UPPER(:c1) "
                  "OR UPPER(CATEGORIE_MP) LIKE UPPER(:c2) "
                  "OR UPPER(COULEUR) LIKE UPPER(:c3) "
                  "OR UPPER(NUM_LOT) LIKE UPPER(:c4) "
                  "ORDER BY ID_STOCK_MP DESC");

    QString pattern = "%" + critere + "%";
    query.bindValue(":c1", pattern);
    query.bindValue(":c2", pattern);
    query.bindValue(":c3", pattern);
    query.bindValue(":c4", pattern);
    query.exec();

    model->setQuery(std::move(query));
    return model;
}

// =============================================
// TRI ALPHABÉTIQUE
// =============================================
QSqlQueryModel* MatierePremiere::trierParCode()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = matiereDatabase();
    if (!db.isOpen())
        return model;
    model->setQuery("SELECT ID_STOCK_MP, CODE_MP, CATEGORIE_MP, NUM_LOT, "
                    "ETAT_MP, COULEUR, QUANTITE, TYPE_STOCKAGE, QUALITE "
                    "FROM MATIERES_PREMIERES "
                    "ORDER BY CODE_MP ASC",
                    db);

    if (model->lastError().isValid()) {
        qDebug() << "Erreur Tri Matières :" << model->lastError().text();
    }
    return model;
}
