#ifndef KNN_GRAPH_WIDGET_H
#define KNN_GRAPH_WIDGET_H

#include <QColor>
#include <QPointF>
#include <QTimer>
#include <QVector>
#include <QWidget>

class KnnGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KnnGraphWidget(QWidget *parent = nullptr);

    // Met a jour l'employe selectionne (X=anciennete en annees, Y=performance 0..100).
    void setEmploye(const QString &nom, double anciennete, double performance);
    // Classe attendue par l'analyse IA globale (S/A/B/C). Sert a afficher la coherence.
    void setClasseIa(const QString &classeIa);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct RefPoint {
        QString nom;
        double anciennete = 0.0;
        double performance = 0.0;
        QString classe; // "A", "B", "C"
        QColor color;
    };

    struct DistItem {
        int index = -1;
        double dist = 0.0;
    };

    QVector<RefPoint> m_refs;
    QString m_nomEmploye;
    double m_empX = 0.0;
    double m_empY = 0.0;
    bool m_hasEmploye = false;

    QVector<int> m_voisinsK3;
    QVector<double> m_voisinsDistK3;
    QString m_classeGagnante;
    QString m_classeIa;
    QColor m_couleurGagnante;
    double m_distanceMin = 0.0;
    int m_plusProcheIndex = -1;
    QTimer *m_pulseTimer = nullptr;
    QVector<QPointF> m_refScreenPoints;
    QPointF m_empScreenPoint;

    double m_xMin = 0.0;
    double m_xMax = 12.0;
    double m_yMin = 0.0;
    double m_yMax = 100.0;

    QRectF plotRect() const;
    QPointF toPixel(double x, double y) const;
    QPointF clampedDataPoint(double x, double y) const;

    QString classForPoint(double x, double y) const;
    QString classLabel(const QString &classe) const;
    QColor classColor(const QString &classe) const;
    void computeKnnForEmploye();
};

#endif // KNN_GRAPH_WIDGET_H
