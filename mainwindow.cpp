#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <QPageSize>
#include <QProgressBar>
#include <QFileInfo>
#include <QDateTime>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QMap>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QDateEdit>
#include <cmath>
#include <QLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QMargins>
#include <QColor>
#include <QFont>
#include <QEasingCurve>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>

// Qt 6: QtCharts types (QChart, QPieSeries, ...) live in the global namespace.

static QList<QColor> chartPaletteFILdOr()
{
    return {
        QColor("#d4af37"), // gold
        QColor("#3e2723"), // deep brown
        QColor("#e0c097"), // beige
        QColor("#8d5524"), // caramel
        QColor("#b87333"), // copper
        QColor("#a1887f"), // taupe
    };
}

static void styleChartBase(QChart *chart)
{
    if(!chart) return;
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setTitle(QString());

    QFont baseFont = chart->font();
    baseFont.setFamily("Segoe UI");
    baseFont.setPointSize(9);
    chart->setFont(baseFont);

    if(chart->legend()) {
        chart->legend()->setVisible(true);
        chart->legend()->setLabelColor(QColor("#3e2723"));
        chart->legend()->setFont(baseFont);
        chart->legend()->setContentsMargins(0, 0, 0, 0);
    }
}

static void styleChartView(QChartView *view)
{
    if(!view) return;
    view->setRenderHint(QPainter::Antialiasing);
    view->setFrameShape(QFrame::NoFrame);
    view->setStyleSheet("background: transparent;");
    view->setContentsMargins(0, 0, 0, 0);
    // Keep charts centered and readable inside cards.
    view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    view->setMinimumHeight(240);
    view->setMaximumSize(700, 300);
}

static void clearLayout(QLayout *layout)
{
    if(!layout) return;
    while(QLayoutItem *child = layout->takeAt(0)) {
        if(QWidget *w = child->widget()) w->deleteLater();
        if(QLayout *l = child->layout()) clearLayout(l);
        delete child;
    }
}

static QVBoxLayout* ensureVBox(QWidget *w)
{
    if(!w) return nullptr;
    if(auto *existing = qobject_cast<QVBoxLayout*>(w->layout())) return existing;
    if(w->layout()) {
        clearLayout(w->layout());
        delete w->layout();
    }
    auto *vl = new QVBoxLayout(w);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    return vl;
}

static void setPieChart(QWidget *container, const QString &legendTitle, const QList<QPair<QString,double>> &slices)
{
    if(!container) return;
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(container->minimumHeight() < 260) container->setMinimumHeight(260);

    auto *vl = ensureVBox(container);
    clearLayout(vl);

    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    QList<QPair<QString,double>> sorted = slices;
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b){ return a.second > b.second; });

    double total = 0.0;
    for(const auto &s : sorted) if(s.second > 0.0) total += s.second;
    if(total <= 0.0) {
        sorted = {{"Aucune donnée", 1.0}};
        total = 1.0;
    }

    auto *series = new QPieSeries();
    series->setHoleSize(0.55);
    // Smaller and centered donut.
    series->setPieSize(0.80);
    series->setPieStartAngle(90);

    const QLocale loc = QLocale::system();
    QStringList names;
    for(const auto &s : sorted) {
        if(s.second <= 0.0) continue;
        names << s.first;
        series->append(s.first, s.second);
    }

    QFont sliceFont;
    sliceFont.setFamily("Segoe UI");
    sliceFont.setPointSize(9);
    sliceFont.setBold(true);

    auto isDark = [](const QColor &c) {
        const int y = (c.red() * 299 + c.green() * 587 + c.blue() * 114) / 1000;
        return y < 145;
    };

    const QList<QColor> palette = chartPaletteFILdOr();
    const QColor divider = QColor("#f3f0eb");

    QPieSlice *biggest = nullptr;
    double biggestVal = -1.0;

    const auto sliceList = series->slices();
    for(int i = 0; i < sliceList.size(); ++i) {
        auto *slice = sliceList.at(i);
        if(!slice) continue;
        const QColor c = palette.at(i % palette.size());
        slice->setBrush(QBrush(c));
        slice->setPen(QPen(divider, 2));

        const double val = slice->value();
        const double pct = (total > 0.0) ? (val * 100.0 / total) : 0.0;

        slice->setLabelVisible(true);
        slice->setLabelFont(sliceFont);
        slice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        slice->setLabel(names.value(i) + "\n" + loc.toString(pct, 'f', 0) + "%");
        slice->setLabelColor(isDark(c) ? QColor("#ffffff") : QColor("#3e2723"));

        if(val > biggestVal) { biggestVal = val; biggest = slice; }
    }

    auto *chart = new QChart();
    chart->addSeries(series);
    styleChartBase(chart);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setAnimationDuration(900);
    chart->setAnimationEasingCurve(QEasingCurve::OutCubic);
    if(chart->legend()) {
        // Bottom legend keeps the donut visually centered.
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setBackgroundVisible(false);
    }

    // GroupBox already has a title; keep the plot area clean.
    Q_UNUSED(legendTitle);
    chart->setTitle(QString());

    // Subtle explode on the biggest slice
    if(biggest && series->slices().size() > 1) {
        biggest->setExploded(true);
        biggest->setExplodeDistanceFactor(0.06);
    }

    // Legend: name + value + percent
    if(chart->legend()) {
        const auto markers = chart->legend()->markers(series);
        const int count = qMin(markers.size(), sliceList.size());
        for(int i = 0; i < count; ++i) {
            auto *marker = markers.at(i);
            auto *slice = sliceList.at(i);
            if(!marker || !slice) continue;
            const double val = slice->value();
            const double pct = (total > 0.0) ? (val * 100.0 / total) : 0.0;
            const QString valTxt = (qFuzzyCompare(val, std::round(val)))
                                       ? loc.toString(static_cast<qlonglong>(std::llround(val)))
                                       : loc.toString(val, 'f', 1);
            marker->setLabel(QString("%1 — %2 (%3%)")
                                 .arg(names.value(i))
                                 .arg(valTxt)
                                 .arg(loc.toString(pct, 'f', 0)));
        }
    }

    auto *view = new QChartView(chart);
    styleChartView(view);

    // Bound the view size so charts look "petits" and centered.
    view->setMaximumSize(640, 280);
    view->setMinimumSize(420, 240);
    QTimer::singleShot(0, container, [container, view]() {
        if(!container || !view) return;
        const int w = qMax(0, container->width());
        const int h = qMax(0, container->height());
        if(w <= 0 || h <= 0) return;
        const int targetW = qMin(640, qMax(420, int(w * 0.92)));
        const int targetH = qMin(280, qMax(240, int(h * 0.92)));
        view->setFixedSize(qMin(targetW, w), qMin(targetH, h));
    });

    vl->addStretch(1);
    vl->addWidget(view, 0, Qt::AlignCenter);
    vl->addStretch(1);
}

