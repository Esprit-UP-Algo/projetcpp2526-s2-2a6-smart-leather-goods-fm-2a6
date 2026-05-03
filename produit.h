#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Produit
{
    int id_produit;
    QString designation;
    double cout;
    QString collection;
    QString type_cuir_requis;
    int temps_fabrication;
    int id_client;
    int id_emplacement;
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

    bool ajouter();
    QSqlQueryModel *afficher(const QString &filtreCollection = QString());
    bool supprimer(int id, QString *messageErreur = nullptr);
    bool modifier(int id);

    // Métier Smart (Arduino)
    static bool ensureChoixColumn();
    static int getProductChoix(int productId);      // retourne 0,1,2 ou -1 (erreur)
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

    /// Met à jour la dernière ligne PENDING (PRODUCT_ID + COMMANDE) — équivalent UPDATE … STATUT / ARDUINO_RESPONSE.
    static bool updateLatestPendingMoteurLog(int productId,
                                             const QString &commande,
                                             const QString &arduinoResponse,
                                             const QString &statut,
                                             int dureeMs = -1);
};

#endif
