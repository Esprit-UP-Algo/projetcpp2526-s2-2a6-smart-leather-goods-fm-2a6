#ifndef UIANIMATOR_H
#define UIANIMATOR_H

#include <QString>

class QLabel;
class QProgressBar;
class QWidget;

class UIAnimator
{
public:
    static void pulseDecisionBadge(QWidget *badgeWidget);
    static void animateLabelDouble(QLabel *label, double fromVal, double toVal, int decimals, const QString &suffix, int durationMs);
    static void animateLabelInt(QLabel *label, int fromVal, int toVal, int durationMs);
    static void animateProgress(QProgressBar *bar, int fromVal, int toVal, int durationMs);
};

#endif