static void setVerticalBarChart(QWidget *container, const QString &title, const QStringList &categories, const QList<double> &values)
{
    if(!container) return;
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if(container->minimumHeight() < 260) container->setMinimumHeight(260);

    auto *vl = ensureVBox(container);
    clearLayout(vl);

    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    const int n = qMin(categories.size(), values.size());
    QStringList cats = categories.mid(0, n);
    QList<double> vals = values.mid(0, n);
    if(cats.isEmpty()) { cats << "-"; vals << 0.0; }

    // When there is only one bar, QtCharts tends to render it on the left.
    // Pad with blank categories so the visual stays centered.
    if(cats.size() == 1 && vals.size() == 1) {
        cats = {" ", cats.at(0), "  "};
        vals = {0.0, vals.at(0), 0.0};
    }

    auto *set0 = new QBarSet(title);
    for(double v : vals) *set0 << v;
    set0->setBrush(QBrush(QColor("#d4af37")));
    set0->setColor(QColor("#d4af37"));

    auto *series = new QBarSeries();
    series->append(set0);
    series->setBarWidth(0.65);
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value");
    series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    auto *chart = new QChart();
    chart->addSeries(series);
    styleChartBase(chart);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setAnimationDuration(750);
    chart->setAnimationEasingCurve(QEasingCurve::OutCubic);
    if(chart->legend()) chart->legend()->setVisible(false);

    auto *axisX = new QBarCategoryAxis();
    axisX->append(cats);
    axisX->setLabelsColor(QColor("#3e2723"));
    axisX->setGridLineVisible(false);
    axisX->setLabelsAngle(-15);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    double maxV = 1.0;
    for(double v : vals) maxV = qMax(maxV, v);
    auto *axisY = new QValueAxis();
    axisY->setRange(0, maxV * 1.25);
    axisY->applyNiceNumbers();
    axisY->setLabelFormat("%.0f");
    axisY->setLabelsColor(QColor("#3e2723"));
    axisY->setGridLineColor(QColor("#eee5dd"));
    axisY->setLinePenColor(QColor("#d7ccc8"));
    axisY->setTickCount(6);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    auto *view = new QChartView(chart);
    styleChartView(view);

    view->setMaximumSize(640, 280);
    view->setMinimumSize(420, 240);
    QTimer::singleShot(0, container, [container, view]() {
        if(!container || !view) return;
        const int w = qMax(0, container->width());
        const int h = qMax(0, container->height());
        if(w <= 0 || h <= 0) return;
        const int targetW = qMin(640, qMax(420, int(w * 0.92)));
        const int targetH = qMin(280, qMax(240, int(h * 0.92)));
        view->setFixedSize(qMin(targetW, w), qMin(targetH, h));
    });

    vl->addStretch(1);
    vl->addWidget(view, 0, Qt::AlignCenter);
    vl->addStretch(1);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Enable sorting (used by the A→Z sort buttons)
    ui->tablePlanif->setSortingEnabled(true);
    ui->tableProduits->setSortingEnabled(true);
    ui->tableEmployes->setSortingEnabled(true);
    ui->tableStock->setSortingEnabled(true);
    ui->tableClients->setSortingEnabled(true);
    ui->tableDepot->setSortingEnabled(true);

    auto selectedIndexFromTable = [](QTableWidget *table) -> int {
        if(!table) return -1;
        const int r = table->currentRow();
        if(r < 0) return -1;
        QTableWidgetItem *it = table->item(r, 0);
        if(!it) return -1;
        const QVariant v = it->data(Qt::UserRole);
        return v.isValid() ? v.toInt() : r;
    };

    // --- VISUEL ---
    QPixmap logo("logo.png");
    if(!logo.isNull()) {
        ui->l_logo_img->setPixmap(logo.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_big_logo->setPixmap(logo.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_big_logo->setText("");
    } else {
        ui->lbl_big_logo->setText("FIL D'OR");
        ui->l_logo_img->setText("IMG");
    }

    myColorDelegate = new ColorDelegate(this);
    ui->tableTimeline->setItemDelegate(myColorDelegate);

    // --- DONNEES ---
    mesCommandes.append({"OF-101", "Sac Voyage Cuir", 50, "Cuir Vachette", QDate::currentDate(), "10/02/2026", "En cours", "", 0});
    mesCommandes.append({"OF-102", "Portefeuille Luxe", 120, "Cuir Agneau", QDate::currentDate().addDays(1), "12/02/2026", "Planifié", "", 0});
    mesProduits.append({"REF-001", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette Pleine Fleur", 5});
    mesEmployes.append({"EMP-001", "Dupont", "Jean", "Chef Atelier", "Production", QDate(2020, 5, 10), 2800.0, "RF-123"});
    mesMatieres.append({"CUIR-V-001", "Cuir", "BRUT", "Marron", "A (Premium)", 100.0, "M2", "Zone A", "Allée 1", "Standard", QDate::currentDate()});

    mesClients.append({"CL-001", "Ben Salah", "+216 55 123 456", "Tunis", "client1@fildor.tn", 120});
    mesClients.append({"CL-002", "Trabelsi", "+216 24 987 654", "Sfax", "client2@fildor.tn", 60});
    mesClients.append({"CL-003", "Mrad", "+216 21 111 222", "Sousse", "client3@fildor.tn", 15});

    mesDepots.append({"DEP-01", "Zone A / Allée 1", "E1", 500.0, 320.0, "Sec"});
    mesDepots.append({"DEP-02", "Zone A / Allée 2", "E3", 300.0, 90.0, "Froid"});
    mesDepots.append({"DEP-03", "Zone B / Allée 1", "E2", 700.0, 610.0, "Sec"});

    rafraichirListeCommandes(); configurerTimelineGantt();
    rafraichirListeProduits(); rafraichirListeEmployes(); rafraichirListeMatieres();
    rafraichirListeClients(); rafraichirListeDepots();

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

    // Lock access until the user logs in
    setNavigationEnabled(false);

    // --- NAVIGATION ---
    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){
        setNavigationEnabled(false);
        ui->stackedWidget->setCurrentWidget(ui->page_home);
    });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){ configurerTimelineGantt(); ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){ rafraichirListeProduits(); ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_nav_rh, &QPushButton::clicked, [=](){ rafraichirListeEmployes(); ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_nav_stock, &QPushButton::clicked, [=](){ rafraichirListeMatieres(); ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_nav_clients, &QPushButton::clicked, [=](){ rafraichirListeClients(); ui->stackedWidget->setCurrentWidget(ui->page_client_list); });
    connect(ui->btn_nav_depot, &QPushButton::clicked, [=](){ rafraichirListeDepots(); ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });
    connect(ui->btn_start_app, &QPushButton::clicked, [=](){
        ui->le_login_nom->clear();
        ui->le_login_prenom->clear();
        ui->le_login_mdp->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_login);
        ui->le_login_nom->setFocus();
    });

    // --- LOGIN ---
    connect(ui->btn_login_back, &QPushButton::clicked, [=](){
        setNavigationEnabled(false);
        ui->stackedWidget->setCurrentWidget(ui->page_home);
    });
    connect(ui->btn_login, &QPushButton::clicked, [=](){
        const QString nom = ui->le_login_nom->text().trimmed();
        const QString prenom = ui->le_login_prenom->text().trimmed();
        const QString mdp = ui->le_login_mdp->text();

        if(nom.isEmpty()) {
            QMessageBox::warning(this, "Connexion", "Veuillez saisir votre nom.");
            ui->le_login_nom->setFocus();
            return;
        }
        if(prenom.isEmpty()) {
            QMessageBox::warning(this, "Connexion", "Veuillez saisir votre prénom.");
            ui->le_login_prenom->setFocus();
            return;
        }
        if(mdp.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Connexion", "Veuillez saisir votre mot de passe.");
            ui->le_login_mdp->setFocus();
            return;
        }

        ui->le_login_mdp->clear();
        setNavigationEnabled(true);
        rafraichirListeCommandes();
        ui->stackedWidget->setCurrentWidget(ui->page_planif_list);
    });
    // PLANIF : IA ESTIMATION TEMPS (NOUVEAU BOUTON)
    connect(ui->btn_open_planif_ia, &QPushButton::clicked, this, &MainWindow::showPlanifIaDialog);

    // BOUTONS RETOUR
    connect(ui->btn_back_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_fab, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_back_stats, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_stats_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_back_stats_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_back_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_back_inn_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_back_stats_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_back_client, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_client_list); });
    connect(ui->btn_back_stats_client, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_client_list); });
    connect(ui->btn_back_depot, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });
    connect(ui->btn_back_stats_depot, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });

    // --- MODULES ---
    // A→Z sorting buttons
    connect(ui->btn_sort_alpha_planif, &QPushButton::clicked, [=](){ ui->tablePlanif->sortItems(1, Qt::AscendingOrder); });   // Produit
    connect(ui->btn_sort_alpha_prod, &QPushButton::clicked, [=](){ ui->tableProduits->sortItems(1, Qt::AscendingOrder); });    // Désignation
    connect(ui->btn_sort_alpha_emp, &QPushButton::clicked, [=](){ ui->tableEmployes->sortItems(1, Qt::AscendingOrder); });     // Nom
    connect(ui->btn_sort_alpha_stock, &QPushButton::clicked, [=](){ ui->tableStock->sortItems(0, Qt::AscendingOrder); });      // Code MP
    connect(ui->btn_sort_alpha_client, &QPushButton::clicked, [=](){ ui->tableClients->sortItems(1, Qt::AscendingOrder); });   // Nom
    connect(ui->btn_sort_alpha_depot, &QPushButton::clicked, [=](){ ui->tableDepot->sortItems(1, Qt::AscendingOrder); });      // Emplacement

    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){ calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_stats); });
    connect(ui->btn_pdf, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_export_excel_planif, &QPushButton::clicked, [=](){ exporterCSV(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_print_planif, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_close_stats_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });

    // PRODUITS
    connect(ui->btn_cout_produit, &QPushButton::clicked, this, &MainWindow::showProduitCoutDialog);
    connect(ui->btn_hist_mode, &QPushButton::clicked, this, &MainWindow::showHistoriqueModeDialog);
    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){ calculerStatsProduits(); ui->stackedWidget->setCurrentWidget(ui->page_stats_prod); });
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_export_excel_prod, &QPushButton::clicked, [=](){ exporterCSV(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_print_prod, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_close_stats_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });

    // RH
    connect(ui->btn_open_eval, &QPushButton::clicked, this, &MainWindow::showEmpEvalDialog); // [NOUVEAU]
    connect(ui->btn_stats_emp, &QPushButton::clicked, [=](){
        ui->lbl_title_se->setText("🤖 Assistant RH");
        ui->st_rh1->setVisible(false);
        ui->st_rh2->setVisible(false);
        ui->gb_chart_dept->setVisible(false);
        ui->gb_chat->setVisible(true);
        if(ui->hl_chat_area) {
            ui->hl_chat_area->setStretch(0, 0);
            ui->hl_chat_area->setStretch(1, 1);
        }
        ui->stackedWidget->setCurrentWidget(ui->page_stats_emp);
    });
    connect(ui->btn_stats_emp_stats, &QPushButton::clicked, [=](){
        calculerStatsRH();
        ui->lbl_title_se->setText("📊 Statistiques RH");
        ui->st_rh1->setVisible(true);
        ui->st_rh2->setVisible(true);
        ui->gb_chart_dept->setVisible(true);
        ui->gb_chat->setVisible(false);
        if(ui->hl_chat_area) {
            ui->hl_chat_area->setStretch(0, 1);
            ui->hl_chat_area->setStretch(1, 0);
        }
        ui->stackedWidget->setCurrentWidget(ui->page_stats_emp);
    });
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });

    // STOCK
    connect(ui->btn_open_compare, &QPushButton::clicked, this, &MainWindow::showCompareDialog); // [RESTITUÉ]
    connect(ui->btn_open_calcul, &QPushButton::clicked, this, &MainWindow::showBesoinDialog);   // [RESTITUÉ]
    connect(ui->btn_stats_stock, &QPushButton::clicked, [=](){ calculerStatsStock(); ui->stackedWidget->setCurrentWidget(ui->page_stock_stats); });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_export_excel_stock, &QPushButton::clicked, [=](){ exporterCSV(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_print_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_close_stats_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });

    // CLIENTS
    connect(ui->btn_open_client_ia, &QPushButton::clicked, this, &MainWindow::showClientIaDialog);
    connect(ui->btn_open_fidelite, &QPushButton::clicked, this, &MainWindow::showFideliteDialog);
    connect(ui->btn_stats_client, &QPushButton::clicked, [=](){ calculerStatsClients(); ui->stackedWidget->setCurrentWidget(ui->page_stats_client); });
    connect(ui->btn_pdf_facture, &QPushButton::clicked, [=](){ exporterFactureClient(); });
    connect(ui->btn_export_excel_client, &QPushButton::clicked, [=](){ exporterCSV(ui->tableClients, "Clients"); });
    connect(ui->btn_print_client, &QPushButton::clicked, [=](){ exporterPDF(ui->tableClients, "Clients"); });
    connect(ui->btn_close_stats_client_dash, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_client_list); });

    // DEPOT
    connect(ui->btn_open_optimize, &QPushButton::clicked, this, &MainWindow::showOptimizeSpaceDialog);
    connect(ui->btn_open_ravit, &QPushButton::clicked, this, &MainWindow::showRavitaillementDialog);
    connect(ui->btn_stats_depot, &QPushButton::clicked, [=](){ calculerStatsDepots(); ui->stackedWidget->setCurrentWidget(ui->page_stats_depot); });
    connect(ui->btn_pdf_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_export_excel_depot, &QPushButton::clicked, [=](){ exporterCSV(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_print_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_close_stats_depot_dash, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });

    // --- CRUD ---
    // Planif CRUD
    auto prepNewPlanif = [=](){ modeModification=false; ui->le_fin_prevue->clear(); ui->stackedWidget->setCurrentWidget(ui->page_planif_form); };
    connect(ui->btn_to_add_planif, &QPushButton::clicked, prepNewPlanif);
    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        QString p=ui->cb_produit->currentText(), m=ui->cb_matiere->currentText();
        if(modeModification && indexModification >= 0 && indexModification < mesCommandes.size()) {
            mesCommandes[indexModification].produit = p;
            mesCommandes[indexModification].quantite = ui->sb_qte->value();
            mesCommandes[indexModification].matiere = m;
            mesCommandes[indexModification].dateDebut = ui->dt_lancement->date();
            mesCommandes[indexModification].dateFinEstimee = ui->le_fin_prevue->text();
        } else {
            mesCommandes.append({"OF-"+QString::number(100+mesCommandes.size()), p, ui->sb_qte->value(), m, ui->dt_lancement->date(), ui->le_fin_prevue->text(), "Planifié", "", 0});
        }
        ui->stackedWidget->setCurrentWidget(ui->page_planif_list); rafraichirListeCommandes(); configurerTimelineGantt();
    });
    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        if(ui->sb_qte->value() > 0) {
            // Logique simple : 2 jours de base + 1 jour tous les 100 articles
            int joursEstimes = 2 + (ui->sb_qte->value() / 100);

            // Calcul et affichage de la date de fin
            QDate fin = ui->dt_lancement->date().addDays(joursEstimes);
            ui->le_fin_prevue->setText(fin.toString("dd/MM/yyyy"));

            // Affichage de la durée dans le champ texte qui était vide
            ui->le_duree_totale->setText(QString::number(joursEstimes) + " Jours");
        }
    });
    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tablePlanif); if(idx < 0 || idx >= mesCommandes.size()) return;
        modeModification = true; indexModification = idx; CommandeInfo c = mesCommandes[idx];
        ui->cb_produit->setCurrentText(c.produit); ui->sb_qte->setValue(c.quantite); ui->cb_matiere->setCurrentText(c.matiere);
        ui->dt_lancement->setDate(c.dateDebut); ui->le_fin_prevue->setText(c.dateFinEstimee);
        ui->gb_form->setTitle("Modifier l'OF : " + c.id); ui->btn_valider_planif->setText("ENREGISTRER");
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    });
    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tablePlanif);
        if(idx >= 0 && idx < mesCommandes.size()) { mesCommandes.removeAt(idx); rafraichirListeCommandes(); configurerTimelineGantt(); }
    });

    // Prod CRUD
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){ modeModifProd=false; ui->stackedWidget->setCurrentWidget(ui->page_produit_form); });
    connect(ui->btn_valider_produit, &QPushButton::clicked, [=](){
        ProduitInfo p;
        p.ref = ui->le_ref_prod->text().trimmed();
        p.nom = ui->le_nom_prod->text().trimmed();
        p.coutMatiere = ui->sb_cout_prod->value();
        p.collection = ui->cb_coll_prod->currentText();
        p.cuir = ui->cb_cuir_prod->currentText();
        p.temps = ui->sb_temps_prod->value();
        if(p.ref.isEmpty() || p.nom.isEmpty()) { QMessageBox::warning(this, "Validation", "Référence et nom sont obligatoires."); return; }

        if(modeModifProd && indexModifProd >= 0 && indexModifProd < mesProduits.size()) mesProduits[indexModifProd] = p;
        else mesProduits.append(p);
        ui->stackedWidget->setCurrentWidget(ui->page_produit_list); rafraichirListeProduits();
    });
    connect(ui->btn_edit_produit, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableProduits);
        if(idx < 0 || idx >= mesProduits.size()) return;
        modeModifProd = true;
        indexModifProd = idx;
        const ProduitInfo &p = mesProduits[idx];
        ui->le_ref_prod->setText(p.ref);
        ui->le_nom_prod->setText(p.nom);
        ui->sb_cout_prod->setValue(p.coutMatiere);
        ui->cb_coll_prod->setCurrentText(p.collection);
        ui->cb_cuir_prod->setCurrentText(p.cuir);
        ui->sb_temps_prod->setValue(p.temps);
        ui->stackedWidget->setCurrentWidget(ui->page_produit_form);
    });
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableProduits);
        if(idx >= 0 && idx < mesProduits.size()) { mesProduits.removeAt(idx); rafraichirListeProduits(); }
    });

    // RH CRUD
    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){ modeModifEmp=false; ui->stackedWidget->setCurrentWidget(ui->page_employe_form); });
    connect(ui->btn_valider_emp, &QPushButton::clicked, [=](){
        EmployeInfo e;
        e.id = ui->le_id_emp->text().trimmed();
        e.nom = ui->le_nom_emp->text().trimmed();
        e.prenom = ui->le_prenom_emp->text().trimmed();
        e.poste = ui->cb_poste_emp->currentText();
        e.departement = ui->cb_dept_emp->currentText();
        e.dateEmbauche = ui->dt_embauche->date();
        e.salaire = ui->sb_salaire_emp->value();
        e.rfid = ui->le_rfid_emp->text().trimmed();
        if(e.id.isEmpty() || e.nom.isEmpty()) { QMessageBox::warning(this, "Validation", "ID et nom sont obligatoires."); return; }

        if(modeModifEmp && indexModifEmp >= 0 && indexModifEmp < mesEmployes.size()) mesEmployes[indexModifEmp] = e;
        else mesEmployes.append(e);
        ui->stackedWidget->setCurrentWidget(ui->page_employe_list); rafraichirListeEmployes();
    });
    connect(ui->btn_edit_emp, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableEmployes);
        if(idx < 0 || idx >= mesEmployes.size()) return;
        modeModifEmp = true;
        indexModifEmp = idx;
        const EmployeInfo &e = mesEmployes[idx];
        ui->le_id_emp->setText(e.id);
        ui->le_nom_emp->setText(e.nom);
        ui->le_prenom_emp->setText(e.prenom);
        ui->cb_poste_emp->setCurrentText(e.poste);
        ui->cb_dept_emp->setCurrentText(e.departement);
        ui->dt_embauche->setDate(e.dateEmbauche);
        ui->sb_salaire_emp->setValue(e.salaire);
        ui->le_rfid_emp->setText(e.rfid);
        ui->stackedWidget->setCurrentWidget(ui->page_employe_form);
    });
    connect(ui->btn_delete_emp, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableEmployes);
        if(idx >= 0 && idx < mesEmployes.size()) { mesEmployes.removeAt(idx); rafraichirListeEmployes(); }
    });
    connect(ui->btn_send_chat, &QPushButton::clicked, [=](){ reponseChatbot(); });

    // Stock CRUD
    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){ modeModifStock=false; ui->stackedWidget->setCurrentWidget(ui->page_stock_form); });
    connect(ui->btn_valider_stock, &QPushButton::clicked, [=](){
        MatiereInfo m;
        m.code = ui->le_code_mp->text().trimmed();
        m.categorie = ui->cb_cat_mp->currentText();
        m.etat = ui->cb_etat_mp->currentText();
        m.couleur = ui->le_coul_mp->text().trimmed();
        m.qualite = ui->cb_qual_mp->currentText();
        m.quantite = ui->sb_qte_mp->value();
        m.unite = ui->cb_unite_mp->currentText();
        m.zone = ui->cb_zone_mp->currentText();
        m.allee = ui->le_allee_mp->text().trimmed();
        m.typeStock = ui->cb_type_stock->currentText();
        m.dateRec = ui->dt_rec_mp->date();
        if(m.code.isEmpty()) { QMessageBox::warning(this, "Validation", "Le code MP est obligatoire."); return; }
        if(modeModifStock && indexModifStock >= 0 && indexModifStock < mesMatieres.size()) mesMatieres[indexModifStock] = m;
        else mesMatieres.append(m);
        ui->stackedWidget->setCurrentWidget(ui->page_stock_list); rafraichirListeMatieres();
    });
    connect(ui->btn_edit_stock, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableStock); if(idx < 0 || idx >= mesMatieres.size()) return;
        modeModifStock=true; indexModifStock=idx; MatiereInfo m = mesMatieres[idx];
        ui->le_code_mp->setText(m.code); ui->cb_cat_mp->setCurrentText(m.categorie);
        ui->cb_etat_mp->setCurrentText(m.etat); ui->le_coul_mp->setText(m.couleur);
        ui->cb_qual_mp->setCurrentText(m.qualite); ui->sb_qte_mp->setValue(m.quantite);
        ui->cb_unite_mp->setCurrentText(m.unite); ui->cb_zone_mp->setCurrentText(m.zone);
        ui->le_allee_mp->setText(m.allee); ui->cb_type_stock->setCurrentText(m.typeStock);
        ui->dt_rec_mp->setDate(m.dateRec);
        ui->stackedWidget->setCurrentWidget(ui->page_stock_form);
    });
    connect(ui->btn_delete_stock, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableStock);
        if(idx >= 0 && idx < mesMatieres.size()) { mesMatieres.removeAt(idx); rafraichirListeMatieres(); }
    });

    // Clients CRUD
    connect(ui->btn_add_client, &QPushButton::clicked, [=](){
        modeModifClient = false; indexModifClient = -1;
        ui->le_id_client->clear(); ui->le_nom_client->clear(); ui->le_tel_client->clear();
        ui->le_adresse_client->clear(); ui->le_email_client->clear(); ui->sb_points_client->setValue(0);
        ui->stackedWidget->setCurrentWidget(ui->page_client_form);
    });
    connect(ui->btn_valider_client, &QPushButton::clicked, [=](){
        ClientInfo c;
        c.id = ui->le_id_client->text().trimmed();
        c.nom = ui->le_nom_client->text().trimmed();
        c.telephone = ui->le_tel_client->text().trimmed();
        c.adresse = ui->le_adresse_client->text().trimmed();
        c.email = ui->le_email_client->text().trimmed();
        c.pointsFidelite = ui->sb_points_client->value();

        if(c.id.isEmpty() || c.nom.isEmpty()) {
            QMessageBox::warning(this, "Validation", "ID client et nom sont obligatoires.");
            return;
        }

        if(modeModifClient && indexModifClient >= 0 && indexModifClient < mesClients.size()) {
            mesClients[indexModifClient] = c;
        } else {
            mesClients.append(c);
        }

        ui->stackedWidget->setCurrentWidget(ui->page_client_list);
        rafraichirListeClients();
    });
    connect(ui->btn_edit_client, &QPushButton::clicked, [=](){
        int r = ui->tableClients->currentRow();
        if(r < 0) return;
        QTableWidgetItem *it = ui->tableClients->item(r, 0);
        if(!it) return;
        int idx = it->data(Qt::UserRole).toInt();
        if(idx < 0 || idx >= mesClients.size()) return;
        modeModifClient = true; indexModifClient = idx;

        const ClientInfo &c = mesClients[idx];
        ui->le_id_client->setText(c.id);
        ui->le_nom_client->setText(c.nom);
        ui->le_tel_client->setText(c.telephone);
        ui->le_adresse_client->setText(c.adresse);
        ui->le_email_client->setText(c.email);
        ui->sb_points_client->setValue(c.pointsFidelite);
        ui->stackedWidget->setCurrentWidget(ui->page_client_form);
    });
    connect(ui->btn_delete_client, &QPushButton::clicked, [=](){
        int r = ui->tableClients->currentRow();
        if(r < 0) return;
        QTableWidgetItem *it = ui->tableClients->item(r, 0);
        if(!it) return;
        int idx = it->data(Qt::UserRole).toInt();
        if(idx < 0 || idx >= mesClients.size()) return;
        mesClients.removeAt(idx);
        rafraichirListeClients();
    });
    connect(ui->btn_search_client, &QPushButton::clicked, [=](){ rafraichirListeClients(); });
    connect(ui->btn_sort_fidelite, &QPushButton::clicked, [=](){
        std::sort(mesClients.begin(), mesClients.end(), [](const ClientInfo &a, const ClientInfo &b){
            return a.pointsFidelite > b.pointsFidelite;
        });
        rafraichirListeClients();
    });

    // Dépôt CRUD
    connect(ui->btn_add_depot, &QPushButton::clicked, [=](){
        modeModifDepot = false; indexModifDepot = -1;
        ui->le_id_depot->clear(); ui->le_emplacement_depot->clear(); ui->le_etagere_depot->clear();
        ui->sb_capacite_depot->setValue(0.0); ui->sb_qte_depot->setValue(0.0);
        ui->cb_type_depot->setCurrentIndex(0);
        ui->stackedWidget->setCurrentWidget(ui->page_depot_form);
    });
    connect(ui->btn_valider_depot, &QPushButton::clicked, [=](){
        DepotInfo d;
        d.id = ui->le_id_depot->text().trimmed();
        d.emplacement = ui->le_emplacement_depot->text().trimmed();
        d.etagere = ui->le_etagere_depot->text().trimmed();
        d.capaciteMax = ui->sb_capacite_depot->value();
        d.quantiteActuelle = ui->sb_qte_depot->value();
        d.typeStockage = ui->cb_type_depot->currentText();

        if(d.id.isEmpty() || d.emplacement.isEmpty()) {
            QMessageBox::warning(this, "Validation", "ID et emplacement sont obligatoires.");
            return;
        }
        if(d.capaciteMax > 0 && d.quantiteActuelle > d.capaciteMax) d.quantiteActuelle = d.capaciteMax;
        if(d.quantiteActuelle < 0) d.quantiteActuelle = 0;

        if(modeModifDepot && indexModifDepot >= 0 && indexModifDepot < mesDepots.size()) {
            mesDepots[indexModifDepot] = d;
        } else {
            mesDepots.append(d);
        }

        ui->stackedWidget->setCurrentWidget(ui->page_depot_list);
        rafraichirListeDepots();
    });
    connect(ui->btn_edit_depot, &QPushButton::clicked, [=](){
        int r = ui->tableDepot->currentRow();
        if(r < 0) return;
        QTableWidgetItem *it = ui->tableDepot->item(r, 0);
        if(!it) return;
        int idx = it->data(Qt::UserRole).toInt();
        if(idx < 0 || idx >= mesDepots.size()) return;
        modeModifDepot = true; indexModifDepot = idx;

        const DepotInfo &d = mesDepots[idx];
        ui->le_id_depot->setText(d.id);
        ui->le_emplacement_depot->setText(d.emplacement);
        ui->le_etagere_depot->setText(d.etagere);
        ui->sb_capacite_depot->setValue(d.capaciteMax);
        ui->sb_qte_depot->setValue(d.quantiteActuelle);
        ui->cb_type_depot->setCurrentText(d.typeStockage);
        ui->stackedWidget->setCurrentWidget(ui->page_depot_form);
    });
    connect(ui->btn_delete_depot, &QPushButton::clicked, [=](){
        int r = ui->tableDepot->currentRow();
        if(r < 0) return;
        QTableWidgetItem *it = ui->tableDepot->item(r, 0);
        if(!it) return;
        int idx = it->data(Qt::UserRole).toInt();
        if(idx < 0 || idx >= mesDepots.size()) return;
        mesDepots.removeAt(idx);
        rafraichirListeDepots();
    });
    connect(ui->btn_search_depot, &QPushButton::clicked, [=](){ rafraichirListeDepots(); });
    connect(ui->btn_sort_remplissage, &QPushButton::clicked, [=](){
        std::sort(mesDepots.begin(), mesDepots.end(), [](const DepotInfo &a, const DepotInfo &b){
            double fa = (a.capaciteMax <= 0) ? 0.0 : (a.quantiteActuelle / a.capaciteMax);
            double fb = (b.capaciteMax <= 0) ? 0.0 : (b.quantiteActuelle / b.capaciteMax);
            return fa > fb;
        });
        rafraichirListeDepots();
    });

    // Activer/désactiver Modifier/Supprimer selon sélection
    auto bindEnableOnSelection = [](QTableWidget *table, QPushButton *btnEdit, QPushButton *btnDelete) {
        if(!table) return;
        auto update = [=]() {
            const bool has = table->currentRow() >= 0;
            if(btnEdit) btnEdit->setEnabled(has);
            if(btnDelete) btnDelete->setEnabled(has);
        };
        QObject::connect(table, &QTableWidget::itemSelectionChanged, table, update);
        update();
    };

    bindEnableOnSelection(ui->tablePlanif, ui->btn_modifier_planif, ui->btn_supprimer_planif);
    bindEnableOnSelection(ui->tableProduits, ui->btn_edit_produit, ui->btn_delete_produit);
    bindEnableOnSelection(ui->tableEmployes, ui->btn_edit_emp, ui->btn_delete_emp);
    bindEnableOnSelection(ui->tableStock, ui->btn_edit_stock, ui->btn_delete_stock);
    bindEnableOnSelection(ui->tableClients, ui->btn_edit_client, ui->btn_delete_client);
    bindEnableOnSelection(ui->tableDepot, ui->btn_edit_depot, ui->btn_delete_depot);

    // Fabrication
    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int r){
        indexCommandeSelectionnee=r;
        ui->lbl_sel_cmd->setText("OF: " + mesCommandes[r].id + " - " + mesCommandes[r].produit);
        ui->lbl_resultat_delta->setText("..."); ui->lbl_resultat_delta->setStyleSheet("color:gray; border:none;");
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &t){
        double v=2.0; if(t=="Assemblage")v=5.0; else if(t=="Couture")v=4.0; else if(t=="Finition")v=1.0;
        ui->lbl_temps_prevu->setText(QString::number(v)+" h");
    });
    connect(ui->btn_valider_etape, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee<0) return;
        double r=ui->sb_temps_reel_input->value(), p=ui->lbl_temps_prevu->text().split(" ")[0].toDouble();
        if(r>p) { ui->lbl_resultat_delta->setText("RETARD"); ui->lbl_resultat_delta->setStyleSheet("background-color:#bf360c;color:white;padding:5px;border-radius:4px;"); mesCommandes[indexCommandeSelectionnee].etatEtape=2; }
        else { ui->lbl_resultat_delta->setText("OK"); ui->lbl_resultat_delta->setStyleSheet("background-color:#2e7d32;color:white;padding:5px;border-radius:4px;"); mesCommandes[indexCommandeSelectionnee].etatEtape=1; }
        configurerTimelineGantt();
    });
    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){ configurerTimelineGantt(); });
    connect(ui->btn_saisie_detail, &QPushButton::clicked, [=](){ QMessageBox::information(this, "Info", "Module Détail"); });
}

