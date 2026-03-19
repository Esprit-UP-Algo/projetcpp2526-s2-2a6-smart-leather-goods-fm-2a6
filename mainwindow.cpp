#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "employe.h"

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
#include <algorithm>
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
#include <QDialogButtonBox>

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
#include <QTimer>


// =========================================================
// ===           STYLES GRAPHIQUES (UI)                  ===
// =========================================================

static QList<QColor> chartPaletteFILdOr() {
    return { QColor("#d4af37"), QColor("#3e2723"), QColor("#e0c097"), QColor("#8d5524"), QColor("#b87333"), QColor("#a1887f") };
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

// =========================================================
// ===             FONCTIONS CHART HELPER                ===
// =========================================================

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

// =========================================================
// ===                  MAIN WINDOW                      ===
// =========================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableEmployes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableEmployes->setSelectionMode(QAbstractItemView::SingleSelection);

    // ✅ اصلاح زر Modifier لوتا
   /* QObject::disconnect(ui->btn_edit_emp, nullptr, nullptr, nullptr);

    connect(ui->btn_edit_emp, &QPushButton::clicked,
             this, &MainWindow::on_btn_edit_emp_clicked, Qt::UniqueConnection);*/


    // DEBUG باش نعرف اسم زر Modifier لتحت
    qDebug() << "=== BUTTONS LIST ===";
    for (auto b : this->findChildren<QPushButton*>())
        qDebug() << b->objectName() << "text=" << b->text();


    QObject::disconnect(ui->btn_edit_emp, nullptr, nullptr, nullptr);
    connect(ui->btn_edit_emp, &QPushButton::clicked,
            this, &MainWindow::on_btn_edit_emp_clicked);
    ui->tableEmployes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableEmployes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableEmployes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // ==== REMPLIR COMBOBOX EMPLOYE ====
    // ✅ Remplir combobox Modifier avec toutes les options
    ui->cb_emp_poste_modif->clear();
    ui->cb_emp_poste_modif->addItems(QStringList()
                                     << "Coupe" << "Assemblage" << "Couture" << "Finition"
                                     << "Contrôle Qualité" << "Magasinier" << "Chef Atelier");

    ui->cb_emp_dept_modif->clear();
    ui->cb_emp_dept_modif->addItems(QStringList()
                                    << "Stock" << "Production" << "RH");
    ui->cb_emp_poste->clear();
    ui->cb_emp_poste->addItems({
        "Coupe",
        "Assemblage",
        "Couture",
        "Finition",
        "Contrôle Qualité",
        "Magasinier",
        "Chef Atelier"
    });

    ui->cb_emp_dept->clear();
    ui->cb_emp_dept->addItems({
        "Production",
        "Qualité",
        "Stock",
        "Logistique",
        "Administration",
        "RH"
    });

    // --- CONNEXION ORACLE (Singleton) ---
    Connexion *cnx = Connexion::getInstance();
    bool test = cnx->estConnecte();
    if (test) {
        qDebug() << "✅ Base Oracle connectée via Singleton";
    } else {
        qDebug() << "❌ Pas de connexion Oracle";
        alerteWarning("Erreur BDD", "Impossible de se connecter à la base Oracle.");
    }

    // Enable sorting
    ui->tablePlanif->setSortingEnabled(true);
    ui->tableProduits->setSortingEnabled(true);
    ui->tableEmployes->setSortingEnabled(true);
    ui->tableStock->setSortingEnabled(true);
    ui->tableClients->setSortingEnabled(true);
    ui->tableDepot->setSortingEnabled(true);

    // --- DONNEES LOCALES TEMPORAIRES ---


    myColorDelegate = new ColorDelegate(this);
    ui->tableTimeline->setItemDelegate(myColorDelegate);

    // Initialisation affichage (uniquement si la BDD est connectée)
    if (cnx->estConnecte()) {
        rafraichirListeCommandes();
        configurerTimelineGantt();
        rafraichirListeProduits();
        rafraichirListeEmployes();
        rafraichirListeMatieres();
        rafraichirListeClients();
        rafraichirListeDepots();
    } else {
        // BDD inaccessible : interface démarrera en mode restreint (données locales ou vides)
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

    // --- NAVIGATION PRINCIPALE ---
    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){ construireDashboardAccueil(); ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){ configurerTimelineGantt(); ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){ rafraichirListeProduits(); ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_nav_rh, &QPushButton::clicked, [=](){ rafraichirListeEmployes(); ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_nav_stock, &QPushButton::clicked, [=](){ rafraichirListeMatieres(); ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_nav_clients, &QPushButton::clicked, [=](){ rafraichirListeClients(); ui->stackedWidget->setCurrentWidget(ui->page_client_list); });
    connect(ui->btn_nav_depot, &QPushButton::clicked, [=](){ rafraichirListeDepots(); ui->stackedWidget->setCurrentWidget(ui->page_depot_list); });

    // --- LOGIN ---
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

        // --- EXCEPTION ADMIN (BACKDOOR) ---
        if(nom == "admin" && prenom == "admin" && mdp_rfid == "1234") {
            ui->le_login_mdp->clear();
            setNavigationEnabled(true);
            rafraichirListeCommandes();
            calculerEtAfficherStats();
            construireDashboardAccueil();
            ui->stackedWidget->setCurrentWidget(ui->page_home);
            return; // On arrête la fonction ici, pas besoin d'interroger Oracle
        }

        // --- REQUÊTE DE SÉCURITÉ (POUR LES AUTRES EMPLOYÉS) ---
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
    // =========================================================
    // === BOUTONS D'ACTION (POPUPS, CRUD, STATS)            ===
    // =========================================================

    // --- 1. PLANIFICATION (Navigation par onglets SPA) ---

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

    // --- STYLES DES NOUVEAUX BOUTONS ---
    ui->btn_valider_modif->setStyleSheet(styleBtnSave()); // Met le bouton "Modifier" en marron FIL D'OR
    ui->btn_valider_modif->setCursor(Qt::PointingHandCursor);

    ui->btn_ia_appliquer->setStyleSheet("background-color: #9c27b0; color: white; border-radius: 8px; padding: 12px 25px; font-weight: bold; font-size: 14px;");
    ui->btn_ia_appliquer->setCursor(Qt::PointingHandCursor);

    // --- BOUTON MODIFIER ---
    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int idx = ui->tablePlanif->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Veuillez sélectionner une ligne."); return; }
        preparerFormulaireModif(idx); // Va remplir les champs et basculer sur l'onglet 3
    });

    // --- BOUTON VALIDER LA MODIFICATION (Onglet 3) ---
    connect(ui->btn_valider_modif, &QPushButton::clicked, [=](){
        QString pId = ui->cb_produit_modif->currentData().toString();
        int q = ui->sb_qte_modif->value();
        QString mId = ui->cb_matiere_modif->currentData().toString();
        QString eId = ui->cb_employe_modif->currentData().toString();
        QDate d1 = ui->dt_lancement_modif->date();
        QDate d2 = QDate::fromString(ui->le_fin_prevue_modif->text().trimmed(), "dd/MM/yyyy");

        OrdreFabrication o(pId, q, mId, d1, d2, "Planifié", eId);
        int idToEdit = mesCommandes[indexModification].id.mid(3).toInt();

        if(o.modifier(idToEdit)) {
            alerteSucces("Succès", "Commande mise à jour !");
            rafraichirListeCommandes(); configurerTimelineGantt(); calculerEtAfficherStats();
            ui->tabWidgetPlanif->setCurrentIndex(0); // Retour à la liste
        } else {
            alerteErreur("Erreur", "Échec BDD.");
        }
    });

    // --- BOUTON APPLIQUER IA (Onglet 4) ---
    connect(ui->btn_ia_appliquer, &QPushButton::clicked, [=](){
        // On récupère les données cachées dans le bouton
        int idCmd = ui->btn_ia_appliquer->property("id_cmd").toInt();
        QDate nvFin = ui->btn_ia_appliquer->property("nv_fin").toDate();

        QSqlQuery q;
        q.prepare("UPDATE PLANIFICATION SET DATE_FIN_PREVUE = :fin WHERE ID_COMMANDE = :id");
        q.bindValue(":fin", nvFin);
        q.bindValue(":id", idCmd);

        if(q.exec()) {
            alerteSucces("IA", "Planning optimisé avec succès !");
            rafraichirListeCommandes(); configurerTimelineGantt();
            ui->tabWidgetPlanif->setCurrentIndex(0); // Retour à la liste
        }
    });

    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        QString pId = ui->cb_produit->currentData().toString();
        int q = ui->sb_qte->value();
        QString mId = ui->cb_matiere->currentData().toString();
        QString eId = ui->cb_employe->currentData().toString();
        QDate d1 = ui->dt_lancement->date();
        QDate d2 = QDate::fromString(ui->le_fin_prevue->text().trimmed(), "dd/MM/yyyy");
        if(!d2.isValid()) d2 = d1.addDays(3);

        OrdreFabrication o(pId, q, mId, d1, d2, "Planifié", eId);
        int idToEdit = -1;
        if(modeModification && indexModification >= 0 && indexModification < mesCommandes.size())
            idToEdit = mesCommandes[indexModification].id.mid(3).toInt();
        bool ok = modeModification ? o.modifier(idToEdit) : o.ajouter();

        if(ok) {
            alerteSucces("Succès", modeModification ? "Commande modifiée !" : "Commande ajoutée !");
            rafraichirListeCommandes();
            configurerTimelineGantt();
            calculerEtAfficherStats();
            modeModification = false;
            ui->tabWidgetPlanif->setCurrentIndex(0);
        } else {
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

    // --- 2. PRODUITS (Navigation par onglets) ---
    ui->btn_valider_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_produit->setCursor(Qt::PointingHandCursor);

    ui->btn_valider_modif_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_produit->setCursor(Qt::PointingHandCursor);

    // Changement d'onglet manuel (Stats)
    connect(ui->tabWidgetProduits, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsProduits(); // Calcule les stats si on va sur l'onglet 4
    });

    // Bouton "+ Ajouter Produit" (Depuis la liste)
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        preparerFormulaireProduit(false);
    });

    // Bouton "Modifier" (Depuis la liste)
    connect(ui->btn_edit_produit, &QPushButton::clicked, [=](){
        int idx = ui->tableProduits->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Sélectionnez un produit."); return; }
        preparerFormulaireProduit(true, idx);
    });

    // Bouton Supprimer
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){
        int idx = ui->tableProduits->currentRow();
        if(idx >= 0 && idx < mesProduits.size()) {
            mesProduits.removeAt(idx);
            rafraichirListeProduits();
        }
    });

    // Bouton VALIDER AJOUT (Dans l'onglet 2)
    connect(ui->btn_valider_produit, &QPushButton::clicked, [=](){
        if(ui->le_prod_nom->text().isEmpty()) { alerteWarning("Erreur", "Désignation obligatoire."); return; }
        ProduitInfo p = {"TMP-ID", ui->le_prod_nom->text(), ui->sb_prod_cout->value(), ui->cb_prod_coll->currentText(), ui->cb_prod_cuir->currentText(), ui->sb_prod_temps->value(), "Client", "Depot"};
        mesProduits.append(p);
        rafraichirListeProduits();
        ui->tabWidgetProduits->setCurrentIndex(0); // Retour à la liste
    });

    // Bouton VALIDER MODIFICATION (Dans l'onglet 3)
    connect(ui->btn_valider_modif_produit, &QPushButton::clicked, [=](){
        if(ui->le_prod_nom_modif->text().isEmpty()) { alerteWarning("Erreur", "Désignation obligatoire."); return; }
        ProduitInfo p = {"TMP-ID", ui->le_prod_nom_modif->text(), ui->sb_prod_cout_modif->value(), ui->cb_prod_coll_modif->currentText(), ui->cb_prod_cuir_modif->currentText(), ui->sb_prod_temps_modif->value(), "Client", "Depot"};
        mesProduits[indexModifProd] = p;
        rafraichirListeProduits();
        ui->tabWidgetProduits->setCurrentIndex(0); // Retour à la liste
    });


    // =========================================================
    // --- 3. EMPLOYÉS (Navigation SPA)
    // =========================================================

    // Styles
    ui->btn_valider_emp->setStyleSheet(styleBtnSave());
    ui->btn_valider_emp->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_emp->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_emp->setCursor(Qt::PointingHandCursor);

    // Clics manuels sur les onglets (Stats, Eval, Ancienneté, IA)
    connect(ui->tabWidgetEmployes, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsRH();
        else if (index == 4) showEmpEvalTab();
        else if (index == 5) showEmpAncienneteTab();
        else if (index == 6) showEmpAssistantTab();
    });

    // Clics sur vos boutons de menu en haut de l'image (s'ils existent encore)
    // Remplacez les noms par ceux de vos vrais boutons :
    // connect(ui->btn_anciennete, &QPushButton::clicked, this, &MainWindow::showEmpAncienneteTab);
    //connect(ui->btn_eval_rh, &QPushButton::clicked, this, &MainWindow::showEmpEvalTab);
    // connect(ui->btn_assistant_rh, &QPushButton::clicked, this, &MainWindow::showEmpAssistantTab);

    // Routage standard
    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){ preparerFormulaireEmploye(false); });

    connect(ui->btn_edit_emp, &QPushButton::clicked, [=](){
        int idx = ui->tableEmployes->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Sélectionnez un employé."); return; }
        preparerFormulaireEmploye(true, idx);
    });

    connect(ui->btn_delete_emp, &QPushButton::clicked, [=](){
        int idx = ui->tableEmployes->currentRow();
        if(idx >= 0 && idx < mesEmployes.size()) { mesEmployes.removeAt(idx); rafraichirListeEmployes(); }
    });

    // Validations (Ajout & Modif)
    connect(ui->btn_valider_emp, &QPushButton::clicked, [=](){
        if(ui->le_emp_nom->text().isEmpty()) { alerteWarning("Erreur", "Le nom est obligatoire."); return; }
        EmployeInfo e = {"EMP", ui->le_emp_nom->text(), ui->le_emp_pre->text(), ui->cb_emp_poste->currentText(), ui->le_emp_email->text(), ui->le_emp_tel->text(), ui->cb_emp_dept->currentText(), ui->dt_emp_emb->date(), ui->sb_emp_sal->value(), ui->le_emp_rfid->text()};
        mesEmployes.append(e);
        rafraichirListeEmployes();
        ui->tabWidgetEmployes->setCurrentIndex(0);
    });

    connect(ui->btn_valider_modif_emp, &QPushButton::clicked, [=](){
        if(ui->le_emp_nom_modif->text().isEmpty()) { alerteWarning("Erreur", "Le nom est obligatoire."); return; }
        EmployeInfo e = {"EMP", ui->le_emp_nom_modif->text(), ui->le_emp_pre_modif->text(), ui->cb_emp_poste_modif->currentText(), ui->le_emp_email_modif->text(), ui->le_emp_tel_modif->text(), ui->cb_emp_dept_modif->currentText(), ui->dt_emp_emb_modif->date(), ui->sb_emp_sal_modif->value(), ui->le_emp_rfid_modif->text()};
        mesEmployes[indexModifEmp] = e;
        rafraichirListeEmployes();
        ui->tabWidgetEmployes->setCurrentIndex(0);
    });

    // =========================================================
    // --- 4. STOCK MATIÈRES (CRUD Oracle)
    // =========================================================

    // Styles boutons
    ui->btn_valider_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_stock->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_stock->setCursor(Qt::PointingHandCursor);

    // Routage des onglets (Stats, Comparateur, Calculateur)
    connect(ui->tabWidgetStock, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsStock();
        else if (index == 4) showStockCompareTab();
        else if (index == 5) showStockCalculTab();
    });

    // --- BOUTON + AJOUTER MATIÈRE (bascule vers onglet formulaire) ---
    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){
        // Vider les champs
        ui->le_stock_code->clear();
        ui->le_stock_lot->clear();
        ui->le_stock_coul->clear();
        ui->sb_stock_qte->setValue(0);
        ui->cb_stock_cat->setCurrentIndex(0);
        ui->cb_stock_etat->setCurrentIndex(0);
        ui->cb_stock_type->setCurrentIndex(0);
        ui->cb_stock_qual->setCurrentIndex(0);
        ui->tabWidgetStock->setCurrentIndex(1); // Onglet Ajouter
    });

    // --- BOUTON VALIDER AJOUT (Oracle) ---
    connect(ui->btn_valider_stock, &QPushButton::clicked, [=](){
        QString code = ui->le_stock_code->text().trimmed();
        if (code.isEmpty()) {
            alerteWarning("Champ requis", "Le code matière première est obligatoire.");
            return;
        }

        MatierePremiere mp(
            code,
            ui->cb_stock_cat->currentText(),
            ui->le_stock_lot->text().trimmed(),
            ui->cb_stock_etat->currentText(),
            ui->le_stock_coul->text().trimmed(),
            ui->sb_stock_qte->value(),
            ui->cb_stock_type->currentText(),
            ui->cb_stock_qual->currentText()
        );

        if (mp.ajouter()) {
            alerteSucces("Matière ajoutée", "La matière première " + code + " a été enregistrée avec succès !");
            rafraichirListeMatieres();
            ui->tabWidgetStock->setCurrentIndex(0);
        } else {
            alerteErreur("Erreur BDD", "Impossible d'ajouter la matière première dans Oracle.");
        }
    });

    // --- BOUTON MODIFIER (pré-remplir le formulaire) ---
    connect(ui->btn_edit_stock, &QPushButton::clicked, [=](){
        int row = ui->tableStock->currentRow();
        if (row < 0) {
            alerteWarning("Sélection", "Veuillez sélectionner une matière à modifier.");
            return;
        }

        // Pré-remplir les champs de modification
        ui->le_stock_code_modif->setText(ui->tableStock->item(row, 0)->text());
        ui->cb_stock_cat_modif->setCurrentText(ui->tableStock->item(row, 1)->text());
        ui->le_stock_lot_modif->setText(ui->tableStock->item(row, 2)->text());
        ui->cb_stock_etat_modif->setCurrentText(ui->tableStock->item(row, 3)->text());
        ui->le_stock_coul_modif->setText(ui->tableStock->item(row, 4)->text());
        ui->cb_stock_qual_modif->setCurrentText(ui->tableStock->item(row, 5)->text());
        ui->sb_stock_qte_modif->setValue(ui->tableStock->item(row, 6)->text().toDouble());
        ui->cb_stock_type_modif->setCurrentText(ui->tableStock->item(row, 7)->text());

        // Stocker l'index pour la modification
        indexModifStock = row;
        ui->tabWidgetStock->setCurrentIndex(2); // Onglet Modifier
    });

    // --- BOUTON VALIDER MODIFICATION (Oracle) ---
    connect(ui->btn_valider_modif_stock, &QPushButton::clicked, [=](){
        QString code = ui->le_stock_code_modif->text().trimmed();
        if (code.isEmpty()) {
            alerteWarning("Champ requis", "Le code matière première est obligatoire.");
            return;
        }

        if (indexModifStock < 0 || indexModifStock >= mesMatieres.size()) {
            alerteWarning("Sélection", "Aucune matière sélectionnée pour la modification.");
            return;
        }

        // Récupérer l'ID Oracle stocké dans la liste locale
        int idOracle = mesMatieres[indexModifStock].id.toInt();

        MatierePremiere mp(
            code,
            ui->cb_stock_cat_modif->currentText(),
            ui->le_stock_lot_modif->text().trimmed(),
            ui->cb_stock_etat_modif->currentText(),
            ui->le_stock_coul_modif->text().trimmed(),
            ui->sb_stock_qte_modif->value(),
            ui->cb_stock_type_modif->currentText(),
            ui->cb_stock_qual_modif->currentText()
        );

        if (mp.modifier(idOracle)) {
            alerteSucces("Matière modifiée", "La matière " + code + " a été mise à jour !");
            rafraichirListeMatieres();
            ui->tabWidgetStock->setCurrentIndex(0);
        } else {
            alerteErreur("Erreur BDD", "Impossible de modifier la matière dans Oracle.");
        }
    });

    // --- BOUTON SUPPRIMER (Oracle) ---
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

    // --- BOUTON RECHERCHER (Oracle) ---
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

    // --- BOUTON TRI A-Z (Oracle) ---
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

    // --- 5. CLIENTS ---

    // Navigation SPA pour Clients : bascule automatique sur les sous-onglets
    connect(ui->tabWidgetClients, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsClients();
        else if (index == 4) showClientFideliteTab();
        else if (index == 5) showClientIaTab();
    });

    connect(ui->btn_add_client, &QPushButton::clicked, [=](){ ouvrirDialogueClient(false); });
    connect(ui->btn_edit_client, &QPushButton::clicked, [=](){
        int idx = ui->tableClients->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Sélectionnez un client."); return; }
        indexModifClient = ui->tableClients->item(idx, 0)->data(Qt::UserRole).toInt();
        ouvrirDialogueClient(true);
    });
    connect(ui->btn_delete_client, &QPushButton::clicked, [=](){
        int r = ui->tableClients->currentRow();
        if(r >= 0 && r < mesClients.size()) { mesClients.removeAt(r); rafraichirListeClients(); }
    });


    // =========================================================
    // --- 6. DÉPÔT & LOGISTIQUE (Navigation SPA)
    // =========================================================

    ui->btn_valider_depot->setStyleSheet(styleBtnSave());
    ui->btn_valider_depot->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_depot->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_depot->setCursor(Qt::PointingHandCursor);

    // Routage des onglets
    connect(ui->tabWidgetDepot, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsDepot();
        else if (index == 4) showDepotOptimizeTab();
        else if (index == 5) showDepotRavitaillementTab();
    });

    // Boutons de la Liste
    connect(ui->btn_add_depot, &QPushButton::clicked, [=](){ preparerFormulaireDepot(false); });
    
    connect(ui->btn_edit_depot, &QPushButton::clicked, [=](){
        int idx = ui->tableDepot->currentRow();
        if(idx < 0) { alerteWarning("Sélection", "Sélectionnez un emplacement."); return; }
        preparerFormulaireDepot(true, idx);
    });

    connect(ui->btn_delete_depot, &QPushButton::clicked, [=](){
        int r = ui->tableDepot->currentRow();
        if(r >= 0 && r < mesDepots.size()) { mesDepots.removeAt(r); rafraichirListeDepots(); }
    });

    // Validations formulaires
    connect(ui->btn_valider_depot, &QPushButton::clicked, [=](){
        if(ui->le_depot_id->text().isEmpty()) { alerteWarning("Erreur", "L'ID est obligatoire."); return; }
        DepotInfo dp = {ui->le_depot_id->text(), ui->le_depot_emp->text(), ui->le_depot_eta->text(), ui->sb_depot_cap->value(), ui->sb_depot_act->value(), ui->cb_depot_type->currentText()};
        mesDepots.append(dp);
        rafraichirListeDepots();
        ui->tabWidgetDepot->setCurrentIndex(0); // Retour liste
    });

    connect(ui->btn_valider_modif_depot, &QPushButton::clicked, [=](){
        if(ui->le_depot_id_modif->text().isEmpty()) { alerteWarning("Erreur", "L'ID est obligatoire."); return; }
        DepotInfo dp = {ui->le_depot_id_modif->text(), ui->le_depot_emp_modif->text(), ui->le_depot_eta_modif->text(), ui->sb_depot_cap_modif->value(), ui->sb_depot_act_modif->value(), ui->cb_depot_type_modif->currentText()};
        mesDepots[indexModifDepot] = dp;
        rafraichirListeDepots();
        ui->tabWidgetDepot->setCurrentIndex(0); // Retour liste
    });


    // --- BOUTONS EXTRA / INNOVATIONS ---
    connect(ui->btn_cout_produit, &QPushButton::clicked, this, &MainWindow::showProduitCoutDialog);
    connect(ui->btn_hist_mode, &QPushButton::clicked, this, &MainWindow::showHistoriqueModeDialog);

    connect(ui->btn_open_planif_ia, &QPushButton::clicked, this, &MainWindow::ouvrirIAPrediction);

    // Exports PDF/Excel
    // Exports PDF/Excel
    connect(ui->btn_pdf, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning"); });

    // (Lignes supprimées car les boutons excel et print n'existent plus dans ce nouvel onglet)

    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    // ... la suite reste pareille
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_export_excel_stock, &QPushButton::clicked, [=](){ exporterCSV(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_print_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_pdf_facture, &QPushButton::clicked, [=](){ exporterFactureClient(); });
    connect(ui->btn_export_excel_client, &QPushButton::clicked, [=](){ exporterCSV(ui->tableClients, "Clients"); });
    connect(ui->btn_pdf_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });


    // Fabrication (Timeline)
    // Fabrication (Timeline)
    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int r){
        if(r >= 0 && r < mesCommandes.size()){
            indexCommandeSelectionnee = r;
            // CORRECTION ICI : idProduit au lieu de produit
            ui->lbl_sel_cmd->setText("OF: " + mesCommandes[r].id + " - " + mesCommandes[r].idProduit);
            ui->lbl_resultat_delta->setText("..."); ui->lbl_resultat_delta->setStyleSheet("color:gray; border:none;");
        }
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &t){
        double v = 2.0; if(t=="Assemblage") v=5.0; else if(t=="Couture") v=4.0; else if(t=="Finition") v=1.0;
        ui->lbl_temps_prevu->setText(QString::number(v)+" h");
    });

    // VALIDATION ÉTAPE ET INSERTION DANS ORACLE
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
        int idEmploye = 1; // Simplification

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

        // --- REQUÊTE DE SÉCURITÉ ---
        QSqlQuery q;
        q.prepare("SELECT POSTE FROM EMPLOYES WHERE NOM = :nom AND PRENOM = :prenom AND RFID_TAG = :rfid");
        q.bindValue(":nom", nom);
        q.bindValue(":prenom", prenom);
        q.bindValue(":rfid", mdp_rfid); // On utilise le tag RFID comme mot de passe secret ici

        if(q.exec() && q.next()) {
            // L'employé existe !
            QString poste = q.value("POSTE").toString();
            alerteSucces("Bienvenue", "Connexion réussie !\nPoste : " + poste);

            ui->le_login_mdp->clear();
            setNavigationEnabled(true);
            rafraichirListeCommandes();
            construireDashboardAccueil();
            ui->stackedWidget->setCurrentWidget(ui->page_home);
        } else {
            // L'employé n'existe pas ou mauvais mot de passe
            alerteErreur("Accès Refusé", "Identifiants incorrects.");
        }
    });
    // BOUTON "SAISIR TEMPS / DÉTAIL"
    connect(ui->btn_saisie_detail, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee < 0 || indexCommandeSelectionnee >= mesCommandes.size()) {
            alerteWarning("Sélection", "Veuillez d'abord cliquer sur une commande dans le tableau.");
            return;
        }

        // Récupérer l'ID de la commande sélectionnée
        QString idStr = mesCommandes[indexCommandeSelectionnee].id;
        int idPlanif = idStr.replace("OF-", "").toInt();

        // Création de la fenêtre Pop-up
        QDialog d(this);
        d.setWindowTitle("Historique de Fabrication - OF-" + QString::number(idPlanif));
        d.setMinimumSize(500, 300);
        d.setStyleSheet(stylePopup());

        QVBoxLayout *l = new QVBoxLayout(&d);

        QLabel *titre = new QLabel("DÉTAIL DU SUIVI : OF-" + QString::number(idPlanif));
        titre->setStyleSheet("font-size: 16px; font-weight: 800; color: #8d5524; text-transform: uppercase; margin-bottom: 10px;");
        titre->setAlignment(Qt::AlignCenter);
        l->addWidget(titre);

        // Le tableau qui va afficher les données d'Oracle
        QTableView *tv = new QTableView();
        tv->setModel(Etape::afficherHistorique(idPlanif)); // APPEL À LA BASE DE DONNÉES !
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->horizontalHeader()->setStyleSheet("background-color: #8d5524; color: white; font-weight: bold;");
        tv->setStyleSheet("background: white; border: 1px solid #d7ccc8;");
        tv->setSelectionMode(QAbstractItemView::NoSelection);
        tv->setAlternatingRowColors(true);
        l->addWidget(tv);

        // Bouton de fermeture
        QPushButton *btnFermer = new QPushButton("Fermer l'historique");
        btnFermer->setStyleSheet(styleBtnCancel());
        connect(btnFermer, &QPushButton::clicked, &d, &QDialog::accept);
        l->addWidget(btnFermer, 0, Qt::AlignCenter);

        d.exec();
    });

    // =========================================================
    // === HARMONISATION VISUELLE DE TOUS LES TABLEAUX       ===
    // === (placé à l'intérieur du constructeur)            ===
    // =========================================================

    // --- Style commun pour tous les tableaux ---
    auto styleTable = [](QTableWidget *table) {
        if(!table) return;

        // 1. Étirer les colonnes sur toute la largeur (comme Planification)
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // 2. Activer les lignes alternées
        table->setAlternatingRowColors(true);

        // 3. Hauteur de ligne généreuse
        table->verticalHeader()->setDefaultSectionSize(45);

        // 4. Afficher les numéros de lignes dans le header vertical
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

        // 5. Style des lignes alternées et des headers
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

        // 6. Sélection par ligne
        table->setFocusPolicy(Qt::StrongFocus);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
    };

    // Appliquer à TOUS les tableaux
    styleTable(ui->tablePlanif);
    styleTable(ui->tableProduits);
    styleTable(ui->tableEmployes);
    styleTable(ui->tableStock);
    styleTable(ui->tableClients);
    styleTable(ui->tableDepot);
    styleTable(ui->tableTimeline);

    // =========================================================
    // === HARMONISATION BARRE DE BOUTONS (STYLE PLANIF)     ===
    // =========================================================

    // --- Styles des boutons de la barre d'outils ---
    // Style Marron standard (Chercher, Tri, Analyses)
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

    // Style Violet (boutons IA / Innovation)
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

    // Style Vert PDF
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

    // Style Bleu Excel
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

    // Style Or (Ajout / Créer)
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

    // Style Rouge (Supprimer)
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

    // Style Gris (Modifier)
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

    // Style Imprimer
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

    // --- PLANIFICATION ---
    ui->btn_rechercher->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_planif->setStyleSheet(btnMarron);
    ui->btn_open_planif_ia->setStyleSheet(btnViolet);
    ui->btn_stat_plan->setStyleSheet(btnMarron);
    ui->btn_pdf->setStyleSheet(btnPDF);
    ui->btn_to_add_planif->setStyleSheet(btnOr);
    ui->btn_tri->setStyleSheet(btnMarron);
    ui->btn_modifier_planif->setStyleSheet(btnModifier);
    ui->btn_supprimer_planif->setStyleSheet(btnRouge);

    // --- MATIÈRES PREMIÈRES (STOCK) ---
    ui->btn_search_stock->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_stock->setStyleSheet(btnMarron);
    ui->btn_pdf_stock->setStyleSheet(btnPDF);
    ui->btn_export_excel_stock->setStyleSheet(btnExcel);
    ui->btn_print_stock->setStyleSheet(btnImprimer);
    ui->btn_add_stock->setStyleSheet(btnOr);
    ui->btn_edit_stock->setStyleSheet(btnModifier);
    ui->btn_delete_stock->setStyleSheet(btnRouge);

    // --- CLIENTS ---
    ui->btn_search_client->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_client->setStyleSheet(btnMarron);
    ui->btn_pdf_facture->setStyleSheet(btnPDF);
    ui->btn_export_excel_client->setStyleSheet(btnExcel);
    ui->btn_add_client->setStyleSheet(btnOr);
    ui->btn_edit_client->setStyleSheet(btnModifier);
    ui->btn_delete_client->setStyleSheet(btnRouge);

    // --- DÉPÔT ---
    ui->btn_search_depot->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_depot->setStyleSheet(btnMarron);
    ui->btn_pdf_depot->setStyleSheet(btnPDF);
    ui->btn_add_depot->setStyleSheet(btnOr);
    ui->btn_edit_depot->setStyleSheet(btnModifier);
    ui->btn_delete_depot->setStyleSheet(btnRouge);

    // --- PRODUITS ---
    ui->btn_search_col->setStyleSheet(btnMarron);
    ui->btn_sort_alpha_prod->setStyleSheet(btnMarron);
    ui->btn_cout_produit->setStyleSheet(btnViolet);
    ui->btn_hist_mode->setStyleSheet(btnViolet);
    ui->btn_stats_prod->setStyleSheet(btnMarron);
    ui->btn_pdf_catalogue->setStyleSheet(btnPDF);
    ui->btn_add_produit->setStyleSheet(btnOr);
    ui->btn_edit_produit->setStyleSheet(btnModifier);
    ui->btn_delete_produit->setStyleSheet(btnRouge);

    // --- EMPLOYÉS ---
    ui->btn_search_emp->setStyleSheet(btnMarron);
    ui->btn_pdf_emp->setStyleSheet(btnPDF);
    ui->btn_add_emp->setStyleSheet(btnOr);
    ui->btn_edit_emp->setStyleSheet(btnModifier);
    ui->btn_delete_emp->setStyleSheet(btnRouge);

    // --- Curseurs pointeur sur tous les boutons ---
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

    // =========================================================
    // === HARMONISATION BARRE D'ONGLETS (STYLE PLANIF)      ===
    // =========================================================

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

    // Appliquer à TOUS les QTabWidget
    ui->tabWidgetPlanif->setStyleSheet(tabStyle);
    ui->tabWidgetProduits->setStyleSheet(tabStyle);
    ui->tabWidgetEmployes->setStyleSheet(tabStyle);
    ui->tabWidgetStock->setStyleSheet(tabStyle);
    ui->tabWidgetClients->setStyleSheet(tabStyle);
    ui->tabWidgetDepot->setStyleSheet(tabStyle);

    // =========================================================
    // === CORRECTION LAYOUT PAGES STOCK/CLIENTS/DÉPÔT       ===
    // === Pour qu'elles soient identiques à Produits/Planif  ===
    // =========================================================

    // 1. Forcer les marges de 40px sur les pages qui n'en ont pas
    if(ui->page_stock_list->layout()) {
        ui->page_stock_list->layout()->setContentsMargins(40, 40, 40, 40);
    }
    if(ui->page_client_list->layout()) {
        ui->page_client_list->layout()->setContentsMargins(40, 40, 40, 40);
    }
    if(ui->page_depot_list->layout()) {
        ui->page_depot_list->layout()->setContentsMargins(40, 40, 40, 40);
    }

    // 2. Style UNIQUE pour TOUS les QTabWidget (écrase les styles inline du .ui)
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

    // 3. Forcer les marges internes des onglets "Liste" pour Stock/Clients/Dépôt
    //    (car le .ui les définit à 20px au lieu de 0/40)
    if(ui->tab_stock_liste->layout()) {
        ui->tab_stock_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }
    if(ui->tab_client_liste->layout()) {
        ui->tab_client_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }
    if(ui->tab_depot_liste->layout()) {
        ui->tab_depot_liste->layout()->setContentsMargins(0, 10, 0, 0);
    }

    // 4. Ajouter le titre de section manquant aux pages Stock/Clients/Dépôt
    //    (Pour que le titre soit DANS l'onglet comme les Produits)
    ui->lbl_stk->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");
    ui->lbl_cli->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");
    ui->lbl_dep->setStyleSheet("font-size: 28px; font-weight: 300; color:#2c1a16;");

    // =========================================================
    // === HARMONISATION FORMULAIRES AJOUT/MODIFIER          ===
    // === Style identique aux Matières Premières            ===
    // =========================================================

    // --- Style commun pour les formulaires ---
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

    // --- Style du titre centré ---
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

    // --- Style du bouton Valider centré ---
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

    // =============================================
    // --- MATIÈRES PREMIÈRES (déjà bon, on uniformise) ---
    // =============================================
    ui->lbl_titre_ajout_stock->setStyleSheet(titreAjoutStyle);
    ui->lbl_titre_ajout_stock->setAlignment(Qt::AlignCenter);
    ui->lbl_titre_modif_stock->setStyleSheet(titreModifStyle);
    ui->lbl_titre_modif_stock->setAlignment(Qt::AlignCenter);
    ui->tab_stock_ajouter->setStyleSheet(formStyle);
    ui->tab_stock_modifier->setStyleSheet(formStyle);
    ui->btn_valider_stock->setStyleSheet(btnValiderStyle);
    ui->btn_valider_modif_stock->setStyleSheet(btnValiderStyle);

    // =============================================
    // --- PRODUITS ---
    // =============================================
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

    // =============================================
    // --- EMPLOYÉS ---
    // =============================================
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

    // =============================================
    // --- CLIENTS (Ajout et Modif dans les onglets) ---
    // =============================================
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

    // =============================================
    // --- DÉPÔT ---
    // =============================================
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

    // =============================================
    // --- PLANIFICATION (Ajout & Modif) ---
    // =============================================
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

    // =========================================================
    // === MODERNISATION PAGE FABRICATION & SUIVI             ===
    // =========================================================

    // --- 1. TITRE PRINCIPAL ---
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

    // --- 2. LÉGENDE MODERNISÉE ---
    ui->l_legende_colors->setStyleSheet(
        "font-weight: bold;"
        "background: white;"
        "padding: 12px 20px;"
        "border: 2px solid #d7ccc8;"
        "border-radius: 12px;"
        "font-size: 13px;"
        "color: #3e2723;"
    );

    // --- 3. BOUTONS HAUT (Actualiser / Saisir) ---
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

    // --- 4. TABLEAU GANTT TIMELINE ---
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

    // --- 5. LIGNE SÉPARATRICE ---
    ui->line_sep_fab->setStyleSheet(
        "background-color: #d4af37;"
        "border: none;"
        "min-height: 3px;"
        "max-height: 3px;"
        "margin: 10px 0px;"
    );

    // --- 6. PANNEAU DE SUPERVISION (en bas) ---
    ui->frame_supervision->setStyleSheet(
        "QFrame#frame_supervision {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #faf7f2);"
        "  border: 2px solid #d7ccc8;"
        "  border-radius: 14px;"
        "  padding: 0px;"
        "}"
    );

    // Titre de la commande sélectionnée
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

    // Labels du formulaire de saisie
    ui->l_et->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");
    ui->l_tp->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");
    ui->l_tr->setStyleSheet("font-weight: bold; color: #5d4037; font-size: 14px; border: none;");

    // Temps prévu (badge orange)
    ui->lbl_temps_prevu->setStyleSheet(
        "font-weight: 900;"
        "color: white;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e65100, stop:1 #ff8f00);"
        "padding: 6px 14px;"
        "border-radius: 10px;"
        "font-size: 14px;"
        "border: none;"
    );

    // ComboBox étape
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

    // SpinBox temps réel
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

    // Bouton VALIDER étape
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

    // Résultat Delta (style initial)
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

    // Séparateur vertical dans le panneau
    ui->line_sup->setStyleSheet(
        "background-color: #d4af37;"
        "border: none;"
        "min-width: 2px;"
        "max-width: 2px;"
        "margin: 5px 10px;"
    );


}

