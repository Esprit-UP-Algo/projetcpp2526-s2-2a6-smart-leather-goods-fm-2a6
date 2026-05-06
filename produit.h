#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Produit
{
    int id_produit;              // NUMBER (Not Null)
    QString designation;         // VARCHAR2(100)
    double cout;                 // NUMBER(10,2)
    QString collection;          // VARCHAR2(50)
    QString type_cuir_requis;    // VARCHAR2(50)
    int temps_fabrication;       // NUMBER
    int id_client;               // NUMBER
    int id_emplacement;          // NUMBER
    QString m_derniereErreurSaisie;

public:
    Produit();

    Produit(int id_produit,
            QString designation,
            double cout,
            QString collection,
            QString type_cuir_requis,
            int temps_fabrication,
            int id_client,
            int id_emplacement);

    QString derniereErreurSaisie() const { return m_derniereErreurSaisie; }

    // CRUD
    bool ajouter();
    /// Liste des produits avec client et dépôt (LEFT JOIN). \a filtreCollection filtre sur UPPER(COLLECTION) si non vide.
    QSqlQueryModel *afficher(const QString &filtreCollection = QString());
    /// Supprime d'abord les lignes PLANIFICATION liées, puis le produit. \a messageErreur reçoit le texte SQL si échec.
    bool supprimer(int id, QString *messageErreur = nullptr);
    bool modifier(int id);

    // Métier Smart (Arduino)
    static bool ensureChoixColumn();
    static int getProductChoix(int productId);
    static bool setProductChoix(int productId, int choix);
    static QString getProductDesignation(int productId);

    // Config Arduino persistée (table ARDUINO_CONFIG)
    static QString getArduinoConfig(const QString &cle, const QString &defaut = QString());
    static bool setArduinoConfig(const QString &cle, const QString &valeur);

    // Journalisation moteur enrichie (table MOTEUR_LOGS)
    static bool logMoteurAction(int productId,
                                int choix,
                                const QString &arduinoResponse = QString(),
                                const QString &commande = QString(),
                                const QString &portCom = QString(),
                                const QString &statut = QStringLiteral("DONE"),
                                int dureeMs = -1,
                                int idCommande = -1);

    static bool updateLatestPendingMoteurLog(int productId,
                                             const QString &commande,
                                             const QString &arduinoResponse,
                                             const QString &statut,
                                             int dureeMs = -1);
};

#endif // PRODUIT_H
