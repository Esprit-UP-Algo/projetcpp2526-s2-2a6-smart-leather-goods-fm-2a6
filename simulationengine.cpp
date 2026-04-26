#include "simulationengine.h"

#include <algorithm>
#include <cmath>
#include <numeric>

int SimulationEngine::confidencePercent(double coherence, double risqueScore, double avgTension)
{
    const double c = std::clamp(coherence, 0.0, 100.0);
    const double r = std::clamp(risqueScore, 0.0, 100.0);
    const double t = std::clamp(avgTension, 0.0, 100.0);
    const double x = c * 0.46 + (100.0 - r) * 0.34 + (100.0 - t) * 0.20;
    return static_cast<int>(std::round(std::clamp(x, 5.0, 99.0)));
}

SimulationOutcome SimulationEngine::compute(ProductAnalyzerInput din)
{
    SimulationOutcome o;
    const ProductProfile prof = ProductAnalyzer::analyze(din);
    o.prof = prof;
    o.tensions = TensionEvaluator::evaluate(din, prof);
    const double avgT = o.tensions.isEmpty()
        ? 0.0
        : std::accumulate(o.tensions.begin(), o.tensions.end(), 0.0,
                          [](double s, const ProductTension &t) { return s + t.score; })
              / static_cast<double>(o.tensions.size());
    o.coherence = std::clamp(100.0 - avgT * 0.78 - std::max(0.0, 48.0 - prof.indiceMarge) * 0.55, 5.0, 96.0);
    o.decisionRaw = DecisionEngine::decide(o.coherence, o.tensions);
    o.act = CostEngine::estimate(din, prof);
    o.opt = OptimizationEngine::propose(o.act, din);
    o.score = ScoreEngine::globalProductScore(o.coherence, o.act.risqueScore);
    o.confidencePct = confidencePercent(o.coherence, o.act.risqueScore, avgT);
    return o;
}
