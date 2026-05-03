#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include "costengine.h"
#include "decisionengine.h"
#include "optimizationengine.h"
#include "productanalyzer.h"
#include "scoreengine.h"
#include "tensionevaluator.h"
#include <QVector>

struct SimulationOutcome {
    QString decisionRaw;
    double coherence = 0.0;
    ProductProfile prof;
    QVector<ProductTension> tensions;
    CostSnapshot act;
    CostSnapshot opt;
    int score = 0;
    int confidencePct = 0;
};

class SimulationEngine
{
public:
    static SimulationOutcome compute(ProductAnalyzerInput din);
    static int confidencePercent(double coherence, double risqueScore, double avgTension);
};

#endif
