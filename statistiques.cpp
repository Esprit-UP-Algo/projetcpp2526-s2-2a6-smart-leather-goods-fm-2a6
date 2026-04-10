#include "statistiques.h"
#include <QSqlQuery>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QVBoxLayout>
#include <QLabel>

Statistiques::Statistiques(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Statistiques");
    setMinimumSize(800, 500);

    QSqlQuery query;

    query.exec("SELECT COUNT(*) FROM client");
    int total = query.next() ? query.value(0).toInt() : 0;

    query.exec("SELECT COUNT(*) FROM client WHERE points > 500");
    int vip = query.next() ? query.value(0).toInt() : 0;

    query.exec("SELECT AVG(points) FROM client");
    double moyenne = query.next() ? query.value(0).toDouble() : 0;

    // Graphique camembert
    QPieSeries *series = new QPieSeries();
    series->append("VIP", vip);
    series->append("Normal", total - vip);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des clients");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Texte
    QLabel *label = new QLabel(QString("Total: %1\nVIP: %2\nMoyenne points: %3")
                                   .arg(total).arg(vip).arg(moyenne));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px; padding: 20px;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    layout->addWidget(label);
}

Statistiques::~Statistiques() {}
