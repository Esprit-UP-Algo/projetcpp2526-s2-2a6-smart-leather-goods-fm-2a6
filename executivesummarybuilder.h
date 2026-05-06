#ifndef EXECUTIVESUMMARYBUILDER_H
#define EXECUTIVESUMMARYBUILDER_H

#include "tensionevaluator.h"
#include <QString>

class ExecutiveSummaryBuilder
{
public:
    static QString build(const QString &decision, double coherenceScore, const QVector<ProductTension> &tensions, const QString &strategie);
};

#endif
