#ifndef ETAPE_H
#define ETAPE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel> // <--- AJOUTER CECI

class Etape
{
    int id_planification;
    int id_employe;
    QString etape_actuelle;
    double temps_reel;
    double delta;
    int alerte_active;

public:
    Etape(int id_plan, int id_emp, QString etape, double temps, double dlt, int alrt);
    bool ajouter();
    // NOUVELLE FONCTION POUR L'HISTORIQUE
    static QSqlQueryModel* afficherHistorique(int idPlanif);
};

#endif // ETAPE_H
