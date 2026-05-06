#include "costengine.h"

#include <algorithm>
#include <cmath>

CostSnapshot CostEngine::estimate(const ProductAnalyzerInput &in, const ProductProfile &profile)
{
    CostSnapshot s;
    const double mat = std::max(0.01, in.coutMatiereTnd);
    const int vol = std::max(1, in.volumeCible);
    const double serie = std::clamp(0.88 + 0.028 * std::log(static_cast<double>(vol)), 0.82, 1.12);
    const double finitionBump = (in.niveauFinition == QStringLiteral("élevé")) ? 1.12 : 1.0;
    const double delaiStress = std::clamp(1.0 + (7 - in.delaiSemaines) * 0.018, 0.94, 1.14);
    s.coutUnitaireTnd = mat * 1.22 * serie * finitionBump * delaiStress + 4.5;
    s.margePct = std::clamp(profile.indiceMarge, 5.0, 92.0);
    s.risqueScore = std::clamp(profile.pressionRisque, 0.0, 100.0);
    return s;
}