MainWindow::~MainWindow() { delete ui; }

// =================================================================
// === POP-UPS (STOCK) - PRESERVES DU CODE FOURNI ===
// =================================================================

namespace {

static QString qssInput()
{
    return "padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px; background: white;";
}

static QString qssTitle(const QString &bg)
{
    return QString("font-size: 18px; font-weight: bold; color: #3e2723; padding: 12px; background-color: %1; border-radius: 10px;")
    .arg(bg);
}

static QString qssCard()
{
    return "padding: 12px; background: white; border: 1px solid #d7ccc8; border-radius: 10px; font-size: 14px;";
}

static QString qssBtnPrimaryTeal()
{
    return "background-color: #00838f; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnBlue()
{
    return "background-color: #2196f3; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnOrange()
{
    return "background-color: #f9a825; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnGreenClose()
{
    return "background-color: #4caf50; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static QString qssBtnPurple()
{
    return "background-color: #7b1fa2; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;";
}

static void applyDialogBase(QDialog &dialog, int w, int h)
{
    dialog.setMinimumSize(w, h);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");
}

static void styleTableHeaderTeal(QTableWidget *table)
{
    if(!table) return;
    table->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #dcdcdc; border-radius: 10px; gridline-color: #ecf0f1; }"
        "QHeaderView::section { background-color: #00838f; color: white; padding: 10px; font-weight: bold; border: none; }"
        "QTableWidget::item { padding: 6px; }"
        );
}

class TableHoverLeaveFilter : public QObject
{
public:
    TableHoverLeaveFilter(int *hoverRow, int *hoverCol, QTableWidget *table, QObject *parent = nullptr)
        : QObject(parent), hoverRow_(hoverRow), hoverCol_(hoverCol), table_(table)
    {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        Q_UNUSED(obj);
        if(!table_ || !hoverRow_ || !hoverCol_) return false;
        if(event && event->type() == QEvent::Leave) {
            *hoverRow_ = -1;
            *hoverCol_ = -1;
            table_->viewport()->update();
        }
        return false;
    }

private:
    int *hoverRow_;
    int *hoverCol_;
    QTableWidget *table_;
};

class StarGlowDelegate : public QStyledItemDelegate
{
public:
    StarGlowDelegate(int noteColumn, const int *hoverRow, const int *hoverCol, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), noteColumn_(noteColumn), hoverRow_(hoverRow), hoverCol_(hoverCol)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if(index.column() != noteColumn_) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        QString stars = index.data(Qt::UserRole + 1).toString();
        if(stars.isEmpty()) stars = opt.text;
        opt.text.clear();
        QStyledItemDelegate::paint(painter, opt, index);

        const bool hovered = hoverRow_ && hoverCol_ && index.row() == *hoverRow_ && index.column() == *hoverCol_;
        const QColor base(0x3e, 0x27, 0x23);
        const QColor gold(0xff, 0xd5, 0x4f);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QFont font = opt.font;
        font.setBold(true);
        font.setPointSizeF(font.pointSizeF() + 2.0);
        painter->setFont(font);

        const QRect r = opt.rect;

        if(hovered) {
            // Stronger glow (fake blur) around the text
            const QVector<QPoint> offsets = {
                {-3, 0}, {3, 0}, {0, -3}, {0, 3},
                {-3, -2}, {-3, 2}, {3, -2}, {3, 2},
                {-2, -3}, {-2, 3}, {2, -3}, {2, 3},
                {-2, 0}, {2, 0}, {0, -2}, {0, 2},
                {-1, 0}, {1, 0}, {0, -1}, {0, 1}
            };
            QColor glow = QColor(255, 215, 0);
            glow.setAlpha(110);
            painter->setPen(glow);
            for(const QPoint &off : offsets) {
                painter->drawText(r.translated(off), Qt::AlignCenter, stars);
            }

            // Golden gradient (shine)
            QLinearGradient grad(r.left(), r.top(), r.right(), r.top());
            grad.setColorAt(0.00, QColor(0xb8, 0x86, 0x0b));   // dark gold
            grad.setColorAt(0.35, QColor(0xff, 0xf2, 0xb0));   // bright highlight
            grad.setColorAt(0.55, QColor(0xff, 0xd5, 0x4f));   // gold
            grad.setColorAt(1.00, QColor(0xb8, 0x86, 0x0b));
            painter->setPen(QPen(QBrush(grad), 0));
            painter->drawText(r, Qt::AlignCenter, stars);

            // Subtle white specular highlight
            QColor spec(255, 255, 255);
            spec.setAlpha(90);
            painter->setPen(spec);
            painter->drawText(r.translated(-1, -1), Qt::AlignCenter, stars);
        } else {
            painter->setPen(base);
            painter->drawText(r, Qt::AlignCenter, stars);
        }

        painter->restore();
    }

private:
    int noteColumn_;
    const int *hoverRow_;
    const int *hoverCol_;
};

}

void MainWindow::showCompareDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Comparateur de Fournisseurs");
    dialog.setMinimumSize(860, 620);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    QLabel *title = new QLabel("Comparateur Intelligent de Fournisseurs");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #3e2723; padding: 10px; background-color: #e1f5fe; border-radius: 10px;");
    mainLayout->addWidget(title);

    QHBoxLayout *selectLayout = new QHBoxLayout();
    QLabel *lblSelect = new QLabel("Matière recherchée :");
    lblSelect->setStyleSheet("font-weight: bold; font-size: 14px;");

    QComboBox *cbMatiere = new QComboBox();
    cbMatiere->addItems({"Cuir Vachette", "Cuir Agneau", "Teinture Noire", "Produit de Finition"});
    cbMatiere->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 14px;");

    QPushButton *btnCompare = new QPushButton("Comparer");
    btnCompare->setStyleSheet("background-color: #00838f; color: white; padding: 8px 20px; border-radius: 15px; font-weight: bold;");

    selectLayout->addWidget(lblSelect);
    selectLayout->addWidget(cbMatiere);
    selectLayout->addWidget(btnCompare);
    mainLayout->addLayout(selectLayout);

    QLabel *lblResults = new QLabel("Résultats de la comparaison :");
    lblResults->setStyleSheet("font-weight: bold; font-size: 14px; padding: 6px;");
    mainLayout->addWidget(lblResults);

    QTableWidget *tableCompare = new QTableWidget(3, 6);
    tableCompare->setHorizontalHeaderLabels({"Fournisseur", "Prix/M²", "Qualité", "Délai", "Stock Dispo", "Note"});
    tableCompare->horizontalHeader()->setStretchLastSection(true);
    tableCompare->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableCompare->setStyleSheet("QHeaderView::section { background-color: #00838f; color: white; padding: 10px; font-weight: bold; }");

    tableCompare->setItem(0, 0, new QTableWidgetItem("TanLeather SA"));
    tableCompare->setItem(0, 1, new QTableWidgetItem("45 DT"));
    tableCompare->setItem(0, 2, new QTableWidgetItem("A (Premium)"));
    tableCompare->setItem(0, 3, new QTableWidgetItem("5 jours"));
    tableCompare->setItem(0, 4, new QTableWidgetItem("500 M²"));
    tableCompare->setItem(0, 5, new QTableWidgetItem("★★★★★"));

    tableCompare->setItem(1, 0, new QTableWidgetItem("Cuir Elite"));
    tableCompare->setItem(1, 1, new QTableWidgetItem("38 DT"));
    tableCompare->setItem(1, 2, new QTableWidgetItem("B (Standard)"));
    tableCompare->setItem(1, 3, new QTableWidgetItem("7 jours"));
    tableCompare->setItem(1, 4, new QTableWidgetItem("300 M²"));
    tableCompare->setItem(1, 5, new QTableWidgetItem("★★★★"));

    tableCompare->setItem(2, 0, new QTableWidgetItem("MegaCuir"));
    tableCompare->setItem(2, 1, new QTableWidgetItem("32 DT"));
    tableCompare->setItem(2, 2, new QTableWidgetItem("C (Économique)"));
    tableCompare->setItem(2, 3, new QTableWidgetItem("10 jours"));
    tableCompare->setItem(2, 4, new QTableWidgetItem("1000 M²"));
    tableCompare->setItem(2, 5, new QTableWidgetItem("★★★"));

    // Store stars in hidden role and clear visible text (delegate will paint them)
    for(int r = 0; r < tableCompare->rowCount(); ++r) {
        QTableWidgetItem *it = tableCompare->item(r, 5);
        if(!it) continue;
        const QString stars = it->text();
        it->setData(Qt::UserRole + 1, stars);
        it->setText(QString());
        it->setTextAlignment(Qt::AlignCenter);
        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    }

    // Hover glow effect on stars (NOTE column)
    int hoverRow = -1;
    int hoverCol = -1;
    tableCompare->setMouseTracking(true);
    tableCompare->viewport()->setMouseTracking(true);
    tableCompare->setItemDelegateForColumn(5, new StarGlowDelegate(5, &hoverRow, &hoverCol, tableCompare));
    connect(tableCompare, &QTableWidget::cellEntered, &dialog, [&](int r, int c){
        hoverRow = r;
        hoverCol = c;
        tableCompare->viewport()->update();
    });
    tableCompare->viewport()->installEventFilter(new TableHoverLeaveFilter(&hoverRow, &hoverCol, tableCompare, tableCompare->viewport()));

    for (int i = 0; i < 6; ++i) {
        tableCompare->item(0, i)->setBackground(QColor(200, 255, 200));
        tableCompare->item(1, i)->setBackground(QColor(255, 250, 205));
        tableCompare->item(2, i)->setBackground(QColor(255, 220, 220));
    }

    mainLayout->addWidget(tableCompare);

    QLabel *lblRecommendation = new QLabel("<b>Recommandation :</b> TanLeather SA (meilleur rapport Qualité/Prix/Fiabilité).");
    lblRecommendation->setStyleSheet("background-color: #c8e6c9; padding: 12px; border-radius: 10px; font-size: 14px;");
    lblRecommendation->setWordWrap(true);
    mainLayout->addWidget(lblRecommendation);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnContact = new QPushButton("Contacter Fournisseur");
    btnContact->setStyleSheet("background-color: #2196f3; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet("background-color: #4caf50; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");

    buttonLayout->addWidget(btnContact);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnClose);
    mainLayout->addLayout(buttonLayout);

    connect(btnCompare, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Comparaison", "Comparaison effectuée pour : " + cbMatiere->currentText());
    });
    connect(btnContact, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Contact", "Email envoyé (démo) !");
    });
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::showBesoinDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Calculer Besoin de Matières");
    dialog.setMinimumSize(820, 680);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    QLabel *title = new QLabel("Calculateur de Besoins Matières");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #3e2723; padding: 12px; background-color: #fff3e0; border-radius: 10px;");
    mainLayout->addWidget(title);

    QGroupBox *gbCalc = new QGroupBox("Paramètres de Production");
    gbCalc->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; padding: 15px; border: 2px solid #f9a825; border-radius: 10px; background-color: white; }");
    QFormLayout *formLayout = new QFormLayout(gbCalc);
    formLayout->setSpacing(12);

    QComboBox *cbProduit = new QComboBox();
    cbProduit->addItems({"Sac Voyage Cuir", "Portefeuille Luxe", "Ceinture Homme", "Sac à Main Femme"});
    cbProduit->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QSpinBox *sbQuantite = new QSpinBox();
    sbQuantite->setRange(1, 10000);
    sbQuantite->setValue(50);
    sbQuantite->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QComboBox *cbMatiere = new QComboBox();
    cbMatiere->addItems({"Cuir Vachette", "Cuir Agneau", "Cuir Premium", "Cuir Synthétique"});
    cbMatiere->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QDoubleSpinBox *sbPerte = new QDoubleSpinBox();
    sbPerte->setRange(0, 50);
    sbPerte->setValue(15);
    sbPerte->setDecimals(1);
    sbPerte->setSuffix(" %");
    sbPerte->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    formLayout->addRow("Type de Produit :", cbProduit);
    formLayout->addRow("Quantité à produire :", sbQuantite);
    formLayout->addRow("Type de Matière :", cbMatiere);
    formLayout->addRow("Marge de Perte :", sbPerte);

    mainLayout->addWidget(gbCalc);

    QPushButton *btnCalculer = new QPushButton("Calculer les Besoins");
    btnCalculer->setStyleSheet("background-color: #f9a825; color: white; padding: 12px; border-radius: 15px; font-weight: bold; font-size: 15px;");
    mainLayout->addWidget(btnCalculer);

    QGroupBox *gbResults = new QGroupBox("Résultats du Calcul");
    gbResults->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; padding: 15px; border: 2px solid #4caf50; border-radius: 10px; background-color: #e8f5e9; }");
    QVBoxLayout *resultsLayout = new QVBoxLayout(gbResults);
    resultsLayout->setSpacing(8);

    QLabel *lblResult1 = new QLabel("Surface nécessaire par unité : <b>0.8 M²</b>");
    QLabel *lblResult2 = new QLabel("Surface totale (sans perte) : <b>40 M²</b>");
    QLabel *lblResult3 = new QLabel("Surface avec marge de perte (15%) : <b>46 M²</b>");
    QLabel *lblResult4 = new QLabel("Stock disponible : <b>100 M²</b>");
    QLabel *lblResult5 = new QLabel("<b>Statut : SUFFISANT</b>");
    lblResult5->setAlignment(Qt::AlignCenter);
    lblResult5->setWordWrap(true);

    resultsLayout->addWidget(lblResult1);
    resultsLayout->addWidget(lblResult2);
    resultsLayout->addWidget(lblResult3);
    resultsLayout->addWidget(lblResult4);
    resultsLayout->addWidget(lblResult5);

    mainLayout->addWidget(gbResults);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    QPushButton *btnReserver = new QPushButton("Réserver Matière");
    btnReserver->setStyleSheet("background-color: #1e88e5; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnCommander = new QPushButton("Commander Plus");
    btnCommander->setStyleSheet("background-color: #d84315; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet("background-color: #43a047; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");

    actionLayout->addWidget(btnReserver);
    actionLayout->addWidget(btnCommander);
    actionLayout->addStretch();
    actionLayout->addWidget(btnClose);
    mainLayout->addLayout(actionLayout);

    connect(btnCalculer, &QPushButton::clicked, &dialog, [&]() {
        const int qte = sbQuantite->value();
        const double pertePercent = sbPerte->value();
        const double surfaceParUnite = 0.8;

        const double surfaceTotal = qte * surfaceParUnite;
        const double surfaceAvecPerte = surfaceTotal * (1.0 + pertePercent / 100.0);

        const double stockDispo = 100.0; // Démo
        const bool ok = (stockDispo >= surfaceAvecPerte);

        lblResult2->setText(QString("Surface totale (sans perte) : <b>%1 M²</b>").arg(surfaceTotal, 0, 'f', 1));
        lblResult3->setText(QString("Surface avec marge de perte (%1%) : <b>%2 M²</b>")
                                .arg(pertePercent, 0, 'f', 1)
                                .arg(surfaceAvecPerte, 0, 'f', 1));
        lblResult4->setText(QString("Stock disponible : <b>%1 M²</b>").arg(stockDispo, 0, 'f', 1));

        if (ok) {
            const double reste = stockDispo - surfaceAvecPerte;
            lblResult5->setText(QString("<b>Statut : SUFFISANT</b> (reste %1 M²)").arg(reste, 0, 'f', 1));
            lblResult5->setStyleSheet("padding: 10px; background-color: #c8e6c9; border-radius: 10px; color: #1b5e20;");
        } else {
            const double manque = surfaceAvecPerte - stockDispo;
            lblResult5->setText(QString("<b>Statut : INSUFFISANT</b> (manque %1 M²)").arg(manque, 0, 'f', 1));
            lblResult5->setStyleSheet("padding: 10px; background-color: #ffcdd2; border-radius: 10px; color: #b71c1c;");
        }
    });

    connect(btnReserver, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Réservation", "Réservation effectuée (démo).");
    });
    connect(btnCommander, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Commande", "Redirection vers le module Commande (démo).");
    });
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

