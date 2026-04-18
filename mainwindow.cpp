#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "depot.h"
#include "employe.h"
#include "client.h"

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
#include <QScrollArea>
#include <QTimer>
#include <QSignalBlocker>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMargins>
#include <QColor>
#include <QFont>
#include <QDoubleSpinBox>
#include <QEasingCurve>
#include <QDialogButtonBox>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>

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

/// Retourne un message d'erreur non vide si la saisie est invalide
/// (ajout / modification produit).
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

/// Validation saisie client (UI avant BDD). Retourne message d’erreur ou chaîne vide si OK.
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

/// ID manuel à l’ajout : vide = séquence automatique ; sinon entier > 0.
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

/// Conformément au MCD : chaque produit est lié à exactement un client (« Acheter ») et un dépôt (« stocker »).
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

/// Si la chaîne est vide, retourne \a siVide (alors considérée comme valide).
/// Sinon parse jj/MM/aaaa, aaaa-MM-jj ou ISO ; en cas d'échec, \a errMsg et date invalide.
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
/// Cellule dont le tri utilise une clé numérique (évite le tri « texte »).
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
} // namespace

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

    auto *series = new QPieSeries(); series->setHoleSize(0.52); series->setPieSize(0.74); series->setPieStartAngle(90);
    const QLocale loc = QLocale::system(); QStringList names;
    for(const auto &s : sorted) { if(s.second <= 0.0) continue; names << s.first; series->append(s.first, s.second); }

    QFont sliceFont; sliceFont.setFamily("Segoe UI"); sliceFont.setPointSize(11); sliceFont.setBold(true);
    const QList<QColor> palette = chartPaletteFILdOr();
    const auto sliceList = series->slices();
    for(int i = 0; i < sliceList.size(); ++i) {
        auto *slice = sliceList.at(i); if(!slice) continue;
        const QColor c = palette.at(i % palette.size());
        slice->setBrush(QBrush(c)); slice->setPen(QPen(QColor("#f3f0eb"), 2));
        slice->setLabelVisible(true); slice->setLabelFont(sliceFont);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setLabelArmLengthFactor(0.15);
        const double pct = (total > 0.0) ? (slice->value() * 100.0 / total) : 0.0;
        slice->setLabel(names.value(i) + "  " + loc.toString(pct, 'f', 1) + "%");
        slice->setLabelColor(QColor("#2b1a17"));
    }

    auto *chart = new QChart(); chart->addSeries(series); styleChartBase(chart);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    if(chart->legend()) {
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setBackgroundVisible(false);
        QFont legendFont = chart->legend()->font();
        legendFont.setPointSize(10);
        legendFont.setBold(true);
        chart->legend()->setFont(legendFont);
        chart->legend()->setLabelColor(QColor("#3e2723"));
    }
    Q_UNUSED(legendTitle);

    auto *view = new QChartView(chart); styleChartView(view);
    view->setMaximumSize(680, 320); view->setMinimumSize(460, 260);
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
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Stock : impossible de valider 0,00 — minimum aligné sur les règles métier (≥ 0,01)
    if (ui->sb_stock_qte) {
        ui->sb_stock_qte->setMinimum(0.01);
        ui->sb_stock_qte->setDecimals(2);
    }
    if (ui->sb_stock_qte_modif) {
        ui->sb_stock_qte_modif->setMinimum(0.01);
        ui->sb_stock_qte_modif->setDecimals(2);
    }
    // Validateurs ligne (saisie guidée — même motifs que validerMatiereAjout())
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

    // Logo sidebar - taille agrandie
    ui->l_logo_img->setMinimumSize(55, 55);
    ui->l_logo_img->setMaximumSize(55, 55);
    ui->l_logo_img->setPixmap(QPixmap(":/logo.png").scaled(55, 55, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->l_logo_img->setScaledContents(false);
    ui->l_logo_img->setAlignment(Qt::AlignCenter);
    ui->l_logo_img->setStyleSheet("border: none; background: transparent;");

    // Construire les pages d'accueil et connexion dynamiquement
    construirePageAccueil();
    construirePageLogin();
    // Ne PAS appeler construirePageEtapes() ici au démarrage
    // On le fera au clic sur le bouton navigation

    // --- CONNEXION ORACLE (Singleton) ---
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

    // Enable sorting
    ui->tablePlanif->setSortingEnabled(true);
    ui->tableProduits->setSortingEnabled(true);
    ui->tableEmployes->setSortingEnabled(true);
    ui->tableStock->setSortingEnabled(true);
    ui->tableClients->setSortingEnabled(true);
    ui->tableDepot->setSortingEnabled(true);

    // --- DONNEES LOCALES TEMPORAIRES ---
    mesProduits.append({"1", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette", 5, "1", "1"});
    mesEmployes.append({"1", "Dupont", "Jean", "Chef Atelier", "jean@fildor.tn", "55123456", "Production", QDate(2020, 5, 10), 2800.0, "RF-123"});
    mesClients.append({"1", "Ben Salah", "55 123 456", "Tunis", "client1@fildor.tn", 120});
    mesDepots.append({"1", "Zone A", "E1", 500.0, 320.0, "Sec"});

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
        rafraichirListeEtapes();
        remplirCombosProduitClientEmplacement();
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
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){
        static bool etapesInit = false;
        if (!etapesInit) {
            construirePageEtapes();
            etapesInit = true;
        } else {
            // Juste rafraîchir les données
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

    // ===================== DEPOT CRUD =====================

    // Bouton + Ajouter (onglet ajout)
    connect(ui->btn_add_depot, &QPushButton::clicked, [=](){
        ui->le_depot_id->clear();
        ui->le_depot_emp->clear();
        ui->le_depot_eta->clear();
        ui->sb_depot_cap->setValue(0);
        ui->sb_depot_act->setValue(0);
        ui->cb_depot_type->setCurrentIndex(0);
        ui->tabWidgetDepot->setCurrentIndex(1);
    });

    // ACTUALISER TIMELINE
    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){
        rafraichirListeEtapes();
    });

    // AJOUT
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

    // MODIFIER (pré-remplir)
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

    // VALIDER MODIF
    // --- BOUTON VALIDER LA MODIFICATION (Onglet 3) ---
    connect(ui->btn_valider_modif, &QPushButton::clicked, [=](){
        // Protection contre crash
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
            // Essayer sans le prefixe
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

    // SUPPRIMER
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

    // RECHERCHE
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

    // TRI A-Z
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

    // Le bouton btn_valider_modif est connecté plus haut avec validations renforcées.

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

            // ✅ Générer automatiquement les 4 étapes (seulement en mode ajout)
            if (!modeModification) {
                QSqlQuery qLastId;
                if (qLastId.exec("SELECT MAX(ID_COMMANDE) FROM PLANIFICATION") && qLastId.next()) {
                    int idNouvelleCommande = qLastId.value(0).toInt();

                    // Récupérer l'employé depuis le formulaire (déjà validé ; repli SQL si besoin)
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

                    // Vérifier que les étapes n'existent pas déjà
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

    // --- 2. PRODUITS (Navigation par onglets) ---
    ui->btn_valider_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_produit->setCursor(Qt::PointingHandCursor);

    ui->btn_valider_modif_produit->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_produit->setCursor(Qt::PointingHandCursor);

    // Changement d'onglet manuel (Stats + rechargement client/dépôt pour les FK)
    connect(ui->tabWidgetProduits, &QTabWidget::currentChanged, [=](int index){
        if (index == 1 || index == 2)
            remplirCombosProduitClientEmplacement();
        if (index == 3) ouvrirStatsProduits(); // Calcule les stats si on va sur l'onglet 4
    });

    // Bouton "+ Ajouter Produit" (Depuis la liste)
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        preparerFormulaireProduit(false);
    });

    // --- FILTRAGE PRODUITS ---
    connect(ui->btn_search_col, &QPushButton::clicked, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
    });
    connect(ui->le_search_coll, &QLineEdit::returnPressed, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
    });

    // --- TRI A→Z (Désignation) ---
    connect(ui->btn_sort_alpha_prod, &QPushButton::clicked, [=](){
        if(ui->tableProduits->rowCount() <= 0) {
            alerteInfo(QStringLiteral("Tri"), QStringLiteral("Aucune ligne à trier."));
            return;
        }
        ui->tableProduits->setSortingEnabled(true);
        const Qt::SortOrder ordre = m_triProduitDesignationDescendant
            ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->tableProduits->sortItems(1, ordre); // colonne 1 = Désignation
        m_triProduitDesignationDescendant = !m_triProduitDesignationDescendant;
    });

    // --- OUVRIR / ACTUALISER LES STATS PRODUITS ---
    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){
        rafraichirListeProduits(ui->le_search_coll->text());
        ouvrirStatsProduits();
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

    // === RH : CRUD basés sur Oracle (slots) ===
    // On NE met pas de connect() ici : Qt relie automatiquement les boutons
    // aux slots nommés on_<objectName>_clicked via setupUi().

    // =========================================================
    // --- RECHERCHE EMPLOYÉS ---
    // =========================================================
    // Fonction de recherche employé (réutilisée par clic et par "typing").
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
            ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre); // colonne 1 = NOM
        }

        delete model;
    };

    // Chercher au clic
    connect(ui->btn_search_emp, &QPushButton::clicked, this, performSearchEmployes);

    // Chercher automatiquement quand l'utilisateur tape (debounce 300ms)
    QTimer *searchDebounce = new QTimer(this);
    searchDebounce->setSingleShot(true);
    connect(ui->le_search_emp, &QLineEdit::textChanged, this, [=](){
        searchDebounce->start(300);
    });
    connect(searchDebounce, &QTimer::timeout, this, [=](){
        performSearchEmployes();
    });

    // =========================================================
    // --- 4. STOCK MATIÈRES (CRUD Oracle)
    // =========================================================

    // Styles boutons
    ui->btn_valider_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_stock->setCursor(Qt::PointingHandCursor);
    ui->btn_valider_modif_stock->setStyleSheet(styleBtnSave());
    ui->btn_valider_modif_stock->setCursor(Qt::PointingHandCursor);

    // Routage des onglets (Stats, Ravitaillement, Calculateur)
    connect(ui->tabWidgetStock, &QTabWidget::currentChanged, [=](int index){
        if (index == 3) ouvrirStatsStock();
        else if (index == 4) showStockRavitaillementTab();
        else if (index == 5) showStockCalculTab();
    });

    // --- BOUTON + AJOUTER MATIÈRE (bascule vers onglet formulaire) ---
    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){
        // Vider les champs
        ui->le_stock_code->clear();
        ui->le_stock_lot->clear();
        ui->le_stock_coul->clear();
        ui->sb_stock_qte->setValue(0.01);
        ui->cb_stock_cat->setCurrentIndex(0);
        ui->cb_stock_etat->setCurrentIndex(0);
        ui->cb_stock_type->setCurrentIndex(0);
        ui->cb_stock_qual->setCurrentIndex(0);
        ui->tabWidgetStock->setCurrentIndex(1); // Onglet Ajouter
    });

    // --- BOUTON VALIDER AJOUT matière : on_btn_valider_stock_clicked() (auto-connect setupUi) ---

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

        // Récupérer l'ID Oracle stocké dans la liste locale
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

    // --- BOUTON RECHERCHER (insensible a la casse + stockage) ---
    connect(ui->btn_search_stock, &QPushButton::clicked, [=](){
        QString critere = ui->le_search_stock->text().trimmed();
        if (critere.isEmpty()) {
            rafraichirListeMatieres();
            return;
        }

        QSqlQueryModel *model = tmpMatiere.afficher();

        if (!model) {
            alerteErreur("Recherche", "Impossible de charger les matieres premieres.");
            return;
        }

        ui->tableStock->setRowCount(0);
        mesMatieres.clear();

        int outRow = 0;
        const int rows = model->rowCount();

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

            const bool match =
                code.contains(critere, Qt::CaseInsensitive) ||
                cat.contains(critere, Qt::CaseInsensitive) ||
                lot.contains(critere, Qt::CaseInsensitive) ||
                etat.contains(critere, Qt::CaseInsensitive) ||
                coul.contains(critere, Qt::CaseInsensitive) ||
                qual.contains(critere, Qt::CaseInsensitive) ||
                type.contains(critere, Qt::CaseInsensitive) ||
                QString::number(qte).contains(critere, Qt::CaseInsensitive);

            if (!match)
                continue;

            ui->tableStock->insertRow(outRow);

            QTableWidgetItem *itemCode = new QTableWidgetItem(code);
            itemCode->setData(Qt::UserRole, idDb);
            ui->tableStock->setItem(outRow, 0, itemCode);
            ui->tableStock->setItem(outRow, 1, new QTableWidgetItem(cat));
            ui->tableStock->setItem(outRow, 2, new QTableWidgetItem(lot));
            ui->tableStock->setItem(outRow, 3, new QTableWidgetItem(etat));
            ui->tableStock->setItem(outRow, 4, new QTableWidgetItem(coul));
            ui->tableStock->setItem(outRow, 5, new QTableWidgetItem(qual));
            ui->tableStock->setItem(outRow, 6, new QTableWidgetItem(QString::number(qte)));
            ui->tableStock->setItem(outRow, 7, new QTableWidgetItem(type));

            MatiereInfo m = {QString::number(idDb), code, cat, lot, etat, coul, qte, type, qual};
            mesMatieres.append(m);
            ++outRow;
        }

        if (outRow == 0) {
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
    // Les handlers CRUD dépôt sont déjà connectés plus haut (version Oracle).
    // On évite ici les doubles connexions qui provoquent des actions en double.

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
        ui->tabWidgetDepot->setCurrentIndex(0); // Retour liste
        alerteSucces("Mise à jour", "Emplacement modifié avec succès.");
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
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_export_excel_stock, &QPushButton::clicked, [=](){ exporterCSV(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_print_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });
    connect(ui->btn_pdf_facture, &QPushButton::clicked, [=](){ exporterFactureClient(); });
    connect(ui->btn_export_excel_client, &QPushButton::clicked, [=](){ exporterCSV(ui->tableClients, "Clients"); });
    connect(ui->btn_pdf_depot, &QPushButton::clicked, [=](){ exporterPDF(ui->tableDepot, "Inventaire Dépôt"); });


    // Fabrication (Timeline)
    // Fabrication (Timeline)
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
        Etape tmpEt;
        tv->setModel(tmpEt.rechercherParCommande(idPlanif));// APPEL À LA BASE DE DONNÉES !
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
    // --- EMPLOYÉS (ComboBox Poste & Département)
    // =============================================
    // On remplit à partir de la base (valeurs distinctes).
    // Si la table est vide / indisponible, on met une liste de fallback.
    auto chargerComboDistinct = [&](QComboBox *cb, const QString &colName, const QStringList &fallback) {
        if(!cb) return;
        cb->clear();

        bool rempli = false;
        if(cnx && cnx->estConnecte()) {
            QSqlQuery q;
            // colName est une colonne fixe (pas d'utilisateur) => concat sûre ici.
            const QString sql = QString("SELECT DISTINCT %1 FROM EMPLOYES ORDER BY %1").arg(colName);
            if(q.exec(sql)) {
                while(q.next()) {
                    const QString v = q.value(0).toString().trimmed();
                    if(!v.isEmpty()) {
                        cb->addItem(v);
                        rempli = true;
                    }
                }
            }
        }

        if(!rempli) {
            cb->addItems(fallback);
        }

        if(cb->count() > 0) cb->setCurrentIndex(0);
    };

    chargerComboDistinct(
        ui->cb_emp_poste,
        "POSTE",
        {"Coupe", "Assemblage", "Couture", "Finition", "Contrôle Qualité", "Magasinier", "Chef Atelier"}
    );
    chargerComboDistinct(
        ui->cb_emp_dept,
        "DEPARTEMENT",
        {"Production", "Qualité", "Stock", "Logistique", "Administration", "RH"}
    );

    chargerComboDistinct(
        ui->cb_emp_poste_modif,
        "POSTE",
        {"Coupe", "Assemblage", "Couture", "Finition", "Contrôle Qualité", "Magasinier", "Chef Atelier"}
    );
    chargerComboDistinct(
        ui->cb_emp_dept_modif,
        "DEPARTEMENT",
        {"Production", "Qualité", "Stock", "Logistique", "Administration", "RH"}
    );

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

    // --- SCROLL AREA pour tout le contenu ---
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

    // =============================================
    // 1. EN-TÊTE
    // =============================================
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

    // =============================================
    // 2. REQUÊTES ORACLE
    // =============================================
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

    // =============================================
    // 3. HELPER CARTE KPI (compacte)
    // =============================================
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

    // =============================================
    // 4. LIGNE 1 : PRODUCTION
    // =============================================
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

    // =============================================
    // 5. LIGNE 2 : RESSOURCES
    // =============================================
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

    // =============================================
    // 6. LIGNE 3 : DÉPÔT & CLIENTÈLE
    // =============================================
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

    // =============================================
    // 7. SECTION BASSE : Commandes + Alertes
    // =============================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(12);

    // --- PANNEAU GAUCHE : Dernières commandes ---
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

    // --- PANNEAU DROIT : Alertes ---
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

    // Alertes dynamiques
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

    // =============================================
    // 8. FOOTER
    // =============================================
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

    // --- Finaliser le scroll ---
    scrollArea->setWidget(scrollContent);
    pageLayout->addWidget(scrollArea);
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
    ui->tableTimeline->setItem(r, c, it); ui->tableTimeline->setSpan(r, c, 1, d);
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

// Rafraichissement listes locales (lecture via CRUD Produit::afficher — même requête que create/update)
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

// =========================================================
// ===                   CRUD PRODUITS                 ===
// =========================================================

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

// =========================================================
// ===                   CRUD CLIENTS                  ===
// =========================================================

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
        // Recharger la page liste (pas l'onglet "Ajouter" pour garder le workflow).
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

    // Rafraîchir depuis Oracle via l'entité.
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
        ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre); // colonne 1 = NOM
    }

    delete model;
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

        // Stocker l'ID dans UserRole (sur la colonne Étagère)
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
// ===                   ETAPES / TIMELINE               ===
// =========================================================

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

        ui->tabWidgetProduits->setCurrentIndex(2); // Modifier
    } else {
        // On vide l'onglet Ajout
        selectedProdId = -1;
        rowToEdit = -1;

        ui->le_prod_nom->clear();
        ui->sb_prod_cout->setValue(0);
        ui->sb_prod_temps->setValue(1);
        reglerComboParIdDonnee(ui->cb_prod_client, 0);
        reglerComboParIdDonnee(ui->cb_prod_empl, 0);

        ui->tabWidgetProduits->setCurrentIndex(1); // Ajouter
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
    kpiL->addWidget(creerCarteStat("💰", QString::number(moy, 'f', 1) + " DT", "Coût Moyen de Fab.", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1c92d2, stop:1 #f2fcfe)"));

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

    QVBoxLayout *rootL = new QVBoxLayout(onglet);
    rootL->setContentsMargins(0, 0, 0, 0);
    rootL->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(onglet);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        " background: rgba(93,64,55,0.18);"
        " width: 14px;"
        " margin: 8px 2px 8px 2px;"
        " border-radius: 7px;"
        "}"
        "QScrollBar::handle:vertical {"
        " background: #d4af37;"
        " min-height: 34px;"
        " border-radius: 7px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #e6c65a; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: transparent; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContent);
    rootL->addWidget(scrollArea);

    QVBoxLayout *mainL = new QVBoxLayout(scrollContent);
    mainL->setSpacing(20); mainL->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("TABLEAU DE BORD - MATIÈRES PREMIÈRES");
    t->setStyleSheet("font-size: 20px; font-weight: 900; color: white; background-color: #5d4037; border-radius: 10px; padding: 15px; letter-spacing: 2px; text-transform: uppercase;");
    t->setAlignment(Qt::AlignCenter); mainL->addWidget(t);

    double volume = 0;
    double sommeQual = 0;
    int nQual = 0;
    int lotsCritiques = 0;
    int lotsPremium = 0;
    QMap<QString, double> parCat;
    QMap<QString, double> parQual;

    for(const auto &m : mesMatieres) {
        volume += m.quantite;
        parCat[m.categorie] += 1;
        parQual[m.qualite] += 1;

        if (m.quantite <= 20.0)
            ++lotsCritiques;

        const QString q = m.qualite.trimmed().toUpper();
        if (q.startsWith("A") || q.contains("PREMIUM"))
            ++lotsPremium;

        if (q.startsWith("A")) { sommeQual += 4.0; ++nQual; }
        else if (q.startsWith("B")) { sommeQual += 3.0; ++nQual; }
        else if (q.startsWith("C")) { sommeQual += 2.0; ++nQual; }
        else if (q.startsWith("D")) { sommeQual += 1.0; ++nQual; }
    }

    const double moyenneParLot = mesMatieres.isEmpty() ? 0.0 : (volume / static_cast<double>(mesMatieres.size()));
    const double scoreQualite = (nQual > 0) ? (sommeQual / static_cast<double>(nQual) * 25.0) : 0.0;

    QString catDominante = "-";
    double maxCat = -1;
    for (auto it = parCat.begin(); it != parCat.end(); ++it) {
        if (it.value() > maxCat) { maxCat = it.value(); catDominante = it.key(); }
    }

    QString qualDominante = "-";
    double maxQual = -1;
    for (auto it = parQual.begin(); it != parQual.end(); ++it) {
        if (it.value() > maxQual) { maxQual = it.value(); qualDominante = it.key(); }
    }

    QHBoxLayout *kpiL1 = new QHBoxLayout();
    kpiL1->setSpacing(12);
    kpiL1->addWidget(creerCarteStat("📦", QString::number(mesMatieres.size()), "Lots Référencés", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4568dc, stop:1 #b06ab3)"));
    kpiL1->addWidget(creerCarteStat("📏", QString::number(volume, 'f', 1) + " u", "Volume Total", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e1eec3, stop:1 #f05053)"));
    kpiL1->addWidget(creerCarteStat("⚠️", QString::number(lotsCritiques), "Lots Critiques", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f857a6, stop:1 #ff5858)"));
    mainL->addLayout(kpiL1);

    QHBoxLayout *kpiL2 = new QHBoxLayout();
    kpiL2->setSpacing(12);
    kpiL2->addWidget(creerCarteStat("🏅", QString::number(lotsPremium), "Lots Premium", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f6d365, stop:1 #fda085)"));
    kpiL2->addWidget(creerCarteStat("📊", QString::number(scoreQualite, 'f', 1) + "%", "Indice Qualité", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d)"));
    kpiL2->addWidget(creerCarteStat("📌", catDominante, "Catégorie Dominante", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7f7fd5, stop:1 #86a8e7)"));
    mainL->addLayout(kpiL2);

    QHBoxLayout *chartsL = new QHBoxLayout();

    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Catégorie"); titrePie->setStyleSheet("color: #8d5524; font-weight: bold; font-size: 15px; padding: 5px;"); layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slices;
    for(auto k : parCat.keys()) slices.append({k, parCat[k]});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie); chartsL->addWidget(framePie);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Qualité des lots"); titreBar->setStyleSheet("color: #8d5524; font-weight: bold; font-size: 15px; padding: 5px;"); layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget(); QStringList cats = parQual.keys();
    QList<double> vals; for(auto k : cats) vals << parQual[k];
    setVerticalBarChart(wBar, "", cats, vals); layoutBar->addWidget(wBar); chartsL->addWidget(frameBar);

    mainL->addLayout(chartsL);

    QHBoxLayout *detailsL = new QHBoxLayout();
    detailsL->setSpacing(12);

    QFrame *frameSynth = new QFrame();
    frameSynth->setStyleSheet("QFrame { background: #fffaf4; border-radius: 10px; border: 1px solid #e4d8c9; }");
    QVBoxLayout *synthL = new QVBoxLayout(frameSynth);
    QLabel *synthTitle = new QLabel("Analyse Détaillée");
    synthTitle->setStyleSheet("color: #8d5524; font-weight: bold; font-size: 15px;");
    QLabel *synthText = new QLabel(QString(
        "• Volume moyen par lot : <b>%1 u</b><br>"
        "• Qualité dominante : <b>%2</b><br>"
        "• Catégories suivies : <b>%3</b><br>"
        "• Part lots premium : <b>%4%</b><br>"
        "• Niveau de risque stock : <b>%5</b>"
    )
        .arg(QString::number(moyenneParLot, 'f', 1))
        .arg(qualDominante)
        .arg(parCat.size())
        .arg(mesMatieres.isEmpty() ? QStringLiteral("0.0") : QString::number((lotsPremium * 100.0) / mesMatieres.size(), 'f', 1))
        .arg(lotsCritiques == 0 ? "FAIBLE" : (lotsCritiques <= 2 ? "MOYEN" : "ELEVÉ"))
    );
    synthText->setTextFormat(Qt::RichText);
    synthText->setStyleSheet("font-size: 13px; color: #3e2723; line-height: 1.6;");
    synthText->setWordWrap(true);
    synthL->addWidget(synthTitle);
    synthL->addWidget(synthText);

    QFrame *frameCrit = new QFrame();
    frameCrit->setStyleSheet("QFrame { background: #fff8f8; border-radius: 10px; border: 1px solid #efc9c9; }");
    QVBoxLayout *critL = new QVBoxLayout(frameCrit);
    QLabel *critTitle = new QLabel("Lots à Surveiller");
    critTitle->setStyleSheet("color: #9c2f2f; font-weight: bold; font-size: 15px;");

    QTableWidget *tblCrit = new QTableWidget();
    tblCrit->setColumnCount(3);
    tblCrit->setHorizontalHeaderLabels(QStringList() << "Code" << "Catégorie" << "Qté");
    tblCrit->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblCrit->verticalHeader()->setVisible(false);
    tblCrit->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblCrit->setSelectionMode(QAbstractItemView::NoSelection);
    tblCrit->setMinimumHeight(150);
    tblCrit->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #ecd0d0; border-radius: 6px; gridline-color: #f1e2e2; }"
        "QHeaderView::section { background-color: #d46a6a; color: white; border: none; padding: 5px; font-weight: 700; }"
    );

    QVector<MatiereInfo> tri = mesMatieres;
    std::sort(tri.begin(), tri.end(), [](const MatiereInfo &a, const MatiereInfo &b) {
        return a.quantite < b.quantite;
    });

    const int nRows = qMin(5, tri.size());
    tblCrit->setRowCount(nRows > 0 ? nRows : 1);
    if (nRows == 0) {
        tblCrit->setItem(0, 0, new QTableWidgetItem("-"));
        tblCrit->setItem(0, 1, new QTableWidgetItem("Aucune donnée"));
        tblCrit->setItem(0, 2, new QTableWidgetItem("0"));
    } else {
        for (int i = 0; i < nRows; ++i) {
            const MatiereInfo &m = tri[i];
            tblCrit->setItem(i, 0, new QTableWidgetItem(m.code));
            tblCrit->setItem(i, 1, new QTableWidgetItem(m.categorie));
            tblCrit->setItem(i, 2, new QTableWidgetItem(QString::number(m.quantite, 'f', 2)));
            if (m.quantite <= 20.0) {
                for (int c = 0; c < 3; ++c) {
                    if (auto *it = tblCrit->item(i, c)) {
                        it->setBackground(QBrush(QColor("#ffe9e9")));
                        it->setForeground(QBrush(QColor("#7a1f1f")));
                    }
                }
            }
        }
    }

    critL->addWidget(critTitle);
    critL->addWidget(tblCrit);

    detailsL->addWidget(frameSynth, 1);
    detailsL->addWidget(frameCrit, 1);
    mainL->addLayout(detailsL);
    mainL->addStretch();
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
    const int idx = ui->tableProduits->currentRow();

    if(idx >= 0 && idx < ui->tableProduits->rowCount()) {
        // Calcul basé sur la ligne de la table (fiable après tri/filtre).
        auto *itDes = ui->tableProduits->item(idx, 1);
        auto *itCout = ui->tableProduits->item(idx, 2);
        auto *itTemp = ui->tableProduits->item(idx, 5);

        const QString designation = itDes ? itDes->text() : QString();
        const double coutMP = itCout ? itCout->text().toDouble() : 0.0;
        const int tempsFab = itTemp ? itTemp->text().toInt() : 0;

        const double coutMainOeuvre = tempsFab * 15.5; // Base: 15.5 DT/h par artisan
        const double coutTotal = coutMP + coutMainOeuvre;
        const double prixVente = coutTotal * 2.5; // La marge Fil d'Or

        QString html = QString(
                           "<div style='background: white; border: 1px solid #d7ccc8; border-radius: 10px; padding: 20px; font-size: 15px; color: #3e2723;'>"
                           "<h3 style='color:#8d5524; margin-top:0;'>Analyse Financière : %1</h3><hr>"
                           "<ul>"
                           "<li>Coût Matière Première (Cuir/Fil) : <b>%2 DT</b></li>"
                           "<li>Coût Main d'Oeuvre estimé : <b>%3 DT</b> (%4 h)</li>"
                           "<li>Coût de revient total : <b><span style='color:#c0392b;'>%5 DT</span></b></li>"
                           "</ul><hr>"
                           "Prix de vente conseillé au public (Marge x2.5) : <b><span style='color:#27ae60; font-size:22px;'>%6 DT</span></b>"
                           "</div>"
                           ).arg(designation).arg(coutMP).arg(coutMainOeuvre).arg(tempsFab).arg(coutTotal).arg(prixVente);
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

void MainWindow::showStockRavitaillementTab() {
    if(ui->tabWidgetStock->count() < 5) return;
    QWidget *onglet = ui->tabWidgetStock->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *rootLayout = new QVBoxLayout(onglet);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(onglet);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        " background: #efefef;"
        " width: 12px;"
        " margin: 8px 4px 8px 2px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        " background: #cba731;"
        " min-height: 46px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #d6b64b; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: transparent; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(scrollArea);

    QVBoxLayout *l = new QVBoxLayout(scrollContent);
    l->setContentsMargins(8, 8, 8, 8);
    l->setSpacing(10);

    QLabel *titre = new QLabel("Ravitaillement Intelligent des Matières Premières");
    titre->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titre->setStyleSheet(
        "font-size: 32px;"
        "font-weight: 900;"
        "color: #0f2f3a;"
        "background-color: #d8edf7;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
    );
    l->addWidget(titre);

    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setSpacing(10);

    QLabel *lblMatiere = new QLabel("Matière à ravitailler :");
    lblMatiere->setStyleSheet("font-size: 22px; font-weight: 700; color: #212121;");

    QComboBox *cbMatiere = new QComboBox();
    cbMatiere->addItem("Cuir Vachette");
    cbMatiere->addItem("Cuir Agneau");
    cbMatiere->addItem("Cuir Veau");
    cbMatiere->setCurrentIndex(0);
    cbMatiere->setFixedHeight(42);
    cbMatiere->setMinimumWidth(300);
    cbMatiere->setStyleSheet(
        "QComboBox {"
        " background: white;"
        " border: 1px solid #a1887f;"
        " border-radius: 10px;"
        " padding: 4px 12px;"
        " font-size: 20px;"
        " color: #3e2723;"
        "}"
        "QComboBox::drop-down {"
        " border: none;"
        " width: 28px;"
        "}"
    );

    QPushButton *btnComparer = new QPushButton("Analyser fournisseurs");
    btnComparer->setCursor(Qt::PointingHandCursor);
    btnComparer->setFixedHeight(42);
    btnComparer->setMinimumWidth(250);
    btnComparer->setStyleSheet(
        "QPushButton {"
        " background-color: #0b9096;"
        " color: white;"
        " border: none;"
        " border-radius: 21px;"
        " font-size: 18px;"
        " font-weight: 700;"
        " padding: 4px 18px;"
        "}"
        "QPushButton:hover { background-color: #0ea4ab; }"
        "QPushButton:pressed { background-color: #087a7f; }"
    );

    topBar->addWidget(lblMatiere);
    topBar->addWidget(cbMatiere);
    topBar->addWidget(btnComparer, 1);
    l->addLayout(topBar);

    QFrame *photoFrame = new QFrame(onglet);
    photoFrame->setStyleSheet(
        "QFrame {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #fef9ef, stop:1 #fffdf7);"
        " border: 1px solid #ead8b6;"
        " border-radius: 12px;"
        "}"
    );
    QVBoxLayout *photoL = new QVBoxLayout(photoFrame);
    photoL->setContentsMargins(10, 10, 10, 10);
    photoL->setSpacing(8);

    QLabel *photoTitle = new QLabel("Aperçu matière sélectionnée");
    photoTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #4d331f;");

    QLabel *photoCuir = new QLabel();
    photoCuir->setMinimumHeight(190);
    photoCuir->setMaximumHeight(220);
    photoCuir->setAlignment(Qt::AlignCenter);
    photoCuir->setStyleSheet(
        "QLabel {"
        " background: #ffffff;"
        " border: 1px solid #d9c7a5;"
        " border-radius: 10px;"
        " padding: 6px;"
        "}"
    );

    QLabel *photoSousTitre = new QLabel();
    photoSousTitre->setAlignment(Qt::AlignCenter);
    photoSousTitre->setWordWrap(true);
    photoSousTitre->setStyleSheet("font-size: 12px; color: #6e4f30; font-weight: 700;");

    photoL->addWidget(photoTitle);
    photoL->addWidget(photoCuir);
    photoL->addWidget(photoSousTitre);
    l->addWidget(photoFrame);

    QFrame *pilotFrame = new QFrame(onglet);
    pilotFrame->setStyleSheet(
        "QFrame {"
        " background: #f4f8fb;"
        " border: 1px solid #c7dbe7;"
        " border-radius: 10px;"
        "}"
    );
    QVBoxLayout *pilotL = new QVBoxLayout(pilotFrame);
    pilotL->setContentsMargins(10, 8, 10, 8);
    pilotL->setSpacing(8);

    QLabel *pilotTitle = new QLabel("Pilotage decisionnel (ponderez les criteres)");
    pilotTitle->setStyleSheet("font-size: 16px; font-weight: 800; color: #17384a;");
    pilotL->addWidget(pilotTitle);

    auto makePoidsWidget = [](const QString &libelle, int valeur, QSpinBox *&spinOut) {
        QWidget *w = new QWidget();
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(4);

        QLabel *lbl = new QLabel(libelle);
        lbl->setStyleSheet("font-size: 12px; font-weight: 700; color: #2a4f61;");

        QSpinBox *sp = new QSpinBox();
        sp->setRange(0, 100);
        sp->setValue(valeur);
        sp->setSuffix(" %");
        sp->setButtonSymbols(QAbstractSpinBox::NoButtons);
        sp->setFixedHeight(34);
        sp->setStyleSheet(
            "QSpinBox {"
            " background: white;"
            " border: 1px solid #97b9cb;"
            " border-radius: 7px;"
            " padding-left: 6px;"
            " font-size: 13px;"
            "}"
        );

        vl->addWidget(lbl);
        vl->addWidget(sp);
        spinOut = sp;
        return w;
    };

    QSpinBox *spPoidsPrix = nullptr;
    QSpinBox *spPoidsQualite = nullptr;
    QSpinBox *spPoidsDelai = nullptr;
    QSpinBox *spPoidsStock = nullptr;

    QHBoxLayout *poidsL = new QHBoxLayout();
    poidsL->setSpacing(8);
    poidsL->addWidget(makePoidsWidget("Prix", 35, spPoidsPrix));
    poidsL->addWidget(makePoidsWidget("Qualite", 30, spPoidsQualite));
    poidsL->addWidget(makePoidsWidget("Delai", 20, spPoidsDelai));
    poidsL->addWidget(makePoidsWidget("Stock", 15, spPoidsStock));

    QPushButton *btnResetPoids = new QPushButton("Reset poids");
    btnResetPoids->setCursor(Qt::PointingHandCursor);
    btnResetPoids->setFixedHeight(34);
    btnResetPoids->setStyleSheet(
        "QPushButton {"
        " background: #355c7d;"
        " color: white;"
        " border: none;"
        " border-radius: 8px;"
        " font-size: 12px;"
        " font-weight: 800;"
        " padding: 0 12px;"
        "}"
        "QPushButton:hover { background: #416b8f; }"
    );
    poidsL->addWidget(btnResetPoids, 0, Qt::AlignBottom);
    pilotL->addLayout(poidsL);

    QLabel *lblPoidsInfo = new QLabel("Total poids = 100% recommande pour un score fournisseur coherent.");
    lblPoidsInfo->setStyleSheet("font-size: 12px; color: #35566a;");
    pilotL->addWidget(lblPoidsInfo);

    auto makeKpiCard = [](const QString &title, const QString &bg, QLabel *&valueLbl) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(
            "QFrame { background: %1; border: 1px solid #c9dce7; border-radius: 8px; }"
        ).arg(bg));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(8, 6, 8, 6);
        cl->setSpacing(2);

        QLabel *t = new QLabel(title);
        t->setStyleSheet("font-size: 11px; font-weight: 700; color: #2f4d5e;");
        QLabel *v = new QLabel("--");
        v->setStyleSheet("font-size: 18px; font-weight: 900; color: #122d3a;");
        v->setAlignment(Qt::AlignCenter);
        cl->addWidget(t);
        cl->addWidget(v);
        valueLbl = v;
        return card;
    };

    QLabel *lblKpiFournisseur = nullptr;
    QLabel *lblKpiBudget = nullptr;
    QLabel *lblKpiCouverture = nullptr;
    QLabel *lblKpiRisqueTxt = nullptr;

    QHBoxLayout *kpiL = new QHBoxLayout();
    kpiL->setSpacing(8);
    kpiL->addWidget(makeKpiCard("Top fournisseur", "#e9f7ff", lblKpiFournisseur));
    kpiL->addWidget(makeKpiCard("Budget estime", "#eef9ee", lblKpiBudget));
    kpiL->addWidget(makeKpiCard("Couverture stock", "#fff8e8", lblKpiCouverture));
    kpiL->addWidget(makeKpiCard("Risque rupture", "#fff1f1", lblKpiRisqueTxt));
    pilotL->addLayout(kpiL);

    QProgressBar *pbRisque = new QProgressBar(onglet);
    pbRisque->setRange(0, 100);
    pbRisque->setValue(0);
    pbRisque->setFormat("Risque rupture: %p%");
    pbRisque->setStyleSheet(
        "QProgressBar {"
        " border: 1px solid #aec7d5;"
        " border-radius: 7px;"
        " background: #f6fbff;"
        " height: 18px;"
        " text-align: center;"
        " font-size: 12px;"
        " font-weight: 700;"
        "}"
        "QProgressBar::chunk {"
        " border-radius: 6px;"
        " background-color: #d9534f;"
        "}"
    );
    pilotL->addWidget(pbRisque);

    QLabel *lblAlerteAuto = new QLabel("Alerte automatique: en attente d'analyse.");
    lblAlerteAuto->setWordWrap(true);
    lblAlerteAuto->setStyleSheet(
        "font-size: 12px;"
        "font-weight: 700;"
        "color: #7a4c12;"
        "background: #fff8e9;"
        "border: 1px solid #f1d28f;"
        "border-radius: 7px;"
        "padding: 6px 8px;"
    );
    pilotL->addWidget(lblAlerteAuto);

    auto setAlerteAuto = [=](const QString &niveau, const QString &message) {
        lblAlerteAuto->setText("Alerte automatique [" + niveau + "]: " + message);
        if (niveau == "ELEVE") {
            lblAlerteAuto->setStyleSheet(
                "font-size: 12px; font-weight: 800; color: #8f1d1d;"
                "background: #ffecec; border: 1px solid #f3adad; border-radius: 7px; padding: 6px 8px;"
            );
        } else if (niveau == "MODERE") {
            lblAlerteAuto->setStyleSheet(
                "font-size: 12px; font-weight: 800; color: #7a4c12;"
                "background: #fff8e9; border: 1px solid #f1d28f; border-radius: 7px; padding: 6px 8px;"
            );
        } else {
            lblAlerteAuto->setStyleSheet(
                "font-size: 12px; font-weight: 800; color: #1d6f38;"
                "background: #ecfbf2; border: 1px solid #a9dfbe; border-radius: 7px; padding: 6px 8px;"
            );
        }
    };

    l->addWidget(pilotFrame);

    QLabel *lblResultats = new QLabel("Offres fournisseurs disponibles :");
    lblResultats->setStyleSheet("font-size: 20px; font-weight: 700; color: #212121; margin-top: 6px;");
    l->addWidget(lblResultats);

    QTableWidget *table = new QTableWidget(3, 6);
    table->setHorizontalHeaderLabels(
        QStringList() << "Fournisseur" << "Prix/M²" << "Qualité" << "Délai" << "Stock Dispo" << "Score" );
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setFocusPolicy(Qt::StrongFocus);
    table->setAlternatingRowColors(false);
    table->verticalHeader()->setVisible(true);
    table->verticalHeader()->setDefaultSectionSize(34);
    table->setVerticalHeaderLabels(QStringList() << "1" << "2" << "3");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setMinimumHeight(36);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setMinimumHeight(160);
    table->setMaximumHeight(190);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    table->setStyleSheet(
        "QTableWidget {"
        " background: white;"
        " border: 1px solid #d7ccc8;"
        " gridline-color: #e5ddd7;"
        "}"
        "QTableWidget::item:selected {"
        " background-color: #d7ecf7;"
        " color: #102a35;"
        "}"
        "QHeaderView::section {"
        " background-color: #058e98;"
        " color: white;"
        " font-weight: 700;"
        " font-size: 14px;"
        " border: none;"
        " padding: 6px;"
        "}"
    );

    QLabel *recommandation = new QLabel("Fournisseur conseillé : TanLeather SA (meilleur rapport Qualité/Prix/Fiabilité).");
    recommandation->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: #1f6d2a;"
        "background-color: #ccefd1;"
        "border-radius: 8px;"
        "padding: 10px 12px;"
    );

    QLabel *selectionInfo = new QLabel("Selection manuelle: cliquez sur une ligne fournisseur pour l'utiliser dans le plan.");
    selectionInfo->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 700;"
        "color: #0f3c58;"
        "background-color: #e6f4ff;"
        "border: 1px solid #b9d6eb;"
        "border-radius: 7px;"
        "padding: 7px 10px;"
    );

    struct CompareRow {
        QString fournisseur;
        QString prix;
        QString qualite;
        QString delai;
        QString stock;
        int note;
        QColor baseBg;
        QColor qualiteBg;
        QColor stockBg;
    };

    auto setCell = [table](int row, int col, const QString &txt, const QColor &bg, const QColor &fg = QColor("#263238")) {
        auto *it = new QTableWidgetItem(txt);
        it->setBackground(QBrush(bg));
        it->setForeground(QBrush(fg));
        it->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setItem(row, col, it);
    };

    auto starsText = [](int note) {
        QString out;
        for (int i = 0; i < note; ++i) out += QStringLiteral("★");
        return out;
    };

    auto parseNumber = [](QString text) {
        text = text.trimmed();
        text.replace(',', '.');
        text.remove(QRegularExpression("[^0-9.]"));
        return text.toDouble();
    };

    auto qualityScore = [](const QString &qualite) {
        const QString q = qualite.trimmed().toUpper();
        if (q.startsWith("A")) return 100.0;
        if (q.startsWith("B")) return 78.0;
        if (q.startsWith("C")) return 55.0;
        return 40.0;
    };

    auto buildData = [](const QString &matiere) {
        QVector<CompareRow> rows;
        QString reco;

        if (matiere == "Cuir Agneau") {
            rows = {
                {"SoftHide Pro", "52 DT", "A (Premium)", "4 jours", "420 M²", 5, QColor("#ddf5df"), QColor("#bff0bf"), QColor("#b8ebb8")},
                {"Agneau Plus", "44 DT", "B+ (Standard)", "6 jours", "280 M²", 4, QColor("#fff6dd"), QColor("#ffe9bc"), QColor("#fff2d4")},
                {"Cuir Sud", "36 DT", "C (Économique)", "9 jours", "900 M²", 3, QColor("#fde6e6"), QColor("#ffd5d5"), QColor("#ffdada")}
            };
            reco = "Fournisseur conseillé : SoftHide Pro (meilleur rapport Qualité/Prix/Fiabilité).";
        } else if (matiere == "Cuir Veau") {
            rows = {
                {"Veau Prestige", "49 DT", "A (Premium)", "5 jours", "380 M²", 5, QColor("#ddf5df"), QColor("#bff0bf"), QColor("#b8ebb8")},
                {"Elite Veau", "41 DT", "B (Standard)", "7 jours", "260 M²", 4, QColor("#fff6dd"), QColor("#ffe9bc"), QColor("#fff2d4")},
                {"Market Cuir", "34 DT", "C (Économique)", "11 jours", "1100 M²", 3, QColor("#fde6e6"), QColor("#ffd5d5"), QColor("#ffdada")}
            };
            reco = "Fournisseur conseillé : Veau Prestige (meilleur rapport Qualité/Prix/Fiabilité).";
        } else {
            rows = {
                {"TanLeather SA", "45 DT", "A (Premium)", "5 jours", "500 M²", 5, QColor("#ddf5df"), QColor("#bff0bf"), QColor("#b8ebb8")},
                {"Cuir Elite", "38 DT", "B (Standard)", "7 jours", "300 M²", 4, QColor("#fff6dd"), QColor("#ffe9bc"), QColor("#fff2d4")},
                {"MegaCuir", "32 DT", "C (Économique)", "10 jours", "1000 M²", 3, QColor("#fde6e6"), QColor("#ffd5d5"), QColor("#ffdada")}
            };
            reco = "Fournisseur conseillé : TanLeather SA (meilleur rapport Qualité/Prix/Fiabilité).";
        }

        return qMakePair(rows, reco);
    };

    auto majPhotoCuir = [=](const QString &matiere) {
        QString chemin;
        QString legende;

        if (matiere == "Cuir Agneau") {
            chemin = ":/pic agneau.jpg";
            legende = "Cuir d'agneau: souple, premium, ideal pour les finitions haut de gamme.";
        } else if (matiere == "Cuir Veau") {
            chemin = ":/pi veau.jpg";
            legende = "Cuir de veau: grain fin, excellent equilibre entre confort et resistance.";
        } else {
            chemin = ":/pic vachette.jpg";
            legende = "Cuir vachette: robustesse et durabilite, parfait pour une production intensive.";
        }

        QPixmap pix(chemin);
        if (pix.isNull()) {
            photoCuir->setText("Image indisponible");
            photoSousTitre->setText("Verifiez la ressource image configuree dans le projet.");
            return;
        }

        photoCuir->setPixmap(pix.scaled(photoCuir->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        photoSousTitre->setText(legende);
    };

    auto *selectedSupplierRow = new int(0);

    auto remplirTable = [=]() {
        const auto data = buildData(cbMatiere->currentText());
        const QVector<CompareRow> &rows = data.first;

        const int poidsPrix = spPoidsPrix->value();
        const int poidsQualite = spPoidsQualite->value();
        const int poidsDelai = spPoidsDelai->value();
        const int poidsStock = spPoidsStock->value();
        const int totalPoids = poidsPrix + poidsQualite + poidsDelai + poidsStock;

        lblPoidsInfo->setText(totalPoids == 100
                              ? "Total poids = 100% (parfait)."
                              : QString("Total poids = %1%%. Conseille: 100%% pour une lecture metier claire.").arg(totalPoids));
        lblPoidsInfo->setStyleSheet(totalPoids == 100
                                    ? "font-size: 12px; color: #256a34; font-weight: 700;"
                                    : "font-size: 12px; color: #a2551d; font-weight: 700;");

        double minPrix = 0.0, maxPrix = 0.0, minDelai = 0.0, maxDelai = 0.0, minStock = 0.0, maxStock = 0.0;
        if (!rows.isEmpty()) {
            minPrix = maxPrix = parseNumber(rows[0].prix);
            minDelai = maxDelai = parseNumber(rows[0].delai);
            minStock = maxStock = parseNumber(rows[0].stock);

            for (const CompareRow &r : rows) {
                const double prix = parseNumber(r.prix);
                const double delai = parseNumber(r.delai);
                const double stock = parseNumber(r.stock);
                minPrix = qMin(minPrix, prix); maxPrix = qMax(maxPrix, prix);
                minDelai = qMin(minDelai, delai); maxDelai = qMax(maxDelai, delai);
                minStock = qMin(minStock, stock); maxStock = qMax(maxStock, stock);
            }
        }

        table->setRowCount(rows.size());
        int bestRow = -1;
        double bestScore = -1.0;
        QString bestSupplier;

        QVector<double> finalScores(rows.size(), 0.0);
        for (int r = 0; r < rows.size(); ++r) {
            const CompareRow &row = rows[r];
            const double prix = parseNumber(row.prix);
            const double delai = parseNumber(row.delai);
            const double stock = parseNumber(row.stock);

            const double scorePrix = (maxPrix > minPrix) ? (100.0 * (maxPrix - prix) / (maxPrix - minPrix)) : 100.0;
            const double scoreDelai = (maxDelai > minDelai) ? (100.0 * (maxDelai - delai) / (maxDelai - minDelai)) : 100.0;
            const double scoreStock = (maxStock > minStock) ? (100.0 * (stock - minStock) / (maxStock - minStock)) : 100.0;
            const double scoreQual = qualityScore(row.qualite);
            const double denom = (totalPoids > 0) ? static_cast<double>(totalPoids) : 1.0;
            const double scoreFinal = ((scorePrix * poidsPrix) + (scoreQual * poidsQualite) + (scoreDelai * poidsDelai) + (scoreStock * poidsStock)) / denom;
            finalScores[r] = scoreFinal;

            if (scoreFinal > bestScore) {
                bestScore = scoreFinal;
                bestSupplier = row.fournisseur;
            }
        }

        QVector<int> sortedRows;
        sortedRows.reserve(rows.size());
        for (int i = 0; i < rows.size(); ++i) sortedRows.push_back(i);
        std::sort(sortedRows.begin(), sortedRows.end(), [&](int a, int b) {
            return finalScores[a] > finalScores[b];
        });

        if (!sortedRows.isEmpty()) {
            bestRow = 0; // apres tri, la 1ere ligne affichée est la meilleure
        }

        for (int rank = 0; rank < sortedRows.size(); ++rank) {
            const int sourceRow = sortedRows[rank];
            const CompareRow &row = rows[sourceRow];
            const double scoreFinal = finalScores[sourceRow];
            const int stars = qMax(1, 5 - rank); // meilleur score = plus d'etoiles

            setCell(rank, 0, row.fournisseur, row.baseBg);
            setCell(rank, 1, row.prix, row.baseBg);
            setCell(rank, 2, row.qualite, row.qualiteBg);
            setCell(rank, 3, row.delai, row.baseBg);
            setCell(rank, 4, row.stock, row.stockBg);
            const QString scoreTxt = QString::number(scoreFinal, 'f', 1) + "/100  " + starsText(stars);
            QColor scoreBg = row.baseBg;
            if (scoreFinal >= 80.0) scoreBg = QColor("#d7f4d7");
            else if (scoreFinal < 60.0) scoreBg = QColor("#ffe3e3");
            setCell(rank, 5, scoreTxt, scoreBg, QColor("#111111"));
            if (QTableWidgetItem *noteItem = table->item(rank, 5)) {
                noteItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            }
        }

        if (bestRow >= 0) {
            *selectedSupplierRow = bestRow;
            for (int c = 0; c < table->columnCount(); ++c) {
                if (QTableWidgetItem *it = table->item(bestRow, c)) {
                    QFont f = it->font();
                    f.setBold(true);
                    it->setFont(f);
                }
            }
            if (QTableWidgetItem *it0 = table->item(bestRow, 0)) {
                it0->setText(QStringLiteral("TOP - ") + it0->text());
            }
            recommandation->setText(QString("Fournisseur recommande (scoring pondere) : %1 | Score global: %2/100")
                                    .arg(bestSupplier)
                                    .arg(QString::number(bestScore, 'f', 1)));
            lblKpiFournisseur->setText(bestSupplier);
            selectionInfo->setText(QString("Fournisseur actif: %1 (auto, meilleur score). Cliquez une autre ligne pour choisir manuellement.")
                                   .arg(bestSupplier));
            lblKpiRisqueTxt->setText(bestScore >= 80.0 ? "Faible" : (bestScore >= 65.0 ? "Modere" : "Eleve"));
            pbRisque->setValue(bestScore >= 80.0 ? 20 : (bestScore >= 65.0 ? 45 : 75));
            setAlerteAuto(bestScore >= 80.0 ? "FAIBLE" : (bestScore >= 65.0 ? "MODERE" : "ELEVE"),
                          bestScore >= 80.0
                          ? "Fournisseur stable. Conditions de ravitaillement favorables."
                          : (bestScore >= 65.0
                             ? "Performance acceptable. Surveiller delai et disponibilite."
                             : "Fournisseur fragile. Prevoir option de secours."));
        }
    };

    QObject::connect(table, &QTableWidget::cellClicked, this, [=](int row, int col) {
        Q_UNUSED(col);
        *selectedSupplierRow = row;

        const QString fournisseur = table->item(row, 0) ? table->item(row, 0)->text() : QStringLiteral("Inconnu");
        const QString scoreTxt = table->item(row, 5) ? table->item(row, 5)->text().section('/', 0, 0).trimmed() : QStringLiteral("0");
        recommandation->setText(QString("Fournisseur choisi manuellement : %1 | Score global: %2/100")
                                .arg(fournisseur, scoreTxt));
        lblKpiFournisseur->setText(fournisseur);
        selectionInfo->setText(QString("Fournisseur actif (manuel): %1. Le plan de ravitaillement utilisera ce fournisseur.")
                               .arg(fournisseur));
        const double score = scoreTxt.toDouble();
        setAlerteAuto(score >= 80.0 ? "FAIBLE" : (score >= 65.0 ? "MODERE" : "ELEVE"),
                  QString("Selection manuelle appliquee (%1/100). ").arg(scoreTxt)
                  + (score >= 80.0 ? "Profil robuste." : (score >= 65.0 ? "Controler les risques." : "Risque eleve, backup recommande.")));
    });

    table->setMouseTracking(true);
    table->viewport()->setMouseTracking(true);
    auto *hoveredNoteRow = new int(-1);
    auto *magicStarTimer = new QTimer(table);
    magicStarTimer->setInterval(110);
    auto *magicRow = new int(-1);
    auto *magicBaseText = new QString();
    auto *magicPhase = new int(0);

    auto stopMagicHover = [=]() {
        magicStarTimer->stop();
        if (*magicRow >= 0) {
            if (QTableWidgetItem *scoreItem = table->item(*magicRow, 5)) {
                if (!magicBaseText->isEmpty()) {
                    scoreItem->setText(*magicBaseText);
                }
                scoreItem->setForeground(QBrush(QColor("#111111")));
            }
        }
        *magicRow = -1;
        magicBaseText->clear();
        *magicPhase = 0;
    };

    QObject::connect(magicStarTimer, &QTimer::timeout, this, [=]() {
        if (*magicRow < 0 || *magicRow >= table->rowCount()) {
            stopMagicHover();
            return;
        }

        QTableWidgetItem *scoreItem = table->item(*magicRow, 5);
        if (!scoreItem) {
            stopMagicHover();
            return;
        }

        static const QVector<QColor> glowPalette = {
            QColor("#d4af37"), QColor("#f6d365"), QColor("#ffd700"), QColor("#ffe08a")
        };
        static const QStringList sparkle = {"  *", "  +", "  *+", "  +*"};

        const int p = (*magicPhase) % glowPalette.size();
        scoreItem->setForeground(QBrush(glowPalette[p]));
        if (!magicBaseText->isEmpty()) {
            scoreItem->setText(*magicBaseText + sparkle[(*magicPhase) % sparkle.size()]);
        }
        *magicPhase = *magicPhase + 1;
    });

    QObject::connect(table, &QTableWidget::itemEntered, this, [=](QTableWidgetItem *item) {
        if (*hoveredNoteRow >= 0 && *hoveredNoteRow != *magicRow) {
            if (QTableWidgetItem *prev = table->item(*hoveredNoteRow, 5)) {
                prev->setForeground(QBrush(QColor("#111111")));
            }
        }

        if (!item || item->column() != 5) {
            *hoveredNoteRow = -1;
            stopMagicHover();
            return;
        }

        *hoveredNoteRow = item->row();
        const QString txt = item->text();
        const bool isFiveStars = txt.contains(QStringLiteral("★★★★★"));

        if (isFiveStars) {
            if (*magicRow != item->row() || magicBaseText->isEmpty()) {
                stopMagicHover();
                *magicRow = item->row();
                *magicBaseText = txt.section("  *", 0, 0).section("  +", 0, 0).trimmed();
            }
            if (!magicStarTimer->isActive()) {
                magicStarTimer->start();
            }
        } else {
            stopMagicHover();
            item->setForeground(QBrush(QColor("#d4af37")));
        }
    });

    QObject::connect(table, &QTableWidget::viewportEntered, this, [=]() {
        stopMagicHover();
        if (*hoveredNoteRow >= 0 && table->item(*hoveredNoteRow, 5)) {
            table->item(*hoveredNoteRow, 5)->setForeground(QBrush(QColor("#111111")));
        }
        *hoveredNoteRow = -1;
    });

    QObject::connect(btnComparer, &QPushButton::clicked, this, [=]() { remplirTable(); });
    QObject::connect(cbMatiere, &QComboBox::currentTextChanged, this, [=](const QString &matiere) {
        majPhotoCuir(matiere);
        remplirTable();
    });
    QObject::connect(spPoidsPrix, qOverload<int>(&QSpinBox::valueChanged), this, [=](int){ remplirTable(); });
    QObject::connect(spPoidsQualite, qOverload<int>(&QSpinBox::valueChanged), this, [=](int){ remplirTable(); });
    QObject::connect(spPoidsDelai, qOverload<int>(&QSpinBox::valueChanged), this, [=](int){ remplirTable(); });
    QObject::connect(spPoidsStock, qOverload<int>(&QSpinBox::valueChanged), this, [=](int){ remplirTable(); });

    QObject::connect(btnResetPoids, &QPushButton::clicked, this, [=]() {
        spPoidsPrix->setValue(35);
        spPoidsQualite->setValue(30);
        spPoidsDelai->setValue(20);
        spPoidsStock->setValue(15);
        remplirTable();
    });

    majPhotoCuir(cbMatiere->currentText());
    remplirTable();

    l->addWidget(table);
    l->addWidget(recommandation);
    l->addWidget(selectionInfo);

    QFrame *planFrame = new QFrame(onglet);
    planFrame->setStyleSheet(
        "QFrame {"
        " background: #f7fbfd;"
        " border: 1px solid #c9e2ee;"
        " border-radius: 10px;"
        "}"
    );
    QVBoxLayout *planLayout = new QVBoxLayout(planFrame);
    planLayout->setContentsMargins(12, 10, 12, 10);
    planLayout->setSpacing(8);

    QLabel *planTitle = new QLabel("Plan de ravitaillement");
    planTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f2f3a;");
    planLayout->addWidget(planTitle);

    QHBoxLayout *planInputs = new QHBoxLayout();
    planInputs->setSpacing(10);

    QDoubleSpinBox *sbStockActuel = new QDoubleSpinBox(onglet);
    sbStockActuel->setDecimals(2);
    sbStockActuel->setRange(0.0, 1000000.0);
    sbStockActuel->setValue(120.0);
    sbStockActuel->setSuffix(" M2");
    sbStockActuel->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbStockActuel->setFixedHeight(40);

    QDoubleSpinBox *sbSeuilSecurite = new QDoubleSpinBox(onglet);
    sbSeuilSecurite->setDecimals(2);
    sbSeuilSecurite->setRange(0.0, 1000000.0);
    sbSeuilSecurite->setValue(300.0);
    sbSeuilSecurite->setSuffix(" M2");
    sbSeuilSecurite->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbSeuilSecurite->setFixedHeight(40);

    QDoubleSpinBox *sbConsoPrevue = new QDoubleSpinBox(onglet);
    sbConsoPrevue->setDecimals(2);
    sbConsoPrevue->setRange(0.0, 1000000.0);
    sbConsoPrevue->setValue(180.0);
    sbConsoPrevue->setSuffix(" M2/7j");
    sbConsoPrevue->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbConsoPrevue->setFixedHeight(40);

    auto styleSpin = QStringLiteral(
        "QDoubleSpinBox {"
        " background: white;"
        " border: 1px solid #8fb8cc;"
        " border-radius: 8px;"
        " min-height: 38px;"
        " padding-left: 8px;"
        " font-size: 14px;"
        "}"
    );

    sbStockActuel->setStyleSheet(styleSpin);
    sbSeuilSecurite->setStyleSheet(styleSpin);
    sbConsoPrevue->setStyleSheet(styleSpin);

    QLabel *lblStockActuel = new QLabel("Stock actuel");
    QLabel *lblSeuil = new QLabel("Seuil sécurité");
    QLabel *lblConso = new QLabel("Conso prévue (7j)");
    lblStockActuel->setStyleSheet("font-size: 13px; font-weight: 700; color: #25414f;");
    lblSeuil->setStyleSheet("font-size: 13px; font-weight: 700; color: #25414f;");
    lblConso->setStyleSheet("font-size: 13px; font-weight: 700; color: #25414f;");

    QVBoxLayout *col1 = new QVBoxLayout();
    col1->addWidget(lblStockActuel);
    col1->addWidget(sbStockActuel);
    QVBoxLayout *col2 = new QVBoxLayout();
    col2->addWidget(lblSeuil);
    col2->addWidget(sbSeuilSecurite);
    QVBoxLayout *col3 = new QVBoxLayout();
    col3->addWidget(lblConso);
    col3->addWidget(sbConsoPrevue);

    planInputs->addLayout(col1);
    planInputs->addLayout(col2);
    planInputs->addLayout(col3);
    planInputs->setStretch(0, 1);
    planInputs->setStretch(1, 1);
    planInputs->setStretch(2, 1);
    planLayout->addLayout(planInputs);

    QPushButton *btnPlanifier = new QPushButton("Générer plan de ravitaillement");
    btnPlanifier->setCursor(Qt::PointingHandCursor);
    btnPlanifier->setStyleSheet(
        "QPushButton {"
        " background-color: #0f7f51;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " min-height: 36px;"
        " font-size: 14px;"
        " font-weight: 700;"
        " padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #149362; }"
        "QPushButton:pressed { background-color: #0d6e47; }"
    );

    QLabel *lblPlan = new QLabel("Cliquez sur 'Générer plan de ravitaillement' pour obtenir une quantité et un budget recommandés.");
    lblPlan->setWordWrap(true);
    lblPlan->setStyleSheet("font-size: 14px; color: #244150; background: #edf7fc; border: 1px solid #c8deea; border-radius: 8px; padding: 8px;");

    QLabel *lblActionPlan = new QLabel("Action manageriale: en attente de simulation.");
    lblActionPlan->setWordWrap(true);
    lblActionPlan->setStyleSheet("font-size: 13px; color: #153f2f; background: #ecf9ef; border: 1px solid #b7e2c1; border-radius: 8px; padding: 7px;");

    QObject::connect(btnPlanifier, &QPushButton::clicked, this, [=]() {
        const int row = (*selectedSupplierRow >= 0 && *selectedSupplierRow < table->rowCount()) ? *selectedSupplierRow : 0;
        if (table->rowCount() <= 0 || !table->item(row, 0) || !table->item(row, 1) || !table->item(row, 4)) {
            lblPlan->setText("Aucun fournisseur disponible pour générer le plan.");
            lblActionPlan->setText("Action manageriale: verifier les donnees fournisseurs avant relance du module.");
            setAlerteAuto("ELEVE", "Plan indisponible: donnees fournisseurs manquantes.");
            return;
        }

        const QString fournisseur = table->item(row, 0)->text();
        const double prixUnitaire = parseNumber(table->item(row, 1)->text());
        const double stockDispo = parseNumber(table->item(row, 4)->text());

        const double stockActuel = sbStockActuel->value();
        const double seuil = sbSeuilSecurite->value();
        const double conso = sbConsoPrevue->value();
        const double besoinBrut = qMax(0.0, seuil + conso - stockActuel);
        const double consoJour = qMax(0.01, conso / 7.0);
        const double couvertureJours = stockActuel / consoJour;

        lblKpiCouverture->setText(QString::number(couvertureJours, 'f', 1) + " j");

        if (besoinBrut <= 0.0) {
            lblPlan->setText("Stock suffisant: aucun ravitaillement urgent n'est nécessaire pour les 7 prochains jours.");
            lblActionPlan->setText("Action manageriale: maintenir le suivi hebdomadaire, aucune commande urgente.");
            lblKpiBudget->setText("0 DT");
            lblKpiRisqueTxt->setText("Faible");
            pbRisque->setValue(15);
            setAlerteAuto("FAIBLE", "Pas de commande urgente. Couverture stock confortable.");
            return;
        }

        const double qteCommande = qMin(besoinBrut, stockDispo);
        const double budget = qteCommande * prixUnitaire;
        const double reliquat = qMax(0.0, besoinBrut - stockDispo);

        QString message = QString(
                              "Fournisseur retenu: <b>%1</b><br>"
                              "Besoin calculé: <b>%2 M2</b><br>"
                              "Quantité à commander maintenant: <b>%3 M2</b><br>"
                              "Budget estimé: <b>%4 DT</b>")
                              .arg(fournisseur)
                              .arg(QString::number(besoinBrut, 'f', 2))
                              .arg(QString::number(qteCommande, 'f', 2))
                              .arg(QString::number(budget, 'f', 2));

        if (reliquat > 0.0) {
            message += QString("<br><span style='color:#b23a2f;'>Alerte: reliquat non couvert %1 M2 (prévoir une 2ème commande).</span>")
                           .arg(QString::number(reliquat, 'f', 2));
        }

        lblPlan->setText(message);
        lblKpiBudget->setText(QString::number(budget, 'f', 0) + " DT");

        if (reliquat > 0.0 || couvertureJours < 5.0) {
            lblActionPlan->setText(
                "Action manageriale: declencher immediatement une commande complementaire et notifier le planning fabrication pour eviter la rupture."
            );
            lblKpiRisqueTxt->setText("Eleve");
            pbRisque->setValue(82);
            setAlerteAuto("ELEVE", "Risque de rupture detecte. Declencher commande et plan backup.");
        } else if (couvertureJours < 8.0) {
            lblActionPlan->setText(
                "Action manageriale: passer commande aujourd'hui et suivre quotidiennement la couverture jusqu'a reception."
            );
            lblKpiRisqueTxt->setText("Modere");
            pbRisque->setValue(52);
            setAlerteAuto("MODERE", "Couverture limitee. Suivi journalier recommande.");
        } else {
            lblActionPlan->setText(
                "Action manageriale: plan valide. Maintenir le fournisseur retenu et recalculer le besoin en fin de semaine."
            );
            lblKpiRisqueTxt->setText("Faible");
            pbRisque->setValue(24);
            setAlerteAuto("FAIBLE", "Plan valide avec bonne couverture.");
        }
    });

    planLayout->addWidget(btnPlanifier, 0, Qt::AlignLeft);
    planLayout->addWidget(lblPlan);
    planLayout->addWidget(lblActionPlan);
    l->addWidget(planFrame);

    QFrame *strategyFrame = new QFrame(onglet);
    strategyFrame->setStyleSheet(
        "QFrame {"
        " background: #f7f6ff;"
        " border: 1px solid #d5d1f5;"
        " border-radius: 10px;"
        "}"
    );
    QVBoxLayout *strategyL = new QVBoxLayout(strategyFrame);
    strategyL->setContentsMargins(12, 10, 12, 10);
    strategyL->setSpacing(8);

    QLabel *strategyTitle = new QLabel("Optimisation Ingenieur (EOQ + Stock Securite + Point Commande)");
    strategyTitle->setStyleSheet("font-size: 17px; font-weight: 900; color: #2d2b58;");
    strategyL->addWidget(strategyTitle);

    auto makeInput = [](const QString &label, double value, double min, double max, const QString &suffix) {
        QWidget *w = new QWidget();
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(3);

        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 12px; font-weight: 700; color: #37355f;");

        QDoubleSpinBox *sb = new QDoubleSpinBox();
        sb->setRange(min, max);
        sb->setDecimals(2);
        sb->setValue(value);
        sb->setSuffix(suffix);
        sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
        sb->setFixedHeight(34);
        sb->setStyleSheet(
            "QDoubleSpinBox {"
            " background: white;"
            " border: 1px solid #aba6db;"
            " border-radius: 7px;"
            " padding-left: 6px;"
            " font-size: 13px;"
            "}"
        );
        vl->addWidget(lbl);
        vl->addWidget(sb);
        return qMakePair(w, sb);
    };

    auto inDemande = makeInput("Demande mensuelle", 1200.0, 1.0, 1000000.0, " M2");
    auto inLead = makeInput("Delai moyen fournisseur", 7.0, 1.0, 120.0, " jours");
    auto inVar = makeInput("Variabilite delai", 20.0, 0.0, 100.0, " %");
    auto inService = makeInput("Niveau service", 95.0, 80.0, 99.9, " %");
    auto inPassation = makeInput("Cout passation commande", 180.0, 1.0, 100000.0, " DT");
    auto inHolding = makeInput("Taux stockage annuel", 22.0, 1.0, 100.0, " %");

    QHBoxLayout *line1 = new QHBoxLayout();
    line1->setSpacing(8);
    line1->addWidget(inDemande.first);
    line1->addWidget(inLead.first);
    line1->addWidget(inVar.first);

    QHBoxLayout *line2 = new QHBoxLayout();
    line2->setSpacing(8);
    line2->addWidget(inService.first);
    line2->addWidget(inPassation.first);
    line2->addWidget(inHolding.first);

    strategyL->addLayout(line1);
    strategyL->addLayout(line2);

    QPushButton *btnStrategie = new QPushButton("Generer strategie optimale");
    btnStrategie->setCursor(Qt::PointingHandCursor);
    btnStrategie->setFixedHeight(36);
    btnStrategie->setStyleSheet(
        "QPushButton {"
        " background-color: #4b3fb2;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " font-size: 14px;"
        " font-weight: 800;"
        " padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #5d50c4; }"
        "QPushButton:pressed { background-color: #3d3392; }"
    );
    strategyL->addWidget(btnStrategie, 0, Qt::AlignLeft);

    QLabel *lblStrategie = new QLabel("Simulation ingenieur en attente.");
    lblStrategie->setWordWrap(true);
    lblStrategie->setStyleSheet("font-size: 13px; color: #2d2b58; background: #efedff; border: 1px solid #d0caf6; border-radius: 8px; padding: 8px;");
    strategyL->addWidget(lblStrategie);

    l->addWidget(strategyFrame);

    QFrame *histFrame = new QFrame(onglet);
    histFrame->setStyleSheet(
        "QFrame {"
        " background: #f8fafb;"
        " border: 1px solid #d2dde3;"
        " border-radius: 10px;"
        "}"
    );
    QVBoxLayout *histL = new QVBoxLayout(histFrame);
    histL->setContentsMargins(12, 10, 12, 10);
    histL->setSpacing(8);

    QLabel *histTitle = new QLabel("Historique des decisions ravitaillement");
    histTitle->setStyleSheet("font-size: 16px; font-weight: 900; color: #1f3644;");
    histL->addWidget(histTitle);

    QHBoxLayout *histFilters = new QHBoxLayout();
    histFilters->setSpacing(8);

    QLabel *lblFrom = new QLabel("Du:");
    QLabel *lblTo = new QLabel("Au:");
    QLabel *lblMat = new QLabel("Matiere:");
    lblFrom->setStyleSheet("font-weight: 700; color: #2c4b5d;");
    lblTo->setStyleSheet("font-weight: 700; color: #2c4b5d;");
    lblMat->setStyleSheet("font-weight: 700; color: #2c4b5d;");

    QDateEdit *deFrom = new QDateEdit(QDate::currentDate().addDays(-30), onglet);
    QDateEdit *deTo = new QDateEdit(QDate::currentDate(), onglet);
    deFrom->setCalendarPopup(true);
    deTo->setCalendarPopup(true);
    deFrom->setDisplayFormat("dd/MM/yyyy");
    deTo->setDisplayFormat("dd/MM/yyyy");
    deFrom->setFixedHeight(32);
    deTo->setFixedHeight(32);

    QLineEdit *leMatFilter = new QLineEdit(onglet);
    leMatFilter->setPlaceholderText("ex: Cuir Vachette");
    leMatFilter->setFixedHeight(32);
    leMatFilter->setStyleSheet("QLineEdit { background: white; border: 1px solid #9ab8c7; border-radius: 7px; padding: 4px 8px; }");

    auto styleDate = QStringLiteral("QDateEdit { background: white; border: 1px solid #9ab8c7; border-radius: 7px; padding: 4px 8px; }");
    deFrom->setStyleSheet(styleDate);
    deTo->setStyleSheet(styleDate);

    QPushButton *btnHistFiltrer = new QPushButton("Filtrer");
    btnHistFiltrer->setCursor(Qt::PointingHandCursor);
    btnHistFiltrer->setFixedHeight(32);
    btnHistFiltrer->setStyleSheet(
        "QPushButton { background: #0f7f51; color: white; border: none; border-radius: 8px; padding: 0 12px; font-weight: 800; }"
        "QPushButton:hover { background: #149362; }"
    );

    QPushButton *btnHistReset = new QPushButton("Reset");
    btnHistReset->setCursor(Qt::PointingHandCursor);
    btnHistReset->setFixedHeight(32);
    btnHistReset->setStyleSheet(
        "QPushButton { background: #607d8b; color: white; border: none; border-radius: 8px; padding: 0 12px; font-weight: 800; }"
        "QPushButton:hover { background: #70909e; }"
    );

    histFilters->addWidget(lblFrom);
    histFilters->addWidget(deFrom);
    histFilters->addWidget(lblTo);
    histFilters->addWidget(deTo);
    histFilters->addWidget(lblMat);
    histFilters->addWidget(leMatFilter, 1);
    histFilters->addWidget(btnHistFiltrer);
    histFilters->addWidget(btnHistReset);
    histL->addLayout(histFilters);

    QTableWidget *tableHist = new QTableWidget(0, 8, onglet);
    tableHist->setHorizontalHeaderLabels(QStringList()
                                         << "Date"
                                         << "Matiere"
                                         << "Fournisseur"
                                         << "Score"
                                         << "Qte"
                                         << "Budget"
                                         << "Risque"
                                         << "Couverture (j)");
    tableHist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableHist->verticalHeader()->setVisible(false);
    tableHist->setAlternatingRowColors(true);
    tableHist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableHist->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableHist->setMinimumHeight(180);
    tableHist->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #cdd9e1; gridline-color: #e4ebf0; }"
        "QHeaderView::section { background: #2d6b8a; color: white; border: none; padding: 6px; font-weight: 700; }"
    );
    histL->addWidget(tableHist);

    QLabel *lblHistStatus = new QLabel("Historique: en attente de chargement.");
    lblHistStatus->setStyleSheet("font-size: 12px; color: #35566a; font-weight: 700;");
    histL->addWidget(lblHistStatus);

    QFrame *trendFrame = new QFrame(onglet);
    trendFrame->setStyleSheet("QFrame { background: #f3f7fa; border: 1px solid #d1dde4; border-radius: 8px; }");
    QVBoxLayout *trendL = new QVBoxLayout(trendFrame);
    trendL->setContentsMargins(8, 6, 8, 6);
    trendL->setSpacing(6);

    QLabel *trendTitle = new QLabel("Evolution recente du budget ravitaillement (DT)");
    trendTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #234253;");

    QWidget *trendChart = new QWidget();
    trendChart->setMinimumHeight(230);

    trendL->addWidget(trendTitle);
    trendL->addWidget(trendChart);
    histL->addWidget(trendFrame);

    l->addWidget(histFrame);

    QDoubleSpinBox *sbDemandeMensuelle = inDemande.second;
    QDoubleSpinBox *sbLeadTime = inLead.second;
    QDoubleSpinBox *sbVariabilite = inVar.second;
    QDoubleSpinBox *sbService = inService.second;
    QDoubleSpinBox *sbCoutPassation = inPassation.second;
    QDoubleSpinBox *sbTauxHolding = inHolding.second;

    auto zFromService = [](double niveauService) {
        if (niveauService >= 99.0) return 2.33;
        if (niveauService >= 97.0) return 1.88;
        if (niveauService >= 95.0) return 1.65;
        if (niveauService >= 90.0) return 1.28;
        return 1.04;
    };

    QObject::connect(btnStrategie, &QPushButton::clicked, this, [=]() {
        if (table->rowCount() <= 0) {
            lblStrategie->setText("Aucune donnee fournisseur disponible. Lancez d'abord 'Analyser fournisseurs'.");
            return;
        }

        int topRow = *selectedSupplierRow;
        if (topRow < 0 || topRow >= table->rowCount()) topRow = 0;

        const QString topSupplier = table->item(topRow, 0) ? table->item(topRow, 0)->text() : QStringLiteral("Inconnu");
        const double prixTop = table->item(topRow, 1) ? parseNumber(table->item(topRow, 1)->text()) : 0.0;
        const double stockTop = table->item(topRow, 4) ? parseNumber(table->item(topRow, 4)->text()) : 0.0;

        int secondRow = -1;
        double secondScore = -1.0;
        for (int i = 0; i < table->rowCount(); ++i) {
            if (i == topRow || !table->item(i, 5)) continue;
            const QString raw = table->item(i, 5)->text().section('/', 0, 0).trimmed();
            const double score = raw.toDouble();
            if (score > secondScore) {
                secondScore = score;
                secondRow = i;
            }
        }

        const QString secondSupplier = (secondRow >= 0 && table->item(secondRow, 0)) ? table->item(secondRow, 0)->text() : QStringLiteral("N/A");
        const double prixSecond = (secondRow >= 0 && table->item(secondRow, 1)) ? parseNumber(table->item(secondRow, 1)->text()) : 0.0;
        const double stockSecond = (secondRow >= 0 && table->item(secondRow, 4)) ? parseNumber(table->item(secondRow, 4)->text()) : 0.0;

        const double demandeMensuelle = sbDemandeMensuelle->value();
        const double lead = sbLeadTime->value();
        const double variabilite = sbVariabilite->value() / 100.0;
        const double service = sbService->value();
        const double coutPassation = sbCoutPassation->value();
        const double tauxHolding = sbTauxHolding->value() / 100.0;

        const double z = zFromService(service);
        const double demandeJour = qMax(0.01, demandeMensuelle / 30.0);
        const double sigma = demandeJour * variabilite * std::sqrt(qMax(1.0, lead));
        const double stockSecurite = z * sigma;
        const double pointCommande = (demandeJour * lead) + stockSecurite;

        const double D = demandeMensuelle * 12.0;
        const double H = qMax(0.01, prixTop * tauxHolding);
        const double eoq = std::sqrt((2.0 * D * coutPassation) / H);

        const double stockActuel = sbStockActuel->value();
        const double besoinRelance = qMax(0.0, pointCommande - stockActuel);
        const double qteCible = qMax(eoq, besoinRelance);

        double qteTop = qteCible;
        double qteSecond = 0.0;
        if (qteTop > stockTop && secondRow >= 0) {
            qteTop = stockTop;
            qteSecond = qMin(qMax(0.0, qteCible - qteTop), stockSecond);
        }
        const double budgetOpt = (qteTop * prixTop) + (qteSecond * prixSecond);

        const double couvertureJours = stockActuel / demandeJour;
        const double risquePct = (stockActuel >= pointCommande)
            ? 18.0
            : qMin(95.0, 50.0 + ((pointCommande - stockActuel) / qMax(1.0, pointCommande)) * 50.0);

        lblKpiBudget->setText(QString::number(budgetOpt, 'f', 0) + " DT");
        lblKpiCouverture->setText(QString::number(couvertureJours, 'f', 1) + " j");
        lblKpiRisqueTxt->setText(risquePct < 35.0 ? "Faible" : (risquePct < 65.0 ? "Modere" : "Eleve"));
        pbRisque->setValue(static_cast<int>(risquePct));
        setAlerteAuto(risquePct < 35.0 ? "FAIBLE" : (risquePct < 65.0 ? "MODERE" : "ELEVE"),
                  risquePct < 35.0
                  ? "Strategie robuste issue du modele EOQ."
                  : (risquePct < 65.0
                 ? "Strategie sensible: renforcer le monitoring fournisseur."
                 : "Strategie critique: lancer double sourcing immediat."));

        lblStrategie->setText(QString(
            "<b>Modele Ingenieur:</b><br>"
            "SS = z x sigma = <b>%1 M2</b> | Point commande = <b>%2 M2</b> | EOQ = <b>%3 M2</b><br>"
            "Quantite cible = max(EOQ, Besoin relance) = <b>%4 M2</b><br>"
            "Strategie sourcing: <b>%5 M2</b> via <b>%6</b>"
            "%7"
            "<br>Budget optimal estime: <b>%8 DT</b><br>"
            "Decision: %9"
        )
            .arg(QString::number(stockSecurite, 'f', 2))
            .arg(QString::number(pointCommande, 'f', 2))
            .arg(QString::number(eoq, 'f', 2))
            .arg(QString::number(qteCible, 'f', 2))
            .arg(QString::number(qteTop, 'f', 2))
            .arg(topSupplier)
            .arg(qteSecond > 0.0
                 ? QString(" + <b>%1 M2</b> via <b>%2</b>").arg(QString::number(qteSecond, 'f', 2), secondSupplier)
                 : QString())
            .arg(QString::number(budgetOpt, 'f', 2))
            .arg(risquePct < 35.0
                 ? QString("Plan robuste. Validez une commande standard.")
                 : (risquePct < 65.0
                    ? QString("Plan sensible. Ajoutez un suivi journalier fournisseur.")
                    : QString("Plan critique. Lancez commande immediate + backup fournisseur.")))
        );
    });

    QHBoxLayout *actions = new QHBoxLayout();
    actions->setContentsMargins(0, 2, 0, 0);
    actions->setSpacing(8);

    QPushButton *btnContact = new QPushButton("Contacter Fournisseur");
    btnContact->setCursor(Qt::PointingHandCursor);
    btnContact->setFixedHeight(38);
    btnContact->setMinimumWidth(150);
    btnContact->setStyleSheet(
        "QPushButton {"
        " background-color: #2a86de;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " font-size: 15px;"
        " font-weight: 700;"
        " padding: 4px 14px;"
        "}"
        "QPushButton:hover { background-color: #4496e6; }"
        "QPushButton:pressed { background-color: #1e6fbc; }"
    );

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setFixedHeight(38);
    btnFermer->setMinimumWidth(90);
    btnFermer->setStyleSheet(
        "QPushButton {"
        " background-color: #43a047;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " font-size: 15px;"
        " font-weight: 700;"
        " padding: 4px 16px;"
        "}"
        "QPushButton:hover { background-color: #4caf50; }"
        "QPushButton:pressed { background-color: #388e3c; }"
    );

    QObject::connect(btnFermer, &QPushButton::clicked, this, [this]() {
        ui->tabWidgetStock->setCurrentIndex(0);
    });

    QPushButton *btnExportRapport = new QPushButton("Exporter Rapport PDF");
    btnExportRapport->setCursor(Qt::PointingHandCursor);
    btnExportRapport->setFixedHeight(38);
    btnExportRapport->setMinimumWidth(170);
    btnExportRapport->setStyleSheet(
        "QPushButton {"
        " background-color: #6a5acd;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " font-size: 14px;"
        " font-weight: 700;"
        " padding: 4px 14px;"
        "}"
        "QPushButton:hover { background-color: #7a69db; }"
        "QPushButton:pressed { background-color: #5b4abb; }"
    );

    QPushButton *btnSauverDecision = new QPushButton("Sauver Historique");
    btnSauverDecision->setCursor(Qt::PointingHandCursor);
    btnSauverDecision->setFixedHeight(38);
    btnSauverDecision->setMinimumWidth(145);
    btnSauverDecision->setStyleSheet(
        "QPushButton {"
        " background-color: #8d5524;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " font-size: 14px;"
        " font-weight: 700;"
        " padding: 4px 14px;"
        "}"
        "QPushButton:hover { background-color: #a3662d; }"
        "QPushButton:pressed { background-color: #75461d; }"
    );

    auto computeSnapshot = [=]() {
        struct Snapshot {
            QString matiere;
            QString fournisseur;
            double score;
            double qte;
            double budget;
            double couverture;
            QString risque;
            QString strategie;
        } s;

        const int row = (*selectedSupplierRow >= 0 && *selectedSupplierRow < table->rowCount()) ? *selectedSupplierRow : 0;
        s.matiere = cbMatiere->currentText();
        s.fournisseur = (table->rowCount() > 0 && table->item(row, 0)) ? table->item(row, 0)->text() : QStringLiteral("Inconnu");
        s.score = (table->rowCount() > 0 && table->item(row, 5)) ? table->item(row, 5)->text().section('/', 0, 0).trimmed().toDouble() : 0.0;

        const double prix = (table->rowCount() > 0 && table->item(row, 1)) ? parseNumber(table->item(row, 1)->text()) : 0.0;
        const double stockDispo = (table->rowCount() > 0 && table->item(row, 4)) ? parseNumber(table->item(row, 4)->text()) : 0.0;
        const double stockActuel = sbStockActuel->value();
        const double seuil = sbSeuilSecurite->value();
        const double conso = sbConsoPrevue->value();
        const double besoinBrut = qMax(0.0, seuil + conso - stockActuel);

        s.qte = qMin(besoinBrut, stockDispo);
        s.budget = s.qte * prix;
        s.couverture = stockActuel / qMax(0.01, conso / 7.0);
        s.risque = lblKpiRisqueTxt->text();
        s.strategie = lblStrategie->text();
        return s;
    };

    auto renderRavitaillementCurve = [=](const QList<double> &values, const QStringList &labels) {
        auto *vl = ensureVBox(trendChart);
        clearLayout(vl);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);

        QList<double> vals = values;
        QStringList xlabels = labels;
        if (vals.isEmpty()) vals << 0.0;
        if (xlabels.isEmpty()) xlabels << "0";

        auto *series = new QLineSeries();
        QPen pen(QColor("#0f7f51"));
        pen.setWidth(3);
        series->setPen(pen);
        series->setPointsVisible(true);

        for (int i = 0; i < vals.size(); ++i) {
            series->append(i, vals.at(i));
        }
        if (vals.size() == 1) {
            series->append(1, vals.first());
            xlabels << (xlabels.first() + " ");
        }

        auto *chart = new QChart();
        chart->addSeries(series);
        styleChartBase(chart);
        chart->setTitle("Courbe du budget");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        if (chart->legend()) chart->legend()->setVisible(false);

        auto *axisX = new QCategoryAxis();
        const int n = vals.size();
        axisX->setRange(-0.4, qMax(1, n - 1) + 0.4);
        if (n <= 3) {
            for (int i = 0; i < xlabels.size() && i < n; ++i) axisX->append(xlabels.at(i), i);
        } else {
            const int mid = n / 2;
            axisX->append(xlabels.value(0), 0);
            axisX->append(xlabels.value(mid), mid);
            axisX->append(xlabels.value(n - 1), n - 1);
        }
        axisX->setLabelsColor(QColor("#3e2723"));
        axisX->setGridLineColor(QColor("#f0e8df"));
        axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        double maxV = 1.0;
        for (double v : vals) maxV = qMax(maxV, v);
        auto *axisY = new QValueAxis();
        axisY->setRange(0.0, maxV * 1.25 + 1.0);
        axisY->applyNiceNumbers();
        axisY->setLabelFormat("%.0f");
        axisY->setLabelsColor(QColor("#3e2723"));
        axisY->setGridLineColor(QColor("#eee5dd"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        auto *view = new QChartView(chart);
        styleChartView(view);
        view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        view->setMinimumHeight(230);
        view->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        vl->addWidget(view);
    };

    auto refreshHistorique = [=]() {
        tableHist->setRowCount(0);
        QStringList budgetCats;
        QList<double> budgetVals;

        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid() || !db.isOpen()) {
            lblHistStatus->setText("Historique: base non connectee.");
            return;
        }

        const QString mat = leMatFilter->text().trimmed();
        const QDateTime fromDt(deFrom->date(), QTime(0, 0, 0));
        const QDateTime toDt(deTo->date().addDays(1), QTime(0, 0, 0));

        QSqlQuery q(db);
        q.prepare(
            "SELECT TO_CHAR(DATE_LOG, 'DD/MM/YYYY HH24:MI'), MATIERE, FOURNISSEUR, SCORE_GLOBAL, "
            "QUANTITE_COMMANDE, BUDGET_ESTIME, RISQUE, COUVERTURE_JOURS "
            "FROM RAVITAILLEMENT_LOG "
            "WHERE DATE_LOG >= :d1 AND DATE_LOG < :d2 "
            "AND (:mat = '' OR UPPER(MATIERE) LIKE :matlike) "
            "ORDER BY DATE_LOG DESC"
        );
        q.bindValue(":d1", fromDt);
        q.bindValue(":d2", toDt);
        q.bindValue(":mat", mat);
        q.bindValue(":matlike", "%" + mat.toUpper() + "%");

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (err.contains("ORA-00942")) {
                lblHistStatus->setText("Historique: aucune table de log encore creee. Sauvez une decision d'abord.");
            } else {
                lblHistStatus->setText("Historique: erreur SQL - " + err);
            }
            return;
        }

        int row = 0;
        while (q.next()) {
            tableHist->insertRow(row);
            for (int c = 0; c < 8; ++c) {
                QString txt = q.value(c).toString();
                if (c == 3 || c == 4 || c == 5 || c == 7) {
                    bool ok = false;
                    double v = txt.toDouble(&ok);
                    if (ok) txt = QString::number(v, 'f', (c == 3 || c == 7) ? 1 : 2);
                }
                auto *it = new QTableWidgetItem(txt);
                it->setTextAlignment((c >= 3) ? (Qt::AlignRight | Qt::AlignVCenter) : (Qt::AlignLeft | Qt::AlignVCenter));
                tableHist->setItem(row, c, it);
            }

            if (budgetCats.size() < 8) {
                const QString rawDate = q.value(0).toString();
                const QString shortDate = (rawDate.size() >= 16)
                    ? rawDate.mid(11, 5)
                    : rawDate;
                budgetCats << shortDate;
                budgetVals << q.value(5).toDouble();
            }
            ++row;
        }

        std::reverse(budgetCats.begin(), budgetCats.end());
        std::reverse(budgetVals.begin(), budgetVals.end());
        renderRavitaillementCurve(budgetVals, budgetCats);

        lblHistStatus->setText(QString("Historique: %1 decision(s) affichee(s).").arg(row));
    };

    QObject::connect(btnExportRapport, &QPushButton::clicked, this, [=]() {
        const auto snap = computeSnapshot();
        QString f = QFileDialog::getSaveFileName(this,
                                                 "Exporter Rapport Ravitaillement",
                                                 "Rapport_Ravitaillement_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") + ".pdf",
                                                 "PDF (*.pdf)");
        if (f.isEmpty()) return;

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setOutputFileName(f);

        QString html = QString(
            "<h2 style='color:#2b3e50;'>Rapport de Ravitaillement - Fil d'Or</h2>"
            "<p><b>Date:</b> %1</p>"
            "<hr>"
            "<p><b>Matiere:</b> %2</p>"
            "<p><b>Fournisseur retenu:</b> %3</p>"
            "<p><b>Score fournisseur:</b> %4 / 100</p>"
            "<p><b>Quantite proposee:</b> %5 M2</p>"
            "<p><b>Budget estime:</b> %6 DT</p>"
            "<p><b>Couverture:</b> %7 jours</p>"
            "<p><b>Risque rupture:</b> %8</p>"
            "<hr><h3>Plan d'action</h3><div>%9</div>"
            "<hr><h3>Optimisation</h3><div>%10</div>"
        )
            .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"))
            .arg(snap.matiere)
            .arg(snap.fournisseur)
            .arg(QString::number(snap.score, 'f', 1))
            .arg(QString::number(snap.qte, 'f', 2))
            .arg(QString::number(snap.budget, 'f', 2))
            .arg(QString::number(snap.couverture, 'f', 1))
            .arg(snap.risque)
            .arg(lblActionPlan->text())
            .arg(snap.strategie);

        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);
        alerteSucces("Export PDF", "Rapport ravitaillement exporte avec succes.");
    });

    QObject::connect(btnSauverDecision, &QPushButton::clicked, this, [=]() {
        const auto snap = computeSnapshot();
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid() || !db.isOpen()) {
            alerteErreur("Base", "Connexion base indisponible pour sauver la decision.");
            return;
        }

        QSqlQuery createQ(db);
        const QString createSql =
            "CREATE TABLE RAVITAILLEMENT_LOG ("
            "LOG_ID VARCHAR2(40) PRIMARY KEY, "
            "DATE_LOG TIMESTAMP, "
            "MATIERE VARCHAR2(80), "
            "FOURNISSEUR VARCHAR2(120), "
            "SCORE_GLOBAL NUMBER(6,2), "
            "QUANTITE_COMMANDE NUMBER(12,2), "
            "BUDGET_ESTIME NUMBER(12,2), "
            "RISQUE VARCHAR2(20), "
            "COUVERTURE_JOURS NUMBER(10,2), "
            "STRATEGIE CLOB)";

        if (!createQ.exec(createSql)) {
            const QString err = createQ.lastError().text();
            if (!err.contains("ORA-00955")) {
                alerteErreur("Base", "Creation table historique impossible: " + err);
                return;
            }
        }

        QSqlQuery ins(db);
        ins.prepare(
            "INSERT INTO RAVITAILLEMENT_LOG "
            "(LOG_ID, DATE_LOG, MATIERE, FOURNISSEUR, SCORE_GLOBAL, QUANTITE_COMMANDE, BUDGET_ESTIME, RISQUE, COUVERTURE_JOURS, STRATEGIE) "
            "VALUES (:id, :dt, :mat, :fou, :score, :qte, :budget, :risque, :cov, :strat)"
        );

        ins.bindValue(":id", QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        ins.bindValue(":dt", QDateTime::currentDateTime());
        ins.bindValue(":mat", snap.matiere.left(80));
        ins.bindValue(":fou", snap.fournisseur.left(120));
        ins.bindValue(":score", snap.score);
        ins.bindValue(":qte", snap.qte);
        ins.bindValue(":budget", snap.budget);
        ins.bindValue(":risque", snap.risque.left(20));
        ins.bindValue(":cov", snap.couverture);
        ins.bindValue(":strat", snap.strategie.left(3900));

        if (!ins.exec()) {
            alerteErreur("Base", "Sauvegarde decision echouee: " + ins.lastError().text());
            return;
        }

        alerteSucces("Historique", "Decision ravitaillement sauvegardee en base.");
        refreshHistorique();
    });

    QObject::connect(btnHistFiltrer, &QPushButton::clicked, this, [=]() { refreshHistorique(); });
    QObject::connect(btnHistReset, &QPushButton::clicked, this, [=]() {
        deFrom->setDate(QDate::currentDate().addDays(-30));
        deTo->setDate(QDate::currentDate());
        leMatFilter->clear();
        refreshHistorique();
    });

    refreshHistorique();

    auto showStyledContactAlert = [this](const QString &title,
                                         const QString &message,
                                         const QString &accent,
                                         const QString &bg,
                                         const QString &btn) {
        QDialog d(this);
        d.setWindowTitle(title);
        d.setModal(true);
        d.setMinimumWidth(460);
        d.setStyleSheet(QString(
            "QDialog { background-color: %1; border: 2px solid %2; border-radius: 12px; }"
            "QFrame#header { background-color: %2; border: none; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
            "QLabel#title { color: white; font-size: 16px; font-weight: 800; border: none; }"
            "QLabel#icon { background-color: %2; color: white; border-radius: 18px; font-size: 20px; font-weight: 900; min-width: 36px; min-height: 36px; max-width: 36px; max-height: 36px; }"
            "QLabel#msg { color: #2f2f2f; font-size: 14px; font-weight: 600; border: none; }"
            "QPushButton { min-width: 95px; min-height: 34px; background-color: %3; color: white; border: none; border-radius: 10px; padding: 6px 14px; font-size: 13px; font-weight: 700; }"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:pressed { background-color: #2b2b2b; }"
        ).arg(bg, accent, btn));

        QVBoxLayout *main = new QVBoxLayout(&d);
        main->setContentsMargins(0, 0, 0, 12);
        main->setSpacing(0);

        QFrame *header = new QFrame(&d);
        header->setObjectName("header");
        QHBoxLayout *hHeader = new QHBoxLayout(header);
        hHeader->setContentsMargins(12, 8, 12, 8);
        QLabel *titleLabel = new QLabel(title, header);
        titleLabel->setObjectName("title");
        hHeader->addWidget(titleLabel);
        hHeader->addStretch();
        main->addWidget(header);

        QHBoxLayout *body = new QHBoxLayout();
        body->setContentsMargins(14, 14, 14, 10);
        body->setSpacing(10);
        QLabel *icon = new QLabel("i", &d);
        icon->setObjectName("icon");
        icon->setAlignment(Qt::AlignCenter);

        QLabel *msg = new QLabel(message, &d);
        msg->setObjectName("msg");
        msg->setWordWrap(true);

        body->addWidget(icon, 0, Qt::AlignTop);
        body->addWidget(msg, 1);
        main->addLayout(body);

        QHBoxLayout *footer = new QHBoxLayout();
        footer->setContentsMargins(14, 0, 14, 0);
        footer->addStretch();
        QPushButton *ok = new QPushButton("OK", &d);
        QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
        footer->addWidget(ok);
        main->addLayout(footer);

        d.exec();
    };

    QObject::connect(btnContact, &QPushButton::clicked, this, [=]() {
        if (table->rowCount() <= 0) {
            showStyledContactAlert("Contact Fournisseur",
                                   "Aucun fournisseur disponible.",
                                   "#c62828",
                                   "#ffefef",
                                   "#e53935");
            return;
        }

        int row = *selectedSupplierRow;
        if (row < 0 || row >= table->rowCount()) {
            row = 0;
        }

        const QString fournisseur = table->item(row, 0) ? table->item(row, 0)->text() : QStringLiteral("Inconnu");
        const QString matiere = cbMatiere->currentText();

        QString email;
        QString telephone;
        if (fournisseur == "TanLeather SA") {
            email = "contact@tanleather.tn";
            telephone = "+216 71 100 201";
        } else if (fournisseur == "Cuir Elite") {
            email = "vente@cuirelite.tn";
            telephone = "+216 71 300 411";
        } else if (fournisseur == "MegaCuir") {
            email = "support@megacuir.tn";
            telephone = "+216 71 555 990";
        } else if (fournisseur == "SoftHide Pro") {
            email = "sales@softhidepro.com";
            telephone = "+33 1 88 70 20 10";
        } else if (fournisseur == "Agneau Plus") {
            email = "contact@agneauplus.com";
            telephone = "+33 1 40 28 64 11";
        } else if (fournisseur == "Cuir Sud") {
            email = "commercial@cuirsud.com";
            telephone = "+216 73 112 800";
        } else if (fournisseur == "Veau Prestige") {
            email = "contact@veauprestige.com";
            telephone = "+33 4 72 10 44 19";
        } else if (fournisseur == "Elite Veau") {
            email = "info@eliteveau.com";
            telephone = "+216 70 991 544";
        } else if (fournisseur == "Market Cuir") {
            email = "sales@marketcuir.com";
            telephone = "+216 74 222 601";
        } else {
            email = "contact@fournisseur.com";
            telephone = "+216 70 000 000";
        }

        showStyledContactAlert(
            "Contact Fournisseur",
            QString("Fournisseur sélectionné : %1\nMatière : %2\n\nTéléphone : %3\nEmail : %4")
                .arg(fournisseur, matiere, telephone, email),
            "#1565c0",
            "#ecf4ff",
            "#1e88e5"
        );
    });

    actions->addWidget(btnContact, 0, Qt::AlignLeft);
    actions->addWidget(btnExportRapport, 0, Qt::AlignLeft);
    actions->addWidget(btnSauverDecision, 0, Qt::AlignLeft);
    actions->addStretch();
    actions->addWidget(btnFermer, 0, Qt::AlignRight);
    l->addLayout(actions);
    l->addStretch();

    ui->tabWidgetStock->setCurrentIndex(4);
}

void MainWindow::showStockCalculTab() {
    if(ui->tabWidgetStock->count() < 6) return;
    QWidget *onglet = ui->tabWidgetStock->widget(5);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *rootLayout = new QVBoxLayout(onglet);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(onglet);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        " background: #efefef;"
        " width: 12px;"
        " margin: 8px 4px 8px 2px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        " background: #cba731;"
        " min-height: 46px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: transparent; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(scrollArea);

    QVBoxLayout *l = new QVBoxLayout(scrollContent);
    l->setContentsMargins(10, 10, 10, 10);
    l->setSpacing(10);

    QFrame *titleFrame = new QFrame(onglet);
    titleFrame->setStyleSheet("QFrame { background-color: #efe6d3; border-radius: 8px; }");
    QHBoxLayout *titleL = new QHBoxLayout(titleFrame);
    QLabel *titre = new QLabel("Calculateur de Besoins Matières", titleFrame);
    titre->setAlignment(Qt::AlignCenter);
    titre->setStyleSheet("font-size: 30px; font-weight: 900; color: #3e2f1f; padding: 10px;");
    titleL->addWidget(titre);
    l->addWidget(titleFrame);

    QFrame *kpiFrame = new QFrame(onglet);
    kpiFrame->setStyleSheet("QFrame { background: #f4f8fb; border: 1px solid #c7dbe7; border-radius: 8px; }");
    QVBoxLayout *kpiWrap = new QVBoxLayout(kpiFrame);
    kpiWrap->setContentsMargins(10, 8, 10, 8);
    kpiWrap->setSpacing(8);

    auto makeCalcKpi = [](const QString &title, const QString &bg, QLabel *&valueLbl) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background: %1; border: 1px solid #c9dce7; border-radius: 8px; }").arg(bg));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(8, 6, 8, 6);
        cl->setSpacing(2);
        QLabel *t = new QLabel(title);
        t->setStyleSheet("font-size: 11px; font-weight: 700; color: #2f4d5e;");
        QLabel *v = new QLabel("--");
        v->setStyleSheet("font-size: 18px; font-weight: 900; color: #122d3a;");
        v->setAlignment(Qt::AlignCenter);
        cl->addWidget(t);
        cl->addWidget(v);
        valueLbl = v;
        return card;
    };

    QLabel *lblKpiBesoin = nullptr;
    QLabel *lblKpiDeficit = nullptr;
    QLabel *lblKpiCouverture = nullptr;
    QLabel *lblKpiRisque = nullptr;
    QHBoxLayout *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(8);
    kpiRow->addWidget(makeCalcKpi("Besoin total", "#e9f7ff", lblKpiBesoin));
    kpiRow->addWidget(makeCalcKpi("Deficit", "#fff1f1", lblKpiDeficit));
    kpiRow->addWidget(makeCalcKpi("Couverture", "#fff8e8", lblKpiCouverture));
    kpiRow->addWidget(makeCalcKpi("Risque", "#eef9ee", lblKpiRisque));
    kpiWrap->addLayout(kpiRow);

    QProgressBar *pbTension = new QProgressBar(onglet);
    pbTension->setRange(0, 100);
    pbTension->setValue(0);
    pbTension->setFormat("Tension d'approvisionnement: %p%");
    pbTension->setStyleSheet(
        "QProgressBar { border: 1px solid #aec7d5; border-radius: 7px; background: #f6fbff; height: 18px; text-align: center; font-size: 12px; font-weight: 700; }"
        "QProgressBar::chunk { border-radius: 6px; background-color: #d9534f; }"
    );
    kpiWrap->addWidget(pbTension);

    QLabel *lblAlerteCalc = new QLabel("Alerte: en attente de calcul.");
    lblAlerteCalc->setStyleSheet("font-size: 12px; font-weight: 700; color: #7a4c12; background: #fff8e9; border: 1px solid #f1d28f; border-radius: 7px; padding: 6px 8px;");
    kpiWrap->addWidget(lblAlerteCalc);

    l->addWidget(kpiFrame);

    QFrame *paramsFrame = new QFrame(onglet);
    paramsFrame->setStyleSheet("QFrame { background: #faf7f2; border: 2px solid #f9a825; border-radius: 8px; }");
    QVBoxLayout *paramsL = new QVBoxLayout(paramsFrame);
    paramsL->setContentsMargins(12, 10, 12, 10);
    paramsL->setSpacing(8);

    QLabel *paramsTitle = new QLabel("Paramètres de Production", paramsFrame);
    paramsTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #2d2d2d; border: none;");
    paramsL->addWidget(paramsTitle);

    QGridLayout *grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);

    QLabel *lblTypeProduit = new QLabel("Type de Produit :", paramsFrame);
    QLabel *lblQuantite = new QLabel("Quantité à produire :", paramsFrame);
    QLabel *lblTypeMatiere = new QLabel("Type de Matière :", paramsFrame);
    QLabel *lblPerte = new QLabel("Marge de Perte :", paramsFrame);
    QLabel *lblScenario = new QLabel("Scénario Demande :", paramsFrame);

    auto inputLabelStyle = QStringLiteral("font-size: 14px; font-weight: 600; color: #3a3a3a; border: none;");
    lblTypeProduit->setStyleSheet(inputLabelStyle);
    lblQuantite->setStyleSheet(inputLabelStyle);
    lblTypeMatiere->setStyleSheet(inputLabelStyle);
    lblPerte->setStyleSheet(inputLabelStyle);
    lblScenario->setStyleSheet(inputLabelStyle);

    QComboBox *cbProduit = new QComboBox(paramsFrame);
    cbProduit->addItems(QStringList() << "Sac Voyage Cuir" << "Sac Main Cuir" << "Portefeuille Cuir" << "Ceinture Cuir");

    QSpinBox *sbQuantite = new QSpinBox(paramsFrame);
    sbQuantite->setRange(1, 100000);
    sbQuantite->setValue(50);
    sbQuantite->setButtonSymbols(QAbstractSpinBox::NoButtons);

    QComboBox *cbMatiere = new QComboBox(paramsFrame);
    cbMatiere->addItems(QStringList() << "Cuir Vachette" << "Cuir Agneau" << "Cuir Veau");
    cbMatiere->setCurrentIndex(0);

    QDoubleSpinBox *dsPerte = new QDoubleSpinBox(paramsFrame);
    dsPerte->setRange(0.0, 100.0);
    dsPerte->setDecimals(1);
    dsPerte->setSingleStep(0.5);
    dsPerte->setValue(15.0);
    dsPerte->setSuffix(" %");
    dsPerte->setButtonSymbols(QAbstractSpinBox::NoButtons);

    QComboBox *cbScenario = new QComboBox(paramsFrame);
    cbScenario->addItems(QStringList() << "Normal" << "Pic saisonnier" << "Urgent client VIP");
    cbScenario->setCurrentIndex(0);

    auto fieldStyle = QStringLiteral(
        "QComboBox, QSpinBox, QDoubleSpinBox {"
        " background: white;"
        " border: 1px solid #b8a9a0;"
        " border-radius: 4px;"
        " padding: 4px 8px;"
        " font-size: 12px;"
        " color: #2f2f2f;"
        "}"
    );
    cbProduit->setStyleSheet(fieldStyle);
    sbQuantite->setStyleSheet(fieldStyle);
    cbMatiere->setStyleSheet(fieldStyle);
    dsPerte->setStyleSheet(fieldStyle);
    cbScenario->setStyleSheet(fieldStyle);

    auto miniBtnStyle = QStringLiteral(
        "QPushButton {"
        " background-color: #efe2cf;"
        " color: #4e342e;"
        " border: 1px solid #c9b299;"
        " border-radius: 3px;"
        " font-size: 10px;"
        " font-weight: 900;"
        " padding: 0px;"
        "}"
        "QPushButton:hover { background-color: #e8d3b5; }"
        "QPushButton:pressed { background-color: #dbbf96; }"
    );

    QWidget *quantiteBox = new QWidget(paramsFrame);
    QHBoxLayout *quantiteL = new QHBoxLayout(quantiteBox);
    quantiteL->setContentsMargins(0, 0, 0, 0);
    quantiteL->setSpacing(1);
    QPushButton *btnMoinsQte = new QPushButton("-", quantiteBox);
    QPushButton *btnPlusQte = new QPushButton("+", quantiteBox);
    btnMoinsQte->setStyleSheet(miniBtnStyle);
    btnPlusQte->setStyleSheet(miniBtnStyle);
    btnMoinsQte->setFixedSize(18, 16);
    btnPlusQte->setFixedSize(18, 16);
    btnMoinsQte->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnPlusQte->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnMoinsQte->setCursor(Qt::PointingHandCursor);
    btnPlusQte->setCursor(Qt::PointingHandCursor);
    quantiteL->addWidget(sbQuantite, 1);
    quantiteL->addWidget(btnMoinsQte);
    quantiteL->addWidget(btnPlusQte);

    QWidget *perteBox = new QWidget(paramsFrame);
    QHBoxLayout *perteL = new QHBoxLayout(perteBox);
    perteL->setContentsMargins(0, 0, 0, 0);
    perteL->setSpacing(1);
    QPushButton *btnMoinsPerte = new QPushButton("-", perteBox);
    QPushButton *btnPlusPerte = new QPushButton("+", perteBox);
    btnMoinsPerte->setStyleSheet(miniBtnStyle);
    btnPlusPerte->setStyleSheet(miniBtnStyle);
    btnMoinsPerte->setFixedSize(18, 16);
    btnPlusPerte->setFixedSize(18, 16);
    btnMoinsPerte->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnPlusPerte->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnMoinsPerte->setCursor(Qt::PointingHandCursor);
    btnPlusPerte->setCursor(Qt::PointingHandCursor);
    perteL->addWidget(dsPerte, 1);
    perteL->addWidget(btnMoinsPerte);
    perteL->addWidget(btnPlusPerte);

    QObject::connect(btnMoinsQte, &QPushButton::clicked, sbQuantite, &QSpinBox::stepDown);
    QObject::connect(btnPlusQte, &QPushButton::clicked, sbQuantite, &QSpinBox::stepUp);
    QObject::connect(btnMoinsPerte, &QPushButton::clicked, dsPerte, &QDoubleSpinBox::stepDown);
    QObject::connect(btnPlusPerte, &QPushButton::clicked, dsPerte, &QDoubleSpinBox::stepUp);

    grid->addWidget(lblTypeProduit, 0, 0);
    grid->addWidget(cbProduit, 0, 1);
    grid->addWidget(lblQuantite, 1, 0);
    grid->addWidget(quantiteBox, 1, 1);
    grid->addWidget(lblTypeMatiere, 2, 0);
    grid->addWidget(cbMatiere, 2, 1);
    grid->addWidget(lblPerte, 3, 0);
    grid->addWidget(perteBox, 3, 1);
    grid->addWidget(lblScenario, 4, 0);
    grid->addWidget(cbScenario, 4, 1);
    paramsL->addLayout(grid);

    QFrame *photoProduitFrame = new QFrame(paramsFrame);
    photoProduitFrame->setStyleSheet(
        "QFrame {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #fffaf1, stop:1 #fffdf8);"
        " border: 1px solid #ead8b6;"
        " border-radius: 10px;"
        "}"
    );
    QVBoxLayout *photoProduitL = new QVBoxLayout(photoProduitFrame);
    photoProduitL->setContentsMargins(10, 8, 10, 8);
    photoProduitL->setSpacing(6);

    QLabel *photoProduitTitle = new QLabel("Aperçu produit sélectionné", photoProduitFrame);
    photoProduitTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #4d331f; border: none;");

    QLabel *photoProduit = new QLabel(photoProduitFrame);
    photoProduit->setMinimumHeight(180);
    photoProduit->setMaximumHeight(210);
    photoProduit->setAlignment(Qt::AlignCenter);
    photoProduit->setStyleSheet(
        "QLabel {"
        " background: #ffffff;"
        " border: 1px solid #d9c7a5;"
        " border-radius: 10px;"
        " padding: 6px;"
        "}"
    );

    QLabel *photoProduitSousTitre = new QLabel(photoProduitFrame);
    photoProduitSousTitre->setWordWrap(true);
    photoProduitSousTitre->setAlignment(Qt::AlignCenter);
    photoProduitSousTitre->setStyleSheet("font-size: 12px; color: #6e4f30; font-weight: 700; border: none;");

    photoProduitL->addWidget(photoProduitTitle);
    photoProduitL->addWidget(photoProduit);
    photoProduitL->addWidget(photoProduitSousTitre);
    paramsL->addWidget(photoProduitFrame);
    l->addWidget(paramsFrame);

    QPushButton *btnCalculer = new QPushButton("Calculer les Besoins", onglet);
    btnCalculer->setCursor(Qt::PointingHandCursor);
    btnCalculer->setFixedHeight(40);
    btnCalculer->setStyleSheet(
        "QPushButton {"
        " background-color: #f9a825;"
        " color: white;"
        " border: none;"
        " border-radius: 12px;"
        " font-size: 16px;"
        " font-weight: 800;"
        "}"
        "QPushButton:hover { background-color: #ffb300; }"
        "QPushButton:pressed { background-color: #f57f17; }"
    );
    l->addWidget(btnCalculer);

    QFrame *resultFrame = new QFrame(onglet);
    resultFrame->setStyleSheet("QFrame { background: #edf8ee; border: 2px solid #66bb6a; border-radius: 8px; }");
    QVBoxLayout *resultL = new QVBoxLayout(resultFrame);
    resultL->setContentsMargins(12, 8, 12, 8);

    QLabel *resultTitle = new QLabel("Résultats du Calcul", resultFrame);
    resultTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #1f1f1f; border: none;");

    QLabel *resultText = new QLabel(resultFrame);
    resultText->setTextFormat(Qt::RichText);
    resultText->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    resultText->setStyleSheet("font-size: 15px; color: #222; border: none; line-height: 1.5; padding-top: 4px;");

    QLabel *lblActionCalc = new QLabel("Action manageriale: en attente de calcul.", resultFrame);
    lblActionCalc->setWordWrap(true);
    lblActionCalc->setStyleSheet("font-size: 13px; color: #153f2f; background: #ecf9ef; border: 1px solid #b7e2c1; border-radius: 8px; padding: 7px;");

    resultL->addWidget(resultTitle);
    resultL->addWidget(resultText);
    resultL->addWidget(lblActionCalc);
    l->addWidget(resultFrame);

    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(10);

    QPushButton *btnReserver = new QPushButton("Réserver Matière", onglet);
    btnReserver->setCursor(Qt::PointingHandCursor);
    btnReserver->setFixedHeight(38);
    btnReserver->setStyleSheet(
        "QPushButton {"
        " background-color: #2a86de;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " padding: 4px 16px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #4496e6; }"
        "QPushButton:pressed { background-color: #1e6fbc; }"
    );

    QPushButton *btnCommander = new QPushButton("Commander Plus", onglet);
    btnCommander->setCursor(Qt::PointingHandCursor);
    btnCommander->setFixedHeight(38);
    btnCommander->setStyleSheet(
        "QPushButton {"
        " background-color: #e65100;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " padding: 4px 16px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #f4511e; }"
        "QPushButton:pressed { background-color: #bf360c; }"
    );

    QPushButton *btnExportCalcul = new QPushButton("Exporter Rapport PDF", onglet);
    btnExportCalcul->setCursor(Qt::PointingHandCursor);
    btnExportCalcul->setFixedHeight(38);
    btnExportCalcul->setStyleSheet(
        "QPushButton {"
        " background-color: #6a5acd;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " padding: 4px 16px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #7a69db; }"
        "QPushButton:pressed { background-color: #5b4abb; }"
    );

    QPushButton *btnSaveCalcul = new QPushButton("Sauver Historique", onglet);
    btnSaveCalcul->setCursor(Qt::PointingHandCursor);
    btnSaveCalcul->setFixedHeight(38);
    btnSaveCalcul->setStyleSheet(
        "QPushButton {"
        " background-color: #8d5524;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " padding: 4px 16px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #a3662d; }"
        "QPushButton:pressed { background-color: #75461d; }"
    );

    QPushButton *btnFermer = new QPushButton("Fermer", onglet);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setFixedHeight(38);
    btnFermer->setMinimumWidth(95);
    btnFermer->setStyleSheet(
        "QPushButton {"
        " background-color: #43a047;"
        " color: white;"
        " border: none;"
        " border-radius: 18px;"
        " padding: 4px 16px;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #4caf50; }"
        "QPushButton:pressed { background-color: #388e3c; }"
    );

    actions->addWidget(btnReserver);
    actions->addWidget(btnCommander);
    actions->addWidget(btnExportCalcul);
    actions->addWidget(btnSaveCalcul);
    actions->addStretch();
    actions->addWidget(btnFermer);
    l->addLayout(actions);

    QFrame *histCalcFrame = new QFrame(onglet);
    histCalcFrame->setStyleSheet("QFrame { background: #f8fafb; border: 1px solid #d2dde3; border-radius: 10px; }");
    QVBoxLayout *histCalcL = new QVBoxLayout(histCalcFrame);
    histCalcL->setContentsMargins(12, 10, 12, 10);
    histCalcL->setSpacing(8);

    QLabel *histCalcTitle = new QLabel("Historique calcul des besoins");
    histCalcTitle->setStyleSheet("font-size: 16px; font-weight: 900; color: #1f3644;");
    histCalcL->addWidget(histCalcTitle);

    QHBoxLayout *histFilters = new QHBoxLayout();
    histFilters->setSpacing(8);

    QLabel *lblFrom = new QLabel("Du:");
    QLabel *lblTo = new QLabel("Au:");
    QLabel *lblMat = new QLabel("Matiere:");
    lblFrom->setStyleSheet("font-weight: 700; color: #2c4b5d;");
    lblTo->setStyleSheet("font-weight: 700; color: #2c4b5d;");
    lblMat->setStyleSheet("font-weight: 700; color: #2c4b5d;");

    QDateEdit *deFrom = new QDateEdit(QDate::currentDate().addDays(-30), onglet);
    QDateEdit *deTo = new QDateEdit(QDate::currentDate(), onglet);
    deFrom->setCalendarPopup(true);
    deTo->setCalendarPopup(true);
    deFrom->setDisplayFormat("dd/MM/yyyy");
    deTo->setDisplayFormat("dd/MM/yyyy");
    deFrom->setFixedHeight(32);
    deTo->setFixedHeight(32);

    QLineEdit *leMatFilter = new QLineEdit(onglet);
    leMatFilter->setPlaceholderText("ex: Cuir Vachette");
    leMatFilter->setFixedHeight(32);
    leMatFilter->setStyleSheet("QLineEdit { background: white; border: 1px solid #9ab8c7; border-radius: 7px; padding: 4px 8px; }");

    auto styleDate = QStringLiteral("QDateEdit { background: white; border: 1px solid #9ab8c7; border-radius: 7px; padding: 4px 8px; }");
    deFrom->setStyleSheet(styleDate);
    deTo->setStyleSheet(styleDate);

    QPushButton *btnHistFiltrer = new QPushButton("Filtrer");
    btnHistFiltrer->setCursor(Qt::PointingHandCursor);
    btnHistFiltrer->setFixedHeight(32);
    btnHistFiltrer->setStyleSheet(
        "QPushButton { background: #0f7f51; color: white; border: none; border-radius: 8px; padding: 0 12px; font-weight: 800; }"
        "QPushButton:hover { background: #149362; }"
    );

    QPushButton *btnHistReset = new QPushButton("Reset");
    btnHistReset->setCursor(Qt::PointingHandCursor);
    btnHistReset->setFixedHeight(32);
    btnHistReset->setStyleSheet(
        "QPushButton { background: #607d8b; color: white; border: none; border-radius: 8px; padding: 0 12px; font-weight: 800; }"
        "QPushButton:hover { background: #70909e; }"
    );

    histFilters->addWidget(lblFrom);
    histFilters->addWidget(deFrom);
    histFilters->addWidget(lblTo);
    histFilters->addWidget(deTo);
    histFilters->addWidget(lblMat);
    histFilters->addWidget(leMatFilter, 1);
    histFilters->addWidget(btnHistFiltrer);
    histFilters->addWidget(btnHistReset);
    histCalcL->addLayout(histFilters);

    QTableWidget *tableHist = new QTableWidget(0, 8, onglet);
    tableHist->setHorizontalHeaderLabels(QStringList()
                                         << "Date"
                                         << "Matiere"
                                         << "Produit"
                                         << "Scenario"
                                         << "Besoin"
                                         << "Deficit"
                                         << "Risque"
                                         << "Couverture");
    tableHist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableHist->verticalHeader()->setVisible(false);
    tableHist->setAlternatingRowColors(true);
    tableHist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableHist->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableHist->setMinimumHeight(170);
    tableHist->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #cdd9e1; gridline-color: #e4ebf0; }"
        "QHeaderView::section { background: #2d6b8a; color: white; border: none; padding: 6px; font-weight: 700; }"
    );
    histCalcL->addWidget(tableHist);

    QLabel *lblHistStatus = new QLabel("Historique: en attente de chargement.");
    lblHistStatus->setStyleSheet("font-size: 12px; color: #35566a; font-weight: 700;");
    histCalcL->addWidget(lblHistStatus);

    QFrame *trendFrame = new QFrame(onglet);
    trendFrame->setStyleSheet("QFrame { background: #f3f7fa; border: 1px solid #d1dde4; border-radius: 8px; }");
    QVBoxLayout *trendL = new QVBoxLayout(trendFrame);
    trendL->setContentsMargins(8, 6, 8, 6);
    trendL->setSpacing(6);

    QLabel *trendTitle = new QLabel("Evolution recente du deficit (M2)");
    trendTitle->setStyleSheet("font-size: 13px; font-weight: 800; color: #234253;");

    QScrollArea *trendScroll = new QScrollArea(trendFrame);
    trendScroll->setWidgetResizable(false);
    trendScroll->setFrameShape(QFrame::NoFrame);
    trendScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    trendScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    trendScroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:horizontal {"
        " background: #efefef;"
        " height: 12px;"
        " margin: 2px 8px 2px 8px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        " background: #cba731;"
        " min-width: 46px;"
        " border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal:hover { background: #d6b64b; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background: transparent; border: none; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }"
    );

    QWidget *trendChart = new QWidget();
    trendChart->setMinimumHeight(230);
    trendChart->setMinimumWidth(1050);
    trendScroll->setWidget(trendChart);

    trendL->addWidget(trendTitle);
    trendL->addWidget(trendScroll);
    histCalcL->addWidget(trendFrame);

    l->addWidget(histCalcFrame);

    auto *stockMap = new QMap<QString, double>({
        {"Cuir Vachette", 100.0},
        {"Cuir Agneau", 80.0},
        {"Cuir Veau", 65.0}
    });

    auto *surfaceMap = new QMap<QString, double>({
        {"Sac Voyage Cuir", 0.8},
        {"Sac Main Cuir", 0.6},
        {"Portefeuille Cuir", 0.25},
        {"Ceinture Cuir", 0.18}
    });

    auto scenarioFactor = [=]() {
        const QString s = cbScenario->currentText();
        if (s == "Pic saisonnier") return 1.25;
        if (s == "Urgent client VIP") return 1.45;
        return 1.0;
    };

    auto majPhotoProduit = [=](const QString &produit) {
        QString chemin;
        QString legende;

        if (produit == "Sac Main Cuir") {
            chemin = ":/sac main.webp";
            legende = "Sac main cuir: finition elegante, cible premium et forte valeur ajoutee.";
        } else if (produit == "Portefeuille Cuir") {
            chemin = ":/portfeuille.webp";
            legende = "Portefeuille cuir: format compact, cadence elevee, precision de coupe.";
        } else if (produit == "Ceinture Cuir") {
            chemin = ":/ceinture.webp";
            legende = "Ceinture cuir: piece technique, regularite de qualite et resistance.";
        } else {
            chemin = ":/sac voyage.jpg";
            legende = "Sac voyage cuir: grande surface, produit iconique a fort impact visuel.";
        }

        QPixmap pix(chemin);
        if (pix.isNull()) {
            photoProduit->setText("Image indisponible");
            photoProduitSousTitre->setText("Verifiez la ressource image du produit dans le projet.");
            return;
        }

        photoProduit->setPixmap(pix.scaled(photoProduit->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        photoProduitSousTitre->setText(legende);
    };

    auto setAlerteCalc = [=](const QString &niveau, const QString &message) {
        lblAlerteCalc->setText("Alerte [" + niveau + "]: " + message);
        if (niveau == "ELEVE") {
            lblAlerteCalc->setStyleSheet("font-size: 12px; font-weight: 800; color: #8f1d1d; background: #ffecec; border: 1px solid #f3adad; border-radius: 7px; padding: 6px 8px;");
        } else if (niveau == "MODERE") {
            lblAlerteCalc->setStyleSheet("font-size: 12px; font-weight: 800; color: #7a4c12; background: #fff8e9; border: 1px solid #f1d28f; border-radius: 7px; padding: 6px 8px;");
        } else {
            lblAlerteCalc->setStyleSheet("font-size: 12px; font-weight: 800; color: #1d6f38; background: #ecfbf2; border: 1px solid #a9dfbe; border-radius: 7px; padding: 6px 8px;");
        }
    };

    auto calculerBesoin = [=]() {
        const QString produit = cbProduit->currentText();
        const QString matiere = cbMatiere->currentText();
        const int quantite = sbQuantite->value();
        const double pertePct = dsPerte->value();
        const double facteurScenario = scenarioFactor();

        const double surfaceUnitaire = surfaceMap->value(produit, 0.8);
        const double surfaceTotale = surfaceUnitaire * static_cast<double>(quantite) * facteurScenario;
        const double surfaceAvecPerte = surfaceTotale * (1.0 + (pertePct / 100.0));
        const double stockDispo = stockMap->value(matiere, 0.0);
        const bool suffisant = stockDispo >= surfaceAvecPerte;
        const double deficit = qMax(0.0, surfaceAvecPerte - stockDispo);
        const double couverturePct = (surfaceAvecPerte > 0.0) ? qMin(100.0, (stockDispo / surfaceAvecPerte) * 100.0) : 100.0;
        const double tension = qBound(0.0, 100.0 - couverturePct, 100.0);

        resultFrame->setStyleSheet(suffisant
            ? "QFrame { background: #edf8ee; border: 2px solid #66bb6a; border-radius: 8px; }"
            : "QFrame { background: #fff0f0; border: 2px solid #ef5350; border-radius: 8px; }");

        const QString statut = suffisant ? "SUFFISANT" : "INSUFFISANT";
        const QString statutColor = suffisant ? "#1b5e20" : "#b71c1c";

        resultText->setText(QString(
            "Surface nécessaire par unité : <b>%1 M²</b><br>"
            "Scénario appliqué : <b>%8</b> (x%9)<br>"
            "Surface totale ajustée (sans perte) : <b>%2 M²</b><br>"
            "Surface avec marge de perte (%3%) : <b>%4 M²</b><br>"
            "Stock disponible : <b>%5 M²</b><br><br>"
            "Déficit estimé : <b>%10 M²</b><br>"
            "<div style='text-align:center; font-weight:900; color:%6;'>Statut : %7</div>"
        )
            .arg(QString::number(surfaceUnitaire, 'f', 2))
            .arg(QString::number(surfaceTotale, 'f', 2))
            .arg(QString::number(pertePct, 'f', 1))
            .arg(QString::number(surfaceAvecPerte, 'f', 2))
            .arg(QString::number(stockDispo, 'f', 2))
            .arg(statutColor)
            .arg(statut)
            .arg(cbScenario->currentText())
            .arg(QString::number(facteurScenario, 'f', 2))
            .arg(QString::number(deficit, 'f', 2)));

        lblKpiBesoin->setText(QString::number(surfaceAvecPerte, 'f', 1) + " M2");
        lblKpiDeficit->setText(QString::number(deficit, 'f', 1) + " M2");
        lblKpiCouverture->setText(QString::number(couverturePct, 'f', 0) + " %");
        pbTension->setValue(static_cast<int>(tension));

        if (deficit <= 0.0) {
            lblKpiRisque->setText("Faible");
            lblActionCalc->setText("Action manageriale: stock suffisant. Valider la reservation puis lancer la production.");
            setAlerteCalc("FAIBLE", "Couverture complete sur le scenario courant.");
        } else if (deficit <= 40.0) {
            lblKpiRisque->setText("Modere");
            lblActionCalc->setText("Action manageriale: commander un complement et suivre l'etat de stock chaque jour.");
            setAlerteCalc("MODERE", "Deficit partiel detecte. Approvisionnement recommande.");
        } else {
            lblKpiRisque->setText("Eleve");
            lblActionCalc->setText("Action manageriale: prioriser un achat urgent + replanifier les ordres de fabrication.");
            setAlerteCalc("ELEVE", "Risque de rupture important sur ce scenario.");
        }
    };

    auto showStyledInfo = [this](const QString &title,
                                 const QString &message,
                                 const QString &accent,
                                 const QString &bg,
                                 const QString &btn) {
        QDialog d(this);
        d.setWindowTitle(title);
        d.setModal(true);
        d.setMinimumWidth(430);
        d.setStyleSheet(QString(
            "QDialog { background-color: %1; border: 2px solid %2; border-radius: 12px; }"
            "QFrame#header { background-color: %2; border: none; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
            "QLabel#title { color: white; font-size: 16px; font-weight: 800; border: none; }"
            "QLabel#icon { background-color: %2; color: white; border-radius: 18px; font-size: 20px; font-weight: 900; min-width: 36px; min-height: 36px; max-width: 36px; max-height: 36px; }"
            "QLabel#msg { color: #2f2f2f; font-size: 14px; font-weight: 600; border: none; }"
            "QPushButton { min-width: 95px; min-height: 34px; background-color: %3; color: white; border: none; border-radius: 10px; padding: 6px 14px; font-size: 13px; font-weight: 700; }"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:pressed { background-color: #2b2b2b; }"
        ).arg(bg, accent, btn));

        QVBoxLayout *main = new QVBoxLayout(&d);
        main->setContentsMargins(0, 0, 0, 12);
        main->setSpacing(0);

        QFrame *header = new QFrame(&d);
        header->setObjectName("header");
        QHBoxLayout *hHeader = new QHBoxLayout(header);
        hHeader->setContentsMargins(12, 8, 12, 8);
        QLabel *titleLabel = new QLabel(title, header);
        titleLabel->setObjectName("title");
        hHeader->addWidget(titleLabel);
        hHeader->addStretch();
        main->addWidget(header);

        QHBoxLayout *body = new QHBoxLayout();
        body->setContentsMargins(14, 14, 14, 10);
        body->setSpacing(10);
        QLabel *icon = new QLabel("i", &d);
        icon->setObjectName("icon");
        icon->setAlignment(Qt::AlignCenter);

        QLabel *msg = new QLabel(message, &d);
        msg->setObjectName("msg");
        msg->setWordWrap(true);

        body->addWidget(icon, 0, Qt::AlignTop);
        body->addWidget(msg, 1);
        main->addLayout(body);

        QHBoxLayout *footer = new QHBoxLayout();
        footer->setContentsMargins(14, 0, 14, 0);
        footer->addStretch();
        QPushButton *ok = new QPushButton("OK", &d);
        QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
        footer->addWidget(ok);
        main->addLayout(footer);

        d.exec();
    };

    auto computeCalcSnapshot = [=]() {
        struct CalcSnapshot {
            QString matiere;
            QString produit;
            QString scenario;
            double besoin;
            double deficit;
            double couverture;
            QString risque;
        } s;

        s.matiere = cbMatiere->currentText();
        s.produit = cbProduit->currentText();
        s.scenario = cbScenario->currentText();

        const double facteurScenario = scenarioFactor();
        const double surfaceUnitaire = surfaceMap->value(s.produit, 0.8);
        const double surfaceTotale = surfaceUnitaire * static_cast<double>(sbQuantite->value()) * facteurScenario;
        s.besoin = surfaceTotale * (1.0 + dsPerte->value() / 100.0);

        const double stockDispo = stockMap->value(s.matiere, 0.0);
        s.deficit = qMax(0.0, s.besoin - stockDispo);
        s.couverture = (s.besoin > 0.0) ? qMin(100.0, (stockDispo / s.besoin) * 100.0) : 100.0;
        s.risque = lblKpiRisque->text();
        return s;
    };

    auto renderDeficitCurve = [=](const QList<double> &values, const QStringList &labels) {
        auto *vl = ensureVBox(trendChart);
        clearLayout(vl);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);

        QList<double> vals = values;
        QStringList xlabels = labels;
        if (vals.isEmpty()) vals << 0.0;
        if (xlabels.isEmpty()) xlabels << "0";

        auto *series = new QLineSeries();
        QPen pen(QColor("#d77a00"));
        pen.setWidth(3);
        series->setPen(pen);
        series->setPointsVisible(true);
        series->setPointLabelsVisible(false);

        for (int i = 0; i < vals.size(); ++i) {
            series->append(i, vals.at(i));
        }
        if (vals.size() == 1) {
            series->append(1, vals.first()); // trace une ligne plate meme avec 1 seul point historique
            xlabels << (xlabels.first() + " ");
        }

        auto *chart = new QChart();
        chart->addSeries(series);
        styleChartBase(chart);
        chart->setTitle("Courbe du deficit");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        if (chart->legend()) chart->legend()->setVisible(false);

        bool allZero = true;
        for (double v : vals) {
            if (qAbs(v) > 0.0001) {
                allZero = false;
                break;
            }
        }
        if (allZero) {
            chart->setTitle("Courbe du deficit (aucun deficit sur la periode)");
        }

        auto *axisX = new QCategoryAxis();
        const int n = vals.size();
        axisX->setRange(-0.4, qMax(1, n - 1) + 0.4);

        if (n <= 3) {
            for (int i = 0; i < xlabels.size() && i < n; ++i) {
                axisX->append(xlabels.at(i), i);
            }
        } else {
            const int mid = n / 2;
            axisX->append(xlabels.value(0), 0);
            axisX->append(xlabels.value(mid), mid);
            axisX->append(xlabels.value(n - 1), n - 1);
        }
        axisX->setLabelsColor(QColor("#3e2723"));
        axisX->setGridLineColor(QColor("#f0e8df"));
        axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        axisX->setLabelsAngle(0);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        double maxV = 1.0;
        for (double v : vals) maxV = qMax(maxV, v);
        auto *axisY = new QValueAxis();
        axisY->setRange(0.0, maxV * 1.25 + 0.5);
        axisY->applyNiceNumbers();
        axisY->setLabelFormat("%.1f");
        axisY->setLabelsColor(QColor("#3e2723"));
        axisY->setGridLineColor(QColor("#eee5dd"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        auto *view = new QChartView(chart);
        styleChartView(view);
        view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        view->setMinimumHeight(230);
        view->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        vl->addWidget(view);
    };

    auto refreshCalcHistorique = [=]() {
        tableHist->setRowCount(0);
        QStringList deficitCats;
        QList<double> deficitVals;

        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid() || !db.isOpen()) {
            lblHistStatus->setText("Historique: base non connectee.");
            return;
        }

        const QString mat = leMatFilter->text().trimmed();
        const QDateTime fromDt(deFrom->date(), QTime(0, 0, 0));
        const QDateTime toDt(deTo->date().addDays(1), QTime(0, 0, 0));

        QSqlQuery q(db);
        q.prepare(
            "SELECT TO_CHAR(DATE_LOG, 'DD/MM/YYYY HH24:MI'), MATIERE, PRODUIT, SCENARIO, "
            "BESOIN_TOTAL, DEFICIT, RISQUE, COUVERTURE_PCT "
            "FROM CALCUL_BESOINS_LOG "
            "WHERE DATE_LOG >= :d1 AND DATE_LOG < :d2 "
            "AND (:mat = '' OR UPPER(MATIERE) LIKE :matlike) "
            "ORDER BY DATE_LOG DESC"
        );
        q.bindValue(":d1", fromDt);
        q.bindValue(":d2", toDt);
        q.bindValue(":mat", mat);
        q.bindValue(":matlike", "%" + mat.toUpper() + "%");

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (err.contains("ORA-00942")) {
                lblHistStatus->setText("Historique: table non creee. Sauvez un calcul d'abord.");
            } else {
                lblHistStatus->setText("Historique: erreur SQL - " + err);
            }
            return;
        }

        int row = 0;
        while (q.next()) {
            tableHist->insertRow(row);
            const QString risque = q.value(6).toString().trimmed().toUpper();
            QColor riskBg = QColor("#ffffff");
            QColor riskFg = QColor("#1f1f1f");
            if (risque.contains("ELEVE")) {
                riskBg = QColor("#ffecec");
                riskFg = QColor("#8f1d1d");
            } else if (risque.contains("MODERE")) {
                riskBg = QColor("#fff8e9");
                riskFg = QColor("#7a4c12");
            } else if (risque.contains("FAIBLE")) {
                riskBg = QColor("#ecfbf2");
                riskFg = QColor("#1d6f38");
            }

            for (int c = 0; c < 8; ++c) {
                QString txt = q.value(c).toString();
                if (c >= 4) {
                    bool ok = false;
                    const double v = txt.toDouble(&ok);
                    if (ok) txt = QString::number(v, 'f', 1);
                }
                auto *it = new QTableWidgetItem(txt);
                it->setTextAlignment((c >= 4 && c != 6) ? (Qt::AlignRight | Qt::AlignVCenter) : (Qt::AlignLeft | Qt::AlignVCenter));
                if (c == 6) {
                    it->setBackground(QBrush(riskBg));
                    it->setForeground(QBrush(riskFg));
                    QFont f = it->font();
                    f.setBold(true);
                    it->setFont(f);
                }
                tableHist->setItem(row, c, it);
            }

            if (deficitCats.size() < 7) {
                const QString rawDate = q.value(0).toString(); // DD/MM/YYYY HH24:MI
                const QString shortDate = (rawDate.size() >= 16)
                    ? rawDate.mid(11, 5)
                    : rawDate;
                deficitCats << shortDate;
                deficitVals << q.value(5).toDouble();
            }
            ++row;
        }

        std::reverse(deficitCats.begin(), deficitCats.end());
        std::reverse(deficitVals.begin(), deficitVals.end());
        renderDeficitCurve(deficitVals, deficitCats);

        lblHistStatus->setText(QString("Historique: %1 calcul(s) affiche(s).").arg(row));
    };

    QObject::connect(btnExportCalcul, &QPushButton::clicked, this, [=]() {
        const auto snap = computeCalcSnapshot();
        QString f = QFileDialog::getSaveFileName(this,
                                                 "Exporter Rapport Calcul",
                                                 "Rapport_Calcul_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") + ".pdf",
                                                 "PDF (*.pdf)");
        if (f.isEmpty()) return;

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setOutputFileName(f);

        QString html = QString(
            "<h2 style='color:#2b3e50;'>Rapport Calcul Besoins - Fil d'Or</h2>"
            "<p><b>Date:</b> %1</p><hr>"
            "<p><b>Matiere:</b> %2</p>"
            "<p><b>Produit:</b> %3</p>"
            "<p><b>Scenario:</b> %4</p>"
            "<p><b>Besoin total:</b> %5 M2</p>"
            "<p><b>Deficit:</b> %6 M2</p>"
            "<p><b>Couverture:</b> %7 %%</p>"
            "<p><b>Risque:</b> %8</p>"
            "<hr><h3>Action manageriale</h3><div>%9</div>"
        )
            .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm"))
            .arg(snap.matiere)
            .arg(snap.produit)
            .arg(snap.scenario)
            .arg(QString::number(snap.besoin, 'f', 2))
            .arg(QString::number(snap.deficit, 'f', 2))
            .arg(QString::number(snap.couverture, 'f', 1))
            .arg(snap.risque)
            .arg(lblActionCalc->text());

        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);
        alerteSucces("Export PDF", "Rapport calcul exporte avec succes.");
    });

    QObject::connect(btnSaveCalcul, &QPushButton::clicked, this, [=]() {
        const auto snap = computeCalcSnapshot();
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid() || !db.isOpen()) {
            alerteErreur("Base", "Connexion base indisponible pour sauver le calcul.");
            return;
        }

        QSqlQuery createQ(db);
        const QString createSql =
            "CREATE TABLE CALCUL_BESOINS_LOG ("
            "LOG_ID VARCHAR2(40) PRIMARY KEY, "
            "DATE_LOG TIMESTAMP, "
            "MATIERE VARCHAR2(80), "
            "PRODUIT VARCHAR2(120), "
            "SCENARIO VARCHAR2(60), "
            "BESOIN_TOTAL NUMBER(12,2), "
            "DEFICIT NUMBER(12,2), "
            "COUVERTURE_PCT NUMBER(8,2), "
            "RISQUE VARCHAR2(20), "
            "ACTION_TXT CLOB)";

        if (!createQ.exec(createSql)) {
            const QString err = createQ.lastError().text();
            if (!err.contains("ORA-00955")) {
                alerteErreur("Base", "Creation table calcul impossible: " + err);
                return;
            }
        }

        QSqlQuery ins(db);
        ins.prepare(
            "INSERT INTO CALCUL_BESOINS_LOG "
            "(LOG_ID, DATE_LOG, MATIERE, PRODUIT, SCENARIO, BESOIN_TOTAL, DEFICIT, COUVERTURE_PCT, RISQUE, ACTION_TXT) "
            "VALUES (:id, :dt, :mat, :prod, :sc, :bes, :def, :cov, :ris, :act)"
        );
        ins.bindValue(":id", QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        ins.bindValue(":dt", QDateTime::currentDateTime());
        ins.bindValue(":mat", snap.matiere.left(80));
        ins.bindValue(":prod", snap.produit.left(120));
        ins.bindValue(":sc", snap.scenario.left(60));
        ins.bindValue(":bes", snap.besoin);
        ins.bindValue(":def", snap.deficit);
        ins.bindValue(":cov", snap.couverture);
        ins.bindValue(":ris", snap.risque.left(20));
        ins.bindValue(":act", lblActionCalc->text().left(3900));

        if (!ins.exec()) {
            alerteErreur("Base", "Sauvegarde calcul echouee: " + ins.lastError().text());
            return;
        }

        alerteSucces("Historique", "Calcul sauvegarde en base.");
        refreshCalcHistorique();
    });

    QObject::connect(btnHistFiltrer, &QPushButton::clicked, this, [=]() { refreshCalcHistorique(); });
    QObject::connect(btnHistReset, &QPushButton::clicked, this, [=]() {
        deFrom->setDate(QDate::currentDate().addDays(-30));
        deTo->setDate(QDate::currentDate());
        leMatFilter->clear();
        refreshCalcHistorique();
    });

    QObject::connect(btnCalculer, &QPushButton::clicked, this, [=]() {
        calculerBesoin();
    });
    QObject::connect(cbScenario, &QComboBox::currentTextChanged, this, [=](const QString &){ calculerBesoin(); });
    QObject::connect(cbProduit, &QComboBox::currentTextChanged, this, [=](const QString &produit){
        majPhotoProduit(produit);
        calculerBesoin();
    });
    QObject::connect(cbMatiere, &QComboBox::currentTextChanged, this, [=](const QString &){ calculerBesoin(); });
    QObject::connect(sbQuantite, qOverload<int>(&QSpinBox::valueChanged), this, [=](int){ calculerBesoin(); });
    QObject::connect(dsPerte, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double){ calculerBesoin(); });

    QObject::connect(btnReserver, &QPushButton::clicked, this, [=]() {
        const QString matiere = cbMatiere->currentText();
        const QString produit = cbProduit->currentText();
        const double surfaceUnitaire = surfaceMap->value(produit, 0.8);
        const double surfaceAvecPerte = (surfaceUnitaire * sbQuantite->value()) * (1.0 + dsPerte->value() / 100.0);
        const double stockDispo = stockMap->value(matiere, 0.0);

        if (stockDispo >= surfaceAvecPerte) {
            (*stockMap)[matiere] = stockDispo - surfaceAvecPerte;
            calculerBesoin();
            showStyledInfo("Réservation",
                           "La matière a été réservée avec succès.",
                           "#2e7d32",
                           "#edf8ee",
                           "#43a047");
        } else {
            QMessageBox::warning(this, "Stock insuffisant", "Stock insuffisant pour réserver cette quantité. Utilisez 'Commander Plus'.");
        }
    });

    QObject::connect(btnCommander, &QPushButton::clicked, this, [=]() {
        const QString matiere = cbMatiere->currentText();
        const QString produit = cbProduit->currentText();
        const double surfaceUnitaire = surfaceMap->value(produit, 0.8);
        const double besoin = (surfaceUnitaire * sbQuantite->value()) * (1.0 + dsPerte->value() / 100.0);
        const double stockActuel = stockMap->value(matiere, 0.0);
        const double ajout = std::max(50.0, besoin - stockActuel + 10.0);

        (*stockMap)[matiere] = stockActuel + ajout;
        calculerBesoin();
        showStyledInfo("Commande",
                       QString("Commande ajoutée : +%1 M² de %2.")
                           .arg(QString::number(ajout, 'f', 2), matiere),
                       "#ef6c00",
                       "#fff4e8",
                       "#fb8c00");
    });

    QObject::connect(btnFermer, &QPushButton::clicked, this, [this]() {
        ui->tabWidgetStock->setCurrentIndex(0);
    });

    majPhotoProduit(cbProduit->currentText());
    calculerBesoin();
    refreshCalcHistorique();
    ui->tabWidgetStock->setCurrentIndex(5);
}


// --- MODULE IA : ESTIMATION DE TEMPS AVANCEE ---
void MainWindow::ouvrirIAPrediction() {
    int idx = ui->tablePlanif->currentRow();
    if(idx < 0) {
        alerteWarning("Assistant IA", "Veuillez d'abord selectionner une commande dans la liste.");
        return;
    }

    CommandeInfo c = mesCommandes[idx];
    QString produit = c.idProduit.toLower();
    int qte = c.quantite;

    // =============================================
    // 1. DETERMINATION DE LA COMPLEXITE PAR PRODUIT
    // =============================================
    struct ProfilProduit {
        QString nom;
        QString complexite;
        double tempsCoupe;       // heures par unite
        double tempsAssemblage;
        double tempsCouture;
        double tempsFinition;
        double tempsTotal;       // par unite
        int capaciteJour;        // unites/jour avec 1 employe
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
        // Produit inconnu - estimation moyenne
        profil = {c.idProduit, "MOYENNE", 0.8, 1.0, 0.8, 0.4, 3.0, 20};
    }

    // =============================================
    // 2. FACTEUR QUANTITE (economies d'echelle)
    // =============================================
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

    // =============================================
    // 3. CALCUL DETAILLE PAR ETAPE
    // =============================================
    double coupeTotale = profil.tempsCoupe * qte * facteurQte;
    double assemblTotale = profil.tempsAssemblage * qte * facteurQte;
    double coutureTotale = profil.tempsCouture * qte * facteurQte;
    double finitionTotale = profil.tempsFinition * qte * facteurQte;
    double tempsTotalH = coupeTotale + assemblTotale + coutureTotale + finitionTotale;

    // Conversion en jours (8h/jour de travail)
    double heuresParJour = 8.0;
    double joursCoupe = std::ceil(coupeTotale / heuresParJour);
    double joursAssembl = std::ceil(assemblTotale / heuresParJour);
    double joursCouture = std::ceil(coutureTotale / heuresParJour);
    double joursFinition = std::ceil(finitionTotale / heuresParJour);
    int joursTotalProduction = static_cast<int>(joursCoupe + joursAssembl + joursCouture + joursFinition);

    // Ajouter marge de securite selon complexite
    double margeSec;
    if (profil.complexite == "TRES ELEVEE") margeSec = 0.20;
    else if (profil.complexite == "ELEVEE") margeSec = 0.15;
    else if (profil.complexite == "MOYENNE") margeSec = 0.10;
    else margeSec = 0.05;

    int joursAvecMarge = static_cast<int>(std::ceil(joursTotalProduction * (1.0 + margeSec)));
    QDate nvFin = c.dateDebut.addDays(joursAvecMarge);


    // Comparaison avec la date prevue
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

    // =============================================
    // 4. AFFICHAGE DETAILLE
    // =============================================
    QString couleurComplexite;
    if (profil.complexite == "TRES ELEVEE") couleurComplexite = "#c62828";
    else if (profil.complexite == "ELEVEE") couleurComplexite = "#ef6c00";
    else if (profil.complexite == "MOYENNE") couleurComplexite = "#f9a825";
    else couleurComplexite = "#2e7d32";

    QString texte = QString(
        "<div style='font-family: Arial; line-height: 1.6;'>"

        // Header
        "<h2 style='color:#6a1b9a; margin-bottom: 5px;'>ANALYSE PREDICTIVE IA</h2>"
        "<hr style='border: 1px solid #ce93d8;'>"

        // Info commande
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

        // Detail par etape
        "<h3 style='color:#4a148c;'>DETAIL PAR ETAPE</h3>"
        "<table style='width:100%%; border-collapse:collapse; margin: 8px 0;'>"
        "<tr style='background:#f3e5f5;'>"
        "<th style='padding:8px; text-align:left; border:1px solid #ce93d8;'>Etape</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Temps/unite</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>x %2 unites</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Avec echelle</th>"
        "<th style='padding:8px; text-align:center; border:1px solid #ce93d8;'>Jours</th>"
        "</tr>"

        // Coupe
        "<tr>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#1565c0; font-weight:bold;'>COUPE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%8 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%9 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%10 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%11 j</td>"
        "</tr>"

        // Assemblage
        "<tr style='background:#fafafa;'>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#ef6c00; font-weight:bold;'>ASSEMBLAGE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%12 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%13 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%14 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%15 j</td>"
        "</tr>"

        // Couture
        "<tr>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#2e7d32; font-weight:bold;'>COUTURE</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%16 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%17 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%18 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%19 j</td>"
        "</tr>"

        // Finition
        "<tr style='background:#fafafa;'>"
        "<td style='padding:6px; border:1px solid #e0e0e0;'><span style='color:#f9a825; font-weight:bold;'>FINITION</span></td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%20 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%21 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0; font-weight:bold;'>%22 h</td>"
        "<td style='padding:6px; text-align:center; border:1px solid #e0e0e0;'>%23 j</td>"
        "</tr>"

        // Total
        "<tr style='background:#6a1b9a; color:white;'>"
        "<td style='padding:8px; border:1px solid #4a148c; font-weight:900;'>TOTAL</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:bold;'>%24 h</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c;'>-</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:900; font-size:14px;'>%25 h</td>"
        "<td style='padding:8px; text-align:center; border:1px solid #4a148c; font-weight:900; font-size:14px;'>%26 j</td>"
        "</tr>"
        "</table>"

        "<hr style='border: 1px dashed #e0e0e0;'>"

        // Resultat final
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

        // Capacite
        "<div style='margin-top:10px; padding:8px; background:#e8eaf6; border-radius:6px; text-align:center;'>"
        "<b>Capacite estimee :</b> %31 unites/jour/employe | "
        "<b>Heures travail/jour :</b> 8h"
        "</div>"

        "</div>"
    )
    .arg(c.idProduit)                                              // %1
    .arg(qte)                                                      // %2
    .arg(c.dateDebut.toString("dd/MM/yyyy"))                       // %3
    .arg(couleurComplexite)                                        // %4
    .arg(profil.complexite)                                        // %5
    .arg(regimeProduction)                                         // %6
    .arg(facteurQte, 0, 'f', 2)                                   // %7
    .arg(profil.tempsCoupe, 0, 'f', 1)                             // %8
    .arg(profil.tempsCoupe * qte, 0, 'f', 1)                      // %9
    .arg(coupeTotale, 0, 'f', 1)                                   // %10
    .arg(joursCoupe, 0, 'f', 0)                                    // %11
    .arg(profil.tempsAssemblage, 0, 'f', 1)                        // %12
    .arg(profil.tempsAssemblage * qte, 0, 'f', 1)                 // %13
    .arg(assemblTotale, 0, 'f', 1)                                 // %14
    .arg(joursAssembl, 0, 'f', 0)                                  // %15
    .arg(profil.tempsCouture, 0, 'f', 1)                           // %16
    .arg(profil.tempsCouture * qte, 0, 'f', 1)                    // %17
    .arg(coutureTotale, 0, 'f', 1)                                 // %18
    .arg(joursCouture, 0, 'f', 0)                                  // %19
    .arg(profil.tempsFinition, 0, 'f', 1)                          // %20
    .arg(profil.tempsFinition * qte, 0, 'f', 1)                   // %21
    .arg(finitionTotale, 0, 'f', 1)                                // %22
    .arg(joursFinition, 0, 'f', 0)                                 // %23
    .arg(profil.tempsTotal, 0, 'f', 1)                             // %24
    .arg(tempsTotalH, 0, 'f', 1)                                   // %25
    .arg(joursTotalProduction)                                      // %26
    .arg(margeSec * 100, 0, 'f', 0)                                // %27
    .arg(joursAvecMarge)                                            // %28
    .arg(nvFin.toString("dd/MM/yyyy"))                              // %29
    .arg(analyseEcart)                                              // %30
    .arg(profil.capaciteJour);                                      // %31

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

    // Widgets requis
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

    // Snapshot initial pour détecter "aucune modification"
    initialNomEmploye = ui->le_emp_nom_modif->text().trimmed();
    initialPrenomEmploye = ui->le_emp_pre_modif->text().trimmed();
    initialEmailEmploye = ui->le_emp_email_modif->text().trimmed();
    initialPosteEmploye = ui->cb_emp_poste_modif->currentText().trimmed();
    initialDepartementEmploye = ui->cb_emp_dept_modif->currentText().trimmed();
    initialDateEmbaucheEmploye = ui->dt_emp_emb_modif->date();
    initialSalaireEmploye = ui->sb_emp_sal_modif->value();
    initialRfidEmploye = ui->le_emp_rfid_modif->text().trimmed();

    // Telephone digits only
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

    // Regex simple et robuste pour email
    static const QRegularExpression emailRe(QStringLiteral("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$"));
    const bool emailOk = emailRe.match(email).hasMatch();

    // Regex "téléphone" : exactement 8 chiffres
    static const QRegularExpression telRe(QStringLiteral("^\\d{8}$"));
    QString telephoneDigits;
    for(const QChar &ch : telephoneRaw) {
        if(ch.isDigit()) telephoneDigits.append(ch);
    }
    const bool telOk = telRe.match(telephoneDigits).hasMatch();

    // Messages uniques (évite double affichage/chaînes multiples)
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

    // Détection "aucune modification"
    QString posteNow = poste;
    QString depNow = departement;
    QString nomNow = nom;
    QString prenomNow = prenom;
    QString emailNow = email;
    QString rfidNow = rfid;
    QDate dateNow = dateEmb;
    double salaireNow = salaire;

    QString telephoneDigitsNow = telephoneDigits; // déjà extrait plus haut (modif)

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
    // 1 clic = activer le tri, puis on toggle A-Z / Z-A
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
        ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre); // colonne 1 = NOM
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

    // ===== Barres : Effectif par département =====
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

    // Mettre camembert + barres côte à côte
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
    l->addStretch(); // Pousse vers le centre

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
        double prime = annees * 50.0; // Exemple : 50 DT par année
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

// =========================================================
// ===        PAGES DYNAMIQUES : ACCUEIL & CONNEXION     ===
// =========================================================

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

    // Spacer haut
    mainL->addStretch(2);

    // CONTENU CENTRAL
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

    // Logo page accueil - grande taille et CENTRÉ
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
    // Forcer le style APRES avoir ajouté tous les onglets
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

    // =============================================
    // ONGLET 1 : SUIVI DES ETAPES
    // =============================================
    QWidget *tabCrud = new QWidget();
    tabCrud->setStyleSheet("background: transparent;");
    QVBoxLayout *crudMainL = new QVBoxLayout(tabCrud);
    crudMainL->setSpacing(8);
    crudMainL->setContentsMargins(16, 12, 16, 8);

    // --- HEADER BANDEAU ---
    // Titre comme les autres modules
    QHBoxLayout *titreL = new QHBoxLayout();
    QLabel *titre = new QLabel("Suivi des Etapes de Fabrication");
    titre->setStyleSheet("font-size: 22px; font-weight: 300; color: #2c1a16;");
    titreL->addWidget(titre);
    titreL->addStretch();
    crudMainL->addLayout(titreL);

    // --- BARRE DE RECHERCHE MODERNE ---
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

    // --- TABLEAU STYLISE ---
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

    // --- BOUTONS CRUD ---
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

    // --- ZONE SUPERVISION GLASSMORPHISM ---
    QFrame *frameSup = new QFrame();
    frameSup->setFixedHeight(85);
    frameSup->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,255,255,0.95), stop:1 rgba(245,240,235,0.95)); "
        "border-radius: 12px; border: 1px solid rgba(141,85,36,0.2); }"
    );
    QHBoxLayout *supL = new QHBoxLayout(frameSup);
    supL->setContentsMargins(14, 6, 14, 6);
    supL->setSpacing(12);

    // Info commande
    QVBoxLayout *infoL = new QVBoxLayout(); infoL->setSpacing(1);
    QLabel *supTitle = new QLabel("Selectionnez une etape...");
    supTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #3e2723; border: none; background: transparent;");
    infoL->addWidget(supTitle);
    QLabel *supDetail = new QLabel("-");
    supDetail->setStyleSheet("color: #8d6e63; border: none; font-size: 10px; background: transparent;");
    infoL->addWidget(supDetail);
    supL->addLayout(infoL, 1);

    // Separateur vertical stylise
    QFrame *vSep = new QFrame();
    vSep->setFrameShape(QFrame::VLine);
    vSep->setStyleSheet("color: rgba(141,85,36,0.2);");
    supL->addWidget(vSep);

    // Formulaire compact
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

    // Bouton Valider futuriste
    QPushButton *btnValider = new QPushButton("VALIDER");
    btnValider->setFixedSize(90, 55);
    btnValider->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8d5524, stop:1 #6d3f1a); "
        "color: white; font-weight: 900; font-size: 13px; border-radius: 10px; border: none; letter-spacing: 1px; } "
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #a0673b, stop:1 #8d5524); } "
        "QPushButton:pressed { background: #5d3a1a; }"
    );
    supL->addWidget(btnValider);

    // Delta indicator
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

    // =============================================
    // ONGLET 2 : TIMELINE GANTT
    // =============================================
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

    // Legende inline
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

    // =============================================
    // ONGLET 3 : STATISTIQUES
    // =============================================
    QWidget *tabStats = new QWidget();
    tabStats->setObjectName("tab_etape_stats");
    QVBoxLayout *stMainL = new QVBoxLayout(tabStats);
    stMainL->setSpacing(14);
    stMainL->setContentsMargins(16, 12, 16, 8);

    tabEtapes->addTab(tabStats, "Statistiques");

    pageL->addWidget(tabEtapes);

    // =============================================
    // FONCTIONS
    // =============================================
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

            // Couleur de l'etape
            QString couleurEtape, bgEtape;
            if (etape == "Coupe")           { couleurEtape = "#1565c0"; bgEtape = "rgba(21,101,192,0.1)"; }
            else if (etape == "Assemblage") { couleurEtape = "#ef6c00"; bgEtape = "rgba(239,108,0,0.1)"; }
            else if (etape == "Couture")    { couleurEtape = "#2e7d32"; bgEtape = "rgba(46,125,50,0.1)"; }
            else if (etape == "Finition")   { couleurEtape = "#f9a825"; bgEtape = "rgba(249,168,37,0.1)"; }
            else { couleurEtape = "#757575"; bgEtape = "rgba(117,117,117,0.1)"; }

            QFont fbold; fbold.setBold(true);
            QFont fnormal; fnormal.setPointSize(10);

            // Col 0 : ID avec badge
            QTableWidgetItem *it0 = new QTableWidgetItem(QString::number(idSuivi));
            it0->setData(Qt::UserRole, idSuivi);
            it0->setData(Qt::UserRole + 1, idPlanif);
            it0->setTextAlignment(Qt::AlignCenter);
            it0->setForeground(QColor("#8d6e63"));
            tbl->setItem(i, 0, it0);

            // Col 1 : CMD en gras
            QTableWidgetItem *it1 = new QTableWidgetItem(QString::number(idPlanif));
            it1->setFont(fbold);
            it1->setTextAlignment(Qt::AlignCenter);
            it1->setForeground(QColor("#3e2723"));
            tbl->setItem(i, 1, it1);

            // Col 2 : Produit
            QTableWidgetItem *itP = new QTableWidgetItem(produit);
            itP->setFont(fbold);
            itP->setForeground(QColor("#4e342e"));
            tbl->setItem(i, 2, itP);

            // Col 3 : Employe
            QTableWidgetItem *it3 = new QTableWidgetItem(employe);
            it3->setForeground(QColor("#6d4c41"));
            tbl->setItem(i, 3, it3);

            // Col 4 : Etape - Badge colore via widget
            QLabel *lblEtape = new QLabel(etape);
            lblEtape->setAlignment(Qt::AlignCenter);
            lblEtape->setStyleSheet(
                QString("background-color: %1; color: white; font-weight: bold; "
                        "font-size: 11px; border-radius: 10px; padding: 4px 12px; "
                        "margin: 3px 6px;").arg(couleurEtape)
            );
            tbl->setCellWidget(i, 4, lblEtape);

            // Col 5 : Temps reel
            QTableWidgetItem *itT = new QTableWidgetItem(QString::number(tempsReel, 'f', 1) + " h");
            itT->setTextAlignment(Qt::AlignCenter);
            itT->setForeground(QColor("#5d4037"));
            itT->setFont(fbold);
            tbl->setItem(i, 5, itT);

            // Col 6 : Delta avec couleur
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

            // Col 7 : Alerte - Pastille
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
        int colJourDebut = 1; // Les jours commencent juste après PRODUIT
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
        // Protection contre crash
        if (row < 0 || row >= tbl->rowCount()) return;
        if (!tbl->item(row, 0)) return;

        selectedEtapeId = tbl->item(row, 0)->data(Qt::UserRole).toInt();
        selectedEtapePlanifId = tbl->item(row, 0)->data(Qt::UserRole + 1).toInt();

        // Recuperer Produit et Employe (items classiques)
        QString prod = tbl->item(row, 2) ? tbl->item(row, 2)->text() : "-";
        QString emp = tbl->item(row, 3) ? tbl->item(row, 3)->text() : "-";

        // Recuperer Etape depuis le widget QLabel (col 4)
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
        // Verifier la connexion Oracle
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) {
            db.open();
            if (!db.isOpen()) {
                alerteErreur("Connexion", "Impossible de se reconnecter a Oracle.\n" + db.lastError().text());
                return;
            }
        }
        // Tester la connexion avec une requete simple
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

        // Calculer delta et alerte
        double tp = 2.0;
        if (et == "Assemblage") tp = 3.0;
        else if (et == "Couture") tp = 4.0;
        else if (et == "Finition") tp = 2.0;
        double d = tr - tp;
        int al = (d > 0) ? 1 : 0;

        // Mettre a jour l'etape avec le temps reel
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

            // Rafraichir le tableau
            refreshCrud();

            // *** FLUX REEL : Verifier si TOUTES les etapes sont terminees ***
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
        // Verifier la connexion Oracle
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

            // *** FLUX REEL : Verifier fin fabrication ***
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

