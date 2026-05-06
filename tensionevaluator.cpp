#include "tensionevaluator.h"

#include <algorithm>

static QString level(double s)
{
    if (s >= 70.0) return QStringLiteral("critique");
    if (s >= 45.0) return QStringLiteral("modéré");
    return QStringLiteral("faible");
}

QVector<ProductTension> TensionEvaluator::evaluate(const ProductAnalyzerInput &in, const ProductProfile &p)
{
    QVector<ProductTension> out;
    out.push_back({QStringLiteral("coût vs qualité"), level(p.pressionImage),
                   QStringLiteral("Le niveau de coût actuel compresse la promesse qualité perçue."), p.pressionImage});
    out.push_back({QStringLiteral("marge vs positionnement"), level(100.0 - p.indiceMarge),
                   QStringLiteral("La marge potentielle et le territoire de marque ne sont pas totalement alignés."),
                   100.0 - p.indiceMarge});
    out.push_back({QStringLiteral("délai vs finition"), level(p.pressionDelai),
                   QStringLiteral("Le délai cible met sous tension la qualité de finition attendue."), p.pressionDelai});
    out.push_back({QStringLiteral("risque fournisseur vs volume"), level(p.pressionRisque),
                   QStringLiteral("Le couple fournisseur/volume augmente la fragilité opérationnelle."), p.pressionRisque});
    if (in.positionnement == QStringLiteral("premium")) {
        const double s = std::clamp(p.pressionImage * 0.8 + p.pressionRisque * 0.35, 0.0, 100.0);
        out.push_back({QStringLiteral("image premium vs structure de coût"), level(s),
                       QStringLiteral("Le discours premium doit être sécurisé par une base économique plus stable."), s});
    }
    std::sort(out.begin(), out.end(), [](const ProductTension &a, const ProductTension &b) { return a.score > b.score; });
    if (out.size() > 5) out.resize(5);
    return out;
}
