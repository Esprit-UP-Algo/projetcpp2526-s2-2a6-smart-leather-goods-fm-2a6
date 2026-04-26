#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "depot.h"
#include "employe.h"
#include "client.h"
#include "costintelligenceengine.h"
#include "decisionengine.h"
#include "costengine.h"
#include "optimizationengine.h"
#include "insightengine.h"
#include "scoreengine.h"
#include "productanalyzer.h"
#include "tensionevaluator.h"
#include "simulationengine.h"
#include "sensitivityengine.h"
#include "breakpointengine.h"
#include "uianimator.h"

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QVariant>
#include <QSqlError>
#include <QDate>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QImageReader>
#include <algorithm>
#include <numeric>
#include <QPageSize>
#include <QProgressBar>
#include <QFileInfo>
#include <QDateTime>
#include <QTime>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEvent>
#include <QDateEdit>
#include <QCursor>
#include <cmath>
#include <QLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFrame>
#include <QCheckBox>
#include <QScrollArea>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSignalBlocker>
#include <QSet>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMargins>
#include <QColor>
#include <QFont>
#include <QEasingCurve>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTcpSocket>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QScrollArea>
#include <QEventLoop>
#include <QTimer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QSlider>
#include <QtGlobal>
#include <QByteArray>
#include <QToolTip>
#include <QComboBox>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QAbstractAnimation>
#include <QVariantAnimation>
#include <QRandomGenerator>
#include <QPointer>
#include <QPauseAnimation>
#include <QPauseAnimation>
#include <QDialog>
#include <QScreen>
#include <limits>
#include <functional>
#include <memory>
#include <tuple>
#include <array>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>


static QList<QColor> chartPaletteFILdOr() {
    return { QColor("#d4af37"), QColor("#3e2723"), QColor("#e0c097"), QColor("#8d5524"), QColor("#b87333"), QColor("#a1887f") };
}

static int fashionOracleListenPort()
{
    const QByteArray p = qgetenv("FASHION_ORACLE_PORT");
    bool ok = false;
    const int v = QString::fromUtf8(p).trimmed().toInt(&ok);
    if (ok && v > 0 && v < 65536)
        return v;
    return 8010;
}

static QString fashionOracleBaseUrl()
{
    return QStringLiteral("http://127.0.0.1:%1").arg(fashionOracleListenPort());
}

static QString stylePopup() {
    return "QDialog { background-color: #f3f0eb; font-family: 'Segoe UI'; color: #3e2723; }"
           "QLabel { font-size: 14px; font-weight: bold; color: #5d4037; }"
           "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit { "
           "    background-color: white; border: 1px solid #d7ccc8; border-radius: 6px; padding: 8px; font-size: 13px; color: #3e2723;"
           "}"
           "QLineEdit:focus, QComboBox:focus { border: 2px solid #8d5524; }";
}

static QString styleBtnSave() {
    return "QPushButton { background-color: #8d5524; color: white; border-radius: 6px; padding: 10px 20px; font-weight: bold; font-size: 13px; }"
           "QPushButton:hover { background-color: #a1887f; }";
}

static QString styleBtnCancel() {
    return "QPushButton { background-color: #b71c1c; color: white; border-radius: 6px; padding: 10px 20px; font-weight: bold; font-size: 13px; }"
           "QPushButton:hover { background-color: #d32f2f; }";
}

static QString qssTitle(const QString &bg) {
    return QString("font-size: 18px; font-weight: bold; color: #3e2723; padding: 12px; background-color: %1; border-radius: 10px;").arg(bg);
}

static QString qssCard() {
    return "padding: 12px; background: white; border: 1px solid #d7ccc8; border-radius: 10px; font-size: 14px;";
}

static QString qssBtnPrimaryTeal() {
    return "background-color: #00838f; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnGreenClose() {
    return "background-color: #4caf50; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnPurple() {
    return "background-color: #7b1fa2; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static void applyDialogBase(QDialog &dialog, int w, int h) {
    dialog.setMinimumSize(w, h);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");
}

static QString messageValidationSaisieProduit(const QString &designation,
                                              double cout,
                                              const QString &collection,
                                              const QString &typeCuir,
                                              int tempsFabricationHeures)
{
    const QString des = designation.trimmed();
    if (des.length() < 5)
        return QStringLiteral("La désignation doit contenir au moins 5 caractères.");
    if (des.length() > 100)
        return QStringLiteral("La désignation ne doit pas dépasser 100 caractères (limite base de données).");

    if (cout < 0.01)
        return QStringLiteral("Le coût doit être au moins 0,01 (valeur strictement positive).");
    if (cout > 999999.99)
        return QStringLiteral("Le coût est trop élevé (maximum autorisé : 999 999,99).");

    const QString coll = collection.trimmed();
    if (coll.isEmpty())
        return QStringLiteral("Indiquez une collection (liste ou saisie libre).");
    if (coll.length() < 2)
        return QStringLiteral("La collection doit contenir au moins 2 caractères.");

    const QString cuir = typeCuir.trimmed();
    if (cuir.isEmpty())
        return QStringLiteral("Indiquez un type de cuir requis.");
    if (cuir.length() < 3)
        return QStringLiteral("Le type de cuir doit contenir au moins 3 caractères.");

    if (tempsFabricationHeures < 1)
        return QStringLiteral("Le temps de fabrication doit être d'au moins 1 heure.");
    if (tempsFabricationHeures > 8760)
        return QStringLiteral("Le temps de fabrication ne peut dépasser 8760 heures (1 an).");

    return {};
}

static QString messageValidationSaisieClient(const QString &nom,
                                             const QString &tel,
                                             const QString &adr,
                                             const QString &email,
                                             int pointsFidelite)
{
    const QString n = nom.trimmed();
    if (n.length() < 2)
        return QStringLiteral("Le nom doit contenir au moins 2 caractères.");
    if (n.length() > 100)
        return QStringLiteral("Le nom ne doit pas dépasser 100 caractères.");

    const QString t = tel.trimmed();
    if (!t.isEmpty()) {
        static const QRegularExpression reTel(QStringLiteral(R"(^[-+().\s\d]{6,32}$)"));
        if (!reTel.match(t).hasMatch())
            return QStringLiteral("Téléphone invalide : utilisez chiffres, espaces, +, -, parenthèses ou points (6 à 32 caractères).");
        int digits = 0;
        for (QChar c : t) {
            if (c.isDigit())
                ++digits;
        }
        if (digits < 8 || digits > 15)
            return QStringLiteral("Le numéro doit comporter entre 8 et 15 chiffres.");
    }

    const QString a = adr.trimmed();
    if (a.length() > 200)
        return QStringLiteral("L’adresse ne doit pas dépasser 200 caractères.");

    const QString e = email.trimmed();
    if (!e.isEmpty()) {
        static const QRegularExpression reMail(QStringLiteral(
            R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"));
        if (!reMail.match(e).hasMatch())
            return QStringLiteral("Adresse e-mail invalide.");
        if (e.length() > 120)
            return QStringLiteral("L’e-mail ne doit pas dépasser 120 caractères.");
    }

    if (pointsFidelite < 0 || pointsFidelite > 100000)
        return QStringLiteral("Les points fidélité doivent être entre 0 et 100 000.");

    return {};
}

static QString messageValidationIdClientAjoutOptionnel(const QString &idText)
{
    const QString t = idText.trimmed();
    if (t.isEmpty())
        return {};
    bool ok = false;
    const int id = t.toInt(&ok);
    if (!ok || id <= 0)
        return QStringLiteral("L’identifiant doit être un entier strictement positif, ou laissé vide pour attribution automatique.");
    return {};
}

static QString messageValidationFkProduitObligatoires(int idClient,
                                                         int idEmpl,
                                                         const QComboBox *cbClient,
                                                         const QComboBox *cbDepot)
{
    const int nCli = (cbClient && cbClient->count() > 1) ? (cbClient->count() - 1) : 0;
    const int nDep = (cbDepot && cbDepot->count() > 1) ? (cbDepot->count() - 1) : 0;

    if (nCli <= 0)
        return QStringLiteral("Aucun client en base : enregistrez des clients (table CLIENTS) avant de créer un produit.");
    if (idClient <= 0)
        return QStringLiteral("Sélectionnez un client — relation « Acheter » (1,1 côté produit).");

    if (nDep <= 0)
        return QStringLiteral("Aucun dépôt en base : créez au moins un emplacement (module Dépôt) avant de créer un produit.");
    if (idEmpl <= 0)
        return QStringLiteral("Sélectionnez un emplacement — relation « stocker » (1,1 côté produit).");

    return {};
}

static int comboIdData(QComboBox *cb)
{
    if (!cb || cb->count() == 0)
        return 0;
    const int ix = cb->currentIndex();
    if (ix < 0)
        return 0;
    const QVariant v = cb->itemData(ix, Qt::UserRole);
    if (v.isValid() && v.canConvert<int>())
        return v.toInt();
    bool ok = false;
    const int n = v.toString().toInt(&ok);
    return ok ? n : 0;
}

static void reglerComboParIdDonnee(QComboBox *cb, int id)
{
    if (!cb)
        return;
    if (id <= 0) {
        cb->setCurrentIndex(0);
        return;
    }
    int ix = cb->findData(id);
    if (ix < 0)
        ix = cb->findData(QString::number(id));
    if (ix >= 0)
        cb->setCurrentIndex(ix);
    else
        cb->setCurrentIndex(0);
}

static int indexMesClientParIdDb(const QVector<ClientInfo> &mesClients, int idClient)
{
    if (idClient <= 0)
        return -1;
    const QString sid = QString::number(idClient);
    for (int i = 0; i < mesClients.size(); ++i) {
        if (mesClients[i].id == sid)
            return i;
    }
    return -1;
}

struct ModeSeriePoint {
    QString ym;
    double qte = 0.0;
};

struct ModePrediction {
    QString collection;
    double baseMoyenne = 0.0;
    double moyenneRecente = 0.0;
    double penteTendance = 0.0;
    double momentum = 0.0;
    double saisonnalite = 0.0;
    double volatilite = 0.0;
    double score = 0.0;
    double confiance = 0.0;
    double prevision = 0.0;
    int pointsHistoriques = 0;
};

static double clamp01(double v)
{
    if (v < 0.0)
        return 0.0;
    if (v > 1.0)
        return 1.0;
    return v;
}

static double normaliserMinMax(double v, double vMin, double vMax)
{
    const double d = vMax - vMin;
    if (qFuzzyIsNull(d))
        return 0.5;
    return clamp01((v - vMin) / d);
}

static double moyenne(const QVector<double> &vals)
{
    if (vals.isEmpty())
        return 0.0;
    double s = 0.0;
    for (double v : vals)
        s += v;
    return s / static_cast<double>(vals.size());
}

static double ecartType(const QVector<double> &vals, double mu)
{
    if (vals.size() <= 1)
        return 0.0;
    double acc = 0.0;
    for (double v : vals) {
        const double d = v - mu;
        acc += d * d;
    }
    return std::sqrt(acc / static_cast<double>(vals.size()));
}

static double penteRegressionLineaire(const QVector<double> &y)
{
    const int n = y.size();
    if (n < 2)
        return 0.0;

    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (int i = 0; i < n; ++i) {
        const double x = static_cast<double>(i);
        sx += x;
        sy += y[i];
        sxx += x * x;
        sxy += x * y[i];
    }
    const double denom = (static_cast<double>(n) * sxx) - (sx * sx);
    if (qFuzzyIsNull(denom))
        return 0.0;
    return ((static_cast<double>(n) * sxy) - (sx * sy)) / denom;
}

static QDate parseDateFinPrevue(const QString &texteBrut, const QDate &siVide, QString *errMsg)
{
    const QString t = texteBrut.trimmed();
    if (t.isEmpty())
        return siVide;

    QDate d = QDate::fromString(t, QStringLiteral("dd/MM/yyyy"));
    if (!d.isValid())
        d = QDate::fromString(t, QStringLiteral("yyyy-MM-dd"));
    if (!d.isValid())
        d = QDate::fromString(t, Qt::ISODate);

    if (!d.isValid()) {
        if (errMsg) {
            *errMsg = QStringLiteral(
                "Date de fin invalide. Formats acceptés : jj/mm/aaaa, aaaa-mm-jj ou date ISO.");
        }
        return {};
    }
    return d;
}

namespace {
class SortableNumericTableWidgetItem : public QTableWidgetItem {
public:
    static constexpr int NumericSortType = QTableWidgetItem::UserType + 77;

    explicit SortableNumericTableWidgetItem(const QString &text, double sortKey)
        : QTableWidgetItem(text, NumericSortType)
        , m_sortKey(sortKey)
    {}

    bool operator<(const QTableWidgetItem &other) const override
    {
        if (other.type() == NumericSortType) {
            const auto &no = *static_cast<const SortableNumericTableWidgetItem *>(&other);
            return m_sortKey < no.m_sortKey;
        }
        return QTableWidgetItem::operator<(other);
    }

private:
    double m_sortKey;
};
}


static void styleChartBase(QChart *chart) {
    if(!chart) return;
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setTitle(QString());
    QFont baseFont = chart->font(); baseFont.setFamily("Segoe UI"); baseFont.setPointSize(9); chart->setFont(baseFont);
    if(chart->legend()) {
        chart->legend()->setVisible(true); chart->legend()->setLabelColor(QColor("#3e2723"));
        chart->legend()->setFont(baseFont); chart->legend()->setContentsMargins(0, 0, 0, 0);
    }
}

static void styleChartView(QChartView *view) {
    if(!view) return;
    view->setRenderHint(QPainter::Antialiasing); view->setFrameShape(QFrame::NoFrame);
    view->setStyleSheet("background: transparent;"); view->setContentsMargins(0, 0, 0, 0);
    view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    view->setMinimumHeight(240); view->setMaximumSize(700, 300);
}

static void clearLayout(QLayout *layout) {
    if(!layout) return;
    while(QLayoutItem *child = layout->takeAt(0)) {
        if(QWidget *w = child->widget()) w->deleteLater();
        if(QLayout *l = child->layout()) clearLayout(l);
        delete child;
    }
}

static QVBoxLayout* ensureVBox(QWidget *w) {
    if(!w) return nullptr;
    if(auto *existing = qobject_cast<QVBoxLayout*>(w->layout())) return existing;
    if(w->layout()) { clearLayout(w->layout()); delete w->layout(); }
    auto *vl = new QVBoxLayout(w); vl->setContentsMargins(0, 0, 0, 0); vl->setSpacing(0); return vl;
}

static void setPieChart(QWidget *container, const QString &legendTitle, const QList<QPair<QString,double>> &slices) {
    if(!container) return;
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(container->minimumHeight() < 260) container->setMinimumHeight(260);
    auto *vl = ensureVBox(container); clearLayout(vl);
    vl->setContentsMargins(0, 0, 0, 0); vl->setSpacing(0);

    QList<QPair<QString,double>> sorted = slices;
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b){ return a.second > b.second; });
    double total = 0.0; for(const auto &s : sorted) if(s.second > 0.0) total += s.second;
    if(total <= 0.0) { sorted = {{"Aucune donnée", 1.0}}; total = 1.0; }

    auto *series = new QPieSeries(); series->setHoleSize(0.55); series->setPieSize(0.80); series->setPieStartAngle(90);
    const QLocale loc = QLocale::system(); QStringList names;
    for(const auto &s : sorted) { if(s.second <= 0.0) continue; names << s.first; series->append(s.first, s.second); }

    QFont sliceFont; sliceFont.setFamily("Segoe UI"); sliceFont.setPointSize(9); sliceFont.setBold(true);
    const QList<QColor> palette = chartPaletteFILdOr();
    const auto sliceList = series->slices();
    for(int i = 0; i < sliceList.size(); ++i) {
        auto *slice = sliceList.at(i); if(!slice) continue;
        const QColor c = palette.at(i % palette.size());
        slice->setBrush(QBrush(c)); slice->setPen(QPen(QColor("#f3f0eb"), 2));
        slice->setLabelVisible(true); slice->setLabelFont(sliceFont);
        slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        const double pct = (total > 0.0) ? (slice->value() * 100.0 / total) : 0.0;
        slice->setLabel(names.value(i) + "\n" + loc.toString(pct, 'f', 0) + "%");
        slice->setLabelColor(QColor((c.red()*299 + c.green()*587 + c.blue()*114)/1000 < 145 ? "#ffffff" : "#3e2723"));
    }

    auto *chart = new QChart(); chart->addSeries(series); styleChartBase(chart);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    if(chart->legend()) { chart->legend()->setAlignment(Qt::AlignBottom); chart->legend()->setBackgroundVisible(false); }
    Q_UNUSED(legendTitle);

    auto *view = new QChartView(chart); styleChartView(view);
    view->setMaximumSize(640, 280); view->setMinimumSize(420, 240);
    vl->addStretch(1); vl->addWidget(view, 0, Qt::AlignCenter); vl->addStretch(1);
}

static void setVerticalBarChart(QWidget *container, const QString &title, const QStringList &categories, const QList<double> &values) {
    if(!container) return;
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(container->minimumHeight() < 260) container->setMinimumHeight(260);
    auto *vl = ensureVBox(container); clearLayout(vl);
    vl->setContentsMargins(0, 0, 0, 0); vl->setSpacing(0);

    const int n = qMin(categories.size(), values.size());
    QStringList cats = categories.mid(0, n); QList<double> vals = values.mid(0, n);
    if(cats.isEmpty()) { cats << "-"; vals << 0.0; }
    if(cats.size() == 1 && vals.size() == 1) { cats = {" ", cats.at(0), "  "}; vals = {0.0, vals.at(0), 0.0}; }

    auto *set0 = new QBarSet(title); for(double v : vals) *set0 << v;
    set0->setBrush(QBrush(QColor("#d4af37"))); set0->setColor(QColor("#d4af37"));

    auto *series = new QBarSeries(); series->append(set0); series->setBarWidth(0.65);
    series->setLabelsVisible(true); series->setLabelsFormat("@value"); series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    auto *chart = new QChart(); chart->addSeries(series); styleChartBase(chart); chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations); if(chart->legend()) chart->legend()->setVisible(false);

    auto *axisX = new QBarCategoryAxis(); axisX->append(cats); axisX->setLabelsColor(QColor("#3e2723"));
    axisX->setGridLineVisible(false); axisX->setLabelsAngle(-15);
    chart->addAxis(axisX, Qt::AlignBottom); series->attachAxis(axisX);

    double maxV = 1.0; for(double v : vals) maxV = qMax(maxV, v);
    auto *axisY = new QValueAxis(); axisY->setRange(0, maxV * 1.25); axisY->applyNiceNumbers();
    axisY->setLabelFormat("%.0f"); axisY->setLabelsColor(QColor("#3e2723"));
    axisY->setGridLineColor(QColor("#eee5dd")); axisY->setLinePenColor(QColor("#d7ccc8")); axisY->setTickCount(6);
    chart->addAxis(axisY, Qt::AlignLeft); series->attachAxis(axisY);

    auto *view = new QChartView(chart); styleChartView(view);
    view->setMaximumSize(640, 280); view->setMinimumSize(420, 240);
    vl->addStretch(1); vl->addWidget(view, 0, Qt::AlignCenter); vl->addStretch(1);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_namCostSim(this)
{
    ui->setupUi(this);

    if (ui->sb_stock_qte) {
        ui->sb_stock_qte->setMinimum(0.01);
        ui->sb_stock_qte->setDecimals(2);
    }
    if (ui->sb_stock_qte_modif) {
        ui->sb_stock_qte_modif->setMinimum(0.01);
        ui->sb_stock_qte_modif->setDecimals(2);
    }
    if (ui->le_stock_code) {
        ui->le_stock_code->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[A-Z]{2,4}-20\\d{2}-\\d{3}$")), this));
    }
    if (ui->le_stock_lot) {
        ui->le_stock_lot->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^LOT-20\\d{2}-[A-Z]$")), this));
    }
    if (ui->le_stock_coul) {
        ui->le_stock_coul->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[A-Za-zÀ-ÿ ]{3,20}$")), this));
    }

    if (ui->le_depot_id) {
        ui->le_depot_id->setReadOnly(true);
        ui->le_depot_id->setPlaceholderText(QStringLiteral("Généré automatiquement (séquence)"));
    }
    if (ui->sb_depot_act)
        ui->sb_depot_act->setMinimum(0.0);
    if (ui->sb_depot_act_modif)
        ui->sb_depot_act_modif->setMinimum(0.0);

    ui->l_logo_img->setMinimumSize(55, 55);
    ui->l_logo_img->setMaximumSize(55, 55);
    ui->l_logo_img->setPixmap(QPixmap(":/logo.png").scaled(55, 55, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->l_logo_img->setScaledContents(false);
    ui->l_logo_img->setAlignment(Qt::AlignCenter);
    ui->l_logo_img->setStyleSheet("border: none; background: transparent;");

    construirePageAccueil();
    construirePageLogin();

    Connexion *cnx = Connexion::getInstance();
    bool test = cnx->estConnecte();
    if (test) {
        qDebug() << "✅ Base Oracle connectée via Singleton";
        QSqlQuery qUser(cnx->getDatabase());
        QString schema = QStringLiteral("?");
        if (qUser.exec(QStringLiteral("SELECT USER FROM DUAL")) && qUser.next())
            schema = qUser.value(0).toString();
        setWindowTitle(QStringLiteral("FIL D'OR — Oracle connecté (%1)").arg(schema));
    } else {
        qDebug() << "❌ Pas de connexion Oracle";
        alerteWarning("Erreur BDD", "Impossible de se connecter à la base Oracle.");
        setWindowTitle(QStringLiteral("FIL D'OR — hors base (mode limité)"));
    }

    ui->tablePlanif->setSortingEnabled(true);
    ui->tableProduits->setSortingEnabled(true);
    ui->tableEmployes->setSortingEnabled(true);
    ui->tableStock->setSortingEnabled(true);
    ui->tableClients->setSortingEnabled(true);
    ui->tableDepot->setSortingEnabled(true);

    mesProduits.append({"1", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette", 5, "1", "1"});
    mesEmployes.append({"1", "Dupont", "Jean", "Chef Atelier", "jean@fildor.tn", "55123456", "Production", QDate(2020, 5, 10), 2800.0, "RF-123"});
    mesClients.append({"1", "Ben Salah", "55 123 456", "Tunis", "client1@fildor.tn", 120});
    mesDepots.append({"1", "Zone A", "E1", 500.0, 320.0, "Sec"});

    myColorDelegate = new ColorDelegate(this);
    ui->tableTimeline->setItemDelegate(myColorDelegate);

    if (cnx->estConnecte()) {
        rafraichirListeCommandes();
        configurerTimelineGantt();
        rafraichirListeProduits();
        rafraichirListeEmployes();
        rafraichirListeMatieres();
        rafraichirListeClients();
        rafraichirListeDepots();
        rafraichirListeEtapes();
        remplirCombosProduitClientEmplacement();
    } else {
    }

    ui->stackedWidget->setCurrentWidget(ui->page_home);

    auto setNavigationEnabled = [this](bool enabled) {
        ui->btn_nav_dashboard->setEnabled(enabled);
        ui->btn_nav_planif->setEnabled(enabled);
        ui->btn_nav_fab->setEnabled(enabled);
        ui->btn_nav_produit->setEnabled(enabled);
        ui->btn_nav_rh->setEnabled(enabled);
        ui->btn_nav_stock->setEnabled(enabled);
        ui->btn_nav_clients->setEnabled(enabled);
        ui->btn_nav_depot->setEnabled(enabled);
    };
    setNavigationEnabled(false);

    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){ construireDashboardAccueil(); ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){
        static bool etapesInit = false;
        if (!etapesInit) {
            construirePageEtapes();
            etapesInit = true;
        } else {
            QTableWidget *tbl = ui->page_fab_list->findChild<QTableWidget*>("tableEtapes");
            if (tbl) {
                Etape e;
                QSqlQueryModel *model = e.afficher();

                tbl->setRowCount(0);
                int rows = model->rowCount();
                tbl->setRowCount(rows);

                for (int i = 0; i < rows; i++) {
                    int idSuivi = model->record(i).value("ID_SUIVI").toInt();
                    int idPlanif = model->record(i).value("ID_PLANIFICATION").toInt();
                    QString produit = model->record(i).value("PRODUIT").toString();
                    QString employe = model->record(i).value("EMPLOYE").toString();
                    QString etape = model->record(i).value("ETAPE_ACTUELLE").toString();
                    double tempsReel = model->record(i).value("TEMPS_REEL_PASSE").toDouble();
                    double delta = model->record(i).value("DELTA").toDouble();
                    int alerte = model->record(i).value("ALERTE_ACTIVE").toInt();

                    QFont fBold; fBold.setBold(true);

                    QTableWidgetItem *itemId = new QTableWidgetItem(QString::number(idSuivi));
                    itemId->setData(Qt::UserRole, idSuivi);
                    itemId->setData(Qt::UserRole + 1, idPlanif);
                    tbl->setItem(i, 0, itemId);
                    tbl->setItem(i, 1, new QTableWidgetItem(QString::number(idPlanif)));

                    QTableWidgetItem *itemProd = new QTableWidgetItem(produit);
                    itemProd->setFont(fBold);
                    tbl->setItem(i, 2, itemProd);
                    tbl->setItem(i, 3, new QTableWidgetItem(employe));

                    QColor couleurEtape;
                    if (etape == "Coupe") couleurEtape = QColor("#1565c0");
                    else if (etape == "Assemblage") couleurEtape = QColor("#ef6c00");
                    else if (etape == "Couture") couleurEtape = QColor("#2e7d32");
                    else if (etape == "Finition") couleurEtape = QColor("#f9a825");
                    else couleurEtape = QColor("#757575");

                    QTableWidgetItem *itemEtape = new QTableWidgetItem(etape);
                    itemEtape->setForeground(couleurEtape);
                    itemEtape->setFont(fBold);
                    tbl->setItem(i, 4, itemEtape);

                    QTableWidgetItem *itemTemps = new QTableWidgetItem(QString::number(tempsReel, 'f', 1) + " h");
                    itemTemps->setTextAlignment(Qt::AlignCenter);
                    tbl->setItem(i, 5, itemTemps);

                    QTableWidgetItem *itemDelta = new QTableWidgetItem((delta > 0 ? "+" : "") + QString::number(delta, 'f', 1) + " h");
                    itemDelta->setTextAlignment(Qt::AlignCenter);
                    if (delta > 0) itemDelta->setForeground(QColor("#c62828"));
                    else if (delta < 0) itemDelta->setForeground(QColor("#2e7d32"));
                    else itemDelta->setForeground(QColor("#757575"));
                    itemDelta->setFont(fBold);
                    tbl->setItem(i, 6, itemDelta);

                    QTableWidgetItem *itemAlerte = new QTableWidgetItem(alerte ? "🔴 OUI" : "🟢 Non");
                    itemAlerte->setTextAlignment(Qt::AlignCenter);
                    if (alerte) itemAlerte->setForeground(QColor("#c62828"));
                    else itemAlerte->setForeground(QColor("#2e7d32"));
                    itemAlerte->setFont(fBold);
                    tbl->setItem(i, 7, itemAlerte);

                    tbl->setRowHeight(i, 38);
                }

                delete model;
            }
        }
        ui->stackedWidget->setCurrentWidget(ui->page_fab_list);
    });
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){
        remplirCombosProduitClientEmplacement();
        rafraichirListeProduits();
        ui->stackedWidget->setCurrentWidget(ui->page_produit_list);
    });
    connect(ui->btn_nav_rh, &QPushButton::clicked, [=](){ rafraichirListeEmployes(); ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_nav_stock, &QPushButton::clicked, [=](){ rafraichirListeMatieres(); ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_nav_clients, &QPushButton::clicked, [=](){ rafraichirListeClients(); ui->stackedWidget->setCurrentWidget(ui->page_client_list); });
    connect(ui->btn_nav_depot, &QPushButton::clicked, [=](){ rafraichirListeDepots(); ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });


    connect(ui->btn_add_depot, &QPushButton::clicked, [=](){
        ui->le_depot_id->clear();
        ui->le_depot_emp->clear();
        ui->le_depot_eta->clear();
        ui->sb_depot_cap->setValue(0);
        ui->sb_depot_act->setValue(0);
        ui->cb_depot_type->setCurrentIndex(0);
        ui->tabWidgetDepot->setCurrentIndex(1);
    });

    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){
        rafraichirListeEtapes();
    });

    connect(ui->btn_valider_depot, &QPushButton::clicked, [=](){
        const QString errIdAj = Depot::messageIdAjoutNeDoitPasEtreSaisi(ui->le_depot_id->text());
        if (!errIdAj.isEmpty()) {
            alerteWarning(QStringLiteral("Identifiant"), errIdAj);
            return;
        }
        const QString emp = ui->le_depot_emp->text();
        const QString eta = ui->le_depot_eta->text();
        if (emp.trimmed().isEmpty() || eta.trimmed().isEmpty()) {
            alerteWarning(QStringLiteral("Saisie incomplète"),
                          QStringLiteral("Renseignez l'emplacement et l'étagère pour valider l'ajout."));
            return;
        }
        const double cap = ui->sb_depot_cap->value();
        const double qte = ui->sb_depot_act->value();

        Depot d(
            emp,
            eta,
            cap,
            qte,
            ui->cb_depot_type->currentText()
        );

        if (d.ajouter()) {
            alerteSucces("Ajout Dépôt", "Emplacement ajouté avec succès.");
            rafraichirListeDepots();
            remplirCombosProduitClientEmplacement();
            ui->tabWidgetDepot->setCurrentIndex(0);
        } else {
            if (!d.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), d.derniereErreurSaisie());
            else
            alerteErreur("Erreur", "Impossible d'ajouter l'emplacement.");
        }
    });

    connect(ui->btn_edit_depot, &QPushButton::clicked, [=](){
        int row = ui->tableDepot->currentRow();
        if (row < 0) {
            alerteWarning("Sélection", "Veuillez sélectionner une ligne.");
            return;
        }

        int idDb = ui->tableDepot->item(row, 2)->data(Qt::UserRole).toInt();

        ui->le_depot_id_modif->setText(QString::number(idDb));
        {
            const QString etCell = ui->tableDepot->item(row, 2)->text();
            const QString sep = QStringLiteral(" — ");
            const int ixSep = etCell.indexOf(sep);
            if (ixSep >= 0) {
                ui->le_depot_emp_modif->setText(etCell.left(ixSep));
                ui->le_depot_eta_modif->setText(etCell.mid(ixSep + sep.size()));
            } else {
                ui->le_depot_emp_modif->clear();
                ui->le_depot_eta_modif->setText(etCell);
            }
        }
        ui->sb_depot_cap_modif->setValue(ui->tableDepot->item(row, 3)->text().toDouble());
        ui->sb_depot_act_modif->setValue(ui->tableDepot->item(row, 4)->text().toDouble());
        ui->cb_depot_type_modif->setCurrentText(ui->tableDepot->item(row, 5)->text());

        ui->tabWidgetDepot->setCurrentIndex(2);
    });

    connect(ui->btn_valider_modif, &QPushButton::clicked, [=](){
        if (indexModification < 0 || indexModification >= mesCommandes.size()) {
            alerteErreur("Erreur", "Aucune commande selectionnee pour modification.");
            ui->tabWidgetPlanif->setCurrentIndex(0);
            return;
        }

        const int idProdM = comboIdData(ui->cb_produit_modif);
        const int idMatM = comboIdData(ui->cb_matiere_modif);
        const int idEmpM = comboIdData(ui->cb_employe_modif);
        const int q = ui->sb_qte_modif->value();
        const QDate d1 = ui->dt_lancement_modif->date();
        QString errDateM;
        const QDate d2 = parseDateFinPrevue(ui->le_fin_prevue_modif->text(), d1.addDays(7), &errDateM);
        if (!errDateM.isEmpty()) {
            alerteWarning(QStringLiteral("Date invalide"), errDateM);
            return;
        }

        const QString pId = QString::number(idProdM);
        const QString mId = QString::number(idMatM);
        const QString eId = QString::number(idEmpM);

        QString idStr = mesCommandes[indexModification].id;
        int idToEdit = idStr.mid(3).toInt();

        if (idToEdit <= 0) {
            idToEdit = idStr.toInt();
        }

        if (idToEdit <= 0) {
            alerteErreur("Erreur", "ID commande invalide: " + idStr);
            return;
        }

        qDebug() << "Modification commande ID:" << idToEdit
                 << "Produit:" << pId << "Qte:" << q
                 << "Matiere:" << mId << "Employe:" << eId
                 << "Debut:" << d1 << "Fin:" << d2;

        OrdreFabrication o(pId, q, mId, d1, d2, "Planifie", eId);

        if(o.modifier(idToEdit)) {
            alerteSucces("Succes", "Commande mise a jour !");
            rafraichirListeCommandes();
            ui->tabWidgetPlanif->setCurrentIndex(0);
        } else {
            if (!o.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), o.derniereErreurSaisie());
            else
            alerteErreur("Erreur", "Echec BDD.");
        }
    });

    connect(ui->btn_delete_depot, &QPushButton::clicked, [=](){
        int row = ui->tableDepot->currentRow();
        if (row < 0) {
            alerteWarning("Sélection", "Sélectionnez une ligne.");
            return;
        }

        int idDb = ui->tableDepot->item(row, 2)->data(Qt::UserRole).toInt();

        Depot d;
        if (d.supprimer(idDb)) {
            alerteSucces("Suppression", "Emplacement supprimé.");
            rafraichirListeDepots();
            remplirCombosProduitClientEmplacement();
        } else {
            alerteErreur("Erreur", "Impossible de supprimer.");
        }
    });

    connect(ui->btn_search_depot, &QPushButton::clicked, [=](){
        QString critere = ui->le_search_depot->text().trimmed();
        if (critere.isEmpty()) {
            rafraichirListeDepots();
            return;
        }

        Depot d;
        QSqlQueryModel *model = d.rechercher(critere);

        ui->tableDepot->setRowCount(0);
        int rows = model->rowCount();
        ui->tableDepot->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idDb = model->record(i).value("ID_EMPLACEMENT").toInt();
            QString et = model->record(i).value("ETAGERE").toString();
            double cap = model->record(i).value("CAPACITE_MAX").toDouble();
            double qte = model->record(i).value("QUANTITE_ACTUELLE").toDouble();
            QString type = model->record(i).value("TYPE_STOCKAGE").toString();

            QString remplissage = (cap > 0) ? QString::number((qte / cap) * 100.0, 'f', 1) + "%" : "0%";

            ui->tableDepot->setItem(i, 0, new QTableWidgetItem(QString::number(idDb)));
            ui->tableDepot->setItem(i, 1, new QTableWidgetItem("Empl. " + QString::number(idDb)));
            ui->tableDepot->setItem(i, 2, new QTableWidgetItem(et));
            ui->tableDepot->setItem(i, 3, new QTableWidgetItem(QString::number(cap)));
            ui->tableDepot->setItem(i, 4, new QTableWidgetItem(QString::number(qte)));
            ui->tableDepot->setItem(i, 5, new QTableWidgetItem(type));
            ui->tableDepot->setItem(i, 6, new QTableWidgetItem(remplissage));

            ui->tableDepot->item(i, 2)->setData(Qt::UserRole, idDb);
        }

        delete model;
    });

    connect(ui->btn_sort_alpha_depot, &QPushButton::clicked, [=](){
        Depot d;
        QSqlQueryModel *model = d.trierParEtagere();

        ui->tableDepot->setRowCount(0);
        int rows = model->rowCount();
        ui->tableDepot->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idDb = model->record(i).value("ID_EMPLACEMENT").toInt();
            QString et = model->record(i).value("ETAGERE").toString();
            double cap = model->record(i).value("CAPACITE_MAX").toDouble();
            double qte = model->record(i).value("QUANTITE_ACTUELLE").toDouble();
            QString type = model->record(i).value("TYPE_STOCKAGE").toString();

            QString remplissage = (cap > 0) ? QString::number((qte / cap) * 100.0, 'f', 1) + "%" : "0%";

            ui->tableDepot->setItem(i, 0, new QTableWidgetItem(QString::number(idDb)));
            ui->tableDepot->setItem(i, 1, new QTableWidgetItem("Empl. " + QString::number(idDb)));
            ui->tableDepot->setItem(i, 2, new QTableWidgetItem(et));
            ui->tableDepot->setItem(i, 3, new QTableWidgetItem(QString::number(cap)));
            ui->tableDepot->setItem(i, 4, new QTableWidgetItem(QString::number(qte)));
            ui->tableDepot->setItem(i, 5, new QTableWidgetItem(type));
            ui->tableDepot->setItem(i, 6, new QTableWidgetItem(remplissage));

            ui->tableDepot->item(i, 2)->setData(Qt::UserRole, idDb);
        }

        delete model;
    });

    connect(ui->btn_start_app, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_login); ui->le_login_nom->setFocus(); });
    connect(ui->btn_login_back, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_login, &QPushButton::clicked, [=](){
        QString nom = ui->le_login_nom->text().trimmed();
        QString prenom = ui->le_login_prenom->text().trimmed();
        QString mdp_rfid = ui->le_login_mdp->text().trimmed();

        if(nom.isEmpty() || prenom.isEmpty() || mdp_rfid.isEmpty()) {
            alerteWarning("Connexion", "Veuillez remplir tous les champs.");
            return;
        }

        if(nom == "admin" && prenom == "admin" && mdp_rfid == "1234") {
            ui->le_login_mdp->clear();
            setNavigationEnabled(true);
            rafraichirListeCommandes();
            calculerEtAfficherStats();
            construireDashboardAccueil();
            ui->stackedWidget->setCurrentWidget(ui->page_home);
            return;
        }

        QSqlQuery q;
        q.prepare("SELECT POSTE FROM EMPLOYES WHERE NOM = :nom AND PRENOM = :prenom AND RFID_TAG = :rfid");
        q.bindValue(":nom", nom);
        q.bindValue(":prenom", prenom);
        q.bindValue(":rfid", mdp_rfid);

        if(q.exec() && q.next()) {
            QString poste = q.value("POSTE").toString();
            alerteSucces("Bienvenue", "Connexion réussie !\nPoste : " + poste);

            ui->le_login_mdp->clear();
            setNavigationEnabled(true);
            rafraichirListeCommandes();
            calculerEtAfficherStats();
            construireDashboardAccueil();
            ui->stackedWidget->setCurrentWidget(ui->page_home);
        } else {
            alerteErreur("Accès Refusé", "Identifiants incorrects.");
        }
    });


    connect(ui->tabWidgetPlanif, &QTabWidget::currentChanged, [=](int index){
        if (index == 1 && !modeModification)
            preparerFormulairePlanif(false);
        else if (index == 2)
            calculerEtAfficherStats();
    });

    connect(ui->btn_to_add_planif, &QPushButton::clicked, [=](){
        preparerFormulairePlanif(false);
        ui->tabWidgetPlanif->setCurrentIndex(1);
    });

    ui->btn_valider_modif->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif->setCursor(Qt::PointingHandCursor);

    ui->btn_ia_appliquer->setStyleSheet("background-color: #9c27b0; color: white; border-radius: 8px; padding: 12px 25px; font-weight: bold; font-size: 14px;");
    ui->btn_ia_appliquer->setCursor(Qt::PointingHandCursor);

    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int idx = ui->tablePlanif->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Veuillez sélectionner une ligne."); return; }
        preparerFormulaireModif(idx);
    });


    connect(ui->btn_ia_appliquer, &QPushButton::clicked, [=](){
        int idCmd = ui->btn_ia_appliquer->property("id_cmd").toInt();
        QDate nvFin = ui->btn_ia_appliquer->property("nv_fin").toDate();

        QSqlQuery q;
        q.prepare("UPDATE PLANIFICATION SET DATE_FIN_PREVUE = :fin WHERE ID_COMMANDE = :id");
        q.bindValue(":fin", nvFin);
        q.bindValue(":id", idCmd);

        if(q.exec()) {
            alerteSucces("IA", "Planning optimisé avec succès !");
            rafraichirListeCommandes(); configurerTimelineGantt();
            ui->tabWidgetPlanif->setCurrentIndex(0);
        }
    });

    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        const int idProd = comboIdData(ui->cb_produit);
        const int idMat = comboIdData(ui->cb_matiere);
        const int idEmp = comboIdData(ui->cb_employe);
        const int q = ui->sb_qte->value();
        const QDate d1 = ui->dt_lancement->date();
        QString errDate;
        const QDate d2 = parseDateFinPrevue(ui->le_fin_prevue->text(), d1.addDays(3), &errDate);
        if (!errDate.isEmpty()) {
            alerteWarning(QStringLiteral("Date invalide"), errDate);
            return;
        }

        const QString pId = QString::number(idProd);
        const QString mId = QString::number(idMat);
        const QString eId = QString::number(idEmp);
        OrdreFabrication o(pId, q, mId, d1, d2, "Planifié", eId);
        int idToEdit = -1;
        if(modeModification && indexModification >= 0 && indexModification < mesCommandes.size())
            idToEdit = mesCommandes[indexModification].id.mid(3).toInt();
        bool ok = modeModification ? o.modifier(idToEdit) : o.ajouter();

        if(ok) {
            alerteSucces("Succès", modeModification ? "Commande modifiée !" : "Commande ajoutée !");
            rafraichirListeCommandes();

            if (!modeModification) {
                QSqlQuery qLastId;
                if (qLastId.exec("SELECT MAX(ID_COMMANDE) FROM PLANIFICATION") && qLastId.next()) {
                    int idNouvelleCommande = qLastId.value(0).toInt();

                    int idEmploye = idEmp;
                    if (idEmploye <= 0 && ui->cb_employe) {
                            QSqlQuery qEmp;
                            qEmp.prepare("SELECT ID_EMPLOYE FROM EMPLOYES WHERE NOM || ' ' || PRENOM = :nom");
                            qEmp.bindValue(":nom", ui->cb_employe->currentText());
                            if (qEmp.exec() && qEmp.next())
                                idEmploye = qEmp.value(0).toInt();
                            }
                    if (idEmploye <= 0)
                        idEmploye = 1;

                    QSqlQuery qCheck;
                    qCheck.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id");
                    qCheck.bindValue(":id", idNouvelleCommande);
                    if (qCheck.exec() && qCheck.next() && qCheck.value(0).toInt() == 0) {
                        Etape::genererEtapesCommande(idNouvelleCommande, idEmploye);
                        qDebug() << "4 etapes generees pour commande" << idNouvelleCommande;
                    }
                }
            }

            modeModification = false;
            ui->tabWidgetPlanif->setCurrentIndex(0);
        } else {
            if (!o.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), o.derniereErreurSaisie());
            else
            alerteErreur("Erreur", "Échec BDD.");
        }
    });

    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        double vitesse = ui->cb_produit->currentText().contains("Sac") ? 20.0 : 50.0;
        int jours = std::ceil(ui->sb_qte->value() / vitesse) + 1;
        QDate fin = ui->dt_lancement->date().addDays(jours);
        ui->le_fin_prevue->setText(fin.toString("dd/MM/yyyy"));
    });

    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){
        int r = ui->tablePlanif->currentRow();
        if(r < 0) return;
        QTableWidgetItem *it = ui->tablePlanif->item(r, 0);
        if(!it) return;
        int idToDelete = it->text().toInt();

        OrdreFabrication o;
        if(o.supprimer(idToDelete)) {
            alerteSucces("Succès", "Commande supprimée.");
            rafraichirListeCommandes(); configurerTimelineGantt(); calculerEtAfficherStats();
        } else { alerteErreur("Erreur", "Impossible de supprimer."); }
    });

    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){
        calculerEtAfficherStats();
        ui->tabWidgetPlanif->setCurrentIndex(2);
    });

    ui->btn_valider_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_produit->setCursor(Qt::PointingHandCursor);

    ui->btn_valider_modif_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_produit->setCursor(Qt::PointingHandCursor);

    connect(ui->tabWidgetProduits, &QTabWidget::currentChanged, [=](int index){
        if (index == 1 || index == 2)
            remplirCombosProduitClientEmplacement();
        if (index == 3) ouvrirStatsProduits();
        if (index == 4)
            showProduitCoutDialog();
    });

    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        preparerFormulaireProduit(false);
    });

    connect(ui->btn_search_col, &QPushButton::clicked, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
    });
    connect(ui->le_search_coll, &QLineEdit::returnPressed, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
    });

    connect(ui->btn_sort_alpha_prod, &QPushButton::clicked, [=](){
        if(ui->tableProduits->rowCount() <= 0) {
            alerteInfo(QStringLiteral("Tri"), QStringLiteral("Aucune ligne à trier."));
            return;
        }
        ui->tableProduits->setSortingEnabled(true);
        const Qt::SortOrder ordre = m_triProduitDesignationDescendant
            ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->tableProduits->sortItems(1, ordre);
        m_triProduitDesignationDescendant = !m_triProduitDesignationDescendant;
    });

    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
        ouvrirStatsProduits();
    });



    ui->btn_valider_emp->setStyleSheet(styleBtnSave());
    ui->btn_valider_emp->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_emp->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_emp->setCursor(Qt::PointingHandCursor);

    connect(ui->tabWidgetEmployes, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsRH();
        else if (index == 4) showEmpEvalTab();
        else if (index == 5) showEmpAncienneteTab();
        else if (index == 6) showEmpAssistantTab();
    });


    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){ preparerFormulaireEmploye(false); });


    auto performSearchEmployes = [=](){
        const QString critere = ui->le_search_emp->text().trimmed();
        if(critere.isEmpty()) {
            rafraichirListeEmployes();
            return;
        }

        const QString like = "%" + critere.toUpper() + "%";

        QSqlQuery q;
        q.prepare(
            "SELECT ID_EMPLOYE, NOM, PRENOM, POSTE, EMAIL, TELEPHONE, DEPARTEMENT, "
            "DATE_EMBAUCHE, SALAIRE, RFID_TAG "
            "FROM EMPLOYES "
            "WHERE UPPER(NOM) LIKE :c "
            "   OR UPPER(PRENOM) LIKE :c "
            "   OR UPPER(POSTE) LIKE :c "
            "   OR UPPER(DEPARTEMENT) LIKE :c "
            "   OR UPPER(EMAIL) LIKE :c "
            "   OR UPPER(TELEPHONE) LIKE :c "
            "   OR UPPER(RFID_TAG) LIKE :c "
            "ORDER BY ID_EMPLOYE DESC"
        );
        q.bindValue(":c", like);

        if(!q.exec()) {
            alerteErreur("Erreur BDD", "Recherche employé impossible.");
            return;
        }

        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(std::move(q));

        ui->tableEmployes->setRowCount(0);
        ui->tableEmployes->setColumnCount(7);
        ui->tableEmployes->setHorizontalHeaderLabels(
            {"ID", "NOM", "PRÉNOM", "POSTE", "DÉPARTEMENT", "SALAIRE", "EMBAUCHE"}
        );

        mesEmployes.clear();

        const int rows = model->rowCount();
        ui->tableEmployes->setRowCount(rows);

        for(int i = 0; i < rows; ++i) {
            const QSqlRecord rec = model->record(i);
            EmployeInfo info;
            info.id = QString::number(rec.value("ID_EMPLOYE").toInt());
            info.nom = rec.value("NOM").toString();
            info.prenom = rec.value("PRENOM").toString();
            info.poste = rec.value("POSTE").toString();
            info.email = rec.value("EMAIL").toString();
            info.telephone = rec.value("TELEPHONE").toString();
            info.departement = rec.value("DEPARTEMENT").toString();
            info.dateEmbauche = rec.value("DATE_EMBAUCHE").toDate();
            info.salaire = rec.value("SALAIRE").toDouble();
            info.rfid = rec.value("RFID_TAG").toString();

            mesEmployes.append(info);

            ui->tableEmployes->setItem(i, 0, new QTableWidgetItem(info.id));
            ui->tableEmployes->setItem(i, 1, new QTableWidgetItem(info.nom));
            ui->tableEmployes->setItem(i, 2, new QTableWidgetItem(info.prenom));
            ui->tableEmployes->setItem(i, 3, new QTableWidgetItem(info.poste));
            ui->tableEmployes->setItem(i, 4, new QTableWidgetItem(info.departement));
            ui->tableEmployes->setItem(i, 5, new QTableWidgetItem(QString::number(info.salaire, 'f', 0) + " DT"));
            ui->tableEmployes->setItem(i, 6, new QTableWidgetItem(info.dateEmbauche.toString("dd/MM/yyyy")));
        }

        if(employeTriAlphaActif && ui->tableEmployes) {
            ui->tableEmployes->setSortingEnabled(true);
            ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre);
        }

        delete model;
    };

    connect(ui->btn_search_emp, &QPushButton::clicked, this, performSearchEmployes);

    QTimer *searchDebounce = new QTimer(this);
    searchDebounce->setSingleShot(true);
    connect(ui->le_search_emp, &QLineEdit::textChanged, this, [=](){
        searchDebounce->start(300);
    });
    connect(searchDebounce, &QTimer::timeout, this, [=](){
        performSearchEmployes();
    });


    ui->btn_valider_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_stock->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_stock->setCursor(Qt::PointingHandCursor);

    connect(ui->tabWidgetStock, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsStock();
        else if (index == 4) showStockCompareTab();
        else if (index == 5) showStockCalculTab();
    });

    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){
        ui->le_stock_code->clear();
        ui->le_stock_lot->clear();
        ui->le_stock_coul->clear();
        ui->sb_stock_qte->setValue(0.01);
        ui->cb_stock_cat->setCurrentIndex(0);
        ui->cb_stock_etat->setCurrentIndex(0);
        ui->cb_stock_type->setCurrentIndex(0);
        ui->cb_stock_qual->setCurrentIndex(0);
        ui->tabWidgetStock->setCurrentIndex(1);
    });


    connect(ui->btn_edit_stock, &QPushButton::clicked, [=](){
        int row = ui->tableStock->currentRow();
        if (row < 0) {
            alerteWarning("Sélection", "Veuillez sélectionner une matière à modifier.");
            return;
        }

        ui->le_stock_code_modif->setText(ui->tableStock->item(row, 0)->text());
        ui->cb_stock_cat_modif->setCurrentText(ui->tableStock->item(row, 1)->text());
        ui->le_stock_lot_modif->setText(ui->tableStock->item(row, 2)->text());
        ui->cb_stock_etat_modif->setCurrentText(ui->tableStock->item(row, 3)->text());
        ui->le_stock_coul_modif->setText(ui->tableStock->item(row, 4)->text());
        ui->cb_stock_qual_modif->setCurrentText(ui->tableStock->item(row, 5)->text());
        ui->sb_stock_qte_modif->setValue(ui->tableStock->item(row, 6)->text().toDouble());
        ui->cb_stock_type_modif->setCurrentText(ui->tableStock->item(row, 7)->text());

        indexModifStock = row;
        ui->tabWidgetStock->setCurrentIndex(2);
    });

    connect(ui->btn_valider_modif_stock, &QPushButton::clicked, [=](){
        const QString code = ui->le_stock_code_modif->text().trimmed();
        const QString codeNorm = code.toUpper();
        const QString lot = ui->le_stock_lot_modif->text().trimmed();
        const QString coul = ui->le_stock_coul_modif->text().trimmed();
        const double qteMp = ui->sb_stock_qte_modif->value();
        const QString cat = ui->cb_stock_cat_modif->currentText();
        const QString etat = ui->cb_stock_etat_modif->currentText();
        const QString typeSt = ui->cb_stock_type_modif->currentText();
        const QString qual = ui->cb_stock_qual_modif->currentText();

        const QString errAvant = MatierePremiere::messageSiSaisieInvalide(
            codeNorm, cat, lot, etat, coul, qteMp, typeSt, qual);
        if (!errAvant.isEmpty()) {
            alerteWarning(QStringLiteral("Saisie invalide"), errAvant);
            return;
        }

        if (indexModifStock < 0 || indexModifStock >= mesMatieres.size()) {
            alerteWarning("Sélection", "Aucune matière sélectionnée pour la modification.");
            return;
        }

        int idOracle = mesMatieres[indexModifStock].id.toInt();

        MatierePremiere mp(
            codeNorm,
            cat,
            lot,
            etat,
            coul,
            qteMp,
            typeSt,
            qual
        );

        if (mp.modifier(idOracle)) {
            alerteSucces("Matière modifiée", "La matière " + codeNorm + " a été mise à jour !");
            rafraichirListeMatieres();
            ui->tabWidgetStock->setCurrentIndex(0);
        } else {
            if (!mp.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), mp.derniereErreurSaisie());
            else
            alerteErreur("Erreur BDD", "Impossible de modifier la matière dans Oracle.");
        }
    });

    connect(ui->btn_delete_stock, &QPushButton::clicked, [=](){
        int row = ui->tableStock->currentRow();
        if (row < 0) {
            alerteWarning("Sélection", "Veuillez sélectionner une matière à supprimer.");
            return;
        }

        QTableWidgetItem *item = ui->tableStock->item(row, 0);
        if (!item) return;
        int idOracle = item->data(Qt::UserRole).toInt();
        QString codeMP = item->text();

        MatierePremiere mp;
        if (mp.supprimer(idOracle)) {
            alerteSucces("Suppression réussie", "La matière " + codeMP + " a été supprimée de la base.");
            rafraichirListeMatieres();
        } else {
            alerteErreur("Erreur BDD", "Impossible de supprimer.\nVérifiez les contraintes de clé étrangère.");
        }
    });

    connect(ui->btn_search_stock, &QPushButton::clicked, [=](){
        QString critere = ui->le_search_stock->text().trimmed();
        if (critere.isEmpty()) {
            rafraichirListeMatieres();
            return;
        }

        MatierePremiere mp;
        QSqlQueryModel *model = mp.rechercher(critere);

        ui->tableStock->setRowCount(0);
        mesMatieres.clear();
        int rows = model->rowCount();
        ui->tableStock->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idDb      = model->record(i).value("ID_STOCK_MP").toInt();
            QString code  = model->record(i).value("CODE_MP").toString();
            QString cat   = model->record(i).value("CATEGORIE_MP").toString();
            QString lot   = model->record(i).value("NUM_LOT").toString();
            QString etat  = model->record(i).value("ETAT_MP").toString();
            QString coul  = model->record(i).value("COULEUR").toString();
            double qte    = model->record(i).value("QUANTITE").toDouble();
            QString type  = model->record(i).value("TYPE_STOCKAGE").toString();
            QString qual  = model->record(i).value("QUALITE").toString();

            QTableWidgetItem *itemCode = new QTableWidgetItem(code);
            itemCode->setData(Qt::UserRole, idDb);
            ui->tableStock->setItem(i, 0, itemCode);
            ui->tableStock->setItem(i, 1, new QTableWidgetItem(cat));
            ui->tableStock->setItem(i, 2, new QTableWidgetItem(lot));
            ui->tableStock->setItem(i, 3, new QTableWidgetItem(etat));
            ui->tableStock->setItem(i, 4, new QTableWidgetItem(coul));
            ui->tableStock->setItem(i, 5, new QTableWidgetItem(qual));
            ui->tableStock->setItem(i, 6, new QTableWidgetItem(QString::number(qte)));
            ui->tableStock->setItem(i, 7, new QTableWidgetItem(type));

            MatiereInfo m = {QString::number(idDb), code, cat, lot, etat, coul, qte, type, qual};
            mesMatieres.append(m);
        }

        if (rows == 0) {
            alerteInfo("Recherche", "Aucun résultat trouvé pour \"" + critere + "\".");
        }

        delete model;
    });

    connect(ui->btn_sort_alpha_stock, &QPushButton::clicked, [=](){
        MatierePremiere mp;
        QSqlQueryModel *model = mp.trierParCode();

        ui->tableStock->setRowCount(0);
        mesMatieres.clear();
        int rows = model->rowCount();
        ui->tableStock->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idDb      = model->record(i).value("ID_STOCK_MP").toInt();
            QString code  = model->record(i).value("CODE_MP").toString();
            QString cat   = model->record(i).value("CATEGORIE_MP").toString();
            QString lot   = model->record(i).value("NUM_LOT").toString();
            QString etat  = model->record(i).value("ETAT_MP").toString();
            QString coul  = model->record(i).value("COULEUR").toString();
            double qte    = model->record(i).value("QUANTITE").toDouble();
            QString type  = model->record(i).value("TYPE_STOCKAGE").toString();
            QString qual  = model->record(i).value("QUALITE").toString();

            QTableWidgetItem *itemCode = new QTableWidgetItem(code);
            itemCode->setData(Qt::UserRole, idDb);
            ui->tableStock->setItem(i, 0, itemCode);
            ui->tableStock->setItem(i, 1, new QTableWidgetItem(cat));
            ui->tableStock->setItem(i, 2, new QTableWidgetItem(lot));
            ui->tableStock->setItem(i, 3, new QTableWidgetItem(etat));
            ui->tableStock->setItem(i, 4, new QTableWidgetItem(coul));
            ui->tableStock->setItem(i, 5, new QTableWidgetItem(qual));
            ui->tableStock->setItem(i, 6, new QTableWidgetItem(QString::number(qte)));
            ui->tableStock->setItem(i, 7, new QTableWidgetItem(type));

            MatiereInfo m = {QString::number(idDb), code, cat, lot, etat, coul, qte, type, qual};
            mesMatieres.append(m);
        }

        delete model;
        alerteSucces("Tri effectué", "Les matières sont triées par code (A → Z).");
    });


    connect(ui->tabWidgetClients, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsClients();
        else if (index == 4) showClientFideliteTab();
        else if (index == 5) showClientIaTab();
    });

    connect(ui->btn_search_client, &QPushButton::clicked, this, [=](){
        Client c;
        QSqlQueryModel *model = c.rechercher(ui->le_search_client->text().trimmed());
        remplirTableClients(model);
    });

    connect(ui->btn_sort_alpha_client, &QPushButton::clicked, this, [=](){
        Client c;
        QSqlQueryModel *model = c.trierParNom();
        remplirTableClients(model);
    });

    connect(ui->btn_add_client, &QPushButton::clicked, this, [=](){
        ui->le_client_id->clear();
        ui->le_client_nom->clear();
        ui->le_client_tel->clear();
        ui->le_client_adr->clear();
        ui->le_client_email->clear();
        ui->sb_client_pts->setValue(0);
        ui->tabWidgetClients->setCurrentIndex(1);
    });

    connect(ui->btn_edit_client, &QPushButton::clicked, this, [=](){
        int row = ui->tableClients->currentRow();
        if (row < 0) {
            alerteWarning(QStringLiteral("Sélection"), QStringLiteral("Sélectionnez un client."));
            return;
        }
        QTableWidgetItem *itId = ui->tableClients->item(row, 0);
        if (!itId)
            return;
        ui->le_client_id_modif->setText(itId->text());
        ui->le_client_nom_modif->setText(ui->tableClients->item(row, 1) ? ui->tableClients->item(row, 1)->text() : QString());
        ui->le_client_tel_modif->setText(ui->tableClients->item(row, 2) ? ui->tableClients->item(row, 2)->text() : QString());
        ui->le_client_adr_modif->setText(ui->tableClients->item(row, 3) ? ui->tableClients->item(row, 3)->text() : QString());
        ui->le_client_email_modif->setText(ui->tableClients->item(row, 4) ? ui->tableClients->item(row, 4)->text() : QString());
        QString ptsTxt = ui->tableClients->item(row, 5) ? ui->tableClients->item(row, 5)->text() : QString();
        ptsTxt.remove(QStringLiteral(" pts"));
        ui->sb_client_pts_modif->setValue(ptsTxt.toInt());
        ui->tabWidgetClients->setCurrentIndex(2);
    });

    connect(ui->btn_delete_client, &QPushButton::clicked, this, [=](){
        int row = ui->tableClients->currentRow();
        if (row < 0) {
            alerteWarning(QStringLiteral("Sélection"), QStringLiteral("Sélectionnez un client."));
            return;
        }
        QTableWidgetItem *itId = ui->tableClients->item(row, 0);
        if (!itId)
            return;
        bool ok = false;
        const int id = itId->text().toInt(&ok);
        if (!ok || id <= 0) {
            alerteWarning(QStringLiteral("Suppression"), QStringLiteral("Identifiant client invalide."));
            return;
        }
        Client c;
        if (c.supprimer(id)) {
            alerteSucces(QStringLiteral("Client supprimé"), QStringLiteral("OK."));
            rafraichirListeClients();
            remplirCombosProduitClientEmplacement();
        } else {
            const QString err = c.derniereErreurSaisie();
            alerteErreur(QStringLiteral("Suppression"), err.isEmpty() ? QStringLiteral("Impossible de supprimer.") : err);
        }
    });

    ui->btn_valider_depot->setStyleSheet(styleBtnSave());
    ui->btn_valider_depot->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_depot->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_depot->setCursor(Qt::PointingHandCursor);

    connect(ui->tabWidgetDepot, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsDepot();
        else if (index == 4) showDepotOptimizeTab();
        else if (index == 5) showDepotRavitaillementTab();
    });


    connect(ui->btn_valider_modif_depot, &QPushButton::clicked, [=](){
        const QString errId = Depot::messageIdModificationTexteInvalide(ui->le_depot_id_modif->text());
        if (!errId.isEmpty()) {
            alerteWarning(QStringLiteral("Identifiant"), errId);
            return;
        }
        const int idDepot = ui->le_depot_id_modif->text().trimmed().toInt();
        const QString empM = ui->le_depot_emp_modif->text();
        const QString etaM = ui->le_depot_eta_modif->text();
        const double capM = ui->sb_depot_cap_modif->value();
        const double qteM = ui->sb_depot_act_modif->value();
        Depot d(
            empM,
            etaM,
            capM,
            qteM,
            ui->cb_depot_type_modif->currentText()
        );
        if (!d.modifier(idDepot)) {
            if (!d.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), d.derniereErreurSaisie());
            else
            alerteErreur("Erreur BDD", "Impossible de mettre à jour l'emplacement.");
            return;
        }
        rafraichirListeDepots();
        remplirCombosProduitClientEmplacement();
        ui->tabWidgetDepot->setCurrentIndex(0);
        alerteSucces("Mise à jour", "Emplacement modifié avec succès.");
    });


    connect(ui->btn_cout_produit, &QPushButton::clicked, this, &MainWindow::showProduitCoutDialog);
    connect(ui->btn_hist_mode, &QPushButton::clicked, this, &MainWindow::showHistoriqueModeDialog);

    connect(ui->btn_open_planif_ia, &QPushButton::clicked, this, &MainWindow::ouvrirIAPrediction);

    connect(ui->btn_pdf, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning"); });


    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_export_excel_stock, &QPushButton::clicked, [=](){ exporterCSV(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_print_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_pdf_facture, &QPushButton::clicked, [=](){ exporterFactureClient(); });
    connect(ui->btn_export_excel_client, &QPushButton::clicked, [=](){ exporterCSV(ui->tableClients, "Clients"); });
    connect(ui->btn_pdf_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });


    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int row, int col){
        Q_UNUSED(col);
        if (row < 0 || !ui->tableTimeline->item(row, 0)) return;
        int idSuivi = ui->tableTimeline->item(row, 0)->data(Qt::UserRole).toInt();
        int idPlanif = ui->tableTimeline->item(row, 0)->data(Qt::UserRole + 1).toInt();
        QString produit = ui->tableTimeline->item(row, 2)->text();
        QString etape = ui->tableTimeline->item(row, 4)->text();

        ui->lbl_sel_cmd_title->setText("📋 Commande #" + QString::number(idPlanif) + " — " + produit);
        ui->lbl_sel_cmd->setText("ID Suivi: " + QString::number(idSuivi) + " | Étape: " + etape);

        int idx = ui->cb_etape_suivi->findText(etape);
        if (idx >= 0) ui->cb_etape_suivi->setCurrentIndex(idx);

        double tempsPrevue = 2.0;
        if (etape == "Coupe") tempsPrevue = 2.0;
        else if (etape == "Assemblage") tempsPrevue = 3.0;
        else if (etape == "Couture") tempsPrevue = 4.0;
        else if (etape == "Finition") tempsPrevue = 2.0;
        ui->lbl_temps_prevu->setText(QString::number(tempsPrevue) + " h");
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &t){
        double v = 2.0; if(t=="Assemblage") v=5.0; else if(t=="Couture") v=4.0; else if(t=="Finition") v=1.0;
        ui->lbl_temps_prevu->setText(QString::number(v)+" h");
    });

    connect(ui->btn_valider_etape, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee < 0 || indexCommandeSelectionnee >= mesCommandes.size()) {
            alerteWarning("Sélection", "Veuillez sélectionner un OF dans le tableau Gantt.");
            return;
        }

        QString idStr = mesCommandes[indexCommandeSelectionnee].id;
        int idPlanif = idStr.replace("OF-", "").toInt();

        QString etapeActuelle = ui->cb_etape_suivi->currentText();
        double tempsReel = ui->sb_temps_reel_input->value();
        double tempsPrevu = ui->lbl_temps_prevu->text().split(" ")[0].toDouble();
        double delta = tempsReel - tempsPrevu;
        int alerte = (delta > 0) ? 1 : 0;
        int idEmploye = 1;

        Etape e(idPlanif, idEmploye, etapeActuelle, tempsReel, delta, alerte);

        if (e.ajouter()) {
            if(alerte == 1) {
                ui->lbl_resultat_delta->setText("⚠️ RETARD +" + QString::number(delta, 'f', 1) + "h");
                ui->lbl_resultat_delta->setStyleSheet(
                    "font-weight: 900; font-size: 15px; color: white;"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #c62828, stop:1 #e53935);"
                    "padding: 10px 18px; border-radius: 12px; border: none; min-width: 120px;"
                );
                mesCommandes[indexCommandeSelectionnee].etatEtape = 2;
            } else {
                ui->lbl_resultat_delta->setText("✅ DANS LES TEMPS");
                ui->lbl_resultat_delta->setStyleSheet(
                    "font-weight: 900; font-size: 15px; color: white;"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2e7d32, stop:1 #43a047);"
                    "padding: 10px 18px; border-radius: 12px; border: none; min-width: 120px;"
                );
                mesCommandes[indexCommandeSelectionnee].etatEtape = 1;
            }
            alerteSucces("Succès", "Temps enregistré dans la base avec succès !");
            configurerTimelineGantt();
        } else {
            alerteErreur("Erreur", "Impossible d'enregistrer l'étape.");
        }
    });

    connect(ui->btn_login, &QPushButton::clicked, [=](){
        QString nom = ui->le_login_nom->text().trimmed();
        QString prenom = ui->le_login_prenom->text().trimmed();
        QString mdp_rfid = ui->le_login_mdp->text().trimmed();

        if(nom.isEmpty() || prenom.isEmpty() || mdp_rfid.isEmpty()) {
            alerteWarning("Connexion", "Veuillez remplir tous les champs.");
            return;
        }

        QSqlQuery q;
        q.prepare("SELECT POSTE FROM EMPLOYES WHERE NOM = :nom AND PRENOM = :prenom AND RFID_TAG = :rfid");
        q.bindValue(":nom", nom);
        q.bindValue(":prenom", prenom);
        q.bindValue(":rfid", mdp_rfid);

        if(q.exec() && q.next()) {
            QString poste = q.value("POSTE").toString();
            alerteSucces("Bienvenue", "Connexion réussie !\nPoste : " + poste);

            ui->le_login_mdp->clear();
            setNavigationEnabled(true);
            rafraichirListeCommandes();
            construireDashboardAccueil();
            ui->stackedWidget->setCurrentWidget(ui->page_home);
        } else {
            alerteErreur("Accès Refusé", "Identifiants incorrects.");
        }
    });
    connect(ui->btn_saisie_detail, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee < 0 || indexCommandeSelectionnee >= mesCommandes.size()) {
            alerteWarning("Sélection", "Veuillez d'abord cliquer sur une commande dans le tableau.");
            return;
        }

        QString idStr = mesCommandes[indexCommandeSelectionnee].id;
        int idPlanif = idStr.replace("OF-", "").toInt();

        QDialog d(this);
        d.setWindowTitle("Historique de Fabrication - OF-" + QString::number(idPlanif));
        d.setMinimumSize(500, 300);
        d.setStyleSheet(stylePopup());

        QVBoxLayout *l = new QVBoxLayout(&d);

        QLabel *titre = new QLabel("DÉTAIL DU SUIVI : OF-" + QString::number(idPlanif));
        titre->setStyleSheet("font-size: 16px; font-weight: 800; color: #8d5524; text-transform: uppercase; margin-bottom: 10px;");
        titre->setAlignment(Qt::AlignCenter);
        l->addWidget(titre);

        QTableView *tv = new QTableView();
        Etape tmpEt;
        tv->setModel(tmpEt.rechercherParCommande(idPlanif));
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->horizontalHeader()->setStyleSheet("background-color: #8d5524; color: white; font-weight: bold;");
        tv->setStyleSheet("background: white; border: 1px solid #d7ccc8;");
        tv->setSelectionMode(QAbstractItemView::NoSelection);
        tv->setAlternatingRowColors(true);
        l->addWidget(tv);

        QPushButton *btnFermer = new QPushButton("Fermer l'historique");
        btnFermer->setStyleSheet(styleBtnCancel());
        connect(btnFermer, &QPushButton::clicked, &d, &QDialog::accept);
        l->addWidget(btnFermer, 0, Qt::AlignCenter);

        d.exec();
    });


    auto styleTable = [](QTableWidget *table) {
        if(!table) return;

        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        table->setAlternatingRowColors(true);

        table->verticalHeader()->setDefaultSectionSize(45);

        table->verticalHeader()->setVisible(true);
        table->verticalHeader()->setStyleSheet(
            "QHeaderView::section {"
            "  background-color: #3e2723;"
            "  color: #e0c097;"
            "  padding: 6px;"
            "  border: none;"
            "  font-weight: bold;"
            "  font-size: 12px;"
            "}"
        );

        table->setStyleSheet(
            table->styleSheet() +
            "QTableWidget {"
            "  background-color: #ffffff;"
            "  alternate-background-color: #faf7f2;"
            "  gridline-color: #e8e0d8;"
            "  border: 1px solid #d7ccc8;"
            "  border-radius: 6px;"
            "  font-size: 13px;"
            "  color: #3e2723;"
            "}"
            "QTableWidget::item {"
            "  padding: 8px 12px;"
            "  border-bottom: 1px solid #f0ebe4;"
            "}"
            "QTableWidget::item:selected {"
            "  background-color: #e0c097;"
            "  color: #2c1a16;"
            "}"
            "QHeaderView::section {"
            "  background-color: #2c1a16;"
            "  color: #e0c097;"
            "  padding: 12px 8px;"
            "  border: none;"
            "  font-weight: bold;"
            "  font-size: 12px;"
            "  text-transform: uppercase;"
            "  letter-spacing: 1px;"
            "}"
        );

        table->setFocusPolicy(Qt::StrongFocus);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
    };

    styleTable(ui->tablePlanif);
    styleTable(ui->tableProduits);
    styleTable(ui->tableEmployes);
    styleTable(ui->tableStock);
    styleTable(ui->tableClients);
    styleTable(ui->tableDepot);
    styleTable(ui->tableTimeline);


    QString btnMarron =
        "QPushButton {"
        "  background-color: #795548;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #8d6e63; }";

    QString btnViolet =
        "QPushButton {"
        "  background-color: #7b1fa2;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #9c27b0; }";

    QString btnPDF =
        "QPushButton {"
        "  background-color: #2e7d32;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #388e3c; }";

    QString btnExcel =
        "QPushButton {"
        "  background-color: #0277bd;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #0288d1; }";

    QString btnOr =
        "QPushButton {"
        "  background-color: #8d5524;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #a0673b; }";

    QString btnRouge =
        "QPushButton {"
        "  background-color: #b71c1c;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #d32f2f; }";

    QString btnModifier =
        "QPushButton {"
        "  background-color: #5d4037;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #795548; }";

    QString btnImprimer =
        "QPushButton {"
        "  background-color: #455a64;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 10px 20px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #546e7a; }";

    ui->btn_rechercher->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_planif->setStyleSheet(btnMarron);
    ui->btn_open_planif_ia->setStyleSheet(btnViolet);
    ui->btn_stat_plan->setStyleSheet(btnMarron);
    ui->btn_pdf->setStyleSheet(btnPDF);
    ui->btn_to_add_planif->setStyleSheet(btnOr);
    ui->btn_tri->setStyleSheet(btnMarron);
    ui->btn_modifier_planif->setStyleSheet(btnModifier);
    ui->btn_supprimer_planif->setStyleSheet(btnRouge);

    ui->btn_search_stock->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_stock->setStyleSheet(btnMarron);
    ui->btn_pdf_stock->setStyleSheet(btnPDF);
    ui->btn_export_excel_stock->setStyleSheet(btnExcel);
    ui->btn_print_stock->setStyleSheet(btnImprimer);
    ui->btn_add_stock->setStyleSheet(btnOr);
    ui->btn_edit_stock->setStyleSheet(btnModifier);
    ui->btn_delete_stock->setStyleSheet(btnRouge);

    ui->btn_search_client->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_client->setStyleSheet(btnMarron);
    ui->btn_pdf_facture->setStyleSheet(btnPDF);
    ui->btn_export_excel_client->setStyleSheet(btnExcel);
    ui->btn_add_client->setStyleSheet(btnOr);
    ui->btn_edit_client->setStyleSheet(btnModifier);
    ui->btn_delete_client->setStyleSheet(btnRouge);

    ui->btn_search_depot->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_depot->setStyleSheet(btnMarron);
    ui->btn_pdf_depot->setStyleSheet(btnPDF);
    ui->btn_add_depot->setStyleSheet(btnOr);
    ui->btn_edit_depot->setStyleSheet(btnModifier);
    ui->btn_delete_depot->setStyleSheet(btnRouge);

    ui->btn_search_col->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_prod->setStyleSheet(btnMarron);
    ui->btn_cout_produit->setStyleSheet(btnViolet);
    ui->btn_hist_mode->setStyleSheet(btnViolet);
    ui->btn_stats_prod->setStyleSheet(btnMarron);
    ui->btn_pdf_catalogue->setStyleSheet(btnPDF);
    ui->btn_add_produit->setStyleSheet(btnOr);
    ui->btn_edit_produit->setStyleSheet(btnModifier);
    ui->btn_delete_produit->setStyleSheet(btnRouge);

    ui->btn_search_emp->setStyleSheet(btnMarron);
    ui->btn_pdf_emp->setStyleSheet(btnPDF);
    ui->btn_add_emp->setStyleSheet(btnOr);
    ui->btn_edit_emp->setStyleSheet(btnModifier);
    ui->btn_delete_emp->setStyleSheet(btnRouge);

    QList<QPushButton*> tousLesBoutons = {
        ui->btn_rechercher, ui->btn_sort_alpha_planif, ui->btn_open_planif_ia,
        ui->btn_stat_plan, ui->btn_pdf, ui->btn_to_add_planif, ui->btn_tri,
        ui->btn_modifier_planif, ui->btn_supprimer_planif,
        ui->btn_search_stock, ui->btn_sort_alpha_stock, ui->btn_pdf_stock,
        ui->btn_export_excel_stock, ui->btn_print_stock, ui->btn_add_stock,
        ui->btn_edit_stock, ui->btn_delete_stock,
        ui->btn_search_client, ui->btn_sort_alpha_client, ui->btn_pdf_facture,
        ui->btn_export_excel_client, ui->btn_add_client, ui->btn_edit_client,
        ui->btn_delete_client,
        ui->btn_search_depot, ui->btn_sort_alpha_depot, ui->btn_pdf_depot,
        ui->btn_add_depot, ui->btn_edit_depot, ui->btn_delete_depot,
        ui->btn_search_col, ui->btn_sort_alpha_prod, ui->btn_cout_produit,
        ui->btn_hist_mode, ui->btn_stats_prod, ui->btn_pdf_catalogue,
        ui->btn_add_produit, ui->btn_edit_produit, ui->btn_delete_produit,
        ui->btn_search_emp, ui->btn_pdf_emp, ui->btn_add_emp,
        ui->btn_edit_emp, ui->btn_delete_emp
    };
    for(auto *btn : tousLesBoutons) {
        if(btn) btn->setCursor(Qt::PointingHandCursor);
    }


    QString tabStyle =
        "QTabWidget::pane {"
        "  border: 1px solid #d7ccc8;"
        "  background: white;"
        "  border-radius: 8px;"
        "  margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: #f3f0eb;"
        "  color: #5d4037;"
        "  border: 1px solid #d7ccc8;"
        "  border-bottom-color: #d7ccc8;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  min-width: 150px;"
        "  padding: 10px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #8d5524;"
        "  color: white;"
        "  border-color: #8d5524;"
        "  border-bottom-color: #8d5524;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e0c097;"
        "}";

    ui->tabWidgetPlanif->setStyleSheet(tabStyle);
    ui->tabWidgetProduits->setStyleSheet(tabStyle);
    ui->tabWidgetEmployes->setStyleSheet(tabStyle);
    ui->tabWidgetStock->setStyleSheet(tabStyle);
    ui->tabWidgetClients->setStyleSheet(tabStyle);
    ui->tabWidgetDepot->setStyleSheet(tabStyle);


    if(ui->page_stock_list->layout()) {
        ui->page_stock_list->layout()->setContentsMargins(40, 40, 40, 40);
    }
    if(ui->page_client_list->layout()) {
        ui->page_client_list->layout()->setContentsMargins(40, 40, 40, 40);
    }
    if(ui->page_depot_list->layout()) {
        ui->page_depot_list->layout()->setContentsMargins(40, 40, 40, 40);
    }

    QString tabStyleUnifie =
        "QTabWidget::pane {"
        "  border: 1px solid #d7ccc8;"
        "  background: white;"
        "  border-radius: 8px;"
        "  margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: #f3f0eb;"
        "  color: #5d4037;"
        "  border: 1px solid #d7ccc8;"
        "  border-bottom-color: #d7ccc8;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  min-width: 150px;"
        "  padding: 10px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #8d5524;"
        "  color: white;"
        "  border-color: #8d5524;"
        "  border-bottom-color: #8d5524;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e0c097;"
        "}";

    ui->tabWidgetPlanif->setStyleSheet(tabStyleUnifie);
    ui->tabWidgetProduits->setStyleSheet(tabStyleUnifie);
    ui->tabWidgetEmployes->setStyleSheet(tabStyleUnifie);
    ui->tabWidgetStock->setStyleSheet(tabStyleUnifie);
    ui->tabWidgetClients->setStyleSheet(tabStyleUnifie);
    ui->tabWidgetDepot->setStyleSheet(tabStyleUnifie);

    if(ui->tab_stock_liste->layout()) {
        ui->tab_stock_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }
    if(ui->tab_client_liste->layout()) {
        ui->tab_client_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }
    if(ui->tab_depot_liste->layout()) {
        ui->tab_depot_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }

    ui->lbl_stk->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");
    ui->lbl_cli->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");
    ui->lbl_dep->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");


    QString formStyle =
        "QGroupBox {"
        "  border: none;"
        "  margin-top: 0px;"
        "  padding-top: 0px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 0px;"
        "  padding: 0px;"
        "  color: transparent;"
        "}"
        "QLabel {"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  color: #5d4037;"
        "}"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QDateTimeEdit {"
        "  background-color: white;"
        "  border: 1px solid #d7ccc8;"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "  font-size: 14px;"
        "  color: #3e2723;"
        "  min-height: 20px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {"
        "  border: 2px solid #8d5524;"
        "}";

    QString titreAjoutStyle =
        "font-size: 22px;"
        "font-weight: 800;"
        "color: #8d5524;"
        "margin-bottom: 15px;";

    QString titreModifStyle =
        "font-size: 22px;"
        "font-weight: 800;"
        "color: #5d4037;"
        "margin-bottom: 15px;";

    QString btnValiderStyle =
        "QPushButton {"
        "  background-color: #8d5524;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 14px 40px;"
        "  font-weight: bold;"
        "  font-size: 15px;"
        "  min-width: 200px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #a0673b;"
        "}";

    ui->lbl_titre_ajout_stock->setStyleSheet(titreAjoutStyle);
    ui->lbl_titre_ajout_stock->setAlignment(Qt::AlignCenter);
    ui->lbl_titre_modif_stock->setStyleSheet(titreModifStyle);
    ui->lbl_titre_modif_stock->setAlignment(Qt::AlignCenter);
    ui->tab_stock_ajouter->setStyleSheet(formStyle);
    ui->tab_stock_modifier->setStyleSheet(formStyle);
    ui->btn_valider_stock->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_stock->setStyleSheet(btnValiderStyle);

    ui->gb_prod_new->setTitle("");
    ui->gb_prod_new->setStyleSheet(formStyle);
    if(ui->tab_prod_new->layout()) {
        QLabel *titreProdAjout = new QLabel("➕ NOUVEAU PRODUIT");
        titreProdAjout->setStyleSheet(titreAjoutStyle);
        titreProdAjout->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_prod_new->layout())->insertWidget(0, titreProdAjout);
    }
    ui->btn_valider_produit->setStyleSheet(btnValiderStyle);
    ui->btn_valider_produit->setText("✅ Valider l'ajout");
    ui->btn_valider_produit->setMinimumSize(200, 45);

    ui->gb_prod_edit->setTitle("");
    ui->gb_prod_edit->setStyleSheet(formStyle);
    if(ui->tab_prod_edit->layout()) {
        QLabel *titreProdModif = new QLabel("✏️ MODIFIER PRODUIT");
        titreProdModif->setStyleSheet(titreModifStyle);
        titreProdModif->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_prod_edit->layout())->insertWidget(0, titreProdModif);
    }
    ui->btn_valider_modif_produit->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_produit->setText("💾 Mettre à jour");
    ui->btn_valider_modif_produit->setMinimumSize(200, 45);

    if(ui->tab_prod_new->layout())
        ui->tab_prod_new->layout()->setContentsMargins(40, 30, 40, 30);
    if(ui->tab_prod_edit->layout())
        ui->tab_prod_edit->layout()->setContentsMargins(40, 30, 40, 30);

    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_prod_new->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_prod_edit->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    ui->groupBox->setTitle("");
    ui->groupBox->setStyleSheet(formStyle);
    if(ui->tab_recrutement->layout()) {
        QLabel *titreEmpAjout = new QLabel("➕ RECRUTEMENT NOUVEL EMPLOYÉ");
        titreEmpAjout->setStyleSheet(titreAjoutStyle);
        titreEmpAjout->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_recrutement->layout())->insertWidget(0, titreEmpAjout);
    }
    ui->btn_valider_emp->setStyleSheet(btnValiderStyle);
    ui->btn_valider_emp->setText("✅ Valider le recrutement");
    ui->btn_valider_emp->setMinimumSize(200, 45);

    ui->groupBox_2->setTitle("");
    ui->groupBox_2->setStyleSheet(formStyle);
    if(ui->tab_modifier_dossier->layout()) {
        QLabel *titreEmpModif = new QLabel("✏️ MODIFIER DOSSIER EMPLOYÉ");
        titreEmpModif->setStyleSheet(titreModifStyle);
        titreEmpModif->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_modifier_dossier->layout())->insertWidget(0, titreEmpModif);
    }
    ui->btn_valider_modif_emp->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_emp->setText("💾 Mettre à jour");
    ui->btn_valider_modif_emp->setMinimumSize(200, 45);

    if(ui->tab_recrutement->layout())
        ui->tab_recrutement->layout()->setContentsMargins(40, 30, 40, 30);
    if(ui->tab_modifier_dossier->layout())
        ui->tab_modifier_dossier->layout()->setContentsMargins(40, 30, 40, 30);

    if(auto *fl = qobject_cast<QFormLayout*>(ui->groupBox->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    if(auto *fl = qobject_cast<QFormLayout*>(ui->groupBox_2->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    auto chargerComboDistinct = [&](QComboBox *cb, const QString &colName, const QStringList &fallback) {
        if(!cb) return;
        cb->clear();

        if(cnx && cnx->estConnecte()) {
            QSqlQuery q;
            const QString sql = QString("SELECT DISTINCT %1 FROM EMPLOYES ORDER BY %1").arg(colName);
            if(q.exec(sql)) {
                while(q.next()) {
                    const QString v = q.value(0).toString().trimmed();
                    if(!v.isEmpty())
                        cb->addItem(v);
                }
            }
        }

        auto dejaPresent = [cb](const QString &want) -> bool {
            const QString w = want.trimmed();
            if(w.isEmpty()) return true;
            for(int i = 0; i < cb->count(); ++i) {
                if(cb->itemText(i).compare(w, Qt::CaseInsensitive) == 0)
                    return true;
            }
            return false;
        };

        for(const QString &fb : fallback) {
            if(!dejaPresent(fb))
                cb->addItem(fb);
        }

        if(cb->count() == 0)
            cb->addItems(fallback);

        if(cb->count() > 0) cb->setCurrentIndex(0);
    };

    const QStringList postesEmployeReference = {
        QStringLiteral("Coupe"),
        QStringLiteral("Assemblage"),
        QStringLiteral("Couture"),
        QStringLiteral("Finition"),
        QStringLiteral("Contrôle Qualité"),
        QStringLiteral("Magasinier"),
        QStringLiteral("Maroquinerie sacs"),
        QStringLiteral("Chef Atelier"),
    };

    chargerComboDistinct(
        ui->cb_emp_poste,
        "POSTE",
        postesEmployeReference
    );
    chargerComboDistinct(
        ui->cb_emp_dept,
        "DEPARTEMENT",
        {"Production", "Qualité", "Stock", "Logistique", "Administration", "RH"}
    );

    chargerComboDistinct(
        ui->cb_emp_poste_modif,
        "POSTE",
        postesEmployeReference
    );
    chargerComboDistinct(
        ui->cb_emp_dept_modif,
        "DEPARTEMENT",
        {"Production", "Qualité", "Stock", "Logistique", "Administration", "RH"}
    );

    ui->gb_client_add->setTitle("");
    ui->gb_client_add->setStyleSheet(formStyle);
    if(ui->tab_client_add->layout()) {
        QLabel *titreCliAjout = new QLabel("➕ NOUVEAU CLIENT");
        titreCliAjout->setStyleSheet(titreAjoutStyle);
        titreCliAjout->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_client_add->layout())->insertWidget(0, titreCliAjout);
    }
    ui->btn_valider_client->setStyleSheet(btnValiderStyle);
    ui->btn_valider_client->setText("✅ Valider l'ajout");

    ui->gb_client_edit->setTitle("");
    ui->gb_client_edit->setStyleSheet(formStyle);
    if(ui->tab_client_edit->layout()) {
        QLabel *titreCliModif = new QLabel("✏️ MODIFIER CLIENT");
        titreCliModif->setStyleSheet(titreModifStyle);
        titreCliModif->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_client_edit->layout())->insertWidget(0, titreCliModif);
    }
    ui->btn_valider_modif_client->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_client->setText("💾 Mettre à jour");

    if(ui->tab_client_add->layout())
        ui->tab_client_add->layout()->setContentsMargins(40, 30, 40, 30);
    if(ui->tab_client_edit->layout())
        ui->tab_client_edit->layout()->setContentsMargins(40, 30, 40, 30);

    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_client_add->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_client_edit->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    ui->gb_depot_add->setTitle("");
    ui->gb_depot_add->setStyleSheet(formStyle);
    if(ui->tab_depot_add->layout()) {
        QLabel *titreDepAjout = new QLabel("➕ NOUVEL EMPLACEMENT");
        titreDepAjout->setStyleSheet(titreAjoutStyle);
        titreDepAjout->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_depot_add->layout())->insertWidget(0, titreDepAjout);
    }
    ui->btn_valider_depot->setStyleSheet(btnValiderStyle);
    ui->btn_valider_depot->setText("✅ Valider l'ajout");

    ui->gb_depot_edit->setTitle("");
    ui->gb_depot_edit->setStyleSheet(formStyle);
    if(ui->tab_depot_edit->layout()) {
        QLabel *titreDepModif = new QLabel("✏️ MODIFIER EMPLACEMENT");
        titreDepModif->setStyleSheet(titreModifStyle);
        titreDepModif->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_depot_edit->layout())->insertWidget(0, titreDepModif);
    }
    ui->btn_valider_modif_depot->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_depot->setText("💾 Mettre à jour");

    if(ui->tab_depot_add->layout())
        ui->tab_depot_add->layout()->setContentsMargins(40, 30, 40, 30);
    if(ui->tab_depot_edit->layout())
        ui->tab_depot_edit->layout()->setContentsMargins(40, 30, 40, 30);

    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_depot_add->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_depot_edit->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    ui->gb_form_planif->setTitle("");
    ui->gb_form_planif->setStyleSheet(formStyle);
    if(ui->tab_planif_form->layout()) {
        QLabel *titrePlanAjout = new QLabel("➕ NOUVEL ORDRE DE FABRICATION");
        titrePlanAjout->setStyleSheet(titreAjoutStyle);
        titrePlanAjout->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_planif_form->layout())->insertWidget(0, titrePlanAjout);
    }
    ui->btn_valider_planif->setStyleSheet(btnValiderStyle);
    ui->btn_valider_planif->setText("✅ Valider");
    ui->btn_valider_planif->setMinimumSize(200, 45);

    ui->gb_form_planif_modif->setTitle("");
    ui->gb_form_planif_modif->setStyleSheet(formStyle);
    if(ui->tab_planif_modif->layout()) {
        QLabel *titrePlanModif = new QLabel("✏️ MODIFIER COMMANDE");
        titrePlanModif->setStyleSheet(titreModifStyle);
        titrePlanModif->setAlignment(Qt::AlignCenter);
        static_cast<QVBoxLayout*>(ui->tab_planif_modif->layout())->insertWidget(0, titrePlanModif);
    }
    ui->btn_valider_modif->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif->setText("💾 Mettre à jour");
    ui->btn_valider_modif->setMinimumSize(200, 45);

    if(ui->tab_planif_form->layout())
        ui->tab_planif_form->layout()->setContentsMargins(40, 30, 40, 30);
    if(ui->tab_planif_modif->layout())
        ui->tab_planif_modif->layout()->setContentsMargins(40, 30, 40, 30);

    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_form_planif->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    if(auto *fl = qobject_cast<QFormLayout*>(ui->gb_form_planif_modif->layout())) {
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }


    ui->l_tf->setStyleSheet(
        "font-size: 32px;"
        "font-weight: 900;"
        "color: #2c1a16;"
        "letter-spacing: 1px;"
    );
    ui->l_tf->setText("🏭 Planning & Supervision");

    ui->l_sub_f->setStyleSheet(
        "color: #8d5524;"
        "font-weight: 600;"
        "font-size: 14px;"
        "font-style: italic;"
        "margin-bottom: 10px;"
    );

    ui->l_legende_colors->setStyleSheet(
        "font-weight: bold;"
        "background: white;"
        "padding: 12px 20px;"
        "border: 2px solid #d7ccc8;"
        "border-radius: 12px;"
        "font-size: 13px;"
        "color: #3e2723;"
    );

    ui->btn_refresh_timeline->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #607d8b, stop:1 #90a4ae);"
        "  color: white;"
        "  padding: 10px 22px;"
        "  border: none;"
        "  border-radius: 18px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #78909c, stop:1 #b0bec5);"
        "}"
    );

    ui->btn_saisie_detail->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5d4037, stop:1 #8d6e63);"
        "  color: white;"
        "  padding: 10px 22px;"
        "  border: none;"
        "  border-radius: 18px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #795548, stop:1 #a1887f);"
        "}"
    );

    ui->tableTimeline->setStyleSheet(
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  alternate-background-color: #faf7f2;"
        "  gridline-color: #e8e0d8;"
        "  border: 2px solid #d7ccc8;"
        "  border-radius: 10px;"
        "  font-size: 12px;"
        "  color: #3e2723;"
        "  selection-background-color: #e0c097;"
        "}"
        "QTableWidget::item {"
        "  border-bottom: 1px solid #f0ebe4;"
        "  padding: 4px;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #e0c097;"
        "  color: #2c1a16;"
        "}"
        "QHeaderView::section {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2c1a16, stop:1 #3e2723);"
        "  color: #e0c097;"
        "  padding: 10px 4px;"
        "  border: none;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "}"
    );
    ui->tableTimeline->setAlternatingRowColors(true);
    ui->tableTimeline->verticalHeader()->setDefaultSectionSize(55);
    ui->tableTimeline->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8d5524, stop:1 #a0673b);"
        "  color: white;"
        "  padding: 8px;"
        "  border: none;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "  border-radius: 0px;"
        "}"
    );

    ui->line_sep_fab->setStyleSheet(
        "background-color: #d4af37;"
        "border: none;"
        "min-height: 3px;"
        "max-height: 3px;"
        "margin: 10px 0px;"
    );

    ui->frame_supervision->setStyleSheet(
        "QFrame#frame_supervision {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #faf7f2);"
        "  border: 2px solid #d7ccc8;"
        "  border-radius: 14px;"
        "  padding: 0px;"
        "}"
    );

    ui->lbl_sel_cmd_title->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 900;"
        "color: #2c1a16;"
        "border: none;"
        "letter-spacing: 1px;"
    );

    ui->lbl_sel_cmd->setStyleSheet(
        "color: #8d5524;"
        "font-weight: 600;"
        "font-size: 14px;"
        "border: none;"
        "font-style: italic;"
    );

    ui->l_et->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");
    ui->l_tp->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");
    ui->l_tr->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");

    ui->lbl_temps_prevu->setStyleSheet(
        "font-weight: 900;"
        "color: white;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e65100, stop:1 #ff8f00);"
        "padding: 6px 14px;"
        "border-radius: 10px;"
        "font-size: 14px;"
        "border: none;"
    );

    ui->cb_etape_suivi->setStyleSheet(
        "QComboBox {"
        "  background-color: white;"
        "  border: 2px solid #d7ccc8;"
        "  border-radius: 10px;"
        "  padding: 8px 14px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  color: #3e2723;"
        "  min-width: 140px;"
        "}"
        "QComboBox:focus {"
        "  border: 2px solid #8d5524;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 30px;"
        "}"
    );

    ui->sb_temps_reel_input->setStyleSheet(
        "QDoubleSpinBox {"
        "  background-color: white;"
        "  border: 2px solid #d7ccc8;"
        "  border-radius: 10px;"
        "  padding: 8px 14px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  color: #3e2723;"
        "  min-width: 100px;"
        "}"
        "QDoubleSpinBox:focus {"
        "  border: 2px solid #8d5524;"
        "}"
    );

    ui->btn_valider_etape->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8d5524, stop:1 #6d4c2a);"
        "  color: white;"
        "  font-weight: 900;"
        "  font-size: 15px;"
        "  border: none;"
        "  border-radius: 14px;"
        "  padding: 14px 35px;"
        "  min-width: 160px;"
        "  letter-spacing: 1px;"
        "  text-transform: uppercase;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #a0673b, stop:1 #8d5524);"
        "}"
        "QPushButton:pressed {"
        "  background: #5d4037;"
        "}"
    );
    ui->btn_valider_etape->setText("✅ VALIDER ÉTAPE");
    ui->btn_valider_etape->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_etape->setMinimumSize(180, 50);

    ui->lbl_resultat_delta->setStyleSheet(
        "font-weight: bold;"
        "font-size: 14px;"
        "color: #90a4ae;"
        "border: 2px dashed #d7ccc8;"
        "border-radius: 10px;"
        "padding: 10px 16px;"
        "min-width: 100px;"
        "background: #fafafa;"
    );
    ui->lbl_resultat_delta->setAlignment(Qt::AlignCenter);
    ui->lbl_resultat_delta->setText("En attente...");

    ui->line_sup->setStyleSheet(
        "background-color: #d4af37;"
        "border: none;"
        "min-width: 2px;"
        "max-width: 2px;"
        "margin: 5px 10px;"
    );


}

MainWindow::~MainWindow() {
    if (m_fashionOracleBackendProcess && m_fashionOracleBackendOwned) {
        if (m_fashionOracleBackendProcess->state() != QProcess::NotRunning) {
            m_fashionOracleBackendProcess->terminate();
            if (!m_fashionOracleBackendProcess->waitForFinished(2500))
                m_fashionOracleBackendProcess->kill();
        }
    }
    delete ui;
}


void MainWindow::alerteSucces(const QString &titre, const QString &message) {
    QDialog d(this);
    d.setWindowTitle(titre);
    d.setFixedSize(450, 280);
    d.setWindowFlags(d.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    d.setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f0eb, stop:1 #e8e0d5);"
        "  border-radius: 16px;"
        "}"
    );

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setSpacing(12);
    l->setContentsMargins(30, 25, 30, 25);

    QLabel *ico = new QLabel("✅");
    ico->setStyleSheet("font-size: 48px; border: none;");
    ico->setAlignment(Qt::AlignCenter);
    l->addWidget(ico);

    QLabel *t = new QLabel(titre);
    t->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 900;"
        "color: #2e7d32;"
        "letter-spacing: 1px;"
        "border: none;"
    );
    t->setAlignment(Qt::AlignCenter);
    l->addWidget(t);

    QLabel *m = new QLabel(message);
    m->setStyleSheet(
        "font-size: 15px;"
        "color: #3e2723;"
        "border: none;"
        "padding: 5px;"
    );
    m->setAlignment(Qt::AlignCenter);
    m->setWordWrap(true);
    l->addWidget(m);

    l->addSpacing(8);

    QPushButton *btn = new QPushButton("Parfait !");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2e7d32, stop:1 #43a047);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 14px;"
        "  padding: 12px 40px;"
        "  font-weight: 900;"
        "  font-size: 14px;"
        "  letter-spacing: 1px;"
        "  min-width: 160px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #388e3c, stop:1 #66bb6a);"
        "}"
    );
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

void MainWindow::alerteErreur(const QString &titre, const QString &message) {
    QDialog d(this);
    d.setWindowTitle(titre);
    d.setFixedSize(450, 280);
    d.setWindowFlags(d.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    d.setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f0eb, stop:1 #e8e0d5);"
        "}"
    );

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setSpacing(12);
    l->setContentsMargins(30, 25, 30, 25);

    QLabel *ico = new QLabel("❌");
    ico->setStyleSheet("font-size: 48px; border: none;");
    ico->setAlignment(Qt::AlignCenter);
    l->addWidget(ico);

    QLabel *t = new QLabel(titre);
    t->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 900;"
        "color: #c62828;"
        "letter-spacing: 1px;"
        "border: none;"
    );
    t->setAlignment(Qt::AlignCenter);
    l->addWidget(t);

    QLabel *m = new QLabel(message);
    m->setStyleSheet("font-size: 15px; color: #3e2723; border: none; padding: 5px;");
    m->setAlignment(Qt::AlignCenter);
    m->setWordWrap(true);
    l->addWidget(m);

    l->addSpacing(8);

    QPushButton *btn = new QPushButton("Compris");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #c62828, stop:1 #e53935);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 14px;"
        "  padding: 12px 40px;"
        "  font-weight: 900;"
        "  font-size: 14px;"
        "  letter-spacing: 1px;"
        "  min-width: 160px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #d32f2f, stop:1 #ef5350);"
        "}"
    );
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

void MainWindow::alerteWarning(const QString &titre, const QString &message) {
    QDialog d(this);
    d.setWindowTitle(titre);
    d.setFixedSize(450, 280);
    d.setWindowFlags(d.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    d.setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f0eb, stop:1 #e8e0d5);"
        "}"
    );

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setSpacing(12);
    l->setContentsMargins(30, 25, 30, 25);

    QLabel *ico = new QLabel("⚠️");
    ico->setStyleSheet("font-size: 48px; border: none;");
    ico->setAlignment(Qt::AlignCenter);
    l->addWidget(ico);

    QLabel *t = new QLabel(titre);
    t->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 900;"
        "color: #e65100;"
        "letter-spacing: 1px;"
        "border: none;"
    );
    t->setAlignment(Qt::AlignCenter);
    l->addWidget(t);

    QLabel *m = new QLabel(message);
    m->setStyleSheet("font-size: 15px; color: #3e2723; border: none; padding: 5px;");
    m->setAlignment(Qt::AlignCenter);
    m->setWordWrap(true);
    l->addWidget(m);

    l->addSpacing(8);

    QPushButton *btn = new QPushButton("D'accord");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e65100, stop:1 #ff8f00);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 14px;"
        "  padding: 12px 40px;"
        "  font-weight: 900;"
        "  font-size: 14px;"
        "  letter-spacing: 1px;"
        "  min-width: 160px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ef6c00, stop:1 #ffa726);"
        "}"
    );
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

void MainWindow::alerteInfo(const QString &titre, const QString &message) {
    QDialog d(this);
    d.setWindowTitle(titre);
    d.setFixedSize(450, 280);
    d.setWindowFlags(d.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    d.setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f0eb, stop:1 #e8e0d5);"
        "}"
    );

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setSpacing(12);
    l->setContentsMargins(30, 25, 30, 25);

    QLabel *ico = new QLabel("💡");
    ico->setStyleSheet("font-size: 48px; border: none;");
    ico->setAlignment(Qt::AlignCenter);
    l->addWidget(ico);

    QLabel *t = new QLabel(titre);
    t->setStyleSheet(
        "font-size: 22px;"
        "font-weight: 900;"
        "color: #0277bd;"
        "letter-spacing: 1px;"
        "border: none;"
    );
    t->setAlignment(Qt::AlignCenter);
    l->addWidget(t);

    QLabel *m = new QLabel(message);
    m->setStyleSheet("font-size: 15px; color: #3e2723; border: none; padding: 5px;");
    m->setAlignment(Qt::AlignCenter);
    m->setWordWrap(true);
    l->addWidget(m);

    l->addSpacing(8);

    QPushButton *btn = new QPushButton("Compris");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0277bd, stop:1 #0288d1);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 14px;"
        "  padding: 12px 40px;"
        "  font-weight: 900;"
        "  font-size: 14px;"
        "  letter-spacing: 1px;"
        "  min-width: 160px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0288d1, stop:1 #29b6f6);"
        "}"
    );
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

void MainWindow::construireDashboardAccueil() {
    QWidget *page = ui->page_home;

    if (page->layout()) {
        clearLayout(page->layout());
        delete page->layout();
    }

    page->setStyleSheet(
        "QWidget#page_home {"
        "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #2c1a16, stop:0.4 #3e2723, stop:0.7 #4e342e, stop:1 #5d4037);"
        "}"
    );

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 8px; background: rgba(0,0,0,0.1); border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: rgba(212,175,55,0.4); border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(30, 20, 30, 15);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame { background: rgba(0,0,0,0.25); border: 1px solid rgba(212,175,55,0.2); border-radius: 14px; }"
    );
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(18, 10, 18, 10);

    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/logo.png").scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setFixedSize(45, 45);
    logoLabel->setScaledContents(false);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("border: none; background: transparent;");
    headerLayout->addWidget(logoLabel);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(1);
    QLabel *titre = new QLabel("✨ FIL D'OR — Tableau de Bord");
    titre->setStyleSheet("font-size: 20px; font-weight: 900; color: #d4af37; letter-spacing: 2px; border: none;");
    titleLayout->addWidget(titre);
    QLabel *sousTitre = new QLabel("Vue d'ensemble de l'atelier de maroquinerie de luxe");
    sousTitre->setStyleSheet("font-size: 11px; color: #bcaaa4; font-style: italic; border: none;");
    titleLayout->addWidget(sousTitre);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    QVBoxLayout *dateLayout = new QVBoxLayout();
    dateLayout->setSpacing(1);
    QLabel *dateLabel = new QLabel("📅 " + QDate::currentDate().toString("dddd dd MMMM yyyy"));
    dateLabel->setStyleSheet("font-size: 12px; color: #e0c097; font-weight: bold; border: none;");
    dateLabel->setAlignment(Qt::AlignRight);
    dateLayout->addWidget(dateLabel);
    QLabel *heureLabel = new QLabel("🕐 " + QTime::currentTime().toString("HH:mm"));
    heureLabel->setStyleSheet("font-size: 11px; color: #a1887f; border: none;");
    heureLabel->setAlignment(Qt::AlignRight);
    dateLayout->addWidget(heureLabel);
    headerLayout->addLayout(dateLayout);

    mainLayout->addWidget(headerFrame);

    int totalCommandes = 0, totalPieces = 0, cmdEnCours = 0, cmdRetard = 0, cmdFini = 0;
    int totalEmployes = 0, totalMatieres = 0, totalClients = 0, totalProduits = 0;
    double masseSalariale = 0, volumeStock = 0;
    int totalDepots = 0, depotCritiques = 0;
    double totalCapDepot = 0, totalQteDepot = 0;

    QSqlQuery q;

    if (q.exec("SELECT COUNT(*), NVL(SUM(QUANTITE),0) FROM PLANIFICATION") && q.next()) {
        totalCommandes = q.value(0).toInt();
        totalPieces = q.value(1).toInt();
    }
    if (q.exec("SELECT COUNT(*) FROM PLANIFICATION WHERE UPPER(STATUT) LIKE '%COURS%'") && q.next())
        cmdEnCours = q.value(0).toInt();
    if (q.exec("SELECT COUNT(*) FROM PLANIFICATION WHERE UPPER(STATUT) LIKE '%FINI%' OR UPPER(STATUT) LIKE '%TERMIN%'") && q.next())
        cmdFini = q.value(0).toInt();
    if (q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next())
        cmdRetard = q.value(0).toInt();
    if (q.exec("SELECT COUNT(*), NVL(SUM(SALAIRE),0) FROM EMPLOYES") && q.next()) {
        totalEmployes = q.value(0).toInt();
        masseSalariale = q.value(1).toDouble();
    }
    if (q.exec("SELECT COUNT(*), NVL(SUM(QUANTITE),0) FROM MATIERES_PREMIERES") && q.next()) {
        totalMatieres = q.value(0).toInt();
        volumeStock = q.value(1).toDouble();
    }
    if (q.exec("SELECT COUNT(*) FROM CLIENTS") && q.next())
        totalClients = q.value(0).toInt();
    if (q.exec("SELECT COUNT(*) FROM PRODUITS") && q.next())
        totalProduits = q.value(0).toInt();
    if (q.exec("SELECT COUNT(*), NVL(SUM(CAPACITE_MAX),0), NVL(SUM(QUANTITE_ACTUELLE),0) FROM DEPOTS") && q.next()) {
        totalDepots = q.value(0).toInt();
        totalCapDepot = q.value(1).toDouble();
        totalQteDepot = q.value(2).toDouble();
    }
    if (q.exec("SELECT COUNT(*) FROM DEPOTS WHERE CAPACITE_MAX > 0 AND (QUANTITE_ACTUELLE / CAPACITE_MAX) > 0.9") && q.next())
        depotCritiques = q.value(0).toInt();

    double tauxRetard = (totalCommandes > 0) ? (static_cast<double>(cmdRetard) * 100.0 / totalCommandes) : 0;
    double tauxRempGlobal = (totalCapDepot > 0) ? (totalQteDepot / totalCapDepot) * 100.0 : 0;

    auto creerCarteKPI = [](QString icone, QString valeur, QString label, QString gradientBg, QString borderColor) -> QFrame* {
        QFrame *card = new QFrame();
        card->setMinimumHeight(85);
        card->setMaximumHeight(95);
        card->setStyleSheet(QString(
            "QFrame { background: %1; border-radius: 12px; border: 1px solid %2; }"
        ).arg(gradientBg, borderColor));

        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(14, 8, 14, 8);
        hl->setSpacing(10);

        QLabel *lblIco = new QLabel(icone);
        lblIco->setFixedSize(40, 40);
        lblIco->setAlignment(Qt::AlignCenter);
        lblIco->setStyleSheet("font-size: 22px; border: none; background: rgba(255,255,255,0.15); border-radius: 20px;");
        hl->addWidget(lblIco);

        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(1);

        QLabel *lblVal = new QLabel(valeur);
        lblVal->setStyleSheet("font-size: 22px; font-weight: 900; color: white; border: none;");
        textLayout->addWidget(lblVal);

        QLabel *lblLabel = new QLabel(label);
        lblLabel->setStyleSheet("font-size: 10px; font-weight: 700; color: rgba(255,255,255,0.75); letter-spacing: 1px; text-transform: uppercase; border: none;");
        textLayout->addWidget(lblLabel);

        hl->addLayout(textLayout, 1);
        return card;
    };

    QLabel *sec1 = new QLabel("🏭  PRODUCTION & PLANIFICATION");
    sec1->setStyleSheet("font-size: 12px; font-weight: 800; color: #d4af37; letter-spacing: 2px; border: none;");
    mainLayout->addWidget(sec1);

    QHBoxLayout *kpiRow1 = new QHBoxLayout();
    kpiRow1->setSpacing(10);
    kpiRow1->addWidget(creerCarteKPI("📋", QString::number(totalCommandes), "Ordres en base",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1565c0, stop:1 #0d47a1)", "rgba(21,101,192,0.4)"));
    kpiRow1->addWidget(creerCarteKPI("📦", QString::number(totalPieces), "Pièces planifiées",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #d4af37, stop:1 #8d5524)", "rgba(212,175,55,0.4)"));
    kpiRow1->addWidget(creerCarteKPI("🔄", QString::number(cmdEnCours), "En production",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #00897b, stop:1 #00695c)", "rgba(0,137,123,0.4)"));
    QString colRetard = (tauxRetard > 20)
        ? "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #c62828, stop:1 #b71c1c)"
        : "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #2e7d32, stop:1 #1b5e20)";
    QString bordRetard = (tauxRetard > 20) ? "rgba(198,40,40,0.4)" : "rgba(46,125,50,0.4)";
    kpiRow1->addWidget(creerCarteKPI("⚠️", QString::number(tauxRetard, 'f', 1) + "%", "Taux de retard", colRetard, bordRetard));
    mainLayout->addLayout(kpiRow1);

    QLabel *sec2 = new QLabel("👥  RESSOURCES & STOCK");
    sec2->setStyleSheet("font-size: 12px; font-weight: 800; color: #d4af37; letter-spacing: 2px; border: none;");
    mainLayout->addWidget(sec2);

    QHBoxLayout *kpiRow2 = new QHBoxLayout();
    kpiRow2->setSpacing(10);
    kpiRow2->addWidget(creerCarteKPI("👥", QString::number(totalEmployes), "Employés actifs",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6a1b9a, stop:1 #4a148c)", "rgba(106,27,154,0.4)"));
    kpiRow2->addWidget(creerCarteKPI("💰", QString::number(masseSalariale, 'f', 0) + " DT", "Masse salariale",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ef6c00, stop:1 #e65100)", "rgba(239,108,0,0.4)"));
    kpiRow2->addWidget(creerCarteKPI("🧵", QString::number(volumeStock, 'f', 0) + " u", "Stock matières",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #00838f, stop:1 #006064)", "rgba(0,131,143,0.4)"));
    kpiRow2->addWidget(creerCarteKPI("🧾", QString::number(totalProduits), "Produits",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #8d5524, stop:1 #5d4037)", "rgba(141,85,36,0.4)"));
    mainLayout->addLayout(kpiRow2);

    QLabel *sec3 = new QLabel("🏬  DÉPÔT & CLIENTÈLE");
    sec3->setStyleSheet("font-size: 12px; font-weight: 800; color: #d4af37; letter-spacing: 2px; border: none;");
    mainLayout->addWidget(sec3);

    QHBoxLayout *kpiRow3 = new QHBoxLayout();
    kpiRow3->setSpacing(10);
    kpiRow3->addWidget(creerCarteKPI("🏬", QString::number(totalDepots), "Emplacements dépôt",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #37474f, stop:1 #263238)", "rgba(55,71,79,0.4)"));
    QString colRemp = (tauxRempGlobal > 85)
        ? "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #c62828, stop:1 #b71c1c)"
        : "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #2e7d32, stop:1 #1b5e20)";
    QString bordRemp = (tauxRempGlobal > 85) ? "rgba(198,40,40,0.4)" : "rgba(46,125,50,0.4)";
    kpiRow3->addWidget(creerCarteKPI("📊", QString::number(tauxRempGlobal, 'f', 1) + "%", "Remplissage global", colRemp, bordRemp));
    QString colCritDash = (depotCritiques > 0)
        ? "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #c62828, stop:1 #b71c1c)"
        : "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #2e7d32, stop:1 #1b5e20)";
    QString bordCritDash = (depotCritiques > 0) ? "rgba(198,40,40,0.4)" : "rgba(46,125,50,0.4)";
    kpiRow3->addWidget(creerCarteKPI("⚠️", QString::number(depotCritiques), "Zones critiques", colCritDash, bordCritDash));
    kpiRow3->addWidget(creerCarteKPI("🧾", QString::number(totalClients), "Clients enregistrés",
        "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #4e342e, stop:1 #3e2723)", "rgba(78,52,46,0.4)"));
    mainLayout->addLayout(kpiRow3);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(12);

    QFrame *frameRecent = new QFrame();
    frameRecent->setStyleSheet(
        "QFrame { background: rgba(0,0,0,0.2); border: 1px solid rgba(212,175,55,0.2); border-radius: 14px; }"
    );
    QVBoxLayout *recentLayout = new QVBoxLayout(frameRecent);
    recentLayout->setContentsMargins(14, 12, 14, 12);
    recentLayout->setSpacing(8);

    QLabel *recentTitle = new QLabel("📋  DERNIÈRES COMMANDES");
    recentTitle->setStyleSheet("font-size: 13px; font-weight: 900; color: #d4af37; letter-spacing: 1px; border: none;");
    recentLayout->addWidget(recentTitle);

    QFrame *lineGold1 = new QFrame();
    lineGold1->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d4af37,stop:1 transparent); border:none; min-height:2px; max-height:2px;");
    recentLayout->addWidget(lineGold1);

    QTableWidget *tableRecent = new QTableWidget();
    tableRecent->setColumnCount(5);
    tableRecent->setHorizontalHeaderLabels({"Produit", "Qté", "Matière", "Statut", "Fin Prévue"});
    tableRecent->setStyleSheet(
        "QTableWidget { background: transparent; border: none; color: #e0c097; gridline-color: rgba(212,175,55,0.1); font-size: 11px; }"
        "QTableWidget::item { border-bottom: 1px solid rgba(255,255,255,0.05); padding: 6px; }"
        "QHeaderView::section { background: rgba(212,175,55,0.12); color: #d4af37; border: none; border-bottom: 2px solid rgba(212,175,55,0.3); padding: 8px 4px; font-weight: 900; font-size: 10px; text-transform: uppercase; letter-spacing: 1px; }"
    );
    tableRecent->horizontalHeader()->setStretchLastSection(true);
    tableRecent->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableRecent->verticalHeader()->setVisible(false);
    tableRecent->setSelectionMode(QAbstractItemView::NoSelection);
    tableRecent->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableRecent->setShowGrid(false);

    QSqlQuery qRecent;
    qRecent.exec(
        "SELECT * FROM ("
        "  SELECT pr.DESIGNATION AS Produit, p.QUANTITE, m.CODE_MP AS Matiere, "
        "  p.STATUT, TO_CHAR(p.DATE_FIN_PREVUE, 'DD/MM/YYYY') AS Fin_Prevue "
        "  FROM PLANIFICATION p "
        "  LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "  LEFT JOIN MATIERES_PREMIERES m ON p.ID_STOCK_MP = m.ID_STOCK_MP "
        "  ORDER BY p.ID_COMMANDE DESC"
        ") WHERE ROWNUM <= 6"
    );

    int rowR = 0;
    while (qRecent.next()) {
        tableRecent->insertRow(rowR);

        QTableWidgetItem *iProd = new QTableWidgetItem(qRecent.value(0).toString());
        iProd->setForeground(QColor("#e0c097"));
        QFont fP; fP.setBold(true); iProd->setFont(fP);
        tableRecent->setItem(rowR, 0, iProd);

        QTableWidgetItem *iQte = new QTableWidgetItem(qRecent.value(1).toString());
        iQte->setForeground(QColor("#ffffff"));
        iQte->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(rowR, 1, iQte);

        QTableWidgetItem *iMat = new QTableWidgetItem(qRecent.value(2).toString());
        iMat->setForeground(QColor("#a1887f"));
        iMat->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(rowR, 2, iMat);

        QString statut = qRecent.value(3).toString();
        QTableWidgetItem *iStat = new QTableWidgetItem(statut);
        if (statut.toLower().contains("retard")) {
            iStat->setForeground(QColor("#ef5350")); iStat->setText("🔴 " + statut);
        } else if (statut.toLower().contains("cours")) {
            iStat->setForeground(QColor("#66bb6a")); iStat->setText("🟢 " + statut);
        } else if (statut.toLower().contains("fini") || statut.toLower().contains("termin")) {
            iStat->setForeground(QColor("#29b6f6")); iStat->setText("✅ " + statut);
        } else {
            iStat->setForeground(QColor("#ffa726")); iStat->setText("🟡 " + statut);
        }
        iStat->setTextAlignment(Qt::AlignCenter);
        QFont fS; fS.setBold(true); iStat->setFont(fS);
        tableRecent->setItem(rowR, 3, iStat);

        QTableWidgetItem *iFin = new QTableWidgetItem(qRecent.value(4).toString());
        iFin->setForeground(QColor("#bcaaa4"));
        iFin->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(rowR, 4, iFin);

        tableRecent->setRowHeight(rowR, 34);
        rowR++;
    }

    recentLayout->addWidget(tableRecent, 1);
    bottomLayout->addWidget(frameRecent, 3);

    QFrame *frameAlertes = new QFrame();
    frameAlertes->setStyleSheet(
        "QFrame { background: rgba(0,0,0,0.2); border: 1px solid rgba(212,175,55,0.2); border-radius: 14px; }"
    );
    QVBoxLayout *alertLayout = new QVBoxLayout(frameAlertes);
    alertLayout->setContentsMargins(14, 12, 14, 12);
    alertLayout->setSpacing(6);

    QLabel *alertTitle = new QLabel("🔔  ALERTES & NOTIFICATIONS");
    alertTitle->setStyleSheet("font-size: 13px; font-weight: 900; color: #d4af37; letter-spacing: 1px; border: none;");
    alertLayout->addWidget(alertTitle);

    QFrame *lineGold2 = new QFrame();
    lineGold2->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d4af37,stop:1 transparent); border:none; min-height:2px; max-height:2px;");
    alertLayout->addWidget(lineGold2);

    auto ajouterAlerte = [&](QString ico, QString texte, QString couleur, QString bgAlpha) {
        QFrame *alertItem = new QFrame();
        alertItem->setStyleSheet(QString(
            "QFrame { background: %1; border-left: 3px solid %2; border-radius: 8px; }"
        ).arg(bgAlpha, couleur));

        QHBoxLayout *hl = new QHBoxLayout(alertItem);
        hl->setContentsMargins(8, 6, 8, 6);
        hl->setSpacing(8);

        QLabel *lblIco = new QLabel(ico);
        lblIco->setStyleSheet("font-size: 16px; border: none;");
        lblIco->setFixedWidth(24);
        hl->addWidget(lblIco);

        QLabel *lblTxt = new QLabel(texte);
        lblTxt->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 700; border: none;").arg(couleur));
        lblTxt->setWordWrap(true);
        hl->addWidget(lblTxt, 1);

        alertLayout->addWidget(alertItem);
    };

    if (cmdRetard > 0)
        ajouterAlerte("🚨", QString("%1 commande(s) en retard critique !").arg(cmdRetard), "#ef5350", "rgba(239,83,80,0.08)");

    if (depotCritiques > 0)
        ajouterAlerte("🏬", QString("%1 zone(s) dépôt en surcharge (>90%) !").arg(depotCritiques), "#ef5350", "rgba(239,83,80,0.08)");

    QSqlQuery qStockBas;
    qStockBas.exec("SELECT CODE_MP, QUANTITE FROM MATIERES_PREMIERES WHERE QUANTITE < 20 ORDER BY QUANTITE ASC");
    while (qStockBas.next())
        ajouterAlerte("📉", "Stock bas : " + qStockBas.value(0).toString() + " — " + qStockBas.value(1).toString() + " u", "#ffa726", "rgba(255,167,38,0.08)");

    if (cmdEnCours > 0)
        ajouterAlerte("🔄", QString("%1 commande(s) en fabrication.").arg(cmdEnCours), "#66bb6a", "rgba(102,187,106,0.08)");

    if (cmdFini > 0)
        ajouterAlerte("✅", QString("%1 commande(s) terminée(s).").arg(cmdFini), "#29b6f6", "rgba(41,182,246,0.08)");

    if (cmdRetard == 0 && depotCritiques == 0)
        ajouterAlerte("🌟", "Production optimale — Aucun retard !", "#66bb6a", "rgba(102,187,106,0.08)");

    ajouterAlerte("📊", QString("Stock : %1 u / Dépôt : %2%").arg(volumeStock, 0, 'f', 0).arg(tauxRempGlobal, 0, 'f', 1), "#ce93d8", "rgba(206,147,216,0.08)");
    ajouterAlerte("🧾", QString("%1 client(s) — %2 produit(s)").arg(totalClients).arg(totalProduits), "#e0c097", "rgba(224,192,151,0.08)");

    alertLayout->addStretch();
    bottomLayout->addWidget(frameAlertes, 2);

    mainLayout->addLayout(bottomLayout, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("QFrame { background: rgba(0,0,0,0.15); border: 1px solid rgba(212,175,55,0.15); border-radius: 8px; }");
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(16, 6, 16, 6);

    QLabel *footerLeft = new QLabel("✨ FIL D'OR — L'Excellence de la Maroquinerie");
    footerLeft->setStyleSheet("font-size: 10px; color: rgba(212,175,55,0.6); font-style: italic; border: none;");
    footerLayout->addWidget(footerLeft);
    footerLayout->addStretch();
    QLabel *footerRight = new QLabel("© 2026 — Atelier de Production");
    footerRight->setStyleSheet("font-size: 10px; color: rgba(161,136,127,0.6); border: none;");
    footerLayout->addWidget(footerRight);

    mainLayout->addWidget(footerFrame);

    scrollArea->setWidget(scrollContent);
    pageLayout->addWidget(scrollArea);
}
void MainWindow::rafraichirListeCommandes() {
    QSqlQueryModel *model = tmpOrdre.afficher();
    mesCommandes.clear();
    ui->tablePlanif->setRowCount(0);

    ui->tablePlanif->setColumnCount(8);
    ui->tablePlanif->setHorizontalHeaderLabels({"ID", "Produit", "Qté", "Matière", "Début", "Fin", "Statut", "Employé"});

    int rows = model->rowCount();
    ui->tablePlanif->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        QString idStr = model->record(i).value("ID_COMMANDE").toString();
        QString prod = model->record(i).value("PRODUIT").toString();
        if(prod.isEmpty()) prod = model->record(i).value("DESIGNATION").toString();
        int qte = model->record(i).value("QUANTITE").toInt();
        QString mat = model->record(i).value("MATIERE").toString();
        if(mat.isEmpty()) mat = model->record(i).value("CODE_MP").toString();
        QDate deb = model->record(i).value("DATE_LANCEMENT").toDate();
        QDate fin = model->record(i).value("DATE_FIN_PREVUE").toDate();
        QString stat = model->record(i).value("STATUT").toString();
        QString emp = model->record(i).value("EMPLOYE").toString();
        if(emp.isEmpty()) emp = model->record(i).value("NOM").toString();

        QString finStr = fin.toString("dd/MM/yyyy");

        ui->tablePlanif->setItem(i, 0, new QTableWidgetItem(idStr));
        ui->tablePlanif->setItem(i, 1, new QTableWidgetItem(prod));
        ui->tablePlanif->setItem(i, 2, new QTableWidgetItem(QString::number(qte)));
        ui->tablePlanif->setItem(i, 3, new QTableWidgetItem(mat));
        ui->tablePlanif->setItem(i, 4, new QTableWidgetItem(deb.toString("dd/MM/yyyy")));
        ui->tablePlanif->setItem(i, 5, new QTableWidgetItem(finStr));
        ui->tablePlanif->setItem(i, 6, new QTableWidgetItem(stat));
        ui->tablePlanif->setItem(i, 7, new QTableWidgetItem(emp));

        CommandeInfo c;
        c.id = "OF-" + idStr;
        c.idProduit = prod;
        c.quantite = qte;
        c.idMatiere = mat;
        c.dateDebut = deb; c.dateFinEstimee = finStr; c.statut = stat; c.etatEtape = 0; c.idEmploye = emp;
        mesCommandes.append(c);
    }
    delete model;
}

void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline; t->clear(); t->setRowCount(0);
    int jours = 31; t->setColumnCount(1 + jours);
    QStringList headers; headers << "PRODUIT";
    QDate today = QDate::currentDate();
    for(int i = 0; i < jours; i++) headers << today.addDays(i).toString("dd");
    t->setHorizontalHeaderLabels(headers);
    t->setColumnWidth(0, 200); for(int i = 1; i <= jours; i++) t->setColumnWidth(i, 35);

    t->setRowCount(mesCommandes.size());
    for(int i = 0; i < mesCommandes.size(); i++) {
        t->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id)); t->setRowHeight(i, 50);
        int offset = today.daysTo(mesCommandes[i].dateDebut);
        if(offset < 0) offset = 0;
        QDate dateFin = QDate::fromString(mesCommandes[i].dateFinEstimee, "dd/MM/yyyy");
        int duree = mesCommandes[i].dateDebut.daysTo(dateFin);
        if(duree < 1) duree = 1;

        int d1 = std::max(1, (int)(duree * 0.2));
        int d2 = std::max(1, (int)(duree * 0.3));
        int d3 = std::max(1, (int)(duree * 0.3));
        int d4 = std::max(1, duree - d1 - d2 - d3);

        QColor c1(41,128,185), c2(211,84,0), c3(39,174,96), c4(241,196,15);
        if(mesCommandes[i].etatEtape == 2) { QColor r(192,57,43); c1=c2=c3=c4=r; }
        else if(mesCommandes[i].etatEtape == 1) { QColor v(46,204,113); c1=c2=c3=c4=v; }

        int cur = offset + 1;
        if(cur < jours) dessinerBarre(i, cur, d1, "Cp", c1, Qt::white); cur += d1;
        if(cur < jours) dessinerBarre(i, cur, d2, "As", c2, Qt::white); cur += d2;
        if(cur < jours) dessinerBarre(i, cur, d3, "Co", c3, Qt::white); cur += d3;
        if(cur < jours) dessinerBarre(i, cur, d4, "Fi", c4, Qt::black);
    }
}

void MainWindow::dessinerBarre(int r, int c, int d, QString t, QColor b, QColor f) {
    if(c+d > ui->tableTimeline->columnCount()) d = ui->tableTimeline->columnCount() - c;
    if(d <= 0) return;
    QTableWidgetItem *it = new QTableWidgetItem(t); it->setData(Qt::BackgroundRole, b); it->setForeground(f);
    ui->tableTimeline->setItem(r, c, it); ui->tableTimeline->setSpan(r, c, 1, d);
}

void MainWindow::calculerEtAfficherStats() {
    QSqlQuery q;

    if(q.exec("SELECT COUNT(*) FROM PLANIFICATION") && q.next()) {
        ui->lbl_stat_total_cmd->setText(q.value(0).toString());
    }

    if(q.exec("SELECT SUM(QUANTITE) FROM PLANIFICATION") && q.next()) {
        ui->lbl_stat_total_qty->setText(q.value(0).toString());
    }

    int plan = 0, cours = 0, ret = 0;
    if(q.exec("SELECT STATUT, COUNT(*) FROM PLANIFICATION GROUP BY STATUT")) {
        while(q.next()) {
            QString st = q.value(0).toString().toLower();
            int cnt = q.value(1).toInt();
            if(st.contains("plan")) plan = cnt;
            else if(st.contains("cours")) cours = cnt;
            else if(st.contains("retard")) ret = cnt;
        }
    }

    int total = ui->lbl_stat_total_cmd->text().toInt();
    int maxBase = std::max(1, total);
    ui->pb_cmd_plan->setMaximum(maxBase); ui->pb_cmd_plan->setValue(plan);
    ui->pb_cmd_cours->setMaximum(maxBase); ui->pb_cmd_cours->setValue(cours);
    ui->pb_cmd_ret->setMaximum(maxBase); ui->pb_cmd_ret->setValue(ret);

    if(q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next()) {
        int cmdEnRetard = q.value(0).toInt();
        double retardPct = (total > 0) ? (static_cast<double>(cmdEnRetard) * 100.0 / total) : 0;
        ui->lbl_stat_retard->setText(QString::number(retardPct, 'f', 0) + "%");
    }
}

void MainWindow::remplirCombosProduitClientEmplacement()
{
    if (!ui->cb_prod_client || !ui->cb_prod_client_modif || !ui->cb_prod_empl || !ui->cb_prod_empl_modif)
        return;

    QSqlDatabase dbClients = Connexion::getInstance()->getDatabase();
    auto remplirClients = [dbClients](QComboBox *cb) {
        cb->clear();
        cb->addItem(QStringLiteral("— Sélectionnez un client —"), QVariant());
        if (!dbClients.isOpen())
            return;
        QSqlQuery q(dbClients);
        if (!q.exec(QStringLiteral("SELECT ID_CLIENT, NOM FROM CLIENTS ORDER BY ID_CLIENT"))) {
            qDebug() << "remplirCombosProduit (CLIENTS):" << q.lastError().text();
            return;
        }
        while (q.next()) {
            const int id = q.value(0).toInt();
            const QString nom = q.value(1).toString();
            cb->addItem(QStringLiteral("%1 — %2").arg(id).arg(nom), id);
        }
    };
    QSqlDatabase dbDepots = Connexion::getInstance()->getDatabase();
    auto remplirDepots = [dbDepots](QComboBox *cb) {
        cb->clear();
        cb->addItem(QStringLiteral("— Sélectionnez un dépôt —"), QVariant());
        if (!dbDepots.isOpen())
            return;
        QSqlQuery q(dbDepots);
        if (!q.exec(QStringLiteral("SELECT ID_EMPLACEMENT, ETAGERE, TYPE_STOCKAGE FROM DEPOTS ORDER BY ID_EMPLACEMENT"))) {
            qDebug() << "remplirCombosProduit (DEPOTS):" << q.lastError().text();
            return;
        }
        while (q.next()) {
            const int id = q.value(0).toInt();
            const QString et = q.value(1).toString();
            const QString ty = q.value(2).toString();
            cb->addItem(QStringLiteral("Empl. %1 — %2 (%3)").arg(id).arg(et, ty), id);
        }
    };

    for (QComboBox *cb : {ui->cb_prod_client, ui->cb_prod_client_modif})
        remplirClients(cb);
    for (QComboBox *cb : {ui->cb_prod_empl, ui->cb_prod_empl_modif})
        remplirDepots(cb);
}

void MainWindow::rafraichirListeProduits(const QString &filtreCollection) {
    const bool triActif = ui->tableProduits->isSortingEnabled();
    ui->tableProduits->setSortingEnabled(false);

    ui->tableProduits->clearContents();
    ui->tableProduits->setRowCount(0);
    ui->tableProduits->setColumnCount(8);
    ui->tableProduits->setHorizontalHeaderLabels({"RÉF", "DÉSIGNATION", "COÛT", "COLLECTION", "CUIR", "TEMPS", "CLIENT", "DÉPÔT"});

    mesProduits.clear();

    QSqlQueryModel *model = tmpProduit.afficher(filtreCollection);
    if (!model || model->lastError().isValid()) {
        alerteErreur("Erreur BDD", "Impossible de charger la liste des produits.");
        delete model;
        ui->tableProduits->setSortingEnabled(triActif);
        return;
    }

    const int rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const QSqlRecord rec = model->record(row);
        const int idProd = rec.value(QStringLiteral("ID_PRODUIT")).toInt();
        const QString designation = rec.value(QStringLiteral("DESIGNATION")).toString();
        const double cout = rec.value(QStringLiteral("COUT")).toDouble();
        const QString coll = rec.value(QStringLiteral("COLLECTION")).toString();
        const QString cuir = rec.value(QStringLiteral("TYPE_CUIR_REQUIS")).toString();
        const int temps = rec.value(QStringLiteral("TEMPS_FABRICATION")).toInt();
        const QVariant vCli = rec.value(QStringLiteral("ID_CLIENT"));
        const QVariant vEmp = rec.value(QStringLiteral("ID_EMPLACEMENT"));
        const int idCli = vCli.isNull() ? 0 : vCli.toInt();
        const int idEmp = vEmp.isNull() ? 0 : vEmp.toInt();
        const QString nomCliDb = rec.value(QStringLiteral("NOM_CLIENT")).toString().trimmed();
        const QString etagereDep = rec.value(QStringLiteral("ETA_DEPOT")).toString().trimmed();

        ProduitInfo p;
        p.id_produit = QString::number(idProd);
        p.designation = designation;
        p.cout = cout;
        p.collection = coll;
        p.typeCuir = cuir;
        p.tempsFab = temps;
        p.idClient = idCli > 0 ? QString::number(idCli) : QString();
        p.idEmplacement = idEmp > 0 ? QString::number(idEmp) : QString();
        mesProduits.append(p);

        ui->tableProduits->insertRow(row);
        ui->tableProduits->setItem(row, 0, new SortableNumericTableWidgetItem(QString::number(idProd), idProd));
        ui->tableProduits->setItem(row, 1, new QTableWidgetItem(designation));
        ui->tableProduits->setItem(row, 2, new SortableNumericTableWidgetItem(QString::number(cout, 'f', 2), cout));
        ui->tableProduits->setItem(row, 3, new QTableWidgetItem(coll));
        ui->tableProduits->setItem(row, 4, new QTableWidgetItem(cuir));
        ui->tableProduits->setItem(row, 5, new SortableNumericTableWidgetItem(QString::number(temps), temps));
        const QString colClient = idCli > 0
            ? (nomCliDb.isEmpty() ? QString::number(idCli)
                                 : QStringLiteral("%1 — %2").arg(idCli).arg(nomCliDb))
            : QStringLiteral("—");
        const QString colDepot = idEmp > 0
            ? (etagereDep.isEmpty() ? QString::number(idEmp)
                                    : QStringLiteral("%1 (%2)").arg(idEmp).arg(etagereDep))
            : QStringLiteral("—");
        ui->tableProduits->setItem(row, 6, new QTableWidgetItem(colClient));
        ui->tableProduits->setItem(row, 7, new QTableWidgetItem(colDepot));
    }

    delete model;
    ui->tableProduits->setSortingEnabled(triActif);
}


void MainWindow::on_btn_edit_produit_clicked() {
    const int idx = ui->tableProduits->currentRow();
    if(idx < 0) {
        alerteWarning("Sélection", "Sélectionnez un produit.");
        return;
    }
    preparerFormulaireProduit(true, idx);
}

void MainWindow::on_tableProduits_cellClicked(int row, int column) {
    Q_UNUSED(column);
    if(row < 0) return;
    QTableWidgetItem *itId = ui->tableProduits->item(row, 0);
    if(!itId) return;
    selectedProdId = itId->text().toInt();
}

void MainWindow::on_btn_delete_produit_clicked() {
    const int idx = ui->tableProduits->currentRow();
    if(idx < 0) {
        alerteWarning("Sélection", "Sélectionnez un produit à supprimer.");
        return;
    }

    QTableWidgetItem *itId = ui->tableProduits->item(idx, 0);
    bool okId = false;
    const int id = itId ? itId->text().toInt(&okId) : 0;
    if(!okId || id <= 0) {
        alerteErreur("Erreur", "Identifiant produit invalide.");
        return;
    }

    QString errSql;
    if(tmpProduit.supprimer(id, &errSql)) {
        alerteSucces("Succès", "Produit supprimé !");
        rafraichirListeProduits(ui->le_search_coll->text());
        ui->tabWidgetProduits->setCurrentIndex(0);
        selectedProdId = -1;
        rowToEdit = -1;
    } else {
        const QString detail = errSql.isEmpty() ? QStringLiteral("Raison inconnue.") : errSql;
        alerteErreur("Erreur", "Suppression impossible.\n\n" + detail);
    }
}

bool MainWindow::validerMatiereAjout()
{
    const QString code = ui->le_stock_code->text().trimmed();
    const QString lot = ui->le_stock_lot->text().trimmed();
    const QString coul = ui->le_stock_coul->text().trimmed();

    static const QRegularExpression rxCode(QStringLiteral("^[A-Z]{2,4}-20\\d{2}-\\d{3}$"));
    static const QRegularExpression rxLot(QStringLiteral("^LOT-20\\d{2}-[A-Z]$"));
    static const QRegularExpression rxCouleur(QStringLiteral("^[A-Za-zÀ-ÿ ]{3,20}$"));

    if (!rxCode.match(code).hasMatch()) {
        alerteErreur(QStringLiteral("Validation"),
                       QStringLiteral("Code MP invalide (ex: CUI-2024-001)."));
        return false;
    }
    if (!rxLot.match(lot).hasMatch()) {
        alerteErreur(QStringLiteral("Validation"),
                       QStringLiteral("Numéro de lot invalide (ex: LOT-2024-A)."));
        return false;
    }
    if (!rxCouleur.match(coul).hasMatch()) {
        alerteErreur(QStringLiteral("Validation"),
                       QStringLiteral("Couleur invalide (lettres et espaces, 3 à 20 caractères)."));
        return false;
    }
    if (ui->sb_stock_qte->value() <= 0.0) {
        alerteErreur(QStringLiteral("Validation"), QStringLiteral("La quantité doit être > 0."));
        return false;
    }

    return true;
}

void MainWindow::on_btn_valider_stock_clicked()
{
    if (!validerMatiereAjout())
        return;

    const QString code = ui->le_stock_code->text().trimmed();
    MatierePremiere mp(
        code,
        ui->cb_stock_cat->currentText(),
        ui->le_stock_lot->text().trimmed(),
        ui->cb_stock_etat->currentText(),
        ui->le_stock_coul->text().trimmed(),
        ui->sb_stock_qte->value(),
        ui->cb_stock_type->currentText(),
        ui->cb_stock_qual->currentText());

    if (mp.ajouter()) {
        alerteSucces(QStringLiteral("Matière ajoutée"),
                     QStringLiteral("La matière première %1 a été enregistrée avec succès !").arg(code));
        rafraichirListeMatieres();
        ui->tabWidgetStock->setCurrentIndex(0);
    } else {
        if (!mp.derniereErreurSaisie().isEmpty())
            alerteErreur(QStringLiteral("Validation/BDD"), mp.derniereErreurSaisie());
        else
            alerteErreur(QStringLiteral("Validation/BDD"),
                          QStringLiteral("Données invalides ou insertion impossible."));
    }
}


void MainWindow::on_btn_valider_client_clicked()
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx || !cnx->estConnecte()) {
        alerteErreur(QStringLiteral("Base de données"), QStringLiteral("Connexion Oracle requise."));
        return;
    }

    const QString nom = ui->le_client_nom->text();
    const QString tel = ui->le_client_tel->text().trimmed();
    const QString adr = ui->le_client_adr->text().trimmed();
    const QString mail = ui->le_client_email->text().trimmed();
    const int pts = ui->sb_client_pts->value();

    const QString errId = messageValidationIdClientAjoutOptionnel(ui->le_client_id->text());
    if (!errId.isEmpty()) {
        alerteWarning(QStringLiteral("Validation"), errId);
        return;
    }
    const QString errSaisie = messageValidationSaisieClient(nom, tel, adr, mail, pts);
    if (!errSaisie.isEmpty()) {
        alerteWarning(QStringLiteral("Validation"), errSaisie);
        return;
    }

    bool okId = false;
    const int idSaisi = ui->le_client_id->text().trimmed().toInt(&okId);
    const int id = okId ? idSaisi : 0;

    Client c(
        id,
        nom.trimmed(),
        tel,
        adr,
        mail,
        pts);

    if (c.ajouter()) {
        alerteSucces(QStringLiteral("Client ajouté"), QStringLiteral("OK."));
        rafraichirListeClients();
        remplirCombosProduitClientEmplacement();
        ui->tabWidgetClients->setCurrentIndex(0);
        ui->le_client_id->clear();
        ui->le_client_nom->clear();
        ui->le_client_tel->clear();
        ui->le_client_adr->clear();
        ui->le_client_email->clear();
        ui->sb_client_pts->setValue(0);
    } else {
        const QString err = c.derniereErreurSaisie();
        alerteErreur(QStringLiteral("Validation/BDD"),
                     err.isEmpty() ? QStringLiteral("Données invalides ou insertion impossible.") : err);
    }
}

void MainWindow::on_btn_valider_modif_client_clicked()
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx || !cnx->estConnecte()) {
        alerteErreur(QStringLiteral("Base de données"), QStringLiteral("Connexion Oracle requise."));
        return;
    }

    bool ok = false;
    const int id = ui->le_client_id_modif->text().trimmed().toInt(&ok);
    if (!ok || id <= 0) {
        alerteWarning(QStringLiteral("Identifiant"), QStringLiteral("Identifiant client invalide."));
        return;
    }

    const QString nom = ui->le_client_nom_modif->text();
    const QString tel = ui->le_client_tel_modif->text().trimmed();
    const QString adr = ui->le_client_adr_modif->text().trimmed();
    const QString mail = ui->le_client_email_modif->text().trimmed();
    const int pts = ui->sb_client_pts_modif->value();

    const QString errSaisie = messageValidationSaisieClient(nom, tel, adr, mail, pts);
    if (!errSaisie.isEmpty()) {
        alerteWarning(QStringLiteral("Validation"), errSaisie);
        return;
    }

    Client c(
        id,
        nom.trimmed(),
        tel,
        adr,
        mail,
        pts);

    if (c.modifier(id)) {
        alerteSucces(QStringLiteral("Client modifié"), QStringLiteral("OK."));
        rafraichirListeClients();
        remplirCombosProduitClientEmplacement();
        ui->tabWidgetClients->setCurrentIndex(0);
    } else {
        const QString err = c.derniereErreurSaisie();
        alerteErreur(QStringLiteral("Modification"),
                     err.isEmpty() ? QStringLiteral("Échec de modification.") : err);
    }
}

void MainWindow::on_btn_valider_produit_clicked() {
    const QString nom = ui->le_prod_nom->text();
    const double cout = ui->sb_prod_cout->value();
    const QString coll = ui->cb_prod_coll->currentText();
    const QString cuir = ui->cb_prod_cuir->currentText();
    const int temps = ui->sb_prod_temps->value();

    const QString errSaisie = messageValidationSaisieProduit(nom, cout, coll, cuir, temps);
    if(!errSaisie.isEmpty()) {
        alerteWarning("Erreur", errSaisie);
        return;
    }

    const QString nomTrim = nom.trimmed();

    const int idClient = comboIdData(ui->cb_prod_client);
    const int idEmpl = comboIdData(ui->cb_prod_empl);
    const QString errFk = messageValidationFkProduitObligatoires(idClient, idEmpl, ui->cb_prod_client, ui->cb_prod_empl);
    if (!errFk.isEmpty()) {
        alerteWarning(QStringLiteral("Client / Dépôt"), errFk);
        return;
    }

    Produit p(0, nomTrim, cout, coll.trimmed(), cuir.trimmed(), temps, idClient, idEmpl);
    if(p.ajouter()) {
        alerteSucces("Succès", "Produit ajouté !");
        rafraichirListeProduits(ui->le_search_coll->text());
        ui->tabWidgetProduits->setCurrentIndex(0);
        ui->le_prod_nom->clear();
        ui->sb_prod_cout->setValue(0);
        ui->sb_prod_temps->setValue(1);
    } else {
        const QString err = p.derniereErreurSaisie();
        alerteErreur(QStringLiteral("Produit / Oracle"),
                     err.isEmpty()
                         ? QStringLiteral("Ajout échoué (vérifiez la clé étrangère ID_CLIENT et que le client existe en base).")
                         : err);
    }
}

void MainWindow::on_btn_valider_modif_produit_clicked() {
    if(selectedProdId <= 0) {
        alerteWarning("Sélection", "Sélectionnez d'abord un produit à modifier.");
        return;
    }

    const QString nom = ui->le_prod_nom_modif->text();
    const double cout = ui->sb_prod_cout_modif->value();
    const QString coll = ui->cb_prod_coll_modif->currentText();
    const QString cuir = ui->cb_prod_cuir_modif->currentText();
    const int temps = ui->sb_prod_temps_modif->value();

    const QString errSaisie = messageValidationSaisieProduit(nom, cout, coll, cuir, temps);
    if(!errSaisie.isEmpty()) {
        alerteWarning("Erreur", errSaisie);
        return;
    }

    const QString nomTrim = nom.trimmed();

    const int idClient = comboIdData(ui->cb_prod_client_modif);
    const int idEmpl = comboIdData(ui->cb_prod_empl_modif);
    const QString errFk = messageValidationFkProduitObligatoires(idClient, idEmpl, ui->cb_prod_client_modif, ui->cb_prod_empl_modif);
    if (!errFk.isEmpty()) {
        alerteWarning(QStringLiteral("Client / Dépôt"), errFk);
        return;
    }

    Produit p(selectedProdId, nomTrim, cout, coll.trimmed(), cuir.trimmed(), temps, idClient, idEmpl);
    if(p.modifier(selectedProdId)) {
        alerteSucces("Succès", "Produit modifié avec succès !");
        rafraichirListeProduits(ui->le_search_coll->text());
        ui->tabWidgetProduits->setCurrentIndex(0);
        selectedProdId = -1;
        rowToEdit = -1;
    } else {
        const QString err = p.derniereErreurSaisie();
        alerteErreur(QStringLiteral("Produit / Oracle"),
                     err.isEmpty()
                         ? QStringLiteral("Mise à jour échouée (contraintes Oracle ou session).")
                         : err);
    }
}

void MainWindow::rafraichirListeEmployes() {
    ui->tableEmployes->setRowCount(0);
    ui->tableEmployes->setColumnCount(7);
    ui->tableEmployes->setHorizontalHeaderLabels({"ID", "NOM", "PRÉNOM", "POSTE", "DÉPARTEMENT", "SALAIRE", "EMBAUCHE"});

    mesEmployes.clear();

    employe emp;
    QSqlQueryModel *model = emp.afficher();
    const int rows = model ? model->rowCount() : 0;
    ui->tableEmployes->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        const QSqlRecord rec = model->record(i);

        EmployeInfo info;
        info.id = QString::number(rec.value("ID_EMPLOYE").toInt());
        info.nom = rec.value("NOM").toString();
        info.prenom = rec.value("PRENOM").toString();
        info.poste = rec.value("POSTE").toString();
        info.email = rec.value("EMAIL").toString();
        info.telephone = rec.value("TELEPHONE").toString();
        info.departement = rec.value("DEPARTEMENT").toString();
        info.dateEmbauche = rec.value("DATE_EMBAUCHE").toDate();
        info.salaire = rec.value("SALAIRE").toDouble();
        info.rfid = rec.value("RFID_TAG").toString();

        mesEmployes.append(info);

        ui->tableEmployes->setItem(i, 0, new QTableWidgetItem(info.id));
        ui->tableEmployes->item(i, 0)->setData(Qt::UserRole, i);
        ui->tableEmployes->setItem(i, 1, new QTableWidgetItem(info.nom));
        ui->tableEmployes->setItem(i, 2, new QTableWidgetItem(info.prenom));
        ui->tableEmployes->setItem(i, 3, new QTableWidgetItem(info.poste));
        ui->tableEmployes->setItem(i, 4, new QTableWidgetItem(info.departement));
        ui->tableEmployes->setItem(i, 5, new QTableWidgetItem(QString::number(info.salaire, 'f', 0) + " DT"));
        ui->tableEmployes->setItem(i, 6, new QTableWidgetItem(info.dateEmbauche.toString("dd/MM/yyyy")));
    }

    if(employeTriAlphaActif && ui->tableEmployes) {
        ui->tableEmployes->setSortingEnabled(true);
        ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre);
    }

    delete model;
}
void MainWindow::rafraichirListeMatieres() {
    QSqlQueryModel *model = tmpMatiere.afficher();

    ui->tableStock->setRowCount(0);
    ui->tableStock->setColumnCount(8);
    ui->tableStock->setHorizontalHeaderLabels({
        "Code", "Catégorie", "Lot", "État", "Couleur", "Qualité", "Qté", "Stockage"
    });

    int rows = model->rowCount();
    ui->tableStock->setRowCount(rows);

    mesMatieres.clear();

    for (int i = 0; i < rows; i++) {
        int idDb      = model->record(i).value("ID_STOCK_MP").toInt();
        QString code  = model->record(i).value("CODE_MP").toString();
        QString cat   = model->record(i).value("CATEGORIE_MP").toString();
        QString lot   = model->record(i).value("NUM_LOT").toString();
        QString etat  = model->record(i).value("ETAT_MP").toString();
        QString coul  = model->record(i).value("COULEUR").toString();
        double qte    = model->record(i).value("QUANTITE").toDouble();
        QString type  = model->record(i).value("TYPE_STOCKAGE").toString();
        QString qual  = model->record(i).value("QUALITE").toString();

        QTableWidgetItem *itemCode = new QTableWidgetItem(code);
        itemCode->setData(Qt::UserRole, idDb);
        ui->tableStock->setItem(i, 0, itemCode);
        ui->tableStock->setItem(i, 1, new QTableWidgetItem(cat));
        ui->tableStock->setItem(i, 2, new QTableWidgetItem(lot));
        ui->tableStock->setItem(i, 3, new QTableWidgetItem(etat));
        ui->tableStock->setItem(i, 4, new QTableWidgetItem(coul));
        ui->tableStock->setItem(i, 5, new QTableWidgetItem(qual));
        ui->tableStock->setItem(i, 6, new QTableWidgetItem(QString::number(qte)));
        ui->tableStock->setItem(i, 7, new QTableWidgetItem(type));

        MatiereInfo m = {QString::number(idDb), code, cat, lot, etat, coul, qte, type, qual};
        mesMatieres.append(m);
    }

    delete model;
}

void MainWindow::remplirTableClients(QSqlQueryModel *model)
{
    if (!model)
        return;

    ui->tableClients->clearContents();
    ui->tableClients->setRowCount(0);
    ui->tableClients->setColumnCount(6);
    ui->tableClients->setHorizontalHeaderLabels({
        QStringLiteral("ID CLIENT"),
        QStringLiteral("NOM"),
        QStringLiteral("TÉLÉPHONE"),
        QStringLiteral("ADRESSE"),
        QStringLiteral("EMAIL"),
        QStringLiteral("FIDÉLITÉ")
    });

    const int rows = model->rowCount();
    ui->tableClients->setRowCount(rows);
    mesClients.clear();

    for (int i = 0; i < rows; ++i) {
        const QSqlRecord rec = model->record(i);
        const int idDb = rec.value(QStringLiteral("ID_CLIENT")).toInt();
        const QString nom = rec.value(QStringLiteral("NOM")).toString();
        const QString tel = rec.value(QStringLiteral("TELEPHONE")).toString();
        const QString adr = rec.value(QStringLiteral("ADRESSE")).toString();
        const QString mail = rec.value(QStringLiteral("EMAIL")).toString();
        const int pts = rec.value(QStringLiteral("POINTS_FIDELITE")).toInt();

        ui->tableClients->setItem(i, 0, new QTableWidgetItem(QString::number(idDb)));
        ui->tableClients->setItem(i, 1, new QTableWidgetItem(nom));
        ui->tableClients->setItem(i, 2, new QTableWidgetItem(tel));
        ui->tableClients->setItem(i, 3, new QTableWidgetItem(adr));
        ui->tableClients->setItem(i, 4, new QTableWidgetItem(mail));
        ui->tableClients->setItem(i, 5, new QTableWidgetItem(QString::number(pts) + QStringLiteral(" pts")));

        mesClients.append({QString::number(idDb), nom, tel, adr, mail, pts});
    }

    delete model;
}

void MainWindow::rafraichirListeClients()
{
    Connexion *cnx = Connexion::getInstance();
    if (cnx && cnx->estConnecte()) {
        Client c;
        remplirTableClients(c.afficher());
        return;
    }

    ui->tableClients->clearContents();
    ui->tableClients->setRowCount(0);
    ui->tableClients->setColumnCount(6);
    ui->tableClients->setHorizontalHeaderLabels({
        QStringLiteral("ID CLIENT"),
        QStringLiteral("NOM"),
        QStringLiteral("TÉLÉPHONE"),
        QStringLiteral("ADRESSE"),
        QStringLiteral("EMAIL"),
        QStringLiteral("FIDÉLITÉ")
    });
    for (int i = 0; i < mesClients.size(); ++i) {
        ui->tableClients->insertRow(i);
        ui->tableClients->setItem(i, 0, new QTableWidgetItem(mesClients[i].id));
        ui->tableClients->setItem(i, 1, new QTableWidgetItem(mesClients[i].nom));
        ui->tableClients->setItem(i, 2, new QTableWidgetItem(mesClients[i].telephone));
        ui->tableClients->setItem(i, 3, new QTableWidgetItem(mesClients[i].adresse));
        ui->tableClients->setItem(i, 4, new QTableWidgetItem(mesClients[i].email));
        ui->tableClients->setItem(i, 5, new QTableWidgetItem(QString::number(mesClients[i].pointsFidelite) + QStringLiteral(" pts")));
    }
}
void MainWindow::rafraichirListeDepots() {
    QSqlQueryModel *model = tmpDepot.afficher();

    ui->tableDepot->setRowCount(0);
    ui->tableDepot->setColumnCount(7);
    ui->tableDepot->setHorizontalHeaderLabels({
        "ID", "Emplacement", "Étagère", "Capacité Max", "Quantité", "Type", "Remplissage"
    });

    int rows = model->rowCount();
    ui->tableDepot->setRowCount(rows);
    mesDepots.clear();

    for (int i = 0; i < rows; i++) {
        int idDb = model->record(i).value("ID_EMPLACEMENT").toInt();
        QString et = model->record(i).value("ETAGERE").toString();
        double cap = model->record(i).value("CAPACITE_MAX").toDouble();
        double qte = model->record(i).value("QUANTITE_ACTUELLE").toDouble();
        QString type = model->record(i).value("TYPE_STOCKAGE").toString();

        QString remplissage = (cap > 0)
            ? QString::number((qte / cap) * 100.0, 'f', 1) + "%"
            : "0%";

        ui->tableDepot->setItem(i, 0, new QTableWidgetItem(QString::number(idDb)));
        ui->tableDepot->setItem(i, 1, new QTableWidgetItem("Empl. " + QString::number(idDb)));
        ui->tableDepot->setItem(i, 2, new QTableWidgetItem(et));
        ui->tableDepot->setItem(i, 3, new QTableWidgetItem(QString::number(cap)));
        ui->tableDepot->setItem(i, 4, new QTableWidgetItem(QString::number(qte)));
        ui->tableDepot->setItem(i, 5, new QTableWidgetItem(type));
        ui->tableDepot->setItem(i, 6, new QTableWidgetItem(remplissage));

        ui->tableDepot->item(i, 2)->setData(Qt::UserRole, idDb);

        DepotInfo dp = {
            QString::number(idDb),
            QString("Empl. %1").arg(idDb),
            et,
            cap,
            qte,
            type
        };
        mesDepots.append(dp);
    }

    delete model;
}
void MainWindow::exporterPDF(QTableWidget *table, QString titre) {
    if(!table) return;
    QString f = QFileDialog::getSaveFileName(this, "Export", titre + ".pdf", "PDF (*.pdf)");
    if(f.isEmpty()) return; if(QFileInfo(f).suffix().isEmpty()) f += ".pdf";
    QPrinter printer(QPrinter::PrinterResolution); printer.setOutputFormat(QPrinter::PdfFormat); printer.setPageSize(QPageSize(QPageSize::A4)); printer.setOutputFileName(f);
    QString html = "<h1>" + titre + "</h1><table border='1' cellspacing='0' cellpadding='5' width='100%'><thead><tr>";
    for(int c=0; c<table->columnCount(); c++) html += "<th>" + table->horizontalHeaderItem(c)->text() + "</th>";
    html += "</tr></thead><tbody>";
    for(int r=0; r<table->rowCount(); r++) {
        html += "<tr>";
        for(int c=0; c<table->columnCount(); c++) html += "<td>" + (table->item(r,c) ? table->item(r,c)->text() : "") + "</td>";
        html += "</tr>";
    }
    html += "</tbody></table>";
    QTextDocument doc; doc.setHtml(html); doc.print(&printer);
    alerteSucces("Succès", "Export PDF réussi !");
}
void MainWindow::exporterCSV(QTableWidget *table, const QString &titre) {
    if(!table) return;
    QString f = QFileDialog::getSaveFileName(this, "Export", titre + ".csv", "CSV (*.csv)");
    if(f.isEmpty()) return;
    QFile file(f); if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream ts(&file);
    for(int c=0; c<table->columnCount(); c++) ts << table->horizontalHeaderItem(c)->text() << ";"; ts << "\n";
    for(int r=0; r<table->rowCount(); r++) {
        for(int c=0; c<table->columnCount(); c++) ts << (table->item(r,c) ? table->item(r,c)->text() : "") << ";";
        ts << "\n";
    }
    file.close(); alerteSucces("Succès", "Export CSV réussi !");
}


void MainWindow::remplirTableEtapes(QSqlQueryModel *model) {
    ui->tableTimeline->setRowCount(0);
    ui->tableTimeline->setColumnCount(8);
    ui->tableTimeline->setHorizontalHeaderLabels({
        "ID Suivi", "N° Cmd", "Produit", "Employé",
        "Étape", "Temps Réel (h)", "Delta (h)", "Alerte"
    });

    int rows = model->rowCount();
    ui->tableTimeline->setRowCount(rows);

    for (int i = 0; i < rows; i++) {
        int idSuivi = model->record(i).value("ID_SUIVI").toInt();
        int idPlanif = model->record(i).value("ID_PLANIFICATION").toInt();
        QString produit = model->record(i).value("PRODUIT").toString();
        QString employe = model->record(i).value("EMPLOYE").toString();
        QString etape = model->record(i).value("ETAPE_ACTUELLE").toString();
        double tempsReel = model->record(i).value("TEMPS_REEL_PASSE").toDouble();
        double delta = model->record(i).value("DELTA").toDouble();
        int alerte = model->record(i).value("ALERTE_ACTIVE").toInt();

        QColor couleurEtape;
        if (etape == "Coupe") couleurEtape = QColor("#1565c0");
        else if (etape == "Assemblage") couleurEtape = QColor("#ef6c00");
        else if (etape == "Couture") couleurEtape = QColor("#2e7d32");
        else if (etape == "Finition") couleurEtape = QColor("#f9a825");
        else couleurEtape = QColor("#757575");

        QTableWidgetItem *itemId = new QTableWidgetItem(QString::number(idSuivi));
        itemId->setData(Qt::UserRole, idSuivi);
        itemId->setData(Qt::UserRole + 1, idPlanif);
        ui->tableTimeline->setItem(i, 0, itemId);

        ui->tableTimeline->setItem(i, 1, new QTableWidgetItem(QString::number(idPlanif)));

        QTableWidgetItem *itemProd = new QTableWidgetItem(produit);
        QFont fBold; fBold.setBold(true); itemProd->setFont(fBold);
        ui->tableTimeline->setItem(i, 2, itemProd);

        ui->tableTimeline->setItem(i, 3, new QTableWidgetItem(employe));

        QTableWidgetItem *itemEtape = new QTableWidgetItem(etape);
        itemEtape->setForeground(couleurEtape);
        itemEtape->setFont(fBold);
        ui->tableTimeline->setItem(i, 4, itemEtape);

        QTableWidgetItem *itemTemps = new QTableWidgetItem(QString::number(tempsReel, 'f', 1) + " h");
        itemTemps->setTextAlignment(Qt::AlignCenter);
        ui->tableTimeline->setItem(i, 5, itemTemps);

        QTableWidgetItem *itemDelta = new QTableWidgetItem(
            (delta > 0 ? "+" : "") + QString::number(delta, 'f', 1) + " h"
        );
        itemDelta->setTextAlignment(Qt::AlignCenter);
        if (delta > 0) itemDelta->setForeground(QColor("#c62828"));
        else if (delta < 0) itemDelta->setForeground(QColor("#2e7d32"));
        else itemDelta->setForeground(QColor("#757575"));
        itemDelta->setFont(fBold);
        ui->tableTimeline->setItem(i, 6, itemDelta);

        QTableWidgetItem *itemAlerte = new QTableWidgetItem(alerte ? "🔴 OUI" : "🟢 Non");
        itemAlerte->setTextAlignment(Qt::AlignCenter);
        if (alerte) itemAlerte->setForeground(QColor("#c62828"));
        else itemAlerte->setForeground(QColor("#2e7d32"));
        itemAlerte->setFont(fBold);
        ui->tableTimeline->setItem(i, 7, itemAlerte);

        ui->tableTimeline->setRowHeight(i, 38);
    }

    ui->tableTimeline->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::rafraichirListeEtapes() {
    QSqlQueryModel *model = tmpEtape.afficher();
    remplirTableEtapes(model);
    delete model;
}



void MainWindow::preparerFormulairePlanif(bool estModif) {
    modeModification = estModif;

    ui->cb_produit->clear();
    QSqlQuery qProd("SELECT ID_PRODUIT, DESIGNATION FROM PRODUITS");
    while(qProd.next()) {
        ui->cb_produit->addItem(qProd.value("DESIGNATION").toString(), qProd.value("ID_PRODUIT"));
    }

    ui->cb_matiere->clear();
    QSqlQuery qMat("SELECT ID_STOCK_MP, CODE_MP FROM MATIERES_PREMIERES");
    while(qMat.next()) {
        ui->cb_matiere->addItem(qMat.value("CODE_MP").toString(), qMat.value("ID_STOCK_MP"));
    }

    ui->cb_employe->clear();
    QSqlQuery qEmp("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES");
    while(qEmp.next()) {
        QString nomComplet = qEmp.value("NOM").toString() + " " + qEmp.value("PRENOM").toString();
        ui->cb_employe->addItem(nomComplet, qEmp.value("ID_EMPLOYE"));
    }

    if(estModif && indexModification >= 0 && indexModification < mesCommandes.size()) {
        CommandeInfo c = mesCommandes[indexModification];
        int idxProd = ui->cb_produit->findText(c.idProduit, Qt::MatchContains);
        if(idxProd >= 0) ui->cb_produit->setCurrentIndex(idxProd);
        int idxMat = ui->cb_matiere->findText(c.idMatiere, Qt::MatchContains);
        if(idxMat >= 0) ui->cb_matiere->setCurrentIndex(idxMat);
        int idxEmp = ui->cb_employe->findText(c.idEmploye, Qt::MatchContains);
        if(idxEmp >= 0) ui->cb_employe->setCurrentIndex(idxEmp);
        ui->sb_qte->setValue(c.quantite);
        ui->dt_lancement->setDate(c.dateDebut);
        ui->le_fin_prevue->setText(c.dateFinEstimee);
        ui->btn_valider_planif->setText("Mettre à jour");
    } else {
        ui->sb_qte->setValue(50);
        ui->le_fin_prevue->clear();
        ui->dt_lancement->setDate(QDate::currentDate());
        ui->btn_valider_planif->setText("Créer Ordre");
    }
}

void MainWindow::preparerFormulaireProduit(bool estModif, int idx) {
    remplirCombosProduitClientEmplacement();

    if(estModif && idx >= 0 && idx < ui->tableProduits->rowCount()) {
        indexModifProd = idx;
        rowToEdit = idx;

        bool okId = false;
        QTableWidgetItem *itId = ui->tableProduits->item(idx, 0);
        selectedProdId = itId ? itId->text().toInt(&okId) : 0;
        if(!okId) selectedProdId = -1;

        auto itDes  = ui->tableProduits->item(idx, 1);
        auto itCout = ui->tableProduits->item(idx, 2);
        auto itColl = ui->tableProduits->item(idx, 3);
        auto itCu  = ui->tableProduits->item(idx, 4);
        auto itTemp = ui->tableProduits->item(idx, 5);

        ui->le_prod_nom_modif->setText(itDes ? itDes->text() : QString());
        ui->sb_prod_cout_modif->setValue(itCout ? itCout->text().toDouble() : 0.0);
        ui->cb_prod_coll_modif->setCurrentText(itColl ? itColl->text() : QString());
        ui->cb_prod_cuir_modif->setCurrentText(itCu ? itCu->text() : QString());
        ui->sb_prod_temps_modif->setValue(itTemp ? itTemp->text().toInt() : 1);

        int idCli = 0;
        int idEmp = 0;
        const QString sid = itId ? itId->text() : QString();
        for (const ProduitInfo &p : mesProduits) {
            if (p.id_produit == sid) {
                idCli = p.idClient.toInt();
                idEmp = p.idEmplacement.toInt();
                break;
            }
        }
        reglerComboParIdDonnee(ui->cb_prod_client_modif, idCli);
        reglerComboParIdDonnee(ui->cb_prod_empl_modif, idEmp);

        ui->tabWidgetProduits->setCurrentIndex(2);
    } else {
        selectedProdId = -1;
        rowToEdit = -1;

        ui->le_prod_nom->clear();
        ui->sb_prod_cout->setValue(0);
        ui->sb_prod_temps->setValue(1);
        reglerComboParIdDonnee(ui->cb_prod_client, 0);
        reglerComboParIdDonnee(ui->cb_prod_empl, 0);

        ui->tabWidgetProduits->setCurrentIndex(1);
    }
}



void MainWindow::preparerFormulaireStock(bool estModif, int idx) {
    if(estModif && idx >= 0 && idx < mesMatieres.size()) {
        indexModifStock = idx;
        const auto &m = mesMatieres[idx];

        ui->le_stock_code_modif->setText(m.code);
        ui->cb_stock_cat_modif->setCurrentText(m.categorie);
        ui->le_stock_lot_modif->setText(m.numLot);
        ui->cb_stock_etat_modif->setCurrentText(m.etat);
        ui->le_stock_coul_modif->setText(m.couleur);
        ui->sb_stock_qte_modif->setValue(m.quantite);
        ui->cb_stock_type_modif->setCurrentText(m.typeStockage);
        ui->cb_stock_qual_modif->setCurrentText(m.qualite);

        ui->tabWidgetStock->setCurrentIndex(2);
    } else {
        ui->le_stock_code->clear();
        ui->le_stock_lot->clear();
        ui->le_stock_coul->clear();
        ui->sb_stock_qte->setValue(0);

        ui->tabWidgetStock->setCurrentIndex(1);
    }
}

void MainWindow::ouvrirDialogueClient(bool estModif) {
    QDialog d(this); d.setWindowTitle("Fiche Client"); d.setMinimumWidth(450); d.setStyleSheet(stylePopup());
    QVBoxLayout *l = new QVBoxLayout(&d);

    QLabel *titre = new QLabel(estModif ? "MODIFIER CLIENT" : "NOUVEAU CLIENT");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    QFormLayout *f = new QFormLayout(); f->setSpacing(15);
    QLineEdit *leId = new QLineEdit(); QLineEdit *leNom = new QLineEdit();
    QLineEdit *leTel = new QLineEdit(); QLineEdit *leAdr = new QLineEdit();
    QLineEdit *leMail = new QLineEdit(); QSpinBox *sbPts = new QSpinBox(); sbPts->setMaximum(100000);

    f->addRow("Identifiant :", leId); f->addRow("Nom Complet :", leNom); f->addRow("Téléphone :", leTel);
    f->addRow("Adresse :", leAdr); f->addRow("Email :", leMail); f->addRow("Points Fidélité :", sbPts);
    l->addLayout(f);

    if(estModif && indexModifClient >= 0 && indexModifClient < mesClients.size()) {
        const auto &c = mesClients[indexModifClient];
        leId->setText(c.id); leNom->setText(c.nom); leTel->setText(c.telephone);
        leAdr->setText(c.adresse); leMail->setText(c.email); sbPts->setValue(c.pointsFidelite);
    }

    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton("Sauvegarder"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave); l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, [this, estModif, &d, leId, leNom, leTel, leAdr, leMail, sbPts]() {
        Connexion *cnx = Connexion::getInstance();
        if (!cnx || !cnx->estConnecte()) {
            alerteErreur(QStringLiteral("Base de données"), QStringLiteral("Connexion Oracle requise."));
            return;
        }
        const QString nom = leNom->text();
        const QString tel = leTel->text().trimmed();
        const QString adr = leAdr->text().trimmed();
        const QString mail = leMail->text().trimmed();
        const int pts = sbPts->value();

        const QString errSaisie = messageValidationSaisieClient(nom, tel, adr, mail, pts);
        if (!errSaisie.isEmpty()) {
            alerteWarning(QStringLiteral("Validation"), errSaisie);
            return;
        }
        if (estModif) {
            if (indexModifClient < 0 || indexModifClient >= mesClients.size()) {
                alerteWarning(QStringLiteral("Sélection"), QStringLiteral("Aucun client valide à modifier."));
                return;
            }
            bool ok = false;
            const int idMod = mesClients[indexModifClient].id.toInt(&ok);
            if (!ok || idMod <= 0) {
                alerteWarning(QStringLiteral("Identifiant"), QStringLiteral("Identifiant client invalide."));
                return;
            }
            Client cl(
                idMod,
                nom.trimmed(),
                tel,
                adr,
                mail,
                pts);
            if (!cl.modifier(idMod)) {
                const QString err = cl.derniereErreurSaisie();
                alerteErreur(QStringLiteral("Modification"),
                             err.isEmpty() ? QStringLiteral("Échec de modification.") : err);
                return;
            }
        } else {
            const QString errId = messageValidationIdClientAjoutOptionnel(leId->text());
            if (!errId.isEmpty()) {
                alerteWarning(QStringLiteral("Validation"), errId);
                return;
            }
            bool okId = false;
            const int idSaisi = leId->text().trimmed().toInt(&okId);
            const int id = okId ? idSaisi : 0;
            Client cl(
                id,
                nom.trimmed(),
                tel,
                adr,
                mail,
                pts);
            if (!cl.ajouter()) {
                const QString err = cl.derniereErreurSaisie();
                alerteErreur(QStringLiteral("Validation/BDD"),
                             err.isEmpty() ? QStringLiteral("Insertion impossible.") : err);
                return;
            }
        }
        alerteSucces(QStringLiteral("Client"), QStringLiteral("Client enregistré en base de données."));
        rafraichirListeClients();
        remplirCombosProduitClientEmplacement();
        d.accept();
    });
    d.exec();
}




QFrame* MainWindow::creerCarteStat(QString icone, QString val, QString titre, QString couleurFond) {
    QFrame *f = new QFrame();
    f->setStyleSheet(QString("QFrame { background: %1; border-radius: 10px; }").arg(couleurFond));
    f->setMinimumHeight(100);
    QVBoxLayout *l = new QVBoxLayout(f);

    QLabel *l_ico = new QLabel(icone); l_ico->setStyleSheet("font-size: 24px; border:none; color: white;");
    QLabel *l_val = new QLabel(val); l_val->setStyleSheet("font-size: 28px; font-weight:900; color: white; border:none;");
    QLabel *l_tit = new QLabel(titre); l_tit->setStyleSheet("font-size: 13px; font-weight:bold; color: #f0f0f0; border:none;");

    l_ico->setAlignment(Qt::AlignRight); l_val->setAlignment(Qt::AlignCenter); l_tit->setAlignment(Qt::AlignCenter);

    l->addWidget(l_ico); l->addWidget(l_val); l->addWidget(l_tit);
    return f;
}

void MainWindow::ouvrirStatsProduits() {
    if(ui->tabWidgetProduits->count() < 4) return;
    QWidget *ongletStats = ui->tabWidgetProduits->widget(3);
    if (!ongletStats) return;

    if (ongletStats->layout()) { clearLayout(ongletStats->layout()); delete ongletStats->layout(); }
    QVBoxLayout *mainL = new QVBoxLayout(ongletStats);
    mainL->setSpacing(20);
    mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - CATALOGUE PRODUITS");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter);
    mainL->addWidget(t);

    int total = mesProduits.size();
    double prixTotal = 0;
    QMap<QString, double> parCollection;
    QMap<QString, double> parCuir;

    for(const auto &p : mesProduits) {
        prixTotal += p.cout;
        parCollection[p.collection] += 1;
        parCuir[p.typeCuir] += 1;
    }
    double moy = (total > 0) ? prixTotal / total : 0;

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->setSpacing(15);

    kpiL->addWidget(creerCarteStat("👜", QString::number(total), "Références Actives", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)"));

    kpiL->addWidget(creerCarteStat("💰", QString::number(moy, 'f', 1) + " DT", "Coût Moyen de Fab.", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));

    QString collDominante = "Aucune";
    if(!parCollection.isEmpty()) {
        auto it = std::max_element(parCollection.begin(), parCollection.end());
        collDominante = it.key();
    }
    kpiL->addWidget(creerCarteStat("✨", collDominante, "Collection Phare", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #5d4037, stop:1 #d4af37)"));

    mainL->addLayout(kpiL);

    QHBoxLayout *chartsL = new QHBoxLayout();

    QFrame *framePie = new QFrame();
    framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Collection");
    titrePie->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;");
    layoutPie->addWidget(titrePie);

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slicesColl;
    for(auto k : parCollection.keys()) slicesColl.append({k, parCollection[k]});
    setPieChart(wPie, "", slicesColl);
    layoutPie->addWidget(wPie);
    chartsL->addWidget(framePie);

    QFrame *frameBar = new QFrame();
    frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Utilisation des Cuirs");
    titreBar->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;");
    layoutBar->addWidget(titreBar);

    QWidget *wBar = new QWidget();
    QStringList cats = parCuir.keys();
    QList<double> vals; for(auto k : cats) vals << parCuir[k];
    setVerticalBarChart(wBar, "", cats, vals);
    layoutBar->addWidget(wBar);
    chartsL->addWidget(frameBar);

    mainL->addLayout(chartsL);
    mainL->addStretch();

    ui->tabWidgetProduits->setCurrentIndex(3);
}



void MainWindow::ouvrirStatsStock() {
    if(ui->tabWidgetStock->count() < 4) return;
    QWidget *onglet = ui->tabWidgetStock->widget(3);
    if (!onglet) return;
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(20); mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - MATIÈRES PREMIÈRES");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    double volume = 0; QMap<QString, double> parCat; QMap<QString, double> parQual;
    for(const auto &m : mesMatieres) { volume += m.quantite; parCat[m.categorie] += 1; parQual[m.qualite] += 1; }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("📦", QString::number(mesMatieres.size()), "Lots Référencés", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)"));
    kpiL->addWidget(creerCarteStat("📏", QString::number(volume) + " Unités", "Volume Total Actuel", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3e2723, stop:1 #8d5524)"));
    mainL->addLayout(kpiL);

    QHBoxLayout *chartsL = new QHBoxLayout();

    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Catégorie"); titrePie->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;"); layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slices;
    for(auto k : parCat.keys()) slices.append({k, parCat[k]});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie); chartsL->addWidget(framePie);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Qualité des lots"); titreBar->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;"); layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget(); QStringList cats = parQual.keys();
    QList<double> vals; for(auto k : cats) vals << parQual[k];
    setVerticalBarChart(wBar, "", cats, vals); layoutBar->addWidget(wBar); chartsL->addWidget(frameBar);

    mainL->addLayout(chartsL); mainL->addStretch();
    ui->tabWidgetStock->setCurrentIndex(3);
}


void MainWindow::ouvrirStatsClients() {
    if(ui->tabWidgetClients->count() < 4) return;
    QWidget *onglet = ui->tabWidgetClients->widget(3);
    if (!onglet) return;

    if (onglet->layout()) {
        QLayout *oldLayout = onglet->layout();
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(20);
    mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - INTELLIGENCE CLIENTS");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter);
    mainL->addWidget(t);

    int vip = 0; QMap<QString, double> parVille;
    for(const auto &c : mesClients) {
        if(c.pointsFidelite >= 100) vip++;
        QString ville = "Autre";
        if(c.adresse.toLower().contains("tunis")) ville = "Tunis";
        else if(c.adresse.toLower().contains("sfax")) ville = "Sfax";
        parVille[ville] += 1;
    }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("🧾", QString::number(mesClients.size()), "Clients Inscrits", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));
    kpiL->addWidget(creerCarteStat("👑", QString::number(vip), "Clients VIP (>100pts)", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #f7f1e3)"));
    mainL->addLayout(kpiL);

    QFrame *framePie = new QFrame();
    framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition Géographique");
    titrePie->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;");
    layoutPie->addWidget(titrePie);

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for(auto k : parVille.keys()) slices.append({k, parVille[k]});
    setPieChart(wPie, "", slices);
    layoutPie->addWidget(wPie);

    mainL->addWidget(framePie);
    mainL->addStretch();

    onglet->setLayout(mainL);
    ui->tabWidgetClients->setCurrentIndex(3);
}

void MainWindow::showClientFideliteTab() {
    if(ui->tabWidgetClients->count() < 5) return;
    QWidget *onglet = ui->tabWidgetClients->widget(4);
    if (!onglet) return;

    if (onglet->layout()) {
        QLayout *oldLayout = onglet->layout();
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("⭐ GESTION PROGRAMME FIDÉLITÉ");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #f1c40f; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    int idx = -1;
    const int row = ui->tableClients->currentRow();
    if (row >= 0) {
        if (QTableWidgetItem *itId = ui->tableClients->item(row, 0)) {
            bool ok = false;
            const int idDb = itId->text().toInt(&ok);
            if (ok)
                idx = indexMesClientParIdDb(mesClients, idDb);
        }
    }
    QLabel *desc = new QLabel();
    if (idx >= 0 && idx < mesClients.size()) {
        ClientInfo c = mesClients[idx];
        QString niveau = (c.pointsFidelite >= 200) ? "💎 PLATINE" : (c.pointsFidelite >= 100) ? "🥇 GOLD" : "🥈 SILVER";
        QString color = (c.pointsFidelite >= 200) ? "#9b59b6" : (c.pointsFidelite >= 100) ? "#d4af37" : "#7f8c8d";

        desc->setText(QString(
            "<div style='background:white; border-radius:12px; padding:30px; border:2px solid %1; color:#3e2723; font-size:16px;'>"
            "<h2 style='color:%1; margin-top:0; text-align:center;'>Statut de %2</h2><hr>"
            "Points actuels : <b style='font-size:20px;'>%3 pts</b><br><br>"
            "Niveau débloqué : <b style='color:%1; font-size:22px;'>%4</b><br><br><hr>"
            "<i>Avantages : 10% de réduction sur la prochaine commande et livraison gratuite.</i>"
            "</div>").arg(color, c.nom).arg(c.pointsFidelite).arg(niveau));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Sélectionnez un client dans la liste pour voir son statut de fidélité.</div>");
    }
    desc->setAlignment(Qt::AlignCenter);
    l->addWidget(desc, 0, Qt::AlignCenter);

    l->addStretch();

    onglet->setLayout(l);
    ui->tabWidgetClients->setCurrentIndex(4);
}

void MainWindow::showClientIaTab() {
    if(ui->tabWidgetClients->count() < 6) return;
    QWidget *onglet = ui->tabWidgetClients->widget(5);
    if (!onglet) return;

    if (onglet->layout()) {
        QLayout *oldLayout = onglet->layout();
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("🧠 IA : PRÉVISION D'ACHAT & CHURN");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #e74c3c; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    int idx = -1;
    const int rowIa = ui->tableClients->currentRow();
    if (rowIa >= 0) {
        if (QTableWidgetItem *itId = ui->tableClients->item(rowIa, 0)) {
            bool ok = false;
            const int idDb = itId->text().toInt(&ok);
            if (ok)
                idx = indexMesClientParIdDb(mesClients, idDb);
        }
    }
    QLabel *desc = new QLabel();
    if (idx >= 0 && idx < mesClients.size()) {
        ClientInfo c = mesClients[idx];
        desc->setText(QString(
            "<div style='background:white; border-radius:12px; padding:30px; border:3px dashed #e74c3c; color:#3e2723; font-size:15px;'>"
            "<h2 style='color:#e74c3c; margin-top:0; text-align:center;'>Analyse du comportement : %1</h2><hr>"
            "<ul>"
            "<li><b>Probabilité d'achat imminent :</b> <span style='color:#27ae60; font-weight:bold;'>ÉLEVÉE (85%)</span></li><br>"
            "<li><b>Produit suggéré :</b> Sac Voyage Cuir Hiver 2026.</li><br>"
            "<li><b>Action Marketing recommandée :</b> Envoyer un email ciblé avec code promo (Inactif depuis > 3 mois).</li>"
            "</ul></div>").arg(c.nom));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Sélectionnez un client dans la liste pour lancer l'analyse IA.</div>");
    }
    desc->setAlignment(Qt::AlignCenter);
    l->addWidget(desc, 0, Qt::AlignCenter);

    l->addStretch();

    onglet->setLayout(l);
    ui->tabWidgetClients->setCurrentIndex(5);
}


void MainWindow::ouvrirStatsPlanification() {
    QDialog d(this);
    d.setWindowTitle("Analyses Planification");
    d.setMinimumSize(850, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("DASHBOARD : PLANIFICATION & PRODUCTION");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    int totalCmd = 0;
    int totalQte = 0;
    int cmdRetard = 0;
    QMap<QString, double> parStatut;
    QMap<QString, double> parProduit;

    QSqlQuery q;
    if(q.exec("SELECT COUNT(*), NVL(SUM(QUANTITE), 0) FROM PLANIFICATION") && q.next()) {
        totalCmd = q.value(0).toInt();
        totalQte = q.value(1).toInt();
    }
    if(q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next()) {
        cmdRetard = q.value(0).toInt();
    }
    if(q.exec("SELECT STATUT, COUNT(*) FROM PLANIFICATION GROUP BY STATUT")) {
        while(q.next()) { parStatut[q.value(0).toString()] = q.value(1).toDouble(); }
    }
    if(q.exec("SELECT pr.DESIGNATION, COUNT(p.ID_COMMANDE) FROM PLANIFICATION p JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT GROUP BY pr.DESIGNATION")) {
        while(q.next()) { parProduit[q.value(0).toString()] = q.value(1).toDouble(); }
    }

    double tauxRetard = (totalCmd > 0) ? (static_cast<double>(cmdRetard) * 100.0 / totalCmd) : 0;

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("📋", QString::number(totalCmd), "Ordres de Fabrication", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));
    kpiL->addWidget(creerCarteStat("📦", QString::number(totalQte), "Pièces Produites", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #8d5524)"));

    QString colorRetard = (tauxRetard > 20) ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EB3349, stop:1 #F45C43)" : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)";
    kpiL->addWidget(creerCarteStat("⚠️", QString::number(tauxRetard, 'f', 1) + "%", "Taux de Retard", colorRetard));
    mainL->addLayout(kpiL);

    QHBoxLayout *chartsL = new QHBoxLayout();

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for(auto k : parStatut.keys()) slices.append({k, parStatut[k]});
    setPieChart(wPie, "Répartition par Statut", slices);

    QWidget *wBar = new QWidget();
    QStringList prods = parProduit.keys();
    QList<double> vals; for(auto k : prods) vals << parProduit[k];
    setVerticalBarChart(wBar, "Commandes par Produit", prods, vals);

    chartsL->addWidget(wPie); chartsL->addWidget(wBar);
    mainL->addLayout(chartsL);

    QPushButton *btn = new QPushButton("Fermer"); btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept); mainL->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

void MainWindow::exporterFactureClient()
{
    const int r = ui->tableClients->currentRow();
    if(r < 0) {
        alerteSucces("Facture", "Sélectionnez un client dans la liste pour générer sa facture.");
        return;
    }

    QTableWidgetItem *it = ui->tableClients->item(r, 0);
    if (!it)
        return;
    bool ok = false;
    const int idDb = it->text().toInt(&ok);
    if (!ok || idDb <= 0)
        return;
    const int idx = indexMesClientParIdDb(mesClients, idDb);
    if (idx < 0 || idx >= mesClients.size())
        return;
    const ClientInfo &c = mesClients[idx];

    QString f = QFileDialog::getSaveFileName(this, "Exporter Facture", "Facture_" + c.id + ".pdf", "PDF (*.pdf)");
    if(f.isEmpty()) return;
    if(QFileInfo(f).suffix().isEmpty()) f += ".pdf";

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(f);

    const QString dateStr = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    const QString niveau = (c.pointsFidelite >= 200) ? "Platine" : (c.pointsFidelite >= 100) ? "Gold" : (c.pointsFidelite >= 50) ? "Silver" : "Bronze";

    const QString css =
        "<style>"
        "  @page { margin: 16mm 12mm 16mm 12mm; }"
        "  body { font-family: 'Segoe UI', Arial, sans-serif; color: #3e2723; }"
        "  .header { padding: 10px 12px; border: 1px solid #d7ccc8; border-radius: 10px; background: #f3f0eb; }"
        "  .brand { font-size: 13px; letter-spacing: 2px; font-weight: 800; color: #2c1a16; text-transform: uppercase; }"
        "  .meta { font-size: 10px; color: #795548; }"
        "  .title { margin: 14px 0 8px 0; font-size: 22px; font-weight: 900; color: #1a1210; }"
        "  .accent { height: 4px; width: 100%; background: #d4af37; border-radius: 3px; margin: 4px 0 14px 0; }"
        "  .card { border: 1px solid #d7ccc8; border-radius: 10px; padding: 12px; background: #ffffff; margin-bottom: 10px; }"
        "  .k { color: #795548; font-size: 10px; text-transform: uppercase; letter-spacing: 1px; }"
        "  .v { font-size: 12px; font-weight: 700; }"
        "  table.data { width: 100%; border-collapse: collapse; border: 1px solid #d7ccc8; border-radius: 8px; overflow: hidden; margin-top: 10px; }"
        "  table.data th { background: #2c1a16; color: #e0c097; text-transform: uppercase; font-size: 10px; letter-spacing: 1px; padding: 10px 8px; }"
        "  table.data td { padding: 9px 8px; border-bottom: 1px solid #eee5dd; font-size: 10.5px; }"
        "  .note { margin-top: 12px; font-size: 9.5px; color: #795548; }"
        "</style>";

    const QString html =
        "<html><head>" + css + "</head><body>"
                               "  <div class='header'>"
                               "    <div class='brand'>FIL D'OR</div>"
                               "    <div class='meta'><b>Facture :</b> " + ("FACT-" + c.id).toHtmlEscaped() + " &nbsp; | &nbsp; <b>Date :</b> " + dateStr.toHtmlEscaped() + "</div>"
                                                                                                           "  </div>"
                                                                                                           "  <div class='title'>FACTURE</div>"
                                                                                                           "  <div class='accent'></div>"
                                                                                                           "  <div class='card'>"
                                                                                                           "    <div class='k'>Client</div>"
                                                                                                           "    <div class='v'>" + c.nom.toHtmlEscaped() + " (" + c.id.toHtmlEscaped() + ")</div>"
                                                                "    <div style='margin-top:6px; font-size:11px;'>"
                                                                "      <b>Téléphone :</b> " + c.telephone.toHtmlEscaped() + "<br/>"
                                        "      <b>Email :</b> " + c.email.toHtmlEscaped() + "<br/>"
                                    "      <b>Adresse :</b> " + c.adresse.toHtmlEscaped() +
        "    </div>"
        "  </div>"
        "  <div class='card'>"
        "    <div class='k'>Fidélité</div>"
        "    <div class='v'>" + niveau.toHtmlEscaped() + " — " + QString::number(c.pointsFidelite).toHtmlEscaped() + " points</div>"
                                                                                               "  </div>"
                                                                                               "  <table class='data'>"
                                                                                               "    <thead><tr><th>Désignation</th><th>Qté</th><th>Prix</th><th>Total</th></tr></thead>"
                                                                                               "    <tbody>"
                                                                                               "      <tr><td>—</td><td>—</td><td>—</td><td>—</td></tr>"
                                                                                               "    </tbody>"
                                                                                               "  </table>"
                                                                                               "  <div class='note'><i>NB: Facture démonstrative (lignes d’articles non connectées à une vente).</i></div>"
                                                                                               "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    alerteSucces("Succès", "Facture exportée en PDF.");
}

void MainWindow::showPlanifIaDialog() {
    preparerFormulairePlanif(false);
}

namespace {

static QString formatCostIntelligenceMcBanner(const QJsonObject &root)
{
    const QJsonObject k = root.value(QStringLiteral("kpis")).toObject();
    if (k.isEmpty())
        return {};
    const QJsonObject ra = root.value(QStringLiteral("riskAnalysis")).toObject();
    const QJsonObject best = root.value(QStringLiteral("bestConfiguration")).toObject();
    const QJsonObject flags = k.value(QStringLiteral("decisionFlags")).toObject();
    QString riskLine = QStringLiteral("—");
    if (flags.value(QStringLiteral("highRisk")).toBool())
        riskLine = QStringLiteral("<b style=\"color:#b71c1c;\">HIGH RISK</b> (score &gt; 70)");
    else if (flags.value(QStringLiteral("notViableMargin")).toBool())
        riskLine = QStringLiteral("<b style=\"color:#e65100;\">Marge sous seuil</b> — non viable au prix cible");
    else
        riskLine = QStringLiteral("Profil sous contrôle opérationnel");

    return QStringLiteral(
               "<div style=\"font-family:'Segoe UI',sans-serif;margin-bottom:14px;padding:14px;border-radius:10px;"
               "border:1px solid #d7ccc8;background:linear-gradient(135deg,#fff8e1 0%,#f5f5f5 100%);\">"
               "<div style=\"font-size:11px;text-transform:uppercase;letter-spacing:1px;color:#6d4c41;\">"
               "Monte Carlo &amp; décision</div>"
               "<table style=\"width:100%;margin-top:8px;font-size:13px;color:#3e2723;\">"
               "<tr><td><b>Tirages MC</b></td><td align=\"right\">%1</td></tr>"
               "<tr><td><b>Coût moy. simulé</b></td><td align=\"right\">%2 TND</td></tr>"
               "<tr><td><b>Enveloppe min / max</b></td><td align=\"right\">%3 — %4 TND</td></tr>"
               "<tr><td><b>Score risque (0–100)</b></td><td align=\"right\">%5 (%6)</td></tr>"
               "<tr><td><b>Prix min. viable</b></td><td align=\"right\">%7 TND</td></tr>"
               "<tr><td><b>Prix reco.</b></td><td align=\"right\">%8 TND</td></tr>"
               "<tr><td><b>Marge attendue</b></td><td align=\"right\">%9 %</td></tr>"
               "<tr><td colspan=\"2\" style=\"padding-top:8px;\">%10</td></tr>"
               "<tr><td colspan=\"2\" style=\"padding-top:6px;font-size:12px;color:#5d4037;\">"
               "<b>Meilleure config.</b> %11 — CRv %12 TND — viabilité %13/100. %14</td></tr>"
               "</table></div>")
        .arg(k.value(QStringLiteral("monteCarloRuns")).toInt())
        .arg(QLocale::system().toString(k.value(QStringLiteral("avgCost")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(k.value(QStringLiteral("bestCaseCost")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(k.value(QStringLiteral("worstCaseCost")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(ra.value(QStringLiteral("riskScore")).toDouble(), 'f', 1))
        .arg(ra.value(QStringLiteral("level")).toString().toHtmlEscaped())
        .arg(QLocale::system().toString(k.value(QStringLiteral("minimumViablePrice")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(k.value(QStringLiteral("recommendedSellingPrice")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(k.value(QStringLiteral("expectedMarginPct")).toDouble(), 'f', 1))
        .arg(riskLine)
        .arg(best.value(QStringLiteral("label")).toString().toHtmlEscaped())
        .arg(QLocale::system().toString(best.value(QStringLiteral("balancedCostPerUnit")).toDouble(), 'f', 2))
        .arg(QLocale::system().toString(best.value(QStringLiteral("productViabilityScore")).toDouble(), 'f', 0))
        .arg(best.value(QStringLiteral("rationale_fr")).toString().toHtmlEscaped());
}

static void fillAdvancedCostKpi(MainWindow *mw, QWidget *host, const QJsonObject &root)
{
    if (!mw || !host)
        return;
    if (QLayout *ol = host->layout()) {
        clearLayout(ol);
        delete ol;
    }
    auto *h = new QHBoxLayout(host);
    h->setSpacing(12);
    const QJsonObject agg = root.value(QStringLiteral("aggregates")).toObject();
    const QJsonObject scores = root.value(QStringLiteral("scores")).toObject();
    const QJsonArray scen = root.value(QStringLiteral("scenarios")).toArray();
    double realistic = agg.value(QStringLiteral("realistic_cost")).toDouble();
    double optimistic = agg.value(QStringLiteral("optimistic_cost")).toDouble();
    double pessimistic = agg.value(QStringLiteral("pessimistic_cost")).toDouble();
    double conf = agg.value(QStringLiteral("cost_confidence_score")).toDouble();
    double price = 0.0;
    for (const QJsonValue &v : scen) {
        const QJsonObject o = v.toObject();
        if (o.value(QStringLiteral("code")).toString() == QLatin1String("balanced")) {
            price = o.value(QStringLiteral("recommended_selling_price")).toDouble();
            break;
        }
    }
    const QString g1 = QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3e2723, stop:1 #5d4037)");
    const QString g2 = QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)");
    const QString g3 = QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #8d5524)");
    const QString g4 = QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)");
    h->addWidget(mw->creerCarteStat(QStringLiteral("📉"), QLocale::system().toString(optimistic, 'f', 2) + QStringLiteral(" TND"),
                                     QStringLiteral("Coût optimiste"), g2), 1);
    h->addWidget(mw->creerCarteStat(QStringLiteral("⚖️"), QLocale::system().toString(realistic, 'f', 2) + QStringLiteral(" TND"),
                                     QStringLiteral("CRv réaliste"), g1), 1);
    h->addWidget(mw->creerCarteStat(QStringLiteral("📈"), QLocale::system().toString(pessimistic, 'f', 2) + QStringLiteral(" TND"),
                                     QStringLiteral("Coût pessimiste"), QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c62828, stop:1 #ff8a65)")), 1);
    h->addWidget(mw->creerCarteStat(QStringLiteral("🏷️"), QLocale::system().toString(price, 'f', 2) + QStringLiteral(" TND"),
                                     QStringLiteral("Prix public indicatif"), g3), 1);
    h->addWidget(mw->creerCarteStat(QStringLiteral("🎯"), QStringLiteral("%1 / 100").arg(QLocale::system().toString(conf, 'f', 0)),
                                     QStringLiteral("Confiance forecast"), g4), 1);
    const double viab = scores.value(QStringLiteral("product_viability")).toDouble();
    h->addWidget(mw->creerCarteStat(QStringLiteral("✨"), QStringLiteral("%1 / 100").arg(QLocale::system().toString(viab, 'f', 0)),
                                     QStringLiteral("Viabilité produit"), QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)")), 1);
}

static void fillAdvancedCostKpiPlaceholder(MainWindow *mw, QWidget *host)
{
    if (!mw || !host)
        return;
    if (QLayout *ol = host->layout()) {
        clearLayout(ol);
        delete ol;
    }
    auto *h = new QHBoxLayout(host);
    h->setSpacing(10);
    const QString gm = QStringLiteral("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #eceff1, stop:1 #cfd8dc)");
    const QStringList titles = {QStringLiteral("Coût optimiste"), QStringLiteral("CRv réaliste"), QStringLiteral("Coût pessimiste"),
                                QStringLiteral("Prix indicatif"), QStringLiteral("Confiance forecast"), QStringLiteral("Viabilité produit")};
    const QStringList icons = {QStringLiteral("📉"), QStringLiteral("⚖️"), QStringLiteral("📈"),
                               QStringLiteral("🏷️"), QStringLiteral("🎯"), QStringLiteral("✨")};
    for (int i = 0; i < 6; ++i) {
        h->addWidget(mw->creerCarteStat(icons.at(i), QStringLiteral("—"), titles.at(i), gm), 1);
    }
}

static QString buildCostSimEmptyStateHtml()
{
    return QStringLiteral(
        "<div style=\"font-family:'Segoe UI',sans-serif;color:#4e342e;padding:20px 24px;\">"
        "<div style=\"font-size:15px;font-weight:700;margin-bottom:8px;\">Aucune analyse en cours</div>"
        "<p style=\"line-height:1.6;color:#6d4c41;margin:0 0 14px 0;\">"
        "Renseignez les <b>cinq champs</b> à gauche, puis lancez une "
        "<b>simulation</b>, une <b>optimisation</b> ou une passe <b>Monte Carlo</b> (intelligence). "
        "Les scénarios éco / équilibré / premium, les risques et les recommandations s’affichent ici — "
        "sans aucune écriture en base.</p>"
        "<div style=\"font-size:12px;color:#8d6e63;\">Backend : <code>%1</code> · "
        "Synthèse IA si <code>FASHION_ORACLE_API_KEY</code> est définie côté serveur.</div></div>")
        .arg(fashionOracleBaseUrl().toHtmlEscaped());
}

static QString buildAdvancedCostCockpitHtml(const QJsonObject &root, const QString &optimizeBanner = QString())
{
    QString html;
    html += QStringLiteral("<div style=\"font-family:'Segoe UI',sans-serif;color:#3e2723;font-size:14px;\">");
    if (!optimizeBanner.isEmpty()) {
        html += QStringLiteral("<div style=\"background:#e8f5e9;border:1px solid #66bb6a;padding:10px;border-radius:8px;margin-bottom:12px;\">%1</div>")
                    .arg(optimizeBanner.toHtmlEscaped());
    }
    const QString summary = root.value(QStringLiteral("executive_summary_fr")).toString();
    html += QStringLiteral("<div style=\"background:#fff8e1;border-left:4px solid #d4af37;padding:14px;margin-bottom:14px;border-radius:6px;\">"
                           "<div style=\"font-size:11px;text-transform:uppercase;letter-spacing:1px;color:#6d4c41;\">Synthèse exécutive</div>"
                           "<p style=\"margin:8px 0 0 0;line-height:1.55;\">%1</p></div>")
                .arg(summary.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")));

    const QJsonObject scores = root.value(QStringLiteral("scores")).toObject();
    html += QStringLiteral("<div style=\"display:flex;flex-wrap:wrap;gap:10px;margin-bottom:14px;\">"
                           "<div style=\"flex:1;min-width:140px;background:#fafafa;border:1px solid #e0e0e0;border-radius:8px;padding:10px;\">"
                           "<b>Efficacité coût</b><br/><span style=\"font-size:22px;color:#2e7d32;\">%1</span><span style=\"font-size:12px;\"> /100</span></div>"
                           "<div style=\"flex:1;min-width:140px;background:#fafafa;border:1px solid #e0e0e0;border-radius:8px;padding:10px;\">"
                           "<b>Exposition risque</b><br/><span style=\"font-size:22px;color:#c62828;\">%2</span><span style=\"font-size:12px;\"> /100</span></div>"
                           "<div style=\"flex:1;min-width:140px;background:#fafafa;border:1px solid #e0e0e0;border-radius:8px;padding:10px;\">"
                           "<b>Santé marge</b><br/><span style=\"font-size:22px;color:#1565c0;\">%3</span><span style=\"font-size:12px;\"> /100</span></div>"
                           "<div style=\"flex:1;min-width:140px;background:#fafafa;border:1px solid #e0e0e0;border-radius:8px;padding:10px;\">"
                           "<b>Viabilité</b><br/><span style=\"font-size:22px;color:#5d4037;\">%4</span><span style=\"font-size:12px;\"> /100</span></div></div>")
                .arg(QLocale::system().toString(scores.value(QStringLiteral("cost_efficiency")).toDouble(), 'f', 0),
                     QLocale::system().toString(scores.value(QStringLiteral("risk_exposure")).toDouble(), 'f', 0),
                     QLocale::system().toString(scores.value(QStringLiteral("margin_health")).toDouble(), 'f', 0),
                     QLocale::system().toString(scores.value(QStringLiteral("product_viability")).toDouble(), 'f', 0));

    html += QStringLiteral("<h3 style=\"color:#5d4037;margin:16px 0 8px 0;\">Comparaison des scénarios business</h3><table style=\"width:100%;border-collapse:collapse;\">"
                           "<tr style=\"background:#efebe9;\"><th align=\"left\" style=\"padding:8px;border:1px solid #d7ccc8;\">Scénario</th>"
                           "<th align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">CRv / u</th>"
                           "<th align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">Marge %</th>"
                           "<th align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">Prix reco.</th>"
                           "<th align=\"center\" style=\"padding:8px;border:1px solid #d7ccc8;\">Risque</th></tr>");
    const QJsonArray scen = root.value(QStringLiteral("scenarios")).toArray();
    for (const QJsonValue &v : scen) {
        const QJsonObject o = v.toObject();
        html += QStringLiteral("<tr><td style=\"padding:8px;border:1px solid #d7ccc8;\"><b>%1</b><br/><span style=\"font-size:12px;color:#666;\">%2</span></td>"
                               "<td align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">%3 TND</td>"
                               "<td align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">%4 %</td>"
                               "<td align=\"right\" style=\"padding:8px;border:1px solid #d7ccc8;\">%5 TND</td>"
                               "<td align=\"center\" style=\"padding:8px;border:1px solid #d7ccc8;\">%6</td></tr>")
                    .arg(o.value(QStringLiteral("label_fr")).toString().toHtmlEscaped(),
                         o.value(QStringLiteral("positioning_fr")).toString().toHtmlEscaped(),
                         QLocale::system().toString(o.value(QStringLiteral("total_cost_per_unit")).toDouble(), 'f', 2),
                         QLocale::system().toString(o.value(QStringLiteral("estimated_margin_pct")).toDouble(), 'f', 1),
                         QLocale::system().toString(o.value(QStringLiteral("recommended_selling_price")).toDouble(), 'f', 2),
                         o.value(QStringLiteral("risk_level")).toString().toHtmlEscaped());
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h3 style=\"color:#5d4037;margin:16px 0 8px 0;\">Analyse des risques</h3><ul style=\"margin:0;padding-left:18px;\">");
    const QJsonArray risks = root.value(QStringLiteral("risks")).toArray();
    for (const QJsonValue &v : risks) {
        const QJsonObject r = v.toObject();
        html += QStringLiteral("<li style=\"margin-bottom:6px;\"><b>[%1]</b> %2 — %3</li>")
                    .arg(r.value(QStringLiteral("severity")).toString().toHtmlEscaped(),
                         r.value(QStringLiteral("titre_fr")).toString().toHtmlEscaped(),
                         r.value(QStringLiteral("detail_fr")).toString().toHtmlEscaped());
    }
    html += QStringLiteral("</ul>");

    html += QStringLiteral("<h3 style=\"color:#5d4037;margin:16px 0 8px 0;\">Recommandations d'optimisation</h3><ol style=\"margin:0;padding-left:18px;\">");
    const QJsonArray recs = root.value(QStringLiteral("recommendations")).toArray();
    for (const QJsonValue &v : recs) {
        const QJsonObject r = v.toObject();
        html += QStringLiteral("<li style=\"margin-bottom:8px;\"><b>P%1 — %2</b> (%3)<br/><span style=\"color:#555;\">%4</span></li>")
                    .arg(r.value(QStringLiteral("priority")).toInt())
                    .arg(r.value(QStringLiteral("titre_fr")).toString().toHtmlEscaped(),
                         r.value(QStringLiteral("categorie")).toString().toHtmlEscaped(),
                         r.value(QStringLiteral("impact_fr")).toString().toHtmlEscaped());
    }
    html += QStringLiteral("</ol>");

    html += QStringLiteral("<h3 style=\"color:#5d4037;margin:16px 0 8px 0;\">Détail du coût (scénario équilibré)</h3>");
    QJsonObject bd;
    for (const QJsonValue &v : scen) {
        const QJsonObject o = v.toObject();
        if (o.value(QStringLiteral("code")).toString() == QLatin1String("balanced")) {
            bd = o.value(QStringLiteral("breakdown")).toObject();
            break;
        }
    }
    html += QStringLiteral("<table style=\"width:100%;border-collapse:collapse;\">");
    const QStringList keys = {QStringLiteral("matiere"), QStringLiteral("main_oeuvre"), QStringLiteral("charges_structure"),
                              QStringLiteral("emballage_logistique"), QStringLiteral("risque_volatilite"), QStringLiteral("durabilite_branding")};
    const QMap<QString, QString> labels = {
        {QStringLiteral("matiere"), QStringLiteral("Matière & risque appro")},
        {QStringLiteral("main_oeuvre"), QStringLiteral("Main-d'œuvre & finition")},
        {QStringLiteral("charges_structure"), QStringLiteral("Structure, rebut, volatilité")},
        {QStringLiteral("emballage_logistique"), QStringLiteral("Emballage & logistique")},
        {QStringLiteral("risque_volatilite"), QStringLiteral("Charge risque / volatilité")},
        {QStringLiteral("durabilite_branding"), QStringLiteral("Durabilité & branding")},
    };
    for (const QString &k : keys) {
        const double val = bd.value(k).toDouble();
        html += QStringLiteral("<tr><td style=\"padding:6px;border:1px solid #d7ccc8;\">%1</td>"
                               "<td align=\"right\" style=\"padding:6px;border:1px solid #d7ccc8;\"><b>%2</b> TND</td></tr>")
                    .arg(labels.value(k, k).toHtmlEscaped(), QLocale::system().toString(val, 'f', 2));
    }
    html += QStringLiteral("</table></div>");
    return html;
}

}

namespace {

struct TrendDataLocal {
    QString name;
    double baseScore = 50.0;
    double momentum = 0.0;
    double materialScore = 50.0;
    double silhouetteScore = 50.0;
    std::array<double, 6> conflictWith{};
    std::array<double, 6> historicalScores{};
    QColor color;
};

struct ForecastConceptLocal {
    QString productName;
    QString style;
    QString palette;
    QString material;
    QString silhouette;
    int marketFit = 0;
    int materialFit = 0;
    int silhouetteFit = 0;
    int globalFit = 0;
    QString badge;
};

struct ForecastResultLocal {
    QString dominantTrend;
    double confidence = 0.0;
    QString palette;
    QString material;
    QString silhouette;
    QString moteur;
    std::array<double, 6> trendScores{};
    int momentum = 0;
    int riskScore = 0;
    QString scenario;
    std::array<ForecastConceptLocal, 3> concepts{};
    QStringList topStyles;
    QStringList palettes;
    QStringList materials;
    QStringList silhouettes;
    QStringList conflicts;
    std::array<QColor, 3> paletteColors{};
    QStringList paletteNames;
    QStringList conceptNames;
};

struct TrendCurveLocal {
    const char* name;
    double base2025;
    double momentum;
    double peakYear;
    double peakDrop;
    QColor color;
};

struct TrendProfileLocal {
    const char* name;
    const char* palette;
    const char* matiere;
    const char* silhouette;
    const char* moteur;
    std::array<QColor, 3> paletteColors;
    std::array<const char*, 3> paletteNames;
    std::array<const char*, 3> conceptNames;
    std::array<std::array<const char*, 4>, 3> conceptTags;
    std::array<const char*, 2> conflictPairs;
    double baseConfidence;
};

static const std::array<TrendProfileLocal, 6> kTrendProfiles = {{
    {
        "artisan-leather", "cognac", "matte leather", "concept volume", "forecast-cockpit-competitive-balanced",
        {QColor("#664015"), QColor("#8c6126"), QColor("#bf9959")},
        {"cognac","caramel","sand"},
        {"Veste premium","Veste technique","Ceinture luxe"},
        {{{"artisan-leather","cognac","matte leather","concept volume"},
          {"minimal-tailoring","neon-lime","nubuck","elongated precision"},
          {"romantic-fluid","ink-blue","bio-fiber satin","clean shoulder"}}},
        {"artisan-leather suppresses athlux","artisan-leather suppresses con"},
        69.0
    },
    {
        "minimal-tailoring", "taupe", "laine structurée", "sharp line", "forecast-cockpit-minimal-optimized",
        {QColor("#8c8575"), QColor("#bfb399"), QColor("#e6d9bf")},
        {"taupe","stone","ivory"},
        {"Veste épurée","Manteau droit","Pantalon tailleur"},
        {{{"minimal-tailoring","taupe","laine structurée","sharp line"},
          {"athlux-utility","steel-blue","nylon premium","utility frame"},
          {"conceptual","lavender","silk blend","drape asymmetric"}}},
        {"minimal-tailoring suppresses romantic","minimal suppresses conceptual"},
        72.0
    },
    {
        "romantic-fluid", "blush", "soie fluide", "fluid drape", "forecast-cockpit-romantic-wave",
        {QColor("#d99aa6"), QColor("#b3738c"), QColor("#f2ccc0")},
        {"blush","mauve","rose poudré"},
        {"Robe fluide","Blouse romantique","Jupe évasée"},
        {{{"romantic-fluid","blush","soie fluide","fluid drape"},
          {"artisan-leather","cognac","velours","structured ease"},
          {"minimal-tailoring","ivory","georgette","soft column"}}},
        {"romantic suppresses athlux","romantic-fluid vs minimal-tail"},
        65.0
    },
    {
        "athlux", "lavender", "soie mate", "fluid drape", "forecast-cockpit-athlux-performance",
        {QColor("#8c73bf"), QColor("#668ca6"), QColor("#bf9966")},
        {"lavender","cobalt","beige"},
        {"Veste sport-luxe","Legging technique","Blouson urbain"},
        {{{"athlux-utility","steel-blue","nylon premium","utility frame"},
          {"artisan-leather","cognac","cuir pleine fleur","structured tote"},
          {"minimal-tailoring","taupe","laine structurée","sharp line"}}},
        {"athlux suppresses romantic","athlux-utility vs minimal-tail"},
        70.0
    },
    {
        "conceptual", "ink-blue", "bio-fiber satin", "clean shoulder", "forecast-cockpit-conceptual-disruptive",
        {QColor("#263373"), QColor("#4d598c"), QColor("#8c80a6")},
        {"ink-blue","midnight","slate"},
        {"Manteau sculptural","Robe architecturale","Veste déstructurée"},
        {{{"conceptual","ink-blue","bio-fiber satin","clean shoulder"},
          {"romantic-fluid","blush","organza","cape volume"},
          {"athlux-utility","charcoal","tech mesh","modular pocket"}}},
        {"conceptual suppresses artisan","conceptual vs annual"},
        63.0
    },
    {
        "annual", "sand", "coton premium", "relaxed tailoring", "forecast-cockpit-annual-stable",
        {QColor("#ccb88c"), QColor("#a68c66"), QColor("#e6d1ad")},
        {"sand","wheat","cream"},
        {"Chemise essentiel","Pantalon casual","Veste quotidienne"},
        {{{"annual","sand","coton premium","relaxed tailoring"},
          {"minimal-tailoring","stone","laine fine","straight cut"},
          {"artisan-leather","cognac","denim premium","workwear edit"}}},
        {"annual vs artisan-leather","annual suppresses conceptual"},
        75.0
    }
}};

static double computeCurveScoreLocal(const TrendCurveLocal &t, int year)
{
    const double y = static_cast<double>(year - 2025);
    double s = t.base2025 + t.momentum * y;
    if (t.peakYear > 0.0 && static_cast<double>(year) > t.peakYear) {
        const double overPeak = static_cast<double>(year) - t.peakYear;
        s += t.peakDrop * overPeak;
    }
    return std::clamp(s, 0.0, 100.0);
}

static void applyScenarioBiasLocal(std::array<double, 6> &scores, const QString &scenarioIn)
{
    const QString s = scenarioIn.trimmed().toLower();
    // indices: 0=artisan, 1=minimal, 2=romantic, 3=athlux, 4=conceptual, 5=annual
    if (s == QStringLiteral("aggressive_growth")) {
        scores[0] += 6.0;
        scores[1] -= 4.0;
        scores[4] += 5.0;
    } else if (s == QStringLiteral("risk_averse")) {
        scores[5] += 5.0;
        scores[3] -= 3.0;
        scores[2] += 3.0;
    } else if (s == QStringLiteral("disruptive")) {
        scores[4] += 8.0;
        scores[5] -= 6.0;
        scores[3] += 4.0;
    } else if (s == QStringLiteral("conservative")) {
        scores[1] += 5.0;
        scores[5] += 3.0;
        scores[4] -= 4.0;
    }
    for (double &v : scores)
        v = std::clamp(v, 0.0, 100.0);
}

static int argmaxLocal(const std::array<double, 6> &scores)
{
    int best = 0;
    for (int i = 1; i < 6; ++i) {
        if (scores[i] > scores[best])
            best = i;
    }
    return best;
}

static ForecastResultLocal computeFashionForecastLocal(int year, const QString &scenarioIn)
{
    const QString scenario = scenarioIn.trimmed().toLower();
    double wTrend = 0.4, wMaterial = 0.3, wSil = 0.3;
    double confidenceBias = 0.0;
    if (scenario == QStringLiteral("aggressive_growth")) {
        wTrend = 0.6; wMaterial = 0.2; wSil = 0.2;
        confidenceBias = -6.0;
    } else if (scenario == QStringLiteral("risk_averse")) {
        wTrend = 0.2; wMaterial = 0.4; wSil = 0.4;
        confidenceBias = 7.5;
    } else if (scenario == QStringLiteral("disruptive")) {
        wTrend = 0.5; wMaterial = 0.1; wSil = 0.4;
        confidenceBias = -3.0;
    } else if (scenario == QStringLiteral("conservative")) {
        wTrend = 0.3; wMaterial = 0.4; wSil = 0.3;
        confidenceBias = 4.0;
    }

    const std::array<TrendCurveLocal, 6> curves = {{
        {"artisan-leather",   35.0, +9.0, 2028.0, -11.0, QColor("#8c4512")},
        {"minimal-tailoring", 62.0, -2.0,    0.0,  0.0, QColor("#597f66")},
        {"romantic-fluid",    28.0, +3.5, 2030.0, -4.0, QColor("#a6668c")},
        {"athlux",            42.0, +6.5, 2031.0, -8.0, QColor("#3366a6")},
        {"conceptual",        12.0, +7.5, 2034.0, -3.0, QColor("#7f4ca6")},
        {"annual",            78.0, -5.5,    0.0,  0.0, QColor("#bf8c40")}
    }};
    const QStringList trendLabels = {QStringLiteral("artisan-leather"), QStringLiteral("minimal-tailoring"),
                                     QStringLiteral("romantic-fluid"), QStringLiteral("athlux"),
                                     QStringLiteral("conceptual"), QStringLiteral("annual")};

    ForecastResultLocal out;
    out.scenario = scenario;
    std::array<double, 6> raw{};
    const int yr = qBound(2025, year, 2040);
    for (int i = 0; i < 6; ++i) {
        raw[i] = computeCurveScoreLocal(curves[i], yr);
    }
    applyScenarioBiasLocal(raw, scenario);
    out.trendScores = raw;
    const int winnerIdx = argmaxLocal(raw);

    QVector<int> order = {0, 1, 2, 3, 4, 5};
    std::sort(order.begin(), order.end(), [&](int a, int b) { return raw[a] > raw[b]; });
    const TrendProfileLocal &p = kTrendProfiles[winnerIdx];
    const auto safeLatin = [](const char *txt, const QString &fallback = QString()) -> QString {
        return (txt && txt[0] != '\0') ? QString::fromLatin1(txt) : fallback;
    };
    out.dominantTrend = QString::fromLatin1(p.name);
    for (int r = 0; r < 3; ++r)
        out.topStyles << trendLabels.value(order[r]);

    const int yearPhase = qBound(0, (yr - 2025) % 3, 2);
    out.palette = safeLatin(p.paletteNames[yearPhase], safeLatin(p.palette, QStringLiteral("N/A")));
    out.material = safeLatin(p.conceptTags[yearPhase][2], safeLatin(p.matiere, QStringLiteral("N/A")));
    out.silhouette = safeLatin(p.conceptTags[(yearPhase + 1) % 3][3], safeLatin(p.silhouette, QStringLiteral("N/A")));
    out.moteur = QString("%1-%2")
                     .arg(safeLatin(p.moteur, QStringLiteral("forecast-cockpit")),
                          QStringList{QStringLiteral("phase-a"), QStringLiteral("phase-b"), QStringLiteral("phase-c")}.value(yearPhase));
    for (int i = 0; i < 3; ++i) {
        const int k = (yearPhase + i) % 3;
        out.paletteColors[i] = p.paletteColors[k];
        out.paletteNames << safeLatin(p.paletteNames[k], out.palette);
        out.palettes << safeLatin(p.paletteNames[k], out.palette);
        out.materials << safeLatin(p.conceptTags[k][2], out.material);
        out.silhouettes << safeLatin(p.conceptTags[k][3], out.silhouette);
        out.conceptNames << safeLatin(p.conceptNames[k], QStringLiteral("Concept %1").arg(i + 1));
    }

    const int topMomentum = int(std::lround(curves[order[0]].momentum + curves[order[1]].momentum + curves[order[2]].momentum));
    out.momentum = qBound(0, topMomentum * 3 + 8, 99);
    out.riskScore = qBound(8, int(std::lround(100.0 - (raw[order[0]] * 0.55 + raw[order[1]] * 0.25 + raw[order[2]] * 0.20))), 92);
    double confMod = 1.0;
    if (scenario == QStringLiteral("aggressive_growth")) confMod = 0.88;
    else if (scenario == QStringLiteral("risk_averse")) confMod = 1.10;
    else if (scenario == QStringLiteral("disruptive")) confMod = 0.80;
    else if (scenario == QStringLiteral("conservative")) confMod = 1.05;
    out.confidence = std::clamp((p.baseConfidence * confMod + confidenceBias) / 100.0, 0.35, 0.99);

    out.conflicts.clear();
    out.conflicts << QString::fromLatin1(p.conflictPairs[0]) << QString::fromLatin1(p.conflictPairs[1]);

    for (int i = 0; i < 3; ++i) {
        const int idx = order[i];
        ForecastConceptLocal c;
        c.productName = out.conceptNames.value(i, QStringLiteral("Concept %1").arg(i + 1));
        c.style = safeLatin(p.conceptTags[i][0], out.dominantTrend);
        c.palette = safeLatin(p.conceptTags[i][1], out.palette);
        c.material = safeLatin(p.conceptTags[i][2], out.material);
        c.silhouette = safeLatin(p.conceptTags[i][3], out.silhouette);
        c.marketFit = qBound(35, int(std::lround(raw[idx] - i * 3)), 98);
        c.materialFit = qBound(30, int(std::lround(56.0 + wMaterial * 34.0 + (idx % 3) * 6.0 - i * 2.0)), 99);
        c.silhouetteFit = qBound(30, int(std::lround(54.0 + wSil * 33.0 + ((idx + 1) % 3) * 5.0 - i * 2.0)), 99);
        c.globalFit = qBound(0, int(std::lround(c.marketFit * 0.45 + c.materialFit * 0.30 + c.silhouetteFit * 0.25)), 100);
        c.badge = (i == 0) ? QStringLiteral("Peak") : (i == 1 ? QStringLiteral("Rising") : QStringLiteral("Differentiator"));
        out.concepts[i] = c;
    }
    return out;
}

static QJsonObject buildFashionForecastPayloadLocal(int year, const QString &scenario)
{
    const ForecastResultLocal fr = computeFashionForecastLocal(year, scenario);
    QJsonObject root;
    root.insert(QStringLiteral("year"), year);
    root.insert(QStringLiteral("forecast_eligible"), true);
    root.insert(QStringLiteral("summary"),
                QStringLiteral("Projection locale %1 : priorité %2, exécution progressive et pilotage multi-signaux.")
                    .arg(year)
                    .arg(fr.dominantTrend));
    root.insert(QStringLiteral("confidence"), fr.confidence);
    root.insert(QStringLiteral("inference_mode"), fr.moteur);
    root.insert(QStringLiteral("prediction_engine_version"), QStringLiteral("local-deterministic-v1"));
    root.insert(QStringLiteral("from_cache"), false);
    root.insert(QStringLiteral("dominant_direction"), fr.dominantTrend);
    root.insert(QStringLiteral("market_posture"), QStringLiteral("Balanced selective growth"));
    root.insert(QStringLiteral("risk_level"), fr.riskScore < 30 ? QStringLiteral("Low") : (fr.riskScore < 60 ? QStringLiteral("Medium") : QStringLiteral("High")));
    root.insert(QStringLiteral("recommended_capsule"), QStringLiteral("Capsule %1").arg(fr.dominantTrend));
    root.insert(QStringLiteral("strongest_signal"), QStringLiteral("Signal dominant : %1").arg(fr.dominantTrend));
    root.insert(QStringLiteral("year_over_year_evolution"), QStringLiteral("Momentum recalibré sur %1").arg(year));
    root.insert(QStringLiteral("source"), QStringLiteral("Fashion Oracle API"));

    QJsonArray topStyles, palettes, mats, sils;
    QJsonArray paletteColorsHex;
    for (int i = 0; i < 3; ++i) {
        topStyles.append(fr.topStyles.value(i));
        palettes.append(fr.palettes.value(i));
        mats.append(fr.materials.value(i));
        sils.append(fr.silhouettes.value(i));
        paletteColorsHex.append(fr.paletteColors[i].name());
    }
    root.insert(QStringLiteral("top_styles"), topStyles);
    root.insert(QStringLiteral("color_palette"), palettes);
    root.insert(QStringLiteral("palette_colors"), paletteColorsHex);
    root.insert(QStringLiteral("fabrics_materials"), mats);
    root.insert(QStringLiteral("silhouettes"), sils);
    root.insert(QStringLiteral("similar_decades"), QJsonArray{QStringLiteral("2020"), QStringLiteral("2025"), QStringLiteral("2030"), QStringLiteral("2035")});

    QJsonObject reco;
    reco.insert(QStringLiteral("Axe style"), fr.dominantTrend);
    reco.insert(QStringLiteral("Niveau risque"), fr.riskScore < 30 ? QStringLiteral("faible") : (fr.riskScore < 60 ? QStringLiteral("modéré") : QStringLiteral("élevé")));
    root.insert(QStringLiteral("recommended_product_attributes"), reco);

    QJsonObject traj;
    QJsonArray years;
    for (int y = 2025; y <= 2032; ++y)
        years.append(y);
    traj.insert(QStringLiteral("years"), years);
    const std::array<TrendCurveLocal, 6> curves = {{
        {"artisan-leather",   35.0, +9.0, 2028.0, -11.0, QColor("#8c4512")},
        {"minimal-tailoring", 62.0, -2.0,    0.0,  0.0, QColor("#597f66")},
        {"romantic-fluid",    28.0, +3.5, 2030.0, -4.0, QColor("#a6668c")},
        {"athlux",            42.0, +6.5, 2031.0, -8.0, QColor("#3366a6")},
        {"conceptual",        12.0, +7.5, 2034.0, -3.0, QColor("#7f4ca6")},
        {"annual",            78.0, -5.5,    0.0,  0.0, QColor("#bf8c40")}
    }};
    std::array<double, 6> scenarioCurveBias = {0, 0, 0, 0, 0, 0};
    applyScenarioBiasLocal(scenarioCurveBias, scenario);
    auto mkCurve = [&](int idx) {
        QJsonArray a;
        for (int y = 2025; y <= 2032; ++y)
            a.append(std::clamp(computeCurveScoreLocal(curves[idx], y) + scenarioCurveBias[idx], 0.0, 100.0));
        return a;
    };
    traj.insert(QStringLiteral("artisan_leather_curve"), mkCurve(0));
    traj.insert(QStringLiteral("romantic_fluid_curve"), mkCurve(2));
    traj.insert(QStringLiteral("athlux_utility_curve"), mkCurve(3));
    traj.insert(QStringLiteral("minimal_tailoring_curve"), mkCurve(1));
    traj.insert(QStringLiteral("conceptual_futurism_curve"), mkCurve(4));
    traj.insert(QStringLiteral("annual_curve"), mkCurve(5));
    traj.insert(QStringLiteral("dominant_trend"), fr.topStyles.value(0));
    traj.insert(QStringLiteral("next_dominant_trend"), fr.topStyles.value(1));
    traj.insert(QStringLiteral("dominance_shifts"), QJsonArray{QStringLiteral("rank flip"), QStringLiteral("momentum shift")});
    traj.insert(QStringLiteral("shock_events"), QJsonArray{QJsonObject{{QStringLiteral("label"), QStringLiteral("supply tension")}}});
    QJsonArray conflictNotes;
    for (const QString &c : fr.conflicts)
        conflictNotes.append(c);
    traj.insert(QStringLiteral("conflict_notes"), conflictNotes);
    traj.insert(QStringLiteral("overtakes"), QJsonArray{QStringLiteral("artisan > minimal")});
    root.insert(QStringLiteral("trend_trajectory"), traj);

    QJsonObject raw;
    raw.insert(QStringLiteral("scenario_mode"), scenario);
    raw.insert(QStringLiteral("volatility_proxy"), fr.riskScore * 0.8);
    raw.insert(QStringLiteral("uncertainty_penalty"), std::max(0, fr.riskScore - 25));
    raw.insert(QStringLiteral("temporal_certainty"), fr.confidence * 100.0);
    raw.insert(QStringLiteral("signal_coherence"), fr.confidence * 95.0);
    raw.insert(QStringLiteral("ranking_gap"), fr.trendScores[0] - fr.trendScores[1]);
    raw.insert(QStringLiteral("commercial_index"), fr.trendScores[0]);
    raw.insert(QStringLiteral("score_artisan_leather"), fr.trendScores[0]);
    raw.insert(QStringLiteral("score_minimal_tailoring"), fr.trendScores[1]);
    raw.insert(QStringLiteral("score_romantic_fluid"), fr.trendScores[2]);
    raw.insert(QStringLiteral("score_athlux"), fr.trendScores[3]);
    raw.insert(QStringLiteral("score_conceptual"), fr.trendScores[4]);
    raw.insert(QStringLiteral("score_annual"), fr.trendScores[5]);
    raw.insert(QStringLiteral("winner_idx"), argmaxLocal(fr.trendScores));
    raw.insert(QStringLiteral("winner_name"), fr.dominantTrend);
    root.insert(QStringLiteral("raw_scores"), raw);

    QJsonArray concepts;
    for (const auto &c : fr.concepts) {
        QJsonObject o;
        o.insert(QStringLiteral("product_name"), c.productName);
        o.insert(QStringLiteral("style"), c.style);
        o.insert(QStringLiteral("palette"), c.palette);
        o.insert(QStringLiteral("material"), c.material);
        o.insert(QStringLiteral("silhouette"), c.silhouette);
        o.insert(QStringLiteral("innovation_score"), c.marketFit);
        o.insert(QStringLiteral("market_score"), c.materialFit);
        o.insert(QStringLiteral("feasibility_score"), c.silhouetteFit);
        o.insert(QStringLiteral("trend_timing_score"), c.globalFit);
        o.insert(QStringLiteral("direction_badge"), c.badge);
        concepts.append(o);
    }
    root.insert(QStringLiteral("concepts_projection"), concepts);
    root.insert(QStringLiteral("rejected_concepts"), QJsonArray{QStringLiteral("Concept X - faible alignement"), QStringLiteral("Concept Y - risque élevé")});
    return root;
}

class MiniTrendSparkline final : public QWidget {
    QVector<double> vals;
    QColor lineColor{QStringLiteral("#8d5524")};
public:
    explicit MiniTrendSparkline(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(60, 18);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    void setSeries(const QJsonArray &arr, const QColor &c)
    {
        vals.clear();
        for (int i = qMax(0, arr.size() - 5); i < arr.size(); ++i)
            vals.push_back(arr.at(i).toDouble(0.0));
        while (vals.size() < 5)
            vals.push_back(vals.isEmpty() ? 0.0 : vals.last());
        lineColor = c;
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::transparent);
        if (vals.size() < 2)
            return;
        double mn = vals[0], mx = vals[0];
        for (double v : vals) { mn = std::min(mn, v); mx = std::max(mx, v); }
        if (std::abs(mx - mn) < 1e-9) { mx += 1.0; mn -= 1.0; }
        QPainterPath path;
        for (int i = 0; i < vals.size(); ++i) {
            const qreal x = 2.0 + qreal(i) * (width() - 4.0) / qreal(vals.size() - 1);
            const qreal y = height() - 2.0 - ((vals[i] - mn) / (mx - mn)) * (height() - 4.0);
            if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
        }
        QPen pen(lineColor, 1.4);
        p.setPen(pen);
        p.drawPath(path);
    }
};

struct CostTabAnimState {
    double lastCost = -1.0;
    double lastDispCost = -1.0;
    double lastMarg = -1.0;
    double lastRisque = -1.0;
    int lastScore = -1;
    int lastStab = -1;
    int scenarioPick = 1;
    quint32 projectionSeed = 1u;
    QString lastDecision;
    int aiBlockRevealGen = 0;
    QAbstractAnimation *aiPulseAnim = nullptr;
    QAbstractAnimation *tierShimmerAnim = nullptr;
    QAbstractAnimation *hdrSweepAnim = nullptr;
    QPointer<QTimer> hdrStripBreathTimer;
    qint64 analysisT0 = 0;
};

struct CostShadowHoverLift final : QObject {
    QWidget *w = nullptr;
    QGraphicsDropShadowEffect *fx = nullptr;
    int baseBlur = 14;
    int baseDx = 0;
    int baseDy = 3;
    const int hoverBlur = 32;
    const int hoverDx = 0;
    const int hoverDy = 10;

    CostShadowHoverLift(QWidget *target, QGraphicsDropShadowEffect *effect, QObject *parent)
        : QObject(parent), w(target), fx(effect)
    {
        if (fx) {
            baseBlur = qMax(6, static_cast<int>(fx->blurRadius()));
            baseDx = int(fx->xOffset());
            baseDy = int(fx->yOffset());
        }
        if (w)
            w->installEventFilter(this);
    }

    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (!fx || !w || o != w)
            return QObject::eventFilter(o, e);
        if (e->type() == QEvent::Enter) {
            fx->setBlurRadius(hoverBlur);
            fx->setOffset(hoverDx, hoverDy);
        } else if (e->type() == QEvent::Leave) {
            fx->setBlurRadius(baseBlur);
            fx->setOffset(baseDx, baseDy);
        }
        return QObject::eventFilter(o, e);
    }
};

static int costProductScoreFromKpis(double margePct, double risqueScore, int volume)
{
    const double x = margePct * 0.5 + (100.0 - risqueScore) * 0.3 + std::min(volume / 10.0, 20.0);
    return qBound(0, static_cast<int>(std::lround(x)), 100);
}

static int costConfidenceFromRisk(double risqueScore)
{
    if (risqueScore >= 55.0)
        return 78;
    if (risqueScore >= 35.0)
        return 85;
    return 92;
}

static void costDecomposeFive(const ProductAnalyzerInput &din, double totalTnd, double &wMat, double &wMo, double &wCh,
                              double &wLog, double &wReb)
{
    const double mat = std::max(0.01, din.coutMatiereTnd);
    const int vol = std::max(1, din.volumeCible);
    const double serie = std::clamp(0.88 + 0.028 * std::log(static_cast<double>(vol)), 0.82, 1.12);
    const double finitionBump = (din.niveauFinition == QStringLiteral("élevé")) ? 1.12 : 1.0;
    const double delaiStress = std::clamp(1.0 + (7 - din.delaiSemaines) * 0.018, 0.94, 1.14);
    const double matLoaded = mat * 1.22 * serie * finitionBump * delaiStress;
    const double fixedPool = std::max(0.0, totalTnd - matLoaded);
    wMat = std::max(0.0, matLoaded);
    wMo = fixedPool * 0.52;
    wCh = fixedPool * 0.22;
    wLog = fixedPool * 0.16;
    wReb = std::max(0.0, totalTnd - wMat - wMo - wCh - wLog);
}

class CostClickFrame final : public QFrame {
public:
    std::function<void()> onPressed;

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onPressed)
            onPressed();
        QFrame::mousePressEvent(e);
    }
};

class CostAiSignalGlyphStrip final : public QWidget {
public:
    explicit CostAiSignalGlyphStrip(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedSize(30, 16);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor amber(QStringLiteral("#FF9500"));
        p.setPen(Qt::NoPen);
        p.setBrush(amber);
        const qreal y0 = 3;
        const qreal y1 = height() - 3.0;
        const qreal x0 = 3.0;
        const qreal x1 = 11.0;
        const qreal xm = (x0 + x1) * 0.5;
        QPolygonF tri;
        tri << QPointF(x0, y0) << QPointF(x1, y0) << QPointF(xm, y1);
        p.drawPolygon(tri);

        p.setBrush(Qt::NoBrush);
        QPen pen(amber.lighter(115));
        pen.setWidthF(1.15);
        p.setPen(pen);
        const qreal cx = 22.0;
        const qreal cy = (y0 + y1) * 0.5;
        QPolygonF dia;
        dia << QPointF(cx, cy - 4.2) << QPointF(cx + 5.0, cy) << QPointF(cx, cy + 4.2) << QPointF(cx - 5.0, cy);
        p.drawPolygon(dia);
    }
};

class CostMiniHBar final : public QWidget {
    double frac = 0.0;
    QColor fill{QStringLiteral("#d4841a")};

public:
    explicit CostMiniHBar(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(10);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    void setFraction(double f)
    {
        frac = std::clamp(f, 0.0, 1.0);
        update();
    }
    void setFill(const QColor &c)
    {
        fill = c;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRect r = rect();
        const int trackH = 3;
        const int y = (r.height() - trackH) / 2;
        QRect track(0, y, r.width(), trackH);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 14));
        p.drawRoundedRect(track, 2, 2);
        const int fw = int(std::round(double(r.width()) * frac));
        if (fw > 0) {
            QRect ch(0, y, fw, trackH);
            p.setBrush(fill);
            p.drawRoundedRect(ch, 2, 2);
        }
    }
};

class CostWaterfallSection final : public QFrame {
    QVector<QLabel *> valLabs;
    QVector<CostMiniHBar *> bars;

public:
    explicit CostWaterfallSection(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setStyleSheet(QStringLiteral(
            "QFrame{background:#1a1712;border:1px solid #3a3428;border-radius:8px;}"));
        auto *vl = new QVBoxLayout(this);
        vl->setContentsMargins(14, 14, 14, 14);
        vl->setSpacing(10);
        auto *t = new QLabel(QStringLiteral("DÉCOMPOSITION COÛT"));
        t->setStyleSheet(QStringLiteral(
            "QLabel{font-size:9px;font-weight:900;letter-spacing:1.5px;color:#888888;background:transparent;border:none;}"));
        vl->addWidget(t);
        const QStringList names = {QStringLiteral("Matière"), QStringLiteral("Main d'œuvre"),
                                   QStringLiteral("Charges ind."), QStringLiteral("Logistique"),
                                   QStringLiteral("Rebut")};
        for (int i = 0; i < 5; ++i) {
            auto *row = new QWidget;
            auto *hl = new QHBoxLayout(row);
            hl->setContentsMargins(0, 0, 0, 0);
            hl->setSpacing(10);
            auto *lab = new QLabel(names.at(i));
            lab->setStyleSheet(QStringLiteral(
                "QLabel{font-size:11px;font-weight:700;color:#B0A69E;background:transparent;border:none;min-width:92px;}"));
            lab->setAttribute(Qt::WA_TransparentForMouseEvents);
            auto *bar = new CostMiniHBar(row);
            auto *vlab = new QLabel(QStringLiteral("0.00 TND"));
            vlab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            vlab->setMinimumWidth(88);
            vlab->setStyleSheet(QStringLiteral(
                "QLabel{font-size:11px;font-weight:800;color:#F0F0F0;background:transparent;border:none;"
                "font-variant-numeric:tabular-nums;}"));
            vlab->setAttribute(Qt::WA_TransparentForMouseEvents);
            hl->addWidget(lab, 0);
            hl->addWidget(bar, 1);
            hl->addWidget(vlab, 0);
            vl->addWidget(row);
            valLabs.push_back(vlab);
            bars.push_back(bar);
        }
    }

    void updateParts(const QVector<double> &amounts, const QVector<QColor> &colors, const QLocale &loc)
    {
        double sum = 0.0;
        for (double v : amounts)
            sum += v;
        if (sum < 1e-9)
            sum = 1e-9;
        for (int i = 0; i < 5 && i < amounts.size(); ++i) {
            valLabs[i]->setText(loc.toString(amounts.at(i), 'f', 2) + QStringLiteral(" TND"));
            bars[i]->setFraction(amounts.at(i) / sum);
            if (i < colors.size())
                bars[i]->setFill(colors.at(i));
        }
    }
};

class CostProjection90 final : public QWidget {
    QVector<double> sOpt, sReal, sPess;

public:
    explicit CostProjection90(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(80);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void rebuild(double baseTnd, quint32 seed)
    {
        sOpt.resize(9);
        sReal.resize(9);
        sPess.resize(9);
        QRandomGenerator rng(seed ? seed : 1u);
        const double n0 = rng.generateDouble() * 0.35 - 0.175;
        for (int i = 0; i < 9; ++i) {
            const double n = (rng.generateDouble() * 0.28 - 0.14);
            sReal[i] = baseTnd + 0.15 * double(i) + n * 0.4 + n0 * 0.25;
            sOpt[i] = baseTnd - 0.2 * double(i) + n * 0.35 + n0 * 0.2;
            sPess[i] = baseTnd + 0.5 * double(i) + n * 0.45 + n0 * 0.3;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRect r = rect().adjusted(6, 4, -6, -18);
        if (r.width() < 4 || r.height() < 4)
            return;
        double mn = sOpt.isEmpty() ? 0.0 : sOpt.first();
        double mx = mn;
        for (double v : sOpt) {
            mn = std::min(mn, v);
            mx = std::max(mx, v);
        }
        for (double v : sReal) {
            mn = std::min(mn, v);
            mx = std::max(mx, v);
        }
        for (double v : sPess) {
            mn = std::min(mn, v);
            mx = std::max(mx, v);
        }
        if (mx - mn < 1e-6) {
            mn -= 1.0;
            mx += 1.0;
        }
        const double span = mx - mn;
        auto xAt = [&](int i) -> qreal {
            return r.left() + qreal(i) * qreal(r.width()) / 8.0;
        };
        auto yAt = [&](double v) -> qreal {
            return r.bottom() - qreal((v - mn) / span) * qreal(r.height() - 1);
        };
        p.setPen(QPen(QColor(255, 255, 255, 22), 1));
        for (int g = 0; g <= 3; ++g) {
            const int yy = r.top() + g * r.height() / 3;
            p.drawLine(r.left(), yy, r.right(), yy);
        }
        auto drawSeries = [&](const QVector<double> &s, const QColor &col, bool dashed) {
            if (s.size() < 2)
                return;
            QPainterPath path;
            path.moveTo(xAt(0), yAt(s.first()));
            for (int i = 1; i < s.size(); ++i)
                path.lineTo(xAt(i), yAt(s.at(i)));
            QPen pen(col, 1.6);
            if (dashed)
                pen.setStyle(Qt::DashLine);
            p.setPen(pen);
            p.drawPath(path);
            p.setBrush(col);
            for (int i = 0; i < s.size(); ++i)
                p.drawEllipse(QPointF(xAt(i), yAt(s.at(i))), 2.2, 2.2);
        };
        drawSeries(sOpt, QColor(QStringLiteral("#4CAF50")), true);
        drawSeries(sReal, QColor(QStringLiteral("#FF9500")), false);
        drawSeries(sPess, QColor(QStringLiteral("#E57373")), true);
        p.setPen(QColor(136, 136, 136));
        QFont f = p.font();
        f.setPixelSize(8);
        p.setFont(f);
        const QStringList ticks = {QStringLiteral("J+0"), QStringLiteral("J+10"), QStringLiteral("J+20"),
                                   QStringLiteral("J+30"), QStringLiteral("J+40"), QStringLiteral("J+50"),
                                   QStringLiteral("J+60"), QStringLiteral("J+70"), QStringLiteral("J+80")};
        for (int i = 0; i < 9; ++i) {
            const qreal x = xAt(i) - 14;
            p.drawText(QRectF(x, rect().bottom() - 16, 28, 14), Qt::AlignHCenter | Qt::AlignTop, ticks.at(i));
        }
    }
};

}

void MainWindow::showProduitCoutDialog() {
    if (ui->tabWidgetProduits->count() < 5)
        return;

    if (m_costSimReply) {
        m_costSimReply->disconnect();
        m_costSimReply->abort();
        m_costSimReply->deleteLater();
        m_costSimReply.clear();
    }
    m_costSimHtmlOut.clear();

    QWidget *ongletCout = ui->tabWidgetProduits->widget(4);
    for (CostIntelligenceEngine *e : ongletCout->findChildren<CostIntelligenceEngine *>())
        e->deleteLater();

    if (ongletCout->layout()) {
        clearLayout(ongletCout->layout());
        delete ongletCout->layout();
    }

    auto *rootLay = new QVBoxLayout(ongletCout);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);
    ongletCout->setStyleSheet(QStringLiteral("QWidget{background:#0C0C0C;}"));

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QStringLiteral(
        "QScrollArea{border:none;background:#0C0C0C;}"));
    auto *cw = new QWidget;
    cw->setStyleSheet(QStringLiteral("background:#121212;"));
    auto *cwOuter = new QVBoxLayout(cw);
    cwOuter->setContentsMargins(0, 0, 0, 0);
    cwOuter->setSpacing(0);
    auto *centerRail = new QHBoxLayout;
    centerRail->setContentsMargins(0, 0, 0, 0);
    centerRail->addStretch(1);
    auto *contentHost = new QWidget;
    contentHost->setMinimumWidth(560);
    contentHost->setMaximumWidth(1440);
    contentHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    centerRail->addWidget(contentHost, 0, Qt::AlignHCenter);
    centerRail->addStretch(1);
    cwOuter->addLayout(centerRail);

    auto *v = new QVBoxLayout(contentHost);
    v->setAlignment(Qt::AlignTop);
    v->setContentsMargins(18, 16, 18, 22);
    v->setSpacing(18);

    auto *consoleBar = new QWidget;
    consoleBar->setStyleSheet(QStringLiteral(
        "QWidget{background:#141414;border:1px solid #252525;border-radius:10px;}"));
    auto *cbLay = new QHBoxLayout(consoleBar);
    cbLay->setContentsMargins(16, 12, 16, 12);
    cbLay->setSpacing(16);
    auto *lblLiveEngine = new QLabel(QStringLiteral("●  LIVE ENGINE"));
    lblLiveEngine->setStyleSheet(QStringLiteral(
        "QLabel{font-size:10px;font-weight:900;letter-spacing:1.4px;color:#FF9500;background:transparent;border:none;}"));
    auto *lblConsoleTitle = new QLabel(QStringLiteral("SIMULATEUR DE COÛT — INTELLIGENCE LAYER V2"));
    lblConsoleTitle->setAlignment(Qt::AlignCenter);
    lblConsoleTitle->setStyleSheet(QStringLiteral(
        "QLabel{font-size:13px;font-weight:800;letter-spacing:0.8px;color:#F0F0F0;background:transparent;border:none;}"));
    auto *lblConsoleClock = new QLabel(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")));
    lblConsoleClock->setStyleSheet(QStringLiteral(
        "QLabel{font-size:12px;font-weight:700;letter-spacing:0.5px;color:#888888;background:transparent;border:none;font-variant-numeric:tabular-nums;}"));
    cbLay->addWidget(lblLiveEngine, 0, Qt::AlignVCenter);
    cbLay->addStretch(1);
    cbLay->addWidget(lblConsoleTitle, 0, Qt::AlignVCenter);
    cbLay->addStretch(1);
    cbLay->addWidget(lblConsoleClock, 0, Qt::AlignVCenter);
    auto *clockTimer = new QTimer(consoleBar);
    clockTimer->setInterval(1000);
    QObject::connect(clockTimer, &QTimer::timeout, lblConsoleClock, [lblConsoleClock]() {
        lblConsoleClock->setText(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")));
    });
    clockTimer->start();

    auto *leLib = new QLineEdit;
    leLib->setPlaceholderText(QStringLiteral("Réf."));
    const int prSel = ui->tableProduits->currentRow();
    if (prSel >= 0 && prSel < ui->tableProduits->rowCount()) {
        if (QTableWidgetItem *it0 = ui->tableProduits->item(prSel, 1)) {
            const QString t0 = it0->text().trimmed();
            if (!t0.isEmpty())
                leLib->setText(t0);
        }
    }
    auto *sbMat = new QDoubleSpinBox;
    sbMat->setRange(0.01, 1e6);
    sbMat->setDecimals(2);
    sbMat->setSuffix(QStringLiteral(" TND"));
    sbMat->setValue(32.0);
    auto *sbVol = new QSpinBox;
    sbVol->setRange(1, 1000000);
    sbVol->setValue(200);

    const QString costFieldStyle = QStringLiteral(
        "QLineEdit,QDoubleSpinBox,QSpinBox{background:#1A1A1A;border:1px solid #2E2E2E;border-radius:8px;"
        "padding:9px 11px;min-height:24px;font-size:13px;color:#F0F0F0;selection-background-color:#FF9500;selection-color:#0C0C0C;}"
        "QLineEdit:focus,QDoubleSpinBox:focus,QSpinBox:focus{border:1px solid #FF9500;background:#161616;}");
    leLib->setStyleSheet(costFieldStyle);
    sbMat->setStyleSheet(costFieldStyle);
    sbVol->setStyleSheet(costFieldStyle);

    auto *btnAi = new QPushButton(QStringLiteral("Analyser"));
    btnAi->setCursor(Qt::PointingHandCursor);
    btnAi->setMinimumWidth(168);
    btnAi->setMaximumWidth(220);
    btnAi->setMinimumHeight(48);
    btnAi->setStyleSheet(QStringLiteral(
        "QPushButton{background:#FF9500;color:#0C0C0C;font-size:13px;font-weight:900;border:none;border-radius:8px;padding:11px 26px;"
        "letter-spacing:0.5px;}"
        "QPushButton:hover{background:#FFB340;}"
        "QPushButton:pressed{background:#E68600;padding-top:12px;padding-bottom:10px;}"
        "QPushButton:disabled{background:#3A3A3A;color:#666666;}"));
    auto *btnAiPressFx = new QGraphicsOpacityEffect(btnAi);
    btnAiPressFx->setOpacity(1.0);
    btnAi->setGraphicsEffect(btnAiPressFx);
    auto *btnAiPressAnim = new QPropertyAnimation(btnAiPressFx, QByteArrayLiteral("opacity"), btnAi);
    btnAiPressAnim->setDuration(85);
    btnAiPressAnim->setEasingCurve(QEasingCurve::OutQuad);

    auto *microW = new QWidget;
    microW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *mh = new QHBoxLayout(microW);
    mh->setContentsMargins(0, 0, 0, 0);
    mh->setSpacing(8);
    auto *btnStress = new QPushButton(QStringLiteral("+10%"));
    btnStress->setCursor(Qt::PointingHandCursor);
    btnStress->setToolTip(QStringLiteral("+10 % matière"));
    btnStress->setStyleSheet(QStringLiteral(
        "QPushButton{font-size:11px;font-weight:800;padding:7px 12px;border-radius:8px;background:#1E1E1E;border:1px solid #333;color:#FF9500;}"
        "QPushButton:hover{background:#2A2A2A;}"));
    auto *sldSens = new QSlider(Qt::Horizontal);
    sldSens->setRange(-15, 15);
    sldSens->setValue(0);
    sldSens->setToolTip(QStringLiteral("Matière ±"));
    auto *lblSens = new QLabel(QStringLiteral("+0%"));
    lblSens->setFixedWidth(44);
    lblSens->setAlignment(Qt::AlignCenter);
    lblSens->setStyleSheet(QStringLiteral("font-size:11px;font-weight:800;color:#FF9500;"));
    auto *modeBtn = new QPushButton(QStringLiteral("⚡"));
    modeBtn->setCheckable(true);
    modeBtn->setFixedWidth(40);
    modeBtn->setCursor(Qt::PointingHandCursor);
    modeBtn->setToolTip(QStringLiteral("⚡ rapide · 🧠 détail"));
    modeBtn->setStyleSheet(QStringLiteral(
        "QPushButton{font-size:14px;border-radius:8px;background:#1A1A1A;border:1px solid #333;color:#E0E0E0;padding:6px;}"));
    mh->addWidget(btnStress);
    mh->addWidget(sldSens, 1);
    mh->addWidget(lblSens);
    mh->addWidget(modeBtn);
    auto *simW = new QWidget;
    simW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *simLay = new QHBoxLayout(simW);
    simLay->setContentsMargins(0, 0, 0, 0);
    simLay->setSpacing(10);
    auto *lblCostSim = new QLabel(QStringLiteral("Simulation coût"));
    lblCostSim->setStyleSheet(QStringLiteral("font-size:10px;font-weight:800;color:#888888;letter-spacing:0.7px;"));
    auto *sldCostShift = new QSlider(Qt::Horizontal);
    sldCostShift->setRange(-25, 25);
    sldCostShift->setValue(0);
    auto *lblCostShift = new QLabel(QStringLiteral("+0%"));
    lblCostShift->setFixedWidth(46);
    lblCostShift->setAlignment(Qt::AlignCenter);
    lblCostShift->setStyleSheet(QStringLiteral("font-size:11px;font-weight:800;color:#FF9500;"));
    auto *btnImprove = new QPushButton(QStringLiteral("Simulate improved version"));
    btnImprove->setCursor(Qt::PointingHandCursor);
    btnImprove->setStyleSheet(QStringLiteral(
        "QPushButton{font-size:11px;font-weight:800;padding:7px 12px;border-radius:8px;background:#142018;border:1px solid #2E4D3A;color:#81C784;}"
        "QPushButton:hover{background:#1A2A22;}"));
    simLay->addWidget(lblCostSim);
    simLay->addWidget(sldCostShift, 1);
    simLay->addWidget(lblCostShift);
    simLay->addWidget(btnImprove);

    leLib->setMinimumWidth(160);
    leLib->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sbMat->setMinimumWidth(120);
    sbVol->setMinimumWidth(100);

    auto *topInputsRow = new QWidget;
    topInputsRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *topH = new QHBoxLayout(topInputsRow);
    topH->setContentsMargins(0, 0, 0, 0);
    topH->setSpacing(24);
    const auto addLabeledField = [&topH](const QString &title, QWidget *field, int stretch) {
        auto *col = new QWidget;
        auto *cv = new QVBoxLayout(col);
        cv->setContentsMargins(0, 0, 0, 0);
        cv->setSpacing(6);
        auto *lt = new QLabel(title);
        lt->setStyleSheet(QStringLiteral(
            "QLabel{font-size:9px;font-weight:800;letter-spacing:1px;color:#888888;background:transparent;border:none;}"));
        cv->addWidget(lt);
        cv->addWidget(field);
        topH->addWidget(col, stretch);
    };
    addLabeledField(QStringLiteral("Produit"), leLib, 2);
    addLabeledField(QStringLiteral("Coût matière"), sbMat, 1);
    addLabeledField(QStringLiteral("Volume"), sbVol, 1);
    topH->addStretch(1);
    topH->addWidget(btnAi, 0, Qt::AlignBottom);

    auto *secondaryControlsRow = new QWidget;
    secondaryControlsRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *secLay = new QHBoxLayout(secondaryControlsRow);
    secLay->setContentsMargins(0, 4, 0, 0);
    secLay->setSpacing(32);
    secLay->addWidget(microW, 1);
    secLay->addWidget(simW, 2);

    const QString costSliderStyle = QStringLiteral(
        "QSlider::groove:horizontal{background:#252525;height:4px;border-radius:2px;border:none;}"
        "QSlider::handle:horizontal{background:#FF9500;width:14px;height:14px;margin:-5px 0;border-radius:7px;border:none;}"
        "QSlider::sub-page:horizontal{background:#3A3A3A;border-radius:2px;}"
        "QSlider::add-page:horizontal{background:#252525;border-radius:2px;}");
    sldSens->setStyleSheet(costSliderStyle);
    sldCostShift->setStyleSheet(costSliderStyle);

    auto *paramsShell = new QFrame;
    paramsShell->setStyleSheet(QStringLiteral(
        "QFrame{background:#161616;border:1px solid #252525;border-radius:10px;}"));
    auto *psOuter = new QVBoxLayout(paramsShell);
    psOuter->setContentsMargins(16, 14, 16, 16);
    psOuter->setSpacing(12);
    auto *lblParamsTitle = new QLabel(QStringLiteral("PARAMÈTRES COÛT"));
    lblParamsTitle->setStyleSheet(QStringLiteral(
        "QLabel{font-size:10px;font-weight:900;letter-spacing:1.6px;color:#888888;background:transparent;border:none;}"));
    psOuter->addWidget(lblParamsTitle);
    psOuter->addWidget(topInputsRow);
    psOuter->addWidget(secondaryControlsRow);

    auto *preAnalysisHost = new QWidget;
    preAnalysisHost->setObjectName(QStringLiteral("costPreAnalysisHost"));
    preAnalysisHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    preAnalysisHost->setStyleSheet(QStringLiteral(
        "QWidget#costPreAnalysisHost{"
        "background:#161616;"
        "border:1px solid #252525;border-radius:10px;}"));
    auto *phaOuter = new QVBoxLayout(preAnalysisHost);
    phaOuter->setContentsMargins(16, 16, 16, 20);
    phaOuter->setSpacing(0);

    auto *preAnalysisCard = new QFrame;
    preAnalysisCard->setObjectName(QStringLiteral("costPreAnalysisCard"));
    preAnalysisCard->setMaximumWidth(620);
    preAnalysisCard->setMinimumWidth(300);
    preAnalysisCard->setMinimumHeight(256);
    preAnalysisCard->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    preAnalysisCard->setStyleSheet(QStringLiteral(
        "QFrame#costPreAnalysisCard{"
        "background:#141414;"
        "border:1px solid #2A2A2A;border-radius:10px;}"));
    auto *cardSh = new QGraphicsDropShadowEffect(preAnalysisCard);
    cardSh->setBlurRadius(22);
    cardSh->setOffset(0, 6);
    cardSh->setColor(QColor(0, 0, 0, 26));
    preAnalysisCard->setGraphicsEffect(cardSh);

    auto *cardInner = new QVBoxLayout(preAnalysisCard);
    cardInner->setContentsMargins(28, 24, 28, 22);
    cardInner->setSpacing(16);

    auto *headRow = new QHBoxLayout;
    headRow->setSpacing(14);
    auto *pulseIcon = new QLabel(QStringLiteral("✦"));
    pulseIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    pulseIcon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    pulseIcon->setStyleSheet(QStringLiteral(
        "QLabel{font-size:26px;color:#FF9500;background:transparent;border:none;padding:4px 10px 0 0;}"));
    auto *iconFx = new QGraphicsOpacityEffect(pulseIcon);
    pulseIcon->setGraphicsEffect(iconFx);
    auto *iconPulse = new QPropertyAnimation(iconFx, QByteArrayLiteral("opacity"), pulseIcon);
    iconPulse->setDuration(1600);
    iconPulse->setStartValue(0.42);
    iconPulse->setEndValue(1.0);
    iconPulse->setEasingCurve(QEasingCurve::InOutSine);
    iconPulse->setLoopCount(-1);
    iconPulse->start();

    auto *ttlCol = new QWidget;
    ttlCol->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *ttlLay = new QVBoxLayout(ttlCol);
    ttlLay->setContentsMargins(0, 0, 0, 0);
    ttlLay->setSpacing(6);
    auto *lblPreTitle = new QLabel(QStringLiteral("Pré-analyse"));
    lblPreTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblPreTitle->setStyleSheet(QStringLiteral(
        "QLabel{font-size:20px;font-weight:800;color:#F5F5F5;letter-spacing:-0.2px;background:transparent;border:none;}"));
    auto *lblPreSub = new QLabel(QStringLiteral("Le moteur IA est prêt à analyser coût, marge et risque"));
    lblPreSub->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblPreSub->setWordWrap(true);
    lblPreSub->setStyleSheet(QStringLiteral(
        "QLabel{font-size:12px;font-weight:500;color:#A8A8A8;line-height:1.5;background:transparent;border:none;}"));
    auto *lblPreReady = new QLabel(QStringLiteral("Système prêt • en attente d’analyse"));
    lblPreReady->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblPreReady->setWordWrap(true);
    lblPreReady->setStyleSheet(QStringLiteral(
        "QLabel{font-size:11px;font-weight:700;letter-spacing:0.4px;color:#888888;background:transparent;border:none;}"));
    ttlLay->addWidget(lblPreTitle);
    ttlLay->addWidget(lblPreSub);
    ttlLay->addWidget(lblPreReady);
    headRow->addWidget(pulseIcon, 0, Qt::AlignTop);
    headRow->addWidget(ttlCol, 1);

    const auto mkPrevCell = [](const QString &cap, const QString &val) {
        auto *cell = new QFrame;
        cell->setStyleSheet(QStringLiteral(
            "QFrame{background:#1A1A1A;border:1px solid #2A2A2A;border-radius:8px;}"));
        auto *cl = new QVBoxLayout(cell);
        cl->setContentsMargins(14, 12, 14, 12);
        cl->setSpacing(4);
        auto *c = new QLabel(cap);
        c->setAttribute(Qt::WA_TransparentForMouseEvents);
        c->setStyleSheet(QStringLiteral(
            "QLabel{font-size:9px;font-weight:800;letter-spacing:0.9px;color:#888888;background:transparent;border:none;}"));
        auto *vlab = new QLabel(val);
        vlab->setAttribute(Qt::WA_TransparentForMouseEvents);
        vlab->setStyleSheet(QStringLiteral(
            "QLabel{font-size:17px;font-weight:800;color:#F0F0F0;background:transparent;border:none;}"));
        cl->addWidget(c);
        cl->addWidget(vlab);
        return cell;
    };
    auto *metricsRow = new QHBoxLayout;
    metricsRow->setSpacing(12);
    metricsRow->addWidget(mkPrevCell(QStringLiteral("COÛT ESTIMÉ"), QStringLiteral("— TND")), 1);
    metricsRow->addWidget(mkPrevCell(QStringLiteral("MARGE ESTIMÉE"), QStringLiteral("— %")), 1);
    metricsRow->addWidget(mkPrevCell(QStringLiteral("RISQUE ESTIMÉ"), QStringLiteral("—")), 1);

    auto *lblPreHint = new QLabel(QStringLiteral("Lancez l’analyse pour générer la décision IA"));
    lblPreHint->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblPreHint->setAlignment(Qt::AlignCenter);
    lblPreHint->setWordWrap(true);
    lblPreHint->setStyleSheet(QStringLiteral(
        "QLabel{font-size:11px;font-weight:600;color:#777777;background:transparent;border:none;padding:2px 0 0 0;}"));

    auto *shimmerBar = new QFrame;
    shimmerBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    shimmerBar->setFixedHeight(4);
    shimmerBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    shimmerBar->setStyleSheet(QStringLiteral(
        "QFrame{background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #D7CCC8, stop:0.5 #8D6E63, stop:1 #D7CCC8);"
        "border:none;border-radius:2px;}"));
    auto *shFx = new QGraphicsOpacityEffect(shimmerBar);
    shimmerBar->setGraphicsEffect(shFx);
    shFx->setOpacity(0.65);
    auto *shAnim = new QPropertyAnimation(shFx, QByteArrayLiteral("opacity"), shimmerBar);
    shAnim->setDuration(1800);
    shAnim->setStartValue(0.35);
    shAnim->setEndValue(1.0);
    shAnim->setEasingCurve(QEasingCurve::InOutSine);
    shAnim->setLoopCount(-1);
    shAnim->start();

    cardInner->addLayout(headRow);
    cardInner->addLayout(metricsRow);
    cardInner->addWidget(lblPreHint);
    cardInner->addWidget(shimmerBar);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch(1);
    centerRow->addWidget(preAnalysisCard, 0, Qt::AlignHCenter);
    centerRow->addStretch(1);
    phaOuter->addLayout(centerRow);

    v->addWidget(consoleBar);
    v->addWidget(paramsShell);
    v->addWidget(preAnalysisHost);

    auto *resultCard = new QFrame;
    resultCard->setMinimumWidth(520);
    resultCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    resultCard->setVisible(false);
    resultCard->setStyleSheet(QStringLiteral(
        "QFrame#smartCostCard{"
        "background: transparent;"
        "border: none;}"));
    resultCard->setObjectName(QStringLiteral("smartCostCard"));

    auto *lblBadge = new QLabel(QStringLiteral("—"));
    lblBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblBadge->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBadge->setWordWrap(false);
    lblBadge->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    lblBadge->setMinimumHeight(26);
    lblBadge->setStyleSheet(QStringLiteral(
        "font-size:9px;font-weight:800;letter-spacing:1.1px;border-radius:999px;padding:6px 14px;color:#E0E0E0;"
        "background:rgba(255,149,0,0.18);border:1px solid rgba(255,149,0,0.35);"));

    auto *lblConf = new QLabel(QStringLiteral("Confiance 0%"));
    lblConf->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblConf->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lblConf->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    lblConf->setStyleSheet(QStringLiteral(
        "font-size:11px;font-weight:800;color:#CCCCCC;letter-spacing:0.35px;background:transparent;border:none;"));
    auto *confProgressBar = new QProgressBar;
    confProgressBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    confProgressBar->setRange(0, 100);
    confProgressBar->setValue(0);
    confProgressBar->setFixedHeight(5);
    confProgressBar->setMaximumWidth(168);
    confProgressBar->setTextVisible(false);
    confProgressBar->setStyleSheet(QStringLiteral(
        "QProgressBar{border:none;border-radius:3px;background:rgba(255,255,255,0.08);height:4px;}"
        "QProgressBar::chunk{border-radius:3px;background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #FF9500,stop:1 #FFB84D);}"));
    auto *confWrap = new QWidget;
    confWrap->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *confVL = new QVBoxLayout(confWrap);
    confVL->setContentsMargins(0, 0, 0, 0);
    confVL->setSpacing(5);
    confVL->addWidget(lblConf, 0, Qt::AlignRight);
    confVL->addWidget(confProgressBar, 0, Qt::AlignRight);

    auto *aiStatusRow = new QWidget;
    aiStatusRow->setObjectName(QStringLiteral("costAiStatusRow"));
    aiStatusRow->setVisible(false);
    aiStatusRow->setStyleSheet(QStringLiteral("QWidget#costAiStatusRow{background:transparent;border:none;}"));
    auto *aiStatusH = new QHBoxLayout(aiStatusRow);
    aiStatusH->setContentsMargins(0, 2, 0, 4);
    aiStatusH->setSpacing(10);
    auto *lblAiPulseDot = new QLabel(QStringLiteral("●"));
    lblAiPulseDot->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblAiPulseDot->setStyleSheet(QStringLiteral(
        "QLabel{color:#FFAB40;font-size:12px;background:transparent;border:none;padding:0;}"));
    auto *lblAiRunning = new QLabel(QStringLiteral("Analyse en cours"));
    lblAiRunning->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblAiRunning->setStyleSheet(QStringLiteral(
        "QLabel{font-size:12px;font-weight:700;color:#CCCCCC;letter-spacing:0.2px;"
        "background:transparent;border:none;}"));
    aiStatusH->addWidget(lblAiPulseDot, 0, Qt::AlignVCenter);
    aiStatusH->addWidget(lblAiRunning, 0, Qt::AlignVCenter);
    aiStatusH->addStretch(1);

    // tier: 0=SIGNAL, 1=CONFLICT (tension), 2=DECISION (héros), 3=IMPACT — shim = shimmer avant révélation
    const auto mkAiTierSlot = [](int tier, const QString &titleUpper, bool showLine2) {
        auto *wrap = new QWidget;
        wrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        auto *wl = new QVBoxLayout(wrap);
        wl->setContentsMargins(0, 0, 0, 0);
        wl->setSpacing(3);

        auto *tierCard = new QFrame;
        tierCard->setObjectName(QStringLiteral("costAiTierCard"));
        tierCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        QString tierCss;
        int outerM = 4;
        int padH = 15;
        int padV = 12;
        int gapInner = 6;
        QString titleSs = QStringLiteral(
            "QLabel{font-size:8px;font-weight:900;letter-spacing:1.9px;color:#CFD8DC;background:transparent;"
            "border:none;}");
        QString l1Ss = QStringLiteral(
            "QLabel{font-size:13px;font-weight:700;color:#EDE7E2;background:transparent;border:none;}");
        QString l2Ss = QStringLiteral(
            "QLabel{font-size:11px;font-weight:600;color:#B0A69E;background:transparent;border:none;}");
        QString innerBg = QStringLiteral("rgba(26,22,20,0.94)");

        QGraphicsDropShadowEffect *tierSh = nullptr;
        if (tier == 0) {
            tierCss = QStringLiteral(
                "QFrame#costAiTierCard{background:rgba(0,0,0,0.11);border:none;border-radius:12px;}");
            tierSh = new QGraphicsDropShadowEffect(tierCard);
            tierSh->setBlurRadius(12);
            tierSh->setOffset(0, 2);
            tierSh->setColor(QColor(0, 0, 0, 28));
        } else if (tier == 1) {
            tierCss = QStringLiteral(
                "QFrame#costAiTierCard{"
                "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,171,64,0.16),stop:1 rgba(24,18,14,0.92));"
                "border:1px solid rgba(255,202,128,0.92);"
                "border-radius:12px;}");
            innerBg = QStringLiteral("rgba(22,16,12,0.97)");
            titleSs = QStringLiteral(
                "QLabel{font-size:8px;font-weight:900;letter-spacing:2px;color:#FFE0B2;background:transparent;"
                "border:none;}");
            l1Ss = QStringLiteral(
                "QLabel{font-size:14px;font-weight:800;color:#FFF8E1;background:transparent;border:none;}");
            tierSh = new QGraphicsDropShadowEffect(tierCard);
            tierSh->setBlurRadius(24);
            tierSh->setOffset(0, 5);
            tierSh->setColor(QColor(255, 145, 0, 70));
        } else if (tier == 2) {
            outerM = 7;
            padH = 24;
            padV = 19;
            gapInner = 11;
            tierCss = QStringLiteral(
                "QFrame#costAiTierCard{"
                "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,235,160,0.26),stop:1 rgba(90,60,20,0.22));"
                "border:1px solid rgba(255,253,231,0.95);"
                "border-radius:14px;}");
            innerBg = QStringLiteral("rgba(22,18,28,0.98)");
            titleSs = QStringLiteral(
                "QLabel{font-size:9px;font-weight:900;letter-spacing:2.5px;color:#FFF176;background:transparent;"
                "border:none;}");
            l1Ss = QStringLiteral(
                "QLabel{font-size:19px;font-weight:800;color:#FFFDE7;background:transparent;border:none;"
                "line-height:1.22;}");
            tierSh = new QGraphicsDropShadowEffect(tierCard);
            tierSh->setBlurRadius(32);
            tierSh->setOffset(0, 8);
            tierSh->setColor(QColor(255, 213, 79, 88));
        } else {
            tierCss = QStringLiteral(
                "QFrame#costAiTierCard{background:rgba(0,0,0,0.09);border:none;border-radius:12px;}");
            innerBg = QStringLiteral("rgba(24,20,18,0.88)");
            titleSs = QStringLiteral(
                "QLabel{font-size:8px;font-weight:800;letter-spacing:1.7px;color:#A1887F;background:transparent;"
                "border:none;}");
            l1Ss = QStringLiteral(
                "QLabel{font-size:12px;font-weight:600;color:rgba(236,230,224,0.74);background:transparent;"
                "border:none;line-height:1.4;}");
            tierSh = new QGraphicsDropShadowEffect(tierCard);
            tierSh->setBlurRadius(9);
            tierSh->setOffset(0, 2);
            tierSh->setColor(QColor(0, 0, 0, 18));
        }
        tierCard->setStyleSheet(tierCss);
        if (tierSh)
            tierCard->setGraphicsEffect(tierSh);

        auto *outerLay = new QVBoxLayout(tierCard);
        outerLay->setContentsMargins(outerM, outerM, outerM, outerM);
        outerLay->setSpacing(0);

        auto *inner = new QFrame;
        inner->setObjectName(QStringLiteral("costAiInner"));
        inner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        {
            QString innerExtra;
            if (tier == 2) {
                innerExtra = QStringLiteral(
                    "border-left:6px solid #FFE082;"
                    "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(36,28,48,1),stop:1 rgba(10,8,14,1));");
                innerBg.clear();
            }
            inner->setStyleSheet(QStringLiteral(
                "QFrame#costAiInner{background:%1;border:none;border-radius:12px;%2}")
                                     .arg(innerBg.isEmpty() ? QStringLiteral("transparent") : innerBg, innerExtra));
        }

        auto *vl = new QVBoxLayout(inner);
        vl->setContentsMargins(padH, padV, padH, padV);
        vl->setSpacing(gapInner);
        const QString tierIcon = (tier == 0) ? QStringLiteral("⌁")
            : (tier == 1)                      ? QStringLiteral("⚡")
                : (tier == 2)                  ? QStringLiteral("▸")
                                               : QStringLiteral("◇");
        QLabel *tt = nullptr;
        if (tier == 0) {
            auto *titleRow = new QHBoxLayout;
            titleRow->setContentsMargins(0, 0, 0, 0);
            titleRow->setSpacing(8);
            auto *glyph = new CostAiSignalGlyphStrip(inner);
            titleRow->addWidget(glyph, 0, Qt::AlignVCenter);
            tt = new QLabel(titleUpper);
            tt->setAttribute(Qt::WA_TransparentForMouseEvents);
            tt->setStyleSheet(titleSs);
            titleRow->addWidget(tt, 1, Qt::AlignVCenter);
            vl->addLayout(titleRow);
        } else {
            tt = new QLabel(QStringLiteral("%1  %2").arg(tierIcon, titleUpper));
            tt->setAttribute(Qt::WA_TransparentForMouseEvents);
            tt->setStyleSheet(titleSs);
            vl->addWidget(tt);
        }
        auto *l1 = new QLabel(QStringLiteral("—"));
        l1->setAttribute(Qt::WA_TransparentForMouseEvents);
        l1->setWordWrap(true);
        l1->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        l1->setStyleSheet(l1Ss);
        auto *l2 = new QLabel;
        l2->setAttribute(Qt::WA_TransparentForMouseEvents);
        l2->setWordWrap(true);
        l2->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        l2->setVisible(showLine2);
        l2->setStyleSheet(l2Ss);
        if (tier == 2) {
            auto *heroRow = new QHBoxLayout;
            heroRow->setSpacing(12);
            heroRow->setContentsMargins(0, 0, 0, 0);
            auto *ico = new QLabel(QStringLiteral("◆"));
            ico->setAttribute(Qt::WA_TransparentForMouseEvents);
            ico->setFixedWidth(24);
            ico->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
            ico->setStyleSheet(QStringLiteral(
                "QLabel{font-size:17px;color:#FFE082;background:transparent;border:none;padding-top:1px;}"));
            heroRow->addWidget(ico, 0, Qt::AlignTop);
            heroRow->addWidget(l1, 1, Qt::AlignTop);
            vl->addLayout(heroRow);
        } else {
            vl->addWidget(l1);
        }
        vl->addWidget(l2);
        outerLay->addWidget(inner);

        auto *liftLayer = new QWidget;
        liftLayer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        auto *liftLay = new QVBoxLayout(liftLayer);
        liftLay->setContentsMargins(0, 0, 0, 0);
        liftLay->setSpacing(0);
        liftLay->addWidget(tierCard);
        auto *shim = new QFrame;
        shim->setObjectName(QStringLiteral("costAiTierShim"));
        shim->setFixedHeight(2);
        shim->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        shim->setStyleSheet(QStringLiteral(
            "QFrame#costAiTierShim{border:none;border-radius:1px;"
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 transparent,stop:0.35 rgba(255,224,130,0.55),"
            "stop:0.55 rgba(255,241,200,0.95),stop:0.75 rgba(255,224,130,0.55),stop:1 transparent);}"));
        shim->hide();
        wl->addWidget(shim);
        wl->addWidget(liftLayer);
        return std::make_tuple(wrap, l1, l2, liftLay, shim);
    };
    const auto sigPack = mkAiTierSlot(0, QStringLiteral("SIGNAL"), true);
    QWidget *wrapSignal = std::get<0>(sigPack);
    QLabel *lblAiSig1 = std::get<1>(sigPack);
    QLabel *lblAiSig2 = std::get<2>(sigPack);
    QVBoxLayout *liftSignal = std::get<3>(sigPack);
    QFrame *shimSignal = std::get<4>(sigPack);
    const auto cflPack = mkAiTierSlot(1, QStringLiteral("CONFLICT"), false);
    QWidget *wrapConflict = std::get<0>(cflPack);
    QLabel *lblAiCfl1 = std::get<1>(cflPack);
    QLabel *lblAiCfl2 = std::get<2>(cflPack);
    QVBoxLayout *liftConflict = std::get<3>(cflPack);
    QFrame *shimConflict = std::get<4>(cflPack);
    lblAiCfl2->setVisible(false);
    const auto decPack = mkAiTierSlot(2, QStringLiteral("DECISION"), false);
    QWidget *wrapDecision = std::get<0>(decPack);
    QLabel *lblAiDec1 = std::get<1>(decPack);
    QLabel *lblAiDec2 = std::get<2>(decPack);
    QVBoxLayout *liftDecision = std::get<3>(decPack);
    QFrame *shimDecision = std::get<4>(decPack);
    lblAiDec2->setVisible(false);
    lblAiDec1->setTextFormat(Qt::RichText);
    const auto impPack = mkAiTierSlot(3, QStringLiteral("IMPACT"), false);
    QWidget *wrapImpact = std::get<0>(impPack);
    QLabel *lblAiImp1 = std::get<1>(impPack);
    QLabel *lblAiImp2 = std::get<2>(impPack);
    QVBoxLayout *liftImpact = std::get<3>(impPack);
    QFrame *shimImpact = std::get<4>(impPack);
    lblAiImp2->setVisible(false);

    auto *sourceStrip = new QFrame;
    sourceStrip->setObjectName(QStringLiteral("costSourceStrip"));
    sourceStrip->setVisible(false);
    sourceStrip->setStyleSheet(QStringLiteral(
        "QFrame#costSourceStrip{background:transparent;border:none;border-radius:0;}"));
    auto *srcLay = new QVBoxLayout(sourceStrip);
    srcLay->setContentsMargins(0, 8, 0, 0);
    srcLay->setSpacing(8);
    auto *lblDataSource = new QLabel;
    lblDataSource->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblDataSource->setAlignment(Qt::AlignCenter);
    lblDataSource->setWordWrap(true);
    lblDataSource->setStyleSheet(QStringLiteral(
        "QLabel{font-size:11px;font-weight:600;color:#777777;padding:0 2px;margin:0;"
        "background:transparent;border:none;}"));
    srcLay->addWidget(lblDataSource);
    auto *sourceChipRow = new QWidget;
    sourceChipRow->setVisible(false);
    auto *chipLay = new QHBoxLayout(sourceChipRow);
    chipLay->setContentsMargins(0, 0, 0, 0);
    chipLay->setSpacing(8);
    auto mkSourceChip = [](const QString &txt) {
        auto *lb = new QLabel(txt);
        lb->setAlignment(Qt::AlignCenter);
        lb->setStyleSheet(QStringLiteral(
            "QLabel{font-size:10px;font-weight:800;color:#FF9500;background:rgba(255,149,0,0.10);"
            "border:1px solid rgba(255,149,0,0.22);border-radius:6px;padding:4px 12px;}"));
        return lb;
    };
    auto *lblSourceChipLocal = mkSourceChip(QStringLiteral("Moteur IA local"));
    auto *lblSourceChipTech = mkSourceChip(QStringLiteral("C++ intégré · temps réel"));
    chipLay->addStretch(1);
    chipLay->addWidget(lblSourceChipLocal);
    chipLay->addWidget(lblSourceChipTech);
    chipLay->addStretch(1);
    srcLay->addWidget(sourceChipRow);

    auto mkKpi = [](const QString &titleCaps, bool withStability, int valueMinWidth, const QString &numColor,
                    const QString &stabilityCaption) {
        auto *box = new QFrame;
        box->setMinimumWidth(qMax(100, valueMinWidth + 24));
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        box->setStyleSheet(QStringLiteral(
            "QFrame{"
            "background:#161616;"
            "border:1px solid #2A2A2A;border-radius:8px;}"));
        auto *boxSh = new QGraphicsDropShadowEffect(box);
        boxSh->setBlurRadius(18);
        boxSh->setOffset(0, 4);
        boxSh->setColor(QColor(0, 0, 0, 55));
        box->setGraphicsEffect(boxSh);
        auto *vl = new QVBoxLayout(box);
        vl->setContentsMargins(16, 14, 16, 14);
        vl->setSpacing(5);
        auto *t = new QLabel(titleCaps);
        t->setAttribute(Qt::WA_TransparentForMouseEvents);
        t->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        t->setStyleSheet(QStringLiteral(
            "QLabel{color:#888888;font-size:10px;font-weight:800;letter-spacing:1.1px;background:transparent;"
            "border:none;padding:0;margin:0;}"));
        auto *num = new QLabel(QStringLiteral("—"));
        num->setAttribute(Qt::WA_TransparentForMouseEvents);
        num->setWordWrap(false);
        num->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        num->setMinimumWidth(valueMinWidth);
        num->setMinimumHeight(44);
        num->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        num->setStyleSheet(QStringLiteral(
            "QLabel{color:%2;font-size:34px;font-weight:800;padding:0;margin:0;"
            "background:transparent;min-width:%1px;}").arg(valueMinWidth).arg(numColor));
        auto *delta = new QLabel;
        delta->setAttribute(Qt::WA_TransparentForMouseEvents);
        delta->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        delta->setMinimumHeight(16);
        delta->setStyleSheet(QStringLiteral("font-size:11px;font-weight:700;color:#888888;"));
        vl->addWidget(t, 0, Qt::AlignLeft);
        vl->addWidget(num, 0, Qt::AlignLeft);
        vl->addWidget(delta, 0, Qt::AlignLeft);
        QProgressBar *stab = nullptr;
        if (withStability) {
            auto *sl = new QLabel(stabilityCaption.isEmpty() ? QStringLiteral("Stabilité") : stabilityCaption);
            sl->setAttribute(Qt::WA_TransparentForMouseEvents);
            sl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            sl->setStyleSheet(QStringLiteral("font-size:9px;color:#666666;font-weight:800;"));
            stab = new QProgressBar;
            stab->setAttribute(Qt::WA_TransparentForMouseEvents);
            stab->setRange(0, 100);
            stab->setFixedHeight(3);
            stab->setTextVisible(false);
            stab->setStyleSheet(QStringLiteral(
                "QProgressBar{border:none;border-radius:2px;background:rgba(255,255,255,0.06);}"
                "QProgressBar::chunk{border-radius:2px;background:#4a9060;}"));
            vl->addWidget(sl);
            vl->addWidget(stab);
        }
        return std::make_tuple(box, num, delta, stab, boxSh);
    };

    const auto kpiPackC = mkKpi(QStringLiteral("COÛT TOTAL"), false, 200, QStringLiteral("#F5F5F5"), QString());
    const auto kpiPackM = mkKpi(QStringLiteral("MARGE"), false, 140, QStringLiteral("#FF9500"), QString());
    const auto kpiPackR = mkKpi(QStringLiteral("RISQUE"), true, 130, QStringLiteral("#66BB6A"), QString());
    const auto kpiPackS = mkKpi(QStringLiteral("SEUIL RENTABILITÉ"), true, 100, QStringLiteral("#F5F5F5"),
                                QStringLiteral("Seuil volume"));
    QFrame *frC = std::get<0>(kpiPackC);
    QLabel *kpiCNum = std::get<1>(kpiPackC);
    QLabel *kpiCDelta = std::get<2>(kpiPackC);
    QFrame *frM = std::get<0>(kpiPackM);
    QLabel *kpiMNum = std::get<1>(kpiPackM);
    QLabel *kpiMDelta = std::get<2>(kpiPackM);
    QFrame *frR = std::get<0>(kpiPackR);
    QLabel *kpiRNum = std::get<1>(kpiPackR);
    QLabel *kpiRDelta = std::get<2>(kpiPackR);
    QProgressBar *stabBar = std::get<3>(kpiPackR);
    QGraphicsDropShadowEffect *frRShadow = std::get<4>(kpiPackR);
    QFrame *frS = std::get<0>(kpiPackS);
    QLabel *kpiSNum = std::get<1>(kpiPackS);
    QLabel *kpiSDelta = std::get<2>(kpiPackS);
    QProgressBar *stabSeuilBar = std::get<3>(kpiPackS);
    QGraphicsDropShadowEffect *frSShadow = std::get<4>(kpiPackS);

    auto *cmpFrame = new QFrame;
    cmpFrame->setParent(resultCard);
    cmpFrame->hide();
    cmpFrame->setStyleSheet(QStringLiteral(
        "QFrame{background:#141414;border-radius:8px;border:1px solid #2A2A2A;}"));
    auto *cmpLay = new QVBoxLayout(cmpFrame);
    cmpLay->setContentsMargins(14, 12, 14, 12);
    cmpLay->setSpacing(8);
    auto *cmpTitle = new QLabel(QStringLiteral("ACTUEL  →  OPTIMISÉ"));
    cmpTitle->setStyleSheet(QStringLiteral(
        "color:#888888;font-size:9px;font-weight:900;letter-spacing:1.6px;"));
    cmpLay->addWidget(cmpTitle);
    auto *lblCmpCost = new QLabel;
    auto *lblCmpMarg = new QLabel;
    auto *lblCmpRisk = new QLabel;
    lblCmpCost->setWordWrap(true);
    lblCmpMarg->setWordWrap(true);
    lblCmpRisk->setWordWrap(true);
    lblCmpCost->setStyleSheet(QStringLiteral("font-size:12px;font-weight:700;color:#E8E8E8;line-height:1.4;background:transparent;"));
    lblCmpMarg->setStyleSheet(QStringLiteral("font-size:12px;font-weight:700;color:#E8E8E8;line-height:1.4;background:transparent;"));
    lblCmpRisk->setStyleSheet(QStringLiteral("font-size:12px;font-weight:700;color:#E8E8E8;line-height:1.4;background:transparent;"));
    cmpLay->addWidget(lblCmpCost);
    cmpLay->addWidget(lblCmpMarg);
    cmpLay->addWidget(lblCmpRisk);

    auto *tagRow = new QHBoxLayout;
    tagRow->setSpacing(8);
    QLabel *tagLabels[4];
    const QStringList tagTexts = {QStringLiteral("Surcoté"), QStringLiteral("Montée en charge"),
                                  QStringLiteral("Risque élevé"), QStringLiteral("Levier marge")};
    for (int i = 0; i < 4; ++i) {
        auto *tg = new QLabel(tagTexts.at(i));
        tg->setAttribute(Qt::WA_TransparentForMouseEvents);
        tg->setAlignment(Qt::AlignCenter);
        tg->setVisible(false);
        tg->setStyleSheet(QStringLiteral(
            "font-size:10px;font-weight:800;color:#faf7f2;background:rgba(255,255,255,0.12);border-radius:10px;"
            "padding:6px 10px;border:none;"));
        tagLabels[i] = tg;
        tagRow->addWidget(tg);
    }
    tagRow->addStretch(1);

    auto *scoreRow = new QHBoxLayout;
    auto *scoreLbl = new QLabel(QStringLiteral("Score produit"));
    scoreLbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    scoreLbl->setStyleSheet(QStringLiteral("color:#888888;font-weight:900;font-size:9px;letter-spacing:1px;"));
    auto *scoreBar = new QProgressBar;
    scoreBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    scoreBar->setRange(0, 100);
    scoreBar->setValue(0);
    scoreBar->setTextVisible(true);
    scoreBar->setFormat(QStringLiteral("%v"));
    scoreBar->setStyleSheet(QStringLiteral(
        "QProgressBar{border:none;border-radius:6px;height:20px;text-align:center;background:rgba(255,255,255,0.06);}"
        "QProgressBar::chunk{background:#FF9500;border-radius:6px;}"));
    auto *scoreNum = new QLabel(QStringLiteral("0"));
    scoreNum->setAttribute(Qt::WA_TransparentForMouseEvents);
    scoreNum->setStyleSheet(QStringLiteral("font-size:26px;font-weight:900;color:#F5F5F5;min-width:48px;"));
    scoreRow->addWidget(scoreLbl);
    scoreRow->addWidget(scoreBar, 1);
    scoreRow->addWidget(scoreNum);
    auto *scoreWrap = new QFrame;
    scoreWrap->setStyleSheet(QStringLiteral(
        "QFrame{background:#161616;border:1px solid #2A2A2A;border-radius:8px;}"));
    auto *scoreSh = new QGraphicsDropShadowEffect(scoreWrap);
    scoreSh->setBlurRadius(24);
    scoreSh->setOffset(0, 6);
    scoreSh->setColor(QColor(0, 0, 0, 12));
    scoreWrap->setGraphicsEffect(scoreSh);
    auto *scoreWrapLay = new QVBoxLayout(scoreWrap);
    scoreWrapLay->setContentsMargins(16, 14, 16, 14);
    scoreWrapLay->setSpacing(8);
    scoreWrapLay->addLayout(scoreRow);

    auto *lblAdv = new QLabel;
    lblAdv->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblAdv->setVisible(false);
    lblAdv->setWordWrap(true);
    lblAdv->setStyleSheet(QStringLiteral("font-size:10px;color:#888888;font-weight:600;background:transparent;border:none;"));

    auto *hdrStrip = new QFrame;
    hdrStrip->setObjectName(QStringLiteral("costAiHeaderStrip"));
    hdrStrip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hdrStrip->setStyleSheet(QStringLiteral(
        "QFrame#costAiHeaderStrip{"
        "background:rgba(255,149,0,0.08);"
        "border:none;"
        "border-radius:8px;"
        "border:1px solid rgba(255,149,0,0.28);}"));
    auto *hStripLay = new QHBoxLayout(hdrStrip);
    hStripLay->setContentsMargins(18, 13, 18, 13);
    hStripLay->setSpacing(14);
    auto *dotHost = new QWidget;
    dotHost->setFixedSize(30, 30);
    dotHost->setStyleSheet(QStringLiteral("background:transparent;border:none;"));
    auto *lblHdrGlowDot = new QLabel(QStringLiteral("●"), dotHost);
    lblHdrGlowDot->setGeometry(0, 0, 30, 30);
    lblHdrGlowDot->setAlignment(Qt::AlignCenter);
    lblHdrGlowDot->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblHdrGlowDot->setStyleSheet(QStringLiteral(
        "QLabel{color:#FF9100;font-size:18px;font-weight:900;background:transparent;border:none;}"));
    auto *hdrDotGlow = new QGraphicsDropShadowEffect(lblHdrGlowDot);
    hdrDotGlow->setBlurRadius(20);
    hdrDotGlow->setColor(QColor(255, 171, 64, 240));
    hdrDotGlow->setOffset(0, 0);
    lblHdrGlowDot->setGraphicsEffect(hdrDotGlow);
    auto *lblHdrTitle = new QLabel(QStringLiteral("AI ENGINE — ANALYSE LIVE"));
    lblHdrTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblHdrTitle->setStyleSheet(QStringLiteral(
        "QLabel{font-size:13px;font-weight:900;letter-spacing:1.2px;color:#F0F0F0;"
        "background:transparent;border:none;}"));
    hStripLay->addWidget(dotHost, 0, Qt::AlignVCenter);
    hStripLay->addWidget(lblHdrTitle, 0, Qt::AlignVCenter);
    hStripLay->addStretch(1);

    auto *hdrGlowLine = new QFrame;
    hdrGlowLine->setFixedHeight(2);
    hdrGlowLine->setStyleSheet(QStringLiteral(
        "QFrame{border:none;border-radius:1px;"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 transparent,stop:0.15 rgba(255,213,79,0.35),stop:0.5 rgba(255,224,130,0.95),"
        "stop:0.85 rgba(255,213,79,0.35),stop:1 transparent);}"));

    auto *hdrSweepHost = new QWidget;
    hdrSweepHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *hdrSweepLay = new QHBoxLayout(hdrSweepHost);
    hdrSweepLay->setContentsMargins(0, 2, 0, 4);
    hdrSweepLay->setSpacing(0);
    auto *hdrSweepBar = new QFrame;
    hdrSweepBar->setObjectName(QStringLiteral("costHdrSweepBar"));
    hdrSweepBar->setFixedHeight(3);
    hdrSweepBar->setFixedWidth(72);
    hdrSweepBar->setStyleSheet(QStringLiteral(
        "QFrame#costHdrSweepBar{background:rgba(255,248,210,0.92);border:none;border-radius:2px;}"));
    hdrSweepLay->addStretch(1);
    hdrSweepLay->addWidget(hdrSweepBar, 0, Qt::AlignHCenter);
    hdrSweepLay->addStretch(1);

    auto *decisionCore = new QFrame;
    decisionCore->setObjectName(QStringLiteral("costDecisionCore"));
    decisionCore->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    decisionCore->setStyleSheet(QStringLiteral(
        "QFrame#costDecisionCore{"
        "background:#0E0E0E;"
        "border:1px solid #2A2A2A;border-radius:10px;}"));
    auto *dcSh = new QGraphicsDropShadowEffect(decisionCore);
    dcSh->setBlurRadius(22);
    dcSh->setOffset(0, 8);
    dcSh->setColor(QColor(0, 0, 0, 70));
    decisionCore->setGraphicsEffect(dcSh);
    auto *dcLay = new QVBoxLayout(decisionCore);
    dcLay->setContentsMargins(20, 12, 20, 20);
    dcLay->setSpacing(16);
    dcLay->addWidget(hdrStrip);
    dcLay->addWidget(hdrGlowLine);
    dcLay->addWidget(hdrSweepHost);
    auto *dcTop = new QHBoxLayout;
    dcTop->setSpacing(12);
    dcTop->setContentsMargins(0, 0, 0, 0);
    dcTop->addWidget(lblBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    dcTop->addStretch(1);
    dcTop->addWidget(confWrap, 0, Qt::AlignRight | Qt::AlignTop);
    dcLay->addLayout(dcTop);
    dcLay->addWidget(aiStatusRow);
    dcLay->addWidget(wrapSignal);
    dcLay->addWidget(wrapConflict);
    dcLay->addWidget(wrapDecision);
    dcLay->addWidget(wrapImpact);
    dcLay->addWidget(scoreWrap);

    auto *scenarioDock = new QFrame;
    scenarioDock->setStyleSheet(QStringLiteral(
        "QFrame{background:#1a1712;border:1px solid #3a3428;border-radius:8px;}"));
    auto *scenOuter = new QVBoxLayout(scenarioDock);
    scenOuter->setContentsMargins(14, 14, 14, 14);
    scenOuter->setSpacing(10);
    auto *scenRow = new QHBoxLayout;
    scenRow->setSpacing(10);
    scenRow->setContentsMargins(0, 0, 0, 0);
    CostClickFrame *scCards[3];
    QLabel *scVals[3];
    const QStringList scenTitles = {QStringLiteral("OPTIMISTE"), QStringLiteral("RÉALISTE"),
                                    QStringLiteral("PESSIMISTE")};
    const QStringList scenBorder = {QStringLiteral("#2a4a30"), QStringLiteral("#3a3428"),
                                    QStringLiteral("#4a2020")};
    const int scenProb[3] = {25, 55, 20};
    for (int i = 0; i < 3; ++i) {
        auto *c = new CostClickFrame;
        c->setObjectName(QStringLiteral("costScenCard"));
        c->setProperty("scenIdx", i);
        c->setCursor(Qt::PointingHandCursor);
        scCards[i] = c;
        auto *vl = new QVBoxLayout(c);
        vl->setContentsMargins(12, 10, 12, 10);
        vl->setSpacing(6);
        auto *lt = new QLabel(scenTitles.at(i));
        lt->setStyleSheet(QStringLiteral(
            "QLabel{font-size:9px;font-weight:900;letter-spacing:1.4px;color:#888888;background:transparent;border:none;}"));
        auto *vv = new QLabel(QStringLiteral("— TND"));
        vv->setStyleSheet(QStringLiteral(
            "QLabel{font-size:14px;font-weight:800;color:#F0F0F0;background:transparent;border:none;"
            "font-variant-numeric:tabular-nums;}"));
        scVals[i] = vv;
        auto *pr = new QLabel(QStringLiteral("prob. %1%").arg(scenProb[i]));
        pr->setStyleSheet(QStringLiteral(
            "QLabel{font-size:10px;font-weight:600;color:#777777;background:transparent;border:none;}"));
        vl->addWidget(lt);
        vl->addWidget(vv);
        vl->addWidget(pr);
        scenRow->addWidget(c, 1);
    }
    scenOuter->addLayout(scenRow);

    auto *wfSection = new CostWaterfallSection(decisionCore);

    auto *projHost = new QFrame;
    projHost->setStyleSheet(QStringLiteral(
        "QFrame{background:#1a1712;border:1px solid #3a3428;border-radius:8px;}"));
    auto *projVL = new QVBoxLayout(projHost);
    projVL->setContentsMargins(14, 14, 14, 14);
    projVL->setSpacing(8);
    auto *projTitle = new QLabel(QStringLiteral("PROJECTION 90J"));
    projTitle->setStyleSheet(QStringLiteral(
        "QLabel{font-size:9px;font-weight:900;letter-spacing:1.5px;color:#888888;background:transparent;border:none;}"));
    auto *projChart = new CostProjection90(projHost);
    projVL->addWidget(projTitle);
    projVL->addWidget(projChart);
    auto *legRow = new QHBoxLayout;
    legRow->setSpacing(16);
    legRow->setContentsMargins(0, 0, 0, 0);
    legRow->addStretch(1);
    auto mkProjLeg = [](const QString &col, const QString &txt) {
        auto *w = new QWidget;
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(6);
        auto *d = new QLabel(QStringLiteral("●"));
        d->setStyleSheet(QStringLiteral("QLabel{font-size:10px;color:%1;background:transparent;border:none;}").arg(col));
        auto *l = new QLabel(txt);
        l->setStyleSheet(QStringLiteral(
            "QLabel{font-size:9px;font-weight:700;color:#888888;background:transparent;border:none;}"));
        h->addWidget(d);
        h->addWidget(l);
        return w;
    };
    legRow->addWidget(mkProjLeg(QStringLiteral("#4CAF50"), QStringLiteral("Opt")));
    legRow->addWidget(mkProjLeg(QStringLiteral("#FF9500"), QStringLiteral("Réel")));
    legRow->addWidget(mkProjLeg(QStringLiteral("#E57373"), QStringLiteral("Pess")));
    legRow->addStretch(1);
    projVL->addLayout(legRow);

    dcLay->addWidget(scenarioDock);
    dcLay->addWidget(wfSection);
    dcLay->addWidget(projHost);
    dcLay->addWidget(lblAdv);
    dcLay->addLayout(tagRow);
    dcLay->addWidget(sourceStrip);

    auto *rvOuter = new QVBoxLayout(resultCard);
    rvOuter->setContentsMargins(0, 12, 0, 0);
    rvOuter->setSpacing(16);

    auto *kpiDock = new QWidget;
    kpiDock->setStyleSheet(QStringLiteral("background:transparent;"));
    auto *kpiDockLay = new QHBoxLayout(kpiDock);
    kpiDockLay->setContentsMargins(0, 0, 0, 0);
    kpiDockLay->setSpacing(12);
    kpiDockLay->addWidget(frC, 1);
    kpiDockLay->addWidget(frM, 1);
    kpiDockLay->addWidget(frR, 1);
    kpiDockLay->addWidget(frS, 1);
    rvOuter->addWidget(kpiDock);
    rvOuter->addWidget(decisionCore, 0);

    v->addWidget(resultCard);
    v->addStretch(1);

    auto crossfadeToResults = [=]() {
        if (!preAnalysisHost->isVisible()) {
            resultCard->setVisible(true);
            preAnalysisHost->hide();
            return;
        }
        preAnalysisHost->raise();
        resultCard->raise();
        auto *preFx = new QGraphicsOpacityEffect(preAnalysisHost);
        preAnalysisHost->setGraphicsEffect(preFx);
        preFx->setOpacity(1.0);
        resultCard->setVisible(true);
        auto *resFx = new QGraphicsOpacityEffect(resultCard);
        resultCard->setGraphicsEffect(resFx);
        resFx->setOpacity(0.0);
        auto *grp = new QParallelAnimationGroup(resultCard);
        auto *outA = new QPropertyAnimation(preFx, QByteArrayLiteral("opacity"), grp);
        outA->setDuration(280);
        outA->setStartValue(1.0);
        outA->setEndValue(0.0);
        outA->setEasingCurve(QEasingCurve::InOutQuad);
        auto *inA = new QPropertyAnimation(resFx, QByteArrayLiteral("opacity"), grp);
        inA->setDuration(340);
        inA->setStartValue(0.0);
        inA->setEndValue(1.0);
        inA->setEasingCurve(QEasingCurve::OutCubic);
        grp->addAnimation(outA);
        grp->addAnimation(inA);
        QObject::connect(grp, &QParallelAnimationGroup::finished, resultCard, [=]() {
            preAnalysisHost->hide();
            preAnalysisHost->setGraphicsEffect(nullptr);
            resultCard->setGraphicsEffect(nullptr);
        });
        grp->start(QAbstractAnimation::DeleteWhenStopped);
    };

    new CostShadowHoverLift(decisionCore, dcSh, this);
    new CostShadowHoverLift(preAnalysisCard, cardSh, this);
    new CostShadowHoverLift(frC, std::get<4>(kpiPackC), this);
    new CostShadowHoverLift(frM, std::get<4>(kpiPackM), this);
    new CostShadowHoverLift(frR, frRShadow, this);
    new CostShadowHoverLift(frS, frSShadow, this);
    new CostShadowHoverLift(scoreWrap, scoreSh, this);

    {
        auto *glowFwd = new QPropertyAnimation(hdrDotGlow, QByteArrayLiteral("blurRadius"), hdrStrip);
        glowFwd->setDuration(900);
        glowFwd->setStartValue(8);
        glowFwd->setEndValue(28);
        glowFwd->setEasingCurve(QEasingCurve::OutCubic);
        auto *glowBwd = new QPropertyAnimation(hdrDotGlow, QByteArrayLiteral("blurRadius"), hdrStrip);
        glowBwd->setDuration(900);
        glowBwd->setStartValue(28);
        glowBwd->setEndValue(8);
        glowBwd->setEasingCurve(QEasingCurve::InCubic);
        auto *glowGrp = new QSequentialAnimationGroup(hdrStrip);
        glowGrp->addAnimation(glowFwd);
        glowGrp->addAnimation(glowBwd);
        glowGrp->setLoopCount(-1);
        glowGrp->start();
    }

    scroll->setWidget(cw);
    rootLay->addWidget(scroll, 1);

    auto animState = std::make_shared<CostTabAnimState>();
    auto liveOutcome = std::make_shared<SimulationOutcome>();
    auto liveDin = std::make_shared<ProductAnalyzerInput>();
    auto liveDisp = std::make_shared<double>(0.0);
    auto liveHas = std::make_shared<bool>(false);
    auto hasManualAnalysis = std::make_shared<bool>(false);

    auto syncSensLabel = [lblSens](int v) {
        lblSens->setText(v > 0 ? QStringLiteral("+%1%").arg(v) : QStringLiteral("%1%").arg(v));
    };
    QObject::connect(sldSens, &QSlider::valueChanged, this, [syncSensLabel](int v) { syncSensLabel(v); });
    auto syncCostShiftLabel = [lblCostShift](int v) {
        lblCostShift->setText(v > 0 ? QStringLiteral("+%1%").arg(v) : QStringLiteral("%1%").arg(v));
    };
    QObject::connect(sldCostShift, &QSlider::valueChanged, this, [syncCostShiftLabel](int v) { syncCostShiftLabel(v); });

    auto buildDin = [=](double matiereTnd) {
        ProductAnalyzerInput din;
        din.nomProduit = leLib->text().trimmed().isEmpty() ? QStringLiteral("Produit") : leLib->text().trimmed();
        din.coutMatiereTnd = matiereTnd;
        din.volumeCible = sbVol->value();
        din.indiceCout = std::clamp(matiereTnd / 0.45, 8.0, 94.0);
        din.positionnement = din.indiceCout >= 62.0 ? QStringLiteral("premium")
                                                     : (din.indiceCout >= 38.0 ? QStringLiteral("aspirationnel")
                                                                               : QStringLiteral("accessible"));
        din.niveauFinition = din.indiceCout >= 68.0 ? QStringLiteral("élevé") : QStringLiteral("standard");
        din.delaiSemaines = sbVol->value() > 600 ? 4 : 7;
        din.risqueFournisseur = sbVol->value() > 800 ? QStringLiteral("élevé") : QStringLiteral("modéré");
        return din;
    };

    const auto applyScenarioChrome = [=]() {
        const int a = animState->scenarioPick;
        for (int i = 0; i < 3; ++i) {
            const bool on = (i == a);
            const QString bd = scenBorder.at(i);
            const QString bg = on ? QStringLiteral("#0a0806") : QStringLiteral("#14110e");
            const int bw = on ? 2 : 1;
            scCards[i]->setStyleSheet(QStringLiteral(
                "QFrame{background:%1;border:%2px solid %3;border-radius:8px;}").arg(bg).arg(bw).arg(bd));
        }
    };

    const auto refreshAuxPanels = [=]() {
        if (!*liveHas)
            return;
        const QLocale loc = QLocale::system();
        const SimulationOutcome &o = *liveOutcome;
        const ProductAnalyzerInput &din = *liveDin;
        const double disp = *liveDisp;
        const double simCost = o.act.coutUnitaireTnd;
        const double m0 = SensitivityEngine::materialWithDelta(sbMat->value(), static_cast<double>(sldSens->value()));
        double K = 1.0;
        if (m0 > 1e-9)
            K = (simCost - 4.5) / m0;
        K = std::max(K, 0.08);
        const double cOpt = m0 * 0.85 * K + 4.5;
        const double cReal = simCost;
        const double cPess = m0 * 1.25 * K + 4.5;
        const double flat = static_cast<double>(sldCostShift->value()) * 0.4;
        scVals[0]->setText(loc.toString(cOpt, 'f', 2) + QStringLiteral(" TND"));
        scVals[1]->setText(loc.toString(cReal + flat, 'f', 2) + QStringLiteral(" TND"));
        scVals[2]->setText(loc.toString(cPess, 'f', 2) + QStringLiteral(" TND"));
        applyScenarioChrome();
        double wm = 0, wmo = 0, wc = 0, wl = 0, wr = 0;
        costDecomposeFive(din, disp, wm, wmo, wc, wl, wr);
        QVector<double> parts({wm, wmo, wc, wl, wr});
        QVector<QColor> cols({QColor(QStringLiteral("#d4841a")), QColor(QStringLiteral("#a06818")),
                              QColor(QStringLiteral("#785010")), QColor(QStringLiteral("#5a3c0c")),
                              QColor(QStringLiteral("#c04040"))});
        wfSection->updateParts(parts, cols, loc);
        const QString projHashKey = QString::number(sbMat->value(), 'f', 2) + QLatin1Char('|')
            + QString::number(sbVol->value()) + QLatin1Char('|') + QString::number(sldSens->value());
        const quint32 seed = animState->projectionSeed ^ quint32(qHash(projHashKey));
        projChart->rebuild(disp, seed ? seed : 1u);
    };

    for (int i = 0; i < 3; ++i) {
        const int idx = i;
        scCards[i]->onPressed = [=]() {
            animState->scenarioPick = idx;
            applyScenarioChrome();
        };
    }
    animState->scenarioPick = 1;
    applyScenarioChrome();

    animState->projectionSeed = animState->projectionSeed + 17u;

    auto setDeltaLabel = [](QLabel *lab, double delta, bool costMetric) {
        if (std::abs(delta) < 1e-7) {
            lab->setText(QString());
            return;
        }
        const bool good = costMetric ? (delta < 0) : (delta > 0);
        const QString arrow = delta > 0 ? QStringLiteral("▲ ") : QStringLiteral("▼ ");
        lab->setStyleSheet(good ? QStringLiteral("font-size:11px;font-weight:800;color:#1B5E20;")
                                : QStringLiteral("font-size:11px;font-weight:800;color:#B71C1C;"));
        if (costMetric)
            lab->setText(QStringLiteral("%1%2 TND").arg(arrow, QLocale::system().toString(std::abs(delta), 'f', 2)));
        else
            lab->setText(QStringLiteral("%1%2%").arg(arrow, QLocale::system().toString(std::abs(delta), 'f', 1)));
    };
    auto animateValueColor = [](QLabel *lb, const QColor &from, const QColor &to, int ms, int minW) {
        auto *anim = new QVariantAnimation(lb);
        anim->setDuration(ms);
        anim->setStartValue(from);
        anim->setEndValue(to);
        QObject::connect(anim, &QVariantAnimation::valueChanged, lb, [lb, minW](const QVariant &v) {
            const QColor c = v.value<QColor>();
            lb->setStyleSheet(QStringLiteral(
                                   "QLabel{color:%1;font-size:34px;font-weight:800;min-width:%2px;padding:0;margin:0;"
                                   "background:transparent;}")
                                   .arg(c.name())
                                   .arg(minW));
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    auto setTag = [](QLabel *lb, bool on, const QString &tintBg, const QString &textColor) {
        lb->setVisible(on);
        if (!on)
            return;
        lb->setStyleSheet(QStringLiteral(
            "font-size:10px;font-weight:800;color:%1;background:%2;border:none;border-radius:10px;padding:6px 10px;")
            .arg(textColor, tintBg));
    };

    auto animateConfidence = [=](int targetPct) {
        const int t = qBound(0, targetPct, 100);
        confProgressBar->setRange(0, 100);
        auto *vAnim = new QVariantAnimation(lblConf);
        vAnim->setDuration(520);
        vAnim->setStartValue(0);
        vAnim->setEndValue(t);
        vAnim->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(vAnim, &QVariantAnimation::valueChanged, lblConf, [=](const QVariant &val) {
            const int n = qBound(0, qRound(val.toDouble()), 100);
            lblConf->setText(QStringLiteral("Confiance %1%").arg(n));
            confProgressBar->setValue(n);
        });
        vAnim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    const QString kPreReadyIdleText = QStringLiteral("Système prêt • en attente d’analyse");
    const QString kPreReadyIdleStyle = QStringLiteral(
        "QLabel{font-size:11px;font-weight:700;letter-spacing:0.4px;color:#8D6E63;background:transparent;border:none;}");

    const auto stopAiRunningPulse = [=]() {
        aiStatusRow->setVisible(false);
        lblPreReady->setText(kPreReadyIdleText);
        lblPreReady->setStyleSheet(kPreReadyIdleStyle);
        lblPreReady->setGraphicsEffect(nullptr);
        if (animState->aiPulseAnim) {
            animState->aiPulseAnim->stop();
            animState->aiPulseAnim->deleteLater();
            animState->aiPulseAnim = nullptr;
        }
        lblAiPulseDot->setGraphicsEffect(nullptr);
        if (animState->tierShimmerAnim) {
            animState->tierShimmerAnim->stop();
            animState->tierShimmerAnim->deleteLater();
            animState->tierShimmerAnim = nullptr;
        }
        for (QFrame *sh : {shimSignal, shimConflict, shimDecision, shimImpact}) {
            if (!sh)
                continue;
            sh->hide();
            sh->setGraphicsEffect(nullptr);
        }
        if (animState->hdrStripBreathTimer) {
            animState->hdrStripBreathTimer->stop();
            animState->hdrStripBreathTimer->deleteLater();
            animState->hdrStripBreathTimer.clear();
        }
        if (animState->hdrSweepAnim) {
            animState->hdrSweepAnim->stop();
            animState->hdrSweepAnim->deleteLater();
            animState->hdrSweepAnim = nullptr;
        }
        hdrSweepBar->setGraphicsEffect(nullptr);
    };

    const auto startAiRunningPulse = [=]() {
        stopAiRunningPulse();
        if (resultCard->isVisible()) {
            aiStatusRow->setVisible(true);
            auto *fx = new QGraphicsOpacityEffect(lblAiPulseDot);
            lblAiPulseDot->setGraphicsEffect(fx);
            fx->setOpacity(1.0);
            auto *pa = new QPropertyAnimation(fx, QByteArrayLiteral("opacity"), lblAiPulseDot);
            pa->setDuration(650);
            pa->setStartValue(0.35);
            pa->setEndValue(1.0);
            pa->setEasingCurve(QEasingCurve::InOutSine);
            pa->setLoopCount(-1);
            animState->aiPulseAnim = pa;
            pa->start();
        } else {
            lblPreReady->setText(QStringLiteral("●  Analyse en cours"));
            lblPreReady->setStyleSheet(QStringLiteral(
                "QLabel{font-size:11px;font-weight:800;letter-spacing:0.35px;color:#B45309;background:transparent;"
                "border:none;}"));
            auto *fx = new QGraphicsOpacityEffect(lblPreReady);
            lblPreReady->setGraphicsEffect(fx);
            fx->setOpacity(1.0);
            auto *pa = new QPropertyAnimation(fx, QByteArrayLiteral("opacity"), lblPreReady);
            pa->setDuration(700);
            pa->setStartValue(0.45);
            pa->setEndValue(1.0);
            pa->setEasingCurve(QEasingCurve::InOutSine);
            pa->setLoopCount(-1);
            animState->aiPulseAnim = pa;
            pa->start();
        }
    };

    auto faceFromLift = [](QVBoxLayout *liftLy) -> QWidget * {
        return liftLy ? liftLy->parentWidget() : nullptr;
    };

    const auto staggerRevealAiBlocks = [=]() {
        const int gen = ++animState->aiBlockRevealGen;
        constexpr int kSlidePx = 12;
        auto prep = [kSlidePx](QWidget *face, QVBoxLayout *lift) {
            if (face && face->graphicsEffect())
                face->setGraphicsEffect(nullptr);
            if (lift)
                lift->setContentsMargins(0, kSlidePx, 0, 0);
        };
        prep(faceFromLift(liftSignal), liftSignal);
        prep(faceFromLift(liftConflict), liftConflict);
        prep(faceFromLift(liftDecision), liftDecision);
        prep(faceFromLift(liftImpact), liftImpact);
        auto runReveal = [=](QWidget *face, QVBoxLayout *lift, int delayMs, bool pulseDecisionShadow) {
            if (!face)
                return;
            QTimer::singleShot(delayMs, this, [=]() {
                if (gen != animState->aiBlockRevealGen)
                    return;
                auto *fx = new QGraphicsOpacityEffect(face);
                face->setGraphicsEffect(fx);
                fx->setOpacity(0.0);
                auto *combo = new QVariantAnimation(face);
                combo->setDuration(340);
                combo->setStartValue(0.0);
                combo->setEndValue(1.0);
                combo->setEasingCurve(QEasingCurve::OutCubic);
                QObject::connect(combo, &QVariantAnimation::valueChanged, face, [=](const QVariant &val) {
                    const double t = qBound(0.0, val.toDouble(), 1.0);
                    fx->setOpacity(t);
                    if (lift) {
                        const int top = qRound(static_cast<double>(kSlidePx) * (1.0 - t));
                        lift->setContentsMargins(0, top, 0, 0);
                    }
                });
                QObject::connect(combo, &QVariantAnimation::finished, face, [=]() {
                    if (lift)
                        lift->setContentsMargins(0, 0, 0, 0);
                    if (qobject_cast<QGraphicsOpacityEffect *>(face->graphicsEffect()))
                        face->setGraphicsEffect(nullptr);
                    if (pulseDecisionShadow) {
                        QFrame *tc = face->findChild<QFrame *>(QStringLiteral("costAiTierCard"));
                        if (!tc)
                            return;
                        auto *sh = qobject_cast<QGraphicsDropShadowEffect *>(tc->graphicsEffect());
                        if (!sh)
                            return;
                        const qreal b0 = sh->blurRadius();
                        const QColor c0 = sh->color();
                        auto *up = new QPropertyAnimation(sh, QByteArrayLiteral("blurRadius"), tc);
                        up->setDuration(140);
                        up->setStartValue(b0);
                        up->setEndValue(qMin(52.0, b0 + 22.0));
                        auto *hold = new QPauseAnimation(tc);
                        hold->setDuration(180);
                        auto *down = new QPropertyAnimation(sh, QByteArrayLiteral("blurRadius"), tc);
                        down->setDuration(320);
                        down->setStartValue(qMin(52.0, b0 + 22.0));
                        down->setEndValue(b0);
                        auto *grp = new QSequentialAnimationGroup(tc);
                        grp->addAnimation(up);
                        grp->addAnimation(hold);
                        grp->addAnimation(down);
                        grp->start(QAbstractAnimation::DeleteWhenStopped);
                        auto *glowPulse = new QVariantAnimation(tc);
                        glowPulse->setDuration(520);
                        glowPulse->setStartValue(0.0);
                        glowPulse->setEndValue(1.0);
                        glowPulse->setEasingCurve(QEasingCurve::OutCubic);
                        QObject::connect(glowPulse, &QVariantAnimation::valueChanged, tc, [=](const QVariant &v) {
                            const double u = v.toDouble();
                            const int a = int(70 + 110 * std::sin(u * 3.141592653589793));
                            sh->setColor(QColor(255, 224, 130, qBound(40, a, 180)));
                        });
                        QObject::connect(glowPulse, &QVariantAnimation::finished, tc, [=]() { sh->setColor(c0); });
                        glowPulse->start(QAbstractAnimation::DeleteWhenStopped);
                        auto *fs = new QVariantAnimation(lblAiDec1);
                        fs->setDuration(260);
                        fs->setStartValue(20.5);
                        fs->setEndValue(19.0);
                        fs->setEasingCurve(QEasingCurve::OutCubic);
                        QObject::connect(fs, &QVariantAnimation::valueChanged, lblAiDec1, [=](const QVariant &sz) {
                            const int px = qRound(sz.toDouble());
                            lblAiDec1->setStyleSheet(QStringLiteral(
                                "QLabel{font-size:%1px;font-weight:800;color:#FFFDE7;background:transparent;border:none;"
                                "line-height:1.22;}")
                                                         .arg(px));
                        });
                        QObject::connect(fs, &QVariantAnimation::finished, lblAiDec1, [=]() {
                            lblAiDec1->setStyleSheet(QStringLiteral(
                                "QLabel{font-size:19px;font-weight:800;color:#FFFDE7;background:transparent;border:none;"
                                "line-height:1.22;"));
                        });
                        fs->start(QAbstractAnimation::DeleteWhenStopped);
                    }
                });
                combo->start(QAbstractAnimation::DeleteWhenStopped);
            });
        };
        constexpr int step = 228;
        runReveal(faceFromLift(liftSignal), liftSignal, 0, false);
        runReveal(faceFromLift(liftConflict), liftConflict, step, false);
        runReveal(faceFromLift(liftDecision), liftDecision, step * 2, true);
        runReveal(faceFromLift(liftImpact), liftImpact, step * 3, false);
    };

    const auto dimAiBlocksPending = [=]() {
        ++animState->aiBlockRevealGen;
        const auto ghostFace = [](QWidget *face, QVBoxLayout *lift) {
            if (!face)
                return;
            if (face->graphicsEffect())
                face->setGraphicsEffect(nullptr);
            auto *fx = new QGraphicsOpacityEffect(face);
            face->setGraphicsEffect(fx);
            fx->setOpacity(0.0);
            if (lift)
                lift->setContentsMargins(0, 0, 0, 0);
        };
        ghostFace(faceFromLift(liftSignal), liftSignal);
        ghostFace(faceFromLift(liftConflict), liftConflict);
        ghostFace(faceFromLift(liftDecision), liftDecision);
        ghostFace(faceFromLift(liftImpact), liftImpact);
    };

    const auto stopTierShimmers = [=]() {
        if (animState->tierShimmerAnim) {
            animState->tierShimmerAnim->stop();
            animState->tierShimmerAnim->deleteLater();
            animState->tierShimmerAnim = nullptr;
        }
        for (QFrame *sh : {shimSignal, shimConflict, shimDecision, shimImpact}) {
            if (!sh)
                continue;
            sh->hide();
            sh->setGraphicsEffect(nullptr);
        }
    };

    const auto startTierShimmers = [=]() {
        stopTierShimmers();
        auto *grp = new QParallelAnimationGroup(decisionCore);
        for (QFrame *sh : {shimSignal, shimConflict, shimDecision, shimImpact}) {
            if (!sh)
                continue;
            sh->show();
            auto *fx = new QGraphicsOpacityEffect(sh);
            sh->setGraphicsEffect(fx);
            fx->setOpacity(0.28);
            auto *a = new QPropertyAnimation(fx, QByteArrayLiteral("opacity"), sh);
            a->setDuration(920);
            a->setStartValue(0.22);
            a->setEndValue(0.82);
            a->setEasingCurve(QEasingCurve::InOutSine);
            a->setLoopCount(-1);
            grp->addAnimation(a);
        }
        animState->tierShimmerAnim = grp;
        grp->start();
    };

    const auto stopHeaderPremiumFx = [=]() {
        if (animState->hdrStripBreathTimer) {
            animState->hdrStripBreathTimer->stop();
            animState->hdrStripBreathTimer->deleteLater();
            animState->hdrStripBreathTimer.clear();
        }
        if (animState->hdrSweepAnim) {
            animState->hdrSweepAnim->stop();
            animState->hdrSweepAnim->deleteLater();
            animState->hdrSweepAnim = nullptr;
        }
        hdrSweepBar->setGraphicsEffect(nullptr);
    };

    const auto startHeaderPremiumFx = [=]() {
        stopHeaderPremiumFx();
        const QString hdrBase = QStringLiteral(
            "QFrame#costAiHeaderStrip{"
            "background:rgba(255,149,0,0.08);"
            "border:none;"
            "border-radius:8px;"
            "border:1px solid rgba(255,149,0,0.28);}");
        const QString hdrAlt = QStringLiteral(
            "QFrame#costAiHeaderStrip{"
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,149,0,0.06),stop:0.5 rgba(255,149,0,0.16),stop:1 rgba(255,149,0,0.08));"
            "border:none;"
            "border-radius:8px;"
            "border:1px solid rgba(255,149,0,0.42);}");
        auto *tm = new QTimer(decisionCore);
        animState->hdrStripBreathTimer = tm;
        bool tick = false;
        QObject::connect(tm, &QTimer::timeout, hdrStrip, [=]() mutable {
            tick = !tick;
            hdrStrip->setStyleSheet(tick ? hdrAlt : hdrBase);
        });
        tm->setInterval(780);
        tm->start();
        auto *swFx = new QGraphicsOpacityEffect(hdrSweepBar);
        hdrSweepBar->setGraphicsEffect(swFx);
        swFx->setOpacity(0.45);
        auto *swA = new QPropertyAnimation(swFx, QByteArrayLiteral("opacity"), hdrSweepBar);
        swA->setDuration(1050);
        swA->setStartValue(0.38);
        swA->setEndValue(1.0);
        swA->setEasingCurve(QEasingCurve::InOutSine);
        swA->setLoopCount(-1);
        swA->start();
        animState->hdrSweepAnim = swA;
    };

    const auto fadeLabelIn = [](QLabel *lab) {
        if (!lab || lab->text().trimmed().isEmpty())
            return;
        auto *fx = new QGraphicsOpacityEffect(lab);
        lab->setGraphicsEffect(fx);
        fx->setOpacity(0.0);
        auto *a = new QPropertyAnimation(fx, QByteArrayLiteral("opacity"), lab);
        a->setDuration(240);
        a->setStartValue(0.0);
        a->setEndValue(1.0);
        a->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(a, &QPropertyAnimation::finished, lab, [lab]() {
            if (qobject_cast<QGraphicsOpacityEffect *>(lab->graphicsEffect()))
                lab->setGraphicsEffect(nullptr);
        });
        a->start(QAbstractAnimation::DeleteWhenStopped);
    };

    auto updateDecisionEngine = [=](double coutAct, double margeAct, double risqueAct, double volume, double coutOpt, double margeOpt, double risqueOpt) {
        setTag(tagLabels[0], coutAct > coutOpt * 1.08, QStringLiteral("rgba(255,200,200,0.35)"), QStringLiteral("#ffcdd2"));
        setTag(tagLabels[1], volume >= 500.0 && margeAct >= 55.0, QStringLiteral("rgba(200,255,210,0.30)"), QStringLiteral("#c8e6c9"));
        setTag(tagLabels[2], risqueAct >= 60.0, QStringLiteral("rgba(255,224,178,0.35)"), QStringLiteral("#ffe0b2"));
        setTag(tagLabels[3], (margeOpt - margeAct) >= 8.0 || (risqueAct - risqueOpt) >= 12.0, QStringLiteral("rgba(225,210,255,0.35)"), QStringLiteral("#e1bee7"));
    };

    auto updateDecisionFlow = [=](double coutAct, double margeAct, double risqueAct, double volume, double coutOpt, double margeOpt, double risqueOpt, int confPct, double seuilApiPct) {
        Q_UNUSED(confPct);
        const QLocale loc = QLocale::system();
        const bool viable = (margeAct >= 58.0 && risqueAct <= 58.0 && volume >= 200.0);
        const double costCutPct = (coutAct > 0.0) ? (100.0 * (coutAct - coutOpt) / coutAct) : 0.0;
        const int volumeTarget = int(std::round(std::max(volume * (viable ? 1.05 : 1.20), volume + 80.0)));
        const bool hasCostPressure = (costCutPct > 0.4);
        const bool marginStress = (margeAct < 58.0);
        const bool conflict = (hasCostPressure && (margeAct < 60.0 || marginStress));
        const QString pctAbs = loc.toString(std::abs(costCutPct), 'f', 1) + QStringLiteral("%");

        if (hasCostPressure)
            lblAiSig1->setText(QStringLiteral("Coût ↑ +%1 vs baseline").arg(pctAbs));
        else
            lblAiSig1->setText(QStringLiteral("Coût stable vs baseline (%1 TND)")
                                   .arg(loc.toString(coutAct, 'f', 1)));
        lblAiSig2->setText(marginStress ? QStringLiteral("Marge sous tension vs cible 58%")
                                        : QStringLiteral("Marge confortable vs baseline"));
        lblAiSig2->setVisible(true);

        lblAiCfl1->setText(conflict ? QStringLiteral("Déséquilibre coût vs marge détecté")
                                    : QStringLiteral("Équilibre coût / marge — pas de conflit majeur"));

        lblAiDec1->setTextFormat(Qt::PlainText);
        if (std::abs(costCutPct) < 0.15) {
            lblAiDec1->setText(QStringLiteral("Conserver l’enveloppe coût | Pas de levier majeur identifié"));
        } else {
            const QString cutS = loc.toString(std::max(0.15, std::abs(costCutPct)), 'f', 1);
            lblAiDec1->setText(QStringLiteral("Réduire coût −%1% | Volume cible : %2").arg(cutS).arg(volumeTarget));
        }

        const double dMarg = margeOpt - margeAct;
        const double dRisk = risqueAct - risqueOpt;
        QString mPart;
        if (std::abs(dMarg) < 0.2) {
            mPart = QStringLiteral("Marge stable");
        } else if (dMarg > 0.15) {
            mPart = QStringLiteral("Marge +%1 %%").arg(loc.toString(dMarg, 'f', 1));
        } else {
            mPart = QStringLiteral("Marge %1 %%").arg(loc.toString(dMarg, 'f', 1));
        }
        const QString rPart = (dRisk >= 0.8) ? QStringLiteral("Risque en baisse")
            : ((dRisk <= -0.8)                     ? QStringLiteral("Risque en hausse")
                                                   : QStringLiteral("Risque stable"));
        QString impCore = QStringLiteral("%1 | %2").arg(mPart, rPart);
        if (seuilApiPct >= 0.0) {
            impCore = QStringLiteral("Seuil matière +%1 %% | %2")
                          .arg(loc.toString(seuilApiPct, 'f', 1), impCore);
        }
        lblAiImp1->setText(impCore);

        kpiSNum->setText(QString::number(volumeTarget));
        kpiSDelta->setText(QStringLiteral("unités"));
        if (stabSeuilBar && volumeTarget > 0)
            stabSeuilBar->setValue(qBound(0, int(std::round(100.0 * volume / double(volumeTarget))), 100));

        animateConfidence(costConfidenceFromRisk(risqueAct));
    };

    auto applyFromBackend = [=](const QJsonObject &root) {
        stopAiRunningPulse();
        if (root.isEmpty())
            return;
        const QString decKey = root.value(QStringLiteral("decision")).toString().trimmed().toUpper();
        QString badge, fg, bg;
        if (decKey == QStringLiteral("LANCER")) {
            badge = QStringLiteral("OPTIMISÉ");
            fg = QStringLiteral("#1b4332");
            bg = QStringLiteral("#f0faf3");
        } else if (decKey == QStringLiteral("AJUSTER")) {
            badge = QStringLiteral("AJUSTER");
            fg = QStringLiteral("#7a4a24");
            bg = QStringLiteral("#fffbf7");
        } else {
            badge = QStringLiteral("RISQUÉ");
            fg = QStringLiteral("#7f1d1d");
            bg = QStringLiteral("#fff8f8");
        }
        lblBadge->setText(badge.toUpper());
        lblBadge->setStyleSheet(QStringLiteral(
            "font-size:9px;font-weight:800;letter-spacing:1.35px;"
            "border-radius:999px;padding:8px 16px;background:%1;color:%2;border:none;")
                                    .arg(bg, fg));
        const QString decNorm = (decKey == QStringLiteral("LANCER")) ? QStringLiteral("LANCER")
            : (decKey == QStringLiteral("AJUSTER"))              ? QStringLiteral("AJUSTER")
                                                                 : QStringLiteral("RISQUE");
        if (!animState->lastDecision.isEmpty() && decNorm != animState->lastDecision)
            UIAnimator::pulseDecisionBadge(lblBadge);
        animState->lastDecision = decNorm;

        const QJsonObject scores = root.value(QStringLiteral("scores")).toObject();
        const int confPct = scores.value(QStringLiteral("confiance")).toInt(0);

        const QJsonObject sens = root.value(QStringLiteral("sensibilite")).toObject();
        const double seuil = sens.value(QStringLiteral("seuilCritiqueMatiere")).toDouble(0.0);

        const QJsonObject bl = root.value(QStringLiteral("baseline")).toObject();
        const double c0 = bl.value(QStringLiteral("cout")).toDouble();
        const double m0 = bl.value(QStringLiteral("marge")).toDouble();
        const double r0 = bl.value(QStringLiteral("risque")).toDouble();
        const QJsonObject op = root.value(QStringLiteral("optimise")).toObject();
        const double c1 = op.value(QStringLiteral("cout")).toDouble();
        const double m1 = op.value(QStringLiteral("marge")).toDouble();
        const double r1 = op.value(QStringLiteral("risque")).toDouble(qMax(0.0, r0 - 10.0));

        const double flatD = static_cast<double>(sldCostShift->value()) * 0.4;
        const double c0Disp = c0 + flatD;
        const double prevDispSnap = (animState->lastDispCost >= 0.0) ? animState->lastDispCost : c0Disp;

        const double prevCostSnap = animState->lastCost;
        const double prevMargSnap = animState->lastMarg;
        const double prevRisqueSnap = animState->lastRisque;
        const int prevStabSnap = animState->lastStab;
        const int prevScoreSnap = animState->lastScore;

        kpiCDelta->setText(QString());
        kpiMDelta->setText(QString());
        kpiRDelta->setText(QString());
        kpiCDelta->setGraphicsEffect(nullptr);
        kpiMDelta->setGraphicsEffect(nullptr);
        kpiRDelta->setGraphicsEffect(nullptr);

        const QString kMutedKpi = QStringLiteral(
            "QLabel{color:#4A4A4A;font-size:34px;font-weight:800;padding:0;margin:0;background:transparent;");
        kpiCNum->setStyleSheet(kMutedKpi + QStringLiteral("min-width:200px;"));
        kpiMNum->setStyleSheet(kMutedKpi + QStringLiteral("min-width:140px;"));
        kpiRNum->setStyleSheet(kMutedKpi + QStringLiteral("min-width:130px;"));
        kpiSNum->setStyleSheet(kMutedKpi + QStringLiteral("min-width:100px;"));
        if (prevCostSnap >= 0.0) {
            kpiCNum->setText(QStringLiteral("%1 TND").arg(QLocale::system().toString(prevDispSnap, 'f', 2)));
            kpiMNum->setText(QStringLiteral("%1%").arg(QLocale::system().toString(prevMargSnap, 'f', 0)));
            kpiRNum->setText(QStringLiteral("%1").arg(QLocale::system().toString(prevRisqueSnap, 'f', 0)));
        } else {
            kpiCNum->setText(QStringLiteral("…"));
            kpiMNum->setText(QStringLiteral("…"));
            kpiRNum->setText(QStringLiteral("…"));
        }
        kpiSNum->setText(QStringLiteral("…"));
        kpiSDelta->setText(QString());
        const int stabTarget = qBound(0, static_cast<int>(std::round(100.0 - r0)), 100);
        if (stabBar)
            stabBar->setValue(prevStabSnap >= 0 ? prevStabSnap : stabTarget);

        lblCmpCost->setText(QStringLiteral("⬇️ %1 TND  →  %2 TND")
                                .arg(QLocale::system().toString(c0, 'f', 2), QLocale::system().toString(c1, 'f', 2)));
        lblCmpMarg->setText(QStringLiteral("⬆️ Marge %1%  →  %2%")
                                .arg(QLocale::system().toString(m0, 'f', 0), QLocale::system().toString(m1, 'f', 0)));
        lblCmpRisk->setText(QStringLiteral("⚠️ Risque %1  →  %2")
                                .arg(QLocale::system().toString(r0, 'f', 0), QLocale::system().toString(r1, 'f', 0)));
        updateDecisionEngine(c0, m0, r0, sbVol->value(), c1, m1, r1);
        updateDecisionFlow(c0, m0, r0, sbVol->value(), c1, m1, r1, confPct, seuil);

        const QJsonArray insA = root.value(QStringLiteral("insights")).toArray();
        for (int i = 0; i < insA.size(); ++i) {
            const QString t = insA.at(i).toString().trimmed();
            if (t.isEmpty())
                continue;
            if (i == 0) {
                if (t.contains(QLatin1Char('\n'))) {
                    const QStringList p = t.split(QLatin1Char('\n'));
                    lblAiSig1->setText(p.value(0).trimmed());
                    const QString s2 = p.value(1).trimmed();
                    if (!s2.isEmpty()) {
                        lblAiSig2->setText(s2);
                        lblAiSig2->setVisible(true);
                    }
                } else {
                    lblAiSig1->setText(t);
                }
            } else if (i == 1) {
                lblAiCfl1->setText(t);
            } else if (i == 2) {
                lblAiDec1->setTextFormat(Qt::PlainText);
                lblAiDec1->setText(t);
            } else if (i == 3) {
                lblAiImp1->setText(t);
            }
        }

        crossfadeToResults();

        aiStatusRow->setVisible(true);
        lblAiRunning->setText(QStringLiteral("Analyse des signaux…"));
        if (animState->aiPulseAnim) {
            animState->aiPulseAnim->stop();
            animState->aiPulseAnim->deleteLater();
            animState->aiPulseAnim = nullptr;
        }
        {
            auto *fx = new QGraphicsOpacityEffect(lblAiPulseDot);
            lblAiPulseDot->setGraphicsEffect(fx);
            fx->setOpacity(1.0);
            auto *pa = new QPropertyAnimation(fx, QByteArrayLiteral("opacity"), lblAiPulseDot);
            pa->setDuration(620);
            pa->setStartValue(0.32);
            pa->setEndValue(1.0);
            pa->setEasingCurve(QEasingCurve::InOutSine);
            pa->setLoopCount(-1);
            animState->aiPulseAnim = pa;
            pa->start();
        }
        startTierShimmers();
        startHeaderPremiumFx();

        const int r0s = scores.value(QStringLiteral("rentabilite")).toInt(0);
        const int r1s = scores.value(QStringLiteral("robustesse")).toInt(0);
        const int r2s = scores.value(QStringLiteral("coherence")).toInt(0);
        const int r3s = scores.value(QStringLiteral("confiance")).toInt(0);
        const int scMoy = costProductScoreFromKpis(m0, r0, sbVol->value());

        const qint64 tNow = QDateTime::currentMSecsSinceEpoch();
        const int targetThink = 800 + QRandomGenerator::global()->bounded(401);
        const int elapsed = int(qMax(qint64(0), tNow - animState->analysisT0));
        const int waitMs = qMax(0, targetThink - elapsed);

        QTimer::singleShot(waitMs, this, [=]() {
            stopTierShimmers();
            stopHeaderPremiumFx();

            kpiCNum->setStyleSheet(QStringLiteral(
                "QLabel{color:#F5F5F5;font-size:34px;font-weight:800;padding:0;margin:0;"
                "background:transparent;min-width:200px;}"));
            kpiMNum->setStyleSheet(QStringLiteral(
                "QLabel{color:#FF9500;font-size:34px;font-weight:800;padding:0;margin:0;"
                "background:transparent;min-width:140px;}"));
            kpiRNum->setStyleSheet(QStringLiteral(
                "QLabel{color:%1;font-size:34px;font-weight:800;padding:0;margin:0;"
                "background:transparent;min-width:130px;}")
                                     .arg(r0 <= 50.0 ? QStringLiteral("#66BB6A") : QStringLiteral("#FF7043")));
            kpiSNum->setStyleSheet(QStringLiteral(
                "QLabel{color:#F5F5F5;font-size:34px;font-weight:800;padding:0;margin:0;"
                "background:transparent;min-width:100px;}"));

            if (prevCostSnap >= 0.0) {
                UIAnimator::animateLabelDouble(kpiCNum, prevDispSnap, c0Disp, 2, QStringLiteral(" TND"), 340);
                UIAnimator::animateLabelDouble(kpiMNum, prevMargSnap, m0, 0, QStringLiteral("%"), 340);
            } else {
                kpiCNum->setText(QStringLiteral("%1 TND").arg(QLocale::system().toString(c0Disp, 'f', 2)));
                kpiMNum->setText(QStringLiteral("%1%").arg(QLocale::system().toString(m0, 'f', 0)));
            }
            animState->lastCost = c0;
            animState->lastDispCost = c0Disp;
            animState->lastMarg = m0;

            if (prevRisqueSnap >= 0.0)
                UIAnimator::animateLabelDouble(kpiRNum, prevRisqueSnap, r0, 0, QString(), 340);
            else
                kpiRNum->setText(QStringLiteral("%1").arg(QLocale::system().toString(r0, 'f', 0)));
            animState->lastRisque = r0;

            const int stabVal = stabTarget;
            if (prevStabSnap >= 0 && stabBar)
                UIAnimator::animateProgress(stabBar, prevStabSnap, stabVal, 300);
            else if (stabBar)
                stabBar->setValue(stabVal);
            animState->lastStab = stabVal;

            if (frRShadow) {
                if (r0 >= 60.0) {
                    frRShadow->setColor(QColor(211, 47, 47, 100));
                    frRShadow->setBlurRadius(34);
                    frRShadow->setOffset(0, 6);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#2A1515;border:1px solid #3A2020;border-radius:8px;}"));
                } else if (r0 >= 45.0) {
                    frRShadow->setColor(QColor(198, 40, 40, 55));
                    frRShadow->setBlurRadius(18);
                    frRShadow->setOffset(0, 4);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#161616;border:1px solid #2A2A2A;border-radius:8px;}"));
                } else {
                    frRShadow->setColor(QColor(0, 0, 0, 18));
                    frRShadow->setBlurRadius(14);
                    frRShadow->setOffset(0, 4);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#161616;border:1px solid #2A2A2A;border-radius:8px;}"));
                }
            }

            QTimer::singleShot(360, this, [=]() {
                if (prevCostSnap >= 0.0) {
                    setDeltaLabel(kpiCDelta, c0Disp - prevDispSnap, true);
                    setDeltaLabel(kpiMDelta, m0 - prevMargSnap, false);
                }
                fadeLabelIn(kpiCDelta);
                fadeLabelIn(kpiMDelta);
            });

            animateValueColor(kpiMNum, QColor("#4A4A4A"), (m0 >= 60.0 ? QColor("#2e7d32") : QColor("#FF9500")), 320, 140);
            animateValueColor(kpiRNum, QColor("#4A4A4A"), (r0 <= 50.0 ? QColor("#66BB6A") : QColor("#FF7043")), 320, 130);

            staggerRevealAiBlocks();

            if (prevScoreSnap >= 0) {
                UIAnimator::animateProgress(scoreBar, prevScoreSnap, scMoy, 300);
                UIAnimator::animateLabelInt(scoreNum, prevScoreSnap, scMoy, 260);
            } else {
                scoreBar->setValue(scMoy);
                scoreNum->setText(QString::number(scMoy));
            }
            animState->lastScore = scMoy;

            lblAdv->setText(QStringLiteral("Rent. %1 · Rob. %2 · Coh. %3 · Conf. %4")
                                .arg(r0s)
                                .arg(r1s)
                                .arg(r2s)
                                .arg(r3s));
            liveOutcome->act.coutUnitaireTnd = c0;
            liveOutcome->act.margePct = m0;
            liveOutcome->act.risqueScore = r0;
            *liveDin = buildDin(SensitivityEngine::materialWithDelta(sbMat->value(), static_cast<double>(sldSens->value())));
            *liveDisp = c0Disp;
            *liveHas = true;
            animState->projectionSeed = animState->projectionSeed + 7u;
            refreshAuxPanels();
            const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
            sourceChipRow->setVisible(false);
            sourceStrip->setStyleSheet(QStringLiteral(
                "QFrame#costSourceStrip{background:transparent;border:none;border-radius:0;}"));
            lblDataSource->setText(QStringLiteral("Source : backend Python (Fashion Oracle) — %1 · mis à jour %2")
                                       .arg(fashionOracleBaseUrl(), ts));
            lblDataSource->setStyleSheet(QStringLiteral(
                "QLabel{font-size:11px;font-weight:600;color:rgba(250,247,244,0.62);padding:0 2px;margin:0;"
                "background:transparent;border:none;}"));
            sourceStrip->setVisible(true);

            QTimer::singleShot(1120, this, [=]() {
                btnAiPressFx->setOpacity(1.0);
                btnAi->setEnabled(true);
                btnAi->setText(QStringLiteral("Analyser"));
            });
        });
    };

    std::function<void(const SimulationOutcome &, bool)> applyOutcome;
    applyOutcome = [=](const SimulationOutcome &o, bool withBreakpoint) {
        QString badge, fg, bg;
        const QString dr = o.decisionRaw.trimmed();
        const QString du = dr.toUpper();
        if (du == QStringLiteral("LANCER") || dr == QStringLiteral("Lancer")) {
            badge = QStringLiteral("OPTIMISÉ");
            fg = QStringLiteral("#1b4332");
            bg = QStringLiteral("#f0faf3");
        } else if (du == QStringLiteral("AJUSTER") || dr == QStringLiteral("Lancer sous conditions")
                   || dr == QStringLiteral("Reconfigurer")) {
            badge = QStringLiteral("AJUSTER");
            fg = QStringLiteral("#7a4a24");
            bg = QStringLiteral("#fffbf7");
        } else {
            badge = QStringLiteral("RISQUÉ");
            fg = QStringLiteral("#7f1d1d");
            bg = QStringLiteral("#fff8f8");
        }
        lblBadge->setText(badge.toUpper());
        lblBadge->setStyleSheet(QStringLiteral(
            "font-size:9px;font-weight:800;letter-spacing:1.35px;border-radius:999px;padding:8px 16px;background:%1;"
            "color:%2;border:none;")
                                    .arg(bg, fg));

        const QString decPulse = (du == QStringLiteral("LANCER") || dr == QStringLiteral("Lancer"))
            ? QStringLiteral("LANCER")
            : ((du == QStringLiteral("AJUSTER") || dr == QStringLiteral("Lancer sous conditions")
                || dr == QStringLiteral("Reconfigurer"))
                   ? QStringLiteral("AJUSTER")
                   : QStringLiteral("RISQUE"));
        if (!animState->lastDecision.isEmpty() && decPulse != animState->lastDecision)
            UIAnimator::pulseDecisionBadge(lblBadge);
        animState->lastDecision = decPulse;

        const double flatSim = static_cast<double>(sldCostShift->value()) * 0.4;
        const double dispNew = o.act.coutUnitaireTnd + flatSim;
        const double dispPrev = (animState->lastDispCost >= 0.0) ? animState->lastDispCost : dispNew;
        const ProductAnalyzerInput dinLiveLine = buildDin(SensitivityEngine::materialWithDelta(
            sbMat->value(), static_cast<double>(sldSens->value())));
        const int userScore = costProductScoreFromKpis(o.act.margePct, o.act.risqueScore, sbVol->value());

        if (animState->lastCost >= 0.0) {
            setDeltaLabel(kpiCDelta, dispNew - dispPrev, true);
            setDeltaLabel(kpiMDelta, o.act.margePct - animState->lastMarg, false);
            UIAnimator::animateLabelDouble(kpiCNum, dispPrev, dispNew, 2, QStringLiteral(" TND"), 220);
            UIAnimator::animateLabelDouble(kpiMNum, animState->lastMarg, o.act.margePct, 0, QStringLiteral("%"), 220);
        } else {
            kpiCDelta->setText(QString());
            kpiMDelta->setText(QString());
            kpiCNum->setText(QStringLiteral("%1 TND").arg(QLocale::system().toString(dispNew, 'f', 2)));
            kpiMNum->setText(QStringLiteral("%1%").arg(QLocale::system().toString(o.act.margePct, 'f', 0)));
        }
        animState->lastCost = o.act.coutUnitaireTnd;
        animState->lastDispCost = dispNew;
        animState->lastMarg = o.act.margePct;

        if (animState->lastRisque >= 0.0)
            UIAnimator::animateLabelDouble(kpiRNum, animState->lastRisque, o.act.risqueScore, 0, QString(), 220);
        else
            kpiRNum->setText(QStringLiteral("%1").arg(QLocale::system().toString(o.act.risqueScore, 'f', 0)));
        animState->lastRisque = o.act.risqueScore;
        kpiRDelta->setText(QString());
        animateValueColor(kpiMNum, QColor("#4A4A4A"), (o.act.margePct >= 60.0 ? QColor("#2e7d32") : QColor("#FF9500")), 300, 140);
        animateValueColor(kpiRNum, QColor("#4A4A4A"),
                           (o.act.risqueScore <= 50.0 ? QColor("#66BB6A") : QColor("#FF7043")), 300, 130);
        const int stabVal = qBound(0, static_cast<int>(std::round(100.0 - o.act.risqueScore)), 100);
        if (animState->lastStab >= 0 && stabBar)
            UIAnimator::animateProgress(stabBar, animState->lastStab, stabVal, 280);
        else if (stabBar)
            stabBar->setValue(stabVal);
        animState->lastStab = stabVal;

        {
            const double r0 = o.act.risqueScore;
            if (frRShadow) {
                if (r0 >= 60.0) {
                    frRShadow->setColor(QColor(211, 47, 47, 100));
                    frRShadow->setBlurRadius(34);
                    frRShadow->setOffset(0, 6);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#2A1515;border:1px solid #3A2020;border-radius:8px;}"));
                } else if (r0 >= 45.0) {
                    frRShadow->setColor(QColor(198, 40, 40, 55));
                    frRShadow->setBlurRadius(18);
                    frRShadow->setOffset(0, 4);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#161616;border:1px solid #2A2A2A;border-radius:8px;}"));
                } else {
                    frRShadow->setColor(QColor(0, 0, 0, 18));
                    frRShadow->setBlurRadius(14);
                    frRShadow->setOffset(0, 4);
                    frR->setStyleSheet(QStringLiteral("QFrame{background:#161616;border:1px solid #2A2A2A;border-radius:8px;}"));
                }
            }
        }

        const QString down = QStringLiteral("⬇️");
        const QString up = QStringLiteral("⬆️");
        lblCmpCost->setText(QStringLiteral("%1 %2 %3  →  %4 TND")
                                .arg(down,
                                     QLocale::system().toString(o.act.coutUnitaireTnd, 'f', 2),
                                     QStringLiteral("TND"),
                                     QLocale::system().toString(o.opt.coutUnitaireTnd, 'f', 2)));
        lblCmpMarg->setText(QStringLiteral("%1 Marge %2%  →  %3%")
                                .arg(up,
                                     QLocale::system().toString(o.act.margePct, 'f', 0),
                                     QLocale::system().toString(o.opt.margePct, 'f', 0)));
        lblCmpRisk->setText(QStringLiteral("⚠️ Risque %1  →  %2")
                                .arg(QLocale::system().toString(o.act.risqueScore, 'f', 0),
                                     QLocale::system().toString(o.opt.risqueScore, 'f', 0)));
        updateDecisionEngine(
            o.act.coutUnitaireTnd,
            o.act.margePct,
            o.act.risqueScore,
            sbVol->value(),
            o.opt.coutUnitaireTnd,
            o.opt.margePct,
            o.opt.risqueScore
        );
        updateDecisionFlow(
            o.act.coutUnitaireTnd,
            o.act.margePct,
            o.act.risqueScore,
            sbVol->value(),
            o.opt.coutUnitaireTnd,
            o.opt.margePct,
            o.opt.risqueScore,
            o.confidencePct,
            -1.0
        );

        QStringList ins = InsightEngine::topInsights(o.tensions, 4);
        const int insCap = qMin(ins.size(), 3);
        for (int i = 0; i < insCap; ++i) {
            const QString t = ins.value(i).trimmed();
            if (t.isEmpty())
                continue;
            if (i == 0) {
                if (t.contains(QLatin1Char('\n'))) {
                    const QStringList p = t.split(QLatin1Char('\n'));
                    lblAiSig1->setText(p.value(0).trimmed());
                    const QString s2 = p.value(1).trimmed();
                    if (!s2.isEmpty()) {
                        lblAiSig2->setText(s2);
                        lblAiSig2->setVisible(true);
                    }
                } else {
                    lblAiSig1->setText(t);
                }
            } else if (i == 1) {
                lblAiCfl1->setText(t);
            } else if (i == 2) {
                lblAiDec1->setTextFormat(Qt::PlainText);
                lblAiDec1->setText(t);
            }
        }
        if (withBreakpoint) {
            const ProductAnalyzerInput ref = buildDin(SensitivityEngine::materialWithDelta(
                sbMat->value(), static_cast<double>(sldSens->value())));
            lblAiImp1->setText(BreakpointEngine::materialThresholdLine(ref));
        } else if (ins.size() > 3 && !ins.value(3).trimmed().isEmpty()) {
            lblAiImp1->setText(ins.value(3).trimmed());
        }
        staggerRevealAiBlocks();

        if (animState->lastScore >= 0) {
            UIAnimator::animateProgress(scoreBar, animState->lastScore, userScore, 300);
            UIAnimator::animateLabelInt(scoreNum, animState->lastScore, userScore, 240);
        } else {
            scoreBar->setValue(userScore);
            scoreNum->setText(QString::number(userScore));
        }
        animState->lastScore = userScore;

        const QString n0 = o.tensions.isEmpty() ? QStringLiteral("—") : o.tensions.first().niveau;
        lblAdv->setText(QStringLiteral("Coh. %1 · Risque %2 · %3")
                            .arg(QLocale::system().toString(o.coherence, 'f', 0),
                                 QLocale::system().toString(o.act.risqueScore, 'f', 0),
                                 n0));

        *liveOutcome = o;
        *liveDin = dinLiveLine;
        *liveDisp = dispNew;
        *liveHas = true;
        animState->projectionSeed = animState->projectionSeed + 31u;
        refreshAuxPanels();

        sourceChipRow->setVisible(true);
        sourceStrip->setStyleSheet(QStringLiteral(
            "QFrame#costSourceStrip{background:transparent;border:none;border-radius:0;}"));
        lblDataSource->setText(QStringLiteral(
            "Source : moteur local C++ intégré (temps réel, sans appel externe)"));
        lblDataSource->setStyleSheet(QStringLiteral(
            "QLabel{font-size:11px;font-weight:600;color:rgba(250,247,244,0.62);padding:0 2px;margin:0;"
            "background:transparent;border:none;}"));
        sourceStrip->setVisible(true);
        crossfadeToResults();
    };

    auto runSim = [=](bool withBreakpoint) {
        if (!*hasManualAnalysis)
            return;
        const double m = SensitivityEngine::materialWithDelta(sbMat->value(), static_cast<double>(sldSens->value()));
        const ProductAnalyzerInput din = buildDin(m);
        const SimulationOutcome o = SimulationEngine::compute(din);
        applyOutcome(o, withBreakpoint);
    };

    QObject::connect(sldSens, &QSlider::valueChanged, this, [=](int) { runSim(false); });
    QObject::connect(sldCostShift, &QSlider::valueChanged, this, [=](int) { runSim(false); });
    QObject::connect(sbMat, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double) { runSim(false); });
    QObject::connect(sbVol, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int) { runSim(false); });
    QObject::connect(leLib, &QLineEdit::textChanged, this, [=] { runSim(false); });
    QObject::connect(btnStress, &QPushButton::clicked, this, [=]() {
        sldSens->setValue(qMin(15, sldSens->value() + 10));
    });
    QObject::connect(modeBtn, &QPushButton::toggled, lblAdv, [=](bool on) {
        modeBtn->setText(on ? QStringLiteral("🧠") : QStringLiteral("⚡"));
        lblAdv->setVisible(on);
    });
    QObject::connect(btnImprove, &QPushButton::clicked, this, [=]() {
        if (!*hasManualAnalysis)
            return;
        const int shift = sldCostShift->value();
        const double baseMat = SensitivityEngine::materialWithDelta(sbMat->value(), static_cast<double>(sldSens->value()));
        const double matAdjusted = baseMat * (1.0 + static_cast<double>(shift) / 100.0);
        const ProductAnalyzerInput din = buildDin(matAdjusted);
        const SimulationOutcome o = SimulationEngine::compute(din);
        applyOutcome(o, true);
    });

    QObject::connect(btnAi, &QAbstractButton::pressed, this, [=]() {
        if (!btnAi->isEnabled())
            return;
        btnAiPressAnim->stop();
        btnAiPressAnim->setStartValue(btnAiPressFx->opacity());
        btnAiPressAnim->setEndValue(0.86);
        btnAiPressAnim->start();
    });
    QObject::connect(btnAi, &QAbstractButton::released, this, [=]() {
        btnAiPressAnim->stop();
        btnAiPressAnim->setStartValue(btnAiPressFx->opacity());
        btnAiPressAnim->setEndValue(1.0);
        btnAiPressAnim->start();
    });

    QObject::connect(btnAi, &QPushButton::clicked, this, [=]() {
        *hasManualAnalysis = true;
        btnAiPressAnim->stop();
        btnAiPressFx->setOpacity(1.0);
        btnAi->setEnabled(false);
        btnAi->setText(QStringLiteral("…"));
        lblPreSub->setText(QStringLiteral("Analyse en cours — connexion au moteur IA…"));
        animState->lastCost = -1.0;
        animState->lastMarg = -1.0;
        animState->lastRisque = -1.0;
        animState->lastScore = -1;
        animState->lastStab = -1;
        animState->lastDecision.clear();
        animState->analysisT0 = QDateTime::currentMSecsSinceEpoch();

        if (m_costSimReply) {
            m_costSimReply->disconnect();
            m_costSimReply->abort();
            m_costSimReply->deleteLater();
            m_costSimReply.clear();
        }

        QString backendErr;
        if (!ensureFashionOracleBackendReady(&backendErr, 32000)) {
            QMessageBox::warning(
                this,
                QStringLiteral("Simulateur coût"),
                QStringLiteral("Le serveur Fashion Oracle n’est pas joignable sur %1.\n%2\n\n"
                               "Le backend n’a pas pu être démarré automatiquement ou le port "
                               "(variable FASHION_ORACLE_PORT, défaut 8010) ne correspond pas à uvicorn.")
                    .arg(fashionOracleBaseUrl(), backendErr.trimmed().isEmpty() ? QStringLiteral("—") : backendErr.trimmed()));
            lblPreSub->setText(QStringLiteral("Le moteur IA est prêt à analyser coût, marge et risque"));
            btnAiPressFx->setOpacity(1.0);
            btnAi->setEnabled(true);
            btnAi->setText(QStringLiteral("Analyser"));
            return;
        }

        QUrl u(fashionOracleBaseUrl() + QStringLiteral("/api/simulateur-cout/analyser"));
        QJsonObject body;
        body.insert(QStringLiteral("produit"),
                    leLib->text().trimmed().isEmpty() ? QStringLiteral("Produit") : leLib->text().trimmed());
        body.insert(QStringLiteral("coutMatiere"), sbMat->value());
        body.insert(QStringLiteral("volume"), sbVol->value());
        body.insert(QStringLiteral("objectif"), QStringLiteral("equilibre"));
        const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

        const std::shared_ptr<int> attempt = std::make_shared<int>(0);
        const auto launch = std::make_shared<std::function<void()>>();
        *launch = [=]() {
            QNetworkRequest req(u);
            req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            req.setTransferTimeout(120000);
#endif
            QNetworkReply *rep = m_namCostSim.post(req, payload);
            m_costSimReply = rep;
            QObject::connect(rep, &QNetworkReply::finished, this, [=]() {
                QNetworkReply *r = m_costSimReply.data();
                if (!r || r != rep)
                    return;
                const QNetworkReply::NetworkError nerr = r->error();
                const QVariant httpAttr = r->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                const int httpSt = httpAttr.isValid() ? httpAttr.toInt() : -1;
                const QByteArray raw = r->readAll();
                r->deleteLater();
                m_costSimReply.clear();

                const bool httpOk = (nerr == QNetworkReply::NoError && httpSt >= 200 && httpSt < 300);
                QJsonParseError pe{};
                const QJsonDocument jd = QJsonDocument::fromJson(raw, &pe);
                if (httpOk && pe.error == QJsonParseError::NoError && jd.isObject()) {
                    applyFromBackend(jd.object());
                    qDebug() << "[SimulateurCout] OK API" << u.toString() << "marge="
                             << jd.object().value(QStringLiteral("baseline")).toObject().value(QStringLiteral("marge")).toDouble();
                    return;
                }
                qDebug() << "[SimulateurCout] API erreur tentative=" << (*attempt)
                         << "url=" << u.toString() << "err=" << nerr << "http=" << httpSt << "corps=" << raw.left(160);
                if (*attempt < 2) {
                    (*attempt) += 1;
                    QTimer::singleShot(1500, this, [launch]() { (*launch)(); });
                    return;
                }
                QMessageBox::warning(
                    this,
                    QStringLiteral("Simulateur coût"),
                    QStringLiteral("L’API n’a pas répondu après plusieurs tentatives. "
                                   "Vérifiez le backend et FASHION_ORACLE_PORT. "
                                   "Aucun calcul local de substitution n’est appliqué."));
                lblPreSub->setText(QStringLiteral("Le moteur IA est prêt à analyser coût, marge et risque"));
                btnAiPressFx->setOpacity(1.0);
                btnAi->setEnabled(true);
                btnAi->setText(QStringLiteral("Analyser"));
                stopAiRunningPulse();
                const auto restoreFace = [=](QVBoxLayout *liftLy) {
                    QWidget *f = faceFromLift(liftLy);
                    if (f && f->graphicsEffect())
                        f->setGraphicsEffect(nullptr);
                };
                restoreFace(liftSignal);
                restoreFace(liftConflict);
                restoreFace(liftDecision);
                restoreFace(liftImpact);
                stopTierShimmers();
            });
        };
        startAiRunningPulse();
        dimAiBlocksPending();
        (*launch)();
    });

    {
        QSignalBlocker blocker(ui->tabWidgetProduits);
        ui->tabWidgetProduits->setCurrentIndex(4);
    }
}

QString MainWindow::resolveFashionOracleDir() const {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir::cleanPath(appDir + "/fashion_oracle"),
        QDir::cleanPath(appDir + "/../fashion_oracle"),
        QDir::cleanPath(appDir + "/../../fashion_oracle"),
        QStringLiteral("c:/integration2/integ0/integration2/projetcpp_Integration/fashion_oracle"),
    };
    for (const QString &dirPath : candidates) {
        const QFileInfo mainPy(QDir(dirPath).filePath("app/main.py"));
        if (!mainPy.exists())
            continue;
        const QFileInfo venvPy(QDir(dirPath).filePath(".venv/Scripts/python.exe"));
        const QFileInfo venvNewPy(QDir(dirPath).filePath(".venv-new/Scripts/python.exe"));
        if (venvPy.exists() || venvNewPy.exists())
            return QDir::cleanPath(dirPath);
    }
    return QString();
}

QString MainWindow::resolveFashionOraclePython() const {
    const QString backendDir = resolveFashionOracleDir();
    if (backendDir.isEmpty())
        return QString();
    const QString vnew = QDir(backendDir).filePath(".venv-new/Scripts/python.exe");
    if (QFileInfo::exists(vnew))
        return QDir::cleanPath(vnew);
    const QString legacy = QDir(backendDir).filePath(".venv/Scripts/python.exe");
    return QFileInfo::exists(legacy) ? QDir::cleanPath(legacy) : QString();
}

bool MainWindow::isFashionOracleHealthy(int timeoutMs) const {
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl(fashionOracleBaseUrl() + QStringLiteral("/health")));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    req.setTransferTimeout(qMax(timeoutMs, 15000));
#endif
    QEventLoop loop;
    QNetworkReply *reply = nam.get(req);
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(timeoutMs);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeout.start();
    loop.exec();

    const bool ok = timeout.isActive() && reply->error() == QNetworkReply::NoError;
    if (!ok) {
        reply->abort();
        reply->deleteLater();
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    const QJsonObject obj = doc.object();
    return obj.value(QStringLiteral("status")).toString().compare(QStringLiteral("ok"), Qt::CaseInsensitive) == 0;
}

/** Environnement du sous-processus uvicorn : copie fashion_oracle/.env (cle=valeur) pour eviter IMAGE_MODEL vide. */
static QProcessEnvironment fashionOracleEnvironmentForSubprocess(const QString &backendDir)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString dotEnvPath = QDir(backendDir).filePath(QStringLiteral(".env"));
    QFile f(dotEnvPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[FashionOracle] .env absent ou illisible path=" << dotEnvPath;
        return env;
    }
    const QByteArray raw = f.readAll();
    const QString text = QString::fromUtf8(raw);
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (QString line : lines) {
        if (line.endsWith(QLatin1Char('\r')))
            line.chop(1);
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;
        const int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0)
            continue;
        const QString key = line.left(eq).trimmed();
        if (key.isEmpty())
            continue;
        QString val = line.mid(eq + 1).trimmed();
        if (val.size() >= 2
            && ((val.startsWith(QLatin1Char('"')) && val.endsWith(QLatin1Char('"')))
                || (val.startsWith(QLatin1Char('\'')) && val.endsWith(QLatin1Char('\''))))) {
            val = val.mid(1, val.size() - 2);
        }
        env.insert(key, val);
    }
    qDebug() << "[FashionOracle] subprocess env charge depuis" << dotEnvPath
             << "IMAGE_MODEL=" << env.value(QStringLiteral("FASHION_ORACLE_IMAGE_MODEL")).left(40);
    return env;
}

bool MainWindow::startFashionOracleBackendProcess(QString *errorOut) {
    if (isFashionOracleHealthy(3000))
        return true;

    if (!m_fashionOracleBackendProcess) {
        m_fashionOracleBackendProcess = new QProcess(this);
        m_fashionOracleBackendProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_fashionOracleBackendProcess, &QProcess::errorOccurred, this, [](QProcess::ProcessError err) {
            qDebug() << "[FashionOracle] backend process error=" << err;
        });
    }

    if (m_fashionOracleBackendProcess->state() != QProcess::NotRunning)
        return true;

    const QString backendDir = resolveFashionOracleDir();
    const QString pythonExe = resolveFashionOraclePython();
    if (backendDir.isEmpty() || pythonExe.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("Les fichiers du moteur Fashion Oracle sont introuvables.");
        return false;
    }

    m_fashionOracleBackendProcess->setWorkingDirectory(backendDir);
    m_fashionOracleBackendProcess->setProgram(pythonExe);
    m_fashionOracleBackendProcess->setArguments({
        QStringLiteral("-m"),
        QStringLiteral("uvicorn"),
        QStringLiteral("app.main:app"),
        QStringLiteral("--host"),
        QStringLiteral("127.0.0.1"),
        QStringLiteral("--port"),
        QString::number(fashionOracleListenPort()),
    });
    m_fashionOracleBackendProcess->setProcessEnvironment(fashionOracleEnvironmentForSubprocess(backendDir));
    m_fashionOracleBackendProcess->start();
    if (!m_fashionOracleBackendProcess->waitForStarted(5000)) {
        if (errorOut)
            *errorOut = QStringLiteral("Le moteur de prédiction n’a pas pu être initialisé.");
        return false;
    }
    m_fashionOracleBackendOwned = true;
    return true;
}

bool MainWindow::ensureFashionOracleBackendReady(QString *errorOut, int startupTimeoutMs) {
    // Delai court : 127.0.0.1 repond vite si le serveur est up ; evite de bloquer l UI 10 s par essai.
    if (isFashionOracleHealthy(2500))
        return true;

    if (!startFashionOracleBackendProcess(errorOut))
        return false;

    QElapsedTimer elapsed;
    elapsed.start();
    while (elapsed.elapsed() < startupTimeoutMs) {
        if (isFashionOracleHealthy(2500))
            return true;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QEventLoop waitLoop;
        QTimer t;
        t.setSingleShot(true);
        QObject::connect(&t, &QTimer::timeout, &waitLoop, &QEventLoop::quit);
        t.start(450);
        waitLoop.exec();
    }
    if (errorOut)
        *errorOut = QStringLiteral("Le moteur de prédiction n’a pas pu être initialisé.");
    return false;
}

static QString fashionOracleConceptTypeFr(const MainWindow::FashionOracleConcept &c)
{
    const QString e = c.productTypeEn.trimmed().toLower();
    if (e == QLatin1String("jacket"))
        return QStringLiteral("veste");
    if (e == QLatin1String("bag"))
        return QStringLiteral("sac");
    if (e == QLatin1String("belt"))
        return QStringLiteral("ceinture");
    return QStringLiteral("veste");
}

QJsonObject MainWindow::buildGenerateVisualsPostJson(const FashionOracleConcept &concept)
{
    QJsonObject o;
    o.insert(QStringLiteral("year"), concept.targetYear);
    o.insert(QStringLiteral("type"), fashionOracleConceptTypeFr(concept));
    o.insert(QStringLiteral("style"), concept.style);
    o.insert(QStringLiteral("palette"), concept.palette);
    o.insert(QStringLiteral("material"), concept.material);
    o.insert(QStringLiteral("concept_index"), concept.conceptIndex);
    o.insert(QStringLiteral("image_prompt"), buildPromptForConcept(concept));
    return o;
}

QByteArray MainWindow::jsonPayloadForFashionOracleGenerateVisuals(const FashionOracleConcept &concept)
{
    return QJsonDocument(buildGenerateVisualsPostJson(concept)).toJson(QJsonDocument::Compact);
}

QNetworkReply *MainWindow::sendFashionOracleGenerateVisualRequest(
    QNetworkAccessManager *nam,
    const FashionOracleConcept &concept,
    int transferTimeoutMs,
    QByteArray *outSentJson)
{
    const QJsonObject jo = buildGenerateVisualsPostJson(concept);
    QUrl url(fashionOracleBaseUrl() + QStringLiteral("/generate-visuals"));
    QUrlQuery qy;
    qy.addQueryItem(QStringLiteral("year"), QString::number(concept.targetYear));
    qy.addQueryItem(QStringLiteral("limit"), QStringLiteral("3"));
    qy.addQueryItem(QStringLiteral("concept_index"), QString::number(concept.conceptIndex));
    qy.addQueryItem(QStringLiteral("type"), jo.value(QStringLiteral("type")).toString());
    qy.addQueryItem(QStringLiteral("style"), concept.style);
    qy.addQueryItem(QStringLiteral("palette"), concept.palette);
    qy.addQueryItem(QStringLiteral("material"), concept.material);
    qy.addQueryItem(QStringLiteral("image_prompt"), jo.value(QStringLiteral("image_prompt")).toString());
    url.setQuery(qy);

    QNetworkRequest req(url);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    req.setTransferTimeout(transferTimeoutMs);
#endif
    const QByteArray logPayload = QJsonDocument(jo).toJson(QJsonDocument::Compact);
    if (outSentJson)
        *outSentJson = logPayload;
    qDebug() << "[FashionOracle] GET generate-visuals (evite 405 si le serveur n accepte pas POST) url="
             << url.toString(QUrl::FullyEncoded);
    qDebug() << "[FashionOracle] equivalent JSON logique=" << QString::fromUtf8(logPayload);
    return nam->get(req);
}

QString MainWindow::buildPromptForConcept(const FashionOracleConcept &concept)
{
    const QString pt = concept.productTypeEn.trimmed().toLower();
    const QString y = QString::number(concept.targetYear);
    const QString st = concept.style.trimmed();
    const QString pal = concept.palette.trimmed();
    const QString mat = concept.material.trimmed();

    if (pt == QLatin1String("jacket") || pt == QLatin1String("veste")) {
        return QStringLiteral(
            "premium product photography, isolated product, no human, no face, no model, clean studio background, "
            "soft lighting, high detail, luxury fashion product, product centered composition. "
            "jacket only, no person wearing it, floating or ghost mannequin style, full jacket visible, front or 3/4 angle, "
            "show structure and material clearly. "
            "NEGATIVE: human, face, model, body, portrait, person, skin, hands, mannequin. "
            "Style %1, palette %2, material %3, year %4.")
            .arg(st, pal, mat, y);
    }
    if (pt == QLatin1String("bag") || pt == QLatin1String("sac")) {
        return QStringLiteral(
            "SAC SAC SAC — UN SEUL sac a main ou bandouliere, packshot fond blanc, objet seul, zero mannequin. "
            "PAS de veste, PAS de blouson, PAS de manches, PAS de torse. La tendance %1 = details du sac seulement. "
            "Matiere %3 = coque du sac (pas vetement). Palette %2. Annee %4. Photo produit 85 mm. "
            "INTERDIT: veste utilitaire, gilet technique, parka, modele humain.")
            .arg(st, pal, mat, y);
    }
    if (pt == QLatin1String("belt") || pt == QLatin1String("ceinture")) {
        return QStringLiteral(
            "premium product photography, isolated product, no human, no face, no model, clean studio background, "
            "soft lighting, high detail, luxury fashion product, product centered composition. "
            "belt only, clean studio shot, high detail, focus on leather and buckle. "
            "NEGATIVE: human, face, model, body, portrait, person, skin, hands, mannequin. "
            "Style %1, palette %2, material %3, year %4.")
            .arg(st, pal, mat, y);
    }
    return QStringLiteral(
        "premium product photography, isolated product, no human, no face, no model, clean studio background, "
        "soft lighting, high detail, luxury fashion product, product centered composition. "
        "NEGATIVE: human, face, model, body, portrait, person, skin, hands, mannequin. "
        "Style %1, palette %2, material %3, year %4.")
        .arg(st, pal, mat, y);
}

void MainWindow::showHistoriqueModeDialog() {
    if (ui->tabWidgetProduits->count() < 6)
        return;
    if (m_histCapsuleReply) {
        m_histCapsuleReply->disconnect();
        m_histCapsuleReply->abort();
        m_histCapsuleReply->deleteLater();
        m_histCapsuleReply.clear();
    }
    ui->tabWidgetProduits->setCurrentIndex(5);

    QWidget *ongletHist = ui->tabWidgetProduits->widget(5);
    if (!ongletHist)
        return;
    if (ongletHist->layout()) {
        clearLayout(ongletHist->layout());
        delete ongletHist->layout();
    }

    QVBoxLayout *root = new QVBoxLayout(ongletHist);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QScrollArea *scroll = new QScrollArea(ongletHist);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    root->addWidget(scroll);

    QWidget *content = new QWidget();
    scroll->setWidget(content);

    QVBoxLayout *l = new QVBoxLayout(content);
    l->setSpacing(18);
    l->setContentsMargins(22, 20, 22, 22);
    content->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #f7f3ee, stop:0.55 #f4efe8, stop:1 #efe8df);"
        "}");

    const int targetYearDefault = QDate::currentDate().year() + 1;
    const QVariant vYear = ui->tabWidgetProduits->property("fashionOracleTargetYear");
    const int targetYear = vYear.isValid() ? vYear.toInt() : targetYearDefault;
    const QString storedScenario = ui->tabWidgetProduits->property("fashionOracleScenario").toString();
    const QString scenarioModeUi = storedScenario.isEmpty() ? QStringLiteral("balanced") : storedScenario.toLower();
    const bool hasComputed = ui->tabWidgetProduits->property("fashionOracleHasComputed").toBool();
    const qint64 predictRequestId = QDateTime::currentMSecsSinceEpoch();
    ui->tabWidgetProduits->setProperty("fashionOracleActiveYear", targetYear);
    ui->tabWidgetProduits->setProperty("fashionOraclePredictRequestId", predictRequestId);
    qDebug() << "[FashionOracle] predict selected year=" << targetYear << " requestId=" << predictRequestId;

    QLabel *titre = new QLabel("HISTORIQUE DE MODE - FASHION ORACLE");
    titre->setStyleSheet("font-size: 27px; font-weight: 900; color: #4b2f2a; margin-bottom: 1px; letter-spacing: 1.3px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QLabel *sub = new QLabel("Cockpit décisionnel de prospective mode au service de la stratégie produit.");
    sub->setStyleSheet("font-size: 12px; color: #766657; margin-bottom: 10px; letter-spacing: 0.2px;");
    sub->setAlignment(Qt::AlignCenter);
    l->addWidget(sub);

    QFrame *controls = new QFrame();
    controls->setStyleSheet("QFrame { background: rgba(255,255,255,0.94); border: none; border-radius: 12px; }");
    QHBoxLayout *ctl = new QHBoxLayout(controls);
    ctl->setContentsMargins(16, 12, 16, 12);
    ctl->setSpacing(14);
    QLabel *lblYear = new QLabel("Année cible");
    lblYear->setStyleSheet("font-size: 12px; font-weight: 800; color: #5d4037; letter-spacing: 0.4px;");
    QSpinBox *sbYear = new QSpinBox();
    sbYear->setRange(2025, 2040);
    sbYear->setValue(targetYear);
    sbYear->setStyleSheet("QSpinBox { background: #fdfaf6; border: 1px solid #ddcfc0; border-radius: 9px; padding: 7px 8px; min-width: 98px; font-weight: 600; color: #4e342e; }");
    QPushButton *btnPredict = new QPushButton("Prédire");
    btnPredict->setStyleSheet(
        "QPushButton { background-color: #8B4513; color: white; border-radius: 6px; min-height: 36px; padding: 0 24px; font-size:13px; font-weight: 800; letter-spacing: 0.3px; }"
        "QPushButton:hover { background-color: #a0521a; }");
    ctl->addWidget(lblYear);
    ctl->addWidget(sbYear);
    ctl->addWidget(btnPredict);
    ctl->addStretch();
    l->addWidget(controls);

    connect(btnPredict, &QPushButton::clicked, this, [this, sbYear]() {
        ui->tabWidgetProduits->setProperty("fashionOracleTargetYear", sbYear->value());
        ui->tabWidgetProduits->setProperty("fashionOracleActiveYear", sbYear->value());
        ui->tabWidgetProduits->setProperty("fashionOracleScenario", QStringLiteral("balanced"));
        ui->tabWidgetProduits->setProperty("fashionOracleHasComputed", true);
        ui->tabWidgetProduits->setProperty("fashionOracleDirty", true);
        ui->tabWidgetProduits->setCurrentIndex(5);
        showHistoriqueModeDialog();
    });
    connect(sbYear, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, sbYear](int) {
        ui->tabWidgetProduits->setProperty("fashionOracleTargetYear", sbYear->value());
        ui->tabWidgetProduits->setProperty("fashionOracleActiveYear", sbYear->value());
    });

    if (!hasComputed) {
        l->addSpacing(40);
        QLabel *placeholder = new QLabel(QStringLiteral("Sélectionnez une année et cliquez sur Prédire"));
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setStyleSheet("font-size: 14px; color: #9a8a70; font-weight: 600;");
        l->addWidget(placeholder);
        l->addStretch();
        return;
    }

    QFrame *loadingBox = new QFrame();
    loadingBox->setStyleSheet("QFrame { background: rgba(255,248,225,0.9); border: none; border-radius: 10px; }");
    QHBoxLayout *loadingLay = new QHBoxLayout(loadingBox);
    loadingLay->setContentsMargins(12, 9, 12, 9);
    QLabel *loadingLbl = new QLabel("Analyse IA en cours... collecte des signaux mode et projection stratégique.");
    loadingLbl->setStyleSheet("color: #6d4c41; font-size: 12px; font-weight: 700;");
    QProgressBar *loadingBar = new QProgressBar();
    loadingBar->setRange(0, 100);
    loadingBar->setValue(8);
    loadingBar->setMaximumWidth(180);
    loadingBar->setTextVisible(false);
    loadingLay->addWidget(loadingLbl);
    loadingLay->addWidget(loadingBar);
    l->addWidget(loadingBox);
    QCoreApplication::processEvents();

    auto runStep = [&](const QString &text, int value, int pauseMs) {
        loadingLbl->setText(text);
        loadingBar->setValue(qBound(0, value, 100));
        QCoreApplication::processEvents();
        QEventLoop stepLoop;
        QTimer stepTimer;
        stepTimer.setSingleShot(true);
        QObject::connect(&stepTimer, &QTimer::timeout, &stepLoop, &QEventLoop::quit);
        stepTimer.start(pauseMs);
        stepLoop.exec();
    };
    runStep("Étape 1/3 - scanning signals...", 24, 220);
    runStep("Étape 2/3 - simulating scenarios...", 46, 220);
    runStep("Étape 3/3 - resolving conflicts...", 66, 180);
    runStep("Compilation des hypothèses IA...", 78, 140);

    {
        const bool dirty = ui->tabWidgetProduits->property("fashionOracleDirty").toBool();
        QJsonObject obj = ui->tabWidgetProduits->property("fashionOracleCachedPayload").toJsonObject();
        if (dirty && hasComputed) {
            obj = buildFashionForecastPayloadLocal(targetYear, scenarioModeUi);
            ui->tabWidgetProduits->setProperty("fashionOracleCachedPayload", obj);
            ui->tabWidgetProduits->setProperty("fashionOracleDirty", false);
        }
        QNetworkReply *reply = nullptr;
        if (!obj.isEmpty()) {
            if (!obj.isEmpty() && obj.contains(QStringLiteral("top_styles"))) {
                const int responseYear = obj.value(QStringLiteral("year")).toInt(targetYear);
                const int activeYear = ui->tabWidgetProduits->property("fashionOracleActiveYear").toInt();
                qDebug() << "[FashionOracle] predict response year=" << responseYear << " activeYear=" << activeYear;
                if (responseYear != activeYear) {
                    qDebug() << "[FashionOracle] stale predict response ignored";
                    if (reply)
                        reply->deleteLater();
                    return;
                }
                const bool forecastEligible = obj.value(QStringLiteral("forecast_eligible")).toBool(true);
                const QString businessNotice = obj.value(QStringLiteral("business_notice")).toString();
                if (!forecastEligible) {
                    loadingBox->hide();
                    QFrame *noticeFrame = new QFrame();
                    noticeFrame->setStyleSheet(
                        "QFrame { background: rgba(255,255,255,0.97); border: none; border-radius: 14px; }");
                    QVBoxLayout *nLay = new QVBoxLayout(noticeFrame);
                    nLay->setContentsMargins(20, 18, 20, 18);
                    nLay->setSpacing(10);
                    QLabel *nh = new QLabel("Périmètre prévision Fashion Oracle");
                    nh->setStyleSheet("font-size: 17px; font-weight: 900; color: #4a2f29;");
                    nh->setAlignment(Qt::AlignCenter);
                    const QString nbText = businessNotice.isEmpty()
                        ? QStringLiteral("La prédiction avancée est disponible uniquement à partir de 2026.")
                        : businessNotice;
                    QLabel *nb = new QLabel(nbText);
                    nb->setWordWrap(true);
                    nb->setAlignment(Qt::AlignCenter);
                    nb->setStyleSheet("font-size: 13px; color: #5d4037; line-height: 1.55;");
                    nLay->addWidget(nh);
                    nLay->addWidget(nb);
                    l->addWidget(noticeFrame);
                    auto *nFx = new QGraphicsDropShadowEffect(noticeFrame);
                    nFx->setBlurRadius(26);
                    nFx->setOffset(0, 7);
                    nFx->setColor(QColor(62, 39, 35, 32));
                    noticeFrame->setGraphicsEffect(nFx);
                    if (reply)
                        reply->deleteLater();
                    ui->tabWidgetProduits->setCurrentIndex(5);
                    return;
                }
                const QString summary = obj.value(QStringLiteral("summary")).toString();
                const double confidence = obj.value(QStringLiteral("confidence")).toDouble();
                const QString inferenceMode = obj.value(QStringLiteral("inference_mode")).toString();
                const QString engineVersion = obj.value(QStringLiteral("prediction_engine_version")).toString();
                const bool fromServerCache = obj.value(QStringLiteral("from_cache")).toBool();
                if (fromServerCache) {
                    loadingLbl->setText(QStringLiteral(
                        "Prévision instantanée (cache serveur — même année, même moteur)."));
                    QCoreApplication::processEvents();
                }
                qDebug() << "[FashionOracle] engine=" << engineVersion << "mode=" << inferenceMode
                         << "year=" << responseYear << "cache=" << fromServerCache;

                QStringList topStyles;
                const QJsonArray arrStyles = obj.value(QStringLiteral("top_styles")).toArray();
                for (const QJsonValue &v : arrStyles)
                    topStyles << v.toString();

                QStringList palette;
                const QJsonArray arrPalette = obj.value(QStringLiteral("color_palette")).toArray();
                for (const QJsonValue &v : arrPalette)
                    palette << v.toString();
                QStringList paletteHex;
                const QJsonArray arrPaletteHex = obj.value(QStringLiteral("palette_colors")).toArray();
                for (const QJsonValue &v : arrPaletteHex)
                    paletteHex << v.toString();

                QStringList fabrics;
                const QJsonArray arrFab = obj.value(QStringLiteral("fabrics_materials")).toArray();
                for (const QJsonValue &v : arrFab)
                    fabrics << v.toString();

                QStringList sil;
                const QJsonArray arrSil = obj.value(QStringLiteral("silhouettes")).toArray();
                for (const QJsonValue &v : arrSil)
                    sil << v.toString();

                const QJsonArray arrDec = obj.value(QStringLiteral("similar_decades")).toArray();
                QStringList decades;
                for (const QJsonValue &v : arrDec)
                    decades << v.toString();

                QMap<QString, QString> recommandations;
                const QJsonObject recoObj = obj.value(QStringLiteral("recommended_product_attributes")).toObject();
                for (auto it = recoObj.begin(); it != recoObj.end(); ++it)
                    recommandations[it.key()] = it.value().toString();

                auto applySoftShadow = [](QWidget *w) {
                    auto *fx = new QGraphicsDropShadowEffect(w);
                    fx->setBlurRadius(26);
                    fx->setOffset(0, 7);
                    fx->setColor(QColor(62, 39, 35, 32));
                    w->setGraphicsEffect(fx);
                };

                auto colorHex = [](const QString &name) -> QString {
                    const QString n = name.trimmed().toLower();
                    static const QMap<QString, QString> map = {
                        {"cognac", "#a26a3d"}, {"espresso", "#4b2e22"}, {"sand", "#c8ad7f"},
                        {"oxblood", "#5a1111"}, {"deep forest green", "#1f4d3a"}, {"olive", "#6b7a3f"},
                        {"beige", "#d7c3a3"}, {"taupe", "#8d7b68"}, {"graphite", "#3d3d3d"},
                        {"off-white", "#f5f3ee"}, {"terracotta", "#c06c4d"}, {"burnt-orange", "#b75a2a"},
                        {"black", "#1c1c1c"}, {"charcoal", "#36454f"}, {"steel-blue", "#567a9e"},
                        {"lavender", "#9d8ec7"}, {"sage", "#9caf88"}, {"pearl", "#e8e4df"}
                    };
                    return map.value(n, "#8d5524");
                };

                auto mkKpiCard = [&](const QString &title, const QString &value, const QString &accent, bool star) -> QFrame* {
                    QFrame *f = new QFrame();
                    f->setStyleSheet(QStringLiteral(
                        "QFrame { background: #ffffff; border-radius: 14px; border: 1px solid #e8e0d0; }"
                        "QFrame:hover { background: #faf6f0; }"));
                    QVBoxLayout *vl = new QVBoxLayout(f);
                    vl->setContentsMargins(16, 16, 16, 16);
                    QLabel *t = new QLabel(title);
                    t->setStyleSheet("font-size: 9px; color: #9a8a70; font-weight: 800; text-transform: uppercase; letter-spacing: 1.2px;");
                    QLabel *v = new QLabel(value);
                    v->setWordWrap(true);
                    v->setStyleSheet(QString("font-size: %1px; color: #1a1208; font-weight: 900;")
                        .arg(star ? 24 : 24));
                    QFrame *line = new QFrame();
                    line->setFixedSize(40, 2);
                    line->setStyleSheet(QString("background: %1; border: none; border-radius: 1px;").arg(accent));
                    vl->addWidget(t);
                    vl->addSpacing(2);
                    vl->addWidget(v);
                    vl->addSpacing(6);
                    vl->addWidget(line);
                    applySoftShadow(f);
                    return f;
                };

                auto mkSectionTitle = [](const QString &txt) -> QLabel* {
                    QLabel *lb = new QLabel(txt);
                    lb->setStyleSheet("font-size: 14px; font-weight: 600; color: #2c1f0e; letter-spacing: 0.3px; margin-top: 8px; margin-bottom: 2px;");
                    return lb;
                };
                auto shortVisionLine = [](const QString &trend) -> QString {
                    const QString t = trend.toLower();
                    if (t.contains("athlux")) return QStringLiteral("Utility leads luxury future");
                    if (t.contains("conceptual")) return QStringLiteral("Experimental codes redefine desire");
                    if (t.contains("artisan")) return QStringLiteral("Craft returns with force");
                    if (t.contains("minimal")) return QStringLiteral("Precision quiets visual noise");
                    if (t.contains("romantic")) return QStringLiteral("Fluid softness regains momentum");
                    return QStringLiteral("Signals converge toward change");
                };

                const QString dominant = topStyles.value(0, "N/A");
                const QString confidenceTxt = QString::number(confidence * 100.0, 'f', 1) + "%";
                const double confPct = qBound(0.0, confidence * 100.0, 100.0);

                QFrame *hero = new QFrame();
                hero->setStyleSheet(
                    "QFrame {"
                    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #fffaf5, stop:0.55 #f7efe6, stop:1 #f3e6d8);"
                    "  border-radius: 16px;"
                    "  border: none;"
                    "}");
                QVBoxLayout *heroL = new QVBoxLayout(hero);
                heroL->setContentsMargins(20, 16, 20, 16);
                QLabel *heroTop = new QLabel(QString("Tendance dominante %1 : <b>%2</b>  |  Confiance IA : <b>%3</b>")
                                             .arg(targetYear).arg(dominant, confidenceTxt));
                heroTop->setTextFormat(Qt::RichText);
                heroTop->setStyleSheet("font-size: 17px; color: #4e342e; font-weight: 850; letter-spacing: 0.2px;");
                QLabel *heroSub = new QLabel("Cockpit décisionnel de prospective mode au service de la stratégie produit.");
                heroSub->setStyleSheet("font-size: 12px; color: #6d4c41; letter-spacing: 0.25px;");
                heroL->addWidget(heroTop);
                heroL->addWidget(heroSub);
                applySoftShadow(hero);
                l->addWidget(hero);

                QPushButton *btnVision = new QPushButton("Future Signals");
                btnVision->setStyleSheet(
                    "QPushButton { background-color: #2b1f1a; color: #f7e8d9; border-radius: 11px; padding: 10px 18px; font-size: 12px; font-weight: 900; letter-spacing: 0.9px; }"
                    "QPushButton:hover { background-color: #3b2a24; }");
                btnVision->setCursor(Qt::PointingHandCursor);
                btnVision->hide();

                auto fetchVisionFrame = [&](int y, const QString &scenario) -> QJsonObject {
                    return buildFashionForecastPayloadLocal(y, scenario);
                };
                connect(btnVision, &QPushButton::clicked, this, [=]() {
                    const QString scenario = ui->tabWidgetProduits->property("fashionOracleScenario").toString().isEmpty()
                        ? QStringLiteral("balanced")
                        : ui->tabWidgetProduits->property("fashionOracleScenario").toString();
                    QVector<QJsonObject> frames;
                    for (int y = 2035; y <= 2040; ++y) {
                        const QJsonObject fr = fetchVisionFrame(y, scenario);
                        if (!fr.isEmpty()) frames.append(fr);
                    }
                    if (frames.isEmpty()) {
                        QToolTip::showText(QCursor::pos(), QStringLiteral("Future Signals unavailable."));
                        return;
                    }

                    struct SignalItem {
                        QString title;
                        QString insight;
                        QString micro;
                        QString derived;
                        QString keyword;
                        int confidence;
                    };
                    auto shortWords = [](const QString &raw, int maxWords) -> QString {
                        const QStringList parts = raw.split(' ', Qt::SkipEmptyParts);
                        QStringList kept;
                        for (int i = 0; i < qMin(maxWords, parts.size()); ++i) kept << parts.at(i);
                        return kept.join(' ');
                    };
                    auto avgConceptScore = [](const QJsonArray &arr, const QString &key, double fallback) -> double {
                        if (arr.isEmpty()) return fallback;
                        double s = 0.0;
                        int n = 0;
                        for (const QJsonValue &v : arr) {
                            if (!v.isObject()) continue;
                            s += v.toObject().value(key).toDouble(fallback);
                            ++n;
                        }
                        return n > 0 ? (s / double(n)) : fallback;
                    };
                    auto arrayDelta = [](const QJsonArray &arr) -> double {
                        if (arr.size() < 2) return 0.0;
                        return arr.at(arr.size() - 1).toDouble(0.0) - arr.at(0).toDouble(0.0);
                    };
                    auto pct = [](double v) -> QString {
                        return QString("%1%").arg(v, 0, 'f', 1);
                    };

                    const QJsonObject f0 = frames.value(0);
                    const QJsonObject f1 = frames.value(1);
                    const QJsonObject fLast = frames.value(frames.size() - 1);
                    const int c0 = qBound(0, int(f0.value(QStringLiteral("confidence")).toDouble(0.5) * 100.0), 100);
                    const int c1 = qBound(0, int(f1.value(QStringLiteral("confidence")).toDouble(0.5) * 100.0), 100);
                    const int cLast = qBound(0, int(fLast.value(QStringLiteral("confidence")).toDouble(0.5) * 100.0), 100);
                    const QJsonObject trajLast = fLast.value(QStringLiteral("trend_trajectory")).toObject();
                    const QJsonObject trajStart = f0.value(QStringLiteral("trend_trajectory")).toObject();
                    const QJsonArray conflicts = trajLast.value(QStringLiteral("conflict_notes")).toArray();
                    const QJsonArray shifts = trajLast.value(QStringLiteral("dominance_shifts")).toArray();
                    const QJsonObject momenta = trajLast.value(QStringLiteral("trend_momentum")).toObject();
                    const QJsonArray conceptStart = f0.value(QStringLiteral("concepts_projection")).toArray();
                    const QJsonArray conceptEnd = fLast.value(QStringLiteral("concepts_projection")).toArray();
                    const QJsonObject rawStart = f0.value(QStringLiteral("raw_scores")).toObject();
                    const QJsonObject rawEnd = fLast.value(QStringLiteral("raw_scores")).toObject();

                    double minMomentum = 0.0;
                    bool minInit = false;
                    for (auto it = momenta.begin(); it != momenta.end(); ++it) {
                        const double v = it.value().toDouble(0.0);
                        if (!minInit || v < minMomentum) {
                            minMomentum = v;
                            minInit = true;
                        }
                    }
                    const int conflictStrength = qMin(22, conflicts.size() * 5);
                    const int shiftStrength = qMin(20, shifts.size() * 4);
                    const int accel = qMax(0, cLast - c0);
                    const double silhouetteAcc = avgConceptScore(conceptEnd, QStringLiteral("trend_timing_score"), 65.0)
                        - avgConceptScore(conceptStart, QStringLiteral("trend_timing_score"), 62.0);
                    const double materialDelta = arrayDelta(trajLast.value(QStringLiteral("material_sophistication")).toArray())
                        - arrayDelta(trajStart.value(QStringLiteral("material_sophistication")).toArray());
                    const double marketDelta = rawEnd.value(QStringLiteral("commercial_index")).toDouble(62.0)
                        - rawStart.value(QStringLiteral("commercial_index")).toDouble(62.0);
                    const double minMom = momenta.value(QStringLiteral("minimal-tailoring")).toDouble(0.0);
                    const double romMom = momenta.value(QStringLiteral("romantic-fluid")).toDouble(0.0);
                    const double conflictGap = minMom - romMom;
                    double leatherInnovation = 0.0;
                    int leatherN = 0;
                    for (const QJsonValue &v : conceptEnd) {
                        if (!v.isObject()) continue;
                        const QJsonObject o = v.toObject();
                        const QString mat = o.value(QStringLiteral("material")).toString().toLower();
                        if (!mat.contains("leather")) continue;
                        leatherInnovation += o.value(QStringLiteral("innovation_score")).toDouble(68.0);
                        ++leatherN;
                    }
                    leatherInnovation = leatherN > 0 ? leatherInnovation / double(leatherN) : avgConceptScore(conceptEnd, QStringLiteral("innovation_score"), 68.0);
                    const double marketAlignShift = avgConceptScore(conceptEnd, QStringLiteral("market_score"), 70.0)
                        - avgConceptScore(conceptStart, QStringLiteral("market_score"), 68.0);
                    const double oppMix = 0.6 * marketAlignShift + 0.4 * shiftStrength;

                    QVector<SignalItem> futureSignals;
                    futureSignals.append(SignalItem{
                        shortWords(QStringLiteral("Shape rupture"), 3),
                        shortWords(QStringLiteral("Silhouettes break away from material inertia"), 8),
                        QStringLiteral("Form language mutates faster than fabric evolution."),
                        QString("Derived from: silhouette acceleration %1 | material variation %2 | market signal %3")
                            .arg(pct(silhouetteAcc), pct(materialDelta), pct(marketDelta)),
                        QStringLiteral("silhouette"),
                        qBound(35, 58 + int(qAbs(silhouetteAcc) * 2.0), 96)
                    });
                    futureSignals.append(SignalItem{
                        shortWords(QStringLiteral("Aesthetic war"), 3),
                        shortWords(QStringLiteral("Minimal and romantic codes collide in-market"), 8),
                        QStringLiteral("Opposed style grammars create emotional buying friction."),
                        QString("Derived from: minimal momentum %1 | romantic momentum %2 | conflict spread %3")
                            .arg(pct(minMom), pct(romMom), pct(conflictGap)),
                        QStringLiteral("conflict"),
                        qBound(34, 55 + conflictStrength, 95)
                    });
                    futureSignals.append(SignalItem{
                        shortWords(QStringLiteral("Leather awakening"), 3),
                        shortWords(QStringLiteral("Leather innovation rises while legacy demand cools"), 8),
                        QStringLiteral("Craft material re-enters through experimental narratives."),
                        QString("Derived from: leather innovation %1 | weakest momentum %2 | confidence gain %3")
                            .arg(pct(leatherInnovation - 50.0), pct(minMomentum), pct(double(accel))),
                        QStringLiteral("leather"),
                        qBound(32, 53 + int((leatherInnovation - 50.0) * 0.7), 93)
                    });
                    futureSignals.append(SignalItem{
                        shortWords(QStringLiteral("Demand realignment"), 3),
                        shortWords(QStringLiteral("Market gravity shifts toward adaptive premium codes"), 8),
                        QStringLiteral("Commercial center abandons static luxury archetypes."),
                        QString("Derived from: market alignment %1 | commercial shift %2 | structure change %3")
                            .arg(pct(marketAlignShift), pct(marketDelta), pct(double(shiftStrength))),
                        QStringLiteral("market"),
                        qBound(36, 57 + int(qAbs(marketAlignShift) * 1.8), 97)
                    });
                    futureSignals.append(SignalItem{
                        shortWords(QStringLiteral("Critical opening"), 3),
                        shortWords(QStringLiteral("Asymmetric signals expose a high-impact launch window"), 8),
                        QStringLiteral("Timing asymmetry now rewards bold product positioning."),
                        QString("Derived from: shift pressure %1 | opportunity mix %2 | market signal %3")
                            .arg(pct(double(shiftStrength)), pct(oppMix), pct(marketDelta)),
                        QStringLiteral("opportunity"),
                        qBound(38, 60 + qMax(0, c1 - c0 / 2), 98)
                    });

                    QDialog *vision = new QDialog(this);
                    vision->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
                    vision->setModal(true);
                    vision->setStyleSheet("QDialog { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #0b0d12, stop:1 #15111b); }");
                    QVBoxLayout *rootVision = new QVBoxLayout(vision);
                    rootVision->setContentsMargins(28, 22, 28, 26);
                    rootVision->setSpacing(14);
                    QHBoxLayout *hudTop = new QHBoxLayout();
                    QLabel *liveLb = new QLabel("FUTURE SIGNALS // MACHINE INTELLIGENCE");
                    liveLb->setStyleSheet("font-size:12px; color:#f3bf8a; font-weight:900; letter-spacing:1.5px;");
                    QLabel *seqLb = new QLabel("dramatic inference stream");
                    seqLb->setStyleSheet("font-size:11px; color:#bca896; font-weight:700;");
                    hudTop->addWidget(liveLb);
                    hudTop->addStretch();
                    hudTop->addWidget(seqLb);
                    rootVision->addLayout(hudTop);
                    rootVision->addStretch();
                    QWidget *center = new QWidget(vision);
                    center->setStyleSheet("QWidget { background: transparent; }");
                    QVBoxLayout *cv = new QVBoxLayout(center);
                    cv->setSpacing(8);
                    cv->setAlignment(Qt::AlignCenter);
                    QLabel *titleLb = new QLabel("FUTURE SIGNAL #1");
                    titleLb->setAlignment(Qt::AlignCenter);
                    titleLb->setStyleSheet("font-size:18px; color:#9f8a7a; font-weight:900; letter-spacing:2.4px;");
                    QLabel *trendLb = new QLabel("HEADLINE");
                    trendLb->setAlignment(Qt::AlignCenter);
                    trendLb->setStyleSheet("font-size:56px; color:#f5d7b7; font-weight:900;");
                    QLabel *lineLb = new QLabel("Insight sentence");
                    lineLb->setAlignment(Qt::AlignCenter);
                    lineLb->setStyleSheet("font-size:22px; color:#e1c9b0; font-weight:800;");
                    QLabel *microLb = new QLabel("Micro explanation line");
                    microLb->setAlignment(Qt::AlignCenter);
                    microLb->setStyleSheet("font-size:16px; color:#b7a495; font-weight:600;");
                    QLabel *derivedLb = new QLabel("Derived from: metrics");
                    derivedLb->setAlignment(Qt::AlignCenter);
                    derivedLb->setWordWrap(true);
                    derivedLb->setStyleSheet("font-size:13px; color:#8f8278; font-weight:600;");
                    QLabel *confLb = new QLabel("0%");
                    confLb->setAlignment(Qt::AlignCenter);
                    confLb->setStyleSheet("font-size:30px; color:#f3bf8a; font-weight:900;");
                    cv->addWidget(titleLb);
                    cv->addWidget(trendLb);
                    cv->addWidget(lineLb);
                    cv->addWidget(microLb);
                    cv->addWidget(derivedLb);
                    cv->addSpacing(10);
                    cv->addWidget(confLb);
                    rootVision->addWidget(center, 0, Qt::AlignCenter);
                    rootVision->addStretch();
                    QPushButton *backBtn = new QPushButton("Back to dashboard");
                    backBtn->setCursor(Qt::PointingHandCursor);
                    backBtn->setStyleSheet("QPushButton { background: rgba(255,255,255,0.10); color:#f2dfcc; border:none; border-radius:10px; padding:9px 14px; font-weight:800; }"
                                           "QPushButton:hover { background: rgba(255,255,255,0.18); }");
                    rootVision->addWidget(backBtn, 0, Qt::AlignHCenter);
                    QObject::connect(backBtn, &QPushButton::clicked, vision, &QDialog::reject);

                    auto *fx = new QGraphicsOpacityEffect(center);
                    center->setGraphicsEffect(fx);
                    fx->setOpacity(0.0);
                    auto *blurFx = new QGraphicsBlurEffect(trendLb);
                    blurFx->setBlurRadius(16.0);
                    trendLb->setGraphicsEffect(blurFx);
                    auto *glowFx = new QGraphicsDropShadowEffect(titleLb);
                    glowFx->setBlurRadius(18.0);
                    glowFx->setOffset(0, 0);
                    glowFx->setColor(QColor(243, 191, 138, 110));
                    titleLb->setGraphicsEffect(glowFx);
                    const int stepMs = 1650;
                    QFrame *cutFlash = new QFrame(vision);
                    cutFlash->setStyleSheet("QFrame { background: rgba(255,245,232,0.0); border:none; }");
                    cutFlash->setGeometry(vision->rect());
                    cutFlash->hide();
                    auto *livePulse = new QVariantAnimation(liveLb);
                    livePulse->setDuration(1400);
                    livePulse->setStartValue(80);
                    livePulse->setEndValue(180);
                    livePulse->setLoopCount(-1);
                    QObject::connect(livePulse, &QVariantAnimation::valueChanged, liveLb, [=](const QVariant &v) {
                        const int a = qBound(70, v.toInt(), 200);
                        liveLb->setStyleSheet(QString("font-size:12px; color: rgba(243,191,138,%1); font-weight:900; letter-spacing:1.5px;").arg(a));
                    });
                    livePulse->start();
                    auto *idx = new int(0);
                    std::function<void()> playFrame;
                    playFrame = [=, &playFrame]() mutable {
                        if (!vision->isVisible()) { delete idx; return; }
                        if (*idx >= futureSignals.size()) {
                            titleLb->setText("FUTURE SIGNAL // END");
                            trendLb->setText("HIDDEN PATTERNS UNLOCKED");
                            lineLb->setText("Strategic intuition now has evidence");
                            microLb->setText("Signals converted into strategic action.");
                            derivedLb->setText("Derived from: multi-factor temporal synthesis");
                            confLb->setText("READY");
                            auto *finalBlur = new QPropertyAnimation(blurFx, "blurRadius", center);
                            finalBlur->setDuration(460);
                            finalBlur->setStartValue(8.0);
                            finalBlur->setEndValue(0.0);
                            finalBlur->start(QAbstractAnimation::DeleteWhenStopped);
                            QTimer::singleShot(1200, vision, [=]() { vision->accept(); });
                            delete idx;
                            return;
                        }
                        const SignalItem s = futureSignals.at(*idx);
                        titleLb->setText(QString("FUTURE SIGNAL #%1").arg(*idx + 1));
                        trendLb->setText(s.title.toUpper());
                        lineLb->setText("");
                        microLb->setText("");
                        derivedLb->setText("");
                        confLb->setText("0%");
                        const QPoint basePos = center->pos();
                        center->move(basePos.x(), basePos.y() + 14);
                        blurFx->setBlurRadius(15.0);
                        fx->setOpacity(0.0);

                        auto animateText = [vision](QLabel *lb, const QString &txt, int stepMs) {
                            lb->setText(QString());
                            auto *pos = new int(0);
                            QTimer *tt = new QTimer(lb);
                            QObject::connect(tt, &QTimer::timeout, lb, [=]() {
                                if (*pos >= txt.size()) {
                                    tt->stop();
                                    tt->deleteLater();
                                    delete pos;
                                    return;
                                }
                                lb->setText(txt.left(*pos + 1));
                                ++(*pos);
                            });
                            tt->start(stepMs);
                        };
                        animateText(lineLb, s.insight, 23);
                        QTimer::singleShot(180, vision, [=]() { animateText(microLb, s.micro, 20); });
                        QTimer::singleShot(360, vision, [=]() { animateText(derivedLb, s.derived, 13); });

                        auto *fadeIn = new QPropertyAnimation(fx, "opacity", center);
                        fadeIn->setDuration(620);
                        fadeIn->setStartValue(0.0);
                        fadeIn->setEndValue(1.0);
                        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
                        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

                        auto *rise = new QPropertyAnimation(center, "pos", center);
                        rise->setDuration(700);
                        rise->setStartValue(QPoint(basePos.x(), basePos.y() + 14));
                        rise->setEndValue(basePos);
                        rise->setEasingCurve(QEasingCurve::OutCubic);
                        rise->start(QAbstractAnimation::DeleteWhenStopped);

                        auto *blurIn = new QPropertyAnimation(blurFx, "blurRadius", center);
                        blurIn->setDuration(680);
                        blurIn->setStartValue(15.0);
                        blurIn->setEndValue(0.0);
                        blurIn->setEasingCurve(QEasingCurve::OutCubic);
                        blurIn->start(QAbstractAnimation::DeleteWhenStopped);

                        auto *glowAnim = new QVariantAnimation(titleLb);
                        glowAnim->setDuration(820);
                        glowAnim->setStartValue(70);
                        glowAnim->setEndValue(170);
                        glowAnim->setLoopCount(2);
                        QObject::connect(glowAnim, &QVariantAnimation::valueChanged, titleLb, [=](const QVariant &v) {
                            glowFx->setColor(QColor(243, 191, 138, qBound(40, v.toInt(), 190)));
                        });
                        glowAnim->start(QAbstractAnimation::DeleteWhenStopped);

                        auto *count = new QVariantAnimation(confLb);
                        count->setDuration(980);
                        count->setStartValue(0.0);
                        count->setEndValue(double(s.confidence));
                        QObject::connect(count, &QVariantAnimation::valueChanged, confLb, [=](const QVariant &v) {
                            confLb->setText(QString("%1%").arg(v.toDouble(), 0, 'f', 0));
                        });
                        count->start(QAbstractAnimation::DeleteWhenStopped);

                        QTimer::singleShot(stepMs - 360, vision, [=]() {
                            cutFlash->show();
                            auto *cutFx = new QGraphicsOpacityEffect(cutFlash);
                            cutFlash->setGraphicsEffect(cutFx);
                            cutFx->setOpacity(0.0);
                            auto *cutIn = new QPropertyAnimation(cutFx, "opacity", cutFlash);
                            cutIn->setDuration(120);
                            cutIn->setStartValue(0.0);
                            cutIn->setEndValue(0.22);
                            QObject::connect(cutIn, &QPropertyAnimation::finished, cutFlash, [=]() {
                                auto *cutOut = new QPropertyAnimation(cutFx, "opacity", cutFlash);
                                cutOut->setDuration(120);
                                cutOut->setStartValue(0.22);
                                cutOut->setEndValue(0.0);
                                QObject::connect(cutOut, &QPropertyAnimation::finished, cutFlash, [=]() { cutFlash->hide(); });
                                cutOut->start(QAbstractAnimation::DeleteWhenStopped);
                            });
                            cutIn->start(QAbstractAnimation::DeleteWhenStopped);
                            auto *fadeOut = new QPropertyAnimation(fx, "opacity", center);
                            fadeOut->setDuration(360);
                            fadeOut->setStartValue(1.0);
                            fadeOut->setEndValue(0.0);
                            fadeOut->setEasingCurve(QEasingCurve::InQuart);
                            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
                        });
                        ++(*idx);
                        QTimer::singleShot(stepMs, vision, playFrame);
                    };
                    QTimer::singleShot(90, vision, playFrame);
                    vision->showFullScreen();
                    vision->exec();
                    vision->deleteLater();
                });

                QGridLayout *cards = new QGridLayout();
                cards->setHorizontalSpacing(14);
                cards->setVerticalSpacing(14);
                cards->addWidget(mkKpiCard("Tendance dominante", dominant, "#8d5524", true), 0, 0);
                QFrame *ringCard = new QFrame();
                ringCard->setStyleSheet("QFrame { background: rgba(255,255,255,0.98); border-radius: 14px; border: none; }");
                QVBoxLayout *ringLay = new QVBoxLayout(ringCard);
                ringLay->setContentsMargins(16, 14, 16, 14);
                QLabel *ringTitle = new QLabel("Confiance IA");
                ringTitle->setStyleSheet("font-size: 10px; color: #8d6e63; font-weight: 800; text-transform: uppercase; letter-spacing: 1px;");
                QLabel *ringLabel = new QLabel();
                ringLabel->setAlignment(Qt::AlignCenter);
                QPixmap ringPx(96, 96);
                ringPx.fill(Qt::transparent);
                {
                    QPainter p(&ringPx);
                    p.setRenderHint(QPainter::Antialiasing, true);
                    QRectF rc(8, 8, 80, 80);
                    p.setPen(QPen(QColor("#f5f0e8"), 10));
                    p.drawArc(rc, 0, 360 * 16);
                    QColor ringColor = confPct >= 75.0 ? QColor("#2e7d32") : (confPct >= 50.0 ? QColor("#f9a825") : QColor("#c62828"));
                    p.setPen(QPen(ringColor, 10, Qt::SolidLine, Qt::RoundCap));
                    p.drawArc(rc, 90 * 16, static_cast<int>(-360.0 * 16.0 * (confPct / 100.0)));
                    p.setPen(QColor("#3e2723"));
                    QFont f("Segoe UI", 10, QFont::Bold);
                    p.setFont(f);
                    p.drawText(rc, Qt::AlignCenter, QString::number(confPct, 'f', 0) + "%");
                }
                ringLabel->setPixmap(ringPx);
                QLabel *ringText = new QLabel(confidenceTxt);
                ringText->setAlignment(Qt::AlignCenter);
                ringText->setStyleSheet("font-size: 15px; color: #3e2723; font-weight: 850;");
                ringLay->addWidget(ringTitle);
                ringLay->addWidget(ringLabel);
                ringLay->addWidget(ringText);
                applySoftShadow(ringCard);
                cards->addWidget(ringCard, 0, 1);
                cards->addWidget(mkKpiCard("Année cible", QString::number(targetYear), "#795548", false), 0, 2);
                cards->addWidget(mkKpiCard("Palette dominante", palette.value(0, "N/A"), "#6d4c41", false), 0, 3);
                cards->addWidget(mkKpiCard("Matière clé", fabrics.value(0, "N/A"), "#5d4037", false), 1, 0);
                cards->addWidget(mkKpiCard("Silhouette clé", sil.value(0, "N/A"), "#5d4037", false), 1, 1);
                cards->addWidget(mkKpiCard("Moteur", inferenceMode.isEmpty() ? "api" : inferenceMode, "#3e2723", false), 1, 2);
                cards->addWidget(mkKpiCard("Source", "Fashion Oracle API", "#3e2723", false), 1, 3);
                l->addLayout(cards);

                // DEBUG TEMPORAIRE: scores de tendance pour validation du winner.
                {
                    const ForecastResultLocal debugForecast = computeFashionForecastLocal(targetYear, scenarioModeUi);
                    const std::array<double, 6> &debugScores = debugForecast.trendScores;
                    const QStringList debugNames = {
                        QStringLiteral("artisan-leather"),
                        QStringLiteral("minimal-tailoring"),
                        QStringLiteral("romantic-fluid"),
                        QStringLiteral("athlux"),
                        QStringLiteral("conceptual"),
                        QStringLiteral("annual")
                    };
                    const int debugWinner = argmaxLocal(debugScores);
                    const QString debugWinnerName = debugNames.value(debugWinner);
                    QFrame *debugFrame = new QFrame();
                    debugFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.92); border: 1px dashed #d0c2b1; border-radius: 8px; }");
                    QVBoxLayout *debugLay = new QVBoxLayout(debugFrame);
                    debugLay->setContentsMargins(10, 8, 10, 8);
                    debugLay->setSpacing(3);
                    QLabel *title = new QLabel(QString("DEBUG SCORES year=%1").arg(targetYear));
                    title->setStyleSheet("font-size: 10px; color: #5d4037; font-weight: 800;");
                    debugLay->addWidget(title);
                    for (int i = 0; i < debugNames.size(); ++i) {
                        const QString line = QString("[%1] %2 = %3 %4")
                                                 .arg(i)
                                                 .arg(debugNames.at(i))
                                                 .arg(QString::number(debugScores[i], 'f', 1))
                                                 .arg(i == debugWinner ? QStringLiteral("<-- WINNER") : QString());
                        QLabel *lb = new QLabel(line);
                        lb->setStyleSheet("font-size: 10px; color: #6d4c41; font-family: 'DM Mono';");
                        debugLay->addWidget(lb);
                    }
                    QLabel *winnerLb = new QLabel(QString("WINNER idx=%1 name=%2").arg(debugWinner).arg(debugWinnerName));
                    winnerLb->setStyleSheet("font-size: 10px; color: #2e7d32; font-weight: 800; font-family: 'DM Mono';");
                    debugLay->addWidget(winnerLb);
                    l->addWidget(debugFrame);
                }

                QFrame *swatchFrame = new QFrame();
                swatchFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.96); border-radius: 14px; border: none; }");
                QHBoxLayout *swatchLay = new QHBoxLayout(swatchFrame);
                swatchLay->setContentsMargins(16, 11, 16, 11);
                swatchLay->setSpacing(11);
                QLabel *swTitle = new QLabel("Palette tendance");
                swTitle->setStyleSheet("font-size: 11px; color: #8d6e63; font-weight: 800; text-transform: uppercase; letter-spacing: 0.9px;");
                swatchLay->addWidget(swTitle);
                const int swCount = qMin(5, qMax(1, palette.size()));
                for (int i = 0; i < swCount; ++i) {
                    const QString pName = palette.value(i, "N/A");
                    const QString hex = (i < paletteHex.size() && !paletteHex.at(i).isEmpty())
                        ? paletteHex.at(i)
                        : colorHex(pName);
                    QFrame *chip = new QFrame();
                    chip->setStyleSheet(QString("background:%1; border:none; border-radius:10px;").arg(hex));
                    chip->setFixedSize(24, 24);
                    QLabel *name = new QLabel(pName);
                    name->setStyleSheet("font-size: 12px; color: #4e342e; font-weight: 700;");
                    swatchLay->addWidget(chip);
                    swatchLay->addWidget(name);
                    swatchLay->addSpacing(6);
                }
                swatchLay->addStretch();
                applySoftShadow(swatchFrame);
                l->addWidget(swatchFrame);

                l->addWidget(mkSectionTitle("Projection Mode IA"));
                QFrame *visualFrame = new QFrame();
                visualFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.98); border: none; border-radius: 16px; }");
                QVBoxLayout *visualRoot = new QVBoxLayout(visualFrame);
                visualRoot->setContentsMargins(16, 14, 16, 14);
                visualRoot->setSpacing(12);

                auto mkScoreBar = [](int value, const QString &chunkColor, int delayMs) -> QProgressBar* {
                    QProgressBar *pb = new QProgressBar();
                    pb->setRange(0, 100);
                    const int clamped = qBound(0, value, 100);
                    pb->setValue(0);
                    pb->setTextVisible(false);
                    pb->setFixedHeight(8);
                    pb->setStyleSheet(
                        QString("QProgressBar { border: 0; border-radius: 4px; background: #f2ebe4; }"
                                "QProgressBar::chunk { background: %1; border-radius: 4px; }")
                            .arg(chunkColor));
                    QTimer::singleShot(qMax(0, delayMs), pb, [pb, clamped]() {
                        auto *anim = new QPropertyAnimation(pb, "value", pb);
                        anim->setDuration(680);
                        anim->setStartValue(0);
                        anim->setEndValue(clamped);
                        anim->setEasingCurve(QEasingCurve::OutCubic);
                        anim->start(QAbstractAnimation::DeleteWhenStopped);
                    });
                    return pb;
                };
                auto phaseFromWindow = [](int prevV, int currV, int nextV) -> QString {
                    const int momentum = currV - prevV;
                    const int forward = nextV - currV;
                    const bool flat = (qAbs(momentum) <= 1 && qAbs(forward) <= 1);
                    if (currV >= 90 && momentum >= 0 && forward < 0) return QStringLiteral("Peak");
                    if (currV <= 22 && momentum <= 1 && forward > 0) return QStringLiteral("Bottom");
                    if (flat) return QStringLiteral("Stable");
                    if (momentum > 1 || (momentum >= 0 && forward > 1)) return QStringLiteral("Rising");
                    if (momentum < -1 || (momentum <= 0 && forward < -1)) return QStringLiteral("Declining");
                    return QStringLiteral("Stable");
                };
                auto phaseIcon = [](const QString &phase) -> QString {
                    if (phase == QStringLiteral("Peak")) return QStringLiteral("▲");
                    if (phase == QStringLiteral("Bottom")) return QStringLiteral("▼");
                    if (phase == QStringLiteral("Rising")) return QStringLiteral("↗");
                    if (phase == QStringLiteral("Declining")) return QStringLiteral("↘");
                    return QStringLiteral("■");
                };
                auto sparklineFor = [](const QJsonArray &arr) -> QString {
                    static const QString bars = QStringLiteral("▁▂▃▄▅▆▇█");
                    if (arr.isEmpty()) return QStringLiteral("▁▂▃▄");
                    int minV = 999;
                    int maxV = -999;
                    for (const QJsonValue &v : arr) {
                        const int n = qBound(0, v.toInt(0), 100);
                        minV = qMin(minV, n);
                        maxV = qMax(maxV, n);
                    }
                    if (maxV <= minV) return QStringLiteral("▄▄▄▄");
                    QString out;
                    for (const QJsonValue &v : arr) {
                        const int n = qBound(0, v.toInt(0), 100);
                        const int idx = qBound(0, ((n - minV) * 7) / qMax(1, maxV - minV), 7);
                        out.append(bars.at(idx));
                    }
                    return out;
                };

                const QString dominantShift = obj.value(QStringLiteral("dominant_direction")).toString(QStringLiteral("Technical Luxury"));
                const QString marketPosture = obj.value(QStringLiteral("market_posture")).toString(QStringLiteral("Balanced selective growth"));
                const QString riskLevelGlobal = obj.value(QStringLiteral("risk_level")).toString(QStringLiteral("Medium"));
                const QString recommendedCapsule = obj.value(QStringLiteral("recommended_capsule")).toString(QStringLiteral("Capsule premium"));
                const QString strongestSignal = obj.value(QStringLiteral("strongest_signal")).toString(QStringLiteral("Signal en transition"));
                const QString yoyEvolution = obj.value(QStringLiteral("year_over_year_evolution")).toString(
                    QString("Signal %1 en accélération vs %2.").arg(targetYear).arg(targetYear - 1));
                const QJsonObject rawScoresObj = obj.value(QStringLiteral("raw_scores")).toObject();
                const QString scenarioMode = rawScoresObj.value(QStringLiteral("scenario_mode")).toString(QStringLiteral("balanced"));
                const QString scenarioLabel =
                    (scenarioMode == QStringLiteral("aggressive_growth")) ? QStringLiteral("Aggressive Growth")
                    : (scenarioMode == QStringLiteral("risk_averse")) ? QStringLiteral("Risk Averse")
                    : (scenarioMode == QStringLiteral("disruptive")) ? QStringLiteral("Disruptive")
                    : (scenarioMode == QStringLiteral("conservative")) ? QStringLiteral("Conservative")
                    : QStringLiteral("Balanced");
                const double volatilityProxy = rawScoresObj.value(QStringLiteral("volatility_proxy")).toDouble(0.0);
                const double uncertaintyPenalty = rawScoresObj.value(QStringLiteral("uncertainty_penalty")).toDouble(0.0);
                const double temporalCertainty = rawScoresObj.value(QStringLiteral("temporal_certainty")).toDouble(0.0);
                const double signalCoherence = rawScoresObj.value(QStringLiteral("signal_coherence")).toDouble(0.0);
                const double rankingGap = rawScoresObj.value(QStringLiteral("ranking_gap")).toDouble(0.0);
                const QJsonArray conceptArr = obj.value(QStringLiteral("concepts_projection")).toArray();

                int innovationAvg = 72;
                int marketAvg = 72;
                int feasibilityAvg = 70;
                if (!conceptArr.isEmpty()) {
                    int sumI = 0, sumM = 0, sumF = 0, n = 0;
                    for (const QJsonValue &v : conceptArr) {
                        if (!v.isObject()) continue;
                        const QJsonObject co = v.toObject();
                        sumI += co.value(QStringLiteral("innovation_score")).toInt(70);
                        sumM += co.value(QStringLiteral("market_score")).toInt(70);
                        sumF += co.value(QStringLiteral("feasibility_score")).toInt(70);
                        ++n;
                    }
                    if (n > 0) {
                        innovationAvg = sumI / n;
                        marketAvg = sumM / n;
                        feasibilityAvg = sumF / n;
                    }
                }
                const int confidenceSignal = qBound(45, static_cast<int>(confPct), 98);
                const int momentumSignal = qBound(35, (innovationAvg + marketAvg) / 2, 96);
                const int riskSignal = qBound(10, 100 - feasibilityAvg, 95);
                const QString riskBg = (riskSignal >= 56) ? QStringLiteral("#8e2d2d")
                                                          : (riskSignal <= 34) ? QStringLiteral("#2e7d32")
                                                                               : QStringLiteral("#b26a00");

                QFrame *heroFrame = new QFrame();
                heroFrame->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #342118, stop:1 #4e342e); border-radius: 14px; }");
                QHBoxLayout *heroLay = new QHBoxLayout(heroFrame);
                heroLay->setContentsMargins(14, 12, 14, 12);
                heroLay->setSpacing(10);
                auto mkHeroChip = [](const QString &label, const QString &value, const QString &bg) -> QLabel* {
                    QLabel *lb = new QLabel(QString("%1  %2").arg(label, value));
                    lb->setAlignment(Qt::AlignCenter);
                    lb->setMinimumWidth(120);
                    lb->setMinimumHeight(32);
                    lb->setStyleSheet(QString("font-size:11px; font-weight:800; color:#fff; background:%1; border-radius:6px; padding:4px 10px;").arg(bg));
                    return lb;
                };
                QLabel *heroTitle = new QLabel(QString("Forecast Cockpit %1").arg(targetYear));
                heroTitle->setStyleSheet("font-size:17px; color:#fff; font-weight:900;");
                QLabel *heroDir = new QLabel(dominantShift.toUpper());
                heroDir->setStyleSheet("font-size:12px; color:#ffe0c2; font-weight:900; letter-spacing:0.7px;");
                QLabel *heroWhy = new QLabel(strongestSignal);
                heroWhy->setStyleSheet("font-size:11px; color:#f0dfd1; font-weight:700;");
                QVBoxLayout *heroLeft = new QVBoxLayout();
                heroLeft->setSpacing(3);
                heroLeft->addWidget(heroTitle);
                heroLeft->addWidget(heroDir);
                heroLeft->addWidget(heroWhy);
                QHBoxLayout *heroRight = new QHBoxLayout();
                heroRight->setSpacing(6);
                heroRight->addWidget(mkHeroChip(QStringLiteral("Confiance"), QString("%1%").arg(confidenceSignal), "#3a2a1a"));
                heroRight->addWidget(mkHeroChip(QStringLiteral("Scenario"), scenarioLabel, "#2a3a2a"));
                heroRight->addWidget(mkHeroChip(QStringLiteral("Momentum"), QString::number(momentumSignal), "#2a2a3a"));
                heroRight->addWidget(mkHeroChip(QStringLiteral("Risque"), riskLevelGlobal, riskBg));
                heroRight->addStretch();
                heroLay->addLayout(heroLeft, 2);
                heroLay->addLayout(heroRight, 3);
                visualRoot->addWidget(heroFrame);

                QJsonObject traj = obj.value(QStringLiteral("trend_trajectory")).toObject();
                const QJsonArray yearsA = traj.value(QStringLiteral("years")).toArray();
                const QJsonArray artisanA = traj.value(QStringLiteral("artisan_leather_curve")).toArray();
                const QJsonArray romanticA = traj.value(QStringLiteral("romantic_fluid_curve")).toArray();
                const QJsonArray athluxA = traj.value(QStringLiteral("athlux_utility_curve")).toArray();
                const QJsonArray minimalA = traj.value(QStringLiteral("minimal_tailoring_curve")).toArray();
                const QJsonArray conceptualA = traj.value(QStringLiteral("conceptual_futurism_curve")).toArray();
                const QJsonArray annualA = traj.value(QStringLiteral("annual_curve")).toArray();
                auto arrValue = [](const QJsonArray &a, int idx, int fallback) -> int {
                    if (idx < 0 || idx >= a.size()) return fallback;
                    return a.at(idx).toInt(fallback);
                };
                const int trendCenterIdx = yearsA.isEmpty() ? 0 : (yearsA.size() / 2);
                const QString dominantTrendId = traj.value(QStringLiteral("dominant_trend")).toString();
                const QString nextDominantTrendId = traj.value(QStringLiteral("next_dominant_trend")).toString();
                const QJsonArray dominanceShiftsA = traj.value(QStringLiteral("dominance_shifts")).toArray();
                const QJsonArray shockEventsA = traj.value(QStringLiteral("shock_events")).toArray();
                const QJsonArray conflictNotesA = traj.value(QStringLiteral("conflict_notes")).toArray();
                const QJsonArray overtakesA = traj.value(QStringLiteral("overtakes")).toArray();
                auto momentumFrom = [&](const QJsonArray &a) -> int {
                    if (a.size() < 2) return 0;
                    const int idx = qBound(1, trendCenterIdx, a.size() - 1);
                    return arrValue(a, idx, 50) - arrValue(a, idx - 1, 50);
                };
                auto mkTrendChip = [&](const QString &trendId, const QString &name, const QJsonArray &curve, const QString &bg) -> QWidget* {
                    QFrame *f = new QFrame();
                    const bool isDominant = (trendId == dominantTrendId);
                    f->setFixedSize(120, 64);
                    f->setStyleSheet(QString("QFrame { background:%1; border-radius:10px; border:%2 solid %3; }")
                                         .arg(isDominant ? QStringLiteral("#fdf5e8") : bg)
                                         .arg(isDominant ? 2 : 1)
                                         .arg(isDominant ? QStringLiteral("#d4841a") : QStringLiteral("#eadfce")));
                    QHBoxLayout *lay = new QHBoxLayout(f);
                    lay->setContentsMargins(8, 6, 8, 6);
                    lay->setSpacing(5);
                    const int m = momentumFrom(curve);
                    const int idx = qBound(0, trendCenterIdx, curve.size() - 1);
                    const int prevV = arrValue(curve, qMax(0, idx - 1), arrValue(curve, idx, 50));
                    const int currV = arrValue(curve, idx, 50);
                    const int nextV = arrValue(curve, qMin(curve.size() - 1, idx + 1), currV);
                    const QString phase = phaseFromWindow(prevV, currV, nextV);
                    QLabel *n = new QLabel(name);
                    n->setStyleSheet("font-size:11px; color:#2c1f0e; font-weight:900;");
                    const QString phaseBg = (phase == QStringLiteral("Rising")) ? QStringLiteral("#2e7d32")
                        : (phase == QStringLiteral("Declining")) ? QStringLiteral("#8e2d2d")
                        : (phase == QStringLiteral("Peak")) ? QStringLiteral("#8d5524")
                        : (phase == QStringLiteral("Bottom")) ? QStringLiteral("#4e342e")
                        : QStringLiteral("#6d4c41");
                    QLabel *phaseLb = new QLabel(QString("%1 %2").arg(phaseIcon(phase), phase));
                    phaseLb->setStyleSheet(QString("font-size:10px; color:#fff; font-weight:900; background:%1; border-radius:7px; padding:2px 6px;").arg(phaseBg));
                    QLabel *mom = new QLabel(QString("M%1%2").arg(m >= 0 ? "+" : "").arg(m));
                    const QString momBg = (m > 0) ? QStringLiteral("#d4841a")
                                                  : (m < 0 ? QStringLiteral("#c04040")
                                                           : QStringLiteral("#9a8a70"));
                    mom->setStyleSheet(QString("font-size:10px; color:#fff; font-weight:800; background:%1; border-radius:8px; padding:1px 6px;").arg(momBg));
                    auto *spark = new MiniTrendSparkline();
                    spark->setSeries(curve, m < 0 ? QColor("#c04040") : (m > 0 ? QColor("#d4841a") : QColor("#9a8a70")));
                    lay->addWidget(n);
                    lay->addWidget(phaseLb);
                    lay->addWidget(mom);
                    lay->addWidget(spark);
                    if (isDominant) {
                        QLabel *dom = new QLabel(QStringLiteral("LEAD"));
                        dom->setStyleSheet("font-size:9px; color:#fff; font-weight:900; background:#8d5524; border-radius:6px; padding:1px 5px;");
                        lay->addWidget(dom);
                    }
                    lay->addStretch();
                    phaseLb->setToolTip(QString("phase=%1\nmomentum=%2\nvolatility=%3\nuncertainty=%4")
                        .arg(phase)
                        .arg(m)
                        .arg(volatilityProxy, 0, 'f', 1)
                        .arg(uncertaintyPenalty, 0, 'f', 1));
                    auto *phaseFx = new QGraphicsOpacityEffect(phaseLb);
                    phaseLb->setGraphicsEffect(phaseFx);
                    auto *phaseAnim = new QPropertyAnimation(phaseFx, "opacity", phaseLb);
                    phaseAnim->setDuration(860);
                    phaseAnim->setStartValue(0.45);
                    phaseAnim->setEndValue(1.0);
                    phaseAnim->setLoopCount(2);
                    phaseAnim->setEasingCurve(QEasingCurve::InOutSine);
                    phaseAnim->start(QAbstractAnimation::DeleteWhenStopped);
                    return f;
                };
                QFrame *evolutionFrame = new QFrame();
                evolutionFrame->setStyleSheet("QFrame { background: #faf6f1; border-radius: 12px; border: 1px solid #ecdfd1; }");
                QVBoxLayout *evoLay = new QVBoxLayout(evolutionFrame);
                evoLay->setContentsMargins(10, 9, 10, 9);
                QLabel *evoTitle = new QLabel(QStringLiteral("Trend Evolution Strip"));
                evoTitle->setStyleSheet("font-size:14px; color:#2c1f0e; font-weight:600;");
                evoLay->addWidget(evoTitle);
                QHBoxLayout *trendStrip = new QHBoxLayout();
                trendStrip->setSpacing(7);
                trendStrip->addWidget(mkTrendChip(QStringLiteral("artisan-leather"), QStringLiteral("Artisan"), artisanA, "#fff8f1"));
                trendStrip->addWidget(mkTrendChip(QStringLiteral("romantic-fluid"), QStringLiteral("Romantic"), romanticA, "#fff6fb"));
                trendStrip->addWidget(mkTrendChip(QStringLiteral("athlux-utility"), QStringLiteral("Athlux"), athluxA, "#f4f9ff"));
                trendStrip->addWidget(mkTrendChip(QStringLiteral("minimal-tailoring"), QStringLiteral("Minimal"), minimalA, "#f8f8f8"));
                trendStrip->addWidget(mkTrendChip(QStringLiteral("conceptual-futurism"), QStringLiteral("Conceptual"), conceptualA, "#f8f4ff"));
                trendStrip->addWidget(mkTrendChip(QStringLiteral("annual"), QStringLiteral("Annual"), annualA, "#fff6e8"));
                evoLay->addLayout(trendStrip);
                visualRoot->addWidget(evolutionFrame);

                QFrame *reasoningStrip = new QFrame();
                reasoningStrip->setStyleSheet("QFrame { background:#f2ebe4; border:1px solid #eadbc8; border-radius:10px; }");
                QHBoxLayout *reasoningLay = new QHBoxLayout(reasoningStrip);
                reasoningLay->setContentsMargins(10, 7, 10, 7);
                QString shiftsTxt = QStringLiteral("no overtakes");
                if (!dominanceShiftsA.isEmpty()) {
                    QStringList chunks;
                    for (int i = 0; i < qMin(2, dominanceShiftsA.size()); ++i) {
                        chunks << dominanceShiftsA.at(i).toString().left(30);
                    }
                    shiftsTxt = chunks.join(QStringLiteral(" | "));
                }
                QString overtakeTxt = QStringLiteral("none");
                if (!overtakesA.isEmpty()) {
                    overtakeTxt = overtakesA.at(0).toString().left(28);
                }
                QLabel *reasonLbl = new QLabel(QString("▲ %1   DOM: %2 -> NEXT: %3   X: %4   OT: %5   ■ Reco: %6")
                    .arg(yoyEvolution.left(42),
                         dominantTrendId.left(18),
                         nextDominantTrendId.left(18),
                         shiftsTxt,
                         overtakeTxt,
                         recommendedCapsule.left(18)));
                reasonLbl->setStyleSheet("font-size: 11px; font-weight: 800; color: #5d4037;");
                reasoningLay->addWidget(reasonLbl);
                visualRoot->addWidget(reasoningStrip);

                QFrame *shockFrame = new QFrame();
                shockFrame->setStyleSheet("QFrame { background:#fff8ef; border:1px solid #ecdcc8; border-radius:10px; }");
                QHBoxLayout *shockLay = new QHBoxLayout(shockFrame);
                shockLay->setContentsMargins(10, 6, 10, 6);
                QLabel *shockTitle = new QLabel(QStringLiteral("Shock & Conflict"));
                shockTitle->setStyleSheet("font-size:11px; font-weight:900; color:#c04040;");
                shockLay->addWidget(shockTitle);
                if (!shockEventsA.isEmpty()) {
                    const QJsonObject evt = shockEventsA.at(0).toObject();
                    QLabel *evtLb = new QLabel(QString("⚡ %1").arg(evt.value(QStringLiteral("label")).toString().left(28)));
                    evtLb->setStyleSheet("font-size:11px; color:#8B4513; font-weight:700; background:#fff0e0; border:1px solid #d4841a; border-radius:12px; padding:4px 10px;");
                    shockLay->addWidget(evtLb);
                    auto *evtFx = new QGraphicsOpacityEffect(evtLb);
                    evtLb->setGraphicsEffect(evtFx);
                    auto *evtAnim = new QPropertyAnimation(evtFx, "opacity", evtLb);
                    evtAnim->setDuration(980);
                    evtAnim->setStartValue(0.35);
                    evtAnim->setEndValue(1.0);
                    evtAnim->setLoopCount(3);
                    evtAnim->setEasingCurve(QEasingCurve::InOutSine);
                    evtAnim->start(QAbstractAnimation::DeleteWhenStopped);
                }
                const int conflictShown = qMin(2, conflictNotesA.size());
                for (int i = 0; i < conflictShown; ++i) {
                    QLabel *cf = new QLabel(conflictNotesA.at(i).toString().left(30));
                    cf->setStyleSheet("font-size:11px; color:#8B4513; font-weight:700; background:#fff0e0; border:1px solid #d4841a; border-radius:12px; padding:4px 10px;");
                    shockLay->addWidget(cf);
                }
                shockLay->addStretch();
                visualRoot->addWidget(shockFrame);

                if (conflictNotesA.size() > 0) {
                    QFrame *mxFrame = new QFrame();
                    mxFrame->setStyleSheet("QFrame { background:#fff; border:1px solid #e0d8cc; border-radius:8px; }");
                    QVBoxLayout *mxLay = new QVBoxLayout(mxFrame);
                    mxLay->setContentsMargins(12, 10, 12, 10);
                    mxLay->setSpacing(6);
                    QLabel *mxTitle = new QLabel(QStringLiteral("MATRICE DE CONFLIT"));
                    mxTitle->setStyleSheet("font-size:9px; font-weight:900; letter-spacing:1.2px; color:#8d6e63;");
                    mxLay->addWidget(mxTitle);
                    const QStringList mxNames = {QStringLiteral("artisan"), QStringLiteral("minimal"),
                                                 QStringLiteral("romantic"), QStringLiteral("athlux"),
                                                 QStringLiteral("conceptual")};
                    const double baseConflicts[5][5] = {
                        { 0, 27, 12, 45, 18},
                        {15,  0, 33, 22, 41},
                        {26, 17,  0, 38, 29},
                        {37, 28, 19,  0, 52},
                        {48, 39, 24, 31,  0},
                    };
                    double mult = 1.0;
                    if (scenarioMode == QStringLiteral("aggressive_growth")) mult = 1.3;
                    else if (scenarioMode == QStringLiteral("risk_averse")) mult = 0.7;
                    else if (scenarioMode == QStringLiteral("disruptive")) mult = 1.5;
                    else if (scenarioMode == QStringLiteral("conservative")) mult = 0.8;
                    QGridLayout *grid = new QGridLayout();
                    grid->setHorizontalSpacing(2);
                    grid->setVerticalSpacing(2);
                    for (int c = 0; c < 5; ++c) {
                        QLabel *h = new QLabel(mxNames.at(c));
                        h->setAlignment(Qt::AlignCenter);
                        h->setMinimumWidth(52);
                        h->setStyleSheet("font-size:10px; color:#7f6a58; font-weight:800; background:#f0ece4; padding:4px 2px;");
                        grid->addWidget(h, 0, c + 1);
                    }
                    for (int r = 0; r < 5; ++r) {
                        QLabel *v = new QLabel(mxNames.at(r));
                        v->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        v->setMinimumWidth(90);
                        v->setStyleSheet("font-size:10px; color:#7f6a58; font-weight:800; background:#f0ece4; padding-right:6px;");
                        grid->addWidget(v, r + 1, 0);
                        for (int c = 0; c < 5; ++c) {
                            QLabel *cell = new QLabel;
                            cell->setFixedSize(52, 26);
                            const int val = (r == c) ? 0 : qBound(0, int(std::round(baseConflicts[r][c] * mult)), 100);
                            cell->setText(r == c ? QStringLiteral("—") : QString::number(val));
                            QString bg = QStringLiteral("transparent");
                            QString fg = QStringLiteral("#8B7355");
                            if (r == c) { bg = QStringLiteral("#e8e4dc"); fg = QStringLiteral("#8B7355"); }
                            else if (val >= 61) { bg = QStringLiteral("#c04040"); fg = QStringLiteral("#ffffff"); }
                            else if (val >= 31) { bg = QStringLiteral("#d4841a"); fg = QStringLiteral("#ffffff"); }
                            else if (val >= 1) { bg = QStringLiteral("#f5e8d0"); fg = QStringLiteral("#8B4513"); }
                            cell->setAlignment(Qt::AlignCenter);
                            cell->setStyleSheet(QString("QLabel{background:%1; color:%2; border:1px solid #e0d8cc; border-radius:0px; font-size:10px; font-weight:800;}").arg(bg, fg));
                            grid->addWidget(cell, r + 1, c + 1);
                        }
                    }
                    mxLay->addLayout(grid);
                    visualRoot->addWidget(mxFrame);
                }

                QFrame *trajFrame = new QFrame();
                trajFrame->setStyleSheet("QFrame { background: #faf7f3; border-radius: 12px; border: 1px solid #ece1d6; }");
                QVBoxLayout *trajLay = new QVBoxLayout(trajFrame);
                trajLay->setContentsMargins(10, 10, 10, 10);
                QLabel *trajTitle = new QLabel(QStringLiteral("Interactive Curve Panel"));
                trajTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #2c1f0e;");
                trajLay->addWidget(trajTitle);
                auto mkSeries = [](const QString &name, const QColor &color) -> QLineSeries* {
                    QLineSeries *s = new QLineSeries();
                    s->setName(name);
                    QPen p(color, 2.0);
                    p.setCapStyle(Qt::RoundCap);
                    s->setPen(p);
                    s->setPointsVisible(false);
                    return s;
                };
                QLineSeries *sArtisan = mkSeries(QStringLiteral("artisan-leather"), QColor("#6d4c41"));
                QLineSeries *sRomantic = mkSeries(QStringLiteral("romantic-fluid"), QColor("#a45d7f"));
                QLineSeries *sAthlux = mkSeries(QStringLiteral("athlux-utility"), QColor("#2e5d7a"));
                QLineSeries *sMinimal = mkSeries(QStringLiteral("minimal-tailoring"), QColor("#7f6a58"));
                QLineSeries *sConcept = mkSeries(QStringLiteral("conceptual-futurism"), QColor("#7a3f99"));
                QLineSeries *sAnnual = mkSeries(QStringLiteral("annual"), QColor("#8d5524"));
                auto mkGhost = [](const QString &name, const QColor &color) -> QLineSeries* {
                    QLineSeries *g = new QLineSeries();
                    g->setName(name);
                    QPen p(color, 1.5, Qt::DashLine);
                    p.setColor(QColor(color.red(), color.green(), color.blue(), 120));
                    g->setPen(p);
                    return g;
                };
                QLineSeries *gArtisan = mkGhost(QStringLiteral("artisan-leather projection"), QColor("#6d4c41"));
                QLineSeries *gRomantic = mkGhost(QStringLiteral("romantic-fluid projection"), QColor("#a45d7f"));
                QLineSeries *gAthlux = mkGhost(QStringLiteral("athlux-utility projection"), QColor("#2e5d7a"));
                QLineSeries *gMinimal = mkGhost(QStringLiteral("minimal-tailoring projection"), QColor("#7f6a58"));
                QLineSeries *gConcept = mkGhost(QStringLiteral("conceptual-futurism projection"), QColor("#7a3f99"));
                QLineSeries *gAnnual = mkGhost(QStringLiteral("annual projection"), QColor("#8d5524"));
                const int nPts = qMax(3, qMin(yearsA.size(), qMax(artisanA.size(), annualA.size())));
                int minYear = targetYear - 2;
                int maxYear = targetYear + 2;
                QVector<QPointF> artPts, romPts, athPts, minPts, conPts, annPts;
                artPts.reserve(nPts); romPts.reserve(nPts); athPts.reserve(nPts); minPts.reserve(nPts); conPts.reserve(nPts); annPts.reserve(nPts);
                for (int i = 0; i < nPts; ++i) {
                    const int x = arrValue(yearsA, i, targetYear - 2 + i);
                    minYear = qMin(minYear, x);
                    maxYear = qMax(maxYear, x);
                    artPts.append(QPointF(x, arrValue(artisanA, i, 58)));
                    romPts.append(QPointF(x, arrValue(romanticA, i, 55)));
                    athPts.append(QPointF(x, arrValue(athluxA, i, 62)));
                    minPts.append(QPointF(x, arrValue(minimalA, i, 60)));
                    conPts.append(QPointF(x, arrValue(conceptualA, i, 57)));
                    annPts.append(QPointF(x, arrValue(annualA, i, 58)));
                }
                const int pivotIdx = qBound(0, trendCenterIdx, qMax(0, nPts - 1));
                for (int i = pivotIdx; i < nPts; ++i) {
                    gArtisan->append(artPts.value(i));
                    gRomantic->append(romPts.value(i));
                    gAthlux->append(athPts.value(i));
                    gMinimal->append(minPts.value(i));
                    gConcept->append(conPts.value(i));
                    gAnnual->append(annPts.value(i));
                }
                QLineSeries *yearMarker = new QLineSeries();
                yearMarker->setName(QStringLiteral("Année cible"));
                QPen markerPen(QColor("#c28b58"));
                markerPen.setStyle(Qt::DashLine);
                markerPen.setWidth(2);
                yearMarker->setPen(markerPen);
                yearMarker->append(targetYear, 0);
                yearMarker->append(targetYear, 100);
                QChart *trajChart = new QChart();
                trajChart->addSeries(sArtisan);
                trajChart->addSeries(sRomantic);
                trajChart->addSeries(sAthlux);
                trajChart->addSeries(sMinimal);
                trajChart->addSeries(sConcept);
                trajChart->addSeries(sAnnual);
                trajChart->addSeries(gArtisan);
                trajChart->addSeries(gRomantic);
                trajChart->addSeries(gAthlux);
                trajChart->addSeries(gMinimal);
                trajChart->addSeries(gConcept);
                trajChart->addSeries(gAnnual);
                trajChart->addSeries(yearMarker);
                styleChartBase(trajChart);
                trajChart->setAnimationOptions(QChart::SeriesAnimations);
                trajChart->legend()->setAlignment(Qt::AlignBottom);
                QFont legendFont = trajChart->legend()->font();
                legendFont.setPointSizeF(9.0);
                trajChart->legend()->setFont(legendFont);
                QValueAxis *axX = new QValueAxis();
                QValueAxis *axY = new QValueAxis();
                axX->setLabelFormat("%d");
                axX->setTickCount(qBound(4, nPts, 8));
                axX->setRange(minYear, maxYear);
                axY->setRange(0, 100);
                axY->setTickCount(6);
                axX->setGridLineVisible(false);
                axY->setGridLineColor(QColor("#eee4d8"));
                trajChart->addAxis(axX, Qt::AlignBottom);
                trajChart->addAxis(axY, Qt::AlignLeft);
                QScatterSeries *overtakeSeries = new QScatterSeries();
                overtakeSeries->setName(QStringLiteral("Trend overtakes"));
                overtakeSeries->setMarkerSize(9.0);
                overtakeSeries->setColor(QColor("#d84315"));
                auto appendOvertakes = [&](const QVector<QPointF> &a, const QVector<QPointF> &b) {
                    for (int i = 1; i < qMin(a.size(), b.size()); ++i) {
                        const double d1 = a[i - 1].y() - b[i - 1].y();
                        const double d2 = a[i].y() - b[i].y();
                        if ((d1 < 0 && d2 > 0) || (d1 > 0 && d2 < 0)) {
                            overtakeSeries->append(a[i].x(), (a[i].y() + b[i].y()) * 0.5);
                        }
                    }
                };
                appendOvertakes(artPts, athPts);
                appendOvertakes(romPts, athPts);
                appendOvertakes(conPts, artPts);
                trajChart->addSeries(overtakeSeries);
                const QList<QLineSeries*> allSeries = {sArtisan, sRomantic, sAthlux, sMinimal, sConcept, sAnnual, gArtisan, gRomantic, gAthlux, gMinimal, gConcept, gAnnual, yearMarker};
                for (QLineSeries *s : allSeries) {
                    s->attachAxis(axX);
                    s->attachAxis(axY);
                    QObject::connect(s, &QLineSeries::hovered, trajChart, [s](const QPointF &point, bool state) {
                        if (!state) return;
                        QToolTip::showText(QCursor::pos(),
                            QString("%1  •  %2 : %3")
                                .arg(s->name())
                                .arg(static_cast<int>(point.x()))
                                .arg(static_cast<int>(point.y())));
                    });
                }
                overtakeSeries->attachAxis(axX);
                overtakeSeries->attachAxis(axY);
                QObject::connect(overtakeSeries, &QScatterSeries::hovered, trajChart, [confidenceSignal, volatilityProxy, uncertaintyPenalty](const QPointF &point, bool state) {
                    if (!state) return;
                    QToolTip::showText(
                        QCursor::pos(),
                        QString("trend overtakes @ %1\nconfidence=%2%%\nvolatility=%3  uncertainty=%4")
                            .arg(static_cast<int>(point.x()))
                            .arg(confidenceSignal)
                            .arg(volatilityProxy, 0, 'f', 1)
                            .arg(uncertaintyPenalty, 0, 'f', 1));
                });
                sArtisan->clear(); sRomantic->clear(); sAthlux->clear(); sMinimal->clear(); sConcept->clear(); sAnnual->clear();
                QTimer *drawTimer = new QTimer(trajChart);
                int *drawIdx = new int(0);
                QObject::connect(drawTimer, &QTimer::timeout, trajChart, [=]() mutable {
                    const int i = *drawIdx;
                    if (i >= nPts) {
                        drawTimer->stop();
                        drawTimer->deleteLater();
                        delete drawIdx;
                        return;
                    }
                    sArtisan->append(artPts.value(i));
                    sRomantic->append(romPts.value(i));
                    sAthlux->append(athPts.value(i));
                    sMinimal->append(minPts.value(i));
                    sConcept->append(conPts.value(i));
                    sAnnual->append(annPts.value(i));
                    *drawIdx = i + 1;
                });
                drawTimer->start(110);
                QChartView *trajView = new QChartView(trajChart);
                styleChartView(trajView);
                trajView->setRubberBand(QChartView::HorizontalRubberBand);
                trajView->setMinimumHeight(280);
                trajView->setToolTip(QString("Confidence breakdown\nTemporal: %1\nCoherence: %2\nRanking gap: %3\nVolatility: %4\nUncertainty penalty: %5")
                    .arg(temporalCertainty, 0, 'f', 1)
                    .arg(signalCoherence, 0, 'f', 1)
                    .arg(rankingGap, 0, 'f', 1)
                    .arg(volatilityProxy, 0, 'f', 1)
                    .arg(uncertaintyPenalty, 0, 'f', 1));
                trajLay->addWidget(trajView);
                visualRoot->addWidget(trajFrame);

                QHBoxLayout *conceptLay = new QHBoxLayout();
                conceptLay->setSpacing(10);
                auto mkInfoChip = [](const QString &txt) -> QLabel* {
                    QLabel *lb = new QLabel(txt);
                    lb->setStyleSheet("font-size:10px; font-weight:800; color:#4e342e; background:#f3e9de; border-radius:8px; padding:3px 7px;");
                    return lb;
                };
                auto mkMetricRow = [&](const QString &label, int val, const QString &chunkColor) -> QWidget* {
                    QWidget *row = new QWidget();
                    QHBoxLayout *rh = new QHBoxLayout(row);
                    rh->setContentsMargins(0, 0, 0, 0);
                    rh->setSpacing(8);
                    QLabel *ll = new QLabel(label);
                    ll->setMinimumWidth(66);
                    ll->setStyleSheet("font-size:9px; color:#8B7355; letter-spacing:0.02em;");
                    QProgressBar *pb = new QProgressBar();
                    pb->setRange(0, 100);
                    pb->setValue(qBound(0, val, 100));
                    pb->setTextVisible(false);
                    pb->setFixedHeight(4);
                    pb->setStyleSheet(QString("QProgressBar { border:0; border-radius:2px; background:#e8e0d0; }"
                                              "QProgressBar::chunk { background:%1; border-radius:2px; }").arg(chunkColor));
                    QLabel *vv = new QLabel(QString::number(val));
                    vv->setMinimumWidth(22);
                    vv->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    vv->setStyleSheet("font-size:9px; color:#4a3828; font-weight:700;");
                    rh->addWidget(ll);
                    rh->addWidget(pb, 1);
                    rh->addWidget(vv);
                    return row;
                };
                for (int i = 0; i < 3; ++i) {
                    const QJsonObject c = (i < conceptArr.size() && conceptArr.at(i).isObject()) ? conceptArr.at(i).toObject() : QJsonObject();
                    const QString title = c.value(QStringLiteral("product_name")).toString(QString("Concept %1").arg(i + 1));
                    const QString style = c.value(QStringLiteral("style")).toString(topStyles.value(i, topStyles.value(0, QStringLiteral("N/A"))));
                    const QString pal = c.value(QStringLiteral("palette")).toString(palette.value(i, palette.value(0, QStringLiteral("N/A"))));
                    const QString mat = c.value(QStringLiteral("material")).toString(fabrics.value(i, fabrics.value(0, QStringLiteral("N/A"))));
                    const QString silh = c.value(QStringLiteral("silhouette")).toString(sil.value(i, sil.value(0, QStringLiteral("N/A"))));
                    const int innovation = c.value(QStringLiteral("innovation_score")).toInt(64 + i * 5);
                    const int market = c.value(QStringLiteral("market_score")).toInt(74 - i * 4);
                    const int feasibility = c.value(QStringLiteral("feasibility_score")).toInt(72 - i * 5);
                    const int timing = c.value(QStringLiteral("trend_timing_score")).toInt(68 - i * 3);
                    const QString rationale = c.value(QStringLiteral("direction_badge")).toString(QStringLiteral("Fit signal marché + différenciation contrôlée."));
                    const QString phase = phaseFromWindow(qBound(0, timing - 5, 100), timing, qBound(0, timing + (innovation - 50) / 10, 100));

                    QFrame *card = new QFrame();
                    card->setAttribute(Qt::WA_Hover, true);
                    card->setStyleSheet(
                        "QFrame { background: #fff; border: 1px solid #efe4d8; border-radius: 13px; }"
                        "QFrame:hover { border: 1px solid #c28b58; background:#fffdf9; }");
                    QVBoxLayout *cv = new QVBoxLayout(card);
                    cv->setContentsMargins(10, 9, 10, 9);
                    cv->setSpacing(6);
                    QLabel *t = new QLabel(title);
                    t->setWordWrap(true);
                    t->setStyleSheet("font-size: 13px; font-weight: 900; color: #3e2723;");
                    QLabel *phaseBadge = new QLabel(QString("%1 %2").arg(phaseIcon(phase), phase));
                    phaseBadge->setStyleSheet("font-size:10px; font-weight:900; color:#fff; background:#6d4c41; border-radius:7px; padding:2px 7px;");
                    QHBoxLayout *tRow = new QHBoxLayout();
                    tRow->addWidget(t);
                    tRow->addWidget(phaseBadge, 0, Qt::AlignRight);
                    cv->addLayout(tRow);
                    QHBoxLayout *chipRow = new QHBoxLayout();
                    chipRow->setSpacing(5);
                    chipRow->addWidget(mkInfoChip(style));
                    chipRow->addWidget(mkInfoChip(pal));
                    chipRow->addWidget(mkInfoChip(mat));
                    chipRow->addWidget(mkInfoChip(silh));
                    chipRow->addStretch();
                    cv->addLayout(chipRow);
                    cv->addWidget(mkMetricRow(QStringLiteral("Market fit"), innovation, "#d4841a"));
                    cv->addWidget(mkMetricRow(QStringLiteral("Matière"), market, "#8B5E3C"));
                    cv->addWidget(mkMetricRow(QStringLiteral("Silhouette"), feasibility, "#6B7F5E"));
                    cv->addWidget(mkMetricRow(QStringLiteral("Global"), timing, "#2c5f8a"));
                    QLabel *oneLine = new QLabel(rationale.left(78));
                    oneLine->setStyleSheet("font-size: 10px; color: #5d4037; font-weight: 700;");
                    cv->addWidget(oneLine);
                    conceptLay->addWidget(card);
                }
                visualRoot->addLayout(conceptLay);

                const QJsonArray rejectedArr = obj.value(QStringLiteral("rejected_concepts")).toArray();
                QFrame *rejFrame = new QFrame();
                rejFrame->setStyleSheet("QFrame { background: #fff7ef; border-radius: 10px; border: 1px solid #f0decd; }");
                QHBoxLayout *rejLay = new QHBoxLayout(rejFrame);
                rejLay->setContentsMargins(10, 7, 10, 7);
                QLabel *rejTitle = new QLabel(QStringLiteral("Rejected Concepts"));
                rejTitle->setStyleSheet("font-size: 11px; font-weight: 900; color: #7a4b2a;");
                rejLay->addWidget(rejTitle);
                const int maxRejectedShown = qMin(3, rejectedArr.size());
                for (int i = 0; i < maxRejectedShown; ++i) {
                    const QString line = rejectedArr.at(i).toString().trimmed();
                    if (line.isEmpty()) continue;
                    QLabel *li = new QLabel(line.left(64));
                    li->setStyleSheet("font-size: 10px; color: #6d4c41; font-weight:700; background:#f8eadb; border-radius:7px; padding:3px 6px;");
                    rejLay->addWidget(li);
                }
                rejLay->addStretch();
                visualRoot->addWidget(rejFrame);

                auto *breathFx = new QGraphicsOpacityEffect(visualFrame);
                visualFrame->setGraphicsEffect(breathFx);
                auto *breathAnim = new QPropertyAnimation(breathFx, "opacity", visualFrame);
                breathAnim->setStartValue(0.93);
                breathAnim->setEndValue(1.0);
                breathAnim->setDuration(2200);
                breathAnim->setEasingCurve(QEasingCurve::InOutSine);
                breathAnim->setLoopCount(-1);
                breathAnim->start(QAbstractAnimation::DeleteWhenStopped);

                auto *fadeEffect = new QGraphicsOpacityEffect(visualFrame);
                visualFrame->setGraphicsEffect(fadeEffect);
                auto *fadeIn = new QPropertyAnimation(fadeEffect, "opacity", visualFrame);
                fadeIn->setDuration(360);
                fadeIn->setStartValue(0.0);
                fadeIn->setEndValue(1.0);
                fadeIn->setEasingCurve(QEasingCurve::OutCubic);
                fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

                applySoftShadow(visualFrame);
                l->addWidget(visualFrame);

                l->addWidget(mkSectionTitle("Top 3 tendances - Ranking éditorial"));
                QHBoxLayout *rankLay = new QHBoxLayout();
                rankLay->setSpacing(12);
                const QStringList medals = {"#D4AF37", "#C0C0C0", "#CD7F32"};
                for (int i = 0; i < qMin(3, topStyles.size()); ++i) {
                    QFrame *card = new QFrame();
                    card->setStyleSheet("QFrame { background: rgba(255,255,255,0.98); border: none; border-radius: 14px; }");
                    card->setMinimumHeight(180);
                    QVBoxLayout *vl = new QVBoxLayout(card);
                    vl->setContentsMargins(16, 14, 16, 14);
                    QLabel *rank = new QLabel(QString("TOP %1").arg(i + 1));
                    rank->setStyleSheet(QString("font-size: 11px; font-weight: 900; color: %1;").arg(medals.value(i, "#8d5524")));
                    QLabel *sty = new QLabel(topStyles.value(i));
                    sty->setStyleSheet("font-size: 16px; font-weight: 900; color: #3e2723;");
                    QLabel *meta = new QLabel(QString("Palette: %1\nMatière: %2\nSilhouette: %3")
                                              .arg(palette.value(i, palette.value(0, "-")))
                                              .arg(fabrics.value(i, fabrics.value(0, "-")))
                                              .arg(sil.value(i, sil.value(0, "-"))));
                    meta->setStyleSheet("font-size: 12px; color: #6d4c41;");
                    meta->setWordWrap(true);
                    QProgressBar *pb = new QProgressBar();
                    pb->setRange(0, 100);
                    const int relScore = qBound(40, static_cast<int>(confPct) - i * 8, 100);
                    pb->setValue(relScore);
                    pb->setFormat(QString::number(relScore) + "%");
                    pb->setStyleSheet(
                        "QProgressBar { border: 1px solid #e0d7cc; border-radius: 7px; background: #faf8f5; text-align:center; }"
                        "QProgressBar::chunk { background: #8d5524; border-radius: 6px; }");
                    vl->addWidget(rank);
                    vl->addWidget(sty);
                    vl->addWidget(meta);
                    vl->addWidget(pb);
                    applySoftShadow(card);
                    rankLay->addWidget(card);
                }
                l->addLayout(rankLay);

                l->addSpacing(10);
                {
                    l->addWidget(mkSectionTitle("RADAR MOMENTUM TENDANCES"));
                    QFrame *radarFrame = new QFrame();
                    radarFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.97); border:1px solid #e0d8cc; border-radius:8px; }");
                    QVBoxLayout *radLay = new QVBoxLayout(radarFrame);
                    radLay->setContentsMargins(14, 14, 14, 14);
                    QLabel *radar = new QLabel();
                    radar->setAlignment(Qt::AlignCenter);
                    QPixmap px(300, 300);
                    px.fill(Qt::transparent);
                    QPainter p(&px);
                    p.setRenderHint(QPainter::Antialiasing, true);
                    const QPointF c(px.width() * 0.5, px.height() * 0.5);
                    const qreal R = 120.0;
                    const QStringList axisN = {QStringLiteral("artisan"), QStringLiteral("minimal"), QStringLiteral("romantic"),
                                               QStringLiteral("athlux"), QStringLiteral("conceptual"), QStringLiteral("annual")};
                    const ForecastResultLocal frNow = computeFashionForecastLocal(targetYear, scenarioModeUi);
                    const ForecastResultLocal frPrev = computeFashionForecastLocal(targetYear - 1, scenarioModeUi);
                    QVector<QPointF> ptsNow, ptsPrev;
                    constexpr qreal kPi = 3.14159265358979323846;
                    for (int i = 0; i < 6; ++i) {
                        const qreal a = -kPi * 0.5 + (kPi * 2.0 * i / 6.0);
                        const QPointF tip(c.x() + std::cos(a) * R, c.y() + std::sin(a) * R);
                        p.setPen(QPen(QColor("#d8cfc3"), 1));
                        p.drawLine(c, tip);
                        p.setPen(QColor("#7f6a58"));
                        p.setFont(QFont("Segoe UI", 8));
                        p.drawText(QRectF(tip.x() - 32, tip.y() - 8, 64, 16), Qt::AlignCenter, axisN.at(i));
                        const qreal rn = qBound(0.0, frNow.trendScores[i], 100.0) / 100.0 * R;
                        const qreal rp = qBound(0.0, frPrev.trendScores[i], 100.0) / 100.0 * R;
                        ptsNow.push_back(QPointF(c.x() + std::cos(a) * rn, c.y() + std::sin(a) * rn));
                        ptsPrev.push_back(QPointF(c.x() + std::cos(a) * rp, c.y() + std::sin(a) * rp));
                    }
                    QPainterPath prevPath, nowPath;
                    prevPath.addPolygon(QPolygonF(ptsPrev));
                    nowPath.addPolygon(QPolygonF(ptsNow));
                    p.setPen(QPen(QColor("#6d655c"), 1.4));
                    p.setBrush(QColor(100, 90, 80, 38));
                    p.drawPath(prevPath);
                    p.setPen(QPen(QColor("#d4841a"), 1.8));
                    p.setBrush(QColor(212, 132, 26, 64));
                    p.drawPath(nowPath);
                    p.setPen(Qt::NoPen);
                    p.setBrush(QColor("#d4841a"));
                    p.drawEllipse(c, 4, 4);
                    radar->setPixmap(px);
                    radLay->addWidget(radar);
                    l->addWidget(radarFrame);
                }

                QHBoxLayout *middle = new QHBoxLayout();
                middle->setSpacing(14);
                QVBoxLayout *leftCol = new QVBoxLayout();
                QVBoxLayout *rightCol = new QVBoxLayout();
                leftCol->setSpacing(12);
                rightCol->setSpacing(12);

                QFrame *histFrame = new QFrame();
                histFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.97); border: none; border-radius: 14px; }");
                QVBoxLayout *histLay = new QVBoxLayout(histFrame);
                histLay->setContentsMargins(16, 14, 16, 14);
                histLay->addWidget(mkSectionTitle("Inspirations historiques"));
                if (decades.isEmpty()) {
                    QLabel *empty = new QLabel("Aucune correspondance historique détectée.");
                    empty->setStyleSheet("font-size: 12px; color: #8d6e63;");
                    histLay->addWidget(empty);
                } else {
                    auto describeDecade = [](const QString &d) {
                        if (d.contains("199")) return QString("Minimal structure et palette neutre.");
                        if (d.contains("200")) return QString("Utility leather revival et esprit urbain.");
                        if (d.contains("201")) return QString("Premium casual fonctionnel.");
                        if (d.contains("202")) return QString("Influence durable et matières responsables.");
                        return QString("Correspondance stylistique pertinente.");
                    };

                    QScrollArea *timelineScroll = new QScrollArea();
                    timelineScroll->setWidgetResizable(true);
                    timelineScroll->setFrameShape(QFrame::NoFrame);
                    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    QWidget *timelineW = new QWidget();
                    QHBoxLayout *timeline = new QHBoxLayout(timelineW);
                    timeline->setContentsMargins(2, 2, 2, 2);
                    timeline->setSpacing(8);
                    const int currentY = QDate::currentDate().year();
                    for (int y = 2020; y <= 2035; ++y) {
                        QVBoxLayout *node = new QVBoxLayout();
                        node->setSpacing(3);
                        QPushButton *dot = new QPushButton();
                        dot->setCursor(Qt::PointingHandCursor);
                        dot->setFixedSize(y == targetYear ? 10 : 8, y == targetYear ? 10 : 8);
                        if (y < currentY) {
                            dot->setStyleSheet("QPushButton{background:#d4841a;border:none;border-radius:5px;}");
                        } else if (y == currentY) {
                            dot->setStyleSheet("QPushButton{background:#d4841a;border:1px solid #8d5524;border-radius:5px;}");
                        } else {
                            dot->setStyleSheet("QPushButton{background:transparent;border:1px solid #8d5524;border-radius:5px;}");
                        }
                        QLabel *d = new QLabel(QString::number(y));
                        d->setAlignment(Qt::AlignCenter);
                        d->setStyleSheet("font-size:9px; color:#5d4037; font-weight:850;");
                        QLabel *nm = new QLabel(y <= targetYear ? topStyles.value((y - 2020) % qMax(1, topStyles.size()), dominant)
                                                                 : QString("<i>%1</i>").arg(topStyles.value((y - 2020) % qMax(1, topStyles.size()), dominant)));
                        nm->setAlignment(Qt::AlignCenter);
                        nm->setStyleSheet("font-size:9px; color:#8d6e63;");
                        nm->setTextFormat(Qt::RichText);
                        nm->setMinimumWidth(68);
                        node->addWidget(dot, 0, Qt::AlignHCenter);
                        node->addWidget(d);
                        node->addWidget(nm);
                        QWidget *nodeW = new QWidget();
                        nodeW->setLayout(node);
                        timeline->addWidget(nodeW);
                        QObject::connect(dot, &QPushButton::clicked, this, [this, y]() {
                            ui->tabWidgetProduits->setProperty("fashionOracleTargetYear", y);
                            ui->tabWidgetProduits->setProperty("fashionOracleActiveYear", y);
                            QTimer::singleShot(0, this, [this]() { showHistoriqueModeDialog(); });
                        });
                        if (y < 2035) {
                            QFrame *connector = new QFrame();
                            connector->setFixedHeight(1);
                            connector->setFixedWidth(20);
                            connector->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            connector->setStyleSheet("background:#dbc8b5; border:none;");
                            timeline->addWidget(connector, 0, Qt::AlignVCenter);
                        }
                    }
                    timeline->addStretch();
                    timelineScroll->setWidget(timelineW);
                    histLay->addWidget(timelineScroll);
                    histLay->addSpacing(3);

                    const int timelineCount = qMin(4, decades.size());
                    for (int i = 0; i < timelineCount; ++i) {
                        const QString d = decades.value(i);
                        QLabel *li = new QLabel(QString("<b>%1</b> - %2").arg(d, describeDecade(d)));
                        li->setTextFormat(Qt::RichText);
                        li->setStyleSheet("font-size: 12px; color: #6d4c41; padding-top:2px; line-height:1.45;");
                        histLay->addWidget(li);
                    }
                }
                applySoftShadow(histFrame);
                leftCol->addWidget(histFrame);

                const QString chartTrendB64 = obj.value(QStringLiteral("chart_trend_base64")).toString();
                const QString chartCycleB64 = obj.value(QStringLiteral("chart_cycle_base64")).toString();
                if (!chartTrendB64.isEmpty() || !chartCycleB64.isEmpty()) {
                    QFrame *chartFrame = new QFrame();
                    chartFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.97); border: none; border-radius: 14px; }");
                    QVBoxLayout *chartLay = new QVBoxLayout(chartFrame);
                    chartLay->setContentsMargins(14, 14, 14, 14);
                    chartLay->addWidget(mkSectionTitle("Visual storytelling"));
                    auto addChart = [chartLay](const QString &b64, const QString &title) {
                        if (b64.isEmpty()) return;
                        QLabel *t = new QLabel(title);
                        t->setStyleSheet("font-size: 12px; font-weight: 850; color: #5d4037; margin-top: 2px;");
                        QLabel *img = new QLabel();
                        img->setAlignment(Qt::AlignCenter);
                        QPixmap px;
                        if (px.loadFromData(QByteArray::fromBase64(b64.toUtf8()), "PNG"))
                            img->setPixmap(px.scaled(520, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        chartLay->addWidget(t);
                        chartLay->addWidget(img);
                    };
                    addChart(chartTrendB64, "Évolution des signaux tendance");
                    addChart(chartCycleB64, "Distribution de similarité historique");
                    applySoftShadow(chartFrame);
                    leftCol->addWidget(chartFrame);
                }

                QFrame *recFrame = new QFrame();
                recFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.97); border: none; border-radius: 14px; }");
                QVBoxLayout *recLay = new QVBoxLayout(recFrame);
                recLay->setContentsMargins(16, 14, 16, 14);
                recLay->addWidget(mkSectionTitle("Recommandation stratégique produit"));
                auto boolToBusiness = [](const QString &k, const QString &v) -> QString {
                    if (v.compare("true", Qt::CaseInsensitive) == 0)
                        return QString("Prioriser cet axe dans la prochaine capsule.");
                    if (v.compare("false", Qt::CaseInsensitive) == 0)
                        return QString("Axe secondaire à tester avec prudence.");
                    return v;
                };
                auto recoLine = [recLay](const QString &k, const QString &v) {
                    QLabel *lb = new QLabel(QString("• <b>%1</b> : %2").arg(k, v));
                    lb->setTextFormat(Qt::RichText);
                    lb->setWordWrap(true);
                    lb->setStyleSheet("font-size: 12px; color: #5d4037; line-height:1.45;");
                    recLay->addWidget(lb);
                };
                recoLine("Axe produit", QString("Développer une capsule %1 orientée désirabilité premium.").arg(dominant));
                recoLine("Matières recommandées", fabrics.mid(0, 3).join(", "));
                recoLine("Palette conseillée", palette.mid(0, 4).join(", "));
                recoLine("Silhouettes à privilégier", sil.mid(0, 3).join(", "));
                if (recommandations.isEmpty()) {
                    recoLine("Opportunité marché", "Fenêtre favorable sur segment premium utilitaire.");
                    recoLine("Niveau de risque / audace", "Audace modérée, risque contrôlé.");
                } else {
                    for (auto it = recommandations.begin(); it != recommandations.end(); ++it)
                        recoLine(it.key(), boolToBusiness(it.key(), it.value()));
                    recoLine("Niveau de risque / audace", confidence >= 0.75 ? "Risque faible, audace soutenue possible." : "Risque modéré, pilotage progressif.");
                }
                applySoftShadow(recFrame);
                rightCol->addWidget(recFrame);

                QFrame *historySummaryFrame = new QFrame();
                historySummaryFrame->setStyleSheet("QFrame { background: rgba(255,255,255,0.97); border: none; border-radius: 14px; }");
                QVBoxLayout *sumLay = new QVBoxLayout(historySummaryFrame);
                sumLay->setContentsMargins(16, 14, 16, 14);
                sumLay->addWidget(mkSectionTitle(QStringLiteral("Historique de mode")));
                const QString fallbackSummary = QString(
                    "Pour %1, les signaux prédictifs convergent vers une montée de %2, "
                    "soutenue par des matières nobles, des palettes terreuses sophistiquées et des silhouettes fonctionnelles raffinées. "
                    "Cette orientation ouvre une opportunité forte pour une ligne conciliant désirabilité, usage quotidien et perception qualité.")
                    .arg(targetYear).arg(dominant);
                QLabel *sum = new QLabel(summary.isEmpty() ? fallbackSummary : summary);
                sum->setWordWrap(true);
                sum->setStyleSheet("font-size: 12px; color: #5d4037; line-height: 1.55;");
                sumLay->addWidget(sum);
                applySoftShadow(historySummaryFrame);
                rightCol->addWidget(historySummaryFrame);
                rightCol->addStretch();

                middle->addLayout(leftCol, 3);
                middle->addLayout(rightCol, 2);
                l->addLayout(middle);

                l->addWidget(mkSectionTitle("Détail analytique complet"));
                QTableWidget *tw = new QTableWidget();
                tw->setColumnCount(6);
                int rowsApi = qMax(1, qMax(qMax(topStyles.size(), palette.size()), qMax(fabrics.size(), sil.size())));
                tw->setRowCount(rowsApi);
                tw->setHorizontalHeaderLabels({"Rang", "Style", "Palette", "Matière", "Silhouette", "Poids"});
                tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                tw->verticalHeader()->setDefaultSectionSize(32);
                tw->setAlternatingRowColors(true);
                tw->setShowGrid(false);
                tw->setStyleSheet(
                    "QTableWidget { background:rgba(255,255,255,0.98); border:none; border-radius:12px; font-size:12px; }"
                    "QHeaderView::section { background: #2f1b16; color: #f8eee4; padding: 10px; font-weight: 850; border: none; }"
                    "QTableWidget::item { border-bottom: 1px solid #f0e7dc; padding: 6px; }"
                    "QTableWidget::item:selected { background: #f9efe3; color: #3e2723; }");
                for (int i = 0; i < rowsApi; ++i) {
                    const QString medal = (i == 0 ? "🥇" : (i == 1 ? "🥈" : (i == 2 ? "🥉" : "•")));
                    tw->setItem(i, 0, new QTableWidgetItem(QString("%1 %2").arg(medal).arg(i + 1)));
                    tw->setItem(i, 1, new QTableWidgetItem(i < topStyles.size() ? topStyles[i] : QString()));
                    tw->setItem(i, 2, new QTableWidgetItem(i < palette.size() ? palette[i] : QString()));
                    tw->setItem(i, 3, new QTableWidgetItem(i < fabrics.size() ? fabrics[i] : QString()));
                    tw->setItem(i, 4, new QTableWidgetItem(i < sil.size() ? sil[i] : QString()));
                    const int weight = qBound(35, static_cast<int>(confPct) - i * 7, 99);
                    tw->setItem(i, 5, new QTableWidgetItem(QString("%1%").arg(weight)));
                }
                applySoftShadow(tw);
                l->addWidget(tw);

                loadingBox->hide();
                ui->tabWidgetProduits->setCurrentIndex(5);
                if (reply)
                    reply->deleteLater();
                return;
            }
            if (reply)
                reply->deleteLater();
        }
    }

LOCAL_PREDICTION_FALLBACK:
    {
        loadingBox->hide();
        qDebug() << "[FashionOracle] predict API failed after retries year=" << targetYear;
        QLabel *badgeErr = new QLabel(
            QStringLiteral("API Fashion Oracle indisponible après plusieurs tentatives (timeout). "
                           "Vérifiez que le backend tourne sur le bon port (FASHION_ORACLE_PORT) et réessayez."));
        badgeErr->setStyleSheet("background: #fdecea; color: #7f1d1d; border:1px solid #f8caca; padding: 12px; border-radius: 10px;");
        badgeErr->setWordWrap(true);
        badgeErr->setAlignment(Qt::AlignCenter);
        l->addWidget(badgeErr);
        QLabel *hint = new QLabel(
            QStringLiteral("Aucun repli local : les résultats affichés doivent provenir de l’API réelle."));
        hint->setStyleSheet("font-size:12px; color:#6d4c41; padding: 10px;");
        hint->setWordWrap(true);
        hint->setAlignment(Qt::AlignCenter);
        l->addWidget(hint);
        ui->tabWidgetProduits->setCurrentIndex(5);
        return;
    }
    QSqlDatabase db = Connexion::getInstance() ? Connexion::getInstance()->getDatabase() : QSqlDatabase();
    if (!db.isOpen()) {
        QLabel *msg = new QLabel("Base Oracle non connectee: prediction indisponible.");
        msg->setStyleSheet("background: #fff3cd; color: #856404; border:1px solid #ffeeba; padding: 12px; border-radius: 8px;");
        msg->setAlignment(Qt::AlignCenter);
        l->addWidget(msg);
        ui->tabWidgetProduits->setCurrentIndex(5);
        return;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT NVL(TRIM(pr.COLLECTION), 'INCONNUE') AS COLLECTION, "
        "TO_CHAR(NVL(p.DATE_LANCEMENT, NVL(p.DATE_FIN_PREVUE, SYSDATE)), 'YYYY-MM') AS YM, "
        "SUM(NVL(p.QUANTITE, 1)) AS QTE "
        "FROM PLANIFICATION p "
        "JOIN PRODUITS pr ON pr.ID_PRODUIT = p.ID_PRODUIT "
        "GROUP BY NVL(TRIM(pr.COLLECTION), 'INCONNUE'), "
        "TO_CHAR(NVL(p.DATE_LANCEMENT, NVL(p.DATE_FIN_PREVUE, SYSDATE)), 'YYYY-MM') "
        "ORDER BY YM"));

    if (!q.exec()) {
        QLabel *msg = new QLabel("Impossible de lire l'historique Oracle pour la prediction.");
        msg->setStyleSheet("background: #fdecea; color: #b71c1c; border:1px solid #f5c6cb; padding: 12px; border-radius: 8px;");
        msg->setAlignment(Qt::AlignCenter);
        l->addWidget(msg);
        l->addWidget(new QLabel(q.lastError().text()));
        ui->tabWidgetProduits->setCurrentIndex(5);
        return;
    }

    QMap<QString, QVector<ModeSeriePoint>> series;
    QSet<QString> allMonths;
    while (q.next()) {
        const QString collection = q.value(QStringLiteral("COLLECTION")).toString().trimmed();
        const QString ym = q.value(QStringLiteral("YM")).toString().trimmed();
        const double qty = q.value(QStringLiteral("QTE")).toDouble();
        if (collection.isEmpty() || ym.isEmpty())
            continue;
        series[collection].append({ym, qty});
        allMonths.insert(ym);
    }

    if (series.isEmpty()) {
        QLabel *msg = new QLabel("Pas assez de donnees historiques pour lancer une prediction mode.");
        msg->setStyleSheet("background: #fff3cd; color: #856404; border:1px solid #ffeeba; padding: 12px; border-radius: 8px;");
        msg->setAlignment(Qt::AlignCenter);
        l->addWidget(msg);
        ui->tabWidgetProduits->setCurrentIndex(5);
        return;
    }

    QStringList months = allMonths.values();
    months.sort();

    QVector<ModePrediction> preds;
    preds.reserve(series.size());

    QVector<double> recentVals;
    QVector<double> trendVals;
    QVector<double> momVals;
    QVector<double> seasVals;
    QVector<double> volVals;

    const QDate nextMonth = QDate::currentDate().addMonths(1);
    const int targetMonth = nextMonth.month();

    for (auto it = series.begin(); it != series.end(); ++it) {
        QMap<QString, double> m;
        for (const ModeSeriePoint &p : it.value())
            m[p.ym] += p.qte;

        QVector<double> y;
        y.reserve(months.size());
        for (const QString &ym : months)
            y.append(m.value(ym, 0.0));

        const double mu = moyenne(y);
        const double slope = penteRegressionLineaire(y);
        const double sigma = ecartType(y, mu);

        QVector<double> rec = y;
        const int startRec = qMax(0, rec.size() - 3);
        const QVector<double> recSlice = rec.mid(startRec);
        const double recMean = moyenne(recSlice);

        const double momentum = (recMean - mu) / (mu + 1.0);

        QVector<double> monthBucket;
        for (auto jt = m.begin(); jt != m.end(); ++jt) {
            const QDate d = QDate::fromString(jt.key() + QStringLiteral("-01"), QStringLiteral("yyyy-MM-dd"));
            if (d.isValid() && d.month() == targetMonth)
                monthBucket.append(jt.value());
        }
        const double seasonalAvg = monthBucket.isEmpty() ? mu : moyenne(monthBucket);
        const double seasonalBoost = (seasonalAvg - mu) / (mu + 1.0);

        ModePrediction pr;
        pr.collection = it.key();
        pr.baseMoyenne = mu;
        pr.moyenneRecente = recMean;
        pr.penteTendance = slope;
        pr.momentum = momentum;
        pr.saisonnalite = seasonalBoost;
        pr.volatilite = sigma;
        pr.pointsHistoriques = y.size();
        preds.append(pr);

        recentVals.append(recMean);
        trendVals.append(slope);
        momVals.append(momentum);
        seasVals.append(seasonalBoost);
        volVals.append(sigma);
    }

    const auto minmaxRecent = std::minmax_element(recentVals.begin(), recentVals.end());
    const auto minmaxTrend = std::minmax_element(trendVals.begin(), trendVals.end());
    const auto minmaxMom = std::minmax_element(momVals.begin(), momVals.end());
    const auto minmaxSeas = std::minmax_element(seasVals.begin(), seasVals.end());
    const auto minmaxVol = std::minmax_element(volVals.begin(), volVals.end());

    for (ModePrediction &pr : preds) {
        const double nRecent = normaliserMinMax(pr.moyenneRecente, *minmaxRecent.first, *minmaxRecent.second);
        const double nTrend = normaliserMinMax(pr.penteTendance, *minmaxTrend.first, *minmaxTrend.second);
        const double nMom = normaliserMinMax(pr.momentum, *minmaxMom.first, *minmaxMom.second);
        const double nSeas = normaliserMinMax(pr.saisonnalite, *minmaxSeas.first, *minmaxSeas.second);
        const double nVol = normaliserMinMax(pr.volatilite, *minmaxVol.first, *minmaxVol.second);

        pr.score = (0.40 * nRecent + 0.25 * nTrend + 0.20 * nMom + 0.15 * nSeas) * 100.0;

        const double confidenceRaw = 0.55 * (1.0 - nVol) + 0.45 * clamp01(static_cast<double>(pr.pointsHistoriques) / 8.0);
        pr.confiance = clamp01(confidenceRaw) * 100.0;

        const double growthFactor = 1.0 + (0.35 * pr.penteTendance / (pr.baseMoyenne + 1.0))
                                    + (0.30 * pr.momentum)
                                    + (0.20 * pr.saisonnalite);
        pr.prevision = qMax(0.0, pr.moyenneRecente * qMax(0.4, growthFactor));
    }

    std::sort(preds.begin(), preds.end(), [](const ModePrediction &a, const ModePrediction &b) {
        return a.score > b.score;
    });

    const QString moisLabel = QLocale::system().toString(nextMonth, QStringLiteral("MMMM yyyy"));
    QLabel *kpi = new QLabel(QString(
                                 "<div style='background:#fff;border:1px solid #d7ccc8;border-radius:10px;padding:10px;'>"
                                 "<b>Top tendance predit :</b> %1"
                                 " &nbsp;|&nbsp; <b>Score :</b> %2/100"
                                 " &nbsp;|&nbsp; <b>Confiance :</b> %3%%"
                                 " &nbsp;|&nbsp; <b>Projection %4 :</b> %5 unites"
                                 "</div>")
                                 .arg(preds.first().collection)
                                 .arg(QString::number(preds.first().score, 'f', 1))
                                 .arg(QString::number(preds.first().confiance, 'f', 0))
                                 .arg(moisLabel)
                                 .arg(QString::number(preds.first().prevision, 'f', 0)));
    kpi->setTextFormat(Qt::RichText);
    l->addWidget(kpi);

    QTableWidget *tw = new QTableWidget();
    tw->setColumnCount(7);
    tw->setRowCount(preds.size());
    tw->setHorizontalHeaderLabels({"Rang", "Collection", "Score tendance", "Projection", "Confiance", "Historique", "Signal"});
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->setAlternatingRowColors(true);
    tw->setStyleSheet("QTableWidget { background:#fff; border:1px solid #d7ccc8; border-radius:8px; }");

    for (int i = 0; i < preds.size(); ++i) {
        const ModePrediction &pr = preds[i];
        const QString signal = pr.score >= 70.0 ? "FORTE HAUSSE"
                             : pr.score >= 50.0 ? "HAUSSE MODEREE"
                             : pr.score >= 35.0 ? "STABLE"
                                                : "A SURVEILLER";

        tw->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        tw->setItem(i, 1, new QTableWidgetItem(pr.collection));
        tw->setItem(i, 2, new QTableWidgetItem(QString::number(pr.score, 'f', 1) + "/100"));
        tw->setItem(i, 3, new QTableWidgetItem(QString::number(pr.prevision, 'f', 0) + " unites"));
        tw->setItem(i, 4, new QTableWidgetItem(QString::number(pr.confiance, 'f', 0) + "%"));
        tw->setItem(i, 5, new QTableWidgetItem(QString::number(pr.pointsHistoriques) + " mois"));
        tw->setItem(i, 6, new QTableWidgetItem(signal));
    }

    l->addWidget(tw);

    QLabel *note = new QLabel(
        "Historique de mode : le score combine la demande recente, la pente de tendance, le momentum et la saisonnalite. "
        "La confiance baisse quand la volatilite augmente ou si l'historique est trop court.");
    note->setWordWrap(true);
    note->setStyleSheet("font-size:12px;color:#6d4c41;");
    l->addWidget(note);

    ui->tabWidgetProduits->setCurrentIndex(5);
}

void MainWindow::showStockCompareTab() {
    if(ui->tabWidgetStock->count() < 5) return;
    QWidget *onglet = ui->tabWidgetStock->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("⚖️ COMPARATEUR DE FOURNISSEURS / LOTS");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #16a085; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    int idx = ui->tableStock->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesMatieres.size()) {
        MatiereInfo m = mesMatieres[idx];
        desc->setText(QString(
                          "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #16a085; color:#3e2723; font-size:15px;'>"
                          "<h2 style='color:#16a085; margin-top:0; text-align:center;'>Analyse du lot : %1</h2><hr>"
                          "<ul>"
                          "<li><b>Qualité :</b> Grade %2</li>"
                          "<li><b>Recommandation IA :</b> Ce lot de %3 est optimal pour la collection <i>Hiver</i>. Le fournisseur actuel offre un rapport qualité/prix 12% supérieur à la moyenne du marché.</li>"
                          "<li><b>Alternative :</b> Lot Cuir-Agneau-002 (Fournisseur B) - Moins cher mais qualité inférieure.</li>"
                          "</ul></div>").arg(m.code, m.qualite, m.categorie));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Veuillez sélectionner un lot dans la liste pour le comparer au marché.</div>");
    }
    desc->setAlignment(Qt::AlignCenter); l->addWidget(desc, 0, Qt::AlignCenter);
    l->addStretch();
    ui->tabWidgetStock->setCurrentIndex(4);
}

void MainWindow::showStockCalculTab() {
    if(ui->tabWidgetStock->count() < 6) return;
    QWidget *onglet = ui->tabWidgetStock->widget(5);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("🧮 CALCULATEUR DE BESOINS (PRODUCTION)");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #f39c12; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    int idx = ui->tableStock->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesMatieres.size()) {
        MatiereInfo m = mesMatieres[idx];
        double piecesEstim = m.quantite / 0.8;
        desc->setText(QString(
                          "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #f39c12; color:#3e2723; font-size:15px;'>"
                          "<h2 style='color:#f39c12; margin-top:0; text-align:center;'>Projection pour : %1</h2><hr>"
                          "Stock actuel disponible : <b>%2 unités</b><br><br>"
                          "<b>Capacité de production estimée :</b><br>"
                          "Avec ce lot, vous pouvez fabriquer environ <b style='color:#d35400; font-size:20px;'>%3 sacs</b> (basé sur un ratio de 0.8u/sac).<br><br>"
                          "<i>Alerte Rupture : Prévoyez un réapprovisionnement si une commande dépasse cette quantité.</i>"
                          "</div>").arg(m.code).arg(m.quantite).arg(static_cast<int>(piecesEstim)));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Veuillez sélectionner une matière dans la liste pour calculer les besoins.</div>");
    }
    desc->setAlignment(Qt::AlignCenter); l->addWidget(desc, 0, Qt::AlignCenter);
    l->addStretch();
    ui->tabWidgetStock->setCurrentIndex(5);
}


void MainWindow::ouvrirIAPrediction() {
    int idx = ui->tablePlanif->currentRow();
    if(idx < 0) {
        alerteWarning("Assistant IA", "Veuillez d'abord selectionner une commande dans la liste.");
        return;
    }

    CommandeInfo c = mesCommandes[idx];
    QString produit = c.idProduit.toLower();
    int qte = c.quantite;

    struct ProfilProduit {
        QString nom;
        QString complexite;
        double tempsCoupe;
        double tempsAssemblage;
        double tempsCouture;
        double tempsFinition;
        double tempsTotal;
        int capaciteJour;
    };

    ProfilProduit profil;

    if (produit.contains("sac") && (produit.contains("voyage") || produit.contains("grand"))) {
        profil = {c.idProduit, "ELEVEE", 1.2, 1.8, 1.5, 0.5, 5.0, 12};
    }
    else if (produit.contains("sac") && (produit.contains("main") || produit.contains("dame"))) {
        profil = {c.idProduit, "ELEVEE", 1.0, 1.5, 1.2, 0.4, 4.1, 15};
    }
    else if (produit.contains("sac") && (produit.contains("dos") || produit.contains("sport"))) {
        profil = {c.idProduit, "ELEVEE", 1.1, 1.6, 1.4, 0.5, 4.6, 13};
    }
    else if (produit.contains("sac")) {
        profil = {c.idProduit, "ELEVEE", 1.0, 1.5, 1.3, 0.4, 4.2, 14};
    }
    else if (produit.contains("portef") || produit.contains("wallet")) {
        profil = {c.idProduit, "FAIBLE", 0.3, 0.4, 0.3, 0.2, 1.2, 45};
    }
    else if (produit.contains("ceinture") || produit.contains("belt")) {
        profil = {c.idProduit, "FAIBLE", 0.2, 0.2, 0.3, 0.1, 0.8, 60};
    }
    else if (produit.contains("pochette") || produit.contains("clutch")) {
        profil = {c.idProduit, "MOYENNE", 0.5, 0.6, 0.5, 0.3, 1.9, 30};
    }
    else if (produit.contains("etui") || produit.contains("housse")) {
        profil = {c.idProduit, "FAIBLE", 0.2, 0.3, 0.2, 0.1, 0.8, 55};
    }
    else if (produit.contains("veste") || produit.contains("jacket") || produit.contains("blouson")) {
        profil = {c.idProduit, "TRES ELEVEE", 2.0, 2.5, 2.0, 0.8, 7.3, 8};
    }
    else if (produit.contains("chaussure") || produit.contains("botte")) {
        profil = {c.idProduit, "TRES ELEVEE", 1.5, 2.0, 1.8, 0.7, 6.0, 10};
    }
    else {
        profil = {c.idProduit, "MOYENNE", 0.8, 1.0, 0.8, 0.4, 3.0, 20};
    }

    double facteurQte;
    QString regimeProduction;
    if (qte <= 10) {
        facteurQte = 1.0;
        regimeProduction = "Artisanal (pas d'economie d'echelle)";
    } else if (qte <= 50) {
        facteurQte = 0.88;
        regimeProduction = "Petite serie (-12% par optimisation)";
    } else if (qte <= 200) {
        facteurQte = 0.75;
        regimeProduction = "Serie moyenne (-25% par routine)";
    } else if (qte <= 500) {
        facteurQte = 0.65;
        regimeProduction = "Grande serie (-35% par cadence)";
    } else {
        facteurQte = 0.58;
        regimeProduction = "Production de masse (-42% par industrialisation)";
    }

    double coupeTotale = profil.tempsCoupe * qte * facteurQte;
    double assemblTotale = profil.tempsAssemblage * qte * facteurQte;
    double coutureTotale = profil.tempsCouture * qte * facteurQte;
    double finitionTotale = profil.tempsFinition * qte * facteurQte;
    double tempsTotalH = coupeTotale + assemblTotale + coutureTotale + finitionTotale;

    double heuresParJour = 8.0;
    double joursCoupe = std::ceil(coupeTotale / heuresParJour);
    double joursAssembl = std::ceil(assemblTotale / heuresParJour);
    double joursCouture = std::ceil(coutureTotale / heuresParJour);
    double joursFinition = std::ceil(finitionTotale / heuresParJour);
    int joursTotalProduction = static_cast<int>(joursCoupe + joursAssembl + joursCouture + joursFinition);

    double margeSec;
    if (profil.complexite == "TRES ELEVEE") margeSec = 0.20;
    else if (profil.complexite == "ELEVEE") margeSec = 0.15;
    else if (profil.complexite == "MOYENNE") margeSec = 0.10;
    else margeSec = 0.05;

    int joursAvecMarge = static_cast<int>(std::ceil(joursTotalProduction * (1.0 + margeSec)));
    QDate nvFin = c.dateDebut.addDays(joursAvecMarge);


    int ecartJours = 0;
    QString analyseEcart;

    QDate dateFin = QDate::fromString(c.dateFinEstimee, "dd/MM/yyyy");
    if (!dateFin.isValid()) dateFin = QDate::fromString(c.dateFinEstimee, "yyyy-MM-dd");

    if (dateFin.isValid()) {
        ecartJours = c.dateDebut.daysTo(dateFin) - joursAvecMarge;
        if (ecartJours > 3) {
            analyseEcart = QString("<span style='color:#2e7d32; font-weight:bold;'>CONFORTABLE (+%1 jours de marge)</span>").arg(ecartJours);
        } else if (ecartJours >= 0) {
            analyseEcart = QString("<span style='color:#ef6c00; font-weight:bold;'>SERRE (seulement %1 jours de marge)</span>").arg(ecartJours);
        } else {
            analyseEcart = QString("<span style='color:#c62828; font-weight:bold;'>IMPOSSIBLE ! Il manque %1 jours !</span>").arg(-ecartJours);
        }
    } else {
        analyseEcart = "<span style='color:#757575;'>Date fin non definie</span>";
    }

    QString couleurComplexite;
    if (profil.complexite == "TRES ELEVEE") couleurComplexite = "#c62828";
    else if (profil.complexite == "ELEVEE") couleurComplexite = "#ef6c00";
    else if (profil.complexite == "MOYENNE") couleurComplexite = "#f9a825";
    else couleurComplexite = "#2e7d32";

    QString texte = QString(
        "<div style='font-family: Arial; line-height: 1.6;'>"

        "<h2 style='color:#6a1b9a; margin-bottom: 5px;'>ANALYSE PREDICTIVE IA</h2>"
        "<hr style='border: 1px solid #ce93d8;'>"

        "<table style='width:100%%; margin: 10px 0;'>"
        "<tr><td style='width:50%%;'>"
        "<b>Produit :</b> %1<br>"
        "<b>Quantite :</b> %2 unites<br>"
        "<b>Lancement :</b> %3"
        "</td><td>"
        "<b>Complexite :</b> <span style='color:%4; font-weight:900; font-size:16px;'>%5</span><br>"
        "<b>Regime :</b> %6<br>"
        "<b>Facteur echelle :</b> x%7"
        "</td></tr></table>"

        "<hr style='border: 1px dashed #e0e0e0;'>"

        "<h3 style='color:#4a148c;'>DETAIL PAR ETAPE</h3>"
        "<table style='width:100%%; border-collapse:collapse; margin: 8px 0;'>"
        "<tr style='background:#f3e5f5;'>"
        "<th style='padding:8px; text-align:left; border:1px solid #ce93d8;'>Etape</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Temps/unite</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>x %2 unites</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Avec echelle</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Jours</th>"
        "</tr>"

        "<tr>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#1565c0; font-weight:bold;'>COUPE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%8 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%9 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%10 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%11 j</td>"
        "</tr>"

        "<tr style='background:#fafafa;'>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#ef6c00; font-weight:bold;'>ASSEMBLAGE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%12 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%13 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%14 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%15 j</td>"
        "</tr>"

        "<tr>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#2e7d32; font-weight:bold;'>COUTURE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%16 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%17 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%18 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%19 j</td>"
        "</tr>"

        "<tr style='background:#fafafa;'>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#f9a825; font-weight:bold;'>FINITION</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%20 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%21 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%22 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%23 j</td>"
        "</tr>"

        "<tr style='background:#6a1b9a; color:white;'>"
        "<td style='padding:8px; border:1px solid #4a148c; font-weight:900;'>TOTAL</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:bold;'>%24 h</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c;'>-</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:900; font-size:14px;'>%25 h</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:900; font-size:14px;'>%26 j</td>"
        "</tr>"
        "</table>"

        "<hr style='border: 1px dashed #e0e0e0;'>"

        "<h3 style='color:#4a148c;'>RECOMMANDATION</h3>"
        "<table style='width:100%%; margin: 8px 0;'>"
        "<tr>"
        "<td style='padding:10px; background:#f3e5f5; border-radius:8px; text-align:center;'>"
        "<b>Marge securite :</b> +%27%%<br>"
        "<b>Duree totale recommandee :</b> <span style='font-size:18px; font-weight:900; color:#6a1b9a;'>%28 jours</span><br>"
        "<b>Date fin recommandee :</b> <span style='font-size:16px; font-weight:900; color:#c62828;'>%29</span><br><br>"
        "<b>Analyse delai actuel :</b> %30"
        "</td>"
        "</tr></table>"

        "<div style='margin-top:10px; padding:8px; background:#e8eaf6; border-radius:6px; text-align:center;'>"
        "<b>Capacite estimee :</b> %31 unites/jour/employe | "
        "<b>Heures travail/jour :</b> 8h"
        "</div>"

        "</div>"
    )
    .arg(c.idProduit)
    .arg(qte)
    .arg(c.dateDebut.toString("dd/MM/yyyy"))
    .arg(couleurComplexite)
    .arg(profil.complexite)
    .arg(regimeProduction)
    .arg(facteurQte, 0, 'f', 2)
    .arg(profil.tempsCoupe, 0, 'f', 1)
    .arg(profil.tempsCoupe * qte, 0, 'f', 1)
    .arg(coupeTotale, 0, 'f', 1)
    .arg(joursCoupe, 0, 'f', 0)
    .arg(profil.tempsAssemblage, 0, 'f', 1)
    .arg(profil.tempsAssemblage * qte, 0, 'f', 1)
    .arg(assemblTotale, 0, 'f', 1)
    .arg(joursAssembl, 0, 'f', 0)
    .arg(profil.tempsCouture, 0, 'f', 1)
    .arg(profil.tempsCouture * qte, 0, 'f', 1)
    .arg(coutureTotale, 0, 'f', 1)
    .arg(joursCouture, 0, 'f', 0)
    .arg(profil.tempsFinition, 0, 'f', 1)
    .arg(profil.tempsFinition * qte, 0, 'f', 1)
    .arg(finitionTotale, 0, 'f', 1)
    .arg(joursFinition, 0, 'f', 0)
    .arg(profil.tempsTotal, 0, 'f', 1)
    .arg(tempsTotalH, 0, 'f', 1)
    .arg(joursTotalProduction)
    .arg(margeSec * 100, 0, 'f', 0)
    .arg(joursAvecMarge)
    .arg(nvFin.toString("dd/MM/yyyy"))
    .arg(analyseEcart)
    .arg(profil.capaciteJour);

    ui->lbl_ia_details->setText(texte);
    ui->lbl_ia_details->setStyleSheet(
        "font-size: 13px; color: #3e2723; background: white; "
        "padding: 20px; border-radius: 10px; border: 2px solid #ce93d8;"
    );
    ui->lbl_ia_details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->lbl_ia_details->setWordWrap(true);

    ui->btn_ia_appliquer->setProperty("id_cmd", c.id.replace("OF-", "").toInt());
    ui->btn_ia_appliquer->setProperty("nv_fin", nvFin);

    ui->tabWidgetPlanif->setCurrentIndex(4);
}

void MainWindow::preparerFormulaireModif(int idx) {
    indexModification = idx;
    CommandeInfo c = mesCommandes[idx];

    ui->cb_produit_modif->clear();
    QSqlQuery qProd("SELECT ID_PRODUIT, DESIGNATION FROM PRODUITS");
    while(qProd.next()) ui->cb_produit_modif->addItem(qProd.value("DESIGNATION").toString(), qProd.value("ID_PRODUIT"));

    ui->cb_matiere_modif->clear();
    QSqlQuery qMat("SELECT ID_STOCK_MP, CODE_MP FROM MATIERES_PREMIERES");
    while(qMat.next()) ui->cb_matiere_modif->addItem(qMat.value("CODE_MP").toString(), qMat.value("ID_STOCK_MP"));

    ui->cb_employe_modif->clear();
    QSqlQuery qEmp("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES");
    while(qEmp.next()) ui->cb_employe_modif->addItem(qEmp.value("NOM").toString() + " " + qEmp.value("PRENOM").toString(), qEmp.value("ID_EMPLOYE"));

    ui->cb_produit_modif->setCurrentText(c.idProduit);
    ui->cb_matiere_modif->setCurrentText(c.idMatiere);
    ui->cb_employe_modif->setCurrentText(c.idEmploye);
    ui->sb_qte_modif->setValue(c.quantite);
    ui->dt_lancement_modif->setDate(c.dateDebut);
    ui->le_fin_prevue_modif->setText(c.dateFinEstimee);

    ui->tabWidgetPlanif->setCurrentIndex(3);
}


void MainWindow::goToTabEmployesByText(const QString& title)
{
    auto *tw = ui ? ui->tabWidgetEmployes : nullptr;
    if(!tw) return;

    const int c = tw->count();
    for(int i = 0; i < c; ++i) {
        const QString t = tw->tabText(i).trimmed();
        if(t == title.trimmed()) {
            tw->setCurrentIndex(i);
            return;
        }
    }
}

void MainWindow::goToTabEmployes(int index)
{
    if (!ui || !ui->tabWidgetEmployes) return;

    const int count = ui->tabWidgetEmployes->count();
    if (index < 0 || index >= count) return;

    QSignalBlocker block(ui->tabWidgetEmployes);
    ui->tabWidgetEmployes->setCurrentIndex(index);
}

void MainWindow::forceTabEmployes(int index)
{
    if (!ui || !ui->tabWidgetEmployes) return;

    const int count = ui->tabWidgetEmployes->count();
    if (index < 0 || index >= count) return;

    QTimer::singleShot(0, this, [this, index]() {
        if (!ui || !ui->tabWidgetEmployes) return;
        ui->tabWidgetEmployes->setCurrentIndex(index);
    });
}

bool MainWindow::chargerEmployePourModification(int id)
{
    if (!ui) return false;

    if (!ui->cb_emp_poste_modif || !ui->cb_emp_dept_modif ||
        !ui->dt_emp_emb_modif  || !ui->sb_emp_sal_modif  ||
        !ui->le_emp_nom_modif  || !ui->le_emp_pre_modif  ||
        !ui->le_emp_email_modif|| !ui->le_emp_tel_modif  ||
        !ui->le_emp_rfid_modif)
    {
        return false;
    }

    QSignalBlocker bPoste(ui->cb_emp_poste_modif);
    QSignalBlocker bDept (ui->cb_emp_dept_modif);
    QSignalBlocker bDate (ui->dt_emp_emb_modif);
    QSignalBlocker bSal  (ui->sb_emp_sal_modif);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare(
        "SELECT NOM, PRENOM, EMAIL, TELEPHONE, RFID_TAG, "
        "POSTE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE "
        "FROM EMPLOYES WHERE ID_EMPLOYE = :id"
    );
    q.bindValue(":id", id);

    if (!q.exec() || !q.next()) return false;

    ui->le_emp_nom_modif->setText(q.value(0).toString());
    ui->le_emp_pre_modif->setText(q.value(1).toString());
    ui->le_emp_email_modif->setText(q.value(2).toString());
    ui->le_emp_tel_modif->setText(q.value(3).toString());
    ui->le_emp_rfid_modif->setText(q.value(4).toString());

    const QString poste = q.value(5).toString().trimmed();
    const QString dept  = q.value(6).toString().trimmed();

    auto ensureComboValue = [](QComboBox* cb, const QString& value) {
        if (!cb) return;
        if (value.isEmpty()) {
            if (cb->count() > 0) cb->setCurrentIndex(0);
            return;
        }

        int idx = cb->findText(value, Qt::MatchFixedString);
        if (idx < 0) {
            cb->addItem(value);
            idx = cb->findText(value, Qt::MatchFixedString);
        }
        if (idx >= 0 && idx < cb->count()) cb->setCurrentIndex(idx);
    };

    ensureComboValue(ui->cb_emp_poste_modif, poste);
    ensureComboValue(ui->cb_emp_dept_modif, dept);

    QDate d = q.value(7).toDate();
    if (!d.isValid()) d = q.value(7).toDateTime().date();
    if (!d.isValid()) {
        const QString ds = q.value(7).toString().trimmed();
        d = QDate::fromString(ds, "yyyy-MM-dd");
        if (!d.isValid()) d = QDate::fromString(ds, "dd/MM/yyyy");
    }
    if (!d.isValid()) d = QDate::currentDate();
    ui->dt_emp_emb_modif->setDate(d);

    bool okSal = false;
    const double sal = q.value(8).toDouble(&okSal);
    ui->sb_emp_sal_modif->setValue(okSal ? sal : 0.0);

    initialNomEmploye = ui->le_emp_nom_modif->text().trimmed();
    initialPrenomEmploye = ui->le_emp_pre_modif->text().trimmed();
    initialEmailEmploye = ui->le_emp_email_modif->text().trimmed();
    initialPosteEmploye = ui->cb_emp_poste_modif->currentText().trimmed();
    initialDepartementEmploye = ui->cb_emp_dept_modif->currentText().trimmed();
    initialDateEmbaucheEmploye = ui->dt_emp_emb_modif->date();
    initialSalaireEmploye = ui->sb_emp_sal_modif->value();
    initialRfidEmploye = ui->le_emp_rfid_modif->text().trimmed();

    {
        QString telLoaded = ui->le_emp_tel_modif->text().trimmed();
        QString digits;
        for(const QChar &ch : telLoaded) if(ch.isDigit()) digits.append(ch);
        initialTelephoneDigitsEmploye = digits;
    }

    return true;
}

void MainWindow::on_btn_valider_emp_clicked()
{
    if(!ui) return;

    const QString nom = ui->le_emp_nom->text().trimmed();
    const QString prenom = ui->le_emp_pre->text().trimmed();
    const QString poste = ui->cb_emp_poste->currentText().trimmed();
    const QString email = ui->le_emp_email->text().trimmed();
    const QString telephoneRaw = ui->le_emp_tel->text().trimmed();
    const QString departement = ui->cb_emp_dept->currentText().trimmed();
    const QDate dateEmb = ui->dt_emp_emb->date();
    const double salaire = ui->sb_emp_sal->value();
    const QString rfid = ui->le_emp_rfid->text().trimmed();

    static const QRegularExpression emailRe(QStringLiteral("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$"));
    const bool emailOk = emailRe.match(email).hasMatch();

    static const QRegularExpression telRe(QStringLiteral("^\\d{8}$"));
    QString telephoneDigits;
    for(const QChar &ch : telephoneRaw) {
        if(ch.isDigit()) telephoneDigits.append(ch);
    }
    const bool telOk = telRe.match(telephoneDigits).hasMatch();

    if(nom.isEmpty() || prenom.isEmpty()) {
        alerteWarning("Erreur", "Nom et prénom sont obligatoires.");
        return;
    }
    if(poste.isEmpty() || departement.isEmpty()) {
        alerteWarning("Erreur", "Poste et département sont obligatoires.");
        return;
    }
    if(email.isEmpty() || !emailOk) {
        alerteWarning("Erreur", "Adresse email invalide. Exemple : nom@domaine.com");
        return;
    }
    if(telephoneDigits.isEmpty() || !telOk) {
        alerteWarning("Erreur", "Numéro de téléphone invalide (8 chiffres).");
        return;
    }
    if(!dateEmb.isValid()) {
        alerteWarning("Erreur", "Date d'embauche invalide.");
        return;
    }
    if(salaire < 0) {
        alerteWarning("Erreur", "Le salaire ne peut pas être négatif.");
        return;
    }
    if(rfid.isEmpty()) {
        alerteWarning("Erreur", "RFID est obligatoire (pour la connexion).");
        return;
    }

    employe e(
        0,
        nom,
        prenom,
        poste,
        email,
        telephoneDigits,
        departement,
        dateEmb,
        salaire,
        rfid
    );

    if(e.ajouter()) {
        rafraichirListeEmployes();
        alerteSucces("RH", "Employé ajouté avec succès");
        forceTabEmployes(0);
    } else {
        alerteErreur("RH", "Erreur lors de l'ajout.\nVérifiez l'unicité de l'email/RFID et la configuration de la séquence Oracle.");
    }
}

void MainWindow::on_btn_valider_modif_emp_clicked()
{
    if(!ui) return;

    if (idEmployeAModifier < 0) {
        alerteWarning("RH", "Aucun employé sélectionné");
        return;
    }

    const int id = idEmployeAModifier;

    const QString nom = ui->le_emp_nom_modif->text().trimmed();
    const QString prenom = ui->le_emp_pre_modif->text().trimmed();
    const QString poste = ui->cb_emp_poste_modif->currentText().trimmed();
    const QString email = ui->le_emp_email_modif->text().trimmed();
    const QString telephoneRaw = ui->le_emp_tel_modif->text().trimmed();
    const QString departement = ui->cb_emp_dept_modif->currentText().trimmed();
    const QDate dateEmb = ui->dt_emp_emb_modif->date();
    const double salaire = ui->sb_emp_sal_modif->value();
    const QString rfid = ui->le_emp_rfid_modif->text().trimmed();

    static const QRegularExpression telRe(QStringLiteral("^\\d{8}$"));

    QString telephoneDigits;
    for(const QChar &ch : telephoneRaw) {
        if(ch.isDigit()) telephoneDigits.append(ch);
    }

    auto isEmailLike = [](const QString &v) -> bool {
        const QString s = v.trimmed();
        const int at = s.indexOf('@');
        if(at <= 0) return false;
        const int dot = s.indexOf('.', at + 1);
        return dot > at + 1;
    };

    if(nom.isEmpty() || prenom.isEmpty()) { alerteWarning("Erreur", "Nom et prénom sont obligatoires."); return; }
    if(poste.isEmpty() || departement.isEmpty()) { alerteWarning("Erreur", "Poste et département sont obligatoires."); return; }
    if(email.isEmpty() || !isEmailLike(email)) { alerteWarning("Erreur", "Adresse email invalide."); return; }
    if(telephoneDigits.isEmpty() || !telRe.match(telephoneDigits).hasMatch()) { alerteWarning("Erreur", "Numéro de téléphone invalide (8 chiffres)."); return; }
    if(!dateEmb.isValid()) { alerteWarning("Erreur", "Date d'embauche invalide."); return; }
    if(salaire < 0) { alerteWarning("Erreur", "Le salaire ne peut pas être négatif."); return; }
    if(rfid.isEmpty()) { alerteWarning("Erreur", "RFID est obligatoire."); return; }

    QString posteNow = poste;
    QString depNow = departement;
    QString nomNow = nom;
    QString prenomNow = prenom;
    QString emailNow = email;
    QString rfidNow = rfid;
    QDate dateNow = dateEmb;
    double salaireNow = salaire;

    QString telephoneDigitsNow = telephoneDigits;

    const bool same =
        (nomNow == initialNomEmploye) &&
        (prenomNow == initialPrenomEmploye) &&
        (posteNow == initialPosteEmploye) &&
        (emailNow == initialEmailEmploye) &&
        (telephoneDigitsNow == initialTelephoneDigitsEmploye) &&
        (depNow == initialDepartementEmploye) &&
        (dateNow == initialDateEmbaucheEmploye) &&
        (QString::number(salaireNow) == QString::number(initialSalaireEmploye)) &&
        (rfidNow == initialRfidEmploye);

    if(same) {
        alerteInfo("Info", "Aucune modification n'est faite.");
        idEmployeAModifier = -1;
        rafraichirListeEmployes();
        forceTabEmployes(0);
        return;
    }

    employe e(
        id,
        nom,
        prenom,
        poste,
        email,
        telephoneDigits,
        departement,
        dateEmb,
        salaire,
        rfid
    );

    bool ok = e.modifier(id);
    if (!ok) {
        alerteErreur("RH", "Erreur lors de la modification");
        return;
    }

    idEmployeAModifier = -1;
    rafraichirListeEmployes();
    forceTabEmployes(0);
    alerteSucces("RH", "Employé modifié avec succès");
}

void MainWindow::on_btn_delete_emp_clicked()
{
    if(!ui) return;

    int row = ui->tableEmployes->currentRow();
    if(row < 0) {
        if (ui->tableEmployes->currentItem())
            row = ui->tableEmployes->currentItem()->row();
    }

    if(row < 0) {
        alerteWarning("RH", "Sélectionnez un employé à supprimer");
        return;
    }

    QTableWidgetItem *it = ui->tableEmployes->item(row, 0);
    if(!it) {
        alerteWarning("RH", "ID introuvable sur la ligne sélectionnée");
        return;
    }

    const int id = it->text().toInt();
    employe emp;
    if (emp.supprimer(id)) {
        rafraichirListeEmployes();
        alerteSucces("RH", "Employé supprimé");
        forceTabEmployes(0);
    } else {
        alerteErreur("RH", "Erreur lors de la suppression");
    }
}

void MainWindow::on_btn_edit_emp_clicked()
{
    if(!ui || !ui->tableEmployes) return;

    int row = ui->tableEmployes->currentRow();
    if(row < 0) {
        alerteWarning("RH", "Sélectionne un employé dans le tableau");
        return;
    }

    QTableWidgetItem *it = ui->tableEmployes->item(row, 0);
    if(!it) {
        alerteWarning("RH", "ID introuvable");
        return;
    }

    idEmployeAModifier = it->text().toInt();
    if(!chargerEmployePourModification(idEmployeAModifier)) {
        alerteErreur("RH", "Erreur chargement employé");
        idEmployeAModifier = -1;
        return;
    }

    forceTabEmployes(2);
}

void MainWindow::on_btn_sort_alpha_emp_clicked()
{
    if(!employeTriAlphaActif) {
        employeTriAlphaActif = true;
        employeTriAlphaOrdre = Qt::AscendingOrder;
    } else {
        employeTriAlphaOrdre = (employeTriAlphaOrdre == Qt::AscendingOrder)
                                 ? Qt::DescendingOrder
                                 : Qt::AscendingOrder;
    }

    if(ui && ui->btn_sort_alpha_emp) {
        ui->btn_sort_alpha_emp->setText(
            (employeTriAlphaOrdre == Qt::AscendingOrder) ? "Tri A-Z" : "Tri Z-A"
        );
    }

    if(ui && ui->tableEmployes) {
        ui->tableEmployes->setSortingEnabled(true);
        ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre);
    }
}

void MainWindow::preparerFormulaireEmploye(bool estModif, int idx) {
    if(estModif && idx >= 0 && idx < mesEmployes.size()) {
        indexModifEmp = idx;
        const auto &e = mesEmployes[idx];
        ui->le_emp_nom_modif->setText(e.nom);
        ui->le_emp_pre_modif->setText(e.prenom);
        ui->le_emp_email_modif->setText(e.email);
        ui->le_emp_tel_modif->setText(e.telephone);
        ui->cb_emp_poste_modif->setCurrentText(e.poste);
        ui->cb_emp_dept_modif->setCurrentText(e.departement);
        ui->dt_emp_emb_modif->setDate(e.dateEmbauche);
        ui->sb_emp_sal_modif->setValue(e.salaire);
        ui->le_emp_rfid_modif->setText(e.rfid);

        ui->tabWidgetEmployes->setCurrentIndex(2);
    } else {
        ui->le_emp_nom->clear(); ui->le_emp_pre->clear();
        ui->le_emp_email->clear(); ui->le_emp_tel->clear(); ui->le_emp_rfid->clear();
        ui->sb_emp_sal->setValue(1500.0);
        ui->dt_emp_emb->setDate(QDate::currentDate());

        ui->tabWidgetEmployes->setCurrentIndex(1);
    }
}

void MainWindow::ouvrirStatsRH() {
    if(ui->tabWidgetEmployes->count() < 4) return;
    QWidget *onglet = ui->tabWidgetEmployes->widget(3);
    if (!onglet) return;
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(20); mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - RESSOURCES HUMAINES");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    int total = mesEmployes.size(); double masseSal = 0; QMap<QString, double> parDept;
    for(const auto &e : mesEmployes) { masseSal += e.salaire; parDept[e.departement] += 1; }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("👥", QString::number(total), "Effectif Total", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)"));
    kpiL->addWidget(creerCarteStat("💸", QString::number(masseSal) + " DT", "Masse Salariale", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FF5F6D, stop:1 #FFC371)"));
    mainL->addLayout(kpiL);

    QFrame *framePie = new QFrame();
    framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Département");
    titrePie->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;");
    layoutPie->addWidget(titrePie);

    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slices;
    for(auto k : parDept.keys()) slices.append({k, parDept[k]});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie);

    QFrame *frameBar = new QFrame();
    frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Effectif par Département");
    titreBar->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;");
    layoutBar->addWidget(titreBar);

    QWidget *wBar = new QWidget();
    QStringList cats = parDept.keys();
    QList<double> vals;
    for(const auto &k : cats) vals.append(parDept[k]);
    setVerticalBarChart(wBar, "", cats, vals);
    layoutBar->addWidget(wBar);

    QHBoxLayout *rowCharts = new QHBoxLayout();
    rowCharts->setSpacing(18);
    rowCharts->setContentsMargins(0, 0, 0, 0);
    rowCharts->addWidget(framePie, 1);
    rowCharts->addWidget(frameBar, 1);
    mainL->addLayout(rowCharts);
    mainL->addStretch();
    ui->tabWidgetEmployes->setCurrentIndex(3);
}

void MainWindow::showEmpEvalTab() {
    if(ui->tabWidgetEmployes->count() < 5) return;
    QWidget *onglet = ui->tabWidgetEmployes->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("📈 ÉVALUATION DES COMPÉTENCES");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #e67e22; margin-bottom: 20px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    const int row = ui->tableEmployes->currentRow();
    int idx = -1;
    if(row >= 0) {
        if(QTableWidgetItem *itId = ui->tableEmployes->item(row, 0)) {
            const int idOracle = itId->text().toInt();
            for(int i = 0; i < mesEmployes.size(); ++i) {
                if(mesEmployes[i].id.toInt() == idOracle) { idx = i; break; }
            }
        }
    }
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesEmployes.size()) {
        EmployeInfo e = mesEmployes[idx];
        desc->setText(QString(
                          "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #e67e22; color:#3e2723; font-size:16px;'>"
                          "<h2 style='color:#8d5524; margin-top:0; text-align:center;'>Dossier de %1 %2</h2><hr>"
                          "<ul>"
                          "<li><b>Maitrise des machines :</b> ⭐⭐⭐⭐☆ (Très bon)</li><br>"
                          "<li><b>Assiduité & Ponctualité :</b> ⭐⭐⭐⭐⭐ (Excellent)</li><br>"
                          "<li><b>Qualité des coutures :</b> ⭐⭐⭐☆☆ (En progression)</li>"
                          "</ul></div>").arg(e.nom, e.prenom));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Veuillez d'abord sélectionner un employé dans l'onglet 'Liste du Personnel'.</div>");
    }
    desc->setAlignment(Qt::AlignCenter);
    l->addWidget(desc, 0, Qt::AlignCenter);

    l->addStretch();
    ui->tabWidgetEmployes->setCurrentIndex(4);
}

void MainWindow::showEmpAncienneteTab() {
    if(ui->tabWidgetEmployes->count() < 6) return;
    QWidget *onglet = ui->tabWidgetEmployes->widget(5);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("📅 CALCUL D'ANCIENNETÉ ET PRIMES");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #2980b9; margin-bottom: 20px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    const int row = ui->tableEmployes->currentRow();
    int idx = -1;
    if(row >= 0) {
        if(QTableWidgetItem *itId = ui->tableEmployes->item(row, 0)) {
            const int idOracle = itId->text().toInt();
            for(int i = 0; i < mesEmployes.size(); ++i) {
                if(mesEmployes[i].id.toInt() == idOracle) { idx = i; break; }
            }
        }
    }
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesEmployes.size()) {
        EmployeInfo e = mesEmployes[idx];
        int annees = e.dateEmbauche.daysTo(QDate::currentDate()) / 365;
        double prime = annees * 50.0;
        desc->setText(QString(
                          "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #3498db; color:#3e2723; font-size:16px;'>"
                          "<h2 style='color:#2980b9; margin-top:0; text-align:center;'>%1 %2</h2><hr>"
                          "Date d'embauche : <b>%3</b><br><br>"
                          "Ancienneté calculée : <b>%4 ans</b><br><br><hr>"
                          "Prime d'ancienneté estimée : <b style='color:#27ae60; font-size:22px;'>%5 DT</b>"
                          "</div>").arg(e.nom, e.prenom, e.dateEmbauche.toString("dd/MM/yyyy")).arg(annees).arg(prime));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Veuillez d'abord sélectionner un employé dans l'onglet 'Liste du Personnel'.</div>");
    }
    desc->setAlignment(Qt::AlignCenter);
    l->addWidget(desc, 0, Qt::AlignCenter);

    l->addStretch();
    ui->tabWidgetEmployes->setCurrentIndex(5);
}

void MainWindow::showEmpAssistantTab() {
    if(ui->tabWidgetEmployes->count() < 7) return;
    QWidget *onglet = ui->tabWidgetEmployes->widget(6);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("🤖 ASSISTANT IA RH");
    titre->setStyleSheet("font-size: 24px; font-weight: 900; color: #8e44ad; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QLabel *desc = new QLabel(
        "<div style='background:white; border-radius:12px; padding:30px; border:3px dashed #9b59b6; color:#3e2723; font-size:15px;'>"
        "<h3 style='color:#8e44ad; margin-top:0;'>💡 Recommandations globales de l'IA :</h3><hr>"
        "<ul style='line-height: 1.8;'>"
        "<li><b>Formation suggérée :</b> 3 employés de la production nécessitent une mise à niveau sur les nouvelles piqueuses.</li>"
        "<li><b>Risque de Turnover :</b> L'employé <i>Dupont Jean</i> a cumulé beaucoup d'heures supplémentaires, attention au surmenage.</li>"
        "<li><b>Recrutement :</b> Prévoir 1 profil <i>Coupeur</i> et renforcer la ligne <i>Maroquinerie sacs</i> "
        "(poste disponible dans le formulaire employé) pour la collection Hiver.</li>"
        "</ul></div>"
        );
    desc->setAlignment(Qt::AlignCenter);

    l->addWidget(desc, 0, Qt::AlignCenter);
    l->addStretch();

    ui->tabWidgetEmployes->setCurrentIndex(6);
}


void MainWindow::preparerFormulaireDepot(bool estModif, int idx) {
    if(estModif && idx >= 0 && idx < mesDepots.size()) {
        indexModifDepot = idx;
        const auto &dp = mesDepots[idx];

        ui->le_depot_id_modif->setText(dp.id);
        {
            const QString sep = QStringLiteral(" — ");
            const int ixSep = dp.etagere.indexOf(sep);
            if (ixSep >= 0) {
                ui->le_depot_emp_modif->setText(dp.etagere.left(ixSep));
                ui->le_depot_eta_modif->setText(dp.etagere.mid(ixSep + sep.size()));
            } else {
                ui->le_depot_emp_modif->clear();
        ui->le_depot_eta_modif->setText(dp.etagere);
            }
        }
        ui->sb_depot_cap_modif->setValue(dp.capaciteMax);
        ui->sb_depot_act_modif->setValue(dp.quantiteActuelle);
        ui->cb_depot_type_modif->setCurrentText(dp.typeStockage);

        ui->tabWidgetDepot->setCurrentIndex(2);
    } else {
        ui->le_depot_id->clear();
        ui->le_depot_emp->clear();
        ui->le_depot_eta->clear();
        ui->sb_depot_cap->setValue(100.0);
        ui->sb_depot_act->setValue(0.0);

        ui->tabWidgetDepot->setCurrentIndex(1);
    }
}

void MainWindow::ouvrirStatsDepot() {
    if(ui->tabWidgetDepot->count() < 4) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(3);
    if (!onglet) return;
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(20); mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - DÉPÔT & LOGISTIQUE");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    double totalCap = 0; double currentLoad = 0; QMap<QString, double> parType;
    for(const auto &dp : mesDepots) {
        totalCap += dp.capaciteMax;
        currentLoad += dp.quantiteActuelle;
        parType[dp.typeStockage] += 1;
    }
    double taux = (totalCap > 0) ? (currentLoad / totalCap) * 100.0 : 0;

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("🏭", QString::number(mesDepots.size()), "Zones de Stockage", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)"));

    QString colorTaux = (taux > 85) ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EB3349, stop:1 #F45C43)" : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)";
    kpiL->addWidget(creerCarteStat("📦", QString::number(taux, 'f', 1) + "%", "Taux de Remplissage", colorTaux));

    kpiL->addWidget(creerCarteStat("🧊", QString::number(parType["Froid"]), "Zones Réfrigérées", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #56ccf2, stop:1 #2f80ed)"));
    mainL->addLayout(kpiL);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Répartition par Type de Stockage");
    titreBar->setStyleSheet("color: #8d5524; font-weight: bold; padding: 5px;"); layoutBar->addWidget(titreBar);

    QWidget *wBar = new QWidget(); QStringList types = parType.keys();
    QList<double> vals; for(auto k : types) vals << parType[k];
    setVerticalBarChart(wBar, "", types, vals); layoutBar->addWidget(wBar);

    mainL->addWidget(frameBar); mainL->addStretch();
    ui->tabWidgetDepot->setCurrentIndex(3);
}

void MainWindow::showDepotOptimizeTab() {
    if(ui->tabWidgetDepot->count() < 5) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("🧩 ALGORITHME D'OPTIMISATION (FIRST-FIT)");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #8e44ad; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    int idx = ui->tableDepot->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesDepots.size()) {
        DepotInfo dp = mesDepots[idx];
        double espaceLibre = dp.capaciteMax - dp.quantiteActuelle;
        desc->setText(QString(
            "<div style='background:white; border-radius:12px; padding:30px; border:3px dashed #8e44ad; color:#3e2723; font-size:16px;'>"
            "<h2 style='color:#8e44ad; margin-top:0; text-align:center;'>Analyse de l'emplacement : %1</h2><hr>"
            "Capacité Totale : <b>%2 U</b> | Espace Libre : <b style='color:#27ae60;'>%3 U</b><br><br>"
            "<b>💡 Recommandation IA :</b><br>"
            "En consolidant les stocks de la Zone A avec cette étagère (%4), vous pouvez libérer 1 emplacement complet pour la nouvelle collection."
            "</div>").arg(dp.id).arg(dp.capaciteMax).arg(espaceLibre).arg(dp.etagere));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Sélectionnez un emplacement dans la liste pour l'optimiser.</div>");
    }
    desc->setAlignment(Qt::AlignCenter); l->addWidget(desc, 0, Qt::AlignCenter);
    l->addStretch();
    ui->tabWidgetDepot->setCurrentIndex(4);
}

void MainWindow::showDepotRavitaillementTab() {
    if(ui->tabWidgetDepot->count() < 6) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(5);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch();

    QLabel *titre = new QLabel("🚚 ALERTES RAVITAILLEMENT");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #e67e22; margin-bottom: 20px; text-transform: uppercase;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    int idx = ui->tableDepot->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesDepots.size()) {
        DepotInfo dp = mesDepots[idx];
        double taux = (dp.capaciteMax > 0) ? (dp.quantiteActuelle / dp.capaciteMax) * 100 : 0;
        QString alerte = (taux < 20) ? "<b>CRITIQUE - Ravitaillement urgent requis !</b>" : "<b>Stock suffisant (Pas d'action requise)</b>";

        desc->setText(QString(
            "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #e67e22; color:#3e2723; font-size:16px;'>"
            "<h2 style='color:#d35400; margin-top:0; text-align:center;'>État du stock : %1</h2><hr>"
            "Taux de remplissage : <b>%2%</b><br><br>"
            "Statut : %3<br><br><hr>"
            "<i>Prochaine livraison fournisseur prévue le : " + QDate::currentDate().addDays(4).toString("dd/MM/yyyy") + "</i>"
            "</div>").arg(dp.id).arg(taux, 0, 'f', 1).arg(alerte));
    } else {
        desc->setText("<div style='background:white; padding:20px; border-radius:10px; color:gray; font-style:italic;'>Sélectionnez un emplacement pour vérifier son besoin en ravitaillement.</div>");
    }
    desc->setAlignment(Qt::AlignCenter); l->addWidget(desc, 0, Qt::AlignCenter);
    l->addStretch();
    ui->tabWidgetDepot->setCurrentIndex(5);
}


void MainWindow::construirePageAccueil() {
    QWidget *page = ui->page_home;

    if (page->layout()) {
        clearLayout(page->layout());
        delete page->layout();
    }

    page->setStyleSheet(
        "QWidget#page_home {"
        "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #1a1210, stop:0.3 #2c1a16, stop:0.7 #3e2723, stop:1 #2c1a16);"
        "}"
    );

    QVBoxLayout *mainL = new QVBoxLayout(page);
    mainL->setSpacing(0);
    mainL->setContentsMargins(0, 0, 0, 0);

    mainL->addStretch(2);

    QVBoxLayout *centerL = new QVBoxLayout();
    centerL->setSpacing(12);
    centerL->setAlignment(Qt::AlignCenter);

    QFrame *lineTop = new QFrame();
    lineTop->setFixedWidth(120);
    lineTop->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 transparent,stop:0.5 #d4af37,stop:1 transparent);"
                           "border:none; min-height:2px; max-height:2px;");
    QHBoxLayout *hlLineTop = new QHBoxLayout();
    hlLineTop->addStretch();
    hlLineTop->addWidget(lineTop);
    hlLineTop->addStretch();
    centerL->addLayout(hlLineTop);

    QLabel *logo = new QLabel();
    QPixmap originalLogo(":/logo.png");
    logo->setPixmap(originalLogo.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setFixedSize(180, 180);
    logo->setAlignment(Qt::AlignCenter);
    logo->setScaledContents(false);
    logo->setStyleSheet("border: none; background: transparent;");

    QHBoxLayout *hlLogo = new QHBoxLayout();
    hlLogo->addStretch();
    hlLogo->addWidget(logo);
    hlLogo->addStretch();
    centerL->addLayout(hlLogo);

    QLabel *lblPre = new QLabel("— ATELIER DE MAROQUINERIE DE LUXE —");
    lblPre->setStyleSheet(
        "font-size: 11px; font-weight: 700; color: #a1887f;"
        "letter-spacing: 4px; text-transform: uppercase; border: none;"
    );
    lblPre->setAlignment(Qt::AlignCenter);
    centerL->addWidget(lblPre);

    QLabel *lblTitre = new QLabel("FIL D'OR");
    lblTitre->setStyleSheet(
        "font-size: 72px; font-weight: 200; color: #ffffff;"
        "letter-spacing: 12px; border: none;"
    );
    lblTitre->setAlignment(Qt::AlignCenter);
    centerL->addWidget(lblTitre);

    QFrame *lineGold = new QFrame();
    lineGold->setFixedWidth(300);
    lineGold->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 transparent,stop:0.2 #d4af37,stop:0.8 #d4af37,stop:1 transparent);"
                             "border:none; min-height:2px; max-height:2px;");
    QHBoxLayout *hlLine = new QHBoxLayout();
    hlLine->addStretch();
    hlLine->addWidget(lineGold);
    hlLine->addStretch();
    centerL->addLayout(hlLine);

    QLabel *lblSub = new QLabel("L'Excellence de la Maroquinerie");
    lblSub->setStyleSheet(
        "font-size: 20px; color: #d4af37; font-style: italic;"
        "font-weight: 400; letter-spacing: 2px; border: none;"
    );
    lblSub->setAlignment(Qt::AlignCenter);
    centerL->addWidget(lblSub);

    centerL->addSpacing(10);

    QLabel *lblDesc = new QLabel("Gestion complète de la production, des stocks,\ndes ressources humaines et de la relation client.");
    lblDesc->setStyleSheet(
        "font-size: 13px; color: #bcaaa4; line-height: 1.6; border: none;"
    );
    lblDesc->setAlignment(Qt::AlignCenter);
    centerL->addWidget(lblDesc);

    centerL->addSpacing(30);

    QPushButton *btnEntrer = new QPushButton("  ENTRER DANS L'ATELIER  ➔  ");
    btnEntrer->setCursor(Qt::PointingHandCursor);
    btnEntrer->setMinimumHeight(55);
    btnEntrer->setMaximumWidth(400);
    btnEntrer->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #d4af37;"
        "  font-size: 16px;"
        "  font-weight: 800;"
        "  letter-spacing: 3px;"
        "  text-transform: uppercase;"
        "  padding: 15px 40px;"
        "  border: 2px solid #d4af37;"
        "  border-radius: 30px;"
        "}"
        "QPushButton:hover {"
        "  background: #d4af37;"
        "  color: #1a1210;"
        "  border-color: #d4af37;"
        "}"
    );

    QHBoxLayout *hlBtn = new QHBoxLayout();
    hlBtn->addStretch();
    hlBtn->addWidget(btnEntrer);
    hlBtn->addStretch();
    centerL->addLayout(hlBtn);

    connect(btnEntrer, &QPushButton::clicked, [=](){
        ui->stackedWidget->setCurrentWidget(ui->page_login);
    });

    mainL->addLayout(centerL);

    mainL->addStretch(2);

    QLabel *footer = new QLabel("© 2026 FIL D'OR — Tous droits réservés — Atelier de Production");
    footer->setStyleSheet(
        "font-size: 10px; color: rgba(161,136,127,0.4); border: none; padding: 12px;"
    );
    footer->setAlignment(Qt::AlignCenter);
    mainL->addWidget(footer);
}

void MainWindow::construirePageLogin() {
    QWidget *page = ui->page_login;

    if (page->layout()) {
        clearLayout(page->layout());
        delete page->layout();
    }

    page->setStyleSheet(
        "QWidget#page_login {"
        "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #1a1210, stop:0.3 #2c1a16, stop:0.7 #3e2723, stop:1 #2c1a16);"
        "}"
    );

    QVBoxLayout *mainL = new QVBoxLayout(page);
    mainL->setSpacing(0);
    mainL->setContentsMargins(0, 0, 0, 0);

    mainL->addStretch(2);

    QFrame *card = new QFrame();
    card->setFixedWidth(480);
    card->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,0,0,0.35);"
        "  border: 1px solid rgba(212,175,55,0.25);"
        "  border-radius: 20px;"
        "}"
    );

    QVBoxLayout *cardL = new QVBoxLayout(card);
    cardL->setSpacing(16);
    cardL->setContentsMargins(40, 35, 40, 35);

    QLabel *icoLock = new QLabel("🔐");
    icoLock->setStyleSheet("font-size: 40px; border: none;");
    icoLock->setAlignment(Qt::AlignCenter);
    cardL->addWidget(icoLock);

    QLabel *lblTitre = new QLabel("Connexion Sécurisée");
    lblTitre->setStyleSheet(
        "font-size: 26px; font-weight: 800; color: #ffffff;"
        "letter-spacing: 1px; border: none;"
    );
    lblTitre->setAlignment(Qt::AlignCenter);
    cardL->addWidget(lblTitre);

    QLabel *lblSub = new QLabel("Accédez à votre espace de gestion FIL D'OR");
    lblSub->setStyleSheet(
        "font-size: 12px; color: #d4af37; font-style: italic; border: none;"
    );
    lblSub->setAlignment(Qt::AlignCenter);
    cardL->addWidget(lblSub);

    QFrame *lineGold = new QFrame();
    lineGold->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 transparent,stop:0.2 #d4af37,stop:0.8 #d4af37,stop:1 transparent);"
        "border:none; min-height:2px; max-height:2px;"
    );
    cardL->addWidget(lineGold);

    cardL->addSpacing(8);

    QString styleInput =
        "QLineEdit {"
        "  background: rgba(255,255,255,0.08);"
        "  border: 1px solid rgba(212,175,55,0.3);"
        "  border-radius: 12px;"
        "  padding: 14px 18px;"
        "  font-size: 14px;"
        "  color: #ffffff;"
        "  selection-background-color: rgba(212,175,55,0.3);"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #d4af37;"
        "  background: rgba(255,255,255,0.12);"
        "}"
        "QLineEdit::placeholder {"
        "  color: rgba(255,255,255,0.35);"
        "}";

    QLabel *lblNom = new QLabel("👤  Nom");
    lblNom->setStyleSheet("font-size: 12px; font-weight: 700; color: #e0c097; border: none; margin-bottom: 2px;");
    cardL->addWidget(lblNom);

    QLineEdit *leNom = ui->le_login_nom;
    leNom->setParent(card);
    leNom->setPlaceholderText("Entrez votre nom...");
    leNom->setStyleSheet(styleInput);
    leNom->setMinimumHeight(48);
    cardL->addWidget(leNom);

    QLabel *lblPre = new QLabel("👤  Prénom");
    lblPre->setStyleSheet("font-size: 12px; font-weight: 700; color: #e0c097; border: none; margin-bottom: 2px;");
    cardL->addWidget(lblPre);

    QLineEdit *lePre = ui->le_login_prenom;
    lePre->setParent(card);
    lePre->setPlaceholderText("Entrez votre prénom...");
    lePre->setStyleSheet(styleInput);
    lePre->setMinimumHeight(48);
    cardL->addWidget(lePre);

    QLabel *lblMdp = new QLabel("🔑  Mot de passe");
    lblMdp->setStyleSheet("font-size: 12px; font-weight: 700; color: #e0c097; border: none; margin-bottom: 2px;");
    cardL->addWidget(lblMdp);

    QLineEdit *leMdp = ui->le_login_mdp;
    leMdp->setParent(card);
    leMdp->setPlaceholderText("Entrez votre mot de passe...");
    leMdp->setEchoMode(QLineEdit::Password);
    leMdp->setStyleSheet(styleInput);
    leMdp->setMinimumHeight(48);
    cardL->addWidget(leMdp);

    cardL->addSpacing(12);

    QHBoxLayout *hlBtns = new QHBoxLayout();
    hlBtns->setSpacing(14);

    QPushButton *btnRetour = ui->btn_login_back;
    btnRetour->setParent(card);
    btnRetour->setText("← Retour");
    btnRetour->setCursor(Qt::PointingHandCursor);
    btnRetour->setMinimumHeight(48);
    btnRetour->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #a1887f;"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  padding: 12px 24px;"
        "  border: 1px solid rgba(161,136,127,0.4);"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(161,136,127,0.15);"
        "  color: #e0c097;"
        "  border-color: #e0c097;"
        "}"
    );
    hlBtns->addWidget(btnRetour);

    hlBtns->addStretch();

    QPushButton *btnLogin = ui->btn_login;
    btnLogin->setParent(card);
    btnLogin->setText("SE CONNECTER  ➔");
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setMinimumHeight(48);
    btnLogin->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #d4af37, stop:1 #8d5524);"
        "  color: #1a1210;"
        "  font-size: 14px;"
        "  font-weight: 900;"
        "  letter-spacing: 2px;"
        "  text-transform: uppercase;"
        "  padding: 12px 32px;"
        "  border: none;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #e0c097, stop:1 #d4af37);"
        "}"
        "QPushButton:pressed {"
        "  background: #8d5524;"
        "}"
    );
    hlBtns->addWidget(btnLogin);

    cardL->addLayout(hlBtns);

    QLabel *lblInfo = new QLabel("🔒 Connexion chiffrée — Authentification RFID supportée");
    lblInfo->setStyleSheet("font-size: 10px; color: rgba(161,136,127,0.5); border: none; margin-top: 8px;");
    lblInfo->setAlignment(Qt::AlignCenter);
    cardL->addWidget(lblInfo);

    QHBoxLayout *hlCard = new QHBoxLayout();
    hlCard->addStretch();
    hlCard->addWidget(card);
    hlCard->addStretch();

    mainL->addLayout(hlCard);

    mainL->addStretch(2);

    QLabel *footer = new QLabel("© 2026 FIL D'OR — Atelier de Maroquinerie de Luxe");
    footer->setStyleSheet("font-size: 10px; color: rgba(161,136,127,0.35); border: none; padding: 10px;");
    footer->setAlignment(Qt::AlignCenter);
    mainL->addWidget(footer);
}
void MainWindow::construirePageEtapes() {
    QWidget *page = ui->page_fab_list;

    QList<QWidget*> enfants = page->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *w : enfants) { w->setParent(nullptr); w->deleteLater(); }
    if (page->layout()) { delete page->layout(); }

    selectedEtapeId = -1;
    selectedEtapePlanifId = -1;

    QVBoxLayout *pageL = new QVBoxLayout(page);
    pageL->setContentsMargins(10, 10, 10, 5);
    pageL->setSpacing(0);

    QTabWidget *tabEtapes = new QTabWidget();
    tabEtapes->setObjectName("tabWidgetEtapes");
    tabEtapes->setStyleSheet(
        "QTabWidget::pane {"
        "  border: 1px solid #d7ccc8;"
        "  background: white;"
        "  border-radius: 8px;"
        "  margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: #f3f0eb;"
        "  color: #5d4037;"
        "  border: 1px solid #d7ccc8;"
        "  border-bottom-color: #d7ccc8;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  min-width: 150px;"
        "  padding: 10px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #8d5524;"
        "  color: white;"
        "  border-color: #8d5524;"
        "  border-bottom-color: #8d5524;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e0c097;"
        "}"
    );

    pageL->addWidget(tabEtapes);

    QWidget *tabCrud = new QWidget();
    tabCrud->setStyleSheet("background: transparent;");
    QVBoxLayout *crudMainL = new QVBoxLayout(tabCrud);
    crudMainL->setSpacing(8);
    crudMainL->setContentsMargins(16, 12, 16, 8);

    QHBoxLayout *titreL = new QHBoxLayout();
    QLabel *titre = new QLabel("Suivi des Etapes de Fabrication");
    titre->setStyleSheet("font-size: 22px; font-weight: 300; color: #2c1a16;");
    titreL->addWidget(titre);
    titreL->addStretch();
    crudMainL->addLayout(titreL);

    QFrame *searchFrame = new QFrame();
    searchFrame->setFixedHeight(45);
    searchFrame->setStyleSheet(
        "QFrame { background: white; border-radius: 10px; border: 1px solid #e0d6cc; }"
    );
    QHBoxLayout *searchL = new QHBoxLayout(searchFrame);
    searchL->setContentsMargins(12, 0, 6, 0);
    searchL->setSpacing(6);

    QLineEdit *leSearch = new QLineEdit();
    leSearch->setPlaceholderText("Rechercher par commande ou etape...");
    leSearch->setStyleSheet("QLineEdit { border: none; font-size: 12px; color: #3e2723; background: transparent; }");
    searchL->addWidget(leSearch, 1);

    auto styleBtnTool = [](QString bg) -> QString {
        return QString("QPushButton { background: %1; color: white; padding: 6px 12px; font-weight: bold; "
                       "font-size: 11px; border-radius: 6px; border: none; } "
                       "QPushButton:hover { background: %1; opacity: 0.85; } "
                       "QPushButton:pressed { padding: 7px 11px; }").arg(bg);
    };

    QPushButton *btnSearch = new QPushButton("Chercher");
    btnSearch->setStyleSheet(styleBtnTool("#795548"));
    searchL->addWidget(btnSearch);
    QPushButton *btnTriCmd = new QPushButton("Tri");
    btnTriCmd->setStyleSheet(styleBtnTool("#5d4037"));
    searchL->addWidget(btnTriCmd);
    QPushButton *btnAlertes = new QPushButton("Alertes");
    btnAlertes->setStyleSheet(styleBtnTool("#c62828"));
    searchL->addWidget(btnAlertes);
    QPushButton *btnRefresh = new QPushButton("Actualiser");
    btnRefresh->setStyleSheet(styleBtnTool("#607d8b"));
    searchL->addWidget(btnRefresh);
    QPushButton *btnGenerer = new QPushButton("+ Generer");
    btnGenerer->setStyleSheet(styleBtnTool("#8d5524"));
    searchL->addWidget(btnGenerer);

    crudMainL->addWidget(searchFrame);

    QTableWidget *tbl = new QTableWidget();
    tbl->setObjectName("tableEtapes");
    tbl->setColumnCount(8);
    tbl->setHorizontalHeaderLabels({"ID", "Cmd", "Produit", "Employe", "Etape", "Temps", "Delta", "Alerte"});
    tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbl->setSelectionMode(QAbstractItemView::SingleSelection);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->verticalHeader()->setVisible(false);
    tbl->setAlternatingRowColors(true);
    tbl->setStyleSheet(
        "QTableWidget {"
        "  background-color: #faf8f5;"
        "  alternate-background-color: #f3ece4;"
        "  gridline-color: transparent;"
        "  border: none;"
        "  border-radius: 8px;"
        "  selection-background-color: rgba(141,85,36,0.15);"
        "  selection-color: #3e2723;"
        "  font-size: 12px;"
        "}"
        "QTableWidget::item {"
        "  padding: 6px 10px;"
        "  border-bottom: 1px solid rgba(215,204,200,0.4);"
        "}"
        "QHeaderView::section {"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3e2723,stop:1 #2c1a16);"
        "  color: #e0c097;"
        "  padding: 10px 8px;"
        "  border: none;"
        "  font-weight: 700;"
        "  font-size: 12px;"
        "  letter-spacing: 1px;"
        "}"
    );
    crudMainL->addWidget(tbl, 1);

    QHBoxLayout *crudBtns = new QHBoxLayout();
    crudBtns->setSpacing(8);
    crudBtns->addStretch();
    QPushButton *btnModifier = new QPushButton("Modifier");
    btnModifier->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #795548, stop:1 #8d6e63); "
        "color: white; padding: 8px 20px; font-weight: bold; border-radius: 8px; border: none; font-size: 12px; }"
        "QPushButton:hover { background: #6d4c41; }"
    );
    crudBtns->addWidget(btnModifier);
    QPushButton *btnSupprimer = new QPushButton("Supprimer");
    btnSupprimer->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #c62828, stop:1 #e53935); "
        "color: white; padding: 8px 20px; font-weight: bold; border-radius: 8px; border: none; font-size: 12px; }"
        "QPushButton:hover { background: #b71c1c; }"
    );
    crudBtns->addWidget(btnSupprimer);
    crudMainL->addLayout(crudBtns);

    QFrame *frameSup = new QFrame();
    frameSup->setFixedHeight(85);
    frameSup->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,255,255,0.95), stop:1 rgba(245,240,235,0.95)); "
        "border-radius: 12px; border: 1px solid rgba(141,85,36,0.2); }"
    );
    QHBoxLayout *supL = new QHBoxLayout(frameSup);
    supL->setContentsMargins(14, 6, 14, 6);
    supL->setSpacing(12);

    QVBoxLayout *infoL = new QVBoxLayout(); infoL->setSpacing(1);
    QLabel *supTitle = new QLabel("Selectionnez une etape...");
    supTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #3e2723; border: none; background: transparent;");
    infoL->addWidget(supTitle);
    QLabel *supDetail = new QLabel("-");
    supDetail->setStyleSheet("color: #8d6e63; border: none; font-size: 10px; background: transparent;");
    infoL->addWidget(supDetail);
    supL->addLayout(infoL, 1);

    QFrame *vSep = new QFrame();
    vSep->setFrameShape(QFrame::VLine);
    vSep->setStyleSheet("color: rgba(141,85,36,0.2);");
    supL->addWidget(vSep);

    QGridLayout *formG = new QGridLayout(); formG->setSpacing(3);
    QLabel *lEt = new QLabel("Etape:"); lEt->setStyleSheet("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;");
    QComboBox *supEtape = new QComboBox(); supEtape->addItems({"Coupe", "Assemblage", "Couture", "Finition"});
    supEtape->setStyleSheet("QComboBox { border: 1px solid #d7ccc8; border-radius: 4px; padding: 3px 6px; font-size: 11px; background: white; min-width: 90px; }");
    QLabel *lPr = new QLabel("Prevu:"); lPr->setStyleSheet("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;");
    QLabel *supPrevu = new QLabel("2 h"); supPrevu->setStyleSheet("font-weight: 800; color: #e65100; font-size: 13px; border: none; background: transparent;");
    QLabel *lRe = new QLabel("Reel:"); lRe->setStyleSheet("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;");
    QDoubleSpinBox *supTemps = new QDoubleSpinBox(); supTemps->setMaximum(100); supTemps->setSingleStep(0.5); supTemps->setSuffix(" h");
    supTemps->setStyleSheet("QDoubleSpinBox { border: 1px solid #d7ccc8; border-radius: 4px; padding: 3px; font-size: 11px; background: white; min-width: 70px; }");
    formG->addWidget(lEt, 0, 0); formG->addWidget(supEtape, 0, 1);
    formG->addWidget(lPr, 1, 0); formG->addWidget(supPrevu, 1, 1);
    formG->addWidget(lRe, 2, 0); formG->addWidget(supTemps, 2, 1);
    supL->addLayout(formG);

    QPushButton *btnValider = new QPushButton("VALIDER");
    btnValider->setFixedSize(90, 55);
    btnValider->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8d5524, stop:1 #6d3f1a); "
        "color: white; font-weight: 900; font-size: 13px; border-radius: 10px; border: none; letter-spacing: 1px; } "
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #a0673b, stop:1 #8d5524); } "
        "QPushButton:pressed { background: #5d3a1a; }"
    );
    supL->addWidget(btnValider);

    QFrame *deltaFrame = new QFrame();
    deltaFrame->setFixedSize(70, 55);
    deltaFrame->setStyleSheet("QFrame { background: rgba(141,85,36,0.08); border-radius: 10px; border: 1px solid rgba(141,85,36,0.15); }");
    QVBoxLayout *deltaL = new QVBoxLayout(deltaFrame); deltaL->setContentsMargins(4, 4, 4, 4); deltaL->setSpacing(0);
    QLabel *supDelta = new QLabel("...");
    supDelta->setStyleSheet("font-weight: 900; font-size: 14px; color: #757575; border: none; background: transparent;");
    supDelta->setAlignment(Qt::AlignCenter);
    QLabel *deltaLbl = new QLabel("DELTA");
    deltaLbl->setStyleSheet("font-size: 7px; font-weight: 700; color: #8d6e63; border: none; background: transparent; letter-spacing: 1px;");
    deltaLbl->setAlignment(Qt::AlignCenter);
    deltaL->addWidget(supDelta); deltaL->addWidget(deltaLbl);
    supL->addWidget(deltaFrame);

    crudMainL->addWidget(frameSup, 0);

    tabEtapes->addTab(tabCrud, "Suivi des Etapes");

    QWidget *tabTimeline = new QWidget();
    QVBoxLayout *tlMainL = new QVBoxLayout(tabTimeline);
    tlMainL->setSpacing(8);
    tlMainL->setContentsMargins(16, 12, 16, 8);

    QFrame *tlHeaderFrame = new QFrame();
    tlHeaderFrame->setFixedHeight(50);
    tlHeaderFrame->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2c1a16, stop:1 #5d4037); border-radius: 10px; }");
    QHBoxLayout *tlHdrL = new QHBoxLayout(tlHeaderFrame);
    tlHdrL->setContentsMargins(16, 0, 16, 0);
    QLabel *tlTitre = new QLabel("TIMELINE DE FABRICATION");
    tlTitre->setStyleSheet("color: white; font-size: 14px; font-weight: 800; letter-spacing: 2px; border: none; background: transparent;");
    tlHdrL->addWidget(tlTitre);
    tlHdrL->addStretch();

    auto legendDot = [](QString color, QString text) -> QLabel* {
        QLabel *l = new QLabel(text);
        l->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px; border: none; background: transparent;").arg(color));
        return l;
    };
    tlHdrL->addWidget(legendDot("#1565c0", "Coupe"));
    tlHdrL->addWidget(legendDot("#ef6c00", "Assemblage"));
    tlHdrL->addWidget(legendDot("#2e7d32", "Couture"));
    tlHdrL->addWidget(legendDot("#f9a825", "Finition"));
    tlHdrL->addWidget(legendDot("#c62828", "Retard"));

    tlMainL->addWidget(tlHeaderFrame);

    QPushButton *btnRefreshTl = new QPushButton("Actualiser");
    btnRefreshTl->setStyleSheet(styleBtnTool("#607d8b"));
    tlMainL->addWidget(btnRefreshTl, 0, Qt::AlignLeft);

    QTableWidget *tableGantt = new QTableWidget();
    tableGantt->setObjectName("tableGantt");
    tableGantt->verticalHeader()->setVisible(false);
    tableGantt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableGantt->setSelectionBehavior(QAbstractItemView::SelectRows);
    tlMainL->addWidget(tableGantt, 1);

    tabEtapes->addTab(tabTimeline, "Timeline ");

    QWidget *tabStats = new QWidget();
    tabStats->setObjectName("tab_etape_stats");
    QVBoxLayout *stMainL = new QVBoxLayout(tabStats);
    stMainL->setSpacing(14);
    stMainL->setContentsMargins(16, 12, 16, 8);

    tabEtapes->addTab(tabStats, "Statistiques");

    pageL->addWidget(tabEtapes);

    auto remplirTable = [=](QSqlQueryModel *model) {
        tbl->setRowCount(0);
        int rows = model->rowCount();
        tbl->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idSuivi = model->record(i).value("ID_SUIVI").toInt();
            int idPlanif = model->record(i).value("ID_PLANIFICATION").toInt();
            QString produit = model->record(i).value("PRODUIT").toString();
            QString employe = model->record(i).value("EMPLOYE").toString();
            QString etape = model->record(i).value("ETAPE_ACTUELLE").toString();
            double tempsReel = model->record(i).value("TEMPS_REEL_PASSE").toDouble();
            double delta = model->record(i).value("DELTA").toDouble();
            int alerte = model->record(i).value("ALERTE_ACTIVE").toInt();

            QString couleurEtape, bgEtape;
            if (etape == "Coupe")           { couleurEtape = "#1565c0"; bgEtape = "rgba(21,101,192,0.1)"; }
            else if (etape == "Assemblage") { couleurEtape = "#ef6c00"; bgEtape = "rgba(239,108,0,0.1)"; }
            else if (etape == "Couture")    { couleurEtape = "#2e7d32"; bgEtape = "rgba(46,125,50,0.1)"; }
            else if (etape == "Finition")   { couleurEtape = "#f9a825"; bgEtape = "rgba(249,168,37,0.1)"; }
            else { couleurEtape = "#757575"; bgEtape = "rgba(117,117,117,0.1)"; }

            QFont fbold; fbold.setBold(true);
            QFont fnormal; fnormal.setPointSize(10);

            QTableWidgetItem *it0 = new QTableWidgetItem(QString::number(idSuivi));
            it0->setData(Qt::UserRole, idSuivi);
            it0->setData(Qt::UserRole + 1, idPlanif);
            it0->setTextAlignment(Qt::AlignCenter);
            it0->setForeground(QColor("#8d6e63"));
            tbl->setItem(i, 0, it0);

            QTableWidgetItem *it1 = new QTableWidgetItem(QString::number(idPlanif));
            it1->setFont(fbold);
            it1->setTextAlignment(Qt::AlignCenter);
            it1->setForeground(QColor("#3e2723"));
            tbl->setItem(i, 1, it1);

            QTableWidgetItem *itP = new QTableWidgetItem(produit);
            itP->setFont(fbold);
            itP->setForeground(QColor("#4e342e"));
            tbl->setItem(i, 2, itP);

            QTableWidgetItem *it3 = new QTableWidgetItem(employe);
            it3->setForeground(QColor("#6d4c41"));
            tbl->setItem(i, 3, it3);

            QLabel *lblEtape = new QLabel(etape);
            lblEtape->setAlignment(Qt::AlignCenter);
            lblEtape->setStyleSheet(
                QString("background-color: %1; color: white; font-weight: bold; "
                        "font-size: 11px; border-radius: 10px; padding: 4px 12px; "
                        "margin: 3px 6px;").arg(couleurEtape)
            );
            tbl->setCellWidget(i, 4, lblEtape);

            QTableWidgetItem *itT = new QTableWidgetItem(QString::number(tempsReel, 'f', 1) + " h");
            itT->setTextAlignment(Qt::AlignCenter);
            itT->setForeground(QColor("#5d4037"));
            itT->setFont(fbold);
            tbl->setItem(i, 5, itT);

            QLabel *lblDelta = new QLabel();
            QString deltaText = (delta > 0 ? "+" : "") + QString::number(delta, 'f', 1) + "h";
            lblDelta->setText(deltaText);
            lblDelta->setAlignment(Qt::AlignCenter);
            if (delta > 0) {
                lblDelta->setStyleSheet(
                    "background-color: rgba(198,40,40,0.12); color: #c62828; font-weight: 900; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            } else if (delta < 0) {
                lblDelta->setStyleSheet(
                    "background-color: rgba(46,125,50,0.12); color: #2e7d32; font-weight: 900; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            } else {
                lblDelta->setStyleSheet(
                    "background-color: rgba(0,0,0,0.05); color: #9e9e9e; font-weight: 700; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            }
            tbl->setCellWidget(i, 6, lblDelta);

            QLabel *lblAlerte = new QLabel();
            lblAlerte->setAlignment(Qt::AlignCenter);
            if (alerte) {
                lblAlerte->setText("ALERTE");
                lblAlerte->setStyleSheet(
                    "background-color: #c62828; color: white; font-weight: 900; "
                    "font-size: 10px; border-radius: 10px; padding: 4px 10px; "
                    "margin: 3px 6px; letter-spacing: 1px;");
            } else {
                lblAlerte->setText("OK");
                lblAlerte->setStyleSheet(
                    "background-color: rgba(46,125,50,0.12); color: #2e7d32; font-weight: 800; "
                    "font-size: 10px; border-radius: 10px; padding: 4px 10px; "
                    "margin: 3px 6px;");
            }
            tbl->setCellWidget(i, 7, lblAlerte);

            tbl->setRowHeight(i, 40);
        }
    };
    auto refreshCrud = [=]() {
        Etape e; QSqlQueryModel *m = e.afficher(); remplirTable(m); delete m;
    };

    auto refreshGantt = [=]() {
        while (tableGantt->rowCount() > 0) tableGantt->removeRow(0);
        tableGantt->setColumnCount(0);
        tableGantt->clear();
        tableGantt->setStyleSheet("");
        tableGantt->setShowGrid(true);

        QSqlQuery qCmd;
        qCmd.exec(
            "SELECT DISTINCT e.ID_PLANIFICATION, "
            "NVL(pr.DESIGNATION, 'Commande ' || e.ID_PLANIFICATION) AS PRODUIT, "
            "p.DATE_LANCEMENT, p.DATE_FIN_PREVUE "
            "FROM ETAPES e "
            "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
            "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
            "WHERE e.ID_PLANIFICATION IS NOT NULL "
            "ORDER BY e.ID_PLANIFICATION"
        );

        struct CmdInfo { int id; QString produit; QDate debut, fin; };
        QList<CmdInfo> cmds;
        QDate globalMin = QDate(2099, 1, 1), globalMax = QDate(2000, 1, 1);

        while (qCmd.next()) {
            CmdInfo c;
            c.id = qCmd.value(0).toInt(); c.produit = qCmd.value(1).toString();
            c.debut = qCmd.value(2).toDate(); c.fin = qCmd.value(3).toDate();
            if (!c.debut.isValid()) c.debut = QDate::currentDate();
            if (!c.fin.isValid()) c.fin = c.debut.addDays(8);
            if (c.debut < globalMin) globalMin = c.debut;
            if (c.fin > globalMax) globalMax = c.fin;
            cmds.append(c);
        }
        if (cmds.isEmpty()) return;

        globalMin = globalMin.addDays(-2); globalMax = globalMax.addDays(3);
        int nbJours = globalMin.daysTo(globalMax) + 1;
        if (nbJours < 10) nbJours = 14; if (nbJours > 45) nbJours = 45;

        int colProduit = 0;
        int colJourDebut = 1;
        int nbCols = colJourDebut + nbJours;
        tableGantt->setColumnCount(nbCols); tableGantt->setRowCount(cmds.size());

        QStringList headers; headers << "PRODUIT";
        for (int d = 0; d < nbJours; d++) headers << globalMin.addDays(d).toString("dd");
        tableGantt->setHorizontalHeaderLabels(headers);
        tableGantt->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #2c1a16; color: #e0c097; padding: 6px; border: none; font-weight: bold; font-size: 11px; }");
        tableGantt->verticalHeader()->setVisible(false);

        QString sCoupe = "#1565c0", sAssemblage = "#ef6c00", sCouture = "#2e7d32", sFinition = "#f9a825", sRetard = "#c62828";

        for (int r = 0; r < cmds.size(); r++) {
            CmdInfo &cmd = cmds[r];
            QLabel *lblProd = new QLabel("  OF-" + QString::number(cmd.id));
            lblProd->setStyleSheet("font-weight: bold; font-size: 11px; color: #3e2723; padding: 4px; background: transparent;");
            tableGantt->setCellWidget(r, 0, lblProd);

            QSqlQuery qEt;
            qEt.prepare("SELECT ETAPE_ACTUELLE, ALERTE_ACTIVE FROM ETAPES WHERE ID_PLANIFICATION = :id ORDER BY CASE ETAPE_ACTUELLE WHEN 'Coupe' THEN 1 WHEN 'Assemblage' THEN 2 WHEN 'Couture' THEN 3 WHEN 'Finition' THEN 4 ELSE 5 END");
            qEt.bindValue(":id", cmd.id); qEt.exec();
            QMap<QString, int> alertes;
            while (qEt.next()) alertes[qEt.value(0).toString()] = qEt.value(1).toInt();

            int totalJ = cmd.debut.daysTo(cmd.fin); if (totalJ <= 0) totalJ = 4;
            int jpe = qMax(1, totalJ / 4);

            struct EB { QString abr, col; QDate d, f; };
            QList<EB> barres;
            barres.append({"Cp", alertes.value("Coupe",0)?sRetard:sCoupe, cmd.debut, cmd.debut.addDays(jpe-1)});
            barres.append({"As", alertes.value("Assemblage",0)?sRetard:sAssemblage, cmd.debut.addDays(jpe), cmd.debut.addDays(2*jpe-1)});
            barres.append({"Co", alertes.value("Couture",0)?sRetard:sCouture, cmd.debut.addDays(2*jpe), cmd.debut.addDays(3*jpe-1)});
            barres.append({"Fi", alertes.value("Finition",0)?sRetard:sFinition, cmd.debut.addDays(3*jpe), cmd.fin});

            for (const EB &b : barres) {
                bool first = true;
                for (int d = 0; d < nbJours; d++) {
                    QDate j = globalMin.addDays(d);
                    if (j >= b.d && j <= b.f) {
                        QLabel *c = new QLabel(); c->setMinimumHeight(35); c->setAlignment(Qt::AlignCenter);
                        if (first) { c->setText(b.abr); c->setStyleSheet(QString("background-color:%1;color:white;font-weight:bold;font-size:10px;border:none;").arg(b.col)); first = false; }
                        else c->setStyleSheet(QString("background-color:%1;border:none;").arg(b.col));
                        tableGantt->setCellWidget(r, colJourDebut + d, c);
                    }
                }
            }
            tableGantt->setRowHeight(r, 42);
        }
        tableGantt->setColumnWidth(0, 130);
        for (int c = colJourDebut; c < nbCols; c++) tableGantt->setColumnWidth(c, 36);
    };

    auto refreshStats = [=]() {
        QWidget *sp = tabStats;
        if (sp->layout()) {
            QLayoutItem *it;
            while ((it = sp->layout()->takeAt(0)) != nullptr) {
                if (it->widget()) delete it->widget();
                if (it->layout()) { QLayoutItem *s; while ((s = it->layout()->takeAt(0)) != nullptr) { if (s->widget()) delete s->widget(); delete s; } }
                delete it;
            }
            delete sp->layout();
        }

        QVBoxLayout *sL = new QVBoxLayout(sp);
        sL->setSpacing(14); sL->setContentsMargins(16, 12, 16, 8);

        QFrame *stHeader = new QFrame(); stHeader->setFixedHeight(50);
        stHeader->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2c1a16, stop:1 #795548); border-radius: 12px; }");
        QHBoxLayout *stHL = new QHBoxLayout(stHeader); stHL->setContentsMargins(20, 0, 20, 0);
        QLabel *stTitre = new QLabel("TABLEAU DE BORD - FABRICATION");
        stTitre->setStyleSheet("color: white; font-size: 15px; font-weight: 800; letter-spacing: 2px; border: none; background: transparent;");
        stHL->addWidget(stTitre);
        sL->addWidget(stHeader);

        int totalEtapes=0, etTerminees=0, etEnRetard=0, etEnAvance=0;
        double tempsReelTotal=0, tempsPrevuTotal=0;
        int nbCoupe=0, nbAssemblage=0, nbCouture=0, nbFinition=0;

        QSqlQuery q;
        if(q.exec("SELECT COUNT(*) FROM ETAPES")&&q.next()) totalEtapes=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE TEMPS_REEL_PASSE>0")&&q.next()) etTerminees=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE ALERTE_ACTIVE=1")&&q.next()) etEnRetard=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE DELTA<0")&&q.next()) etEnAvance=q.value(0).toInt();
        if(q.exec("SELECT NVL(SUM(TEMPS_REEL_PASSE),0) FROM ETAPES")&&q.next()) tempsReelTotal=q.value(0).toDouble();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE ETAPE_ACTUELLE='Coupe'")&&q.next()) nbCoupe=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE ETAPE_ACTUELLE='Assemblage'")&&q.next()) nbAssemblage=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE ETAPE_ACTUELLE='Couture'")&&q.next()) nbCouture=q.value(0).toInt();
        if(q.exec("SELECT COUNT(*) FROM ETAPES WHERE ETAPE_ACTUELLE='Finition'")&&q.next()) nbFinition=q.value(0).toInt();

        tempsPrevuTotal=nbCoupe*2.0+nbAssemblage*3.0+nbCouture*4.0+nbFinition*2.0;
        double tauxC=(totalEtapes>0)?(double(etTerminees)/totalEtapes)*100.0:0;
        double eff=(tempsPrevuTotal>0)?(tempsReelTotal/tempsPrevuTotal)*100.0:0;

        auto kpi = [](QString val, QString label, QString grad) -> QFrame* {
            QFrame *f = new QFrame(); f->setMinimumHeight(100);
            f->setStyleSheet(QString("QFrame{background:%1;border-radius:14px;}").arg(grad));
            QVBoxLayout *v=new QVBoxLayout(f); v->setSpacing(4);
            QLabel *lv=new QLabel(val); lv->setStyleSheet("color:white;font-size:30px;font-weight:900;border:none;background:transparent;"); lv->setAlignment(Qt::AlignCenter);
            QLabel *ll=new QLabel(label); ll->setStyleSheet("color:rgba(255,255,255,0.75);font-size:10px;font-weight:700;border:none;background:transparent;"); ll->setAlignment(Qt::AlignCenter);
            v->addStretch(); v->addWidget(lv); v->addWidget(ll); v->addStretch();
            return f;
        };

        QHBoxLayout *kpiR=new QHBoxLayout(); kpiR->setSpacing(12);
        kpiR->addWidget(kpi(QString::number(totalEtapes),"Total Etapes","qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #27ae60,stop:1 #00897b)"));
        kpiR->addWidget(kpi(QString::number(tauxC,'f',1)+"%","Completion","qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0277bd,stop:1 #00838f)"));
        kpiR->addWidget(kpi(QString::number(eff,'f',1)+"%","Efficacite",(eff>110)?"qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #c62828,stop:1 #b71c1c)":"qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ef6c00,stop:1 #f9a825)"));
        kpiR->addWidget(kpi(QString::number(etEnRetard),"En Retard",(etEnRetard>0)?"qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #c62828,stop:1 #b71c1c)":"qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2e7d32,stop:1 #1b5e20)"));
        sL->addLayout(kpiR);

        auto barGroup=[](QString t,QList<QPair<QString,int>> d,int tot)->QGroupBox*{
            QGroupBox *g=new QGroupBox(t);
            g->setStyleSheet("QGroupBox{background:#fff;border:2px solid #d7ccc8;border-radius:12px;margin-top:10px;padding:12px;font-weight:700;}QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;color:#8d5524;}");
            QVBoxLayout *v=new QVBoxLayout(g);
            for(auto&p:d){
                QLabel *l=new QLabel(p.first+": "+QString::number(p.second)); l->setStyleSheet("font-weight:bold;font-size:12px;color:#5d4037;"); v->addWidget(l);
                QProgressBar *pb=new QProgressBar(); pb->setMaximum(tot>0?tot:1); pb->setValue(p.second); pb->setFormat("%v/%m");
                pb->setStyleSheet("QProgressBar{border:1px solid #d7ccc8;border-radius:9px;text-align:center;background:#f3f0eb;height:20px;font-weight:700;}QProgressBar::chunk{border-radius:9px;background-color:#8d5524;}");
                v->addWidget(pb);
            }
            v->addStretch(); return g;
        };

        QHBoxLayout *chR=new QHBoxLayout(); chR->setSpacing(12);
        chR->addWidget(barGroup("Par Etape",{{"Coupe",nbCoupe},{"Assemblage",nbAssemblage},{"Couture",nbCouture},{"Finition",nbFinition}},totalEtapes));
        chR->addWidget(barGroup("Par Statut",{{"Terminees",etTerminees},{"Retard",etEnRetard},{"Avance",etEnAvance},{"Attente",totalEtapes-etTerminees}},totalEtapes));
        sL->addLayout(chR);

        QLabel *foot=new QLabel(QString("Reel: %1h | Prevu: %2h | Delta: %3h").arg(tempsReelTotal,0,'f',1).arg(tempsPrevuTotal,0,'f',1).arg(tempsReelTotal-tempsPrevuTotal,0,'f',1));
        foot->setStyleSheet("background:rgba(0,0,0,0.05);padding:12px;border-radius:8px;font-weight:bold;font-size:13px;color:#3e2723;");
        foot->setAlignment(Qt::AlignCenter);
        sL->addWidget(foot);
    };

    refreshCrud();

    connect(btnRefresh,&QPushButton::clicked,[=](){refreshCrud();});
    connect(btnSearch,&QPushButton::clicked,[=](){
        QString c=leSearch->text().trimmed();
        if(c.isEmpty()){refreshCrud();return;}
        bool ok;int id=c.toInt(&ok);
        Etape e;QSqlQueryModel*m=ok?e.rechercherParCommande(id):e.rechercherParEtape(c);
        remplirTable(m);delete m;
    });
    connect(btnTriCmd,&QPushButton::clicked,[=](){Etape e;QSqlQueryModel*m=e.trierParCommande();remplirTable(m);delete m;});
    connect(btnAlertes,&QPushButton::clicked,[=](){Etape e;QSqlQueryModel*m=e.afficherAlertes();remplirTable(m);delete m;});

    connect(tbl, &QTableWidget::cellClicked, [=](int row, int) {
        if (row < 0 || row >= tbl->rowCount()) return;
        if (!tbl->item(row, 0)) return;

        selectedEtapeId = tbl->item(row, 0)->data(Qt::UserRole).toInt();
        selectedEtapePlanifId = tbl->item(row, 0)->data(Qt::UserRole + 1).toInt();

        QString prod = tbl->item(row, 2) ? tbl->item(row, 2)->text() : "-";
        QString emp = tbl->item(row, 3) ? tbl->item(row, 3)->text() : "-";

        QString et = "Coupe";
        QLabel *lblEt = qobject_cast<QLabel*>(tbl->cellWidget(row, 4));
        if (lblEt) et = lblEt->text();

        supTitle->setText("Cmd #" + QString::number(selectedEtapePlanifId) + " - " + prod);
        supDetail->setText("Employe: " + emp + " | ID: " + QString::number(selectedEtapeId));
        int idx = supEtape->findText(et); if (idx >= 0) supEtape->setCurrentIndex(idx);
        double tp = 2.0; if (et == "Assemblage") tp = 3.0; else if (et == "Couture") tp = 4.0;
        supPrevu->setText(QString::number(tp) + " h");
        supDelta->setText("Pret");
        supDelta->setStyleSheet("font-weight:900;font-size:14px;color:#757575;border:none;background:transparent;");
    });

    connect(supEtape,&QComboBox::currentTextChanged,[=](const QString&et){
        double tp=2.0;if(et=="Assemblage")tp=3.0;else if(et=="Couture")tp=4.0;
        supPrevu->setText(QString::number(tp)+" h");
    });

    connect(btnValider, &QPushButton::clicked, [=]() {
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) {
            db.open();
            if (!db.isOpen()) {
                alerteErreur("Connexion", "Impossible de se reconnecter a Oracle.\n" + db.lastError().text());
                return;
            }
        }
        {
            QSqlQuery qTest;
            if (!qTest.exec("SELECT 1 FROM DUAL")) {
                db.close();
                db.open();
                if (!db.isOpen()) {
                    alerteErreur("Connexion", "Connexion Oracle perdue. Relancez l'application.");
                    return;
                }
            }
        }

        int row = tbl->currentRow();
        if (row < 0 || !tbl->item(row, 0)) {
            alerteWarning("Selection", "Selectionnez une etape.");
            return;
        }

        int idS = tbl->item(row, 0)->data(Qt::UserRole).toInt();
        int idP = tbl->item(row, 0)->data(Qt::UserRole + 1).toInt();
        QString et = supEtape->currentText();
        double tr = supTemps->value();

        if (tr <= 0) {
            alerteWarning("Temps", "Saisissez un temps reel > 0.");
            return;
        }

        double tp = 2.0;
        if (et == "Assemblage") tp = 3.0;
        else if (et == "Couture") tp = 4.0;
        else if (et == "Finition") tp = 2.0;
        double d = tr - tp;
        int al = (d > 0) ? 1 : 0;

        QSqlQuery qUpdate;
        qUpdate.prepare(
            "UPDATE ETAPES SET TEMPS_REEL_PASSE = :tr, DELTA = :d, "
            "ALERTE_ACTIVE = :al WHERE ID_SUIVI = :id"
        );
        qUpdate.bindValue(":tr", tr);
        qUpdate.bindValue(":d", d);
        qUpdate.bindValue(":al", al);
        qUpdate.bindValue(":id", idS);

        if (qUpdate.exec()) {
            QSqlQuery().exec("COMMIT");
            alerteSucces("OK", "Etape '" + et + "' validee avec " +
                        QString::number(tr, 'f', 1) + "h");

            refreshCrud();

            verifierFinFabrication(idP);

        } else {
            alerteErreur("Erreur", "Echec mise a jour: " + qUpdate.lastError().text());
        }
    });

    connect(btnGenerer,&QPushButton::clicked,[=](){
        QSqlQuery qC;qC.exec("SELECT p.ID_COMMANDE,e2.ID_EMPLOYE FROM PLANIFICATION p LEFT JOIN EMPLOYES e2 ON p.ID_EMPLOYE=e2.ID_EMPLOYE WHERE p.ID_COMMANDE NOT IN(SELECT DISTINCT ID_PLANIFICATION FROM ETAPES WHERE ID_PLANIFICATION IS NOT NULL)");
        int cnt=0;
        while(qC.next()){int id=qC.value(0).toInt(),emp=qC.value(1).toInt();if(emp==0)emp=1;if(Etape::genererEtapesCommande(id,emp))cnt++;}
        if(cnt>0){alerteSucces("OK",QString("%1 commande(s)").arg(cnt));refreshCrud();}
        else alerteWarning("Info","Toutes les commandes ont deja leurs etapes.");
    });

    connect(btnModifier, &QPushButton::clicked, [=]() {
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) { db.open(); }
        {
            QSqlQuery qTest;
            if (!qTest.exec("SELECT 1 FROM DUAL")) {
                db.close(); db.open();
                if (!db.isOpen()) {
                    alerteErreur("Connexion", "Connexion Oracle perdue.");
                    return;
                }
            }
        }
        int row = tbl->currentRow();
        if (row < 0 || !tbl->item(row, 0)) {
            alerteWarning("Selection", "Selectionnez une etape.");
            return;
        }

        int idS = tbl->item(row, 0)->data(Qt::UserRole).toInt();
        int idP = tbl->item(row, 0)->data(Qt::UserRole + 1).toInt();
        QString et = supEtape->currentText();
        double tr = supTemps->value();
        double tp = 2.0;
        if (et == "Assemblage") tp = 3.0;
        else if (et == "Couture") tp = 4.0;
        else if (et == "Finition") tp = 2.0;
        double d = tr - tp;
        int al = (d > 0) ? 1 : 0;

        QSqlQuery qE;
        int idEmp = 1;
        qE.prepare("SELECT ID_EMPLOYE FROM ETAPES WHERE ID_SUIVI=:id");
        qE.bindValue(":id", idS);
        if (qE.exec() && qE.next()) idEmp = qE.value(0).toInt();

        Etape e(idP, idEmp, et, tr, d, al);
        if (e.modifier(idS)) {
            alerteSucces("OK", "Etape modifiee.");
            refreshCrud();

            if (tr > 0) {
                verifierFinFabrication(idP);
            }
        }
    });

    connect(btnSupprimer,&QPushButton::clicked,[=](){
        int row=tbl->currentRow();if(row<0){alerteWarning("Selection","Selectionnez une etape.");return;}
        int idS=tbl->item(row,0)->data(Qt::UserRole).toInt();
        if(QMessageBox::question(this,"Confirmer","Supprimer?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
            Etape e;if(e.supprimer(idS)){alerteSucces("OK","Supprimee.");refreshCrud();selectedEtapeId=-1;}
        }
    });

    connect(tabEtapes,&QTabWidget::currentChanged,[=](int idx){
        if(idx==1)refreshGantt();else if(idx==2)refreshStats();
    });
    connect(btnRefreshTl,&QPushButton::clicked,[=](){refreshGantt();});
}

void MainWindow::verifierFinFabrication(int idPlanification)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { db.open(); }

    QSqlQuery qCount;
    qCount.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id AND TEMPS_REEL_PASSE > 0");
    qCount.bindValue(":id", idPlanification);
    int etapesTerminees = 0;
    if (qCount.exec() && qCount.next()) etapesTerminees = qCount.value(0).toInt();

    QSqlQuery qTotal;
    qTotal.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id");
    qTotal.bindValue(":id", idPlanification);
    int totalEtapes = 0;
    if (qTotal.exec() && qTotal.next()) totalEtapes = qTotal.value(0).toInt();

    qDebug() << "Commande" << idPlanification << ":" << etapesTerminees << "/" << totalEtapes;

    if (totalEtapes <= 0 || etapesTerminees < totalEtapes) return;


    int idProduit = 0;
    int quantite = 0;
    QString nomProduit = "Produit";
    {
        QSqlQuery q;
        q.prepare("SELECT P.ID_PRODUIT, P.QUANTITE, PR.DESIGNATION "
                  "FROM PLANIFICATION P "
                  "JOIN PRODUITS PR ON P.ID_PRODUIT = PR.ID_PRODUIT "
                  "WHERE P.ID_COMMANDE = :id");
        q.bindValue(":id", idPlanification);
        if (q.exec() && q.next()) {
            idProduit = q.value(0).toInt();
            quantite = q.value(1).toInt();
            nomProduit = q.value(2).toString();
        }
    }

    {
        QSqlQuery q;
        q.prepare("UPDATE PLANIFICATION SET STATUT = 'Termine' WHERE ID_COMMANDE = :id");
        q.bindValue(":id", idPlanification);
        q.exec();
    }

    if (idProduit > 0) {
        QSqlQuery q;
        q.prepare("UPDATE PRODUITS SET REQUIS = NVL(REQUIS, 0) + :qte WHERE ID_PRODUIT = :id");
        q.bindValue(":qte", quantite);
        q.bindValue(":id", idProduit);
        q.exec();
    }

    double tempsTotal = 0;
    double deltaTotal = 0;
    {
        QSqlQuery q;
        q.prepare("SELECT SUM(TEMPS_REEL_PASSE), SUM(DELTA) FROM ETAPES WHERE ID_PLANIFICATION = :id");
        q.bindValue(":id", idPlanification);
        if (q.exec() && q.next()) {
            tempsTotal = q.value(0).toDouble();
            deltaTotal = q.value(1).toDouble();
        }
    }

    {
        QSqlQuery q;
        q.exec("COMMIT");
    }

    QString statMsg = (deltaTotal <= 0)
        ? "EN AVANCE de " + QString::number(qAbs(deltaTotal), 'f', 1) + "h"
        : "EN RETARD de " + QString::number(deltaTotal, 'f', 1) + "h";

    QString alerteMsg = QString(
        "<div style='font-family:Arial; line-height:1.8;'>"
        "<h2 style='color:#2e7d32; text-align:center;'>FABRICATION TERMINEE !</h2>"
        "<hr style='border:2px solid #4caf50;'>"
        "<table style='width:100%%; margin:10px 0;'>"
        "<tr><td><b>Commande :</b></td>"
        "<td style='font-weight:900; font-size:16px;'>OF-%1</td></tr>"
        "<tr><td><b>Produit :</b></td>"
        "<td style='color:#4e342e; font-weight:bold;'>%2</td></tr>"
        "<tr><td><b>Quantite :</b></td>"
        "<td style='color:#1565c0; font-weight:900; font-size:16px;'>%3 unites</td></tr>"
        "<tr><td><b>Temps total :</b></td>"
        "<td>%4 heures</td></tr>"
        "<tr><td><b>Performance :</b></td>"
        "<td style='color:%5; font-weight:bold;'>%6</td></tr>"
        "</table>"
        "<hr style='border:1px dashed #ccc;'>"
        "<div style='padding:10px; background:%7; border-radius:8px; text-align:center;'>"
        "<b>Actions automatiques :</b><br>"
        "1. Statut → <b>Termine</b><br>"
        "2. Stock produit → <b>+%3 unites</b><br>"
        "3. Module Produits → <b>Notifie</b>"
        "</div></div>"
    )
    .arg(idPlanification)
    .arg(nomProduit)
    .arg(quantite)
    .arg(tempsTotal, 0, 'f', 1)
    .arg(deltaTotal <= 0 ? "#2e7d32" : "#c62828")
    .arg(statMsg)
    .arg(deltaTotal <= 0 ? "#e8f5e9" : "#ffebee");

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Fabrication Terminee");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(alerteMsg);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(
        "QMessageBox { background: white; }"
        "QPushButton { background:#2e7d32; color:white; padding:8px 20px; "
        "font-weight:bold; border-radius:6px; }"
    );
    msgBox.exec();

    rafraichirListeCommandes();

    qDebug() << "FABRICATION TERMINEE - OF-" << idPlanification << nomProduit << "x" << quantite;
}
