#include "strategyengine.h"

QString StrategyEngine::recommend(double coherenceScore, const QVector<ProductTension> &tensions)
{
    const double top = tensions.isEmpty() ? 0.0 : tensions.first().score;
    if (coherenceScore >= 72.0 && top < 55.0)
        return QStringLiteral("Stratégie d’accélération: lancer rapidement, sécuriser l’exécution et capter la traction marché.");
    if (coherenceScore >= 58.0 && top < 72.0)
        return QStringLiteral("Stratégie marge/image: lancer sous conditions avec cadrage prix et finition priorisée.");
    if (top >= 75.0)
        return QStringLiteral("Stratégie sécurisation: reconfigurer la proposition avant engagement volume.");
    return QStringLiteral("Stratégie de repositionnement: réaligner promesse, coût et exposition fournisseur.");
}
