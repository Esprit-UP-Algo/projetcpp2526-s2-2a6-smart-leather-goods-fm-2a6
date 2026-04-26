#include "executivesummarybuilder.h"

QString ExecutiveSummaryBuilder::build(const QString &decision, double coherenceScore, const QVector<ProductTension> &tensions, const QString &strategie)
{
    const QString topType = tensions.isEmpty() ? QStringLiteral("aucune tension majeure") : tensions.first().type;
    return QStringLiteral(
        "Décision %1 avec un niveau de cohérence de %2/100. "
        "La tension dominante porte sur « %3 », ce qui justifie une exécution sélective. "
        "%4")
        .arg(decision.toLower())
        .arg(QString::number(static_cast<int>(coherenceScore)))
        .arg(topType)
        .arg(strategie);
}
