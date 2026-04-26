#include "knn_graph_widget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QToolTip>
#include <algorithm>
#include <cmath>

namespace {
constexpr int K_NEIGHBORS = 3;
}

KnnGraphWidget::KnnGraphWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(500, 400);
    setMouseTracking(true);
    setStyleSheet(QStringLiteral(
        "QToolTip {"
        " background:#1E1A10;"
        " color:#F8F1E3;"
        " border:1px solid #C8972A;"
        " border-radius:6px;"
        " padding:6px;"
        "}"));

    m_refs = {
        {"wechtet1 omar", 8.0, 91.0, "A", QColor("#2ECC71")},
        {"ref_A2", 10.0, 85.0, "A", QColor("#2ECC71")},
        {"ref_A3", 7.0, 88.0, "A", QColor("#2ECC71")},
        {"test1 test", 4.0, 60.0, "B", QColor("#F0C040")},
        {"ref_B2", 3.0, 65.0, "B", QColor("#F0C040")},
        {"ref_B3", 5.0, 58.0, "B", QColor("#F0C040")},
        {"emna bouhachem", 1.0, 38.0, "C", QColor("#E74C3C")},
        {"ref_C2", 2.0, 42.0, "C", QColor("#E74C3C")},
        {"ref_C3", 1.0, 30.0, "C", QColor("#E74C3C")}
    };

    m_pulseTimer = nullptr;
    computeKnnForEmploye();
}

void KnnGraphWidget::setEmploye(const QString &nom, double anciennete, double performance)
{
    m_nomEmploye = nom.trimmed();
    m_empX = std::max(m_xMin, std::min(m_xMax, anciennete));
    m_empY = std::max(m_yMin, std::min(m_yMax, performance));
    m_hasEmploye = true;
    computeKnnForEmploye();
    update();
}

void KnnGraphWidget::setClasseIa(const QString &classeIa)
{
    m_classeIa = classeIa.trimmed();
    update();
}

QRectF KnnGraphWidget::plotRect() const
{
    const qreal left = 70.0;
    const qreal top = 42.0;
    const qreal right = width() - 24.0;
    const qreal bottom = height() - 56.0;
    return QRectF(left, top, std::max(60.0, right - left), std::max(60.0, bottom - top));
}

QPointF KnnGraphWidget::toPixel(double x, double y) const
{
    const QRectF r = plotRect();
    const double nx = (x - m_xMin) / (m_xMax - m_xMin);
    const double ny = (y - m_yMin) / (m_yMax - m_yMin);
    return QPointF(r.left() + nx * r.width(), r.bottom() - ny * r.height());
}

QPointF KnnGraphWidget::clampedDataPoint(double x, double y) const
{
    return QPointF(std::max(m_xMin, std::min(m_xMax, x)),
                   std::max(m_yMin, std::min(m_yMax, y)));
}

QColor KnnGraphWidget::classColor(const QString &classe) const
{
    if (classe == "A")
        return QColor("#2ECC71");
    if (classe == "B")
        return QColor("#F0C040");
    return QColor("#E74C3C");
}

QString KnnGraphWidget::classLabel(const QString &classe) const
{
    if (classe == "A")
        return QStringLiteral("Excellent");
    if (classe == "B")
        return QStringLiteral("Moyen");
    return QStringLiteral("Profil a risque");
}

QString KnnGraphWidget::classForPoint(double x, double y) const
{
    std::vector<DistItem> dists;
    dists.reserve(static_cast<size_t>(m_refs.size()));
    for (int i = 0; i < m_refs.size(); ++i) {
        const double dx = x - m_refs[i].anciennete;
        const double dy = y - m_refs[i].performance;
        dists.push_back({i, std::sqrt(dx * dx + dy * dy)});
    }
    std::sort(dists.begin(), dists.end(), [](const DistItem &a, const DistItem &b) {
        return a.dist < b.dist;
    });

    int countA = 0;
    int countB = 0;
    int countC = 0;
    const int used = std::min(K_NEIGHBORS, static_cast<int>(dists.size()));
    for (int i = 0; i < used; ++i) {
        const QString c = m_refs[dists[i].index].classe;
        if (c == "A")
            ++countA;
        else if (c == "B")
            ++countB;
        else
            ++countC;
    }

    if (countA >= countB && countA >= countC)
        return QStringLiteral("A");
    if (countB >= countA && countB >= countC)
        return QStringLiteral("B");
    return QStringLiteral("C");
}

