#include "breakpointengine.h"

#include "simulationengine.h"

#include <QLocale>
#include <algorithm>

static ProductAnalyzerInput cloneAdjustedMat(const ProductAnalyzerInput &ref, double materialTnd)
{
    ProductAnalyzerInput d = ref;
    d.coutMatiereTnd = materialTnd;
    d.indiceCout = std::clamp(materialTnd / 0.45, 8.0, 94.0);
    d.positionnement = d.indiceCout >= 62.0 ? QStringLiteral("premium")
                                            : (d.indiceCout >= 38.0 ? QStringLiteral("aspirationnel")
                                                                    : QStringLiteral("accessible"));
    d.niveauFinition = d.indiceCout >= 68.0 ? QStringLiteral("élevé") : QStringLiteral("standard");
    return d;
}

QString BreakpointEngine::materialThresholdLine(const ProductAnalyzerInput &reference)
{
    const SimulationOutcome base = SimulationEngine::compute(reference);
    const QString d0 = base.decisionRaw;
    const double m0 = std::max(0.01, reference.coutMatiereTnd);
    for (int step = 1; step <= 80; ++step) {
        const double pct = step * 0.5;
        const double m = m0 * (1.0 + pct / 100.0);
        const ProductAnalyzerInput d = cloneAdjustedMat(reference, m);
        const SimulationOutcome o = SimulationEngine::compute(d);
        if (o.decisionRaw != d0) {
            return QStringLiteral("Seuil : +%1% matière")
                .arg(QLocale::system().toString(pct, 'f', (pct < 10.0 ? 1 : 0)));
        }
    }
    return QStringLiteral("Seuil : >40% matière");
}
