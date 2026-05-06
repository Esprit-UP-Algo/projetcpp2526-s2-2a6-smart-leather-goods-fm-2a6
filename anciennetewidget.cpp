#include "anciennetewidget.h"
#include "ui_anciennetewidget.h"

#include <QDate>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {
void clearLayout(QLayout *lay)
{
    if (!lay) return;
    while (QLayoutItem *it = lay->takeAt(0)) {
        if (QLayout *sl = it->layout()) clearLayout(sl);
        else if (QWidget *w = it->widget()) delete w;
        delete it;
    }
}
} // namespace

AncienneteWidget::AncienneteWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AncienneteWidget)
{
    ui->setupUi(this);
    setMaximumSize(640, 580);

    ui->frame_card->setObjectName(QStringLiteral("frame_card"));
    ui->frame_card->setFixedWidth(620);
    ui->frame_card->setMaximumHeight(560);
    ui->frame_card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    ui->frame_card->setStyleSheet(QStringLiteral(
        "QFrame#frame_card {"
        "  background-color: #1a1208;"
        "  border: 3px solid #c9a030;"
        "  border-radius: 18px;"
        "}"));

    ui->scrollArea_content->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea_content->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_content->setAlignment(Qt::AlignTop);
    ui->scrollArea_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->scrollArea_content->setFrameShape(QFrame::NoFrame);
    ui->scrollArea_content->setStyleSheet(QStringLiteral(
        "QScrollArea { background-color: #1a1208; border: none; }"
        "QScrollBar:vertical { width: 8px; background: #2a1e0e; border-radius: 4px; }"
        "QScrollBar::handle:vertical { min-height: 28px; background: #c9a030; border-radius: 4px; }"));

    if (QWidget *sw = ui->scrollArea_content->widget())
        sw->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}

AncienneteWidget::~AncienneteWidget()
{
    delete ui;
}

void AncienneteWidget::setEmployeData(const AncienneteEmployeData &data)
{
    m_data = data;
    m_data.empty = false;
    rebuildContent();
}

void AncienneteWidget::setEmptyState()
{
    m_data = AncienneteEmployeData{};
    m_data.empty = true;
    rebuildContent();
}