void KnnGraphWidget::computeKnnForEmploye()
{
    m_voisinsK3.clear();
    m_voisinsDistK3.clear();
    m_classeGagnante = QStringLiteral("B");
    m_couleurGagnante = classColor(m_classeGagnante);
    m_distanceMin = 0.0;
    m_plusProcheIndex = -1;

    const double x = m_hasEmploye ? m_empX : 5.0;
    const double y = m_hasEmploye ? m_empY : 55.0;

    std::vector<DistItem> dists;
    dists.reserve(static_cast<size_t>(m_refs.size()));
    for (int i = 0; i < m_refs.size(); ++i) {
        const double dx = x - m_refs[i].anciennete;
        const double dy = y - m_refs[i].performance;
        dists.push_back({i, std::sqrt(dx * dx + dy * dy)});
    }
    std::sort(dists.begin(), dists.end(), [](const DistItem &a, const DistItem &b) {
        return a.dist < b.dist;
    });

    int countA = 0;
    int countB = 0;
    int countC = 0;
    const int used = std::min(K_NEIGHBORS, static_cast<int>(dists.size()));
    for (int i = 0; i < used; ++i) {
        m_voisinsK3.push_back(dists[i].index);
        m_voisinsDistK3.push_back(dists[i].dist);
        const QString c = m_refs[dists[i].index].classe;
        if (c == "A")
            ++countA;
        else if (c == "B")
            ++countB;
        else
            ++countC;
    }

    if (countA >= countB && countA >= countC)
        m_classeGagnante = QStringLiteral("A");
    else if (countB >= countA && countB >= countC)
        m_classeGagnante = QStringLiteral("B");
    else
        m_classeGagnante = QStringLiteral("C");

    m_couleurGagnante = classColor(m_classeGagnante);
    if (!dists.empty()) {
        m_distanceMin = dists.front().dist;
        m_plusProcheIndex = dists.front().index;
    }
}

void KnnGraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Fond moderne FIL D'OR.
    QLinearGradient bg(rect().topLeft(), rect().bottomRight());
    bg.setColorAt(0.0, QColor("#1a1209"));
    bg.setColorAt(1.0, QColor("#25170b"));
    p.fillRect(rect(), bg);

    const QRectF r = plotRect();

    // Zone principale.
    p.setBrush(QColor(255, 255, 255, 8));
    p.setPen(QPen(QColor("#C8972A"), 1.2));
    p.drawRoundedRect(r, 10.0, 10.0);

    // Titre + explication tres claire X/Y.
    p.setPen(QColor("#F0C040"));
    QFont ft = p.font();
    ft.setPointSize(13);
    ft.setBold(true);
    p.setFont(ft);
    p.drawText(QRectF(0, 8, width(), 24), Qt::AlignCenter, QStringLiteral("Carte employes - Classification IA"));
    ft.setPointSize(10);
    ft.setBold(false);
    p.setFont(ft);
    p.setPen(QColor(245, 238, 220, 200));
    p.drawText(QRectF(0, 30, width(), 18), Qt::AlignCenter,
               QStringLiteral("X = anciennete (annees)   |   Y = performance (%)"));

    // Decoupage visuel en 3 parties A / B / C.
    const qreal oneThird = r.width() / 3.0;
    const QRectF partA(r.left(), r.top(), oneThird, r.height());
    const QRectF partB(r.left() + oneThird, r.top(), oneThird, r.height());
    const QRectF partC(r.left() + (2.0 * oneThird), r.top(), oneThird, r.height());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(46, 204, 113, 24));
    p.drawRoundedRect(partA, 10.0, 10.0);
    p.setBrush(QColor(240, 192, 64, 24));
    p.drawRect(partB);
    p.setBrush(QColor(231, 76, 60, 24));
    p.drawRoundedRect(partC, 10.0, 10.0);

    // Labels grandes zones (lisibles).
    QFont fz = p.font();
    fz.setPointSize(19);
    fz.setBold(true);
    p.setFont(fz);
    p.setPen(QColor(46, 204, 113, 120));
    p.drawText(QRectF(partA.left(), partA.top() + 12, partA.width(), 28), Qt::AlignCenter, QStringLiteral("ZONE A"));
    p.setPen(QColor(240, 192, 64, 120));
    p.drawText(QRectF(partB.left(), partB.top() + 12, partB.width(), 28), Qt::AlignCenter, QStringLiteral("ZONE B"));
    p.setPen(QColor(231, 76, 60, 120));
    p.drawText(QRectF(partC.left(), partC.top() + 12, partC.width(), 28), Qt::AlignCenter, QStringLiteral("ZONE C"));

    // Grille.
    p.setPen(QPen(QColor(200, 151, 42, 46), 0.8));
    for (int x = 0; x <= 10; x += 2)
        p.drawLine(toPixel(x, m_yMin), toPixel(x, m_yMax));
    for (int y = 0; y <= 100; y += 20)
        p.drawLine(toPixel(m_xMin, y), toPixel(m_xMax, y));

    // Axes + valeurs mieux lisibles.
    p.setPen(QPen(QColor("#C8972A"), 1.4));
    p.drawRoundedRect(r, 10.0, 10.0);
    QFont fa = p.font();
    fa.setPointSize(11);
    fa.setBold(true);
    p.setFont(fa);
    p.setPen(QColor("#F5EEDC"));
    for (int x = 0; x <= 10; x += 2) {
        const QPointF px = toPixel(x, 0);
        p.drawText(QRectF(px.x() - 12, r.bottom() + 4, 24, 18), Qt::AlignCenter, QString::number(x));
    }
    for (int y = 0; y <= 100; y += 20) {
        const QPointF py = toPixel(0, y);
        p.drawText(QRectF(r.left() - 34, py.y() - 9, 26, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(y));
    }

    QFont faxis = p.font();
    faxis.setPointSize(12);
    faxis.setBold(true);
    p.setFont(faxis);
    p.setPen(QColor("#F0C040"));
    p.drawText(QRectF(r.left(), r.bottom() + 22, r.width(), 20), Qt::AlignCenter, QStringLiteral("Anciennete (annees)"));
    p.save();
    p.translate(24, r.center().y());
    p.rotate(-90);
    p.drawText(QRectF(-90, -12, 180, 22), Qt::AlignCenter, QStringLiteral("Performance (%)"));
    p.restore();

    // Points references.
    m_refScreenPoints.resize(m_refs.size());
    for (int i = 0; i < m_refs.size(); ++i) {
        const QPointF pt = toPixel(m_refs[i].anciennete, m_refs[i].performance);
        m_refScreenPoints[i] = pt;
        // Duplication legere (ombre) pour un rendu plus vivant/futuriste.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 28));
        p.drawEllipse(pt + QPointF(3.0, 2.0), 4.6, 4.6);
        p.setBrush(m_refs[i].color);
        p.drawEllipse(pt, 5.0, 5.0);
    }

    // Classe affichee: alignee sur l'IA globale si disponible.
    QString classeAffichee = m_classeGagnante;
    if (!m_classeIa.isEmpty()) {
        const QString c = m_classeIa.toUpper();
        if (c == QStringLiteral("S") || c == QStringLiteral("A"))
            classeAffichee = QStringLiteral("A");
        else if (c == QStringLiteral("B"))
            classeAffichee = QStringLiteral("B");
        else if (c == QStringLiteral("C"))
            classeAffichee = QStringLiteral("C");
    }

    // Point selectionne bleu + effet shine.
    if (m_hasEmploye) {
        // IMPORTANT: on garde le calcul KNN inchangé, mais on contraint l'affichage
        // dans la zone visuelle de la classe finale pour eviter toute contradiction UI.
        double displayX = m_empX;
        if (classeAffichee == QStringLiteral("A"))
            displayX = std::max(0.5, std::min(3.5, m_empX));
        else if (classeAffichee == QStringLiteral("B"))
            displayX = std::max(4.2, std::min(7.8, m_empX));
        else
            displayX = std::max(8.2, std::min(11.6, m_empX));
        m_empScreenPoint = toPixel(displayX, m_empY);

        // Lignes KNN visibles vers les 3 plus proches voisins (style moderne).
        for (int i = 0; i < m_voisinsK3.size(); ++i) {
            const int idx = m_voisinsK3[i];
            if (idx < 0 || idx >= m_refScreenPoints.size())
                continue;

            const QPointF v = m_refScreenPoints[idx];
            p.setPen(QPen(QColor(58, 142, 255, 210), 1.2, Qt::DashLine));
            p.drawLine(m_empScreenPoint, v);

            // Petit badge numerote pres du voisin (1..3).
            const QPointF nPos = v + QPointF(10.0, -10.0);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(10, 8, 6, 220));
            p.drawEllipse(nPos, 8.0, 8.0);
            p.setPen(QColor("#F0C040"));
            QFont fn = p.font();
            fn.setPointSize(8);
            fn.setBold(true);
            p.setFont(fn);
            p.drawText(QRectF(nPos.x() - 8.0, nPos.y() - 8.0, 16.0, 16.0), Qt::AlignCenter, QString::number(i + 1));
        }

        // Halo externe (shine).
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(59, 130, 246, 38));
        p.drawEllipse(m_empScreenPoint, 22.0, 22.0);
        p.setBrush(QColor(59, 130, 246, 72));
        p.drawEllipse(m_empScreenPoint, 15.0, 15.0);
        p.setBrush(QColor(59, 130, 246, 80));
        p.drawEllipse(m_empScreenPoint, 13.0, 13.0);
        p.setBrush(QColor("#2563EB"));
        p.setPen(QPen(QColor("#1D4ED8"), 1.6));
        p.drawEllipse(m_empScreenPoint, 6.0, 6.0);

        // Anneau de classe autour du point pour le mettre en avant.
        p.setPen(QPen(classColor(classeAffichee), 2.0));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(m_empScreenPoint, 9.5, 9.5);

        // Etiquette claire sur l'employe selectionne.
        p.setPen(QColor("#FFFFFF"));
        QFont fe = p.font();
        fe.setPointSize(10);
        fe.setBold(true);
        p.setFont(fe);
        p.drawText(QRectF(m_empScreenPoint.x() + 8.0, m_empScreenPoint.y() - 16.0, 180.0, 18.0),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   m_nomEmploye.isEmpty() ? QStringLiteral("Employe selectionne") : m_nomEmploye);

        // Badge clair "VOUS" pour eviter toute confusion avec les references.
        const QRectF badgeRect(m_empScreenPoint.x() - 18.0, m_empScreenPoint.y() - 30.0, 36.0, 14.0);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(10, 8, 6, 210));
        p.drawRoundedRect(badgeRect, 5.0, 5.0);
        p.setPen(QColor("#F0C040"));
        QFont fv = p.font();
        fv.setPointSize(8);
        fv.setBold(true);
        p.setFont(fv);
        p.drawText(badgeRect, Qt::AlignCenter, QStringLiteral("VOUS"));
    } else {
        m_empScreenPoint = QPointF();
    }

    // Carte decision deplacee en bas a droite (evite de cacher les points).
    const QRectF info(width() - 250, height() - 130, 232, 102);
    p.setPen(QPen(classColor(classeAffichee), 1.2));
    p.setBrush(QColor(10, 8, 6, 190));
    p.drawRoundedRect(info, 9.0, 9.0);
    p.setPen(QColor("#F8EDCF"));
    QFont fi = p.font();
    fi.setPointSize(10);
    fi.setBold(true);
    p.setFont(fi);
    p.drawText(info.adjusted(10, 8, -8, 0), Qt::AlignLeft, QStringLiteral("Decision"));
    fi.setBold(false);
    p.setFont(fi);
    p.drawText(info.adjusted(10, 28, -8, -46), Qt::AlignLeft,
               QStringLiteral("Classe: %1 (%2)").arg(classeAffichee, classLabel(classeAffichee)));
    p.drawText(info.adjusted(10, 48, -8, -26), Qt::AlignLeft,
               QStringLiteral("Distance min: %1").arg(QString::number(m_distanceMin, 'f', 2)));
    p.drawText(info.adjusted(10, 68, -8, -8), Qt::AlignLeft,
               QStringLiteral("Base: 9 references"));
}

void KnnGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF pos = event->pos();
    QString tip;
    QString classeAffichee = m_classeGagnante;
    if (!m_classeIa.isEmpty()) {
        const QString c = m_classeIa.toUpper();
        if (c == QStringLiteral("S") || c == QStringLiteral("A"))
            classeAffichee = QStringLiteral("A");
        else if (c == QStringLiteral("B"))
            classeAffichee = QStringLiteral("B");
        else if (c == QStringLiteral("C"))
            classeAffichee = QStringLiteral("C");
    }

    for (int i = 0; i < m_refScreenPoints.size() && i < m_refs.size(); ++i) {
        const double d = std::hypot(pos.x() - m_refScreenPoints[i].x(), pos.y() - m_refScreenPoints[i].y());
        if (d <= 10.0) {
            const RefPoint &rp = m_refs[i];
            tip = QStringLiteral("Reference: %1\nClasse reference: %2\nAnciennete: %3\nPerformance: %4")
                      .arg(rp.nom, rp.classe,
                           QString::number(rp.anciennete, 'f', 1),
                           QString::number(rp.performance, 'f', 0));
            break;
        }
    }

    if (!tip.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QToolTip::showText(event->globalPosition().toPoint(), tip, this);
#else
        QToolTip::showText(event->globalPos(), tip, this);
#endif
    } else if (m_hasEmploye) {
        const double d = std::hypot(pos.x() - m_empScreenPoint.x(), pos.y() - m_empScreenPoint.y());
        if (d <= 12.0) {
            const QString empTip = QStringLiteral("Employe selectionne: %1\nClasse finale: %2 (%3)\nX anciennete: %4\nY performance: %5")
                                       .arg(m_nomEmploye.isEmpty() ? QStringLiteral("selectionne") : m_nomEmploye,
                                            classeAffichee,
                                            classLabel(classeAffichee),
                                            QString::number(m_empX, 'f', 1),
                                            QString::number(m_empY, 'f', 0));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QToolTip::showText(event->globalPosition().toPoint(), empTip, this);
#else
            QToolTip::showText(event->globalPos(), empTip, this);
#endif
        } else {
            QToolTip::hideText();
        }
    } else {
        QToolTip::hideText();
    }

    QWidget::mouseMoveEvent(event);
}

void KnnGraphWidget::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QWidget::leaveEvent(event);
}