// =================================================================
// === POP-UPS (NOUVEAUX: PRODUITS & RH) ===
// =================================================================

// 3. SIMULATEUR RENTABILITÉ (Produits)
void MainWindow::showProdSimDialog() {
    QDialog d(this);
    d.setWindowTitle("Simulateur Rentabilité");
    applyDialogBase(d, 620, 420);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Simulateur Rentabilité");
    ti->setAlignment(Qt::AlignCenter);
    ti->setStyleSheet(qssTitle("#e0f2f1"));
    l->addWidget(ti);

    QFormLayout *f = new QFormLayout();
    QDoubleSpinBox *sbCout = new QDoubleSpinBox(); sbCout->setRange(0,10000); sbCout->setValue(50);
    QDoubleSpinBox *sbPrix = new QDoubleSpinBox(); sbPrix->setRange(0,10000); sbPrix->setValue(120);
    sbCout->setStyleSheet(qssInput());
    sbPrix->setStyleSheet(qssInput());
    f->addRow("Coût de Production (€):", sbCout);
    f->addRow("Prix de Vente (€):", sbPrix);
    l->addLayout(f);

    QLabel *lblRes = new QLabel("Marge Nette : <b>-</b>");
    lblRes->setStyleSheet(qssCard());
    lblRes->setWordWrap(true);

    QPushButton *btnCalc = new QPushButton("Calculer");
    btnCalc->setStyleSheet(qssBtnPrimaryTeal());
    connect(btnCalc, &QPushButton::clicked, [=](){
        double m = sbPrix->value() - sbCout->value();
        lblRes->setText("Marge : <b>" + QString::number(m) + " €</b>");
    });

    QHBoxLayout *actions = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(qssBtnGreenClose());
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    actions->addWidget(btnCalc);
    actions->addStretch(1);
    actions->addWidget(btnClose);

    l->addLayout(actions);
    l->addWidget(lblRes);
    d.exec();
}