// ============================================================
// FLUX REEL : Verification automatique de fin de fabrication
// ============================================================
void MainWindow::verifierFinFabrication(int idPlanification)
{
    // Verifier connexion Oracle
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { db.open(); }

    // 1. Compter etapes terminees
    QSqlQuery qCount;
    qCount.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id AND TEMPS_REEL_PASSE > 0");
    qCount.bindValue(":id", idPlanification);
    int etapesTerminees = 0;
    if (qCount.exec() && qCount.next()) etapesTerminees = qCount.value(0).toInt();

    // 2. Compter total etapes
    QSqlQuery qTotal;
    qTotal.prepare("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id");
    qTotal.bindValue(":id", idPlanification);
    int totalEtapes = 0;
    if (qTotal.exec() && qTotal.next()) totalEtapes = qTotal.value(0).toInt();

    qDebug() << "Commande" << idPlanification << ":" << etapesTerminees << "/" << totalEtapes;

    // 3. Si pas toutes terminees, on sort
    if (totalEtapes <= 0 || etapesTerminees < totalEtapes) return;

    // === TOUTES LES ETAPES SONT TERMINEES ===

    // 4. Recuperer infos commande
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

    // 5. Statut → Termine
    {
        QSqlQuery q;
        q.prepare("UPDATE PLANIFICATION SET STATUT = 'Termine' WHERE ID_COMMANDE = :id");
        q.bindValue(":id", idPlanification);
        q.exec();
    }

    // 6. Stock produit += quantite
    if (idProduit > 0) {
        QSqlQuery q;
        q.prepare("UPDATE PRODUITS SET REQUIS = NVL(REQUIS, 0) + :qte WHERE ID_PRODUIT = :id");
        q.bindValue(":qte", quantite);
        q.bindValue(":id", idProduit);
        q.exec();
    }

    // 7. Stats production
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

    // 8. COMMIT
    {
        QSqlQuery q;
        q.exec("COMMIT");
    }

    // 9. Alerte
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

    // 10. Rafraichir
    rafraichirListeCommandes();

    qDebug() << "FABRICATION TERMINEE - OF-" << idPlanification << nomProduit << "x" << quantite;
}
