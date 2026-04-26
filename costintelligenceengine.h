#ifndef COSTINTELLIGENCEENGINE_H
#define COSTINTELLIGENCEENGINE_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QUrl>

class QNetworkReply;

struct ProductCostInputSnapshot {
    double baseCost = 0.0;
    QString materialType;
    double productionTimeHours = 0;
    QString supplierRiskLevel;
    QString packagingLevel;
    QString finishingLevel;
};

struct SimulationResultSnapshot {
    double avgCost = 0;
    double worstCaseCost = 0;
    double bestCaseCost = 0;
    double riskScore = 0;
    double profitabilityScore = 0;
    QString recommendationText;
};

class CostIntelligenceEngine : public QObject
{
    Q_OBJECT

public:
    explicit CostIntelligenceEngine(QNetworkAccessManager *nam, QObject *parent = nullptr);

    static QJsonObject applyWhatIfMaterialPlus10Percent(const QJsonObject &advancedSimulatePayload);
    static QJsonObject applyWhatIfProductionTimeMinus8Percent(const QJsonObject &advancedSimulatePayload);
    static QJsonObject applyWhatIfCombinedBenchmark(const QJsonObject &advancedSimulatePayload);

    void abort();

public slots:
    void startSimulation(const QUrl &fashionOracleRootUrl,
                         const QJsonObject &advancedSimulatePayload,
                         int monteCarloRuns = 750,
                         int monteCarloSeed = -1);

signals:
    void simulationStarted();
    void simulationFinished(const QJsonObject &result);
    void simulationFailed(const QString &errorMessage);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_nam = nullptr;
    QPointer<QNetworkReply> m_reply;
};

#endif