// 3.b COÛT DE PRODUIT (Produits)
void MainWindow::showProduitCoutDialog()
{
    QDialog d(this);
    d.setWindowTitle("Coût de Produit");
    applyDialogBase(d, 720, 520);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Coût de Produit — Calcul avancé");
    ti->setAlignment(Qt::AlignCenter);
    ti->setStyleSheet(qssTitle("#e0f2f1"));
    l->addWidget(ti);

    // Try to preselect from table if possible
    int preselect = -1;
    if(ui->tableProduits) {
        const int row = ui->tableProduits->currentRow();
        if(row >= 0) {
            QTableWidgetItem *it = ui->tableProduits->item(row, 0);
            if(it) preselect = it->data(Qt::UserRole).toInt();
        }
    }

    QFormLayout *f = new QFormLayout();
    QComboBox *cbProd = new QComboBox();
    cbProd->setStyleSheet(qssInput());
    for(int i = 0; i < mesProduits.size(); ++i) {
        const auto &p = mesProduits[i];
        cbProd->addItem(p.ref + " — " + p.nom, i);
    }
    if(preselect >= 0 && preselect < mesProduits.size()) {
        const int idx = cbProd->findData(preselect);
        if(idx >= 0) cbProd->setCurrentIndex(idx);
    }

    QDoubleSpinBox *sbTauxHoraire = new QDoubleSpinBox();
    sbTauxHoraire->setRange(0, 1000);
    sbTauxHoraire->setValue(35);
    sbTauxHoraire->setSuffix(" €/h");
    sbTauxHoraire->setStyleSheet(qssInput());

    QDoubleSpinBox *sbFrais = new QDoubleSpinBox();
    sbFrais->setRange(0, 200);
    sbFrais->setValue(12);
    sbFrais->setSuffix(" %");
    sbFrais->setStyleSheet(qssInput());

    QDoubleSpinBox *sbMarge = new QDoubleSpinBox();
    sbMarge->setRange(0, 300);
    sbMarge->setValue(25);
    sbMarge->setSuffix(" %");
    sbMarge->setStyleSheet(qssInput());

    f->addRow("Produit :", cbProd);
    f->addRow("Taux horaire (MO) :", sbTauxHoraire);
    f->addRow("Frais indirects :", sbFrais);
    f->addRow("Marge cible :", sbMarge);
    l->addLayout(f);

    QLabel *out = new QLabel("Résultat : <b>-</b>");
    out->setStyleSheet(qssCard());
    out->setWordWrap(true);
    l->addWidget(out);

    auto recompute = [=]() {
        const int pIdx = cbProd->currentData().toInt();
        if(pIdx < 0 || pIdx >= mesProduits.size()) {
            out->setText("Résultat : <b>Aucun produit</b>");
            return;
        }
        const auto &p = mesProduits[pIdx];
        const double mat = p.coutMatiere;
        const double mo = static_cast<double>(p.temps) * sbTauxHoraire->value();
        const double base = mat + mo;
        const double frais = base * (sbFrais->value() / 100.0);
        const double coutRevient = base + frais;
        const double prixCible = coutRevient * (1.0 + sbMarge->value() / 100.0);

        out->setText(
            "<b>Produit :</b> " + (p.ref + " — " + p.nom).toHtmlEscaped() + "<br/>"
                                                                            "<b>Matière :</b> " + QLocale(QLocale::French).toString(mat, 'f', 2) + " €<br/>"
                                                               "<b>Main-d'œuvre :</b> " + QLocale(QLocale::French).toString(mo, 'f', 2) + " € (" +
            QString::number(p.temps) + " h × " + QLocale(QLocale::French).toString(sbTauxHoraire->value(), 'f', 0) + " €/h)<br/>"
                                                                                                                     "<b>Frais indirects :</b> " + QLocale(QLocale::French).toString(frais, 'f', 2) + " €<br/>"
                                                                 "<hr style='border:0; border-top:1px solid #eee5dd; margin:8px 0;'/>"
                                                                 "<b>Coût de revient :</b> " + QLocale(QLocale::French).toString(coutRevient, 'f', 2) + " €<br/>"
                                                                       "<b>Prix de vente cible :</b> <span style='color:#2c1a16; font-size:14px;'><b>" +
            QLocale(QLocale::French).toString(prixCible, 'f', 2) + " €</b></span>");
    };

    QPushButton *btnCalc = new QPushButton("Calculer");
    btnCalc->setStyleSheet(qssBtnPrimaryTeal());
    QObject::connect(btnCalc, &QPushButton::clicked, &d, recompute);
    QObject::connect(cbProd, &QComboBox::currentIndexChanged, &d, recompute);
    QObject::connect(sbTauxHoraire, qOverload<double>(&QDoubleSpinBox::valueChanged), &d, recompute);
    QObject::connect(sbFrais, qOverload<double>(&QDoubleSpinBox::valueChanged), &d, recompute);
    QObject::connect(sbMarge, qOverload<double>(&QDoubleSpinBox::valueChanged), &d, recompute);

    QHBoxLayout *actions = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(qssBtnGreenClose());
    QObject::connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    actions->addWidget(btnCalc);
    actions->addStretch(1);
    actions->addWidget(btnClose);
    l->addLayout(actions);

    recompute();
    d.exec();
}

// 3.c HISTORIQUE DE MODE (Produits)
void MainWindow::showHistoriqueModeDialog()
{
    QDialog d(this);
    d.setWindowTitle("Historique de Mode");
    applyDialogBase(d, 760, 520);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Historique de Mode — Collections & tendances");
    ti->setAlignment(Qt::AlignCenter);
    ti->setStyleSheet(qssTitle("#f3e5f5"));
    l->addWidget(ti);

    QHBoxLayout *filters = new QHBoxLayout();
    QComboBox *cbVue = new QComboBox();
    cbVue->addItems({"Vue par collection", "Vue par cuir"});
    cbVue->setStyleSheet(qssInput());
    QLineEdit *leFiltre = new QLineEdit();
    leFiltre->setPlaceholderText("Filtrer (ex: Hiver, Été, Croco...)");
    leFiltre->setStyleSheet(qssInput());
    filters->addWidget(cbVue);
    filters->addWidget(leFiltre, 1);
    l->addLayout(filters);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Catégorie", "Nb produits", "Détail"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    l->addWidget(table, 1);

    QLabel *hint = new QLabel("Astuce : utilisez ce tableau comme une lecture “métier” des tendances internes (données démo)." );
    hint->setStyleSheet(qssCard());
    hint->setWordWrap(true);
    l->addWidget(hint);

    auto refresh = [=]() {
        const QString filter = leFiltre->text().trimmed().toLower();
        const bool byCollection = (cbVue->currentIndex() == 0);
        QMap<QString, int> counts;
        QMap<QString, QStringList> details;

        for(const auto &p : mesProduits) {
            const QString key = byCollection ? p.collection : p.cuir;
            const QString keyLow = key.toLower();
            const QString hay = (p.ref + " " + p.nom + " " + p.collection + " " + p.cuir).toLower();
            if(!filter.isEmpty() && !hay.contains(filter) && !keyLow.contains(filter)) continue;
            counts[key] += 1;
            details[key].append(p.ref);
        }

        QList<QString> keys = counts.keys();
        std::sort(keys.begin(), keys.end(), [&](const QString &a, const QString &b){
            if(counts[a] != counts[b]) return counts[a] > counts[b];
            return a.localeAwareCompare(b) < 0;
        });

        table->setRowCount(keys.size());
        for(int r = 0; r < keys.size(); ++r) {
            const QString k = keys[r];
            table->setItem(r, 0, new QTableWidgetItem(k));
            table->setItem(r, 1, new QTableWidgetItem(QString::number(counts[k])));
            table->setItem(r, 2, new QTableWidgetItem(details[k].join(", ")));
        }
        table->resizeColumnsToContents();
    };

    QObject::connect(cbVue, &QComboBox::currentIndexChanged, &d, refresh);
    QObject::connect(leFiltre, &QLineEdit::textChanged, &d, refresh);

    QHBoxLayout *actions = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(qssBtnGreenClose());
    QObject::connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    actions->addStretch(1);
    actions->addWidget(btnClose);
    l->addLayout(actions);

    refresh();
    d.exec();
}

