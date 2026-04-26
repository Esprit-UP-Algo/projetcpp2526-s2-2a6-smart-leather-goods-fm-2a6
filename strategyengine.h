#ifndef STRATEGYENGINE_H
#define STRATEGYENGINE_H

#include "tensionevaluator.h"
#include <QString>

class StrategyEngine
{
public:
    static QString recommend(double coherenceScore, const QVector<ProductTension> &tensions);
};

#endif
