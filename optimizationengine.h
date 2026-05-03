#ifndef OPTIMIZATIONENGINE_H
#define OPTIMIZATIONENGINE_H

#include "costengine.h"

class OptimizationEngine
{
public:
    static CostSnapshot propose(const CostSnapshot &actuel, const ProductAnalyzerInput &in);
};

#endif
