#include "insightengine.h"

#include <algorithm>

static QString microInsight(const QString &type)
{
    const QString t = type.toLower();
    if (t.contains(QStringLiteral("coût")) && t.contains(QStringLiteral("qualité")))
        return QStringLiteral("🎨 Finition chère");
    if (t.contains(QStringLiteral("marge")) && t.contains(QStringLiteral("positionnement")))
        return QStringLiteral("📈 Marge tendue");
    if (t.contains(QStringLiteral("délai")) || t.contains(QStringLiteral("delai")))
        return QStringLiteral("⏱️ Délais courts");
    if (t.contains(QStringLiteral("risque fournisseur")))
        return QStringLiteral("⚠️ Fournisseur fragile");
    if (t.contains(QStringLiteral("image premium")))
        return QStringLiteral("💎 Premium coûteux");
    return QStringLiteral("📊 Veille active");
}

QStringList InsightEngine::topInsights(const QVector<ProductTension> &tensions, int maxItems)
{
    QStringList out;
    const int n = std::min(maxItems, static_cast<int>(tensions.size()));
    for (int i = 0; i < n; ++i)
        out.append(microInsight(tensions.at(i).type));
    while (out.size() < maxItems)
        out.append(QString());
    return out.mid(0, maxItems);
}