MainWindow::~MainWindow() { delete ui; }

// =========================================================
// === ALERTES PERSONNALISÉES FIL D'OR                    ===
// =========================================================

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

    // Icône
    QLabel *ico = new QLabel("✅");
    ico->setStyleSheet("font-size: 48px; border: none;");
    ico->setAlignment(Qt::AlignCenter);
    l->addWidget(ico);

    // Titre
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

    // Message
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

    // Bouton OK
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

// =========================================================
// ===       LOGIQUE MÉTIER & AFFICHAGE (TABLEAUX)       ===
// =========================================================

void MainWindow::construireDashboardAccueil() {
    QWidget *page = ui->page_home;

    // Supprimer l'ancien layout
    if (page->layout()) {
        clearLayout(page->layout());
        delete page->layout();
    }

    // --- FOND ÉLÉGANT (pas trop sombre) ---
    page->setStyleSheet(
        "QWidget#page_home {"
        "  background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #2c1a16, stop:0.4 #3e2723, stop:0.7 #4e342e, stop:1 #5d4037);"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(35, 25, 35, 20);

    // =============================================
    // 1. EN-TÊTE : Logo + Titre + Date + Heure
    // =============================================
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,0,0,0.25);"
        "  border: 1px solid rgba(212,175,55,0.2);"
        "  border-radius: 16px;"
        "}"
    );
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 14, 20, 14);

    // Logo
    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/logo.png").scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setStyleSheet("border: none;");
    headerLayout->addWidget(logoLabel);

    // Titres
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    QLabel *titre = new QLabel("✨ FIL D'OR — Tableau de Bord");
    titre->setStyleSheet(
        "font-size: 24px; font-weight: 900; color: #d4af37;"
        "letter-spacing: 2px; border: none;"
    );
    titleLayout->addWidget(titre);

    QLabel *sousTitre = new QLabel("Vue d'ensemble de l'atelier de maroquinerie de luxe");
    sousTitre->setStyleSheet(
        "font-size: 12px; color: #bcaaa4; font-style: italic; border: none;"
    );
    titleLayout->addWidget(sousTitre);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Date & Heure
    QVBoxLayout *dateLayout = new QVBoxLayout();
    dateLayout->setSpacing(2);
    QLabel *dateLabel = new QLabel("📅 " + QDate::currentDate().toString("dddd dd MMMM yyyy"));
    dateLabel->setStyleSheet(
        "font-size: 13px; color: #e0c097; font-weight: bold; border: none;"
    );
    dateLabel->setAlignment(Qt::AlignRight);
    dateLayout->addWidget(dateLabel);

    QLabel *heureLabel = new QLabel("🕐 " + QTime::currentTime().toString("HH:mm"));
    heureLabel->setStyleSheet(
        "font-size: 12px; color: #a1887f; border: none;"
    );
    heureLabel->setAlignment(Qt::AlignRight);
    dateLayout->addWidget(heureLabel);
    headerLayout->addLayout(dateLayout);

    mainLayout->addWidget(headerFrame);

    // =============================================
    // 2. REQUÊTES ORACLE
    // =============================================
    int totalCommandes = 0, totalPieces = 0, cmdEnCours = 0, cmdRetard = 0, cmdFini = 0;
    int totalEmployes = 0, totalMatieres = 0, totalClients = 0, totalProduits = 0;
    double masseSalariale = 0, volumeStock = 0;

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

    double tauxRetard = (totalCommandes > 0) ? (static_cast<double>(cmdRetard) * 100.0 / totalCommandes) : 0;

    // =============================================
    // 3. HELPER : Créer une carte KPI luxe
    // =============================================
    auto creerCarteKPI = [](QString icone, QString valeur, QString label, QString gradientBg, QString borderColor) -> QFrame* {
        QFrame *card = new QFrame();
        card->setMinimumHeight(110);
        card->setMaximumHeight(130);
        card->setStyleSheet(QString(
            "QFrame {"
            "  background: %1;"
            "  border-radius: 14px;"
            "  border: 1px solid %2;"
            "}"
        ).arg(gradientBg, borderColor));

        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(18, 12, 18, 12);
        hl->setSpacing(12);

        // Icône à GAUCHE dans un cercle
        QLabel *lblIco = new QLabel(icone);
        lblIco->setFixedSize(50, 50);
        lblIco->setAlignment(Qt::AlignCenter);
        lblIco->setStyleSheet(
            "font-size: 26px;"
            "border: none;"
            "background: rgba(255,255,255,0.15);"
            "border-radius: 25px;"
        );
        hl->addWidget(lblIco);

        // Textes à DROITE
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(2);

        QLabel *lblVal = new QLabel(valeur);
        lblVal->setStyleSheet(
            "font-size: 28px; font-weight: 900; color: white; border: none; letter-spacing: 1px;"
        );
        textLayout->addWidget(lblVal);

        QLabel *lblLabel = new QLabel(label);
        lblLabel->setStyleSheet(
            "font-size: 11px; font-weight: 700; color: rgba(255,255,255,0.75);"
            "letter-spacing: 1px; text-transform: uppercase; border: none;"
        );
        textLayout->addWidget(lblLabel);

        hl->addLayout(textLayout, 1);
        return card;
    };

    // =============================================
    // 4. SECTION 1 : PRODUCTION & PLANIFICATION
    // =============================================
    QLabel *sec1 = new QLabel("🏭  PRODUCTION & PLANIFICATION");
    sec1->setStyleSheet(
        "font-size: 13px; font-weight: 800; color: #d4af37;"
        "letter-spacing: 2px; border: none; margin-top: 2px;"
    );
    mainLayout->addWidget(sec1);

    QHBoxLayout *kpiRow1 = new QHBoxLayout();
    kpiRow1->setSpacing(14);

    kpiRow1->addWidget(creerCarteKPI("📋", QString::number(totalCommandes), "Ordres en base",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1565c0, stop:1 #0d47a1)",
        "rgba(21,101,192,0.4)"));

    kpiRow1->addWidget(creerCarteKPI("📦", QString::number(totalPieces), "Pièces planifiées",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #8d5524)",
        "rgba(212,175,55,0.4)"));

    kpiRow1->addWidget(creerCarteKPI("🔄", QString::number(cmdEnCours), "En production",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #00897b, stop:1 #00695c)",
        "rgba(0,137,123,0.4)"));

    QString colRetard = (tauxRetard > 20)
        ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c62828, stop:1 #b71c1c)"
        : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2e7d32, stop:1 #1b5e20)";
    QString bordRetard = (tauxRetard > 20) ? "rgba(198,40,40,0.4)" : "rgba(46,125,50,0.4)";
    kpiRow1->addWidget(creerCarteKPI("⚠️", QString::number(tauxRetard, 'f', 1) + "%", "Taux de retard",
        colRetard, bordRetard));

    mainLayout->addLayout(kpiRow1);

    // =============================================
    // 5. SECTION 2 : RESSOURCES & STOCK
    // =============================================
    QLabel *sec2 = new QLabel("👥  RESSOURCES, STOCK & CLIENTS");
    sec2->setStyleSheet(
        "font-size: 13px; font-weight: 800; color: #d4af37;"
        "letter-spacing: 2px; border: none; margin-top: 2px;"
    );
    mainLayout->addWidget(sec2);

    QHBoxLayout *kpiRow2 = new QHBoxLayout();
    kpiRow2->setSpacing(14);

    kpiRow2->addWidget(creerCarteKPI("👥", QString::number(totalEmployes), "Employés actifs",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6a1b9a, stop:1 #4a148c)",
        "rgba(106,27,154,0.4)"));

    kpiRow2->addWidget(creerCarteKPI("💰", QString::number(masseSalariale, 'f', 0) + " €", "Masse salariale",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ef6c00, stop:1 #e65100)",
        "rgba(239,108,0,0.4)"));

    kpiRow2->addWidget(creerCarteKPI("🧵", QString::number(volumeStock, 'f', 0) + " u", "Stock total",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #00838f, stop:1 #006064)",
        "rgba(0,131,143,0.4)"));

    kpiRow2->addWidget(creerCarteKPI("🧾", QString::number(totalProduits), "Produits enregistrés",
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #8d5524, stop:1 #5d4037)",
        "rgba(141,85,36,0.4)"));

    mainLayout->addLayout(kpiRow2);

    // =============================================
    // 6. SECTION BASSE : Dernières commandes + Alertes
    // =============================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(14);

    // --- PANNEAU GAUCHE : Dernières commandes ---
    QFrame *frameRecent = new QFrame();
    frameRecent->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,0,0,0.2);"
        "  border: 1px solid rgba(212,175,55,0.2);"
        "  border-radius: 14px;"
        "}"
    );
    QVBoxLayout *recentLayout = new QVBoxLayout(frameRecent);
    recentLayout->setContentsMargins(16, 14, 16, 14);
    recentLayout->setSpacing(10);

    QLabel *recentTitle = new QLabel("📋  DERNIÈRES COMMANDES");
    recentTitle->setStyleSheet(
        "font-size: 14px; font-weight: 900; color: #d4af37;"
        "letter-spacing: 1px; border: none;"
    );
    recentLayout->addWidget(recentTitle);

    // Ligne dorée sous le titre
    QFrame *lineGold1 = new QFrame();
    lineGold1->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d4af37,stop:1 transparent);"
                              "border:none; min-height:2px; max-height:2px;");
    recentLayout->addWidget(lineGold1);

    QTableWidget *tableRecent = new QTableWidget();
    tableRecent->setColumnCount(5);
    tableRecent->setHorizontalHeaderLabels({"Produit", "Qté", "Matière", "Statut", "Fin Prévue"});
    tableRecent->setStyleSheet(
        "QTableWidget {"
        "  background: transparent;"
        "  border: none;"
        "  color: #e0c097;"
        "  gridline-color: rgba(212,175,55,0.1);"
        "  font-size: 12px;"
        "  selection-background-color: rgba(212,175,55,0.15);"
        "}"
        "QTableWidget::item {"
        "  border-bottom: 1px solid rgba(255,255,255,0.05);"
        "  padding: 8px 6px;"
        "}"
        "QHeaderView::section {"
        "  background: rgba(212,175,55,0.12);"
        "  color: #d4af37;"
        "  border: none;"
        "  border-bottom: 2px solid rgba(212,175,55,0.3);"
        "  padding: 10px 6px;"
        "  font-weight: 900;"
        "  font-size: 11px;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
    );
    tableRecent->horizontalHeader()->setStretchLastSection(true);
    tableRecent->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableRecent->verticalHeader()->setVisible(false);
    tableRecent->setSelectionMode(QAbstractItemView::NoSelection);
    tableRecent->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableRecent->setShowGrid(false);
    tableRecent->setAlternatingRowColors(false);

    // Charger les 6 dernières commandes (compatible Oracle XE 11g+)
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

    // Debug : vérifier si la requête a fonctionné
    if (qRecent.lastError().isValid()) {
        qDebug() << "Erreur Dashboard Commandes :" << qRecent.lastError().text();
    }

    int row = 0;
    while (qRecent.next()) {
        tableRecent->insertRow(row);

        // Produit
        QTableWidgetItem *iProd = new QTableWidgetItem(qRecent.value(0).toString());
        iProd->setForeground(QColor("#e0c097"));
        QFont fProd; fProd.setBold(true); iProd->setFont(fProd);
        tableRecent->setItem(row, 0, iProd);

        // Quantité
        QTableWidgetItem *iQte = new QTableWidgetItem(qRecent.value(1).toString());
        iQte->setForeground(QColor("#ffffff"));
        iQte->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(row, 1, iQte);

        // Matière
        QTableWidgetItem *iMat = new QTableWidgetItem(qRecent.value(2).toString());
        iMat->setForeground(QColor("#a1887f"));
        iMat->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(row, 2, iMat);

        // Statut avec couleur conditionnelle
        QString statut = qRecent.value(3).toString();
        QTableWidgetItem *iStat = new QTableWidgetItem(statut);
        if (statut.toLower().contains("retard")) {
            iStat->setForeground(QColor("#ef5350"));
            iStat->setText("🔴 " + statut);
        } else if (statut.toLower().contains("cours")) {
            iStat->setForeground(QColor("#66bb6a"));
            iStat->setText("🟢 " + statut);
        } else if (statut.toLower().contains("fini") || statut.toLower().contains("termin")) {
            iStat->setForeground(QColor("#29b6f6"));
            iStat->setText("✅ " + statut);
        } else {
            iStat->setForeground(QColor("#ffa726"));
            iStat->setText("🟡 " + statut);
        }
        iStat->setTextAlignment(Qt::AlignCenter);
        QFont fStat; fStat.setBold(true); iStat->setFont(fStat);
        tableRecent->setItem(row, 3, iStat);

        // Date fin
        QTableWidgetItem *iFin = new QTableWidgetItem(qRecent.value(4).toString());
        iFin->setForeground(QColor("#bcaaa4"));
        iFin->setTextAlignment(Qt::AlignCenter);
        tableRecent->setItem(row, 4, iFin);

        tableRecent->setRowHeight(row, 40);
        row++;
    }

    recentLayout->addWidget(tableRecent, 1);
    bottomLayout->addWidget(frameRecent, 3);

    // --- PANNEAU DROIT : Alertes & Notifications ---
    QFrame *frameAlertes = new QFrame();
    frameAlertes->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,0,0,0.2);"
        "  border: 1px solid rgba(212,175,55,0.2);"
        "  border-radius: 14px;"
        "}"
    );
    QVBoxLayout *alertLayout = new QVBoxLayout(frameAlertes);
    alertLayout->setContentsMargins(16, 14, 16, 14);
    alertLayout->setSpacing(8);

    QLabel *alertTitle = new QLabel("🔔  ALERTES & NOTIFICATIONS");
    alertTitle->setStyleSheet(
        "font-size: 14px; font-weight: 900; color: #d4af37;"
        "letter-spacing: 1px; border: none;"
    );
    alertLayout->addWidget(alertTitle);

    QFrame *lineGold2 = new QFrame();
    lineGold2->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d4af37,stop:1 transparent);"
                              "border:none; min-height:2px; max-height:2px;");
    alertLayout->addWidget(lineGold2);

    // Helper pour créer une alerte
    auto ajouterAlerte = [&](QString ico, QString texte, QString couleur, QString bgAlpha) {
        QFrame *alertItem = new QFrame();
        alertItem->setStyleSheet(QString(
            "QFrame {"
            "  background: %1;"
            "  border-left: 4px solid %2;"
            "  border-radius: 10px;"
            "  margin: 2px 0px;"
            "}"
        ).arg(bgAlpha, couleur));

        QHBoxLayout *hl = new QHBoxLayout(alertItem);
        hl->setContentsMargins(12, 10, 12, 10);
        hl->setSpacing(10);

        QLabel *lblIco = new QLabel(ico);
        lblIco->setStyleSheet("font-size: 20px; border: none;");
        lblIco->setFixedWidth(30);
        hl->addWidget(lblIco);

        QLabel *lblTxt = new QLabel(texte);
        lblTxt->setStyleSheet(QString(
            "color: %1; font-size: 12px; font-weight: 700; border: none; line-height: 1.4;"
        ).arg(couleur));
        lblTxt->setWordWrap(true);
        hl->addWidget(lblTxt, 1);

        alertLayout->addWidget(alertItem);
    };

    // --- Alertes dynamiques depuis Oracle ---
    int alertCount = 0;

    // Retards
    if (cmdRetard > 0) {
        ajouterAlerte("🚨", QString("%1 commande(s) en retard critique !").arg(cmdRetard),
            "#ef5350", "rgba(239,83,80,0.08)");
        alertCount++;
    }

    // Stock bas (< 20 unités)
    QSqlQuery qStockBas;
    qStockBas.exec("SELECT CODE_MP, QUANTITE FROM MATIERES_PREMIERES WHERE QUANTITE < 20 ORDER BY QUANTITE ASC");
    while (qStockBas.next()) {
        ajouterAlerte("📉", "Stock critique : " + qStockBas.value(0).toString() +
                      " — seulement " + qStockBas.value(1).toString() + " unités restantes",
                      "#ffa726", "rgba(255,167,38,0.08)");
        alertCount++;
    }

    // En production
    if (cmdEnCours > 0) {
        ajouterAlerte("🔄", QString("%1 commande(s) en cours de fabrication.").arg(cmdEnCours),
            "#66bb6a", "rgba(102,187,106,0.08)");
        alertCount++;
    }

    // Terminées
    if (cmdFini > 0) {
        ajouterAlerte("✅", QString("%1 commande(s) terminée(s) avec succès.").arg(cmdFini),
            "#29b6f6", "rgba(41,182,246,0.08)");
        alertCount++;
    }

    // Volume stock
    ajouterAlerte("📊", QString("Volume total en stock : %1 unités sur %2 lots.")
        .arg(volumeStock, 0, 'f', 0).arg(totalMatieres),
        "#ce93d8", "rgba(206,147,216,0.08)");
    alertCount++;

    // Aucune alerte
    if (cmdRetard == 0 && alertCount <= 3) {
        ajouterAlerte("🌟", "Production optimale — Aucun retard détecté !",
            "#66bb6a", "rgba(102,187,106,0.08)");
    }

    // Résumé clients
    ajouterAlerte("🧾", QString("%1 client(s) — %2 produit(s) au catalogue.")
        .arg(totalClients).arg(totalProduits),
        "#e0c097", "rgba(224,192,151,0.08)");

    alertLayout->addStretch();
    bottomLayout->addWidget(frameAlertes, 2);

    mainLayout->addLayout(bottomLayout, 1);

    // =============================================
    // 7. FOOTER
    // =============================================
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,0,0,0.15);"
        "  border: 1px solid rgba(212,175,55,0.15);"
        "  border-radius: 10px;"
        "}"
    );
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(20, 8, 20, 8);

    QLabel *footerLeft = new QLabel("✨ FIL D'OR — L'Excellence de la Maroquinerie");
    footerLeft->setStyleSheet("font-size: 11px; color: rgba(212,175,55,0.6); font-style: italic; border: none;");
    footerLayout->addWidget(footerLeft);

    footerLayout->addStretch();

    QLabel *footerRight = new QLabel("© 2026 — Atelier de Production");
    footerRight->setStyleSheet("font-size: 11px; color: rgba(161,136,127,0.6); border: none;");
    footerLayout->addWidget(footerRight);

    mainLayout->addWidget(footerFrame);
}

