#include "optimizationengine.h"

#include <algorithm>

CostSnapshot OptimizationEngine::propose(const CostSnapshot &actuel, const ProductAnalyzerInput &in)
{
    CostSnapshot o;
    const int vol = std::max(1, in.volumeCible);
    const double volBonus = std::clamp(0.06 + (vol > 400 ? 0.04 : 0.0) + (vol > 750 ? 0.03 : 0.0), 0.04, 0.16);
    o.coutUnitaireTnd = actuel.coutUnitaireTnd * (1.0 - volBonus);
    const double margeGain = std::clamp(3.5 + volBonus * 42.0, 2.0, 14.0);
    o.margePct = std::min(92.0, actuel.margePct + margeGain);
    o.risqueScore = std::max(0.0, actuel.risqueScore - (in.risqueFournisseur == QStringLiteral("élevé") ? 8.0 : 12.0));
    return o;
}
