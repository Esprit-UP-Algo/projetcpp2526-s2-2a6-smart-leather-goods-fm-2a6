#include "productanalyzer.h"

#include <algorithm>

ProductProfile ProductAnalyzer::analyze(const ProductAnalyzerInput &in)
{
    ProductProfile p;
    const double premiumBias = (in.positionnement == QStringLiteral("premium")) ? 1.0
        : (in.positionnement == QStringLiteral("aspirationnel") ? 0.6 : 0.25);
    p.indiceMarge = std::clamp(68.0 - in.indiceCout * 0.55 + premiumBias * 10.0, 8.0, 82.0);
    p.pressionImage = std::clamp(in.indiceCout * 0.65 - premiumBias * 22.0, 0.0, 100.0);
    p.pressionDelai = std::clamp((7.0 - in.delaiSemaines) * 14.0
        + (in.niveauFinition == QStringLiteral("élevé") ? 18.0 : 6.0), 0.0, 100.0);
    p.pressionRisque = std::clamp((in.risqueFournisseur == QStringLiteral("élevé") ? 55.0 : 28.0)
        + std::max(0, in.volumeCible - 450) * 0.05, 0.0, 100.0);
    return p;
}