// 4. ÉVALUATION COMPÉTENCES (RH)
void MainWindow::showEmpEvalDialog() {
    QDialog d(this);
    d.setWindowTitle("Évaluation Compétences");
    applyDialogBase(d, 620, 420);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Évaluation Compétences");
    ti->setAlignment(Qt::AlignCenter);
    ti->setStyleSheet(qssTitle("#fff3e0"));
    l->addWidget(ti);

    QFormLayout *f = new QFormLayout();
    QComboBox *cb = new QComboBox(); for(const auto &e : mesEmployes) cb->addItem(e.nom);
    QSpinBox *sb = new QSpinBox(); sb->setRange(0,20); sb->setValue(15);
    cb->setStyleSheet(qssInput());
    sb->setStyleSheet(qssInput());
    f->addRow("Employé :", cb); f->addRow("Note Globale (/20) :", sb);
    l->addLayout(f);

    QLabel *app = new QLabel("Appréciation : <b>Excellent</b>");
    app->setStyleSheet(qssCard());
    l->addWidget(app);

    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(qssBtnGreenClose());
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btnClose, 0, Qt::AlignRight);
    d.exec();
}

// --- FONCTIONS EXISTANTES ---
void MainWindow::exporterPDF(QTableWidget *table, QString titre) {
    if(!table) return;

    QString f = QFileDialog::getSaveFileName(this, "Export", titre + ".pdf", "PDF (*.pdf)");
    if(f.isEmpty()) return;
    if(QFileInfo(f).suffix().isEmpty()) f += ".pdf";

    auto tableToHtml = [](QTableWidget *t) -> QString {
        const int rows = t->rowCount();
        const int cols = t->columnCount();

        QString html;
        html += "<table class='data'>";

        html += "<thead><tr>";
        for(int c = 0; c < cols; c++) {
            const QString head = t->horizontalHeaderItem(c) ? t->horizontalHeaderItem(c)->text() : QString("Col %1").arg(c + 1);
            html += "<th>" + head.toHtmlEscaped() + "</th>";
        }
        html += "</tr></thead>";

        html += "<tbody>";
        for(int r = 0; r < rows; r++) {
            html += "<tr>";
            for(int c = 0; c < cols; c++) {
                const QString v = t->item(r, c) ? t->item(r, c)->text() : QString();
                html += "<td>" + v.toHtmlEscaped() + "</td>";
            }
            html += "</tr>";
        }
        html += "</tbody></table>";
        return html;
    };

    const QString dateStr = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");

    const QString css =
        "<style>"
        "  @page { margin: 16mm 12mm 16mm 12mm; }"
        "  body { font-family: 'Segoe UI', Arial, sans-serif; color: #3e2723; }"
        "  .header { padding: 10px 12px; border: 1px solid #d7ccc8; border-radius: 10px; background: #f3f0eb; }"
        "  .brand { font-size: 13px; letter-spacing: 2px; font-weight: 800; color: #2c1a16; text-transform: uppercase; }"
        "  .meta { font-size: 10px; color: #795548; }"
        "  h1 { margin: 14px 0 10px 0; font-size: 22px; font-weight: 800; color: #1a1210; }"
        "  .accent { height: 4px; width: 100%; background: #d4af37; border-radius: 3px; margin: 4px 0 14px 0; }"
        "  table.data { width: 100%; border-collapse: collapse; border: 1px solid #d7ccc8; border-radius: 8px; overflow: hidden; }"
        "  table.data th { background: #2c1a16; color: #e0c097; text-transform: uppercase; font-size: 10px; letter-spacing: 1px; padding: 10px 8px; }"
        "  table.data td { padding: 9px 8px; border-bottom: 1px solid #eee5dd; font-size: 10.5px; }"
        "  table.data tr:nth-child(even) td { background: #f7f4ef; }"
        "  .footer { margin-top: 12px; font-size: 9.5px; color: #795548; }"
        "</style>";

    const QString html =
        "<html><head>" + css + "</head><body>"
                               "  <div class='header'>"
                               "    <div class='brand'>FIL D'OR</div>"
                               "    <div class='meta'><b>Document :</b> " + titre.toHtmlEscaped() + " &nbsp; | &nbsp; <b>Généré :</b> " + dateStr.toHtmlEscaped() + "</div>"
                                                                                                  "  </div>"
                                                                                                  "  <h1>" + titre.toHtmlEscaped() + "</h1>"
                                  "  <div class='accent'></div>"
        + tableToHtml(table) +
        "  <div class='footer'>Export PDF — Production Manager</div>"
        "</body></html>";

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(f);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Succès", "PDF généré !");
}

void MainWindow::exporterCSV(QTableWidget *table, const QString &titreDocument)
{
    if(!table) return;

    QString f = QFileDialog::getSaveFileName(this, "Export", titreDocument + ".csv", "CSV (*.csv)");
    if(f.isEmpty()) return;
    if(QFileInfo(f).suffix().isEmpty()) f += ".csv";

    QFile file(f);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export", "Impossible d'écrire le fichier CSV.");
        return;
    }

    auto csvEscape = [](const QString &s) -> QString {
        QString out = s;
        out.replace('"', "\"\"");
        if(out.contains(';') || out.contains('\n') || out.contains('\r') || out.contains('"')) {
            out = '"' + out + '"';
        }
        return out;
    };

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);

    const int cols = table->columnCount();
    const int rows = table->rowCount();

    // Header
    for(int c = 0; c < cols; c++) {
        const QString head = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text() : QString("Col %1").arg(c + 1);
        ts << csvEscape(head);
        if(c < cols - 1) ts << ';';
    }
    ts << "\n";

    // Data
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {
            const QString v = table->item(r, c) ? table->item(r, c)->text() : QString();
            ts << csvEscape(v);
            if(c < cols - 1) ts << ';';
        }
        ts << "\n";
    }

    file.close();
    QMessageBox::information(this, "Succès", "CSV exporté !");
}

// ===============================
// === MODULE CLIENTS (CRUD)   ===
// ===============================

void MainWindow::rafraichirListeClients()
{
    const QString q = ui->le_search_client->text().trimmed();
    ui->tableClients->setRowCount(0);
    ui->tableClients->setColumnCount(6);
    ui->tableClients->setHorizontalHeaderLabels({"ID Client","Nom","Téléphone","Adresse","Email","Fidélité"});

    int row = 0;
    for(int i = 0; i < mesClients.size(); i++) {
        const ClientInfo &c = mesClients[i];
        if(!q.isEmpty()) {
            const QString hay = (c.id + " " + c.nom + " " + c.telephone + " " + c.adresse + " " + c.email).toLower();
            if(!hay.contains(q.toLower())) continue;
        }

        ui->tableClients->insertRow(row);

        auto *it0 = new QTableWidgetItem(c.id);
        it0->setData(Qt::UserRole, i);
        ui->tableClients->setItem(row, 0, it0);
        ui->tableClients->setItem(row, 1, new QTableWidgetItem(c.nom));
        ui->tableClients->setItem(row, 2, new QTableWidgetItem(c.telephone));
        ui->tableClients->setItem(row, 3, new QTableWidgetItem(c.adresse));
        ui->tableClients->setItem(row, 4, new QTableWidgetItem(c.email));
        ui->tableClients->setItem(row, 5, new QTableWidgetItem(QString::number(c.pointsFidelite)));

        row++;
    }
}

void MainWindow::calculerStatsClients()
{
    const int total = mesClients.size();
    ui->val_tot_cli->setText(QString::number(total));

    int vip = 0;
    int tunis = 0;
    int sfax = 0;
    int stdSeg = 0;
    int silver = 0;
    int vipSeg = 0;
    int sumPts = 0;

    for(const auto &c : mesClients) {
        if(c.pointsFidelite >= 100) vip++;
        sumPts += c.pointsFidelite;
        if(c.pointsFidelite < 50) stdSeg++;
        else if(c.pointsFidelite < 100) silver++;
        else vipSeg++;
        const QString adr = c.adresse.toLower();
        if(adr.contains("tunis")) tunis++;
        else if(adr.contains("sfax")) sfax++;
    }
    const int autres = total - tunis - sfax;

    ui->val_vip_cli->setText(QString::number(vip));
    const double avgPts = (total <= 0) ? 0.0 : (static_cast<double>(sumPts) / static_cast<double>(total));
    ui->val_avg_pts_cli->setText(QString::number(avgPts, 'f', 0) + " pts");

    ui->pb_cli_tunis->setMaximum(std::max(1, total));
    ui->pb_cli_sfax->setMaximum(std::max(1, total));
    ui->pb_cli_autres->setMaximum(std::max(1, total));

    ui->pb_cli_tunis->setValue(tunis);
    ui->pb_cli_sfax->setValue(sfax);
    ui->pb_cli_autres->setValue(std::max(0, autres));

    const int maxBase = std::max(1, total);
    ui->pb_cli_std->setMaximum(maxBase);
    ui->pb_cli_silver->setMaximum(maxBase);
    ui->pb_cli_vip->setMaximum(maxBase);
    ui->pb_cli_std->setValue(stdSeg);
    ui->pb_cli_silver->setValue(silver);
    ui->pb_cli_vip->setValue(vipSeg);
}

void MainWindow::exporterFactureClient()
{
    const int r = ui->tableClients->currentRow();
    if(r < 0) {
        QMessageBox::information(this, "Facture", "Sélectionnez un client dans la liste.");
        return;
    }

    QTableWidgetItem *it = ui->tableClients->item(r, 0);
    if(!it) return;
    const int idx = it->data(Qt::UserRole).toInt();
    if(idx < 0 || idx >= mesClients.size()) return;
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

    QMessageBox::information(this, "Succès", "Facture exportée en PDF.");
}

void MainWindow::showClientIaDialog()
{
    const int r = ui->tableClients->currentRow();
    if(r < 0) {
        QMessageBox::information(this, "Prévision IA", "Sélectionnez un client pour lancer la prévision.");
        return;
    }
    QTableWidgetItem *it = ui->tableClients->item(r, 0);
    if(!it) return;
    const int idx = it->data(Qt::UserRole).toInt();
    if(idx < 0 || idx >= mesClients.size()) return;
    const ClientInfo &c = mesClients[idx];

    const QString niveau = (c.pointsFidelite >= 200) ? "Platine" : (c.pointsFidelite >= 100) ? "Gold" : (c.pointsFidelite >= 50) ? "Silver" : "Bronze";
    const QString proba = (c.pointsFidelite >= 100) ? "Forte" : (c.pointsFidelite >= 50) ? "Moyenne" : "Faible";
    const QString action = (c.pointsFidelite >= 100) ? "Offre VIP + priorité de service" : (c.pointsFidelite >= 50) ? "Offre fidélité (5%)" : "Campagne de réactivation";

    QDialog d(this);
    d.setWindowTitle("Prévision IA (Clients)");
    applyDialogBase(d, 560, 340);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Prévision IA – Client");
    ti->setAlignment(Qt::AlignCenter);
    ti->setStyleSheet(qssTitle("#f3e5f5"));
    l->addWidget(ti);

    QLabel *info = new QLabel(
        "<b>Client :</b> " + c.nom + " (" + c.id + ")<br/>"
                                                   "<b>Niveau :</b> " + niveau + "<br/>"
                   "<b>Probabilité de retour :</b> " + proba + "<br/>"
                  "<b>Action suggérée :</b> " + action);
    info->setStyleSheet(qssCard());
    l->addWidget(info);

    QPushButton *close = new QPushButton("Fermer");
    close->setStyleSheet(qssBtnGreenClose());
    connect(close, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(close, 0, Qt::AlignRight);
    d.exec();
}

void MainWindow::showFideliteDialog()
{
    QDialog d(this);
    d.setWindowTitle("Programme de fidélité");
    applyDialogBase(d, 620, 380);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Programme de fidélité");
    ti->setStyleSheet(qssTitle("#e8f5e9"));
    ti->setAlignment(Qt::AlignCenter);
    l->addWidget(ti);

    QFormLayout *f = new QFormLayout();
    QComboBox *cb = new QComboBox();
    for(const auto &c : mesClients) cb->addItem(c.id + " - " + c.nom);
    cb->setStyleSheet(qssInput());

    QSpinBox *sb = new QSpinBox();
    sb->setRange(0, 100000);
    sb->setValue(10);
    sb->setStyleSheet(qssInput());

    QLabel *lbl = new QLabel("Sélectionnez un client puis ajoutez des points.");
    lbl->setStyleSheet(qssCard());
    f->addRow("Client :", cb);
    f->addRow("Ajouter points :", sb);
    l->addLayout(f);
    l->addWidget(lbl);

    QPushButton *btnApply = new QPushButton("Appliquer");
    QPushButton *btnClose = new QPushButton("Fermer");
    btnApply->setStyleSheet(qssBtnPrimaryTeal());
    btnClose->setStyleSheet(qssBtnGreenClose());

    connect(btnApply, &QPushButton::clicked, [=]() {
        const int idx = cb->currentIndex();
        if(idx < 0 || idx >= mesClients.size()) return;
        mesClients[idx].pointsFidelite += sb->value();
        rafraichirListeClients();
        lbl->setText("Points mis à jour: <b>" + QString::number(mesClients[idx].pointsFidelite) + "</b>");
    });
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);

    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(btnApply);
    hl->addStretch(1);
    hl->addWidget(btnClose);
    l->addLayout(hl);

    d.exec();
}

// ===============================
// === MODULE DEPOT (CRUD)     ===
// ===============================

