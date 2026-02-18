#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include <QSqlRecord>
#include <QSqlQueryModel>
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
#include <QSqlRecord>      // <--- Indispensable pour utiliser .record()
#include <QSqlQueryModel>  // <--- Indispensable pour utiliser le modèle
#include <QSqlError>

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
    Connexion c;
    if(!c.createconnect()) {
        QMessageBox::warning(this, "Erreur BDD", "Impossible de se connecter à la base Oracle.");
    }

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
        ui->l_logo_img->setText("logo.png");
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
    connect(ui->btn_sort_alpha_planif, &QPushButton::clicked, [=](){ ui->tablePlanif->sortItems(1, Qt::AscendingOrder); });    // Produit
    connect(ui->btn_sort_alpha_prod, &QPushButton::clicked, [=](){ ui->tableProduits->sortItems(1, Qt::AscendingOrder); });     // Désignation
    connect(ui->btn_sort_alpha_emp, &QPushButton::clicked, [=](){ ui->tableEmployes->sortItems(1, Qt::AscendingOrder); });      // Nom
    connect(ui->btn_sort_alpha_stock, &QPushButton::clicked, [=](){ ui->tableStock->sortItems(0, Qt::AscendingOrder); });       // Code MP
    connect(ui->btn_sort_alpha_client, &QPushButton::clicked, [=](){ ui->tableClients->sortItems(1, Qt::AscendingOrder); });    // Nom
    connect(ui->btn_sort_alpha_depot, &QPushButton::clicked, [=](){ ui->tableDepot->sortItems(1, Qt::AscendingOrder); });       // Emplacement

    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){ calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_stats); });
    connect(ui->btn_pdf, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_export_excel_planif, &QPushButton::clicked, [=](){ exporterCSV(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_print_planif, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning Production"); });
    connect(ui->btn_close_stats_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });

    // PRODUITS
    connect(ui->btn_cout_produit, &QPushButton::clicked, this, &MainWindow::showProduitCoutDialog);
    connect(ui->btn_hist_mode, &QPushButton::clicked, this, &MainWindow::showHistoriqueModeDialog);
    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){
        ouvrirStatsProduits();
    });
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_export_excel_prod, &QPushButton::clicked, [=](){ exporterCSV(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_print_prod, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_close_stats_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });

    // RH
    connect(ui->btn_open_eval, &QPushButton::clicked, this, &MainWindow::showEmpEvalDialog); // [NOUVEAU]
    connect(ui->btn_stats_emp, &QPushButton::clicked, [=](){
        ouvrirStatsRH();
    });
    connect(ui->btn_stats_emp_stats, &QPushButton::clicked, [=](){
        ouvrirStatsRH();
    });
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });

    // STOCK
    connect(ui->btn_open_compare, &QPushButton::clicked, this, &MainWindow::showCompareDialog); // [RESTITUÉ]
    connect(ui->btn_open_calcul, &QPushButton::clicked, this, &MainWindow::showBesoinDialog);   // [RESTITUÉ]
    connect(ui->btn_stats_stock, &QPushButton::clicked, [=](){
        ouvrirStatsStock();
    });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_export_excel_stock, &QPushButton::clicked, [=](){ exporterCSV(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_print_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_close_stats_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });

    // CLIENTS
    connect(ui->btn_open_client_ia, &QPushButton::clicked, this, &MainWindow::showClientIaDialog);
    connect(ui->btn_open_fidelite, &QPushButton::clicked, this, &MainWindow::showFideliteDialog);
    connect(ui->btn_stats_client, &QPushButton::clicked, [=](){
        ouvrirStatsClients();
    });
    connect(ui->btn_pdf_facture, &QPushButton::clicked, [=](){ exporterFactureClient(); });
    connect(ui->btn_export_excel_client, &QPushButton::clicked, [=](){ exporterCSV(ui->tableClients, "Clients"); });
    connect(ui->btn_print_client, &QPushButton::clicked, [=](){ exporterPDF(ui->tableClients, "Clients"); });
    connect(ui->btn_close_stats_client_dash, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_client_list); });

    // DEPOT
    connect(ui->btn_open_optimize, &QPushButton::clicked, this, &MainWindow::showOptimizeSpaceDialog);
    connect(ui->btn_open_ravit, &QPushButton::clicked, this, &MainWindow::showRavitaillementDialog);
    connect(ui->btn_stats_depot, &QPushButton::clicked, [=](){
        ouvrirStatsDepot();
    });
    connect(ui->btn_pdf_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_export_excel_depot, &QPushButton::clicked, [=](){ exporterCSV(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_print_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });
    connect(ui->btn_close_stats_depot_dash, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });

    // --- CRUD ---
    // Planif CRUD
    auto prepNewPlanif = [=](){ modeModification=false; ui->le_fin_prevue->clear(); ui->stackedWidget->setCurrentWidget(ui->page_planif_form); };
    // Remplacement du bouton AJOUTER
    connect(ui->btn_to_add_planif, &QPushButton::clicked, [=](){
        // Plus besoin de changer de page !
        // ui->stackedWidget->setCurrentWidget(ui->page_planif_form); // <--- On enlève ça

        ouvrirDialoguePlanif(false); // false = Mode Ajout
    });
    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        // 1. Récupération des champs
        QString prod = ui->cb_produit->currentText();
        int qte = ui->sb_qte->value();
        QString mat = ui->cb_matiere->currentText();
        QDate deb = ui->dt_lancement->date();

        // Gestion Date Fin
        QString dateFinStr = ui->le_fin_prevue->text();
        QDate fin = QDate::fromString(dateFinStr, "dd/MM/yyyy");
        if (!fin.isValid()) fin = deb.addDays(3); // Durée par défaut

        // 2. Création de l'objet métier avec les nouvelles valeurs
        // Note: Le statut reste "Planifié" ou on pourrait le récupérer de l'interface
        OrdreFabrication o(prod, qte, mat, deb, fin, "Planifié");

        bool success = false;

        // 3. Logique AJOUT ou MODIFICATION
        if (modeModification) {
            // Cas Modification : On utilise l'ID stocké lors du clic sur le crayon
            // indexModification contient l'index du tableau, on doit récupérer l'ID réel
            // ATTENTION : il faut récupérer l'ID "OF-105" et en extraire "105"
            QString idStr = mesCommandes[indexModification].id;
            int idReel = idStr.replace("OF-", "").toInt();

            success = o.modifier(idReel); // Appel de la fonction UPDATE

            if(success) QMessageBox::information(this, "Mise à jour", "Commande modifiée avec succès !");
        }
        else {
            // Cas Ajout
            success = o.ajouter(); // Appel de la fonction INSERT
            if(success) QMessageBox::information(this, "Succès", "Nouvelle commande créée !");
        }

        // 4. Rafraichissement si ça a marché
        if (success) {
            ui->stackedWidget->setCurrentWidget(ui->page_planif_list);
            rafraichirListeCommandes(); // Recharge tout depuis Oracle
            configurerTimelineGantt(); // Met à jour le graphique

            // On remet le mode à "Ajout" par défaut pour la prochaine fois
            modeModification = false;
        } else {
            QMessageBox::critical(this, "Erreur", "Opération échouée en base de données.");
        }
    });
    // --- BOUTON IA (ESTIMATION INTELLIGENTE) ---
    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        int qte = ui->sb_qte->value();

        if (qte <= 0) {
            QMessageBox::warning(this, "IA", "Veuillez entrer une quantité supérieure à 0.");
            return;
        }

        // 1. Récupération du type de produit pour adapter la vitesse
        QString typeProduit = ui->cb_produit->currentText();

        // Vitesse de production (pièces par jour)
        double piecesParJour = 50.0; // Vitesse standard

        if (typeProduit.contains("Sac")) {
            piecesParJour = 20.0; // Plus long à fabriquer
        } else if (typeProduit.contains("Portefeuille")) {
            piecesParJour = 100.0; // Très rapide
        }

        // 2. Calcul du temps (Arrondi au supérieur avec ceil)
        // Ex: 25 sacs / 20 par jour = 1.25 jours -> arrondi à 2 jours + 1 jour de prépa
        int joursProduction = std::ceil((double)qte / piecesParJour);
        int joursTotal = 1 + joursProduction; // +1 jour de préparation logistique

        // 3. Calcul de la date de fin (en évitant le Weekend si on veut pousser le réalisme, simple ici)
        QDate dateDeb = ui->dt_lancement->date();
        QDate dateFin = dateDeb.addDays(joursTotal);

        // 4. Affichage du résultat dans les champs
        ui->le_fin_prevue->setText(dateFin.toString("dd/MM/yyyy"));
        ui->le_duree_totale->setText(QString::number(joursTotal) + " Jours (Estimé)");

        // Petit effet visuel pour dire que ça a marché
        ui->le_fin_prevue->setStyleSheet("background-color: #dcedc8; color: #333; font-weight: bold;");
        QTimer::singleShot(1000, [=](){ ui->le_fin_prevue->setStyleSheet(""); }); // Remet la couleur normale après 1s
    });
    // Remplacement du bouton MODIFIER
    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int idx = ui->tablePlanif->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une ligne.");
            return;
        }

        // On sauvegarde l'index pour que la fonction sache quoi modifier
        indexModification = idx;

        ouvrirDialoguePlanif(true); // true = Mode Modification
    });
    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){
        int r = ui->tablePlanif->currentRow();
        if(r < 0) return;

        QTableWidgetItem *it = ui->tablePlanif->item(r, 0); // Colonne ID
        if(!it) return;

        int idToDelete = it->text().toInt(); // Récupère l'ID réel de la base

        OrdreFabrication o;
        if(o.supprimer(idToDelete)) {
            QMessageBox::information(this, "Succès", "Commande supprimée.");
            rafraichirListeCommandes();
            configurerTimelineGantt();
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer.");
        }
    });

    // Prod CRUD
    // Produits
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        ouvrirDialogueProduit(false);
    });

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
        int idx = ui->tableProduits->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Sélectionnez un produit à modifier.");
            return;
        }

        // On récupère l'index pour savoir quel produit modifier
        indexModifProd = ui->tableProduits->item(idx, 0)->data(Qt::UserRole).toInt();

        // ON OUVRE UNIQUEMENT LA POP-UP (On ne change plus de page !)
        ouvrirDialogueProduit(true);
    });
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableProduits);
        if(idx >= 0 && idx < mesProduits.size()) { mesProduits.removeAt(idx); rafraichirListeProduits(); }
    });

    // RH CRUD
    // Employés
    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){
        ouvrirDialogueEmploye(false);
    });
    connect(ui->btn_edit_emp, &QPushButton::clicked, [=](){
        int idx = ui->tableEmployes->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Sélectionnez un employé.");
            return;
        }
        indexModifEmp = ui->tableEmployes->item(idx, 0)->data(Qt::UserRole).toInt();

        // UNIQUEMENT LA POP-UP
        ouvrirDialogueEmploye(true);
    });
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

    connect(ui->btn_delete_emp, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableEmployes);
        if(idx >= 0 && idx < mesEmployes.size()) { mesEmployes.removeAt(idx); rafraichirListeEmployes(); }
    });
    connect(ui->btn_send_chat, &QPushButton::clicked, [=](){ reponseChatbot(); });

    // Stock CRUD
    // Stock
    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){
        ouvrirDialogueStock(false);
    });
    connect(ui->btn_edit_stock, &QPushButton::clicked, [=](){
        int idx = ui->tableStock->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Sélectionnez une matière.");
            return;
        }
        indexModifStock = ui->tableStock->item(idx, 0)->data(Qt::UserRole).toInt();

        // UNIQUEMENT LA POP-UP
        ouvrirDialogueStock(true);
    });
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
    connect(ui->btn_delete_stock, &QPushButton::clicked, [=](){
        int idx = selectedIndexFromTable(ui->tableStock);
        if(idx >= 0 && idx < mesMatieres.size()) { mesMatieres.removeAt(idx); rafraichirListeMatieres(); }
    });

    // Clients CRUD
    // Clients
    connect(ui->btn_add_client, &QPushButton::clicked, [=](){
        ouvrirDialogueClient(false);
    });
    connect(ui->btn_edit_client, &QPushButton::clicked, [=](){
        int idx = ui->tableClients->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Sélectionnez un client.");
            return;
        }
        indexModifClient = ui->tableClients->item(idx, 0)->data(Qt::UserRole).toInt();

        // UNIQUEMENT LA POP-UP
        ouvrirDialogueClient(true);
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
    // Dépôt
    connect(ui->btn_add_depot, &QPushButton::clicked, [=](){
        ouvrirDialogueDepot(false);
    });
    connect(ui->btn_edit_depot, &QPushButton::clicked, [=](){
        int idx = ui->tableDepot->currentRow();
        if(idx < 0) {
            QMessageBox::warning(this, "Sélection", "Sélectionnez un emplacement.");
            return;
        }
        indexModifDepot = ui->tableDepot->item(idx, 0)->data(Qt::UserRole).toInt();

        // UNIQUEMENT LA POP-UP
        ouvrirDialogueDepot(true);
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
    // --- NOUVELLE FONCTION : FORMULAIRE EN POP-UP ---
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
// --- NOUVELLE FONCTION : FORMULAIRE EN POP-UP ---
void MainWindow::ouvrirDialoguePlanif(bool estModification) {
    // 1. Création de la fenêtre (Dialog)
    QDialog *d = new QDialog(this);
    d->setWindowTitle(estModification ? "Modifier une Commande" : "Nouvelle Planification");
    d->setMinimumWidth(500);
    d->setStyleSheet("background-color: #f3f0eb; font-family: 'Segoe UI'; color: #3e2723;");

    QVBoxLayout *layout = new QVBoxLayout(d);

    // Titre
    QLabel *titre = new QLabel(estModification ? "MODIFICATION" : "NOUVEL ORDRE");
    titre->setStyleSheet("font-size: 18px; font-weight: bold; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    layout->addWidget(titre);

    // Formulaire
    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);

    QComboBox *cbProd = new QComboBox();
    cbProd->addItems({"Sac Voyage Cuir", "Portefeuille Luxe", "Ceinture Homme", "Sac à Main Femme"});
    cbProd->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    QSpinBox *sbQte = new QSpinBox();
    sbQte->setRange(1, 10000); sbQte->setValue(50);
    sbQte->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    QComboBox *cbMat = new QComboBox();
    cbMat->addItems({"Cuir Vachette", "Cuir Agneau", "Cuir Premium"});
    cbMat->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    QDateEdit *dtDeb = new QDateEdit(QDate::currentDate());
    dtDeb->setCalendarPopup(true);
    dtDeb->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    // Ligne pour la date de fin et l'IA
    QHBoxLayout *hl_fin = new QHBoxLayout();
    QLineEdit *leFin = new QLineEdit();
    leFin->setPlaceholderText("jj/MM/yyyy");
    leFin->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    QPushButton *btnIA = new QPushButton("⚡ IA");
    btnIA->setCursor(Qt::PointingHandCursor);
    btnIA->setToolTip("Calculer la date de fin estimée");
    btnIA->setStyleSheet("background-color: #7b1fa2; color: white; border-radius: 4px; padding: 8px 12px; font-weight: bold;");

    hl_fin->addWidget(leFin);
    hl_fin->addWidget(btnIA);

    // Ajout au formulaire
    form->addRow("Produit :", cbProd);
    form->addRow("Quantité :", sbQte);
    form->addRow("Matière :", cbMat);
    form->addRow("Début :", dtDeb);
    form->addRow("Fin (Est.) :", hl_fin);

    layout->addLayout(form);

    // --- LOGIQUE IA (INTEGRÉE) ---
    connect(btnIA, &QPushButton::clicked, [=](){
        double vitesse = cbProd->currentText().contains("Sac") ? 20.0 : 50.0;
        int jours = std::ceil(sbQte->value() / vitesse) + 1;
        QDate fin = dtDeb->date().addDays(jours);
        leFin->setText(fin.toString("dd/MM/yyyy"));
        leFin->setStyleSheet("background-color: #dcedc8; padding: 8px; border: 1px solid #8bc34a; border-radius: 4px;");
    });

    // Pré-remplissage si modification
    int idToEdit = -1;
    if(estModification && indexModification >= 0 && indexModification < mesCommandes.size()) {
        CommandeInfo c = mesCommandes[indexModification];
        // Extraction de l'ID réel (enlever "OF-")
        idToEdit = c.id.replace("OF-", "").toInt();

        cbProd->setCurrentText(c.produit);
        sbQte->setValue(c.quantite);
        // Note: Pour la matière, idéalement il faudrait matcher le texte avec l'ID, ici on simplifie
        dtDeb->setDate(c.dateDebut);
        leFin->setText(c.dateFinEstimee);
    }

    // Boutons Action
    QHBoxLayout *hl_btns = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler");
    btnCancel->setStyleSheet("background-color: #b71c1c; color: white; padding: 10px; border-radius: 5px; font-weight: bold;");

    QPushButton *btnSave = new QPushButton(estModification ? "Modifier" : "Ajouter");
    btnSave->setStyleSheet("background-color: #8d5524; color: white; padding: 10px; border-radius: 5px; font-weight: bold;");

    hl_btns->addStretch();
    hl_btns->addWidget(btnCancel);
    hl_btns->addWidget(btnSave);
    layout->addSpacing(10);
    layout->addLayout(hl_btns);

    // --- LOGIQUE SAUVEGARDE ---
    connect(btnCancel, &QPushButton::clicked, d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [=](){
        // 1. Récupération
        QString p = cbProd->currentText();
        int q = sbQte->value();
        QString m = cbMat->currentText(); // Texte envoyé (ID forcé à 1 dans la classe pour l'instant)
        QDate d1 = dtDeb->date();
        QDate d2 = QDate::fromString(leFin->text(), "dd/MM/yyyy");
        if(!d2.isValid()) d2 = d1.addDays(3);

        // 2. Appel SQL
        OrdreFabrication o(p, q, m, d1, d2, "Planifié");
        bool ok = false;

        if(estModification) {
            ok = o.modifier(idToEdit);
        } else {
            ok = o.ajouter();
        }

        if(ok) {
            QMessageBox::information(d, "Succès", estModification ? "Commande modifiée !" : "Commande ajoutée !");
            rafraichirListeCommandes(); // Mise à jour du tableau derrière
            configurerTimelineGantt();
            d->accept(); // Ferme la fenêtre
        } else {
            QMessageBox::critical(d, "Erreur", "Opération échouée en base de données.");
        }
    });

    // Affichage Modale (Bloque le reste tant que pas fermé)
    d->exec();

    // Nettoyage mémoire auto à la fin de la fonction
    delete d;
}

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
    // 1. Récupérer les données de la base
    QSqlQueryModel *model = tmpOrdre.afficher();

    // 2. Vider la liste locale et le tableau
    mesCommandes.clear();
    ui->tablePlanif->setRowCount(0);

    // 3. Configurer le tableau
    ui->tablePlanif->setColumnCount(7);
    ui->tablePlanif->setHorizontalHeaderLabels({"ID", "Produit", "Qté", "Matière", "Début", "Fin", "Statut"});

    int rows = model->rowCount();
    ui->tablePlanif->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        // --- CORRECTION DES NOMS DE COLONNES ICI ---
        QString idStr = model->record(i).value("ID_COMMANDE").toString();

        // Nom corrigé : REF_PRODUIT au lieu de ID_PRODUIT
        QString prod = model->record(i).value("REF_PRODUIT").toString();

        int qte = model->record(i).value("QUANTITE").toInt();

        // On lit maintenant le CODE (texte) récupéré grâce au JOIN
        QString mat = model->record(i).value("CODE_MP").toString();

        // Si jamais le code est vide (ex: matière supprimée), on met un texte par défaut
        if(mat.isEmpty()) mat = "Inconnu/Supprimé";

        QDate deb = model->record(i).value("DATE_LANCEMENT").toDate();
        QDate fin = model->record(i).value("DATE_FIN_PREVUE").toDate();
        QString stat = model->record(i).value("STATUT").toString();
        QString finStr = fin.toString("dd/MM/yyyy");

        // Remplissage du tableau graphique
        ui->tablePlanif->setItem(i, 0, new QTableWidgetItem(idStr));
        ui->tablePlanif->setItem(i, 1, new QTableWidgetItem(prod));
        ui->tablePlanif->setItem(i, 2, new QTableWidgetItem(QString::number(qte)));
        ui->tablePlanif->setItem(i, 3, new QTableWidgetItem(mat)); // Affiche "1" pour l'instant (l'ID)
        ui->tablePlanif->setItem(i, 4, new QTableWidgetItem(deb.toString("dd/MM/yyyy")));
        ui->tablePlanif->setItem(i, 5, new QTableWidgetItem(finStr));
        ui->tablePlanif->setItem(i, 6, new QTableWidgetItem(stat));

        // Mise à jour de la liste locale pour le Gantt
        CommandeInfo c;
        c.id = "OF-" + idStr;
        c.produit = prod;
        c.quantite = qte;
        c.matiere = mat;
        c.dateDebut = deb;
        c.dateFinEstimee = finStr;
        c.statut = stat;
        c.etatEtape = 0;

        mesCommandes.append(c);
    }

    delete model;
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
// =========================================================
// ===    FORMULAIRES POP-UP AVEC DESIGN "FIL D'OR"    ===
// =========================================================

// Style commun pour éviter la répétition
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

// --- 1. MODULE PRODUITS ---
void MainWindow::ouvrirDialogueProduit(bool estModif) {
    QDialog d(this);
    d.setWindowTitle(estModif ? "Modifier Produit" : "Nouveau Produit");
    d.setMinimumWidth(450);
    d.setStyleSheet(stylePopup());

    QVBoxLayout *l = new QVBoxLayout(&d);

    // Titre
    QLabel *titre = new QLabel(estModif ? "ÉDITION PRODUIT" : "CRÉATION PRODUIT");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QFormLayout *f = new QFormLayout();
    f->setSpacing(15);

    QLineEdit *leRef = new QLineEdit(); leRef->setPlaceholderText("Ex: SAC-H-2026");
    QLineEdit *leNom = new QLineEdit();
    QDoubleSpinBox *sbCout = new QDoubleSpinBox(); sbCout->setMaximum(10000);
    QComboBox *cbColl = new QComboBox(); cbColl->addItems({"Hiver 2026", "Étè 2026", "Intemporel"});
    QComboBox *cbCuir = new QComboBox(); cbCuir->addItems({"Vachette", "Agneau", "Croco", "Exotique"});
    QSpinBox *sbTemps = new QSpinBox();

    f->addRow("Référence :", leRef);
    f->addRow("Désignation :", leNom);
    f->addRow("Coût Matière (€) :", sbCout);
    f->addRow("Collection :", cbColl);
    f->addRow("Type Cuir :", cbCuir);
    f->addRow("Temps Fab. (h) :", sbTemps);
    l->addLayout(f);

    if(estModif && indexModifProd >= 0 && indexModifProd < mesProduits.size()) {
        const auto &p = mesProduits[indexModifProd];
        leRef->setText(p.ref); leNom->setText(p.nom); sbCout->setValue(p.coutMatiere);
        cbColl->setCurrentText(p.collection); cbCuir->setCurrentText(p.cuir); sbTemps->setValue(p.temps);
    }

    // Boutons Custom
    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton(estModif ? "Modifier" : "Ajouter"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave);
    l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leRef->text().isEmpty() || leNom->text().isEmpty()) {
            QMessageBox::warning(&d, "Erreur", "Référence et Nom sont obligatoires.");
            return;
        }
        ProduitInfo p = {leRef->text(), leNom->text(), sbCout->value(), cbColl->currentText(), cbCuir->currentText(), sbTemps->value()};

        if(estModif) mesProduits[indexModifProd] = p;
        else mesProduits.append(p);

        rafraichirListeProduits();
        d.accept();
    });
    d.exec();
}

