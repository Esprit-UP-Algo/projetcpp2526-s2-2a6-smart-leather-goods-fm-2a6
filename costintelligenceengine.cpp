#include "costintelligenceengine.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>

CostIntelligenceEngine::CostIntelligenceEngine(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
}

QJsonObject CostIntelligenceEngine::applyWhatIfMaterialPlus10Percent(const QJsonObject &advancedSimulatePayload)
{
    QJsonObject o(advancedSimulatePayload);
    const double v = o.value(QStringLiteral("material_unit_cost")).toDouble(0.0) * 1.10;
    o.insert(QStringLiteral("material_unit_cost"), v);
    return o;
}

QJsonObject CostIntelligenceEngine::applyWhatIfProductionTimeMinus8Percent(const QJsonObject &advancedSimulatePayload)
{
    QJsonObject o(advancedSimulatePayload);
    const double v = o.value(QStringLiteral("labor_hours")).toDouble(0.0) * 0.92;
    o.insert(QStringLiteral("labor_hours"), v);
    return o;
}

QJsonObject CostIntelligenceEngine::applyWhatIfCombinedBenchmark(const QJsonObject &advancedSimulatePayload)
{
    return applyWhatIfProductionTimeMinus8Percent(applyWhatIfMaterialPlus10Percent(advancedSimulatePayload));
}

void CostIntelligenceEngine::abort()
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply.clear();
    }
}

void CostIntelligenceEngine::startSimulation(const QUrl &fashionOracleRootUrl,
                                             const QJsonObject &advancedSimulatePayload,
                                             int monteCarloRuns,
                                             int monteCarloSeed)
{
    if (!m_nam) {
        emit simulationFailed(QStringLiteral("NetworkAccessManager non initialisé."));
        return;
    }
    abort();

    QJsonObject body(advancedSimulatePayload);
    body.insert(QStringLiteral("monte_carlo_runs"), monteCarloRuns);
    if (monteCarloSeed >= 0)
        body.insert(QStringLiteral("monte_carlo_seed"), monteCarloSeed);

    QString base = fashionOracleRootUrl.toString(QUrl::RemoveFragment | QUrl::RemoveQuery).trimmed();
    while (base.endsWith(QLatin1Char('/')))
        base.chop(1);
    const QUrl url(base + QStringLiteral("/cost/intelligence"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_nam->post(req, payload);
    m_reply = reply;
    emit simulationStarted();
    connect(reply, &QNetworkReply::finished, this, &CostIntelligenceEngine::onReplyFinished);
}

void CostIntelligenceEngine::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    if (m_reply.data() == reply)
        m_reply.clear();

    const QByteArray bytes = reply->readAll();
    const QVariant codeVar = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    const int code = codeVar.isValid() ? codeVar.toInt() : 0;
    reply->deleteLater();

    if (code != 200 || bytes.isEmpty()) {
        emit simulationFailed(
            QStringLiteral("HTTP %1 — %2")
                .arg(code)
                .arg(QString::fromUtf8(bytes.left(2000))));
        return;
    }

    QJsonParseError pe{};
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &pe);
    if (!doc.isObject()) {
        emit simulationFailed(QStringLiteral("JSON invalide (%1)").arg(pe.errorString()));
        return;
    }
    emit simulationFinished(doc.object());
}
