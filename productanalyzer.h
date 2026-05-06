#ifndef PRODUCTANALYZER_H
#define PRODUCTANALYZER_H

#include <QString>

struct ProductAnalyzerInput {
    QString nomProduit;
    QString positionnement;
    int volumeCible = 200;
    double indiceCout = 40.0;
    double coutMatiereTnd = 32.0;
    QString niveauFinition;
    int delaiSemaines = 6;
    QString risqueFournisseur;
};

struct ProductProfile {
    double indiceMarge = 0.0;
    double pressionImage = 0.0;
    double pressionDelai = 0.0;
    double pressionRisque = 0.0;
};

class ProductAnalyzer
{
public:
    static ProductProfile analyze(const ProductAnalyzerInput &in);
};

#endif