// --- 2. MODULE EMPLOYÉS (RH) ---
void MainWindow::ouvrirDialogueEmploye(bool estModif) {
    QDialog d(this);
    d.setWindowTitle(estModif ? "Modifier Employé" : "Recrutement");
    d.setMinimumWidth(450);
    d.setStyleSheet(stylePopup());

    QVBoxLayout *l = new QVBoxLayout(&d);

    QLabel *titre = new QLabel(estModif ? "DOSSIER EMPLOYÉ" : "RECRUTEMENT");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QFormLayout *f = new QFormLayout();
    f->setSpacing(15);

    QLineEdit *leId = new QLineEdit();
    QLineEdit *leNom = new QLineEdit();
    QLineEdit *lePre = new QLineEdit();
    QComboBox *cbPoste = new QComboBox(); cbPoste->addItems({"Artisan Maroquinier", "Coupeur", "Chef Atelier", "Contrôle Qualité", "Designer"});
    QComboBox *cbDept = new QComboBox(); cbDept->addItems({"Production", "Logistique", "Bureau Études", "Commercial"});
    QDateEdit *dtEmb = new QDateEdit(QDate::currentDate()); dtEmb->setCalendarPopup(true);
    QDoubleSpinBox *sbSal = new QDoubleSpinBox(); sbSal->setMaximum(100000); sbSal->setSingleStep(50);
    QLineEdit *leRfid = new QLineEdit(); leRfid->setPlaceholderText("Scan Badge...");

    f->addRow("ID Unique :", leId);
    f->addRow("Nom :", leNom);
    f->addRow("Prénom :", lePre);
    f->addRow("Poste :", cbPoste);
    f->addRow("Département :", cbDept);
    f->addRow("Date Embauche :", dtEmb);
    f->addRow("Salaire (€) :", sbSal);
    f->addRow("Badge RFID :", leRfid);
    l->addLayout(f);

    if(estModif && indexModifEmp >= 0 && indexModifEmp < mesEmployes.size()) {
        const auto &e = mesEmployes[indexModifEmp];
        leId->setText(e.id); leNom->setText(e.nom); lePre->setText(e.prenom);
        cbPoste->setCurrentText(e.poste); cbDept->setCurrentText(e.departement);
        dtEmb->setDate(e.dateEmbauche); sbSal->setValue(e.salaire); leRfid->setText(e.rfid);
    }

    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton(estModif ? "Enregistrer" : "Recruter"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave);
    l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leId->text().isEmpty() || leNom->text().isEmpty()) return;
        EmployeInfo e = {leId->text(), leNom->text(), lePre->text(), cbPoste->currentText(), cbDept->currentText(), dtEmb->date(), sbSal->value(), leRfid->text()};

        if(estModif) mesEmployes[indexModifEmp] = e;
        else mesEmployes.append(e);

        rafraichirListeEmployes();
        d.accept();
    });
    d.exec();
}

