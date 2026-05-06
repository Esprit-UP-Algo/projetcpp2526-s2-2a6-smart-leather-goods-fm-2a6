#include "uianimator.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QLocale>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>

void UIAnimator::pulseDecisionBadge(QWidget *badgeWidget)
{
    if (!badgeWidget)
        return;
    auto *opacity = new QGraphicsOpacityEffect(badgeWidget);
    badgeWidget->setGraphicsEffect(opacity);
    auto *a1 = new QPropertyAnimation(opacity, "opacity", badgeWidget);
    a1->setDuration(100);
    a1->setStartValue(1.0);
    a1->setEndValue(0.78);
    auto *a2 = new QPropertyAnimation(opacity, "opacity", badgeWidget);
    a2->setDuration(160);
    a2->setStartValue(0.78);
    a2->setEndValue(1.0);
    auto *g = new QSequentialAnimationGroup(badgeWidget);
    g->addAnimation(a1);
    g->addAnimation(a2);
    QObject::connect(g, &QSequentialAnimationGroup::finished, badgeWidget, [opacity, badgeWidget]() {
        badgeWidget->setGraphicsEffect(nullptr);
        opacity->deleteLater();
    });
    g->start(QAbstractAnimation::DeleteWhenStopped);
}

void UIAnimator::animateLabelDouble(QLabel *label, double fromVal, double toVal, int decimals, const QString &suffix, int durationMs)
{
    if (!label)
        return;
    auto *anim = new QVariantAnimation(label);
    anim->setDuration(durationMs);
    anim->setStartValue(fromVal);
    anim->setEndValue(toVal);
    QObject::connect(anim, &QVariantAnimation::valueChanged, label, [label, decimals, suffix](const QVariant &v) {
        const double x = v.toDouble();
        label->setText(QStringLiteral("%1%2")
                           .arg(QLocale::system().toString(x, 'f', decimals), suffix));
    });
    QObject::connect(anim, &QVariantAnimation::finished, label, [label, toVal, decimals, suffix, anim]() {
        label->setText(QStringLiteral("%1%2")
                           .arg(QLocale::system().toString(toVal, 'f', decimals), suffix));
        anim->deleteLater();
    });
    anim->start();
}

void UIAnimator::animateLabelInt(QLabel *label, int fromVal, int toVal, int durationMs)
{
    if (!label)
        return;
    auto *anim = new QVariantAnimation(label);
    anim->setDuration(durationMs);
    anim->setStartValue(fromVal);
    anim->setEndValue(toVal);
    QObject::connect(anim, &QVariantAnimation::valueChanged, label, [label](const QVariant &v) {
        label->setText(QString::number(v.toInt()));
    });
    QObject::connect(anim, &QVariantAnimation::finished, label, [label, toVal, anim]() {
        label->setText(QString::number(toVal));
        anim->deleteLater();
    });
    anim->start();
}

void UIAnimator::animateProgress(QProgressBar *bar, int fromVal, int toVal, int durationMs)
{
    if (!bar)
        return;
    auto *anim = new QVariantAnimation(bar);
    anim->setDuration(durationMs);
    anim->setStartValue(fromVal);
    anim->setEndValue(toVal);
    QObject::connect(anim, &QVariantAnimation::valueChanged, bar, [bar](const QVariant &v) {
        bar->setValue(v.toInt());
    });
    QObject::connect(anim, &QVariantAnimation::finished, bar, [bar, toVal, anim]() {
        bar->setValue(toVal);
        anim->deleteLater();
    });
    anim->start();
}
