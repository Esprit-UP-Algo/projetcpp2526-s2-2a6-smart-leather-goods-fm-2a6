#ifndef TENSIONEVALUATOR_H
#define TENSIONEVALUATOR_H

#include "productanalyzer.h"
#include <QString>
#include <QVector>

struct ProductTension {
    QString type;
    QString niveau;
    QString explication;
    double score = 0.0;
};

class TensionEvaluator
{
public:
    static QVector<ProductTension> evaluate(const ProductAnalyzerInput &in, const ProductProfile &p);
};

#endif
