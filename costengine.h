#ifndef COSTENGINE_H
#define COSTENGINE_H

#include "productanalyzer.h"
#include <QString>

struct CostSnapshot {
    double coutUnitaireTnd = 0.0;
    double margePct = 0.0;
    double risqueScore = 0.0;
};

class CostEngine
{
public:
    static CostSnapshot estimate(const ProductAnalyzerInput &in, const ProductProfile &profile);
};

#endif
