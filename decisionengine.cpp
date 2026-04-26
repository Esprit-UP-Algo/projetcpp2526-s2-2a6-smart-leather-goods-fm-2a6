#include "decisionengine.h"

#include "executivesummarybuilder.h"
#include "strategyengine.h"
#include "tensionevaluator.h"

#include <algorithm>
#include <numeric>
#include <QJsonArray>

QString DecisionEngine::decide(double coherenceScore, const QVector<ProductTension> &tensions)
{
    const double top = tensions.isEmpty() ? 0.0 : tensions.first().score;
    if (coherenceScore >= 72.0 && top < 55.0) return QStringLiteral("Lancer");
    if (coherenceScore >= 58.0 && top < 72.0) return QStringLiteral("Lancer sous conditions");
    if (coherenceScore >= 45.0) return QStringLiteral("Reconfigurer");
    if (coherenceScore >= 32.0) return QStringLiteral("À risque");
    return QStringLiteral("À éviter");
}

QJsonObject DecisionEngine::analyze(const ProductAnalyzerInput &in)
{
    const ProductProfile p = ProductAnalyzer::analyze(in);
    const QVector<ProductTension> tensions = TensionEvaluator::evaluate(in, p);
    const double avgTension = tensions.isEmpty() ? 0.0 : std::accumulate(tensions.begin(), tensions.end(), 0.0,
        [](double s, const ProductTension &t) { return s + t.score; }) / static_cast<double>(tensions.size());
    const double coherence = std::clamp(100.0 - avgTension * 0.78 - std::max(0.0, 48.0 - p.indiceMarge) * 0.55, 5.0, 96.0);
    const QString decision = decide(coherence, tensions);
    const QString strategie = StrategyEngine::recommend(coherence, tensions);
    const QString synthese = ExecutiveSummaryBuilder::build(decision, coherence, tensions, strategie);

    QJsonArray tensionsJson;
    for (const ProductTension &t : tensions) {
        QJsonObject o;
        o.insert(QStringLiteral("type"), t.type);
        o.insert(QStringLiteral("niveau"), t.niveau);
        o.insert(QStringLiteral("explication"), t.explication);
        tensionsJson.append(o);
    }

    QJsonObject out;
    out.insert(QStringLiteral("decision"), decision);
    out.insert(QStringLiteral("coherenceScore"), coherence);
    out.insert(QStringLiteral("tensions"), tensionsJson);
    out.insert(QStringLiteral("strategie"), strategie);
    out.insert(QStringLiteral("synthese"), synthese);
    return out;
}