void MainWindow::rafraichirListeCommandes() {
    QSqlQueryModel *model = tmpOrdre.afficher();
    mesCommandes.clear();
    ui->tablePlanif->setRowCount(0);

    // ON PASSE À 8 COLONNES POUR L'EMPLOYÉ
    ui->tablePlanif->setColumnCount(8);
    ui->tablePlanif->setHorizontalHeaderLabels({"ID", "Produit", "Qté", "Matière", "Début", "Fin", "Statut", "Employé"});

    int rows = model->rowCount();
    ui->tablePlanif->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        QString idStr = model->record(i).value("ID_COMMANDE").toString();
        QString prod = model->record(i).value("PRODUIT").toString(); // Nom aliasé dans la requête SQL
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
        c.idProduit = prod;   // <--- CORRIGÉ (idProduit)
        c.quantite = qte;
        c.idMatiere = mat;    // <--- CORRIGÉ (idMatiere)
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
    ui->tableTimeline->setItem(r, c, it);
    if (d > 1) {
        ui->tableTimeline->setSpan(r, c, d, 1);
    }
}

void MainWindow::calculerEtAfficherStats() {
    QSqlQuery q;

    // 1. Total Commandes
    if(q.exec("SELECT COUNT(*) FROM PLANIFICATION") && q.next()) {
        ui->lbl_stat_total_cmd->setText(q.value(0).toString());
    }

    // 2. Total Quantité Produite
    if(q.exec("SELECT SUM(QUANTITE) FROM PLANIFICATION") && q.next()) {
        ui->lbl_stat_total_qty->setText(q.value(0).toString());
    }

    // 3. Répartition par Statut (Planifié, En Cours, Retard)
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

    // 4. Calcul du taux de retard (Basé sur la table ETAPES)
    if(q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next()) {
        int cmdEnRetard = q.value(0).toInt();
        double retardPct = (total > 0) ? (static_cast<double>(cmdEnRetard) * 100.0 / total) : 0;
        ui->lbl_stat_retard->setText(QString::number(retardPct, 'f', 0) + "%");
    }
}