void AncienneteWidget::rebuildContent()
{
    clearLayout(ui->verticalLayout_scrollInner);

    // Root container — solid background so children never bleed onto white
    auto *innerRoot = new QWidget;
    innerRoot->setStyleSheet(QStringLiteral("QWidget { background-color: #1a1208; }"));
    innerRoot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto *rootLay = new QVBoxLayout(innerRoot);
    rootLay->setContentsMargins(18, 16, 18, 18);
    rootLay->setSpacing(14);

    // ── Header strip ──────────────────────────────────────────────────────────
    auto *hero = new QFrame(innerRoot);
    hero->setObjectName(QStringLiteral("ancHeroStrip"));
    hero->setStyleSheet(QStringLiteral(
        "QFrame#ancHeroStrip {"
        "  background-color: #241a08;"
        "  border: 1px solid #c9a030;"
        "  border-radius: 14px;"
        "}"));
    auto *heroLay = new QHBoxLayout(hero);
    heroLay->setContentsMargins(14, 10, 14, 10);
    heroLay->setSpacing(12);

    auto *accent = new QFrame(hero);
    accent->setFixedWidth(5);
    accent->setMinimumHeight(48);
    accent->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #e6b34a; border-radius: 3px; }"));

    auto *tag = new QLabel(QStringLiteral("FIL D'OR · RH"), hero);
    tag->setStyleSheet(QStringLiteral(
        "QLabel { background-color: transparent; color: #e6b34a;"
        " font-size: 10px; font-weight: 800; letter-spacing: 4px; }"));

    auto *heroTitle = new QLabel(QStringLiteral("Ancienneté & primes"), hero);
    heroTitle->setStyleSheet(QStringLiteral(
        "QLabel { background-color: transparent; color: #f5efe6;"
        " font-size: 16px; font-weight: 900; }"));

    auto *heroSub = new QLabel(QStringLiteral("Projection indicative · 50 DT / an d'ancienneté"), hero);
    heroSub->setStyleSheet(QStringLiteral(
        "QLabel { background-color: transparent; color: #c8b48a;"
        " font-size: 12px; font-weight: 600; }"));

    auto *heroTxt = new QVBoxLayout;
    heroTxt->setSpacing(3);
    heroTxt->addWidget(tag);
    heroTxt->addWidget(heroTitle);
    heroTxt->addWidget(heroSub);

    auto *heroBadge = new QLabel(QStringLiteral("SYS"), hero);
    heroBadge->setAlignment(Qt::AlignCenter);
    heroBadge->setFixedSize(38, 38);
    heroBadge->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  background-color: #0d0a05;"
        "  color: #e6b34a;"
        "  border: 1px solid #c9a030;"
        "  border-radius: 14px;"
        "  font-size: 9px; font-weight: 900; letter-spacing: 2px;"
        "}"));

    heroLay->addWidget(accent);
    heroLay->addLayout(heroTxt, 1);
    heroLay->addWidget(heroBadge, 0, Qt::AlignVCenter);

    rootLay->addWidget(hero);

    if (!m_data.empty) {
        // ── Employee name ─────────────────────────────────────────────────────
        const int annees = m_data.dateEmbauche.daysTo(QDate::currentDate()) / 365;
        const double prime = annees * 50.0;
        const int barPct = qMin(100, qMax(0, (annees * 100) / 40));

        auto *nameLb = new QLabel(
            QStringLiteral("%1 %2").arg(m_data.prenom, m_data.nom).trimmed(), innerRoot);
        nameLb->setAlignment(Qt::AlignCenter);
        nameLb->setStyleSheet(QStringLiteral(
            "QLabel {"
            "  background-color: transparent;"
            "  color: #ffe9a8;"
            "  font-size: 18px; font-weight: 900; letter-spacing: 1px;"
            "  padding: 4px 0 2px 0;"
            "}"));
        rootLay->addWidget(nameLb);

        // ── Metric tiles ──────────────────────────────────────────────────────
        auto makeTile = [](const QString &label, const QString &value) -> QFrame* {
            auto *f = new QFrame;
            f->setObjectName(QStringLiteral("ancMetricTile"));
            f->setStyleSheet(QStringLiteral(
                "QFrame#ancMetricTile {"
                "  background-color: #241a08;"
                "  border: 1px solid #c9a030;"
                "  border-radius: 14px;"
                "}"));
            auto *vl = new QVBoxLayout(f);
            vl->setContentsMargins(16, 12, 16, 12);
            vl->setSpacing(6);

            auto *lb = new QLabel(label, f);
            lb->setStyleSheet(QStringLiteral(
                "QLabel { background-color: transparent; color: #a89060;"
                " font-size: 10px; font-weight: 800; letter-spacing: 3px; }"));

            auto *va = new QLabel(value, f);
            va->setWordWrap(true);
            va->setStyleSheet(QStringLiteral(
                "QLabel { background-color: transparent; color: #f5efe6;"
                " font-size: 20px; font-weight: 800; }"));

            vl->addWidget(lb);
            vl->addWidget(va);
            return f;
        };

        auto *tilesRow = new QHBoxLayout;
        tilesRow->setSpacing(12);
        tilesRow->addWidget(makeTile(
            QStringLiteral("DATE D'EMBAUCHE"),
            m_data.dateEmbauche.toString(QStringLiteral("dd / MM / yyyy"))), 1);
        tilesRow->addWidget(makeTile(
            QStringLiteral("ANCIENNETÉ"),
            QStringLiteral("%1 an%2").arg(annees).arg(annees > 1 ? "s" : "")), 1);

        auto *tilesWrap = new QWidget(innerRoot);
        tilesWrap->setStyleSheet(QStringLiteral("QWidget { background-color: transparent; }"));
        tilesWrap->setLayout(tilesRow);
        rootLay->addWidget(tilesWrap);

        // ── Progress bar ──────────────────────────────────────────────────────
        auto *progWrap = new QWidget(innerRoot);
        progWrap->setStyleSheet(QStringLiteral("QWidget { background-color: transparent; }"));
        auto *progLay = new QVBoxLayout(progWrap);
        progLay->setContentsMargins(0, 0, 0, 0);
        progLay->setSpacing(6);

        auto *progLbl = new QLabel(
            QStringLiteral("Référence visuelle — barème 40 ans"), progWrap);
        progLbl->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #c8b48a;"
            " font-size: 11px; font-weight: 700; letter-spacing: 2px; }"));

        auto *bar = new QProgressBar(progWrap);
        bar->setRange(0, 100);
        bar->setValue(barPct);
        bar->setTextVisible(false);
        bar->setFixedHeight(12);
        bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        bar->setStyleSheet(QStringLiteral(
            "QProgressBar {"
            "  border: 1px solid #8b6914;"
            "  border-radius: 6px;"
            "  background-color: #0d0a05;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 5px;"
            "  background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "    stop:0 #9a7520, stop:0.5 #e6b34a, stop:1 #ffe08a);"
            "}"));

        progLay->addWidget(progLbl);
        progLay->addWidget(bar);
        rootLay->addWidget(progWrap);

        // ── Prime panel ───────────────────────────────────────────────────────
        auto *primeFrame = new QFrame(innerRoot);
        primeFrame->setObjectName(QStringLiteral("ancPrimePanel"));
        primeFrame->setStyleSheet(QStringLiteral(
            "QFrame#ancPrimePanel {"
            "  background-color: #2a1e08;"
            "  border: 2px solid #e6b34a;"
            "  border-radius: 20px;"
            "}"));

        auto *primeFx = new QGraphicsDropShadowEffect(primeFrame);
        primeFx->setBlurRadius(28);
        primeFx->setOffset(0, 4);
        primeFx->setColor(QColor(230, 179, 74, 120));
        primeFrame->setGraphicsEffect(primeFx);

        auto *primeLay = new QVBoxLayout(primeFrame);
        primeLay->setContentsMargins(16, 12, 16, 16);
        primeLay->setSpacing(4);

        auto *primeTag = new QLabel(QStringLiteral("OUTPUT · PRIME ESTIMÉE"), primeFrame);
        primeTag->setAlignment(Qt::AlignCenter);
        primeTag->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #e6b34a;"
            " font-size: 11px; font-weight: 800; letter-spacing: 4px; }"));

        auto *primeRow = new QHBoxLayout;
        primeRow->addStretch();

        auto *primeNum = new QLabel(QString::number(prime, 'f', 0), primeFrame);
        primeNum->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #ffe9a8;"
            " font-size: 42px; font-weight: 900; }"));

        auto *primeUnit = new QLabel(QStringLiteral("DT"), primeFrame);
        primeUnit->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #f5efe6;"
            " font-size: 20px; font-weight: 800; padding-left: 8px; padding-top: 14px; }"));

        primeRow->addWidget(primeNum);
        primeRow->addWidget(primeUnit);
        primeRow->addStretch();

        primeLay->addWidget(primeTag);
        primeLay->addLayout(primeRow);

        rootLay->addWidget(primeFrame);

    } else {
        // ── Empty state ───────────────────────────────────────────────────────
        auto *emptyCard = new QFrame(innerRoot);
        emptyCard->setObjectName(QStringLiteral("ancEmptyCard"));
        emptyCard->setStyleSheet(QStringLiteral(
            "QFrame#ancEmptyCard {"
            "  background-color: #1e1508;"
            "  border: 1px dashed #8b6914;"
            "  border-radius: 16px;"
            "}"));
        auto *ev = new QVBoxLayout(emptyCard);
        ev->setContentsMargins(28, 32, 28, 32);
        ev->setSpacing(12);

        auto *t1 = new QLabel(QStringLiteral("Aucun employé sélectionné"), emptyCard);
        t1->setAlignment(Qt::AlignCenter);
        t1->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #f5efe6;"
            " font-size: 17px; font-weight: 800; }"));

        auto *t2 = new QLabel(
            QStringLiteral("Choisissez une ligne dans « Liste du Personnel », puis rouvrez cet onglet."),
            emptyCard);
        t2->setAlignment(Qt::AlignCenter);
        t2->setWordWrap(true);
        t2->setStyleSheet(QStringLiteral(
            "QLabel { background-color: transparent; color: #c8b48a;"
            " font-size: 13px; }"));

        ev->addWidget(t1);
        ev->addWidget(t2);
        rootLay->addWidget(emptyCard);
    }

    ui->verticalLayout_scrollInner->addWidget(innerRoot);

    if (QWidget *sw = ui->scrollArea_content->widget()) {
        sw->adjustSize();
        sw->updateGeometry();
    }
}