void MainWindow::rafraichirListeDepots()
{
    const QString q = ui->le_search_depot->text().trimmed();
    ui->tableDepot->setRowCount(0);
    ui->tableDepot->setColumnCount(7);
    ui->tableDepot->setHorizontalHeaderLabels({"ID","Emplacement","Étagère","Capacité Max","Quantité","Type","Remplissage"});

    int row = 0;
    for(int i = 0; i < mesDepots.size(); i++) {
        const DepotInfo &d = mesDepots[i];
        if(!q.isEmpty()) {
            const QString hay = (d.id + " " + d.emplacement + " " + d.etagere + " " + d.typeStockage).toLower();
            if(!hay.contains(q.toLower())) continue;
        }

        const double fill = (d.capaciteMax <= 0) ? 0.0 : (d.quantiteActuelle / d.capaciteMax) * 100.0;

        ui->tableDepot->insertRow(row);
        auto *it0 = new QTableWidgetItem(d.id);
        it0->setData(Qt::UserRole, i);
        ui->tableDepot->setItem(row, 0, it0);
        ui->tableDepot->setItem(row, 1, new QTableWidgetItem(d.emplacement));
        ui->tableDepot->setItem(row, 2, new QTableWidgetItem(d.etagere));
        ui->tableDepot->setItem(row, 3, new QTableWidgetItem(QString::number(d.capaciteMax, 'f', 1)));
        ui->tableDepot->setItem(row, 4, new QTableWidgetItem(QString::number(d.quantiteActuelle, 'f', 1)));
        ui->tableDepot->setItem(row, 5, new QTableWidgetItem(d.typeStockage));
        ui->tableDepot->setItem(row, 6, new QTableWidgetItem(QString::number(fill, 'f', 1) + "%"));
        row++;
    }
}

void MainWindow::calculerStatsDepots()
{
    const int total = mesDepots.size();
    ui->val_tot_dep->setText(QString::number(total));

    double sumFill = 0.0;
    double maxFill = 0.0;
    int froid = 0;
    int sec = 0;
    int low = 0, mid = 0, high = 0;
    for(const auto &d : mesDepots) {
        const double fill = (d.capaciteMax <= 0) ? 0.0 : (d.quantiteActuelle / d.capaciteMax) * 100.0;
        sumFill += fill;
        if(fill > maxFill) maxFill = fill;
        if(fill <= 33.0) low++;
        else if(fill <= 66.0) mid++;
        else high++;
        if(d.typeStockage.toLower().contains("froid")) froid++;
        else sec++;
    }
    const double avgFill = (total <= 0) ? 0.0 : (sumFill / total);
    ui->val_avg_fill->setText(QString::number(avgFill, 'f', 1) + "%");
    ui->val_max_fill_dep->setText(QString::number(maxFill, 'f', 1) + "%");

    ui->pb_dep_froid->setMaximum(std::max(1, total));
    ui->pb_dep_sec->setMaximum(std::max(1, total));
    ui->pb_dep_froid->setValue(froid);
    ui->pb_dep_sec->setValue(sec);

    const int maxBase = std::max(1, total);
    ui->pb_dep_low->setMaximum(maxBase);
    ui->pb_dep_mid->setMaximum(maxBase);
    ui->pb_dep_high->setMaximum(maxBase);
    ui->pb_dep_low->setValue(low);
    ui->pb_dep_mid->setValue(mid);
    ui->pb_dep_high->setValue(high);
}

