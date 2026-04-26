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
    static bool logMoteurAction(int productId, int choix);
};

#endif