// --- 3. MODULE STOCK ---
void MainWindow::ouvrirDialogueStock(bool estModif) {
    QDialog d(this);
    d.setWindowTitle(estModif ? "Modifier Matière" : "Nouvelle Matière");
    d.setMinimumWidth(450);
    d.setStyleSheet(stylePopup());

    QVBoxLayout *l = new QVBoxLayout(&d);

    QLabel *titre = new QLabel(estModif ? "MODIFIER STOCK" : "AJOUT STOCK");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QFormLayout *f = new QFormLayout();
    f->setSpacing(15);

    QLineEdit *leCode = new QLineEdit();
    QComboBox *cbCat = new QComboBox(); cbCat->addItems({"Cuir", "Teinture", "Fil", "Accessoire Métal", "Produit Chimique"});
    QComboBox *cbEtat = new QComboBox(); cbEtat->addItems({"BRUT", "TEINT", "TRAITÉ", "FINI"});
    QLineEdit *leCoul = new QLineEdit();
    QComboBox *cbQual = new QComboBox(); cbQual->addItems({"A (Premium)", "B (Standard)", "C (Eco)"});
    QDoubleSpinBox *sbQte = new QDoubleSpinBox(); sbQte->setMaximum(100000);
    QComboBox *cbUnite = new QComboBox(); cbUnite->addItems({"M2", "Kg", "Litre", "Pièce"});
    QComboBox *cbZone = new QComboBox(); cbZone->addItems({"Zone A", "Zone B", "Zone C", "Zone Frigo"});

    f->addRow("Code Matière :", leCode);
    f->addRow("Catégorie :", cbCat);
    f->addRow("État :", cbEtat);
    f->addRow("Couleur :", leCoul);
    f->addRow("Qualité :", cbQual);
    f->addRow("Quantité :", sbQte);
    f->addRow("Unité :", cbUnite);
    f->addRow("Zone Stockage :", cbZone);
    l->addLayout(f);

    if(estModif && indexModifStock >= 0 && indexModifStock < mesMatieres.size()) {
        const auto &m = mesMatieres[indexModifStock];
        leCode->setText(m.code); cbCat->setCurrentText(m.categorie); cbEtat->setCurrentText(m.etat);
        leCoul->setText(m.couleur); cbQual->setCurrentText(m.qualite); sbQte->setValue(m.quantite);
        cbUnite->setCurrentText(m.unite); cbZone->setCurrentText(m.zone);
    }

    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton("Valider"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave);
    l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leCode->text().isEmpty()) return;
        MatiereInfo m = {leCode->text(), cbCat->currentText(), cbEtat->currentText(), leCoul->text(), cbQual->currentText(), sbQte->value(), cbUnite->currentText(), cbZone->currentText(), "", "Standard", QDate::currentDate()};

        if(estModif) mesMatieres[indexModifStock] = m;
        else mesMatieres.append(m);

        rafraichirListeMatieres();
        d.accept();
    });
    d.exec();
}

