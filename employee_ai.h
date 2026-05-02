#ifndef EMPLOYEE_AI_H
#define EMPLOYEE_AI_H

#include <QDate>
#include <QMap>
#include <QString>
#include <QVector>

struct Employee
{
    QString id;
    QString nom;
    QString prenom;
    QString poste;
    QString departement;
    QDate dateEmbauche;
    double salaire = 0.0;

    // Critères IA (0..100)
    float experience = 0.0f;
    float performance = 0.0f;
    float formation = 0.0f;
    float discipline = 0.0f;
    float productivite = 0.0f;
};

struct ClassificationResult
{
    float score = 0.0f;
    QString niveau;
    QString classification;
    QString recommandation;
    QString resume;
    QMap<QString, float> criteres;

    /// Préuve détaillée (3 références A/B/C + distances) — affichage UI.
    QString preuveAbc;
    QString refA_id;
    QString refA_nom;
    QString refB_id;
    QString refB_nom;
    QString refC_id;
    QString refC_nom;
    float distanceA = -1.0f;
    float distanceB = -1.0f;
    float distanceC = -1.0f;
    QString plusProcheId;
    QString plusProcheNom;
    QString lettreFinale;
};

class AIClassifier
{
public:
    static float calculerScore(const Employee &e);
    static QString classerNiveau(float score);
    static QString genererRecommandation(const Employee &e);
    static QString genererClassification(float score);
    static ClassificationResult analyser(const Employee &e);
    /// Classification par distance minimale vers 3 références (une par niveau A, B, C).
    static ClassificationResult classifyWithThreeReferences(const Employee &current,
                                                          const QVector<Employee> &base,
                                                          const QString &selfId = QString());
    static ClassificationResult analyserAvecBase(const Employee &e, const QVector<Employee> &base, const QString &selfId = QString());

    static ClassificationResult simulerFormation(const Employee &e, float gainFormation);

    static QVector<Employee> topEmployes(const QVector<Employee> &employes, int limite = 5);
    static QVector<Employee> employesARisque(const QVector<Employee> &employes, float seuil = 50.0f);
};

#endif // EMPLOYEE_AI_H
