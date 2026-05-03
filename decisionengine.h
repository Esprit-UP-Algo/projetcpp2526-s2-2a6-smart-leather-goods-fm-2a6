#ifndef DECISIONENGINE_H
#define DECISIONENGINE_H

#include "productanalyzer.h"
#include "tensionevaluator.h"
#include <QJsonObject>

class DecisionEngine
{
public:
    static QJsonObject analyze(const ProductAnalyzerInput &in);
    static QString decide(double coherenceScore, const QVector<ProductTension> &tensions);
};

#endif