// --- 4. MODULE CLIENTS ---
void MainWindow::ouvrirDialogueClient(bool estModif) {
    QDialog d(this);
    d.setWindowTitle(estModif ? "Fiche Client" : "Nouveau Client");
    d.setMinimumWidth(450);
    d.setStyleSheet(stylePopup());

    QVBoxLayout *l = new QVBoxLayout(&d);

    QLabel *titre = new QLabel(estModif ? "MODIFIER CLIENT" : "NOUVEAU CLIENT");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QFormLayout *f = new QFormLayout();
    f->setSpacing(15);

    QLineEdit *leId = new QLineEdit();
    QLineEdit *leNom = new QLineEdit();
    QLineEdit *leTel = new QLineEdit();
    QLineEdit *leAdr = new QLineEdit();
    QLineEdit *leMail = new QLineEdit();
    QSpinBox *sbPts = new QSpinBox(); sbPts->setMaximum(100000);

    f->addRow("Identifiant :", leId);
    f->addRow("Nom Complet :", leNom);
    f->addRow("Téléphone :", leTel);
    f->addRow("Adresse :", leAdr);
    f->addRow("Email :", leMail);
    f->addRow("Points Fidélité :", sbPts);
    l->addLayout(f);

    if(estModif && indexModifClient >= 0 && indexModifClient < mesClients.size()) {
        const auto &c = mesClients[indexModifClient];
        leId->setText(c.id); leNom->setText(c.nom); leTel->setText(c.telephone);
        leAdr->setText(c.adresse); leMail->setText(c.email); sbPts->setValue(c.pointsFidelite);
    }

    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton("Sauvegarder"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave);
    l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leId->text().isEmpty() || leNom->text().isEmpty()) return;
        ClientInfo c = {leId->text(), leNom->text(), leTel->text(), leAdr->text(), leMail->text(), sbPts->value()};

        if(estModif) mesClients[indexModifClient] = c;
        else mesClients.append(c);

        rafraichirListeClients();
        d.accept();
    });
    d.exec();
}

