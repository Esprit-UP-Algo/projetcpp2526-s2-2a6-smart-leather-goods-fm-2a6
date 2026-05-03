#include "sensitivityengine.h"

#include <algorithm>

double SensitivityEngine::materialWithDelta(double baseMaterialTnd, double percentDelta)
{
    const double b = std::max(0.01, baseMaterialTnd);
    const double m = b * (1.0 + percentDelta / 100.0);
    return std::max(0.01, m);
}
