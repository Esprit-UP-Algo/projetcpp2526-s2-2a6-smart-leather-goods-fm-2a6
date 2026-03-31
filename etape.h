#ifndef ETAPE_H
#define ETAPE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Etape
{
    int id_planification;
    int id_employe;
    QString etape_actuelle;
    double temps_reel_passe;
    double delta;
    int alerte_active;

public:
    Etape();
    Etape(int idPlanif, int idEmp, QString etape, double tempsReel, double delta, int alerte);

    // CRUD
    bool ajouter();
    bool modifier(int idSuivi);
    bool supprimer(int idSuivi);
    QSqlQueryModel* afficher();

    // Génération automatique des 4 étapes pour une commande
    static bool genererEtapesCommande(int idPlanification, int idEmploye);

    // Recherche & Tri
    QSqlQueryModel* rechercherParCommande(int idPlanification);
    QSqlQueryModel* rechercherParEtape(const QString &etape);
    QSqlQueryModel* trierParCommande();
    QSqlQueryModel* afficherAlertes();

    // Mise à jour du temps réel + calcul delta
    static bool saisirTempsReel(int idSuivi, double tempsReel, double tempsPrevue);
};

#endif // ETAPE_H