// Rafraichissement listes locales
void MainWindow::rafraichirListeProduits() {
    ui->tableProduits->setRowCount(0);
    ui->tableProduits->setColumnCount(6);
    ui->tableProduits->setHorizontalHeaderLabels({"RÉF", "DÉSIGNATION", "COÛT", "COLLECTION", "CUIR", "TEMPS"});
    for(int i=0; i<mesProduits.size(); i++) {
        ui->tableProduits->insertRow(i);
        ui->tableProduits->setItem(i,0,new QTableWidgetItem(mesProduits[i].id_produit));
        ui->tableProduits->item(i,0)->setData(Qt::UserRole, i);
        ui->tableProduits->setItem(i,1,new QTableWidgetItem(mesProduits[i].designation));
        ui->tableProduits->setItem(i,2,new QTableWidgetItem(QString::number(mesProduits[i].cout, 'f', 2) + " €"));
        ui->tableProduits->setItem(i,3,new QTableWidgetItem(mesProduits[i].collection));
        ui->tableProduits->setItem(i,4,new QTableWidgetItem(mesProduits[i].typeCuir));
        ui->tableProduits->setItem(i,5,new QTableWidgetItem(QString::number(mesProduits[i].tempsFab) + " h"));
    }
}
void MainWindow::rafraichirListeEmployes()
{
    QSqlQueryModel *model = tmpEmploye.afficher();

    ui->tableEmployes->setRowCount(0);
    ui->tableEmployes->setColumnCount(7);
    ui->tableEmployes->setHorizontalHeaderLabels(
        {"ID", "NOM", "PRÉNOM", "POSTE", "DÉPARTEMENT", "SALAIRE", "EMBAUCHE"}
        );

    for (int i = 0; i < model->rowCount(); ++i) {
        ui->tableEmployes->insertRow(i);

        for (int c = 0; c < 7; ++c) {
            QString val;
            if (c == 5) { // SALAIRE
                val = model->index(i, c).data().toString() + " €";
            } else if (c == 6) { // DATE_EMBAUCHE
                val = model->index(i, c).data().toDate().toString("dd/MM/yyyy");
            } else {
                val = model->index(i, c).data().toString();
            }
            ui->tableEmployes->setItem(i, c, new QTableWidgetItem(val));
        }
    }
}
void MainWindow::rafraichirListeMatieres() {
    // Lecture depuis Oracle
    QSqlQueryModel *model = tmpMatiere.afficher();

    ui->tableStock->setRowCount(0);
    ui->tableStock->setColumnCount(8);
    ui->tableStock->setHorizontalHeaderLabels({
        "Code", "Catégorie", "Lot", "État", "Couleur", "Qualité", "Qté", "Stockage"
    });

    int rows = model->rowCount();
    ui->tableStock->setRowCount(rows);

    // Synchroniser la liste locale
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

        // Colonne 0 : Code (avec ID Oracle caché dans UserRole)
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

        // Liste locale synchronisée
        MatiereInfo m = {QString::number(idDb), code, cat, lot, etat, coul, qte, type, qual};
        mesMatieres.append(m);
    }

    delete model;
}
void MainWindow::rafraichirListeClients() {
    ui->tableClients->setRowCount(0);
    ui->tableClients->setColumnCount(6);
    ui->tableClients->setHorizontalHeaderLabels({"ID CLIENT", "NOM", "TÉLÉPHONE", "ADRESSE", "EMAIL", "FIDÉLITÉ"});
    for(int i=0; i<mesClients.size(); i++) {
        ui->tableClients->insertRow(i);
        ui->tableClients->setItem(i,0,new QTableWidgetItem(mesClients[i].id));
        ui->tableClients->item(i,0)->setData(Qt::UserRole, i);
        ui->tableClients->setItem(i,1,new QTableWidgetItem(mesClients[i].nom));
        ui->tableClients->setItem(i,2,new QTableWidgetItem(mesClients[i].telephone));
        ui->tableClients->setItem(i,3,new QTableWidgetItem(mesClients[i].adresse));
        ui->tableClients->setItem(i,4,new QTableWidgetItem(mesClients[i].email));
        ui->tableClients->setItem(i,5,new QTableWidgetItem(QString::number(mesClients[i].pointsFidelite) + " pts"));
    }
}
void MainWindow::rafraichirListeDepots() {
    ui->tableDepot->setRowCount(0);
    ui->tableDepot->setColumnCount(7);
    ui->tableDepot->setHorizontalHeaderLabels({"ID", "EMPLACEMENT", "ÉTAGÈRE", "CAPACITÉ MAX", "QTÉ ACTUELLE", "TYPE", "REMPLISSAGE"});
    for(int i=0; i<mesDepots.size(); i++) {
        ui->tableDepot->insertRow(i);
        ui->tableDepot->setItem(i,0,new QTableWidgetItem(mesDepots[i].id));
        ui->tableDepot->item(i,0)->setData(Qt::UserRole, i);
        ui->tableDepot->setItem(i,1,new QTableWidgetItem(mesDepots[i].emplacement));
        ui->tableDepot->setItem(i,2,new QTableWidgetItem(mesDepots[i].etagere));
        ui->tableDepot->setItem(i,3,new QTableWidgetItem(QString::number(mesDepots[i].capaciteMax)));
        ui->tableDepot->setItem(i,4,new QTableWidgetItem(QString::number(mesDepots[i].quantiteActuelle)));
        ui->tableDepot->setItem(i,5,new QTableWidgetItem(mesDepots[i].typeStockage));
        double r = (mesDepots[i].capaciteMax > 0) ? (mesDepots[i].quantiteActuelle/mesDepots[i].capaciteMax)*100 : 0;
        ui->tableDepot->setItem(i,6,new QTableWidgetItem(QString::number(r, 'f', 1) + "%"));
    }
}

