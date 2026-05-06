#ifndef BREAKPOINTENGINE_H
#define BREAKPOINTENGINE_H

#include "productanalyzer.h"
#include <QString>

class BreakpointEngine
{
public:
    static QString materialThresholdLine(const ProductAnalyzerInput &reference);
};

#endif