void MainWindow::showOptimizeSpaceDialog()
{
    QDialog d(this);
    d.setWindowTitle("Optimisation d'Espace (First Fit)");
    applyDialogBase(d, 650, 420);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Optimisation d'Espace – First Fit");
    ti->setStyleSheet(qssTitle("#e1f5fe"));
    ti->setAlignment(Qt::AlignCenter);
    l->addWidget(ti);

    QFormLayout *f = new QFormLayout();
    QDoubleSpinBox *sbQty = new QDoubleSpinBox();
    sbQty->setRange(0, 100000);
    sbQty->setValue(50);
    sbQty->setStyleSheet(qssInput());
    QComboBox *cbType = new QComboBox();
    cbType->addItems({"Froid","Sec"});
    cbType->setStyleSheet(qssInput());
    f->addRow("Quantité à ranger :", sbQty);
    f->addRow("Type stockage :", cbType);
    l->addLayout(f);

    QLabel *res = new QLabel("Résultat : -");
    res->setStyleSheet(qssCard());
    l->addWidget(res);

    QPushButton *btn = new QPushButton("Trouver un emplacement");
    btn->setStyleSheet(qssBtnPrimaryTeal());
    connect(btn, &QPushButton::clicked, [=](){
        const double qty = sbQty->value();
        const QString type = cbType->currentText();
        int found = -1;
        for(int i = 0; i < mesDepots.size(); i++) {
            const auto &dep = mesDepots[i];
            if(dep.typeStockage != type) continue;
            const double free = dep.capaciteMax - dep.quantiteActuelle;
            if(free >= qty) { found = i; break; }
        }
        if(found < 0) {
            res->setText("Résultat : Aucun emplacement compatible (capacité insuffisante).");
        } else {
            const auto &dep = mesDepots[found];
            res->setText("Résultat : " + dep.id + " | " + dep.emplacement + " | Étagère " + dep.etagere +
                         "<br/><b>Capacité libre :</b> " + QString::number(dep.capaciteMax - dep.quantiteActuelle, 'f', 1));
        }
    });
    l->addWidget(btn);

    QPushButton *close = new QPushButton("Fermer");
    close->setStyleSheet(qssBtnGreenClose());
    connect(close, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(close, 0, Qt::AlignRight);
    d.exec();
}

void MainWindow::showRavitaillementDialog()
{
    QDialog d(this);
    d.setWindowTitle("Ravitaillement & Livraison");
    applyDialogBase(d, 680, 460);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    QLabel *ti = new QLabel("Ravitaillement & Livraison");
    ti->setStyleSheet(qssTitle("#fff3e0"));
    ti->setAlignment(Qt::AlignCenter);
    l->addWidget(ti);

    QFormLayout *f = new QFormLayout();
    QComboBox *cbDepot = new QComboBox();
    for(const auto &dep : mesDepots) cbDepot->addItem(dep.id + " - " + dep.emplacement);
    cbDepot->setStyleSheet(qssInput());
    QComboBox *cbOp = new QComboBox();
    cbOp->addItems({"Ravitaillement (+)", "Livraison (-)"});
    cbOp->setStyleSheet(qssInput());
    QDoubleSpinBox *sbQty = new QDoubleSpinBox();
    sbQty->setRange(0, 100000);
    sbQty->setValue(10);
    sbQty->setStyleSheet(qssInput());

    QDateEdit *deLivraison = new QDateEdit(QDate::currentDate());
    deLivraison->setCalendarPopup(true);
    deLivraison->setDisplayFormat("dd/MM/yyyy");
    deLivraison->setStyleSheet(qssInput());
    f->addRow("Emplacement :", cbDepot);
    f->addRow("Opération :", cbOp);
    f->addRow("Quantité :", sbQty);
    f->addRow("Date de livraison :", deLivraison);
    l->addLayout(f);

    QLabel *res = new QLabel("-");
    res->setStyleSheet(qssCard());
    l->addWidget(res);

    QPushButton *apply = new QPushButton("Appliquer");
    QPushButton *close = new QPushButton("Fermer");
    apply->setStyleSheet(qssBtnOrange());
    close->setStyleSheet(qssBtnGreenClose());

    connect(apply, &QPushButton::clicked, [=](){
        const int idx = cbDepot->currentIndex();
        if(idx < 0 || idx >= mesDepots.size()) return;
        const bool add = (cbOp->currentIndex() == 0);
        const double qty = sbQty->value();
        DepotInfo &dep = mesDepots[idx];

        double next = dep.quantiteActuelle + (add ? qty : -qty);
        if(next < 0) next = 0;
        if(dep.capaciteMax > 0 && next > dep.capaciteMax) next = dep.capaciteMax;
        dep.quantiteActuelle = next;
        rafraichirListeDepots();

        const double fill = (dep.capaciteMax <= 0) ? 0.0 : (dep.quantiteActuelle / dep.capaciteMax) * 100.0;
        const QString dt = deLivraison->date().toString("dd/MM/yyyy");
        res->setText("Nouveau stock: <b>" + QString::number(dep.quantiteActuelle, 'f', 1) +
                     "</b> | Remplissage: <b>" + QString::number(fill, 'f', 1) + "%</b>" +
                     "<br/>Date de livraison : <b>" + dt + "</b>");
    });
    connect(close, &QPushButton::clicked, &d, &QDialog::accept);

    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(apply);
    hl->addStretch(1);
    hl->addWidget(close);
    l->addLayout(hl);

    d.exec();
}

void MainWindow::calculerStatsStock() {
    const int total = mesMatieres.size();
    ui->val_tot_mat->setText(QString::number(total));

    double volume = 0.0;
    double valeur = 0.0;
    int cuir = 0, teint = 0, chim = 0;
    int qa = 0, qb = 0, qc = 0;

    for(const auto &m : mesMatieres) {
        volume += m.quantite;

        const QString cat = m.categorie.toLower();
        if(cat.contains("cuir")) cuir++;
        else if(cat.contains("teint")) teint++;
        else chim++;

        const QString q = m.qualite.toLower();
        double qFactor = 0.7;
        if(q.contains("a")) { qa++; qFactor = 1.0; }
        else if(q.contains("b")) { qb++; qFactor = 0.7; }
        else { qc++; qFactor = 0.5; }

        // Estimation simple (démo) : base selon catégorie, pondérée par qualité.
        // (Les couleurs/style sont le focus; la valeur est une estimation.)
        double base = 20.0;
        if(cat.contains("cuir")) base = 80.0;
        else if(cat.contains("teint")) base = 25.0;
        else base = 15.0;

        valeur += (m.quantite * base * qFactor);
    }

    ui->val_tot_vol->setText(QString::number(volume, 'f', 1) + " M²");
    ui->val_tot_val_stock->setText(QLocale(QLocale::French).toString(valeur, 'f', 0) + " DT");

    const int maxBase = std::max(1, total);
    ui->pb_cuir->setMaximum(maxBase);
    ui->pb_teint->setMaximum(maxBase);
    ui->pb_chim->setMaximum(maxBase);
    ui->pb_cuir->setValue(cuir);
    ui->pb_teint->setValue(teint);
    ui->pb_chim->setValue(chim);

    ui->pb_qa->setMaximum(maxBase);
    ui->pb_qb->setMaximum(maxBase);
    ui->pb_qc->setMaximum(maxBase);
    ui->pb_qa->setValue(qa);
    ui->pb_qb->setValue(qb);
    ui->pb_qc->setValue(qc);

    // Charts (Stock dashboard)
    setPieChart(ui->w_pie_stk, "Catégories", {{"Cuir", cuir}, {"Teinture", teint}, {"Chimique", chim}});
    setVerticalBarChart(ui->w_bar_stk, "Qualité", {"A", "B", "C"}, {static_cast<double>(qa), static_cast<double>(qb), static_cast<double>(qc)});
}
void MainWindow::rafraichirListeMatieres() {
    ui->tableStock->setRowCount(0); ui->tableStock->setColumnCount(7); ui->tableStock->setHorizontalHeaderLabels({"Code","Cat","Etat","Qual","Qté","Stock","Zone"});
    for(int i=0; i<mesMatieres.size(); i++) {
        ui->tableStock->insertRow(i);
        auto *it0 = new QTableWidgetItem(mesMatieres[i].code);
        it0->setData(Qt::UserRole, i);
        ui->tableStock->setItem(i,0,it0);
        ui->tableStock->setItem(i,1,new QTableWidgetItem(mesMatieres[i].categorie));
        ui->tableStock->setItem(i,2,new QTableWidgetItem(mesMatieres[i].etat));
        ui->tableStock->setItem(i,3,new QTableWidgetItem(mesMatieres[i].qualite));
        ui->tableStock->setItem(i,4,new QTableWidgetItem(QString::number(mesMatieres[i].quantite)));
        ui->tableStock->setItem(i,5,new QTableWidgetItem(mesMatieres[i].typeStock));
        ui->tableStock->setItem(i,6,new QTableWidgetItem(mesMatieres[i].zone));
    }
}
void MainWindow::calculerStatsRH() {
    int t=mesEmployes.size(); ui->val_tot_emp->setText(QString::number(t)); double m=0; int p=0,l=0,b=0;
    for(const auto &e:mesEmployes) { m+=e.salaire; if(e.departement.contains("Prod")) p++; else if(e.departement.contains("Log")) l++; else b++; }
    ui->val_masse_sal->setText(QString::number(m)); ui->pb_prod_rh->setMaximum(t); ui->pb_prod_rh->setValue(p); ui->pb_log_rh->setMaximum(t); ui->pb_log_rh->setValue(l); ui->pb_be_rh->setMaximum(t); ui->pb_be_rh->setValue(b);
}
void MainWindow::reponseChatbot() { ui->txt_chat_history->append("Bot: Bonjour !"); }
void MainWindow::rafraichirListeEmployes() {
    ui->tableEmployes->setRowCount(0); ui->tableEmployes->setColumnCount(7); ui->tableEmployes->setHorizontalHeaderLabels({"ID","Nom","Pre","Pos","Dep","Sal","Date"});
    for(int i=0; i<mesEmployes.size(); i++) {
        ui->tableEmployes->insertRow(i);
        auto *it0 = new QTableWidgetItem(mesEmployes[i].id);
        it0->setData(Qt::UserRole, i);
        ui->tableEmployes->setItem(i,0,it0);
        ui->tableEmployes->setItem(i,1,new QTableWidgetItem(mesEmployes[i].nom));
        ui->tableEmployes->setItem(i,2,new QTableWidgetItem(mesEmployes[i].prenom));
        ui->tableEmployes->setItem(i,3,new QTableWidgetItem(mesEmployes[i].poste));
        ui->tableEmployes->setItem(i,4,new QTableWidgetItem(mesEmployes[i].departement));
        ui->tableEmployes->setItem(i,5,new QTableWidgetItem(QString::number(mesEmployes[i].salaire)));
        ui->tableEmployes->setItem(i,6,new QTableWidgetItem(mesEmployes[i].dateEmbauche.toString("dd/MM")));
    }
}
void MainWindow::rafraichirListeProduits() {
    ui->tableProduits->setRowCount(0); ui->tableProduits->setColumnCount(6); ui->tableProduits->setHorizontalHeaderLabels({"Ref","Nom","Cout","Col","Cuir","Temps"});
    for(int i=0; i<mesProduits.size(); i++) {
        ui->tableProduits->insertRow(i);
        auto *it0 = new QTableWidgetItem(mesProduits[i].ref);
        it0->setData(Qt::UserRole, i);
        ui->tableProduits->setItem(i,0,it0);
        ui->tableProduits->setItem(i,1,new QTableWidgetItem(mesProduits[i].nom));
        ui->tableProduits->setItem(i,2,new QTableWidgetItem(QString::number(mesProduits[i].coutMatiere)));
        ui->tableProduits->setItem(i,3,new QTableWidgetItem(mesProduits[i].collection));
        ui->tableProduits->setItem(i,4,new QTableWidgetItem(mesProduits[i].cuir));
        ui->tableProduits->setItem(i,5,new QTableWidgetItem(QString::number(mesProduits[i].temps)));
    }
}
void MainWindow::calculerStatsProduits() {
    const int t = mesProduits.size();
    ui->val_tot_ref->setText(QString::number(t));

    double sumCost = 0.0;
    int hiver = 0, ete = 0, intemp = 0;
    int vachette = 0, agneau = 0, croco = 0;

    for(const auto &p : mesProduits) {
        sumCost += p.coutMatiere;

        const QString col = p.collection.toLower();
        if(col.contains("hiv")) hiver++;
        else if(col.contains("été") || col.contains("ete")) ete++;
        else intemp++;

        const QString cuir = p.cuir.toLower();
        if(cuir.contains("vach")) vachette++;
        else if(cuir.contains("agn")) agneau++;
        else if(cuir.contains("croc")) croco++;
    }

    const double avg = (t <= 0) ? 0.0 : (sumCost / static_cast<double>(t));
    ui->val_avg_cost->setText(QString::number(avg, 'f', 1) + " €");
    ui->val_tot_val_prod->setText(QLocale(QLocale::French).toString(sumCost, 'f', 0) + " €");

    const int maxBase = std::max(1, t);
    ui->pb_hiver->setMaximum(maxBase);
    ui->pb_ete->setMaximum(maxBase);
    ui->pb_intemp->setMaximum(maxBase);
    ui->pb_hiver->setValue(hiver);
    ui->pb_ete->setValue(ete);
    ui->pb_intemp->setValue(intemp);

    ui->pb_cuir_vachette->setMaximum(maxBase);
    ui->pb_cuir_agneau->setMaximum(maxBase);
    ui->pb_cuir_croco->setMaximum(maxBase);
    ui->pb_cuir_vachette->setValue(vachette);
    ui->pb_cuir_agneau->setValue(agneau);
    ui->pb_cuir_croco->setValue(croco);
}
void MainWindow::rafraichirListeCommandes() {
    ui->tablePlanif->setRowCount(0); ui->tablePlanif->setColumnCount(7); ui->tablePlanif->setHorizontalHeaderLabels({"ID","Prod","Qté","Mat","Deb","Fin","Stat"});
    for(int i=0; i<mesCommandes.size(); i++) {
        ui->tablePlanif->insertRow(i);
        auto *it0 = new QTableWidgetItem(mesCommandes[i].id);
        it0->setData(Qt::UserRole, i);
        ui->tablePlanif->setItem(i,0,it0);
        ui->tablePlanif->setItem(i,1,new QTableWidgetItem(mesCommandes[i].produit));
        ui->tablePlanif->setItem(i,2,new QTableWidgetItem(QString::number(mesCommandes[i].quantite)));
        ui->tablePlanif->setItem(i,3,new QTableWidgetItem(mesCommandes[i].matiere));
        ui->tablePlanif->setItem(i,4,new QTableWidgetItem(mesCommandes[i].dateDebut.toString("dd/MM")));
        ui->tablePlanif->setItem(i,5,new QTableWidgetItem(mesCommandes[i].dateFinEstimee));
        ui->tablePlanif->setItem(i,6,new QTableWidgetItem(mesCommandes[i].statut));
    }
}
void MainWindow::calculerEtAfficherStats() {
    const int total = mesCommandes.size();
    ui->lbl_stat_total_cmd->setText(QString::number(total));

    int totalQty = 0;
    int retard = 0;
    int plan = 0;
    int cours = 0;
    int sacs = 0;
    int portefeuilles = 0;
    int ceintures = 0;

    for(const auto &cmd : mesCommandes) {
        totalQty += cmd.quantite;

        const QString st = cmd.statut.toLower();
        if(st.contains("plan")) plan++;
        else if(st.contains("cours")) cours++;

        const QString prod = cmd.produit.toLower();
        if(prod.contains("sac")) sacs++;
        else if(prod.contains("portefeuille")) portefeuilles++;
        else if(prod.contains("ceinture")) ceintures++;

        if(cmd.etatEtape == 2 || st.contains("retard")) retard++;
    }

    const int termine = std::max(0, total - plan - cours);
    const double retardPct = (total <= 0) ? 0.0 : (static_cast<double>(retard) * 100.0 / static_cast<double>(total));

    ui->lbl_stat_total_qty->setText(QString::number(totalQty));
    ui->lbl_stat_retard->setText(QString::number(retardPct, 'f', 0) + "%");

    const int maxBase = std::max(1, total);
    ui->pb_sac->setMaximum(maxBase);
    ui->pb_portefeuille->setMaximum(maxBase);
    ui->pb_ceinture->setMaximum(maxBase);
    ui->pb_sac->setValue(sacs);
    ui->pb_portefeuille->setValue(portefeuilles);
    ui->pb_ceinture->setValue(ceintures);

    ui->pb_cmd_plan->setMaximum(maxBase);
    ui->pb_cmd_cours->setMaximum(maxBase);
    ui->pb_cmd_ret->setMaximum(maxBase);
    ui->pb_cmd_plan->setValue(plan);
    ui->pb_cmd_cours->setValue(cours);
    ui->pb_cmd_ret->setValue(retard);
}
// --------------------------- IA PLANIFICATION (POP-UP) ---------------------------
void MainWindow::showPlanifIaDialog() {
    QDialog d(this);
    d.setWindowTitle("IA Estimation Temps");
    applyDialogBase(d, 650, 460);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(18, 18, 18, 18);
    l->setSpacing(14);

    // Titre
    QLabel *ti = new QLabel("IA Estimation & Optimisation");
    ti->setStyleSheet(qssTitle("#f3e5f5"));
    ti->setAlignment(Qt::AlignCenter);
    l->addWidget(ti);

    // Formulaire
    QFormLayout *f = new QFormLayout();

    QComboBox *cbProd = new QComboBox();
    cbProd->addItems({"Sac Voyage Cuir", "Portefeuille Luxe", "Ceinture Homme"});
    cbProd->setStyleSheet(qssInput());

    QSpinBox *sbQte = new QSpinBox();
    sbQte->setRange(0, 10000);
    sbQte->setValue(100);
    sbQte->setStyleSheet(qssInput());

    f->addRow("Produit :", cbProd);
    f->addRow("Quantité à produire :", sbQte);

    l->addSpacing(10);
    l->addLayout(f);

    // Résultat
    QLabel *lblRes = new QLabel("Durée Estimée : -");
    lblRes->setStyleSheet(QString("font-size: 15px; font-weight: bold; color: #4a148c; ") + qssCard());
    lblRes->setAlignment(Qt::AlignCenter);

    // Bouton Calculer
    QPushButton *btnCalc = new QPushButton("Lancer l'Analyse IA");
    btnCalc->setStyleSheet(qssBtnPurple());

    // Logique du calcul (identique à celle que vous aviez, mais dans la pop-up)
    connect(btnCalc, &QPushButton::clicked, [=](){
        // Exemple de logique : 2 jours de base + 1 jour tous les 100 articles
        // + une petite variation selon le produit (ex: Sac = plus long)
        int base = 2;
        if(cbProd->currentText().contains("Sac")) base = 4;

        int j = base + (sbQte->value() / 50);

        // Date de fin estimée (par rapport à aujourd'hui)
        QDate fin = QDate::currentDate().addDays(j);

        lblRes->setText("Durée Estimée : " + QString::number(j) + " Jours\n"
                                                                  "Fin Prévue : " + fin.toString("dd/MM/yyyy"));
    });

    l->addSpacing(10);
    l->addWidget(btnCalc);
    l->addWidget(lblRes);

    // Bouton Fermer
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(qssBtnGreenClose());
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btnClose, 0, Qt::AlignRight);

    d.exec();
}
// --------------------------- FABRICATION (TIMELINE GANTT) ---------------------------
void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline;
    t->clear();
    t->setRowCount(0);

    int jours = 30; // Nombre de jours à afficher
    t->setColumnCount(1 + jours); // 1 colonne Produit + 30 colonnes jours

    // Génération des en-têtes avec la date d'aujourd'hui
    QStringList headers;
    headers << "PRODUIT";

    QDate today = QDate::currentDate(); // Date du jour dynamique

    for(int i = 0; i < jours; i++) {
        // Affiche le jour (ex: "09" pour le 9 février)
        headers << today.addDays(i).toString("dd");
    }

    t->setHorizontalHeaderLabels(headers);

    // Ajustement des largeurs
    t->setColumnWidth(0, 200); // Colonne Produit large
    for(int i = 1; i <= jours; i++) {
        t->setColumnWidth(i, 35); // Colonnes jours étroites
    }

    // Remplissage des données (Commandes)
    t->setRowCount(mesCommandes.size());
    for(int i = 0; i < mesCommandes.size(); i++) {
        // Colonne 0 : Nom de la commande
        t->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id));
        t->setRowHeight(i, 50);

        // Calcul du décalage (offset) par rapport à aujourd'hui
        // Si la commande commence aujourd'hui, offset = 0.
        // Si elle commence demain, offset = 1.
        int offset = today.daysTo(mesCommandes[i].dateDebut);

        // Si la commande est dans le passé, on ne l'affiche pas ou on la coupe
        if(offset < 0) offset = 0;

        // Définition des couleurs (Identique à ta demande précédente)
        QColor c1(41,128,185); // Bleu (Coupe)
        QColor c2(211,84,0);   // Orange (Assemblage)
        QColor c3(39,174,96);  // Vert (Couture)
        QColor c4(241,196,15); // Jaune (Finition)

        // Gestion du statut (Retard / OK)
        if(mesCommandes[i].etatEtape == 2) { // Retard
            QColor r(192,57,43); // Rouge
            if(mesCommandes[i].etapeAuditee == "Coupe") c1 = r;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2 = r;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3 = r;
            else c4 = r;
        }
        else if(mesCommandes[i].etatEtape == 1) { // OK
            QColor o(46,204,113); // Vert clair
            if(mesCommandes[i].etapeAuditee == "Coupe") c1 = o;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2 = o;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3 = o;
            else c4 = o;
        }

        // Dessin des barres (offset + 1 car la colonne 0 est le nom du produit)
        // Vérification pour ne pas dessiner hors du tableau
        if(offset < jours) dessinerBarre(i, offset + 1, 2, "Cp", c1, Qt::white);
        if(offset + 2 < jours) dessinerBarre(i, offset + 3, 3, "As", c2, Qt::white);
        if(offset + 5 < jours) dessinerBarre(i, offset + 6, 3, "Co", c3, Qt::white);
        if(offset + 8 < jours) dessinerBarre(i, offset + 9, 2, "Fi", c4, Qt::black);
    }

}
void MainWindow::dessinerBarre(int r, int c, int d, QString t, QColor b, QColor f) {
    if(c+d>30) return; QTableWidgetItem *it=new QTableWidgetItem(t); it->setData(Qt::BackgroundRole,b); it->setForeground(f);
    ui->tableTimeline->setItem(r,c,it); ui->tableTimeline->setSpan(r,c,1,d);
}