// --- 5. MODULE DÉPÔT ---
void MainWindow::ouvrirDialogueDepot(bool estModif) {
    QDialog d(this);
    d.setWindowTitle(estModif ? "Gestion Emplacement" : "Nouvel Emplacement");
    d.setMinimumWidth(450);
    d.setStyleSheet(stylePopup());

    QVBoxLayout *l = new QVBoxLayout(&d);

    QLabel *titre = new QLabel(estModif ? "DÉTAIL EMPLACEMENT" : "AJOUT EMPLACEMENT");
    titre->setStyleSheet("font-size: 18px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    QFormLayout *f = new QFormLayout();
    f->setSpacing(15);

    QLineEdit *leId = new QLineEdit();
    QLineEdit *leEmp = new QLineEdit(); leEmp->setPlaceholderText("Ex: Zone A - Allée 3");
    QLineEdit *leEta = new QLineEdit();
    QDoubleSpinBox *sbCap = new QDoubleSpinBox(); sbCap->setMaximum(100000);
    QDoubleSpinBox *sbAct = new QDoubleSpinBox(); sbAct->setMaximum(100000);
    QComboBox *cbType = new QComboBox(); cbType->addItems({"Sec", "Froid", "Sécurisé"});

    f->addRow("ID Emplacement :", leId);
    f->addRow("Zone / Allée :", leEmp);
    f->addRow("Étagère :", leEta);
    f->addRow("Capacité Max :", sbCap);
    f->addRow("Quantité Actuelle :", sbAct);
    f->addRow("Type de Stockage :", cbType);
    l->addLayout(f);

    if(estModif && indexModifDepot >= 0 && indexModifDepot < mesDepots.size()) {
        const auto &dp = mesDepots[indexModifDepot];
        leId->setText(dp.id); leEmp->setText(dp.emplacement); leEta->setText(dp.etagere);
        sbCap->setValue(dp.capaciteMax); sbAct->setValue(dp.quantiteActuelle); cbType->setCurrentText(dp.typeStockage);
    }

    QHBoxLayout *hl = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("Annuler"); btnCancel->setStyleSheet(styleBtnCancel());
    QPushButton *btnSave = new QPushButton("Confirmer"); btnSave->setStyleSheet(styleBtnSave());
    hl->addStretch(); hl->addWidget(btnCancel); hl->addWidget(btnSave);
    l->addSpacing(10); l->addLayout(hl);

    connect(btnCancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leId->text().isEmpty()) return;
        DepotInfo dp = {leId->text(), leEmp->text(), leEta->text(), sbCap->value(), sbAct->value(), cbType->currentText()};

        if(estModif) mesDepots[indexModifDepot] = dp;
        else mesDepots.append(dp);

        rafraichirListeDepots();
        d.accept();
    });
    d.exec();
}
// --------------------------- FABRICATION (TIMELINE GANTT) ---------------------------
void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline;
    t->clear();
    t->setRowCount(0);

    int jours = 31; // On affiche un mois complet
    t->setColumnCount(1 + jours);

    QStringList headers;
    headers << "PRODUIT";

    QDate today = QDate::currentDate();

    for(int i = 0; i < jours; i++) {
        headers << today.addDays(i).toString("dd");
    }

    t->setHorizontalHeaderLabels(headers);
    t->setColumnWidth(0, 200);
    for(int i = 1; i <= jours; i++) {
        t->setColumnWidth(i, 35);
    }

    t->setRowCount(mesCommandes.size());
    for(int i = 0; i < mesCommandes.size(); i++) {
        t->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id));
        t->setRowHeight(i, 50);

        int offset = today.daysTo(mesCommandes[i].dateDebut);
        if(offset < 0) offset = 0; // Si c'est commencé avant aujourd'hui, on commence à la col 1

        // --- CALCUL DYNAMIQUE ---
        // 1. On récupère la date de fin réelle stockée en base
        QDate dateFin = QDate::fromString(mesCommandes[i].dateFinEstimee, "dd/MM/yyyy");

        // 2. On calcule la durée totale disponible (en jours)
        int dureeTotale = mesCommandes[i].dateDebut.daysTo(dateFin);
        if (dureeTotale < 1) dureeTotale = 1; // Minimum 1 jour

        // 3. On répartit cette durée proportionnellement sur les 4 étapes
        // Coupe (20%), Assemblage (30%), Couture (30%), Finition (reste)
        int d1 = std::max(1, (int)(dureeTotale * 0.2));
        int d2 = std::max(1, (int)(dureeTotale * 0.3));
        int d3 = std::max(1, (int)(dureeTotale * 0.3));
        int d4 = std::max(1, dureeTotale - d1 - d2 - d3); // Le reste pour tomber juste

        // Couleurs
        QColor c1(41,128,185), c2(211,84,0), c3(39,174,96), c4(241,196,15);
        if(mesCommandes[i].etatEtape == 2) { QColor r(192,57,43); c1=c2=c3=c4=r; } // Rouge si retard global
        else if(mesCommandes[i].etatEtape == 1) { QColor v(46,204,113); c1=c2=c3=c4=v; } // Vert si OK global

        // 4. Dessin des barres (Les unes à la suite des autres)
        int currentPos = offset + 1; // +1 car la colonne 0 est le nom

        if(currentPos < jours) dessinerBarre(i, currentPos, d1, "Cp", c1, Qt::white);
        currentPos += d1;

        if(currentPos < jours) dessinerBarre(i, currentPos, d2, "As", c2, Qt::white);
        currentPos += d2;

        if(currentPos < jours) dessinerBarre(i, currentPos, d3, "Co", c3, Qt::white);
        currentPos += d3;

        if(currentPos < jours) dessinerBarre(i, currentPos, d4, "Fi", c4, Qt::black);
    }
}
// =========================================================
// ===      TABLEAUX DE BORD (STATS) EN POP-UP           ===
// =========================================================