// Exports
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


// =========================================================
// ===    SPA - NAVIGATION FLUIDE (PLANIFICATION)       ===
// =========================================================

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
    if(estModif && idx >= 0 && idx < mesProduits.size()) {
        indexModifProd = idx;
        const auto &p = mesProduits[idx];

        // On pré-remplit l'onglet Modifier
        ui->le_prod_nom_modif->setText(p.designation);
        ui->sb_prod_cout_modif->setValue(p.cout);
        ui->cb_prod_coll_modif->setCurrentText(p.collection);
        ui->cb_prod_cuir_modif->setCurrentText(p.typeCuir);
        ui->sb_prod_temps_modif->setValue(p.tempsFab);

        // On bascule sur l'onglet "Modifier" (Index 2)
        ui->tabWidgetProduits->setCurrentIndex(2);
    } else {
        // On vide l'onglet Ajout
        ui->le_prod_nom->clear();
        ui->sb_prod_cout->setValue(0);
        ui->sb_prod_temps->setValue(1);

        // On bascule sur l'onglet "Ajouter" (Index 1)
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

        ui->tabWidgetStock->setCurrentIndex(2); // Bascule sur Modifier
    } else {
        ui->le_stock_code->clear();
        ui->le_stock_lot->clear();
        ui->le_stock_coul->clear();
        ui->sb_stock_qte->setValue(0);

        ui->tabWidgetStock->setCurrentIndex(1); // Bascule sur Ajouter
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
    connect(btnSave, &QPushButton::clicked, [&](){
        if(leId->text().isEmpty() || leNom->text().isEmpty()) return;
        ClientInfo c = {leId->text(), leNom->text(), leTel->text(), leAdr->text(), leMail->text(), sbPts->value()};
        if(estModif) mesClients[indexModifClient] = c; else mesClients.append(c);
        rafraichirListeClients(); d.accept();
    });
    d.exec();
}



