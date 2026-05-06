#ifndef INSIGHTENGINE_H
#define INSIGHTENGINE_H

#include "tensionevaluator.h"
#include <QStringList>

class InsightEngine
{
public:
    static QStringList topInsights(const QVector<ProductTension> &tensions, int maxItems = 3);
};

#endif