// Utilitaire pour créer une jolie carte (KPI)
QFrame* MainWindow::creerCarteStat(QString icone, QString val, QString titre, QString couleurFond) {
    QFrame *f = new QFrame();
    f->setStyleSheet(QString("QFrame { background: %1; border-radius: 10px; }").arg(couleurFond));
    f->setMinimumHeight(100);
    QVBoxLayout *l = new QVBoxLayout(f);

    QLabel *l_ico = new QLabel(icone); l_ico->setStyleSheet("font-size: 24px; border:none; color: white;");
    QLabel *l_val = new QLabel(val); l_val->setStyleSheet("font-size: 28px; font-weight:900; color: white; border:none;");
    QLabel *l_tit = new QLabel(titre); l_tit->setStyleSheet("font-size: 13px; font-weight:bold; color: #f0f0f0; border:none;");

    l_ico->setAlignment(Qt::AlignRight);
    l_val->setAlignment(Qt::AlignCenter);
    l_tit->setAlignment(Qt::AlignCenter);

    l->addWidget(l_ico);
    l->addWidget(l_val);
    l->addWidget(l_tit);
    return f;
}

// --- 1. STATS PRODUITS ---
void MainWindow::ouvrirStatsProduits() {
    QDialog d(this);
    d.setWindowTitle("Statistiques Catalogue");
    d.setMinimumSize(800, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");

    QVBoxLayout *mainL = new QVBoxLayout(&d);

    // Titre
    QLabel *t = new QLabel("DASHBOARD PRODUITS");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    // Calculs
    int total = mesProduits.size();
    double prixTotal = 0;
    QMap<QString, double> parCollection;
    QMap<QString, double> parCuir;

    for(const auto &p : mesProduits) {
        prixTotal += p.coutMatiere;
        parCollection[p.collection] += 1;
        parCuir[p.cuir] += 1;
    }
    double moy = (total > 0) ? prixTotal / total : 0;

    // Cartes KPI
    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("👜", QString::number(total), "Références", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #8d5524)"));
    kpiL->addWidget(creerCarteStat("💰", QString::number(moy, 'f', 1) + " €", "Coût Moyen", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2c3e50, stop:1 #4ca1af)"));
    mainL->addLayout(kpiL);

    // Graphiques
    QHBoxLayout *chartsL = new QHBoxLayout();

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slicesColl;
    for(auto k : parCollection.keys()) slicesColl.append({k, parCollection[k]});
    setPieChart(wPie, "Collections", slicesColl);

    QWidget *wBar = new QWidget();
    QStringList cats = parCuir.keys();
    QList<double> vals; for(auto k : cats) vals << parCuir[k];
    setVerticalBarChart(wBar, "Types de Cuir", cats, vals);

    chartsL->addWidget(wPie);
    chartsL->addWidget(wBar);
    mainL->addLayout(chartsL);

    // Bouton Fermer
    QPushButton *btn = new QPushButton("Fermer");
    btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    mainL->addWidget(btn, 0, Qt::AlignCenter);

    d.exec();
}

// --- 2. STATS RH ---
void MainWindow::ouvrirStatsRH() {
    QDialog d(this);
    d.setWindowTitle("Statistiques RH"); d.setMinimumSize(800, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("ANALYSE RESSOURCES HUMAINES");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    int total = mesEmployes.size();
    double masseSal = 0;
    QMap<QString, double> parDept;

    for(const auto &e : mesEmployes) {
        masseSal += e.salaire;
        parDept[e.departement] += 1;
    }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("👥", QString::number(total), "Employés", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)"));
    kpiL->addWidget(creerCarteStat("💸", QString::number(masseSal) + " €", "Masse Salariale", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FF5F6D, stop:1 #FFC371)"));
    mainL->addLayout(kpiL);

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for(auto k : parDept.keys()) slices.append({k, parDept[k]});
    setPieChart(wPie, "Départements", slices);
    mainL->addWidget(wPie);

    QPushButton *btn = new QPushButton("Fermer"); btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    mainL->addWidget(btn, 0, Qt::AlignCenter);
    d.exec();
}

// --- 3. STATS STOCK ---
void MainWindow::ouvrirStatsStock() {
    QDialog d(this);
    d.setWindowTitle("Statistiques Stock"); d.setMinimumSize(800, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("ÉTAT DU STOCK");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    double volume = 0;
    QMap<QString, double> parCat;
    QMap<QString, double> parQual;

    for(const auto &m : mesMatieres) {
        volume += m.quantite;
        parCat[m.categorie] += 1;
        parQual[m.qualite] += 1;
    }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("📦", QString::number(mesMatieres.size()), "Références MP", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)"));
    kpiL->addWidget(creerCarteStat("📏", QString::number(volume) + " U", "Volume Total", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e1eec3, stop:1 #f05053)"));
    mainL->addLayout(kpiL);

    QHBoxLayout *chartsL = new QHBoxLayout();
    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for(auto k : parCat.keys()) slices.append({k, parCat[k]});
    setPieChart(wPie, "Catégories", slices);

    QWidget *wBar = new QWidget();
    QStringList cats = parQual.keys();
    QList<double> vals; for(auto k : cats) vals << parQual[k];
    setVerticalBarChart(wBar, "Qualité", cats, vals);

    chartsL->addWidget(wPie); chartsL->addWidget(wBar);
    mainL->addLayout(chartsL);

    QPushButton *btn = new QPushButton("Fermer"); btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    mainL->addWidget(btn, 0, Qt::AlignCenter);
    d.exec();
}

// --- 4. STATS CLIENTS ---
void MainWindow::ouvrirStatsClients() {
    QDialog d(this);
    d.setWindowTitle("Statistiques Clients"); d.setMinimumSize(800, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("INTELLIGENCE CLIENTS");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    int vip = 0;
    QMap<QString, double> parVille;
    for(const auto &c : mesClients) {
        if(c.pointsFidelite >= 100) vip++;
        // Extraction simplifiée de la ville (basé sur le texte adresse)
        QString ville = "Autre";
        if(c.adresse.toLower().contains("tunis")) ville = "Tunis";
        else if(c.adresse.toLower().contains("sfax")) ville = "Sfax";
        else if(c.adresse.toLower().contains("sousse")) ville = "Sousse";
        parVille[ville] += 1;
    }

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("🧾", QString::number(mesClients.size()), "Clients Total", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));
    kpiL->addWidget(creerCarteStat("👑", QString::number(vip), "Clients VIP", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #f7f1e3)"));
    mainL->addLayout(kpiL);

    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for(auto k : parVille.keys()) slices.append({k, parVille[k]});
    setPieChart(wPie, "Répartition Géographique", slices);
    mainL->addWidget(wPie);

    QPushButton *btn = new QPushButton("Fermer"); btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    mainL->addWidget(btn, 0, Qt::AlignCenter);
    d.exec();
}

// --- 5. STATS DÉPÔT ---
void MainWindow::ouvrirStatsDepot() {
    QDialog d(this);
    d.setWindowTitle("Statistiques Dépôt"); d.setMinimumSize(800, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("CAPACITÉ & STOCKAGE");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    double totalCap = 0;
    double currentLoad = 0;
    QMap<QString, double> parType;

    for(const auto &dp : mesDepots) {
        totalCap += dp.capaciteMax;
        currentLoad += dp.quantiteActuelle;
        parType[dp.typeStockage] += 1;
    }
    double taux = (totalCap > 0) ? (currentLoad/totalCap)*100.0 : 0;

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("🏭", QString::number(taux, 'f', 1) + "%", "Taux Remplissage", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EB3349, stop:1 #F45C43)"));
    kpiL->addWidget(creerCarteStat("🧊", QString::number(mesDepots.size()), "Emplacements", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #56ccf2, stop:1 #2f80ed)"));
    mainL->addLayout(kpiL);

    QWidget *wBar = new QWidget();
    QStringList types = parType.keys();
    QList<double> vals; for(auto k : types) vals << parType[k];
    setVerticalBarChart(wBar, "Types d'emplacement", types, vals);
    mainL->addWidget(wBar);

    QPushButton *btn = new QPushButton("Fermer"); btn->setStyleSheet(styleBtnCancel());
    connect(btn, &QPushButton::clicked, &d, &QDialog::accept);
    mainL->addWidget(btn, 0, Qt::AlignCenter);
    d.exec();
}
void MainWindow::dessinerBarre(int r, int c, int d, QString t, QColor b, QColor f) {
    if(c+d>30) return; QTableWidgetItem *it=new QTableWidgetItem(t); it->setData(Qt::BackgroundRole,b); it->setForeground(f);
    ui->tableTimeline->setItem(r,c,it); ui->tableTimeline->setSpan(r,c,1,d);
}
