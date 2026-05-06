#include "scoreengine.h"

#include <algorithm>
#include <cmath>

int ScoreEngine::globalProductScore(double coherence0to100, double risque0to100)
{
    const double r = std::clamp(risque0to100, 0.0, 100.0);
    const double c = std::clamp(coherence0to100, 0.0, 100.0);
    const double x = c * 0.72 + (100.0 - r) * 0.28;
    return static_cast<int>(std::round(std::clamp(x, 0.0, 100.0)));
}