// =========================================================
// ===      TABLEAUX DE BORD (STATS) EN POP-UP           ===
// =========================================================

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
    // 1. Cible l'onglet "Analyses" des Produits (Index 3)
    if(ui->tabWidgetProduits->count() < 4) return;
    QWidget *ongletStats = ui->tabWidgetProduits->widget(3);
    if (!ongletStats) return;

    // 2. Nettoie la page pour redessiner proprement
    if (ongletStats->layout()) { clearLayout(ongletStats->layout()); delete ongletStats->layout(); }
    QVBoxLayout *mainL = new QVBoxLayout(ongletStats);
    mainL->setSpacing(20);
    mainL->setContentsMargins(20, 20, 20, 20);

    // --- BANNIÈRE TITRE (Style Planification) ---
    QLabel *t = new QLabel("TABLEAU DE BORD - CATALOGUE PRODUITS");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter);
    mainL->addWidget(t);

    // --- LECTURE DES DONNÉES (MesProduits temporaire ou Oracle plus tard) ---
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

    // --- LES 3 CARTES KPI EN HAUT ---
    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->setSpacing(15);

    // Carte 1 : Vert (Total Références)
    kpiL->addWidget(creerCarteStat("👜", QString::number(total), "Références Actives", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)"));

    // Carte 2 : Bleu (Coût Moyen)
    kpiL->addWidget(creerCarteStat("💰", QString::number(moy, 'f', 1) + " €", "Coût Moyen de Fab.", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));

    // Carte 3 : Orange (Infos Collection - Exemple : Collection dominante)
    QString collDominante = "Aucune";
    if(!parCollection.isEmpty()) {
        auto it = std::max_element(parCollection.begin(), parCollection.end());
        collDominante = it.key();
    }
    kpiL->addWidget(creerCarteStat("✨", collDominante, "Collection Phare", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f12711, stop:1 #f5af19)"));

    mainL->addLayout(kpiL);

    // --- LES GRAPHIQUES EN BAS ---
    QHBoxLayout *chartsL = new QHBoxLayout();

    // Cadre Blanc pour le Camembert
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

    // Cadre Blanc pour l'Histogramme
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

    // 3. Bascule automatique sur l'onglet
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
    kpiL->addWidget(creerCarteStat("📏", QString::number(volume) + " Unités", "Volume Total Actuel", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e1eec3, stop:1 #f05053)"));
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

// =========================================================
// ===             MÉTIERS AVANCÉS CLIENTS               ===
// =========================================================

void MainWindow::ouvrirStatsClients() {
    if(ui->tabWidgetClients->count() < 4) return;
    QWidget *onglet = ui->tabWidgetClients->widget(3);
    if (!onglet) return;

    // Suppression sécurisée de l'ancien layout
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
    
    // Force la mise à jour visuelle
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

    int idx = ui->tableClients->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesClients.size()) {
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

    int idx = ui->tableClients->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesClients.size()) {
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


// =========================================================
// ===             EXPORT FACTURE CLIENT                 ===
// =========================================================
// --- 0. STATS PLANIFICATION & PRODUCTION ---
void MainWindow::ouvrirStatsPlanification() {
    QDialog d(this);
    d.setWindowTitle("Analyses Planification");
    d.setMinimumSize(850, 600);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);

    QLabel *t = new QLabel("DASHBOARD : PLANIFICATION & PRODUCTION");
    t->setStyleSheet("font-size: 22px; font-weight: 800; color: #8d5524; margin-bottom: 10px;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    // --- REQUÊTES SQL VERS ORACLE ---
    int totalCmd = 0;
    int totalQte = 0;
    int cmdRetard = 0;
    QMap<QString, double> parStatut;
    QMap<QString, double> parProduit;

    QSqlQuery q;
    // 1. Total commandes et pièces
    if(q.exec("SELECT COUNT(*), NVL(SUM(QUANTITE), 0) FROM PLANIFICATION") && q.next()) {
        totalCmd = q.value(0).toInt();
        totalQte = q.value(1).toInt();
    }
    // 2. Commandes en retard (depuis la table ETAPES)
    if(q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next()) {
        cmdRetard = q.value(0).toInt();
    }
    // 3. Commandes par Statut
    if(q.exec("SELECT STATUT, COUNT(*) FROM PLANIFICATION GROUP BY STATUT")) {
        while(q.next()) { parStatut[q.value(0).toString()] = q.value(1).toDouble(); }
    }
    // 4. Commandes par Produit (Avec Jointure)
    if(q.exec("SELECT pr.DESIGNATION, COUNT(p.ID_COMMANDE) FROM PLANIFICATION p JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT GROUP BY pr.DESIGNATION")) {
        while(q.next()) { parProduit[q.value(0).toString()] = q.value(1).toDouble(); }
    }

    double tauxRetard = (totalCmd > 0) ? (static_cast<double>(cmdRetard) * 100.0 / totalCmd) : 0;

    // --- CARTES KPI (En Haut) ---
    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("📋", QString::number(totalCmd), "Ordres de Fabrication", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));
    kpiL->addWidget(creerCarteStat("📦", QString::number(totalQte), "Pièces Produites", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d4af37, stop:1 #8d5524)"));

    // Si le retard est > 20%, la carte devient Rouge, sinon elle est Verte !
    QString colorRetard = (tauxRetard > 20) ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EB3349, stop:1 #F45C43)" : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)";
    kpiL->addWidget(creerCarteStat("⚠️", QString::number(tauxRetard, 'f', 1) + "%", "Taux de Retard", colorRetard));
    mainL->addLayout(kpiL);

    // --- GRAPHIQUES (En Bas) ---
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

    // --- BOUTON FERMER ---
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

    alerteSucces("Succès", "Facture exportée en PDF.");
}

// Outils (Vides pour l'instant)// Outils
void MainWindow::showPlanifIaDialog() {
    // On utilise la nouvelle navigation fluide au lieu de l'ancien pop-up
    preparerFormulairePlanif(false);
}
void MainWindow::showProduitCoutDialog() {
    if(ui->tabWidgetProduits->count() < 5) return; // Sécurité si l'onglet n'existe pas

    QWidget *ongletCout = ui->tabWidgetProduits->widget(4); // Index 4 = 5ème onglet
    if (ongletCout->layout()) { clearLayout(ongletCout->layout()); delete ongletCout->layout(); }

    QVBoxLayout *l = new QVBoxLayout(ongletCout);
    QLabel *titre = new QLabel("💰 SIMULATEUR DE COÛTS & MARGES");
    titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    QLabel *desc = new QLabel();
    int idx = ui->tableProduits->currentRow();

    if(idx >= 0 && idx < mesProduits.size()) {
        // SI UN PRODUIT EST SÉLECTIONNÉ : ON CALCULE SA MARGE !
        ProduitInfo p = mesProduits[idx];
        double coutMP = p.cout;
        double coutMainOeuvre = p.tempsFab * 15.5; // Base: 15.5€/h par artisan
        double coutTotal = coutMP + coutMainOeuvre;
        double prixVente = coutTotal * 2.5; // La marge Fil d'Or

        QString html = QString(
                           "<div style='background: white; border: 1px solid #d7ccc8; border-radius: 10px; padding: 20px; font-size: 15px; color: #3e2723;'>"
                           "<h3 style='color:#8d5524; margin-top:0;'>Analyse Financière : %1</h3><hr>"
                           "<ul>"
                           "<li>Coût Matière Première (Cuir/Fil) : <b>%2 €</b></li>"
                           "<li>Coût Main d'Oeuvre estimé : <b>%3 €</b> (%4 h)</li>"
                           "<li>Coût de revient total : <b><span style='color:#c0392b;'>%5 €</span></b></li>"
                           "</ul><hr>"
                           "Prix de vente conseillé au public (Marge x2.5) : <b><span style='color:#27ae60; font-size:22px;'>%6 €</span></b>"
                           "</div>"
                           ).arg(p.designation).arg(coutMP).arg(coutMainOeuvre).arg(p.tempsFab).arg(coutTotal).arg(prixVente);
        desc->setText(html);
    } else {
        // AUCUN PRODUIT SÉLECTIONNÉ
        desc->setText("Veuillez sélectionner un produit dans l'onglet 'Liste des Produits', puis cliquez à nouveau sur le module Coût pour simuler sa marge.");
        desc->setStyleSheet("font-size: 16px; color: #7f8c8d; font-style: italic;");
        desc->setAlignment(Qt::AlignCenter);
    }

    l->addWidget(desc);
    l->addStretch();

    ui->tabWidgetProduits->setCurrentIndex(4); // Bascule sur l'onglet Coût
}

void MainWindow::showHistoriqueModeDialog() {
    if(ui->tabWidgetProduits->count() < 6) return; // Sécurité

    QWidget *ongletHist = ui->tabWidgetProduits->widget(5); // Index 5 = 6ème onglet
    if (ongletHist->layout()) { clearLayout(ongletHist->layout()); delete ongletHist->layout(); }

    QVBoxLayout *l = new QVBoxLayout(ongletHist);
    QLabel *titre = new QLabel("📜 HISTORIQUE DU CYCLE DE VIE");
    titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #5d4037; margin-bottom: 20px;");
    titre->setAlignment(Qt::AlignCenter); l->addWidget(titre);

    QTableView *tv = new QTableView();
    tv->setStyleSheet("background: white; border: 1px solid #d7ccc8;");
    tv->horizontalHeader()->setStretchLastSection(true);

    // Requête de démonstration : On va lire la table PRODUITS d'Oracle !
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_PRODUIT as Référence, DESIGNATION as Nom, COUT as Coût_Actuel FROM PRODUITS");
    tv->setModel(model);

    l->addWidget(tv);
    ui->tabWidgetProduits->setCurrentIndex(5); // Bascule sur l'onglet Historique
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


// --- MODULE IA : ESTIMATION DE TEMPS ---
void MainWindow::ouvrirIAPrediction() {
    int idx = ui->tablePlanif->currentRow();
    if(idx < 0) {
        alerteWarning("Assistant IA", "Veuillez d'abord sélectionner une commande dans la liste.");
        return;
    }

    CommandeInfo c = mesCommandes[idx];
    double vitesse = c.idProduit.contains("Sac") ? 20.0 : 50.0;
    int jours = std::ceil(c.quantite / vitesse) + 1;
    QDate nvFin = c.dateDebut.addDays(jours);

    // On écrit le résultat dans le label de l'onglet IA
    QString texte = QString(
                        "<h2 style='color:#9c27b0;'>🧠 ANALYSE PRÉDICTIVE IA</h2><br>"
                        "L'intelligence artificielle a analysé la production de <b>%1</b> (Qté: %2).<br><br>"
                        "👉 <b>Vitesse moyenne :</b> %3 pièces / jour<br>"
                        "👉 <b>Durée optimale :</b> %4 jours<br>"
                        "👉 <b>Nouvelle date recommandée :</b> <span style='color:#d32f2f; font-weight:bold;'>%5</span>")
                        .arg(c.idProduit).arg(c.quantite).arg(vitesse).arg(jours).arg(nvFin.toString("dd/MM/yyyy"));

    ui->lbl_ia_details->setText(texte);

    // AJOUTEZ CES 3 LIGNES ICI :
    ui->lbl_ia_details->setStyleSheet("font-size: 15px; color: #3e2723; background: white; padding: 20px; border-radius: 10px; border: 2px dashed #ce93d8;");
    ui->lbl_ia_details->setAlignment(Qt::AlignCenter);
    ui->lbl_ia_details->setWordWrap(true);

    // On cache l'ID et la nouvelle date dans le bouton pour les réutiliser au clic
    ui->btn_ia_appliquer->setProperty("id_cmd", c.id.replace("OF-", "").toInt());
    ui->btn_ia_appliquer->setProperty("nv_fin", nvFin);

    // On bascule sur l'onglet IA (Index 4)
    ui->tabWidgetPlanif->setCurrentIndex(4);
}

void MainWindow::preparerFormulaireModif(int idx) {
    indexModification = idx;
    CommandeInfo c = mesCommandes[idx];

    // Remplissage dynamique des combobox de modif
    ui->cb_produit_modif->clear();
    QSqlQuery qProd("SELECT ID_PRODUIT, DESIGNATION FROM PRODUITS");
    while(qProd.next()) ui->cb_produit_modif->addItem(qProd.value("DESIGNATION").toString(), qProd.value("ID_PRODUIT"));

    ui->cb_matiere_modif->clear();
    QSqlQuery qMat("SELECT ID_STOCK_MP, CODE_MP FROM MATIERES_PREMIERES");
    while(qMat.next()) ui->cb_matiere_modif->addItem(qMat.value("CODE_MP").toString(), qMat.value("ID_STOCK_MP"));

    ui->cb_employe_modif->clear();
    QSqlQuery qEmp("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES");
    while(qEmp.next()) ui->cb_employe_modif->addItem(qEmp.value("NOM").toString() + " " + qEmp.value("PRENOM").toString(), qEmp.value("ID_EMPLOYE"));

    // Pré-sélection
    ui->cb_produit_modif->setCurrentText(c.idProduit);
    ui->cb_matiere_modif->setCurrentText(c.idMatiere);
    ui->cb_employe_modif->setCurrentText(c.idEmploye);
    ui->sb_qte_modif->setValue(c.quantite);
    ui->dt_lancement_modif->setDate(c.dateDebut);
    ui->le_fin_prevue_modif->setText(c.dateFinEstimee);

    // On bascule sur l'onglet Modifier (Index 3)
    ui->tabWidgetPlanif->setCurrentIndex(3);
}

// =========================================================
// ===        MODULE RH : FONCTIONS ET ONGLETS           ===
// =========================================================

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

        ui->tabWidgetEmployes->setCurrentIndex(2); // Bascule sur Modifier
    } else {
        ui->le_emp_nom->clear(); ui->le_emp_pre->clear();
        ui->le_emp_email->clear(); ui->le_emp_tel->clear(); ui->le_emp_rfid->clear();
        ui->sb_emp_sal->setValue(1500.0);
        ui->dt_emp_emb->setDate(QDate::currentDate());

        ui->tabWidgetEmployes->setCurrentIndex(1); // Bascule sur Recrutement
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
    kpiL->addWidget(creerCarteStat("💸", QString::number(masseSal) + " €", "Masse Salariale", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FF5F6D, stop:1 #FFC371)"));
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

    mainL->addWidget(framePie); mainL->addStretch();
    ui->tabWidgetEmployes->setCurrentIndex(3);
}

void MainWindow::showEmpEvalTab() {
    if(ui->tabWidgetEmployes->count() < 5) return;
    QWidget *onglet = ui->tabWidgetEmployes->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *l = new QVBoxLayout(onglet);
    l->addStretch(); // Pousse vers le centre

    QLabel *titre = new QLabel("📈 ÉVALUATION DES COMPÉTENCES");
    titre->setStyleSheet("font-size: 24px; font-weight: bold; color: #e67e22; margin-bottom: 20px;");
    titre->setAlignment(Qt::AlignCenter);
    l->addWidget(titre);

    int idx = ui->tableEmployes->currentRow();
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
    l->addWidget(desc, 0, Qt::AlignCenter); // Centre la carte horizontalement

    l->addStretch(); // Pousse vers le centre
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

    int idx = ui->tableEmployes->currentRow();
    QLabel *desc = new QLabel();
    if(idx >= 0 && idx < mesEmployes.size()) {
        EmployeInfo e = mesEmployes[idx];
        int annees = e.dateEmbauche.daysTo(QDate::currentDate()) / 365;
        double prime = annees * 50.0; // Exemple : 50€ par année
        desc->setText(QString(
                          "<div style='background:white; border-radius:12px; padding:30px; border:2px solid #3498db; color:#3e2723; font-size:16px;'>"
                          "<h2 style='color:#2980b9; margin-top:0; text-align:center;'>%1 %2</h2><hr>"
                          "Date d'embauche : <b>%3</b><br><br>"
                          "Ancienneté calculée : <b>%4 ans</b><br><br><hr>"
                          "Prime d'ancienneté estimée : <b style='color:#27ae60; font-size:22px;'>%5 €</b>"
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
    l->addStretch(); // Centre verticalement

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
        "<li><b>Recrutement :</b> Il manque 1 profil <i>Coupeur</i> pour atteindre l'objectif de production de la collection Hiver.</li>"
        "</ul></div>"
        );
    desc->setAlignment(Qt::AlignCenter);

    l->addWidget(desc, 0, Qt::AlignCenter); // Centre horizontalement
    l->addStretch();

    ui->tabWidgetEmployes->setCurrentIndex(6);
}

// =========================================================
// ===      MODULE DÉPÔT : FONCTIONS ET ONGLETS          ===
// =========================================================

void MainWindow::preparerFormulaireDepot(bool estModif, int idx) {
    if(estModif && idx >= 0 && idx < mesDepots.size()) {
        indexModifDepot = idx;
        const auto &dp = mesDepots[idx];
        
        ui->le_depot_id_modif->setText(dp.id);
        ui->le_depot_emp_modif->setText(dp.emplacement);
        ui->le_depot_eta_modif->setText(dp.etagere);
        ui->sb_depot_cap_modif->setValue(dp.capaciteMax);
        ui->sb_depot_act_modif->setValue(dp.quantiteActuelle);
        ui->cb_depot_type_modif->setCurrentText(dp.typeStockage);
        
        ui->tabWidgetDepot->setCurrentIndex(2); // Bascule sur Modifier
    } else {
        ui->le_depot_id->clear();
        ui->le_depot_emp->clear();
        ui->le_depot_eta->clear();
        ui->sb_depot_cap->setValue(100.0);
        ui->sb_depot_act->setValue(0.0);
        
        ui->tabWidgetDepot->setCurrentIndex(1); // Bascule sur Ajouter
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

    // Cartes KPI
    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->addWidget(creerCarteStat("🏭", QString::number(mesDepots.size()), "Zones de Stockage", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)"));
    
    QString colorTaux = (taux > 85) ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EB3349, stop:1 #F45C43)" : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)";
    kpiL->addWidget(creerCarteStat("📦", QString::number(taux, 'f', 1) + "%", "Taux de Remplissage", colorTaux));
    
    kpiL->addWidget(creerCarteStat("🧊", QString::number(parType["Froid"]), "Zones Réfrigérées", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #56ccf2, stop:1 #2f80ed)"));
    mainL->addLayout(kpiL);

    // Graphique
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

void MainWindow::on_btn_valider_emp_clicked()
{
    employe e(
        0,
        ui->le_emp_nom->text(),
        ui->le_emp_pre->text(),
        ui->cb_emp_poste->currentText(),   // QComboBox
        ui->le_emp_email->text(),
        ui->le_emp_tel->text(),
        ui->cb_emp_dept->currentText(),    // QComboBox
        ui->dt_emp_emb->date(),            // QDateEdit
        ui->sb_emp_sal->value(),           // QDoubleSpinBox
        ui->le_emp_rfid->text()
        );

    if(e.ajouter()){
        rafraichirListeEmployes();
        alerteSucces("RH", "Employé ajouté avec succès");
    } else {
        alerteErreur("RH", "Erreur lors de l'ajout");
    }
}


void MainWindow::on_btn_valider_modif_emp_clicked()
{
    qDebug() << "=== BTN MODIF CLICKED ===";

    int id = idEmployeAModifier;
    qDebug() << "idEmployeAModifier =" << id;

    if (id < 0) {
        alerteWarning("RH", "Aucun employé sélectionné");
        return;
    }

    if (!ui) {
        qDebug() << "ui is null";
        return;
    }

    employe e(
        id,
        ui->le_emp_nom_modif->text(),
        ui->le_emp_pre_modif->text(),
        ui->cb_emp_poste_modif->currentText(),
        ui->le_emp_email_modif->text(),
        ui->le_emp_tel_modif->text(),
        ui->cb_emp_dept_modif->currentText(),
        ui->dt_emp_emb_modif->date(),
        ui->sb_emp_sal_modif->value(),
        ui->le_emp_rfid_modif->text()
        );

    qDebug() << "avant modifier()";

    bool ok = e.modifier(id);

    qDebug() << "apres modifier(), ok =" << ok;

    if (!ok) {
        alerteErreur("RH", "Erreur lors de la modification");
        return;
    }

    qDebug() << "modification SQL OK";

    idEmployeAModifier = -1;

    // 1) refresh d'abord
    qDebug() << "avant rafraichirListeEmployes()";
    rafraichirListeEmployes();
    qDebug() << "apres rafraichirListeEmployes()";

    // 2) changer onglet après
    qDebug() << "avant goToTabEmployes(0)";
    forceTabEmployes(0);
    qDebug() << "apres forceTabEmployes(0)";

    alerteSucces("RH", "Employé modifié avec succès");
}
void MainWindow::on_btn_delete_emp_clicked()
{
    int row = ui->tableEmployes->currentRow();
    if(row < 0 && ui->tableEmployes->currentItem())
        row = ui->tableEmployes->currentItem()->row();

    if(row < 0){
        alerteWarning("RH", "Clique sur une ligne ثم اضغط Modifier");
        return;
    }
    if (row < 0) {
        alerteWarning("RH", "Sélectionnez un employé à supprimer");
        return;
    }

    int id = ui->tableEmployes->item(row, 0)->text().toInt();

    if (tmpEmploye.supprimer(id)) {
        rafraichirListeEmployes();
        alerteSucces("RH", "Employé supprimé");
    } else {
        alerteErreur("RH", "Erreur lors de la suppression");
    }

}
void MainWindow::goToTabEmployesByText(const QString& title)
{
    auto *tw = ui->tabWidgetEmployes;   // ✅ استعمل نفس tabWidget مرة وحدة

    if(!tw){
        qDebug() << "tabWidgetEmployes is null";
        return;
    }

    int c = tw->count();
    qDebug() << "Want tab =" << title << "count=" << c;

    for(int i = 0; i < c; ++i){
        QString t = tw->tabText(i).trimmed();
        qDebug() << "tab[" << i << "] =" << t;

        if(t == title.trimmed()){
            tw->setCurrentIndex(i);
            qDebug() << "SWITCH TAB ->" << title << " index =" << i;
            return;
        }
    }

    qDebug() << "TAB NOT FOUND:" << title;
}
void MainWindow::on_btn_edit_emp_clicked()
{
    qDebug() << "MODIFIER LOUTA CLICKED";

    if(!ui || !ui->tableEmployes) return;

    int row = ui->tableEmployes->currentRow();
    if(row < 0){
        alerteWarning("RH", "Sélectionne un employé dans le tableau");
        return;
    }

    QTableWidgetItem *it = ui->tableEmployes->item(row,0);
    if(!it){
        alerteWarning("RH", "ID introuvable");
        return;
    }


    idEmployeAModifier = it->text().toInt();

    if(!chargerEmployePourModification(idEmployeAModifier)){
        alerteErreur("RH", "Erreur chargement employé");
        return;
    }

    // نبدّل التاب بعد شوية باش ما يرجعوش للـ Add
    QTimer::singleShot(10, this, [this](){
        if(ui && ui->tabWidgetEmployes)
            ui->tabWidgetEmployes->setCurrentIndex(2); // Modifier Dossier
    });
}void MainWindow::goToTabEmployes(int index)
{
    if (!ui || !ui->tabWidgetEmployes) return;

    const int count = ui->tabWidgetEmployes->count();
    if (index < 0 || index >= count) return;

    QSignalBlocker block(ui->tabWidgetEmployes);   // ✅ يمنع currentChanged مؤقتاً
    ui->tabWidgetEmployes->setCurrentIndex(index);
}
bool MainWindow::chargerEmployePourModification(int id)
{
    // --- Protections anti-crash ---
    if (!ui) return false;
    if (!ui->cb_emp_poste_modif || !ui->cb_emp_dept_modif ||
        !ui->dt_emp_emb_modif  || !ui->sb_emp_sal_modif  ||
        !ui->le_emp_nom_modif  || !ui->le_emp_pre_modif  ||
        !ui->le_emp_email_modif|| !ui->le_emp_tel_modif  || !ui->le_emp_rfid_modif)
    {
        qDebug() << "UI widgets missing (nullptr) in chargerEmployePourModification";
        return false;
    }

    // Block signals (important to avoid currentIndexChanged side effects)
    QSignalBlocker bPoste(ui->cb_emp_poste_modif);
    QSignalBlocker bDept (ui->cb_emp_dept_modif);
    QSignalBlocker bDate (ui->dt_emp_emb_modif);
    QSignalBlocker bSal  (ui->sb_emp_sal_modif);

    // DB check
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "DB invalid or closed in chargerEmployePourModification";
        return false;
    }

    QSqlQuery q(db);
    q.prepare(
        "SELECT NOM, PRENOM, EMAIL, TELEPHONE, RFID_TAG, POSTE, DEPARTEMENT, DATE_EMBAUCHE, SALAIRE "
        "FROM EMPLOYES WHERE ID_EMPLOYE = :id"
        );
    q.bindValue(":id", id);

    if (!q.exec() || !q.next()) {
        qDebug() << "LOAD EMP FAIL:" << q.lastError().text() << "ID=" << id;
        return false;
    }

    // Fill line edits
    ui->le_emp_nom_modif->setText(q.value(0).toString());
    ui->le_emp_pre_modif->setText(q.value(1).toString());
    ui->le_emp_email_modif->setText(q.value(2).toString());
    ui->le_emp_tel_modif->setText(q.value(3).toString());
    ui->le_emp_rfid_modif->setText(q.value(4).toString());

    // Poste / Département (safe even if combobox empty or value not in list)
    const QString poste = q.value(5).toString().trimmed();
    const QString dept  = q.value(6).toString().trimmed();

    auto ensureComboValue = [](QComboBox* cb, const QString& value)
    {
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

        if (idx >= 0 && idx < cb->count())
            cb->setCurrentIndex(idx);
    };

    ensureComboValue(ui->cb_emp_poste_modif, poste);
    ensureComboValue(ui->cb_emp_dept_modif,  dept);

    // Date (Oracle sometimes returns QDate, QDateTime, or string)
    QDate d = q.value(7).toDate();
    if (!d.isValid())
        d = q.value(7).toDateTime().date();
    if (!d.isValid()) {
        const QString ds = q.value(7).toString().trimmed();
        d = QDate::fromString(ds, "yyyy-MM-dd");
        if (!d.isValid()) d = QDate::fromString(ds, "dd/MM/yyyy");
    }
    if (!d.isValid()) d = QDate::currentDate(); // fallback
    ui->dt_emp_emb_modif->setDate(d);

    // Salaire safe
    bool okSal = false;
    const double sal = q.value(8).toDouble(&okSal);
    ui->sb_emp_sal_modif->setValue(okSal ? sal : 0.0);

    // Debug useful (optional)
    qDebug() << "LOAD EMP OK id=" << id
             << "poste=" << poste << "dept=" << dept
             << "posteCount=" << ui->cb_emp_poste_modif->count()
             << "deptCount=" << ui->cb_emp_dept_modif->count();

    return true;
}
void MainWindow::on_tabWidgetEmployes_currentChanged(int index)
{

}
void MainWindow::forceTabEmployes(int index)
{
    if (!ui || !ui->tabWidgetEmployes) return;

    const int count = ui->tabWidgetEmployes->count();
    qDebug() << "forceTabEmployes index=" << index << "count=" << count;

    if (index < 0 || index >= count) {
        qDebug() << "forceTabEmployes BAD index";
        return;
    }

    QTimer::singleShot(0, this, [this, index]() {
        if (!ui || !ui->tabWidgetEmployes) return;

        const int c = ui->tabWidgetEmployes->count();
        if (index >= 0 && index < c) {
            ui->tabWidgetEmployes->setCurrentIndex(index);
            qDebug() << "forceTabEmployes OK -> index =" << index;
        }
    });
}
