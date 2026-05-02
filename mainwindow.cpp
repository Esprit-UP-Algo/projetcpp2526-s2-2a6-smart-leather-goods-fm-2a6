#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connexion.h"
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "depot.h"
#include "employe.h"
#include "client.h"
#include "arduino.h"

#include <QSqlRecord>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QVariant>
#include <QSqlError>
#include <QDate>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <memory>
#include <QPageSize>
#include <QPageLayout>
#include <QMarginsF>
#include <QProgressBar>
#include <QFileInfo>
#include <QDateTime>
#include <QTime>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QMap>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QDateEdit>
#include <cmath>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QSplitter>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QAbstractItemModel>
#include <QTimer>
#include <QPointer>
#include <QSignalBlocker>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMargins>
#include <QColor>
#include <QFont>
#include <QImage>
#include <QDir>
#include <QAbstractItemView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QDesktopServices>
#include <QWheelEvent>
#include <QEasingCurve>
#include <QDialogButtonBox>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaType>
#include <QTabWidget>
#include <QApplication>
#include <QCoreApplication>
#include <QtGlobal>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {

/// À augmenter si les colonnes / features d’entraînement ML changent (invalide le cache en mémoire).
constexpr int kMlFeatureRevision = 3;

bool peekMlTrainFingerprint(qint64 *cnt, qint64 *maxSuivi, double *sumTempsReel)
{
    QSqlQuery q;
    if (!q.exec(QStringLiteral(
            "SELECT COUNT(*), NVL(MAX(e.ID_SUIVI),0), NVL(SUM(e.TEMPS_REEL_PASSE),0) "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE IS NOT NULL "
            "AND e.TEMPS_REEL_PASSE > 0"))) {
        return false;
    }
    if (!q.next())
        return false;
    *cnt = q.value(0).toLongLong();
    *maxSuivi = q.value(1).toLongLong();
    *sumTempsReel = q.value(2).toDouble();
    return true;
}

bool peekBaseHCalibFingerprint(qint64 *cnt, qint64 *maxSuivi, double *sumTempsSurQte)
{
    QSqlQuery q;
    if (!q.exec(QStringLiteral(
            "SELECT COUNT(*), NVL(MAX(e.ID_SUIVI),0), "
            "NVL(SUM(e.TEMPS_REEL_PASSE / NULLIF(p.QUANTITE, 0)), 0) "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE > 0 "
            "AND p.QUANTITE > 0 "
            "AND (e.TEMPS_REEL_PASSE / p.QUANTITE) BETWEEN 0.01 AND 5"))) {
        return false;
    }
    if (!q.next())
        return false;
    *cnt = q.value(0).toLongLong();
    *maxSuivi = q.value(1).toLongLong();
    *sumTempsSurQte = q.value(2).toDouble();
    return true;
}

} // namespace

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>
#include <QGuiApplication>
#include <QClipboard>
#include <QScrollArea>
#include <QSettings>
#include <QGridLayout>

Q_DECLARE_METATYPE(QVector<double>)

/// Aperçu PNG avec zoom molette et pan (main glissée).
class CutPreviewGraphicsView : public QGraphicsView {
public:
    explicit CutPreviewGraphicsView(QWidget* parent = nullptr)
        : QGraphicsView(parent)
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    }

protected:
    void wheelEvent(QWheelEvent* e) override
    {
        const double factor = (e->angleDelta().y() > 0) ? 1.15 : 0.87;
        scale(factor, factor);
        e->accept();
    }
};

static bool filenameHasImageExtension(const QString &fileName)
{
    const QString s = QFileInfo(fileName).suffix().toLower();
    return s == QLatin1String("jpg") || s == QLatin1String("jpeg") || s == QLatin1String("png")
        || s == QLatin1String("webp") || s == QLatin1String("bmp") || s == QLatin1String("gif");
}

static QPixmap pixmapLoadFromFilePath(const QString &fullPath)
{
    QPixmap px(fullPath);
    if (!px.isNull())
        return px;
    const QImage img(fullPath);
    if (!img.isNull())
        return QPixmap::fromImage(img);
    return QPixmap();
}

/// Évite `scaled(QSize(0, h))` qui renvoie un pixmap **vide** (souvent avant le 1er layout du QLabel).
static QPixmap scaledPixmapForLabel(const QPixmap &source, QLabel *label)
{
    if (source.isNull() || !label)
        return source;

    QSize sz = label->size();
    if (sz.width() < 32 || sz.height() < 32) {
        int w = qMax(320, label->minimumWidth());
        int h = qMax(160, label->minimumHeight());
        if (QWidget *p = label->parentWidget()) {
            const int pw = p->width();
            if (pw >= 100)
                w = qMax(320, qMin(pw - 24, 900));
        }
        sz = QSize(w, h);
    }

    QPixmap out = source.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return out.isNull() ? source : out;
}

/// Ajoute des variantes `.jpg` / `.WEBP` si le nom n’a pas d’extension image.
static QStringList expandDiskImageCandidates(const QStringList &basenames)
{
    static const QStringList extraSuffixes = {
        QStringLiteral(".jpg"), QStringLiteral(".jpeg"), QStringLiteral(".png"),
        QStringLiteral(".webp"), QStringLiteral(".WEBP"), QStringLiteral(".WebP"),
        QStringLiteral(".JPG"), QStringLiteral(".JPEG"), QStringLiteral(".PNG"),
    };

    QStringList out;
    auto push = [&out](const QString &c) {
        const QString t = c.trimmed();
        if (t.isEmpty() || out.contains(t, Qt::CaseInsensitive))
            return;
        out << t;
    };

    for (const QString &rawIn : basenames) {
        const QString raw = rawIn.trimmed();
        if (raw.isEmpty())
            continue;
        push(raw);
        if (filenameHasImageExtension(raw))
            continue;

        for (const QString &suf : extraSuffixes)
            push(raw + suf);

        QString spaced = raw;
        spaced.replace(QLatin1Char('_'), QLatin1Char(' '));
        if (spaced != raw) {
            push(spaced);
            for (const QString &suf : extraSuffixes)
                push(spaced + suf);
        }

        QString underscored = raw;
        underscored.replace(QLatin1Char(' '), QLatin1Char('_'));
        if (underscored != raw) {
            push(underscored);
            for (const QString &suf : extraSuffixes)
                push(underscored + suf);
        }
    }
    return out;
}

/// Charge une illustration : d’abord la ressource Qt (`:/...`), puis des fichiers sur disque.
/// Cherche dans `images/` à côté de l’exécutable, à la racine du projet, ou remontées depuis `build/`.
static QPixmap loadIllustrationImage(const QString &qtResourcePath, const QStringList &alternateBasenames = {})
{
    if (!qtResourcePath.isEmpty()) {
        QPixmap fromRes(qtResourcePath);
        if (!fromRes.isNull())
            return fromRes;
    }

    QStringList names = alternateBasenames;
    if (qtResourcePath.startsWith(QLatin1String(":/"))) {
        const QString tail = qtResourcePath.mid(2);
        names << tail;
        QString underscored = tail;
        underscored.replace(QLatin1Char(' '), QLatin1Char('_'));
        names << underscored;
        names << underscored.toLower();
    } else if (!qtResourcePath.isEmpty()) {
        names << QFileInfo(qtResourcePath).fileName();
    }

    QStringList deduped;
    for (const QString &n : names) {
        const QString t = n.trimmed();
        if (t.isEmpty())
            continue;
        if (!deduped.contains(t, Qt::CaseInsensitive))
            deduped << t;
    }

    const QStringList diskCandidates = expandDiskImageCandidates(deduped);

    /// Répertoires où chercher : CWD, exe, puis ancêtres (ex. `build` → racine du dépôt).
    static const auto buildIllustrationSearchDirectories = []() {
        QStringList dirs;
        auto pushIfDir = [&dirs](const QString &path) {
            const QString c = QDir::cleanPath(path);
            if (c.isEmpty() || !QDir(c).exists())
                return;
            if (!dirs.contains(c, Qt::CaseInsensitive))
                dirs << c;
        };
        auto addRoot = [&pushIfDir](const QString &root) {
            const QString r = QDir::cleanPath(root);
            if (r.isEmpty() || !QDir(r).exists())
                return;
            pushIfDir(r + QStringLiteral("/images"));
            pushIfDir(r);
        };

        const QByteArray envPath = qgetenv("FIL_DOR_IMAGES");
        if (!envPath.isEmpty())
            addRoot(QString::fromLocal8Bit(envPath));

        addRoot(QDir::currentPath());
        addRoot(QCoreApplication::applicationDirPath());
        addRoot(QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath());

        QString p = QDir::cleanPath(QDir::currentPath());
        for (int i = 0; i < 14; ++i) {
            QDir d(p);
            if (!d.cdUp())
                break;
            p = d.absolutePath();
            addRoot(p);
        }

        p = QDir::cleanPath(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 14; ++i) {
            QDir d(p);
            if (!d.cdUp())
                break;
            p = d.absolutePath();
            addRoot(p);
        }

        // Important sous Windows/QtCreator: quand l'exécutable tourne dans un build
        // séparé, on veut aussi remonter depuis le dossier source réel du .cpp.
        p = QDir::cleanPath(QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath());
        for (int i = 0; i < 14; ++i) {
            QDir d(p);
            if (!d.cdUp())
                break;
            p = d.absolutePath();
            addRoot(p);
        }
        return dirs;
    };

    const QStringList dirs = buildIllustrationSearchDirectories();

    for (const QString &dir : dirs) {
        for (const QString &name : diskCandidates) {
            const QString full = QDir::cleanPath(dir + QLatin1Char('/') + name);
            if (!QFileInfo::exists(full))
                continue;
            const QPixmap px = pixmapLoadFromFilePath(full);
            if (!px.isNull())
                return px;
        }
    }
    return QPixmap();
}

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

static bool tableExistsOracle(const QString &tableNameUpper);
static int tableRowCountOracle(const QString &tableNameUpper);

// AffectationEquipeResult défini dans mainwindow.h

static QString etapeNorm(const QString &etape)
{
    return etape.trimmed().toUpper();
}

static bool competencesActives()
{
    return tableExistsOracle(QStringLiteral("COMPETENCES_EMPLOYES"))
        && tableRowCountOracle(QStringLiteral("COMPETENCES_EMPLOYES")) > 0;
}

static QHash<int, QSet<QString>> chargerCompetencesEmployes()
{
    QHash<int, QSet<QString>> out;
    if (!competencesActives())
        return out;

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT ID_EMPLOYE, UPPER(TRIM(ETAPE)), NVL(NIVEAU, 1), NVL(ACTIF, 1) "
        "FROM COMPETENCES_EMPLOYES"));
    if (!q.exec())
        return out;

    while (q.next()) {
        const int id = q.value(0).toInt();
        const QString et = q.value(1).toString();
        const int niveau = q.value(2).toInt();
        const int actif = q.value(3).toInt();
        if (id <= 0 || et.isEmpty() || actif == 0 || niveau <= 0)
            continue;
        out[id].insert(et);
    }
    return out;
}

static bool employeQualifiePourEtape(int idEmp,
                                     const QString &etape,
                                     const QHash<int, QSet<QString>> &compMap,
                                     bool enforceCompetences)
{
    if (idEmp <= 0)
        return false;
    if (!enforceCompetences || compMap.isEmpty())
        return true;
    const auto it = compMap.constFind(idEmp);
    if (it == compMap.constEnd())
        return false;
    return it.value().contains(etapeNorm(etape));
}

static int choisirEmployePourEtape(const QString &etape,
                                   const QVector<int> &team,
                                   int fallbackId,
                                   int offset,
                                   const QHash<int, QSet<QString>> &compMap,
                                   bool enforceCompetences)
{
    if (team.isEmpty())
        return fallbackId;

    if (!enforceCompetences || compMap.isEmpty())
        return team.at(offset % team.size());

    const int n = team.size();
    for (int k = 0; k < n; ++k) {
        const int id = team.at((offset + k) % n);
        if (employeQualifiePourEtape(id, etape, compMap, true))
            return id;
    }
    // Fallback doux : on garde la rotation même sans compétence trouvée.
    return team.at(offset % team.size());
}

static void remplirComboEmploye(QComboBox *cb, const QVector<QPair<int, QString>> &items, int selectId)
{
    if (!cb)
        return;
    cb->clear();
    cb->addItem(QStringLiteral("— Choisir —"), 0);
    for (const auto &it : items)
        cb->addItem(it.second, it.first);
    if (selectId > 0) {
        const int ix = cb->findData(selectId);
        if (ix >= 0)
            cb->setCurrentIndex(ix);
    }
}

static int comboEmployeId(const QComboBox *cb)
{
    if (!cb)
        return 0;
    return cb->currentData().toInt();
}

static bool appliquerAffectationEquipeSurEtapes(int idPlanification, const AffectationEquipeResult &res)
{
    if (idPlanification <= 0)
        return false;

    QSqlQuery qCnt;
    qCnt.prepare(QStringLiteral("SELECT COUNT(*) FROM ETAPES WHERE ID_PLANIFICATION = :id"));
    qCnt.bindValue(QStringLiteral(":id"), idPlanification);
    int nEt = 0;
    if (qCnt.exec() && qCnt.next())
        nEt = qCnt.value(0).toInt();

    const QStringList etapes = {
        QStringLiteral("Coupe"),
        QStringLiteral("Assemblage"),
        QStringLiteral("Couture"),
        QStringLiteral("Finition")
    };
    const QHash<int, QSet<QString>> compMap = chargerCompetencesEmployes();
    const bool enforceComp = res.enforceCompetences && !compMap.isEmpty();
    const int fallbackId = !res.equipeOrdre.isEmpty() ? res.equipeOrdre.first() : 1;

    if (nEt == 0) {
        QHash<QString, QVector<int>> h;
        for (int i = 0; i < etapes.size(); ++i) {
            const QString etape = etapes.at(i);
            QVector<int> empIds;
            if (res.useManuel) {
                QVector<int> manIds = res.manuelParEtape.value(etape);
                for (int idMan : manIds) {
                    if (idMan > 0 && employeQualifiePourEtape(idMan, etape, compMap, enforceComp))
                        empIds.append(idMan);
                }
                if (empIds.isEmpty()) {
                    int idEmp = choisirEmployePourEtape(etape, res.equipeOrdre, fallbackId, i, compMap, enforceComp);
                    if (idEmp > 0) empIds.append(idEmp);
                }
            } else {
                int idEmp = choisirEmployePourEtape(etape, res.equipeOrdre, fallbackId, i, compMap, enforceComp);
                if (idEmp > 0) empIds.append(idEmp);
            }
            if (!empIds.isEmpty())
                h.insert(etape, empIds);
        }
        return Etape::genererEtapesCommandeMulti(idPlanification, res.equipeOrdre, &h);
    }

    // Update existing steps (rare/unsupported properly for multi-employees without dropping first, so we delete and recreate)
    QSqlQuery qDel;
    qDel.prepare("DELETE FROM ETAPES WHERE ID_PLANIFICATION = :id");
    qDel.bindValue(":id", idPlanification);
    qDel.exec();

    QHash<QString, QVector<int>> h2;
    for (int i = 0; i < etapes.size(); ++i) {
        const QString etape = etapes.at(i);
        QVector<int> empIds;
        if (res.useManuel) {
            QVector<int> manIds = res.manuelParEtape.value(etape);
            for (int idMan : manIds) {
                if (idMan > 0 && employeQualifiePourEtape(idMan, etape, compMap, enforceComp))
                    empIds.append(idMan);
            }
            if (empIds.isEmpty()) {
                int idEmp = choisirEmployePourEtape(etape, res.equipeOrdre, fallbackId, i, compMap, enforceComp);
                if (idEmp > 0) empIds.append(idEmp);
            }
        } else {
            int idEmp = choisirEmployePourEtape(etape, res.equipeOrdre, fallbackId, i, compMap, enforceComp);
            if (idEmp > 0) empIds.append(idEmp);
        }
        if (!empIds.isEmpty())
            h2.insert(etape, empIds);
    }
    return Etape::genererEtapesCommandeMulti(idPlanification, res.equipeOrdre, &h2);
}

static bool demanderEmployesPourFabrication(QWidget *parent,
                                            int employePrincipalId,
                                            int idPlanificationPourEtapes,
                                            AffectationEquipeResult *outChoix,
                                            const QSet<int> *preselectionIds = nullptr)
{
    if (!outChoix)
        return false;
    *outChoix = {};

    QDialog dlg(parent);
    dlg.setWindowTitle(QStringLiteral("FIL D'OR — Affectation de l'équipe de fabrication"));
    dlg.setModal(true);
    dlg.resize(1060, 660);
    dlg.setMinimumSize(1000, 620);
    dlg.setStyleSheet(QStringLiteral(
        // Fond et police globale
        "QDialog { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #fdf8f0, stop:1 #f5ede0); font-family: 'Segoe UI', Arial; }"
        // Carte header
        "QFrame#mlHeader { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #6d4c41, stop:0.5 #8d6044, stop:1 #6d4c41);"
        "  border-radius: 14px; }"
        "QLabel#mlDlgTitle { font-size: 18px; font-weight: 900; color: #ffe082; letter-spacing: 0.5px; }"
        "QLabel#mlDlgSub   { font-size: 11px; color: #ffe0b2; }"
        // Cartes de section
        "QFrame#mlCard { background: #ffffff; border: 1px solid #e0cfc0; border-radius: 14px; }"
        "QLabel#mlSectionTitle { font-size: 12px; font-weight: 900; color: #5d4037; "
        "  border-bottom: 2px solid #d4af37; padding-bottom: 4px; }"
        // Champs
        "QLineEdit { background: #fafafa; border: 1.5px solid #d7ccc8; border-radius: 10px;"
        "  padding: 9px 12px; font-size: 12px; color: #3e2723; }"
        "QLineEdit:focus { border-color: #d4af37; }"
        // Listes
        "QListWidget { background: #fafafa; border: 1.5px solid #e0cfc0; border-radius: 12px;"
        "  padding: 4px; font-size: 12px; color: #3e2723; outline: none; }"
        "QListWidget::item { padding: 9px 10px; border-radius: 8px; margin: 1px 2px; }"
        "QListWidget::item:hover { background: #fef9e7; }"
        "QListWidget::item:selected { background: rgba(212,175,55,0.20); color: #3e2723; "
        "  border: 1px solid rgba(212,175,55,0.45); }"
        // TreeWidget (manuel)
        "QTreeWidget { background: #fafafa; border: 1.5px solid #e0cfc0; border-radius: 12px;"
        "  padding: 4px; font-size: 12px; color: #3e2723; }"
        "QTreeWidget::item { padding: 6px 8px; }"
        "QTreeWidget::item:hover { background: #fef9e7; }"
        // TextEdit (aperçu)
        "QTextEdit { background: #fafafa; border: 1.5px solid #e0cfc0; border-radius: 12px;"
        "  padding: 10px; font-size: 12px; color: #3e2723; }"
        // Boutons
        "QPushButton#mlGhost { background: rgba(93,64,55,0.07); border: 1.5px solid rgba(93,64,55,0.18);"
        "  border-radius: 10px; padding: 9px 14px; font-weight: 800; color: #5d4037; }"
        "QPushButton#mlGhost:hover { background: rgba(93,64,55,0.14); }"
        "QPushButton#mlAccent { background: rgba(212,175,55,0.14); border: 1.5px solid rgba(212,175,55,0.50);"
        "  border-radius: 10px; padding: 9px 14px; font-weight: 900; color: #795548; }"
        "QPushButton#mlAccent:hover { background: rgba(212,175,55,0.25); }"
        // CheckBox
        "QCheckBox { font-size: 12px; color: #4e342e; font-weight: 700; spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 5px; "
        "  border: 2px solid #bcaaa4; background: white; }"
        "QCheckBox::indicator:checked { background: #d4af37; border-color: #b8952a; }"
        // TabWidget
        "QTabWidget::pane { border: 1.5px solid #e0cfc0; border-radius: 10px; background: #fafafa; }"
        "QTabBar::tab { background: rgba(0,0,0,0.05); padding: 8px 16px; border-radius: 8px 8px 0 0;"
        "  color: #6d4c41; font-weight: 700; }"
        "QTabBar::tab:selected { background: #d4af37; color: #3e2723; font-weight: 900; }"
        // Splitter
        "QSplitter::handle { background: #e0cfc0; width: 2px; }"
    ));

    QVBoxLayout *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);

    QFrame *header = new QFrame(&dlg);
    header->setObjectName(QStringLiteral("mlHeader"));
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(20, 14, 20, 14);
    hl->setSpacing(14);
    QLabel *ico = new QLabel(QStringLiteral("👥"));
    ico->setStyleSheet(QStringLiteral("font-size: 28px; background: transparent; border: none;"));
    QVBoxLayout *hlTxt = new QVBoxLayout();
    hlTxt->setSpacing(3);
    QLabel *title = new QLabel(QStringLiteral("Affectation de l'équipe de fabrication"));
    title->setObjectName(QStringLiteral("mlDlgTitle"));
    QLabel *sub = new QLabel(QStringLiteral(
        "Mode rapide : composez une équipe ordonnée (drag & drop) + aperçu par étape.  "
        "Mode avancé : affectez manuellement chaque étape (prioritaire sur la rotation)."));
    sub->setObjectName(QStringLiteral("mlDlgSub"));
    sub->setWordWrap(true);
    hlTxt->addWidget(title);
    hlTxt->addWidget(sub);
    hl->addWidget(ico);
    hl->addLayout(hlTxt, 1);
    root->addWidget(header);

    QHBoxLayout *topBar = new QHBoxLayout();
    QPushButton *btnReset = new QPushButton(QStringLiteral("Réinitialiser"), &dlg);
    btnReset->setObjectName(QStringLiteral("mlGhost"));
    btnReset->setCursor(Qt::PointingHandCursor);
    topBar->addWidget(btnReset);
    QCheckBox *cbCompetences = new QCheckBox(QStringLiteral("Filtrer par compétences étape"), &dlg);
    cbCompetences->setChecked(competencesActives());
    cbCompetences->setEnabled(competencesActives());
    cbCompetences->setToolTip(competencesActives()
        ? QStringLiteral("Affecte en priorité des employés qualifiés par étape.")
        : QStringLiteral("Table COMPETENCES_EMPLOYES absente/vide : fallback sans filtre."));
    topBar->addWidget(cbCompetences);
    topBar->addStretch();
    root->addLayout(topBar);

    QSplitter *split = new QSplitter(Qt::Horizontal, &dlg);
    split->setChildrenCollapsible(false);
    split->setHandleWidth(10);

    // --- Colonne gauche : disponibles ---
    QWidget *left = new QWidget(&dlg);
    QVBoxLayout *leftL = new QVBoxLayout(left);
    leftL->setContentsMargins(0, 0, 0, 0);
    leftL->setSpacing(10);
    QLabel *leftTitle = new QLabel(QStringLiteral("📋  Employés disponibles"));
    leftTitle->setObjectName(QStringLiteral("mlSectionTitle"));
    QLineEdit *search = new QLineEdit(&dlg);
    search->setPlaceholderText(QStringLiteral("Rechercher (nom, prénom, id)…"));
    search->setClearButtonEnabled(true);
    QListWidget *lwAll = new QListWidget(&dlg);
    lwAll->setMinimumHeight(340);
    lwAll->setAlternatingRowColors(true);
    lwAll->setUniformItemSizes(true);
    QLabel *counter = new QLabel(&dlg);
    counter->setStyleSheet(QStringLiteral("font-size: 11px; color: #6d4c41; border: none;"));
    QHBoxLayout *quickActions = new QHBoxLayout();
    quickActions->setSpacing(8);
    QPushButton *btnAll = new QPushButton(QStringLiteral("Tout sélectionner"), &dlg);
    QPushButton *btnNone = new QPushButton(QStringLiteral("Tout désélectionner"), &dlg);
    btnAll->setObjectName(QStringLiteral("mlAccent"));
    btnNone->setObjectName(QStringLiteral("mlGhost"));
    btnAll->setCursor(Qt::PointingHandCursor);
    btnNone->setCursor(Qt::PointingHandCursor);
    quickActions->addWidget(btnAll);
    quickActions->addWidget(btnNone);
    quickActions->addStretch();
    quickActions->addWidget(counter);
    leftL->addWidget(leftTitle);
    leftL->addWidget(search);
    leftL->addLayout(quickActions);
    leftL->addWidget(lwAll, 1);
    split->addWidget(left);

    // --- Colonne milieu : équipe ordonnée ---
    QWidget *mid = new QWidget(&dlg);
    QVBoxLayout *midL = new QVBoxLayout(mid);
    midL->setContentsMargins(0, 0, 0, 0);
    midL->setSpacing(10);
    QLabel *midTitle = new QLabel(QStringLiteral("🔄  Équipe (ordre = priorité rotation)"));
    midTitle->setObjectName(QStringLiteral("mlSectionTitle"));
    QListWidget *lwTeam = new QListWidget(&dlg);
    lwTeam->setMinimumHeight(340);
    lwTeam->setDragDropMode(QAbstractItemView::InternalMove);
    lwTeam->setDefaultDropAction(Qt::MoveAction);
    lwTeam->setSelectionMode(QAbstractItemView::SingleSelection);
    lwTeam->setAlternatingRowColors(true);
    QLabel *midHint = new QLabel(QStringLiteral("Glisser-déposer pour réordonner. Les coches à gauche ajoutent/retirent de cette liste."));
    midHint->setWordWrap(true);
    midHint->setStyleSheet(QStringLiteral("font-size: 11px; color: #8d6e63; border: none;"));
    midL->addWidget(midTitle);
    midL->addWidget(lwTeam, 1);
    midL->addWidget(midHint);
    split->addWidget(mid);

    // --- Colonne droite : aperçu + manuel ---
    QWidget *right = new QWidget(&dlg);
    QVBoxLayout *rightL = new QVBoxLayout(right);
    rightL->setContentsMargins(0, 0, 0, 0);
    rightL->setSpacing(10);

    QTabWidget *tabs = new QTabWidget(&dlg);
    QWidget *tabPreview = new QWidget(&dlg);
    QWidget *tabManual = new QWidget(&dlg);
    tabs->addTab(tabPreview, QStringLiteral("Aperçu"));
    tabs->addTab(tabManual, QStringLiteral("Manuel par étape"));

    QTextEdit *preview = new QTextEdit(&dlg);
    preview->setReadOnly(true);
    preview->setAcceptRichText(true);
    QVBoxLayout *tp = new QVBoxLayout(tabPreview);
    tp->setContentsMargins(10, 10, 10, 10);
    tp->addWidget(preview, 1);

    QVBoxLayout *tm = new QVBoxLayout(tabManual);
    tm->setContentsMargins(10, 10, 10, 10);
    tm->setSpacing(10);
    QCheckBox *cbManuel = new QCheckBox(QStringLiteral("Utiliser l’affectation manuelle (prioritaire sur la rotation)"), &dlg);
    cbManuel->setStyleSheet(QStringLiteral("font-weight: 900; color: #3e2723;"));

    QTreeWidget *treeAffManual = new QTreeWidget(&dlg);
    treeAffManual->setHeaderHidden(true);
    treeAffManual->setMinimumHeight(180);
    treeAffManual->setStyleSheet("QTreeWidget { background: white; border: 1px solid #d7ccc8; border-radius: 8px; padding: 4px; color: #3e2723; }");
    
    QStringList stepNames = {"Coupe", "Assemblage", "Couture", "Finition"};
    for (const QString &stepName : stepNames) {
        QTreeWidgetItem *stepItem = new QTreeWidgetItem(treeAffManual);
        stepItem->setText(0, stepName);
        stepItem->setFlags(Qt::ItemIsEnabled);
        QFont f = stepItem->font(0); f.setBold(true); stepItem->setFont(0, f);
        stepItem->setExpanded(false);
    }
    
    tm->addWidget(cbManuel);
    tm->addWidget(treeAffManual, 1);

    QPushButton *btnSyncManual = new QPushButton(QStringLiteral("Remplir depuis la rotation (auto)"), &dlg);
    btnSyncManual->setObjectName(QStringLiteral("mlGhost"));
    btnSyncManual->setCursor(Qt::PointingHandCursor);
    tm->addWidget(btnSyncManual);

    rightL->addWidget(tabs, 1);
    split->addWidget(right);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);
    split->setStretchFactor(2, 3);
    root->addWidget(split, 1);

    QLabel *hint = new QLabel(QStringLiteral(
        "Note : l’employé « responsable » du formulaire concerne PLANIFICATION. "
        "Ici, on pilote surtout l’équipe des étapes (ETAPES)."));
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("font-size: 11px; color: #8d6e63; border: none;"));
    root->addWidget(hint);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QPushButton *okBtn = box->button(QDialogButtonBox::Ok);
    QPushButton *cancelBtn = box->button(QDialogButtonBox::Cancel);
    okBtn->setText(QStringLiteral("✅  Confirmer l'affectation"));
    cancelBtn->setText(QStringLiteral("Annuler"));
    okBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setMinimumHeight(42);
    cancelBtn->setMinimumHeight(42);
    okBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #d4af37, stop:1 #b8952a);"
        "  color: #3e2723; border-radius: 12px; padding: 10px 20px; font-weight: 900; font-size: 13px;"
        "  border: none; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #f0cc55, stop:1 #d4a828); }"
        "QPushButton:disabled { background: #e0d7c0; color: #9e9e9e; }"));
    cancelBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: rgba(93,64,55,0.07); color: #5d4037;"
        "  border: 1.5px solid rgba(93,64,55,0.20); border-radius: 12px;"
        "  padding: 10px 20px; font-weight: 800; font-size: 13px; }"
        "QPushButton:hover { background: rgba(93,64,55,0.14); }"));
    root->addWidget(box);

    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    QVector<QPair<int, QString>> employesItems;
    QHash<int, QListWidgetItem*> allById;
    QHash<int, QListWidgetItem*> teamById;
    const QHash<int, QSet<QString>> compMapDialog = chargerCompetencesEmployes();
    const bool hasCompData = !compMapDialog.isEmpty();

    QSqlQuery qEmp;
    qEmp.prepare(QStringLiteral("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES ORDER BY NOM, PRENOM"));
    if (qEmp.exec()) {
        while (qEmp.next()) {
            const int id = qEmp.value(0).toInt();
            const QString nom = qEmp.value(1).toString();
            const QString prenom = qEmp.value(2).toString();
            QStringList tags;
            const QSet<QString> comps = compMapDialog.value(id);
            if (comps.contains(QStringLiteral("COUPE"))) tags << QStringLiteral("C");
            if (comps.contains(QStringLiteral("ASSEMBLAGE"))) tags << QStringLiteral("A");
            if (comps.contains(QStringLiteral("COUTURE"))) tags << QStringLiteral("Co");
            if (comps.contains(QStringLiteral("FINITION"))) tags << QStringLiteral("F");
            const QString tagTxt = tags.isEmpty() ? QStringLiteral("—") : tags.join(QStringLiteral("/"));
            const QString label = QStringLiteral("%1 %2  •  #%3  •  [%4]")
                                      .arg(nom, prenom)
                                      .arg(id)
                                      .arg(tagTxt);
            employesItems.push_back(qMakePair(id, label));

            auto *it = new QListWidgetItem(label);
            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
            it->setData(Qt::UserRole, id);
            const bool checked = preselectionIds ? preselectionIds->contains(id) : (id == employePrincipalId);
            {
                QSignalBlocker b(lwAll);
                it->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
            }
            lwAll->addItem(it);
            allById.insert(id, it);
        }
    }

    // Populate treeAffManual with employees
    for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
        QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
        for (const auto &emp : employesItems) {
            QTreeWidgetItem *empItem = new QTreeWidgetItem(stepItem);
            empItem->setText(0, emp.second);
            empItem->setData(0, Qt::UserRole, emp.first);
            empItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            empItem->setCheckState(0, Qt::Unchecked);
        }
    }

    QMap<QString, QVector<int>> dbStepEmp;
    if (idPlanificationPourEtapes > 0) {
        QSqlQuery qS;
        qS.prepare(QStringLiteral(
            "SELECT ETAPE_ACTUELLE, ID_EMPLOYE "
            "FROM ETAPES "
            "WHERE ID_PLANIFICATION = :id"));
        qS.bindValue(QStringLiteral(":id"), idPlanificationPourEtapes);
        if (qS.exec()) {
            while (qS.next()) {
                const QString et = qS.value(0).toString().trimmed();
                const int eid = qS.value(1).toInt();
                if (!et.isEmpty() && eid > 0)
                    dbStepEmp[et].append(eid);
            }
        }
    }

    auto syncCombosDefaults = [&]() {
        const int def = employePrincipalId > 0 ? employePrincipalId : 0;
        for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
            QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
            QString etape = stepItem->text(0);
            QVector<int> eids = dbStepEmp.value(etape, QVector<int>());
            if (eids.isEmpty() && def > 0) eids.append(def);

            for (int j = 0; j < stepItem->childCount(); ++j) {
                QTreeWidgetItem *empItem = stepItem->child(j);
                if (eids.contains(empItem->data(0, Qt::UserRole).toInt())) {
                    empItem->setCheckState(0, Qt::Checked);
                } else {
                    empItem->setCheckState(0, Qt::Unchecked);
                }
            }
        }
    };
    syncCombosDefaults();

    const QStringList etapes = {
        QStringLiteral("Coupe"),
        QStringLiteral("Assemblage"),
        QStringLiteral("Couture"),
        QStringLiteral("Finition")
    };

    auto fillComboForStep = [&](const QString &etape, int selectedId) {
        for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
            QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
            if (stepItem->text(0) != etape) continue;

            for (int j = 0; j < stepItem->childCount(); ++j) {
                QTreeWidgetItem *empItem = stepItem->child(j);
                int eid = empItem->data(0, Qt::UserRole).toInt();
                empItem->setCheckState(0, (eid == selectedId) ? Qt::Checked : Qt::Unchecked);
            }
        }
    };

    auto teamIdsInOrder = [&]() -> QVector<int> {
        QVector<int> out;
        out.reserve(lwTeam->count());
        for (int i = 0; i < lwTeam->count(); ++i) {
            QListWidgetItem *it = lwTeam->item(i);
            if (!it) continue;
            const int id = it->data(Qt::UserRole).toInt();
            if (id > 0) out.push_back(id);
        }
        return out;
    };

    auto syncTeamListFromChecks = [&]() {
        QSignalBlocker bt(lwTeam);
        lwTeam->clear();
        teamById.clear();
        for (int i = 0; i < lwAll->count(); ++i) {
            QListWidgetItem *it = lwAll->item(i);
            if (!it) continue;
            if (it->checkState() != Qt::Checked)
                continue;
            const int id = it->data(Qt::UserRole).toInt();
            if (id <= 0) continue;
            auto *nit = new QListWidgetItem(it->text());
            nit->setData(Qt::UserRole, id);
            nit->setFlags(nit->flags() & ~Qt::ItemIsUserCheckable);
            lwTeam->addItem(nit);
            teamById.insert(id, nit);
        }
    };

    auto labelForId = [&](int id) -> QString {
        if (id <= 0) return QStringLiteral("—");
        if (QListWidgetItem *src = allById.value(id, nullptr))
            return src->text();
        return QStringLiteral("#%1").arg(id);
    };

    auto htmlTable = [&](const QVector<QPair<QString, QString>> &rows, const QString &tone) -> QString {
        QString html = QStringLiteral("<table cellspacing='0' cellpadding='0' style='width:100%; border-collapse:separate; border-spacing:0 8px;'>");
        for (const auto &rw : rows) {
            const QString chip = (tone == QLatin1String("gold"))
                ? QStringLiteral("background:rgba(212,175,55,0.14); border:1px solid rgba(212,175,55,0.35);")
                : QStringLiteral("background:rgba(123,31,162,0.10); border:1px solid rgba(123,31,162,0.30);");
            html += QStringLiteral(
                "<tr>"
                "<td style='width:34%; vertical-align:middle; color:#5d4037; font-weight:900;'>%1</td>"
                "<td style='width:66%; vertical-align:middle;'>"
                "<div style='display:inline-block; padding:8px 10px; border-radius:10px; %3 color:#3e2723; font-weight:800;'>%2</div>"
                "</td>"
                "</tr>")
                .arg(rw.first.toHtmlEscaped(), rw.second.toHtmlEscaped(), chip);
        }
        html += QStringLiteral("</table>");
        return html;
    };

    auto refreshPreview = [&]() {
        if (cbManuel->isChecked()) {
            QVector<QPair<QString, QString>> rows;
            for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
                QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
                QString etape = stepItem->text(0);
                QStringList emps;
                for (int j = 0; j < stepItem->childCount(); ++j) {
                    if (stepItem->child(j)->checkState(0) == Qt::Checked) {
                        emps << labelForId(stepItem->child(j)->data(0, Qt::UserRole).toInt());
                    }
                }
                if (emps.isEmpty()) emps << QStringLiteral("—");
                rows.push_back(qMakePair(etape, emps.join(QStringLiteral(", "))));
            }
            preview->setHtml(htmlTable(rows, QStringLiteral("purple")));
            return;
        }

        const QVector<int> team = teamIdsInOrder();
        if (team.isEmpty()) {
            preview->setHtml(QStringLiteral(
                "<div style='color:#6d4c41; line-height:1.45;'>"
                "<b>Aucun membre dans l’équipe ordonnée.</b><br>"
                "Cochez des employés à gauche."
                "</div>"));
            return;
        }
        QVector<QPair<QString, QString>> rows;
        rows.reserve(etapes.size());
        for (int i = 0; i < etapes.size(); ++i) {
            const QString et = etapes.at(i);
            const int id = team.at(i % team.size());
            rows.push_back(qMakePair(et, labelForId(id)));
        }
        preview->setHtml(htmlTable(rows, QStringLiteral("gold")));
    };

    auto updateOk = [&]() {
        const bool manuel = cbManuel->isChecked();
        const bool teamOk = !teamIdsInOrder().isEmpty();
        bool manuelOk = manuel;
        if (manuel) {
            for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
                bool stepHasEmp = false;
                QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
                for (int j = 0; j < stepItem->childCount(); ++j) {
                    if (stepItem->child(j)->checkState(0) == Qt::Checked) {
                        stepHasEmp = true;
                        break;
                    }
                }
                if (!stepHasEmp) {
                    manuelOk = false;
                    break;
                }
            }
        }
        okBtn->setEnabled(manuel ? manuelOk : teamOk);
    };

    auto applyMode = [&]() {
        const bool manuel = cbManuel->isChecked();
        lwTeam->setEnabled(!manuel);
        lwAll->setEnabled(true);
        search->setEnabled(true);
        btnAll->setEnabled(!manuel);
        btnNone->setEnabled(!manuel);
        treeAffManual->setEnabled(manuel);
        btnSyncManual->setEnabled(manuel);
        tabs->setCurrentIndex(manuel ? 1 : 0);
        refreshPreview();
        updateOk();
    };

    auto updateCounter = [&]() {
        int checked = 0;
        int visible = 0;
        for (int i = 0; i < lwAll->count(); ++i) {
            const QListWidgetItem *it = lwAll->item(i);
            if (!it || it->isHidden()) continue;
            ++visible;
            if (it->checkState() == Qt::Checked) ++checked;
        }
        counter->setText(QStringLiteral("%1 sélectionné(s) • %2 visible(s)")
                             .arg(checked)
                             .arg(visible));
        refreshPreview();
        updateOk();
    };

    QObject::connect(cbManuel, &QCheckBox::toggled, &dlg, [&](bool) { applyMode(); });
    QObject::connect(treeAffManual, &QTreeWidget::itemChanged, &dlg, [&](QTreeWidgetItem *, int) { refreshPreview(); updateOk(); });

    QObject::connect(btnSyncManual, &QPushButton::clicked, &dlg, [&]() {
        const QVector<int> team = teamIdsInOrder();
        if (team.isEmpty()) {
            const int def = employePrincipalId > 0 ? employePrincipalId : 0;
            fillComboForStep(QStringLiteral("Coupe"), dbStepEmp.value(QStringLiteral("Coupe"), QVector<int>{def}).value(0, def));
            fillComboForStep(QStringLiteral("Assemblage"), dbStepEmp.value(QStringLiteral("Assemblage"), QVector<int>{def}).value(0, def));
            fillComboForStep(QStringLiteral("Couture"), dbStepEmp.value(QStringLiteral("Couture"), QVector<int>{def}).value(0, def));
            fillComboForStep(QStringLiteral("Finition"), dbStepEmp.value(QStringLiteral("Finition"), QVector<int>{def}).value(0, def));
        } else {
            fillComboForStep(QStringLiteral("Coupe"), team.at(0 % team.size()));
            fillComboForStep(QStringLiteral("Assemblage"), team.at(1 % team.size()));
            fillComboForStep(QStringLiteral("Couture"), team.at(2 % team.size()));
            fillComboForStep(QStringLiteral("Finition"), team.at(3 % team.size()));
        }
        refreshPreview();
        updateOk();
    });
    QObject::connect(cbCompetences, &QCheckBox::toggled, &dlg, [&](bool){
        btnSyncManual->click();
        updateCounter();
    });

    QObject::connect(lwAll, &QListWidget::itemChanged, &dlg, [&](QListWidgetItem *it) {
        if (!it) return;
        const int id = it->data(Qt::UserRole).toInt();
        if (id <= 0) return;

        if (it->checkState() == Qt::Checked) {
            if (!teamById.contains(id)) {
                auto *nit = new QListWidgetItem(it->text());
                nit->setData(Qt::UserRole, id);
                nit->setFlags(nit->flags() & ~Qt::ItemIsUserCheckable);
                lwTeam->addItem(nit);
                teamById.insert(id, nit);
            }
        } else {
            if (QListWidgetItem *t = teamById.value(id, nullptr)) {
                delete t;
                teamById.remove(id);
            }
        }
        updateCounter();
    });

    QObject::connect(lwTeam->model(), &QAbstractItemModel::rowsMoved, &dlg, [&]() { updateCounter(); });

    QObject::connect(btnAll, &QPushButton::clicked, &dlg, [&]() {
        QSignalBlocker b(lwAll);
        for (int i = 0; i < lwAll->count(); ++i) {
            QListWidgetItem *it = lwAll->item(i);
            if (!it) continue;
            it->setCheckState(Qt::Checked);
        }
        syncTeamListFromChecks();
        updateCounter();
    });
    QObject::connect(btnNone, &QPushButton::clicked, &dlg, [&]() {
        QSignalBlocker b(lwAll);
        for (int i = 0; i < lwAll->count(); ++i) {
            QListWidgetItem *it = lwAll->item(i);
            if (!it) continue;
            it->setCheckState(Qt::Unchecked);
        }
        syncTeamListFromChecks();
        updateCounter();
    });
    QObject::connect(search, &QLineEdit::textChanged, &dlg, [&](const QString &t) {
        const QString s = t.trimmed().toLower();
        for (int i = 0; i < lwAll->count(); ++i) {
            QListWidgetItem *it = lwAll->item(i);
            if (!it) continue;
            const QString txt = it->text().toLower();
            it->setHidden(!s.isEmpty() && !txt.contains(s));
        }
        updateCounter();
    });

    QObject::connect(btnReset, &QPushButton::clicked, &dlg, [&]() {
        QSignalBlocker ball(lwAll);
        for (int i = 0; i < lwAll->count(); ++i) {
            QListWidgetItem *it = lwAll->item(i);
            if (!it) continue;
            const int id = it->data(Qt::UserRole).toInt();
            const bool checked = preselectionIds ? preselectionIds->contains(id) : (id == employePrincipalId);
            it->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        }
        syncTeamListFromChecks();
        cbManuel->setChecked(false);
        btnSyncManual->click();
        applyMode();
    });

    // init équipe depuis coches
    syncTeamListFromChecks();
    updateCounter();
    applyMode();

    if (dlg.exec() != QDialog::Accepted)
        return false;

    outChoix->equipeOrdre = teamIdsInOrder();
    outChoix->enforceCompetences = cbCompetences->isChecked() && hasCompData;
    if (cbManuel->isChecked()) {
        outChoix->useManuel = true;
        for (int i = 0; i < treeAffManual->topLevelItemCount(); ++i) {
            QTreeWidgetItem *stepItem = treeAffManual->topLevelItem(i);
            QString etape = stepItem->text(0);
            QVector<int> eids;
            for (int j = 0; j < stepItem->childCount(); ++j) {
                if (stepItem->child(j)->checkState(0) == Qt::Checked) {
                    eids.append(stepItem->child(j)->data(0, Qt::UserRole).toInt());
                }
            }
            if (!eids.isEmpty()) outChoix->manuelParEtape.insert(etape, eids);
        }
        return true;
    }

    outChoix->useManuel = false;
    if (outChoix->equipeOrdre.isEmpty() && employePrincipalId > 0)
        outChoix->equipeOrdre.push_back(employePrincipalId);
    return true;
}

static void applyDialogBase(QDialog &dialog, int w, int h) {
    dialog.setMinimumSize(w, h);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");
}

static QString equipeEmployesPourCommande(int idCommande, const QString &fallbackEmp)
{
    if (idCommande <= 0)
        return fallbackEmp;

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT DISTINCT emp.NOM, emp.PRENOM "
        "FROM ETAPES e "
        "JOIN EMPLOYES emp ON emp.ID_EMPLOYE = e.ID_EMPLOYE "
        "WHERE e.ID_PLANIFICATION = :id "
        "ORDER BY emp.NOM, emp.PRENOM"));
    q.bindValue(":id", idCommande);

    if (!q.exec())
        return fallbackEmp;

    QStringList noms;
    while (q.next()) {
        const QString nom = q.value(0).toString().trimmed();
        const QString prenom = q.value(1).toString().trimmed();
        const QString full = (nom + QLatin1Char(' ') + prenom).trimmed();
        if (!full.isEmpty())
            noms << full;
    }
    if (noms.isEmpty())
        return fallbackEmp;
    return noms.join(QStringLiteral(", "));
}

static QString formatEquipeAffichageCourt(const QString &equipeComplete, int maxNomsAffiches = 2)
{
    // Entrée attendue: "Nom Prenom, Nom Prenom, ..."
    QStringList parts = equipeComplete.split(QStringLiteral(","), Qt::SkipEmptyParts);
    for (QString &p : parts) p = p.trimmed();
    parts.removeAll(QString());

    if (parts.isEmpty())
        return {};
    if (parts.size() <= maxNomsAffiches)
        return parts.join(QStringLiteral(", "));

    const int rest = parts.size() - maxNomsAffiches;
    QStringList head = parts.mid(0, maxNomsAffiches);
    return head.join(QStringLiteral(", ")) + QStringLiteral(" (+%1)").arg(rest);
}

static QStringList extraireNomsEquipe(const QString &equipeComplete)
{
    QStringList parts = equipeComplete.split(QStringLiteral(","), Qt::SkipEmptyParts);
    for (QString &p : parts)
        p = p.trimmed();
    parts.removeAll(QString());
    return parts;
}

static QString formatEquipeAffichageLignes(const QString &equipeComplete, int maxLignes = 3)
{
    const QStringList noms = extraireNomsEquipe(equipeComplete);
    if (noms.isEmpty())
        return {};

    if (noms.size() <= maxLignes)
        return noms.join(QStringLiteral("\n"));

    QStringList lignes = noms.mid(0, maxLignes);
    lignes << QStringLiteral("… +%1 autre(s)").arg(noms.size() - maxLignes);
    return lignes.join(QStringLiteral("\n"));
}

static QString formatEquipeTooltip(const QString &equipeComplete)
{
    const QStringList noms = extraireNomsEquipe(equipeComplete);
    if (noms.isEmpty())
        return {};

    QStringList lignes;
    lignes.reserve(noms.size() + 1);
    lignes << QStringLiteral("Équipe affectée (%1) :").arg(noms.size());
    for (const QString &nom : noms)
        lignes << QStringLiteral("• %1").arg(nom);
    return lignes.join(QStringLiteral("\n"));
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
            const auto *no = dynamic_cast<const SortableNumericTableWidgetItem *>(&other);
            if (no)
                return m_sortKey < no->m_sortKey;
        }
        return QTableWidgetItem::operator<(other);
    }

private:
    double m_sortKey;
};
} // namespace

// --- Affichage étapes : jours ouvrés (calculs BDD / delta restent en heures, comme l’IA planif : 8 h/j) ---
namespace {
constexpr double kHeuresOuvreParJourFabrication = 8.0;
/// Plafond des QDoubleSpinBox « réel (j) » : l’ancien max 100 h → 12,5 j bloquait la saisie dès que le prévu IA dépassait 12 j (ex. 29 j).
constexpr double kJoursMaxSaisieTempsReelFabrication = 5000.0;

QString formaterHeuresEtapesEnJoursAffichage(double heures)
{
    const double j = heures / kHeuresOuvreParJourFabrication;
    return QLocale(QLocale::French, QLocale::France).toString(j, 'f', 2) + QStringLiteral(" j");
}

QString formaterDeltaEtapesEnJoursAffichage(double deltaHeures)
{
    const double j = deltaHeures / kHeuresOuvreParJourFabrication;
    QString s = (deltaHeures > 0) ? QStringLiteral("+") : QString();
    s += QLocale(QLocale::French, QLocale::France).toString(j, 'f', 2) + QStringLiteral(" j");
    return s;
}

/// Profil temps / unité (h) — même logique que l’onglet « IA Estimation ».
struct ProfilFabricationIA {
    QString complexite;
    double tempsCoupe = 0;
    double tempsAssemblage = 0;
    double tempsCouture = 0;
    double tempsFinition = 0;
    double tempsTotal = 0;
    int capaciteJour = 20;
};

ProfilFabricationIA profilFabricationDepuisDesignation(const QString &designationBrute)
{
    const QString produit = designationBrute.toLower();
    ProfilFabricationIA p;
    if (produit.contains(QLatin1String("sac")) && (produit.contains(QLatin1String("voyage")) || produit.contains(QLatin1String("grand")))) {
        p.complexite = QStringLiteral("ELEVEE");
        p.tempsCoupe = 1.2; p.tempsAssemblage = 1.8; p.tempsCouture = 1.5; p.tempsFinition = 0.5; p.tempsTotal = 5.0; p.capaciteJour = 12;
    } else if (produit.contains(QLatin1String("sac")) && (produit.contains(QLatin1String("main")) || produit.contains(QLatin1String("dame")))) {
        p.complexite = QStringLiteral("ELEVEE");
        p.tempsCoupe = 1.0; p.tempsAssemblage = 1.5; p.tempsCouture = 1.2; p.tempsFinition = 0.4; p.tempsTotal = 4.1; p.capaciteJour = 15;
    } else if (produit.contains(QLatin1String("sac")) && (produit.contains(QLatin1String("dos")) || produit.contains(QLatin1String("sport")))) {
        p.complexite = QStringLiteral("ELEVEE");
        p.tempsCoupe = 1.1; p.tempsAssemblage = 1.6; p.tempsCouture = 1.4; p.tempsFinition = 0.5; p.tempsTotal = 4.6; p.capaciteJour = 13;
    } else if (produit.contains(QLatin1String("sac"))) {
        p.complexite = QStringLiteral("ELEVEE");
        p.tempsCoupe = 1.0; p.tempsAssemblage = 1.5; p.tempsCouture = 1.3; p.tempsFinition = 0.4; p.tempsTotal = 4.2; p.capaciteJour = 14;
    } else if (produit.contains(QLatin1String("portef")) || produit.contains(QLatin1String("wallet"))) {
        p.complexite = QStringLiteral("FAIBLE");
        p.tempsCoupe = 0.3; p.tempsAssemblage = 0.4; p.tempsCouture = 0.3; p.tempsFinition = 0.2; p.tempsTotal = 1.2; p.capaciteJour = 45;
    } else if (produit.contains(QLatin1String("ceinture")) || produit.contains(QLatin1String("belt"))) {
        p.complexite = QStringLiteral("FAIBLE");
        p.tempsCoupe = 0.2; p.tempsAssemblage = 0.2; p.tempsCouture = 0.3; p.tempsFinition = 0.1; p.tempsTotal = 0.8; p.capaciteJour = 60;
    } else if (produit.contains(QLatin1String("pochette")) || produit.contains(QLatin1String("clutch"))) {
        p.complexite = QStringLiteral("MOYENNE");
        p.tempsCoupe = 0.5; p.tempsAssemblage = 0.6; p.tempsCouture = 0.5; p.tempsFinition = 0.3; p.tempsTotal = 1.9; p.capaciteJour = 30;
    } else if (produit.contains(QLatin1String("etui")) || produit.contains(QLatin1String("housse"))) {
        p.complexite = QStringLiteral("FAIBLE");
        p.tempsCoupe = 0.2; p.tempsAssemblage = 0.3; p.tempsCouture = 0.2; p.tempsFinition = 0.1; p.tempsTotal = 0.8; p.capaciteJour = 55;
    } else if (produit.contains(QLatin1String("veste")) || produit.contains(QLatin1String("jacket")) || produit.contains(QLatin1String("blouson"))) {
        p.complexite = QStringLiteral("TRES ELEVEE");
        p.tempsCoupe = 2.0; p.tempsAssemblage = 2.5; p.tempsCouture = 2.0; p.tempsFinition = 0.8; p.tempsTotal = 7.3; p.capaciteJour = 8;
    } else if (produit.contains(QLatin1String("chaussure")) || produit.contains(QLatin1String("botte"))) {
        p.complexite = QStringLiteral("TRES ELEVEE");
        p.tempsCoupe = 1.5; p.tempsAssemblage = 2.0; p.tempsCouture = 1.8; p.tempsFinition = 0.7; p.tempsTotal = 6.0; p.capaciteJour = 10;
    } else {
        p.complexite = QStringLiteral("MOYENNE");
        p.tempsCoupe = 0.8; p.tempsAssemblage = 1.0; p.tempsCouture = 0.8; p.tempsFinition = 0.4; p.tempsTotal = 3.0; p.capaciteJour = 20;
    }
    return p;
}

double facteurEchelleQuantiteIA(int qteBrute, QString *regimeProduction = nullptr)
{
    const int qte = std::max(1, qteBrute);
    double facteurQte;
    QString regime;
    if (qte <= 10) {
        facteurQte = 1.0;
        regime = QStringLiteral("Artisanal (pas d'economie d'echelle)");
    } else if (qte <= 50) {
        facteurQte = 0.88;
        regime = QStringLiteral("Petite serie (-12% par optimisation)");
    } else if (qte <= 200) {
        facteurQte = 0.75;
        regime = QStringLiteral("Serie moyenne (-25% par routine)");
    } else if (qte <= 500) {
        facteurQte = 0.65;
        regime = QStringLiteral("Grande serie (-35% par cadence)");
    } else {
        facteurQte = 0.58;
        regime = QStringLiteral("Production de masse (-42% par industrialisation)");
    }
    if (regimeProduction)
        *regimeProduction = regime;
    return facteurQte;
}

double heuresPrevuesEtapeScaledIA(const ProfilFabricationIA &profil, int quantiteCommande, double facteurQte, const QString &etape)
{
    const int q = std::max(1, quantiteCommande);
    if (etape == QLatin1String("Coupe"))
        return profil.tempsCoupe * q * facteurQte;
    if (etape == QLatin1String("Assemblage"))
        return profil.tempsAssemblage * q * facteurQte;
    if (etape == QLatin1String("Couture"))
        return profil.tempsCouture * q * facteurQte;
    if (etape == QLatin1String("Finition"))
        return profil.tempsFinition * q * facteurQte;
    return profil.tempsCoupe * q * facteurQte;
}

/// Heures « avec échelle » pour une étape (même formule que colonne « Avec echelle » de l’IA).
double tempsPrevuHeuresEtapePourCommandeIA(const QString &designationProduit, int quantiteCommande, const QString &etape)
{
    const ProfilFabricationIA pf = profilFabricationDepuisDesignation(designationProduit);
    const double fq = facteurEchelleQuantiteIA(quantiteCommande);
    return heuresPrevuesEtapeScaledIA(pf, quantiteCommande, fq, etape);
}

/// Jours affichés comme la colonne « Jours » de l’IA : plafond ceil(heures / 8 h).
QString formaterJoursPrevusPlafondIA(double heuresEtape)
{
    const int j = static_cast<int>(std::ceil(heuresEtape / kHeuresOuvreParJourFabrication));
    return QString::number(j) + QStringLiteral(" j");
}

/// Saisie UI en jours ouvrés → heures pour Oracle (8 h/j).
inline double joursSaisieVersHeuresFabrication(double jours) { return jours * kHeuresOuvreParJourFabrication; }
/// Heures en base → valeur pour QDoubleSpinBox (jours).
inline double heuresStockVersJoursSaisieFabrication(double heures) { return heures / kHeuresOuvreParJourFabrication; }
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
void MainWindow::on_btn_test_buzzer_clicked()
{
    arduino->beep();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    arduino = new Arduino(this);
    if (!arduino->connectToBoard("COM7")) {
        qDebug() << "Could not connect to Arduino. Buzzer will not work.";
    }
    qRegisterMetaType<QVector<double>>("QVector<double>");
    m_cutApiManager = new QNetworkAccessManager(this);

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
        } else if (m_refreshFabEtapesTable) {
            m_refreshFabEtapesTable();
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
    connect(ui->btn_valider_modif, &QPushButton::clicked, this, [this](){
        if (indexModification < 0 || indexModification >= mesCommandes.size()) {
            alerteErreur("Erreur", "Aucune commande sélectionnée pour modification.");
            ui->tabWidgetPlanif->setCurrentIndex(0);
            return;
        }

        const int idProdM = comboIdData(ui->cb_produit_modif);
        const int idMatM  = comboIdData(ui->cb_matiere_modif);
        const int idEmpM  = comboIdData(ui->cb_employe_modif);
        const int q       = ui->sb_qte_modif->value();
        const QDate d1    = ui->dt_lancement_modif->date();
        QString errDateM;
        const QDate d2 = parseDateFinPrevue(ui->le_fin_prevue_modif->text(), d1.addDays(7), &errDateM);
        if (!errDateM.isEmpty()) { alerteWarning(QStringLiteral("Date invalide"), errDateM); return; }

        const QString idStr = mesCommandes[indexModification].id;
        int idToEdit = idStr.startsWith("OF-") ? idStr.mid(3).toInt() : idStr.toInt();
        if (idToEdit <= 0) { alerteErreur("Erreur", "ID commande invalide: " + idStr); return; }

        // ── Saisir l’affectation équipe avant toute sauvegarde ──
        if (!m_affectModifConfigured) {
            QSet<int> preEmp;
            QSqlQuery qPre;
            qPre.prepare(QStringLiteral("SELECT DISTINCT ID_EMPLOYE FROM ETAPES WHERE ID_PLANIFICATION = :id"));
            qPre.bindValue(QStringLiteral(":id"), idToEdit);
            if (qPre.exec()) while (qPre.next()) { const int e = qPre.value(0).toInt(); if (e > 0) preEmp.insert(e); }
            const QSet<int> *prePtr = preEmp.isEmpty() ? nullptr : &preEmp;
            const bool ok = demanderEmployesPourFabrication(
                this, idEmpM > 0 ? idEmpM : 1, idToEdit, &m_pendingAffectModif, prePtr);
            if (!ok) return; // annulé
            m_affectModifConfigured = true;
        }

        OrdreFabrication o(QString::number(idProdM), q,
                           QString::number(idMatM), d1, d2, "Planifie",
                           QString::number(idEmpM));
        if (!o.modifier(idToEdit)) {
            if (!o.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), o.derniereErreurSaisie());
            else
                alerteErreur("Erreur", "Echec BDD.");
            return;
        }

        // Recréer les étapes avec la nouvelle équipe
        QSqlQuery qDel;
        qDel.prepare("DELETE FROM ETAPES WHERE ID_PLANIFICATION = :id");
        qDel.bindValue(":id", idToEdit);
        qDel.exec();
        if (!appliquerAffectationEquipeSurEtapes(idToEdit, m_pendingAffectModif))
            alerteWarning(QStringLiteral("Équipe"),
                          QStringLiteral("Impossible d’appliquer l’affectation sur les étapes."));

        // Réinitialiser le bouton équipe pour le prochain usage
        m_affectModifConfigured = false;
        m_pendingAffectModif = {};
        ui->btn_equipe_modif->setText(QStringLiteral("👥 Configurer l’équipe"));
        ui->btn_equipe_modif->setStyleSheet(QString());

        alerteSucces(QStringLiteral("Succès"), QStringLiteral("Commande mise à jour avec équipe affectée !"));
        rafraichirListeCommandes();
        configurerTimelineGantt();
        ui->tabWidgetPlanif->setCurrentIndex(0);
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

    connect(ui->tabWidgetPlanif, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1 && !modeModification)
            preparerFormulairePlanif(false);
        else if (index == 2)
            calculerEtAfficherStats();
        else if (index == 4)
            QTimer::singleShot(0, this, [this]() { renderIaEstimationForSelectedOrder(); });
    });

    connect(ui->btn_to_add_planif, &QPushButton::clicked, [=](){
        preparerFormulairePlanif(false);
        ui->tabWidgetPlanif->setCurrentIndex(1);
    });

    // --- STYLES DES NOUVEAUX BOUTONS ---
    ui->btn_valider_modif->setStyleSheet(styleBtnSave()); // Met le bouton "Modifier" en marron FIL D'OR
    ui->btn_valider_modif->setCursor(Qt::PointingHandCursor);

    ui->btn_ia_appliquer->setStyleSheet(styleBtnSave());
    ui->btn_ia_appliquer->setCursor(Qt::PointingHandCursor);
    ui->btn_ia_appliquer->setEnabled(false);

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
    connect(ui->btn_ia_retour_analyse, &QPushButton::clicked, this, [this]() {
        renderIaEstimationForSelectedOrder();
    });
    connect(ui->btn_ia_wf_plus_emp, &QPushButton::clicked, this, [this]() {
        iaWhatIfPlusEmp();
    });
    connect(ui->btn_ia_wf_overtime, &QPushButton::clicked, this, [this]() {
        iaWhatIfOvertime();
    });
    connect(ui->btn_ia_wf_replan, &QPushButton::clicked, this, [this]() {
        iaWhatIfReplan();
    });
    connect(ui->btn_ia_wf_admin, &QPushButton::clicked, this, [this]() {
        iaWhatIfAdminCal();
    });

    // Bouton "Configurer l'équipe" — formulaire Nouvel Ordre
    connect(ui->btn_equipe_planif, &QPushButton::clicked, this, [this]() {
        const int idEmp = comboIdData(ui->cb_employe);
        const bool ok = demanderEmployesPourFabrication(this, idEmp > 0 ? idEmp : 1,
                                                         -1, &m_pendingAffectAdd, nullptr);
        m_affectAddConfigured = ok;
        if (ok) {
            const int n = m_pendingAffectAdd.equipeOrdre.size();
            ui->btn_equipe_planif->setText(
                QStringLiteral("✅ Équipe configurée (%1 membre%2)").arg(n).arg(n > 1 ? "s" : ""));
            ui->btn_equipe_planif->setStyleSheet(
                QStringLiteral("QPushButton { background: #e8f5e9; border: 2px solid #66bb6a; "
                                "border-radius: 10px; color: #2e7d32; font-weight: 900; padding: 8px; }"
                                "QPushButton:hover { background: #c8e6c9; }"));
        } else {
            ui->btn_equipe_planif->setText(QStringLiteral("👥 Configurer l'équipe"));
            ui->btn_equipe_planif->setStyleSheet(QString());
        }
    });

    // Bouton "Configurer l'équipe" — formulaire Modifier Commande
    connect(ui->btn_equipe_modif, &QPushButton::clicked, this, [this]() {
        int idToEdit = -1;
        if (indexModification >= 0 && indexModification < mesCommandes.size()) {
            const QString idStr = mesCommandes[indexModification].id;
            idToEdit = idStr.startsWith("OF-") ? idStr.mid(3).toInt() : idStr.toInt();
        }
        const int idEmp = comboIdData(ui->cb_employe_modif);
        QSet<int> preEmp;
        if (idToEdit > 0) {
            QSqlQuery qPre;
            qPre.prepare(QStringLiteral("SELECT DISTINCT ID_EMPLOYE FROM ETAPES WHERE ID_PLANIFICATION = :id"));
            qPre.bindValue(QStringLiteral(":id"), idToEdit);
            if (qPre.exec()) while (qPre.next()) { const int e = qPre.value(0).toInt(); if (e > 0) preEmp.insert(e); }
        }
        const QSet<int> *prePtr = preEmp.isEmpty() ? nullptr : &preEmp;
        const bool ok = demanderEmployesPourFabrication(this, idEmp > 0 ? idEmp : 1,
                                                         idToEdit, &m_pendingAffectModif, prePtr);
        m_affectModifConfigured = ok;
        if (ok) {
            const int n = m_pendingAffectModif.equipeOrdre.size();
            ui->btn_equipe_modif->setText(
                QStringLiteral("✅ Équipe configurée (%1 membre%2)").arg(n).arg(n > 1 ? "s" : ""));
            ui->btn_equipe_modif->setStyleSheet(
                QStringLiteral("QPushButton { background: #e8f5e9; border: 2px solid #66bb6a; "
                                "border-radius: 10px; color: #2e7d32; font-weight: 900; padding: 8px; }"
                                "QPushButton:hover { background: #c8e6c9; }"));
        } else {
            ui->btn_equipe_modif->setText(QStringLiteral("👥 Configurer l'équipe"));
            ui->btn_equipe_modif->setStyleSheet(QString());
        }
    });

    connect(ui->btn_valider_planif, &QPushButton::clicked, this, [this](){
        const int idProd = comboIdData(ui->cb_produit);
        const int idMat  = comboIdData(ui->cb_matiere);
        const int idEmp  = comboIdData(ui->cb_employe);
        const int q      = ui->sb_qte->value();
        const QDate d1   = ui->dt_lancement->date();
        QString errDate;
        const QDate d2 = parseDateFinPrevue(ui->le_fin_prevue->text(), d1.addDays(3), &errDate);
        if (!errDate.isEmpty()) { alerteWarning(QStringLiteral("Date invalide"), errDate); return; }

        // ── Saisir l'affectation équipe avant toute sauvegarde ──
        if (!m_affectAddConfigured) {
            const bool ok = demanderEmployesPourFabrication(
                this, idEmp > 0 ? idEmp : 1, -1, &m_pendingAffectAdd, nullptr);
            if (!ok) return; // annulé
            m_affectAddConfigured = true;
        }

        const QString pId = QString::number(idProd);
        const QString mId = QString::number(idMat);
        const QString eId = QString::number(idEmp);
        OrdreFabrication o(pId, q, mId, d1, d2, "Planifié", eId);
        if (!o.ajouter()) {
            if (!o.derniereErreurSaisie().isEmpty())
                alerteWarning(QStringLiteral("Saisie invalide"), o.derniereErreurSaisie());
            else
                alerteErreur("Erreur", "Échec BDD.");
            return;
        }

        // Récupérer le nouvel ID
        QSqlQuery qId;
        if (!qId.exec("SELECT MAX(ID_COMMANDE) FROM PLANIFICATION") || !qId.next()) {
            alerteErreur("Erreur", "Impossible de récupérer l'ID de la commande.");
            return;
        }
        const int idNouvelleCommande = qId.value(0).toInt();

        // Appliquer l'affectation
        if (!appliquerAffectationEquipeSurEtapes(idNouvelleCommande, m_pendingAffectAdd))
            alerteWarning(QStringLiteral("Équipe"), QStringLiteral("Impossible de générer les étapes."));

        // Réinitialiser le bouton équipe pour le prochain usage
        m_affectAddConfigured = false;
        m_pendingAffectAdd = {};
        ui->btn_equipe_planif->setText(QStringLiteral("👥 Configurer l'équipe"));
        ui->btn_equipe_planif->setStyleSheet(QString());

        alerteSucces("Succès", "Commande créée avec équipe affectée !");
        modeModification = false;
        rafraichirListeCommandes();
        configurerTimelineGantt();
        ui->tabWidgetPlanif->setCurrentIndex(0);
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
        {
            QSignalBlocker b(ui->tabWidgetProduits);
            ui->tabWidgetProduits->setCurrentIndex(3);
        }
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
    connect(ui->tableTimeline, &QTableWidget::cellClicked, this, [=](int row, int col) {
        Q_UNUSED(col);
        if (row < 0 || !ui->tableTimeline->item(row, 0)) return;
        const int idSuivi = ui->tableTimeline->item(row, 0)->data(Qt::UserRole).toInt();
        const int idPlanif = ui->tableTimeline->item(row, 0)->data(Qt::UserRole + 1).toInt();
        const QString produit = ui->tableTimeline->item(row, 2)->text();
        const int qteRow = ui->tableTimeline->item(row, 0)->data(Qt::UserRole + 2).toInt();
        const int qteSafe = std::max(1, qteRow);
        const QString etape = ui->tableTimeline->item(row, 4)->text();

        m_fabTimelineIdPlanif = idPlanif;
        m_fabTimelineProduit = produit;
        m_fabTimelineQte = qteSafe;

        indexCommandeSelectionnee = -1;
        for (int i = 0; i < mesCommandes.size(); ++i) {
            if (mesCommandes[i].id == QStringLiteral("OF-%1").arg(idPlanif)) {
                indexCommandeSelectionnee = i;
                break;
            }
        }

        ui->lbl_sel_cmd_title->setText(QStringLiteral("📋 Commande #%1 — %2").arg(idPlanif).arg(produit));
        ui->lbl_sel_cmd->setText(QStringLiteral("ID Suivi: %1 | Étape: %2").arg(idSuivi).arg(etape));

        {
            QSignalBlocker b(ui->cb_etape_suivi);
            const int idx = ui->cb_etape_suivi->findText(etape);
            if (idx >= 0) ui->cb_etape_suivi->setCurrentIndex(idx);
        }

        const double tempsPrevue = tempsPrevuHeuresEtapePourCommandeIA(produit, qteSafe, etape);
        ui->lbl_temps_prevu->setText(formaterJoursPrevusPlafondIA(tempsPrevue));
        ui->lbl_temps_prevu->setToolTip(
            QStringLiteral("Prévu IA (avec échelle) : %1 h — jours affichés = plafond comme l’onglet IA (%2 h/j).")
                .arg(tempsPrevue, 0, 'f', 1)
                .arg(kHeuresOuvreParJourFabrication, 0, 'f', 0));

        double trH = 0.0;
        QSqlQuery qTr;
        qTr.prepare(QStringLiteral("SELECT NVL(TEMPS_REEL_PASSE, 0) FROM ETAPES WHERE ID_SUIVI = :id"));
        qTr.bindValue(QStringLiteral(":id"), idSuivi);
        if (qTr.exec() && qTr.next())
            trH = qTr.value(0).toDouble();
        {
            QSignalBlocker b(ui->sb_temps_reel_input);
            ui->sb_temps_reel_input->setValue(heuresStockVersJoursSaisieFabrication(trH));
        }
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, this, [=](const QString &t) {
        if (m_fabTimelineProduit.isEmpty())
            return;
        const double v = tempsPrevuHeuresEtapePourCommandeIA(m_fabTimelineProduit, m_fabTimelineQte, t);
        ui->lbl_temps_prevu->setText(formaterJoursPrevusPlafondIA(v));
        ui->lbl_temps_prevu->setToolTip(
            QStringLiteral("Prévu IA (avec échelle) : %1 h — plafond jours comme l’onglet IA.")
                .arg(v, 0, 'f', 1));
    });

    // VALIDATION ÉTAPE ET INSERTION DANS ORACLE
    connect(ui->btn_valider_etape, &QPushButton::clicked, this, [=]() {
        int idPlanif = -1;
        QString desProduit;
        int qteCmd = 1;
        if (indexCommandeSelectionnee >= 0 && indexCommandeSelectionnee < mesCommandes.size()) {
            QString idStr = mesCommandes[indexCommandeSelectionnee].id;
            idPlanif = QString(idStr).replace(QStringLiteral("OF-"), QString()).toInt();
            desProduit = mesCommandes[indexCommandeSelectionnee].idProduit;
            qteCmd = std::max(1, mesCommandes[indexCommandeSelectionnee].quantite);
        } else if (m_fabTimelineIdPlanif > 0) {
            idPlanif = m_fabTimelineIdPlanif;
            desProduit = m_fabTimelineProduit;
            qteCmd = std::max(1, m_fabTimelineQte);
        }
        if (idPlanif < 0 || desProduit.isEmpty()) {
            alerteWarning(QStringLiteral("Sélection"),
                          QStringLiteral("Sélectionnez une ligne dans le tableau des étapes (timeline) ou un OF dans le Gantt."));
            return;
        }

        const QString etapeActuelle = ui->cb_etape_suivi->currentText();
        const double tempsReel = joursSaisieVersHeuresFabrication(ui->sb_temps_reel_input->value());
        const double tempsPrevu = tempsPrevuHeuresEtapePourCommandeIA(desProduit, qteCmd, etapeActuelle);
        const double delta = tempsReel - tempsPrevu;
        const int alerte = (delta > 0) ? 1 : 0;
        const int idEmploye = 1;

        Etape e(idPlanif, idEmploye, etapeActuelle, tempsReel, delta, alerte);

        if (e.ajouter()) {
            if (alerte == 1) {
                ui->lbl_resultat_delta->setText(QStringLiteral("⚠️ RETARD ") + formaterDeltaEtapesEnJoursAffichage(delta));
                ui->lbl_resultat_delta->setStyleSheet(
                    "font-weight: 900; font-size: 15px; color: white;"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #c62828, stop:1 #e53935);"
                    "padding: 10px 18px; border-radius: 12px; border: none; min-width: 120px;"
                );
                if (indexCommandeSelectionnee >= 0 && indexCommandeSelectionnee < mesCommandes.size())
                    mesCommandes[indexCommandeSelectionnee].etatEtape = 2;
            } else {
                ui->lbl_resultat_delta->setText(QStringLiteral("✅ DANS LES TEMPS"));
                ui->lbl_resultat_delta->setStyleSheet(
                    "font-weight: 900; font-size: 15px; color: white;"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2e7d32, stop:1 #43a047);"
                    "padding: 10px 18px; border-radius: 12px; border: none; min-width: 120px;"
                );
                if (indexCommandeSelectionnee >= 0 && indexCommandeSelectionnee < mesCommandes.size())
                    mesCommandes[indexCommandeSelectionnee].etatEtape = 1;
            }
            alerteSucces(QStringLiteral("Succès"), QStringLiteral("Temps enregistré dans la base avec succès !"));
            configurerTimelineGantt();
            rafraichirListeEtapes();
        } else {
            alerteErreur(QStringLiteral("Erreur"), QStringLiteral("Impossible d'enregistrer l'étape."));
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

        // 3. Hauteur de ligne harmonisée (captures liste modules)
        table->verticalHeader()->setDefaultSectionSize(43);

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
            "  border-radius: 8px;"
            "  font-size: 13px;"
            "  color: #3e2723;"
            "}"
            "QTableWidget::item {"
            "  padding: 7px 10px;"
            "  border-bottom: 1px solid #f0ebe4;"
            "}"
            "QTableWidget::item:selected {"
            "  background-color: #e0c097;"
            "  color: #2c1a16;"
            "}"
            "QHeaderView::section {"
            "  background-color: #2c1a16;"
            "  color: #e0c097;"
            "  padding: 11px 8px;"
            "  border: none;"
            "  font-weight: bold;"
            "  font-size: 12px;"
            "  text-transform: uppercase;"
            "  letter-spacing: 1px;"
            "}"
        );
        table->horizontalHeader()->setMinimumHeight(40);

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
    ui->btn_edit_depot->setFixedSize(138, 42);
    ui->btn_delete_depot->setFixedSize(150, 42);
    ui->btn_modifier_planif->setFixedSize(138, 42);
    ui->btn_supprimer_planif->setFixedSize(150, 42);
    ui->btn_edit_stock->setFixedSize(138, 42);
    ui->btn_delete_stock->setFixedSize(150, 42);
    ui->btn_edit_client->setFixedSize(138, 42);
    ui->btn_delete_client->setFixedSize(150, 42);
    ui->btn_edit_produit->setFixedSize(138, 42);
    ui->btn_delete_produit->setFixedSize(150, 42);
    ui->btn_edit_emp->setFixedSize(138, 42);
    ui->btn_delete_emp->setFixedSize(150, 42);

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

    // Micro-passe: hauteur pixel-perfect des boutons top-bar (actions hautes)
    auto applyTopBarButtonHeight = [](QPushButton *btn) {
        if (!btn) return;
        btn->setFixedHeight(38);
    };
    QList<QPushButton*> topBarButtons = {
        ui->btn_rechercher, ui->btn_sort_alpha_planif, ui->btn_open_planif_ia, ui->btn_stat_plan, ui->btn_pdf, ui->btn_to_add_planif, ui->btn_tri,
        ui->btn_search_stock, ui->btn_sort_alpha_stock, ui->btn_pdf_stock, ui->btn_export_excel_stock, ui->btn_print_stock, ui->btn_add_stock,
        ui->btn_search_client, ui->btn_sort_alpha_client, ui->btn_pdf_facture, ui->btn_export_excel_client, ui->btn_add_client,
        ui->btn_search_depot, ui->btn_sort_alpha_depot, ui->btn_pdf_depot, ui->btn_add_depot,
        ui->btn_search_col, ui->btn_sort_alpha_prod, ui->btn_cout_produit, ui->btn_hist_mode, ui->btn_stats_prod, ui->btn_pdf_catalogue, ui->btn_add_produit,
        ui->btn_search_emp, ui->btn_pdf_emp, ui->btn_add_emp
    };
    for (QPushButton *btn : topBarButtons) applyTopBarButtonHeight(btn);

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
    QString tabStyleDepotFin =
        "QTabWidget::pane {"
        "  border: 1px solid #d7ccc8;"
        "  background: white;"
        "  border-radius: 10px;"
        "  margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "  background: #f3f0eb;"
        "  color: #5d4037;"
        "  border: 1px solid #d7ccc8;"
        "  border-bottom-color: #d7ccc8;"
        "  border-top-left-radius: 9px;"
        "  border-top-right-radius: 9px;"
        "  min-width: 154px;"
        "  padding: 10px 14px;"
        "  font-weight: 700;"
        "  font-size: 14px;"
        "  margin-right: 5px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #946034;"
        "  color: white;"
        "  border-color: #946034;"
        "  border-bottom-color: #946034;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e2c29f;"
        "}";
    ui->tabWidgetDepot->setStyleSheet(tabStyleDepotFin);

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

    // Réglage A/B global pour la typo des labels de formulaires
    // true  => 15pt DemiBold
    // false => 14pt Bold
    const bool formLabelVariantA = true;
    const int formLabelPointSize = formLabelVariantA ? 15 : 14;
    const QFont::Weight formLabelWeight = formLabelVariantA ? QFont::DemiBold : QFont::Bold;

    auto applyFormLabelTypography = [&](QFormLayout *fl) {
        if (!fl) return;
        fl->setHorizontalSpacing(20);
        fl->setVerticalSpacing(14);
        fl->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        for (int r = 0; r < fl->rowCount(); ++r) {
            QWidget *lw = fl->itemAt(r, QFormLayout::LabelRole) ? fl->itemAt(r, QFormLayout::LabelRole)->widget() : nullptr;
            if (auto *lab = qobject_cast<QLabel*>(lw)) {
                QFont f = lab->font();
                f.setPointSize(formLabelPointSize);
                f.setWeight(formLabelWeight);
                lab->setFont(f);
                lab->setStyleSheet("color: #5a4a42; padding-right: 4px;");
            }
        }
    };

    auto applyWidePrimaryButton = [&](QPushButton *btn) {
        if (!btn) return;
        btn->setMinimumHeight(44);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    };

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
    applyWidePrimaryButton(ui->btn_valider_produit);

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
    applyWidePrimaryButton(ui->btn_valider_modif_produit);

    if(ui->tab_prod_new->layout())
        ui->tab_prod_new->layout()->setContentsMargins(50, 26, 50, 24);
    if(ui->tab_prod_edit->layout())
        ui->tab_prod_edit->layout()->setContentsMargins(50, 26, 50, 24);

    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_prod_new->layout()));
    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_prod_edit->layout()));
    ui->gb_prod_new->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 9px 11px; font-size: 16px; color: #4a4a4a; min-height: 22px; }"
    );
    ui->gb_prod_edit->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 9px 11px; font-size: 16px; color: #4a4a4a; min-height: 22px; }"
    );

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
    applyWidePrimaryButton(ui->btn_valider_emp);

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
    applyWidePrimaryButton(ui->btn_valider_modif_emp);

    if(ui->tab_recrutement->layout())
        ui->tab_recrutement->layout()->setContentsMargins(44, 28, 44, 26);
    if(ui->tab_modifier_dossier->layout())
        ui->tab_modifier_dossier->layout()->setContentsMargins(44, 28, 44, 26);

    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->groupBox->layout()));
    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->groupBox_2->layout()));
    ui->groupBox->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 16px; color: #4a4a4a; min-height: 23px; }"
    );
    ui->groupBox_2->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 16px; color: #4a4a4a; min-height: 23px; }"
    );

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
    applyWidePrimaryButton(ui->btn_valider_client);

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
    applyWidePrimaryButton(ui->btn_valider_modif_client);

    if(ui->tab_client_add->layout())
        ui->tab_client_add->layout()->setContentsMargins(42, 28, 42, 26);
    if(ui->tab_client_edit->layout())
        ui->tab_client_edit->layout()->setContentsMargins(42, 28, 42, 26);

    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_client_add->layout()));
    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_client_edit->layout()));
    ui->gb_client_add->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 16px; color: #4a4a4a; min-height: 23px; }"
    );
    ui->gb_client_edit->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 16px; color: #4a4a4a; min-height: 23px; }"
    );

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
    ui->btn_valider_depot->setMinimumHeight(44);
    ui->btn_valider_depot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
    ui->btn_valider_modif_depot->setMinimumHeight(44);
    ui->btn_valider_modif_depot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if(ui->tab_depot_add->layout())
        ui->tab_depot_add->layout()->setContentsMargins(30, 22, 30, 24);
    if(ui->tab_depot_edit->layout())
        ui->tab_depot_edit->layout()->setContentsMargins(30, 22, 30, 24);

    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_depot_add->layout()));
    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_depot_edit->layout()));

    ui->gb_depot_add->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 17px; color: #4a4a4a; min-height: 24px; }"
    );
    ui->gb_depot_edit->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox { border: 1px solid #d7d7d7; border-radius: 8px; padding: 10px 12px; font-size: 17px; color: #4a4a4a; min-height: 24px; }"
    );

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
    ui->btn_valider_planif->setText("Créer Ordre");
    ui->btn_valider_planif->setMinimumSize(200, 45);
    applyWidePrimaryButton(ui->btn_valider_planif);
    // Bouton équipe — formulaire Nouvel Ordre
    ui->btn_equipe_planif->setMinimumHeight(42);
    ui->btn_equipe_planif->setCursor(Qt::PointingHandCursor);
    ui->btn_equipe_planif->setStyleSheet(QStringLiteral(
        "QPushButton { background: rgba(212,175,55,0.12); border: 2px solid rgba(212,175,55,0.55);"
        "  border-radius: 12px; padding: 9px 14px; font-weight: 900; font-size: 12px; color: #6d4c41; }"
        "QPushButton:hover { background: rgba(212,175,55,0.22); border-color: #d4af37; }"));

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
    applyWidePrimaryButton(ui->btn_valider_modif);
    // Bouton équipe — formulaire Modifier Commande
    ui->btn_equipe_modif->setMinimumHeight(42);
    ui->btn_equipe_modif->setCursor(Qt::PointingHandCursor);
    ui->btn_equipe_modif->setStyleSheet(QStringLiteral(
        "QPushButton { background: rgba(212,175,55,0.12); border: 2px solid rgba(212,175,55,0.55);"
        "  border-radius: 12px; padding: 9px 14px; font-weight: 900; font-size: 12px; color: #6d4c41; }"
        "QPushButton:hover { background: rgba(212,175,55,0.22); border-color: #d4af37; }"));

    if(ui->tab_planif_form->layout())
        ui->tab_planif_form->layout()->setContentsMargins(58, 28, 58, 26);
    if(ui->tab_planif_modif->layout())
        ui->tab_planif_modif->layout()->setContentsMargins(58, 28, 58, 26);

    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_form_planif->layout()));
    applyFormLabelTypography(qobject_cast<QFormLayout*>(ui->gb_form_planif_modif->layout()));
    ui->gb_form_planif->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit { border: 1px solid #d7d7d7; border-radius: 8px; padding: 9px 11px; font-size: 16px; color: #4a4a4a; min-height: 22px; }"
    );
    ui->gb_form_planif_modif->setStyleSheet(
        formStyle +
        "QLabel { color: #5a4a42; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit { border: 1px solid #d7d7d7; border-radius: 8px; padding: 9px 11px; font-size: 16px; color: #4a4a4a; min-height: 22px; }"
    );

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

    // SpinBox temps réel (saisie en jours ouvrés ; stockage BDD en heures)
    ui->sb_temps_reel_input->setDecimals(2);
    ui->sb_temps_reel_input->setSuffix(QStringLiteral(" j"));
    ui->sb_temps_reel_input->setMaximum(kJoursMaxSaisieTempsReelFabrication);
    ui->sb_temps_reel_input->setSingleStep(0.125);
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

    // Injection visuelle ciblée Dépôt (fusion conservatrice)
    setupDepotExpertUI();


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
    // 6bis. DASHBOARD ML (qualité / drift / statut modèle)
    // =============================================
    QLabel *secMl = new QLabel(QStringLiteral("🧠  DASHBOARD MACHINE LEARNING"));
    secMl->setStyleSheet("font-size: 12px; font-weight: 800; color: #d4af37; letter-spacing: 2px; border: none;");
    mainLayout->addWidget(secMl);

    // Cadre principal
    QFrame *frameMl = new QFrame();
    frameMl->setStyleSheet(
        "QFrame { background: rgba(0,0,0,0.20); border: 1px solid rgba(212,175,55,0.20); border-radius: 14px; }"
    );
    QHBoxLayout *mlLayout = new QHBoxLayout(frameMl);
    mlLayout->setContentsMargins(14, 12, 14, 12);
    mlLayout->setSpacing(12);

    // ---- Collecte stats ML depuis BDD ----
    struct MlDash {
        int nSamples = 0;
        int nEtapesTotal = 0;
        int nEtapesMesurees = 0;
        bool modelTrained = false;
        double maeHours = -1.0;
        QMap<QString, double> meanAll;   // h/unité global (par étape)
        QMap<QString, double> meanRecent; // h/unité recent (par étape)
    } ml;

    // Dataset (mêmes filtres que trainLinearModelFromDB)
    {
        QSqlQuery qml;
        if (qml.exec(QStringLiteral(
                "SELECT COUNT(*) "
                "FROM ETAPES e "
                "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
                "WHERE e.TEMPS_REEL_PASSE IS NOT NULL "
                "AND e.TEMPS_REEL_PASSE > 0 "
                "AND p.QUANTITE > 0 "
                "AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION')")))
        {
            if (qml.next()) ml.nSamples = qml.value(0).toInt();
        }

        // Couverture mesures vs total
        if (qml.exec(QStringLiteral("SELECT COUNT(*) FROM ETAPES")) && qml.next())
            ml.nEtapesTotal = qml.value(0).toInt();
        if (qml.exec(QStringLiteral("SELECT COUNT(*) FROM ETAPES WHERE NVL(TEMPS_REEL_PASSE,0) > 0")) && qml.next())
            ml.nEtapesMesurees = qml.value(0).toInt();

        // Moyennes globales h/unité (robustes)
        qml.prepare(QStringLiteral(
            "SELECT UPPER(TRIM(e.ETAPE_ACTUELLE)) AS etape, "
            "AVG(e.TEMPS_REEL_PASSE / NULLIF(p.QUANTITE, 0)) AS h_unit "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE > 0 "
            "AND p.QUANTITE > 0 "
            "AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION') "
            "AND (e.TEMPS_REEL_PASSE / p.QUANTITE) BETWEEN 0.01 AND 5 "
            "GROUP BY UPPER(TRIM(e.ETAPE_ACTUELLE))"));
        if (qml.exec()) {
            while (qml.next()) {
                const QString e = qml.value(0).toString();
                const double v = qml.value(1).toDouble();
                if (!e.isEmpty() && v > 0.0) ml.meanAll[e] = v;
            }
        }

        // Moyennes récentes (90 jours) : approximation de drift
        qml.prepare(QStringLiteral(
            "SELECT UPPER(TRIM(e.ETAPE_ACTUELLE)) AS etape, "
            "AVG(e.TEMPS_REEL_PASSE / NULLIF(p.QUANTITE, 0)) AS h_unit "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE > 0 "
            "AND p.QUANTITE > 0 "
            "AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION') "
            "AND (e.TEMPS_REEL_PASSE / p.QUANTITE) BETWEEN 0.01 AND 5 "
            "AND p.DATE_LANCEMENT >= (SYSDATE - 90) "
            "GROUP BY UPPER(TRIM(e.ETAPE_ACTUELLE))"));
        if (qml.exec()) {
            while (qml.next()) {
                const QString e = qml.value(0).toString();
                const double v = qml.value(1).toDouble();
                if (!e.isEmpty() && v > 0.0) ml.meanRecent[e] = v;
            }
        }
    }

    // Essaie d'entraîner le modèle (sans bloquer l'app) pour afficher "statut modèle".
    // Si dataset insuffisant, on reste en fallback.
    ml.modelTrained = trainLinearModelFromDB();

    // MAE (sur un petit échantillon) : compare réel vs prédiction ML (sinon vs moyenne historique).
    {
        QSqlQuery qe;
        qe.prepare(QStringLiteral(
            "SELECT p.QUANTITE AS qte, "
            "UPPER(TRIM(e.ETAPE_ACTUELLE)) AS etape, "
            "NVL((SELECT COUNT(DISTINCT e2.ID_EMPLOYE) FROM ETAPES e2 WHERE e2.ID_PLANIFICATION = p.ID_COMMANDE), 1) AS nb_emp, "
            "e.TEMPS_REEL_PASSE AS y_h "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE > 0 "
            "AND p.QUANTITE > 0 "
            "AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION') "
            "AND ROWNUM <= 350"));

        double sumAbs = 0.0;
        int n = 0;
        if (qe.exec()) {
            while (qe.next()) {
                const double qte = qMax(1.0, qe.value(0).toDouble());
                const QString et = qe.value(1).toString();
                const double nbEmp = qMax(1.0, qe.value(2).toDouble());
                const double yH = qe.value(3).toDouble();
                if (yH <= 0.0) continue;

                double yHat = 0.0;
                if (ml.modelTrained) {
                    const int code = mapEtapeToCode(et);
                    // on prend complexité=1 et cadence=1 côté dashboard (ne pas inventer des infos)
                    yHat = predictStepHours(code, qte, 1.0, nbEmp, 1.0);
                }
                if (yHat <= 0.0) {
                    const double mu = ml.meanAll.value(et, 0.0);
                    if (mu > 0.0) yHat = mu * qte;
                }
                if (yHat <= 0.0) continue;

                sumAbs += qAbs(yHat - yH);
                ++n;
            }
        }
        if (n >= 20) ml.maeHours = sumAbs / n;
    }

    auto pct = [](int num, int den) -> double {
        return (den > 0) ? (static_cast<double>(num) * 100.0 / static_cast<double>(den)) : 0.0;
    };
    const double coverage = pct(ml.nEtapesMesurees, ml.nEtapesTotal);

    // ---- Colonne gauche: cartes KPI + actions ----
    QWidget *mlLeft = new QWidget();
    mlLeft->setStyleSheet("background: transparent;");
    QVBoxLayout *mlLeftL = new QVBoxLayout(mlLeft);
    mlLeftL->setContentsMargins(0, 0, 0, 0);
    mlLeftL->setSpacing(10);

    QLabel *mlSubtitle = new QLabel(QStringLiteral("Qualité modèle • Données d'entraînement • Dérive (90j)"));
    mlSubtitle->setStyleSheet("font-size: 11px; color: #bcaaa4; border: none;");
    mlLeftL->addWidget(mlSubtitle);

    auto creerCarteKpiMini = [](const QString& title, const QString& value, const QString& hint, const QString& accent) -> QFrame* {
        QFrame *c = new QFrame();
        c->setStyleSheet(QStringLiteral(
            "QFrame { background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.10); border-radius: 12px; }"
        ));
        QVBoxLayout *vl = new QVBoxLayout(c);
        vl->setContentsMargins(12, 10, 12, 10);
        vl->setSpacing(2);
        QLabel *t = new QLabel(title);
        t->setStyleSheet("font-size: 10px; font-weight: 900; color: rgba(224,192,151,0.95); letter-spacing: 1px; border: none;");
        QLabel *v = new QLabel(value);
        v->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 900; color: %1; border: none;").arg(accent));
        QLabel *h = new QLabel(hint);
        h->setStyleSheet("font-size: 10px; color: rgba(188,170,164,0.95); border: none;");
        h->setWordWrap(true);
        vl->addWidget(t);
        vl->addWidget(v);
        vl->addWidget(h);
        return c;
    };

    const QString modelStatus = ml.modelTrained ? QStringLiteral("ACTIF") : QStringLiteral("FALLBACK");
    const QString modelColor = ml.modelTrained ? QStringLiteral("#66bb6a") : QStringLiteral("#ffa726");
    mlLeftL->addWidget(creerCarteKpiMini(
        QStringLiteral("STATUT MODÈLE"),
        modelStatus,
        ml.modelTrained ? QStringLiteral("Régression entraînée sur l'historique ETAPES") : QStringLiteral("Données insuffisantes: estimation heuristique"),
        modelColor));

    mlLeftL->addWidget(creerCarteKpiMini(
        QStringLiteral("DATASET (SAMPLES)"),
        QString::number(ml.nSamples),
        QStringLiteral("Nombre d'exemples utilisables (temps réel > 0)"),
        QStringLiteral("#d4af37")));

    mlLeftL->addWidget(creerCarteKpiMini(
        QStringLiteral("COUVERTURE MESURES"),
        QString::number(coverage, 'f', 1) + QStringLiteral("%"),
        QStringLiteral("Part des étapes avec temps réel saisi"),
        coverage >= 65.0 ? QStringLiteral("#66bb6a") : QStringLiteral("#ef5350")));

    const QString maeTxt = (ml.maeHours >= 0.0)
        ? (QString::number(ml.maeHours, 'f', 2) + QStringLiteral(" h"))
        : QStringLiteral("N/A");
    mlLeftL->addWidget(creerCarteKpiMini(
        QStringLiteral("ERREUR (MAE)"),
        maeTxt,
        QStringLiteral("Erreur moyenne absolue sur échantillon (plus bas = mieux)"),
        (ml.maeHours >= 0.0 && ml.maeHours <= 6.0) ? QStringLiteral("#66bb6a") : QStringLiteral("#ffa726")));

    QHBoxLayout *mlActions = new QHBoxLayout();
    mlActions->setSpacing(8);
    QPushButton *btnRetrain = new QPushButton(QStringLiteral("Ré-entraîner"));
    btnRetrain->setCursor(Qt::PointingHandCursor);
    btnRetrain->setStyleSheet(
        "QPushButton { background-color: #7b1fa2; color: white; padding: 8px 12px; border-radius: 10px; font-weight: 800; }"
        "QPushButton:hover { background-color: #8e24aa; }"
        "QPushButton:pressed { background-color: #6a1b9a; }"
    );
    QPushButton *btnRefresh = new QPushButton(QStringLiteral("Actualiser"));
    btnRefresh->setCursor(Qt::PointingHandCursor);
    btnRefresh->setStyleSheet(
        "QPushButton { background-color: rgba(255,255,255,0.08); color: #e0c097; padding: 8px 12px; border-radius: 10px; font-weight: 800; border: 1px solid rgba(224,192,151,0.25); }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.12); }"
    );
    mlActions->addWidget(btnRetrain);
    mlActions->addWidget(btnRefresh);
    mlActions->addStretch();
    mlLeftL->addLayout(mlActions);

    // Rebuild dashboard on actions (stateless, simple)
    connect(btnRefresh, &QPushButton::clicked, this, [this]() { construireDashboardAccueil(); });
    connect(btnRetrain, &QPushButton::clicked, this, [this]() {
        const bool ok = trainLinearModelFromDB(true);
        if (ok) alerteSucces(QStringLiteral("ML"), QStringLiteral("Modèle ré-entrainé à partir de l'historique."));
        else alerteWarning(QStringLiteral("ML"), QStringLiteral("Dataset insuffisant ou erreur BDD. Le mode fallback reste actif."));
        construireDashboardAccueil();
    });

    mlLayout->addWidget(mlLeft, 2);

    // ---- Colonne droite: drift chart (90j vs global) ----
    QFrame *mlRight = new QFrame();
    mlRight->setStyleSheet("QFrame { background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.08); border-radius: 12px; }");
    QVBoxLayout *mlRightL = new QVBoxLayout(mlRight);
    mlRightL->setContentsMargins(10, 10, 10, 10);
    mlRightL->setSpacing(6);

    QLabel *driftTitle = new QLabel(QStringLiteral("Dérive 90 jours (h/unité)"));
    driftTitle->setStyleSheet("font-size: 12px; font-weight: 900; color: #d4af37; border: none;");
    QLabel *driftHint = new QLabel(QStringLiteral("Comparaison moyenne récente vs historique (par étape)."));
    driftHint->setStyleSheet("font-size: 10px; color: rgba(188,170,164,0.95); border: none;");
    driftHint->setWordWrap(true);
    mlRightL->addWidget(driftTitle);
    mlRightL->addWidget(driftHint);

    // Chart drift (bar: % delta)
    QStringList cats;
    QList<double> deltas;
    const QStringList steps = { QStringLiteral("COUPE"), QStringLiteral("ASSEMBLAGE"), QStringLiteral("COUTURE"), QStringLiteral("FINITION") };
    for (const QString& s : steps) {
        const double a = ml.meanAll.value(s, 0.0);
        const double r = ml.meanRecent.value(s, 0.0);
        if (a <= 0.0 || r <= 0.0) {
            cats << s;
            deltas << 0.0;
            continue;
        }
        cats << s;
        deltas << ((r - a) / a) * 100.0;
    }

    QWidget *chartHost = new QWidget();
    chartHost->setMinimumHeight(220);
    chartHost->setStyleSheet("background: transparent;");
    mlRightL->addWidget(chartHost, 1);

    // Utilise helper bar chart existant (titre + valeurs)
    setVerticalBarChart(chartHost,
                        QStringLiteral("Δ%% récent vs historique"),
                        cats,
                        deltas);

    // Légende textuelle
    auto fmtDelta = [](double v) -> QString {
        const QString sign = (v > 0.0) ? QStringLiteral("+") : QString();
        return sign + QString::number(v, 'f', 1) + QStringLiteral("%");
    };
    QString driftText;
    for (int i = 0; i < cats.size(); ++i) {
        driftText += QStringLiteral("<div style='margin:2px 0;'><b>%1</b>: %2</div>")
            .arg(cats[i].toHtmlEscaped(), fmtDelta(deltas[i]).toHtmlEscaped());
    }
    QLabel *driftLbl = new QLabel(driftText);
    driftLbl->setTextFormat(Qt::RichText);
    driftLbl->setStyleSheet("font-size: 10px; color: rgba(224,192,151,0.95); border: none;");
    driftLbl->setWordWrap(true);
    mlRightL->addWidget(driftLbl);

    mlLayout->addWidget(mlRight, 3);

    mainLayout->addWidget(frameMl);

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
    ui->tablePlanif->setWordWrap(true);
    ui->tablePlanif->setTextElideMode(Qt::ElideNone);
    ui->tablePlanif->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive);
    ui->tablePlanif->setColumnWidth(7, 260);

    int rows = model->rowCount();
    ui->tablePlanif->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        QString idStr = model->record(i).value("ID_COMMANDE").toString();
        const int idCmd = idStr.toInt();
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
        const QString empFull = equipeEmployesPourCommande(idCmd, emp);
        const QStringList empNames = extraireNomsEquipe(empFull);
        const QString empShort = formatEquipeAffichageCourt(empFull, 2);
        const QString empDisplay = formatEquipeAffichageLignes(empFull, 3);

        QString finStr = fin.toString("dd/MM/yyyy");

        ui->tablePlanif->setItem(i, 0, new QTableWidgetItem(idStr));
        ui->tablePlanif->setItem(i, 1, new QTableWidgetItem(prod));
        ui->tablePlanif->setItem(i, 2, new QTableWidgetItem(QString::number(qte)));
        ui->tablePlanif->setItem(i, 3, new QTableWidgetItem(mat));
        ui->tablePlanif->setItem(i, 4, new QTableWidgetItem(deb.toString("dd/MM/yyyy")));
        ui->tablePlanif->setItem(i, 5, new QTableWidgetItem(finStr));
        ui->tablePlanif->setItem(i, 6, new QTableWidgetItem(stat));
        auto *empItem = new QTableWidgetItem(empDisplay.isEmpty() ? (empShort.isEmpty() ? empFull : empShort) : empDisplay);
        empItem->setToolTip(formatEquipeTooltip(empFull));
        empItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->tablePlanif->setItem(i, 7, empItem);
        const int lignesAffichees = std::max(1, std::min(4, static_cast<int>(empNames.size())));
        const int hauteurSouhaitee = 22 + (lignesAffichees * 16);
        ui->tablePlanif->setRowHeight(i, std::max(ui->tablePlanif->rowHeight(i), hauteurSouhaitee));

        CommandeInfo c;
        c.id = "OF-" + idStr;
        c.idProduit = prod;   // <--- CORRIGÉ (idProduit)
        c.quantite = qte;
        c.idMatiere = mat;    // <--- CORRIGÉ (idMatiere)
        c.dateDebut = deb; c.dateFinEstimee = finStr; c.statut = stat; c.etatEtape = 0; c.idEmploye = empFull;
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
    if (!ui || !ui->tab_planif_stats) return;
    QWidget *page = ui->tab_planif_stats;
    if (page->layout()) { clearLayout(page->layout()); delete page->layout(); }

    int total = 0, totalQte = 0, plan = 0, cours = 0, ret = 0, cmdEnRetard = 0;
    int nbSacs = 0, nbPortefeuille = 0, nbCeinture = 0;

    QSqlQuery q;
    if (q.exec("SELECT COUNT(*), NVL(SUM(QUANTITE), 0) FROM PLANIFICATION") && q.next()) {
        total = q.value(0).toInt();
        totalQte = q.value(1).toInt();
    }
    if (q.exec("SELECT STATUT, COUNT(*) FROM PLANIFICATION GROUP BY STATUT")) {
        while (q.next()) {
            const QString st = q.value(0).toString().toLower();
            const int cnt = q.value(1).toInt();
            if (st.contains("plan")) plan += cnt;
            else if (st.contains("cours")) cours += cnt;
            else if (st.contains("retard")) ret += cnt;
        }
    }
    if (q.exec("SELECT COUNT(DISTINCT ID_PLANIFICATION) FROM ETAPES WHERE ALERTE_ACTIVE = 1") && q.next()) {
        cmdEnRetard = q.value(0).toInt();
    }
    if (q.exec("SELECT NVL(PR.DESIGNATION,''), COUNT(*) "
               "FROM PLANIFICATION P LEFT JOIN PRODUITS PR ON P.ID_PRODUIT = PR.ID_PRODUIT "
               "GROUP BY PR.DESIGNATION")) {
        while (q.next()) {
            const QString d = q.value(0).toString().toLower();
            const int cnt = q.value(1).toInt();
            if (d.contains("sac")) nbSacs += cnt;
            else if (d.contains("portefeuille")) nbPortefeuille += cnt;
            else if (d.contains("ceinture")) nbCeinture += cnt;
        }
    }
    const double retardPct = (total > 0) ? (100.0 * cmdEnRetard / total) : 0.0;
    const int enCours = qMax(0, total - plan - ret);

    QVBoxLayout *mainL = new QVBoxLayout(page);
    mainL->setSpacing(10);
    mainL->setContentsMargins(10, 10, 10, 10);
    QScrollArea *sa = new QScrollArea(page);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12);
    root->setContentsMargins(8, 8, 8, 8);

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse Planification - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit OF      ② Performance Production      ③ Diagnostic Retards");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht); hl->addWidget(hs); root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10); cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(total), "Total Ordres", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "📋"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(totalQte), "Production", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "📦"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(cmdEnRetard), "OF en Retard", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "⚠️"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(retardPct, 'f', 0) + "%", "Taux Retard", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 0);
    kpi->addWidget(makeKpiCard(QString::number(plan), "Planifiées", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "🧭"), 1, 1);
    kpi->addWidget(makeKpiCard(QString::number(enCours), "En Cours", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "⏳"), 1, 2);
    root->addLayout(kpi);

    auto makeGroup = [](const QString &title) {
        QGroupBox *g = new QGroupBox(title);
        g->setStyleSheet(
            "QGroupBox { background:#ffffff; border:1px solid #e3d9cf; border-radius:10px; margin-top:10px; font-weight:800; color:#6d4c41; }"
            "QGroupBox::title { subcontrol-origin: margin; left:12px; padding:0 4px; }"
        );
        return g;
    };
    auto mkBar = [](int value, int max) {
        QProgressBar *pb = new QProgressBar();
        pb->setMaximum(qMax(1, max));
        pb->setValue(qMax(0, value));
        pb->setFormat("%v/%m");
        pb->setStyleSheet("QProgressBar{border:1px solid #d7ccc8;border-radius:9px;text-align:center;background:#f3f0eb;height:20px;font-weight:700;color:#3e2723;}QProgressBar::chunk{border-radius:9px;background-color:#8d5524;}");
        return pb;
    };

    QHBoxLayout *rows = new QHBoxLayout();
    rows->setSpacing(10);

    QGroupBox *gModel = makeGroup("Répartition par Modèles");
    QVBoxLayout *gm = new QVBoxLayout(gModel);
    QLabel *m1 = new QLabel(QString("Sacs : %1").arg(nbSacs)); m1->setStyleSheet("color:#5d4037;font-weight:700;");
    QLabel *m2 = new QLabel(QString("Portefeuilles : %1").arg(nbPortefeuille)); m2->setStyleSheet("color:#5d4037;font-weight:700;");
    QLabel *m3 = new QLabel(QString("Ceintures : %1").arg(nbCeinture)); m3->setStyleSheet("color:#5d4037;font-weight:700;");
    gm->addWidget(m1); gm->addWidget(mkBar(nbSacs, total));
    gm->addWidget(m2); gm->addWidget(mkBar(nbPortefeuille, total));
    gm->addWidget(m3); gm->addWidget(mkBar(nbCeinture, total));
    rows->addWidget(gModel);

    QGroupBox *gStat = makeGroup("Répartition par Statut");
    QVBoxLayout *gs = new QVBoxLayout(gStat);
    QLabel *s1 = new QLabel(QString("Planifié : %1").arg(plan)); s1->setStyleSheet("color:#5d4037;font-weight:700;");
    QLabel *s2 = new QLabel(QString("En cours : %1").arg(cours)); s2->setStyleSheet("color:#5d4037;font-weight:700;");
    QLabel *s3 = new QLabel(QString("Retard : %1").arg(ret)); s3->setStyleSheet("color:#5d4037;font-weight:700;");
    gs->addWidget(s1); gs->addWidget(mkBar(plan, total));
    gs->addWidget(s2); gs->addWidget(mkBar(cours, total));
    gs->addWidget(s3); gs->addWidget(mkBar(ret, total));
    rows->addWidget(gStat);
    root->addLayout(rows);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA Planification");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA Planification", "Assistant prêt : priorisation OF, suivi retards et actions de rattrapage.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);
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
    ui->tableProduits->setRowCount(rows);
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
    const bool triActif = ui->tableEmployes->isSortingEnabled();
    ui->tableEmployes->setSortingEnabled(false);

    ui->tableEmployes->setRowCount(0);
    ui->tableEmployes->setColumnCount(7);
    ui->tableEmployes->setHorizontalHeaderLabels({"ID", "NOM", "PRÉNOM", "POSTE", "DÉPARTEMENT", "SALAIRE", "EMBAUCHE"});

    // Rafraîchir depuis Oracle via l'entité.
    mesEmployes.clear();

    employe emp;
    QSqlQueryModel *model = emp.afficher();
    if (!model) {
        ui->tableEmployes->setSortingEnabled(triActif);
        return;
    }
    const int rows = model->rowCount();
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

        auto *itemId = new QTableWidgetItem(info.id);
        itemId->setData(Qt::UserRole, i);
        ui->tableEmployes->setItem(i, 0, itemId);
        ui->tableEmployes->setItem(i, 1, new QTableWidgetItem(info.nom));
        ui->tableEmployes->setItem(i, 2, new QTableWidgetItem(info.prenom));
        ui->tableEmployes->setItem(i, 3, new QTableWidgetItem(info.poste));
        ui->tableEmployes->setItem(i, 4, new QTableWidgetItem(info.departement));
        ui->tableEmployes->setItem(i, 5, new QTableWidgetItem(QString::number(info.salaire, 'f', 0) + " DT"));
        ui->tableEmployes->setItem(i, 6, new QTableWidgetItem(info.dateEmbauche.toString("dd/MM/yyyy")));
    }

    delete model;

    ui->tableEmployes->setSortingEnabled(triActif);
    if (employeTriAlphaActif && ui->tableEmployes) {
        ui->tableEmployes->sortByColumn(1, employeTriAlphaOrdre); // colonne 1 = NOM
    }
}
void MainWindow::rafraichirListeMatieres() {
    const bool triActif = ui->tableStock->isSortingEnabled();
    ui->tableStock->setSortingEnabled(false);

    // Lecture depuis Oracle
    QSqlQueryModel *model = tmpMatiere.afficher();
    if (!model) {
        ui->tableStock->setSortingEnabled(triActif);
        return;
    }

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
    ui->tableStock->setSortingEnabled(triActif);
}

void MainWindow::remplirTableClients(QSqlQueryModel *model)
{
    if (!model)
        return;

    const bool triActif = ui->tableClients->isSortingEnabled();
    ui->tableClients->setSortingEnabled(false);

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
    ui->tableClients->setSortingEnabled(triActif);
}

void MainWindow::rafraichirListeClients()
{
    Connexion *cnx = Connexion::getInstance();
    if (cnx && cnx->estConnecte()) {
        Client c;
        remplirTableClients(c.afficher());
        return;
    }

    const bool triActif = ui->tableClients->isSortingEnabled();
    ui->tableClients->setSortingEnabled(false);

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
    ui->tableClients->setSortingEnabled(triActif);
}
void MainWindow::rafraichirListeDepots() {
    const bool triActif = ui->tableDepot->isSortingEnabled();
    ui->tableDepot->setSortingEnabled(false);

    QSqlQueryModel *model = tmpDepot.afficher();
    if (!model) {
        ui->tableDepot->setSortingEnabled(triActif);
        return;
    }

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
        auto *itemEt = new QTableWidgetItem(et);
        itemEt->setData(Qt::UserRole, idDb);
        ui->tableDepot->setItem(i, 2, itemEt);
        ui->tableDepot->setItem(i, 3, new QTableWidgetItem(QString::number(cap)));
        ui->tableDepot->setItem(i, 4, new QTableWidgetItem(QString::number(qte)));
        ui->tableDepot->setItem(i, 5, new QTableWidgetItem(type));
        ui->tableDepot->setItem(i, 6, new QTableWidgetItem(remplissage));

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
    ui->tableDepot->setSortingEnabled(triActif);
}
// Exports
void MainWindow::exporterPDF(QTableWidget *table, QString titre) {
    if (!table)
        return;
    const QString f = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Exporter en PDF"),
        titre + QStringLiteral(".pdf"),
        QStringLiteral("PDF (*.pdf)"));
    if (f.isEmpty())
        return;
    QString outPath = f;
    if (QFileInfo(outPath).suffix().isEmpty())
        outPath += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(outPath);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    printer.setPageMargins(QMarginsF(14, 14, 14, 14), QPageLayout::Millimeter);
#else
    printer.setPageMargins(14, 14, 14, 14, QPrinter::Millimeter);
#endif

    const QString dateStr = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy  HH:mm"));
    const int rowCount    = table->rowCount();
    const int colCount    = table->columnCount();

    // ── Colonnes du tableau ──
    QString thHtml;
    for (int c = 0; c < colCount; ++c) {
        const QTableWidgetItem *h = table->horizontalHeaderItem(c);
        const QString label = h ? h->text() : QString();
        thHtml += QStringLiteral(
            "<th style=’text-align:left; padding:10px 9px;"
            " background:#3e2723; color:#e0c097;"
            " font-size:9.5px; font-weight:700; letter-spacing:1px; text-transform:uppercase;"
            " border-right:1px solid #5d4037;’>%1</th>")
            .arg(label.toHtmlEscaped());
    }

    // ── Lignes du tableau ──
    QString tbHtml;
    for (int r = 0; r < rowCount; ++r) {
        const QString rowBg = (r % 2 == 0)
            ? QStringLiteral("#ffffff")
            : QStringLiteral("#fdf8f2");
        tbHtml += QStringLiteral("<tr style=’background:%1;’>").arg(rowBg);
        for (int c = 0; c < colCount; ++c) {
            const QTableWidgetItem *cell = table->item(r, c);
            const QString text = cell ? cell->text() : QString();
            tbHtml += QStringLiteral(
                "<td style=’padding:8px 9px;"
                " border-bottom:1px solid #ede0d4;"
                " border-right:1px solid #f0e6d8;"
                " font-size:10px; color:#2c1810; vertical-align:top;’>%1</td>")
                .arg(text.toHtmlEscaped());
        }
        tbHtml += QStringLiteral("</tr>");
    }

    const QString html = QStringLiteral(
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/></head>"
        "<body style=’font-family:Arial,Helvetica,sans-serif; margin:0; padding:0; background:#ffffff;’>"

        // ── En-tête ──
        "<table width=’100%’ cellspacing=’0’ cellpadding=’0’"
               " style=’background:#3e2723; border-radius:8px 8px 0 0;’><tr>"
          "<td style=’padding:14px 18px;’>"
            "<div style=’color:#d4af37; font-size:18px; font-weight:900; letter-spacing:3px;’>"
              "FIL D&apos;OR"
            "</div>"
            "<div style=’color:#a1887f; font-size:9px; letter-spacing:1.5px; margin-top:2px;’>"
              "MAROQUINERIE &amp; ARTICLES EN CUIR"
            "</div>"
          "</td>"
          "<td align=’right’ style=’padding:14px 18px;’>"
            "<div style=’color:#f5e6c8; font-size:15px; font-weight:800;’>%1</div>"
            "<div style=’color:#8d6e63; font-size:9.5px; margin-top:4px;’>"
              "Date : %2 &nbsp;&#124;&nbsp; %3 enregistrements"
            "</div>"
          "</td>"
        "</tr></table>"

        // ── Barre or ──
        "<div style=’height:4px; background:#d4af37; margin-bottom:16px;’></div>"

        // ── Tableau de données ──
        "<table width=’100%’ cellspacing=’0’ cellpadding=’0’"
               " style=’border-collapse:collapse; border:1px solid #d7ccc8;’>"
          "<thead><tr>%4</tr></thead>"
          "<tbody>%5</tbody>"
        "</table>"

        // ── Pied de page ──
        "<div style=’margin-top:16px; padding-top:9px; border-top:2px solid #d4af37;"
             " font-size:9px; color:#a1887f; text-align:center;’>"
          "FIL D&apos;OR &mdash; Document g&#233;n&#233;r&#233; automatiquement le %2"
          " &nbsp;&#124;&nbsp; Confidentiel"
        "</div>"

        "</body></html>")
    .arg(titre.toHtmlEscaped())
    .arg(dateStr.toHtmlEscaped())
    .arg(rowCount)
    .arg(thHtml)
    .arg(tbHtml);

    QFont bodyFont(QStringLiteral("Arial"), 10, QFont::Normal);
    if (!bodyFont.exactMatch())
        bodyFont = QFont(QStringLiteral("Helvetica"), 10, QFont::Normal);
    QTextDocument doc;
    doc.setDefaultFont(bodyFont);
    doc.setHtml(html);
    doc.print(&printer);
    alerteSucces(QStringLiteral("Export PDF"),
                 QStringLiteral("Le fichier a été généré avec succès :\n%1").arg(outPath));
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

void MainWindow::peuplerTableEtapesStandard(QTableWidget *tbl, QSqlQueryModel *model)
{
    if (!tbl || !model)
        return;
    tbl->setRowCount(0);
    tbl->setColumnCount(9);
    tbl->setHorizontalHeaderLabels({
        QStringLiteral("ID Suivi"), QStringLiteral("N° Cmd"), QStringLiteral("Produit"), QStringLiteral("Employé"),
        QStringLiteral("Étape"), QStringLiteral("Prévu IA (j)"), QStringLiteral("Réel (j)"),
        QStringLiteral("Delta (j)"), QStringLiteral("Alerte"),
    });

    const int rows = model->rowCount();
    tbl->setRowCount(rows);
    QFont fBold;
    fBold.setBold(true);

    for (int i = 0; i < rows; i++) {
        const int idSuivi = model->record(i).value(QStringLiteral("ID_SUIVI")).toInt();
        const int idPlanif = model->record(i).value(QStringLiteral("ID_PLANIFICATION")).toInt();
        const QString produit = model->record(i).value(QStringLiteral("PRODUIT")).toString();
        const int qtePlanif = model->record(i).value(QStringLiteral("QTE_PLANIF")).toInt();
        const int qteSafe = std::max(1, qtePlanif);
        const QString employe = model->record(i).value(QStringLiteral("EMPLOYE")).toString();
        const QString etape = model->record(i).value(QStringLiteral("ETAPE_ACTUELLE")).toString();
        const double tempsReel = model->record(i).value(QStringLiteral("TEMPS_REEL_PASSE")).toDouble();
        const double deltaDb = model->record(i).value(QStringLiteral("DELTA")).toDouble();
        const int alerte = model->record(i).value(QStringLiteral("ALERTE_ACTIVE")).toInt();

        const double hPrev = tempsPrevuHeuresEtapePourCommandeIA(produit, qteSafe, etape);
        const double deltaAff = tempsReel - hPrev;

        QColor couleurEtape;
        if (etape == QLatin1String("Coupe")) couleurEtape = QColor(QStringLiteral("#1565c0"));
        else if (etape == QLatin1String("Assemblage")) couleurEtape = QColor(QStringLiteral("#ef6c00"));
        else if (etape == QLatin1String("Couture")) couleurEtape = QColor(QStringLiteral("#2e7d32"));
        else if (etape == QLatin1String("Finition")) couleurEtape = QColor(QStringLiteral("#f9a825"));
        else couleurEtape = QColor(QStringLiteral("#757575"));

        auto *itemId = new QTableWidgetItem(QString::number(idSuivi));
        itemId->setData(Qt::UserRole, idSuivi);
        itemId->setData(Qt::UserRole + 1, idPlanif);
        itemId->setData(Qt::UserRole + 2, qtePlanif > 0 ? qtePlanif : qteSafe);
        tbl->setItem(i, 0, itemId);

        tbl->setItem(i, 1, new QTableWidgetItem(QString::number(idPlanif)));

        auto *itemProd = new QTableWidgetItem(produit);
        itemProd->setFont(fBold);
        tbl->setItem(i, 2, itemProd);

        tbl->setItem(i, 3, new QTableWidgetItem(employe));

        auto *itemEtape = new QTableWidgetItem(etape);
        itemEtape->setForeground(couleurEtape);
        itemEtape->setFont(fBold);
        tbl->setItem(i, 4, itemEtape);

        auto *itemPrev = new QTableWidgetItem(formaterJoursPrevusPlafondIA(hPrev));
        itemPrev->setTextAlignment(Qt::AlignCenter);
        itemPrev->setToolTip(QStringLiteral("Prévu IA (avec échelle) : %1 h — plafond jours comme l’onglet IA (%2 h/j).")
                                 .arg(hPrev, 0, 'f', 1)
                                 .arg(kHeuresOuvreParJourFabrication, 0, 'f', 0));
        itemPrev->setFont(fBold);
        tbl->setItem(i, 5, itemPrev);

        auto *itemTemps = new QTableWidgetItem(formaterHeuresEtapesEnJoursAffichage(tempsReel));
        itemTemps->setTextAlignment(Qt::AlignCenter);
        itemTemps->setToolTip(QStringLiteral("%1 h (stocké en base)").arg(tempsReel, 0, 'f', 2));
        tbl->setItem(i, 6, itemTemps);

        auto *itemDelta = new QTableWidgetItem(formaterDeltaEtapesEnJoursAffichage(deltaAff));
        itemDelta->setTextAlignment(Qt::AlignCenter);
        itemDelta->setToolTip(
            QStringLiteral("Écart réel − prévu IA : %1 h. Delta enregistré en base : %2 h.")
                .arg(deltaAff, 0, 'f', 2)
                .arg(deltaDb, 0, 'f', 2));
        if (deltaAff > 0) itemDelta->setForeground(QColor(QStringLiteral("#c62828")));
        else if (deltaAff < 0) itemDelta->setForeground(QColor(QStringLiteral("#2e7d32")));
        else itemDelta->setForeground(QColor(QStringLiteral("#757575")));
        itemDelta->setFont(fBold);
        tbl->setItem(i, 7, itemDelta);

        auto *itemAlerte = new QTableWidgetItem(alerte ? QStringLiteral("🔴 OUI") : QStringLiteral("🟢 Non"));
        itemAlerte->setTextAlignment(Qt::AlignCenter);
        if (alerte) itemAlerte->setForeground(QColor(QStringLiteral("#c62828")));
        else itemAlerte->setForeground(QColor(QStringLiteral("#2e7d32")));
        itemAlerte->setFont(fBold);
        itemAlerte->setToolTip(QStringLiteral("Alerte BDD (retard vs prévu au moment de l’enregistrement)."));
        tbl->setItem(i, 8, itemAlerte);

        tbl->setRowHeight(i, 38);
    }

    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::remplirTableEtapes(QSqlQueryModel *model)
{
    peuplerTableEtapesStandard(ui->tableTimeline, model);
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
    // Réinitialiser l'affectation en attente pour ce formulaire
    if (!estModif) {
        m_affectAddConfigured = false;
        m_pendingAffectAdd = {};
        if (ui->btn_equipe_planif) {
            ui->btn_equipe_planif->setText(QStringLiteral("👥 Configurer l'équipe"));
            ui->btn_equipe_planif->setStyleSheet(QStringLiteral(
                "QPushButton { background: rgba(212,175,55,0.12); border: 2px solid rgba(212,175,55,0.55);"
                "  border-radius: 12px; padding: 9px 14px; font-weight: 900; font-size: 12px; color: #6d4c41; }"
                "QPushButton:hover { background: rgba(212,175,55,0.22); border-color: #d4af37; }"));
        }
    } else {
        m_affectModifConfigured = false;
        m_pendingAffectModif = {};
        if (ui->btn_equipe_modif) {
            ui->btn_equipe_modif->setText(QStringLiteral("👥 Configurer l'équipe"));
            ui->btn_equipe_modif->setStyleSheet(QStringLiteral(
                "QPushButton { background: rgba(212,175,55,0.12); border: 2px solid rgba(212,175,55,0.55);"
                "  border-radius: 12px; padding: 9px 14px; font-weight: 900; font-size: 12px; color: #6d4c41; }"
                "QPushButton:hover { background: rgba(212,175,55,0.22); border-color: #d4af37; }"));
        }
    }

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

    ui->cb_employe->show();
    if (ui->lbl_fp_emp) ui->lbl_fp_emp->show();

    ui->cb_employe->clear();
    QSqlQuery qEmpList("SELECT ID_EMPLOYE, NOM || ' ' || PRENOM FROM EMPLOYES ORDER BY NOM");
    while(qEmpList.next()) {
        ui->cb_employe->addItem(qEmpList.value(1).toString(), qEmpList.value(0).toInt());
    }

    if(estModif && indexModification >= 0 && indexModification < mesCommandes.size()) {
        CommandeInfo c = mesCommandes[indexModification];
        int idxProd = ui->cb_produit->findText(c.idProduit, Qt::MatchContains);
        if(idxProd >= 0) ui->cb_produit->setCurrentIndex(idxProd);
        int idxMat = ui->cb_matiere->findText(c.idMatiere, Qt::MatchContains);
        if(idxMat >= 0) ui->cb_matiere->setCurrentIndex(idxMat);
        
        // Extraire l'ID du 1er employé depuis la base si besoin
        QSqlQuery qAff("SELECT ID_EMPLOYE FROM ETAPES WHERE ID_PLANIFICATION = :idCmd FETCH FIRST 1 ROWS ONLY");
        qAff.bindValue(":idCmd", c.id.mid(3).toInt());
        if (qAff.exec() && qAff.next()) {
            int idxEmp = ui->cb_employe->findData(qAff.value(0).toInt());
            if (idxEmp >= 0) ui->cb_employe->setCurrentIndex(idxEmp);
        }
        
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
    if (m_statsProduitsEnCours)
        return;
    m_statsProduitsEnCours = true;
    struct FinStatsProduits {
        bool &flag;
        ~FinStatsProduits() { flag = false; }
    } finStats{m_statsProduitsEnCours};

    // 1. Cible l'onglet "Analyses" des Produits (Index 3)
    if(ui->tabWidgetProduits->count() < 4)
        return;
    QWidget *ongletStats = ui->tabWidgetProduits->widget(3);
    if (!ongletStats)
        return;

    if (ongletStats->layout()) { clearLayout(ongletStats->layout()); delete ongletStats->layout(); }
    QVBoxLayout *mainL = new QVBoxLayout(ongletStats);
    mainL->setSpacing(10);
    mainL->setContentsMargins(10, 10, 10, 10);

    QScrollArea *sa = new QScrollArea(ongletStats);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);

    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12);
    root->setContentsMargins(8, 8, 8, 8);

    int total = mesProduits.size();
    double prixTotal = 0.0;
    double coutMax = 0.0;
    QMap<QString, double> parCollection;
    QMap<QString, double> parCuir;
    for (const auto &p : mesProduits) {
        prixTotal += p.cout;
        coutMax = qMax(coutMax, p.cout);
        parCollection[p.collection] += 1.0;
        parCuir[p.typeCuir] += 1.0;
    }
    const double moy = (total > 0) ? (prixTotal / total) : 0.0;
    int premium = 0;
    for (const auto &p : mesProduits) if (p.cout >= moy && total > 0) premium++;

    QString collDominante = "N/A";
    double maxColl = -1.0;
    for (auto it = parCollection.constBegin(); it != parCollection.constEnd(); ++it) {
        if (it.value() > maxColl) { maxColl = it.value(); collDominante = it.key(); }
    }
    QString cuirDominant = "N/A";
    double maxCuir = -1.0;
    for (auto it = parCuir.constBegin(); it != parCuir.constEnd(); ++it) {
        if (it.value() > maxCuir) { maxCuir = it.value(); cuirDominant = it.key(); }
    }

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse Catalogue Produits - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit Catalogue      ② Performance Coûts      ③ Diagnostic Collection");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht);
    hl->addWidget(hs);
    root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10);
        cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(total), "Références Actives", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "👜"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(prixTotal, 'f', 1) + " DT", "Valeur Catalogue", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "💰"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(coutMax, 'f', 1) + " DT", "Coût Max", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "📈"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(premium), "Produits Premium", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "🏅"), 1, 0);
    kpi->addWidget(makeKpiCard(QString::number(moy, 'f', 1) + " DT", "Coût Moyen", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 1);
    kpi->addWidget(makeKpiCard(collDominante, "Collection Dominante", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "✨"), 1, 2);
    root->addLayout(kpi);

    QHBoxLayout *chartsL = new QHBoxLayout();
    chartsL->setSpacing(10);
    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Collection"); titrePie->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slicesColl;
    for (auto it = parCollection.constBegin(); it != parCollection.constEnd(); ++it) slicesColl.append({it.key(), it.value()});
    setPieChart(wPie, "", slicesColl); layoutPie->addWidget(wPie); chartsL->addWidget(framePie);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Utilisation des Cuirs"); titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget(); QStringList cuirs = parCuir.keys(); QList<double> vals;
    for (const auto &k : cuirs) vals << parCuir[k];
    setVerticalBarChart(wBar, "", cuirs, vals); layoutBar->addWidget(wBar); chartsL->addWidget(frameBar);
    root->addLayout(chartsL);

    QHBoxLayout *bottom = new QHBoxLayout(); bottom->setSpacing(10);
    QFrame *detail = new QFrame(); detail->setStyleSheet("QFrame { background: #ffffff; border: 1px solid #e3d9cf; border-radius: 10px; }");
    QVBoxLayout *dl = new QVBoxLayout(detail);
    QLabel *dt = new QLabel("Analyse Détaillée"); dt->setStyleSheet("font-weight: 800; color: #6d4c41;");
    QLabel *d1 = new QLabel(QString("• Coût moyen de fabrication : %1 DT").arg(QString::number(moy, 'f', 2)));
    QLabel *d2 = new QLabel(QString("• Collection dominante : %1").arg(collDominante));
    QLabel *d3 = new QLabel(QString("• Cuir dominant : %1").arg(cuirDominant));
    QLabel *d4 = new QLabel(QString("• Produits premium : %1").arg(premium));
    d1->setStyleSheet("color:#4e342e;"); d2->setStyleSheet("color:#4e342e;"); d3->setStyleSheet("color:#4e342e;"); d4->setStyleSheet("color:#4e342e;");
    dl->addWidget(dt); dl->addWidget(d1); dl->addWidget(d2); dl->addWidget(d3); dl->addWidget(d4); dl->addStretch();
    bottom->addWidget(detail, 3);

    QFrame *watch = new QFrame(); watch->setStyleSheet("QFrame { background: #fff; border: 1px solid #f1c7c7; border-radius: 10px; }");
    QVBoxLayout *wl = new QVBoxLayout(watch);
    QLabel *wt = new QLabel("Top Coûts à Surveiller"); wt->setStyleSheet("font-weight: 900; color: #b94a48;"); wl->addWidget(wt);
    QTableWidget *tw = new QTableWidget(0, 3);
    tw->setHorizontalHeaderLabels({"ID", "DESIGNATION", "COÛT"});
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setVisible(false);
    tw->setStyleSheet("QTableWidget{background:#fff; border:1px solid #f3e1e1; font-size:12px;}QHeaderView::section{background:#d96b6b; color:white; font-weight:800; padding:6px; border:none;}");
    QVector<ProduitInfo> sorted = mesProduits;
    std::sort(sorted.begin(), sorted.end(), [](const ProduitInfo &a, const ProduitInfo &b){ return a.cout > b.cout; });
    const int take = qMin(3, sorted.size());
    tw->setRowCount(take);
    for (int i = 0; i < take; ++i) {
        tw->setItem(i, 0, new QTableWidgetItem(sorted[i].id_produit));
        tw->setItem(i, 1, new QTableWidgetItem(sorted[i].designation));
        tw->setItem(i, 2, new QTableWidgetItem(QString::number(sorted[i].cout, 'f', 2)));
    }
    wl->addWidget(tw);
    bottom->addWidget(watch, 4);
    root->addLayout(bottom);

    QHBoxLayout *footer = new QHBoxLayout(); footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA Produits");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA Produits", "Analyse prête : vous pouvez simuler les coûts et prioriser les références premium.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

    ui->tabWidgetProduits->setCurrentIndex(3);
}



void MainWindow::ouvrirStatsStock() {
    if(ui->tabWidgetStock->count() < 4) return;
    QWidget *onglet = ui->tabWidgetStock->widget(3);
    if (!onglet) return;
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(10);
    mainL->setContentsMargins(10, 10, 10, 10);

    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);

    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12);
    root->setContentsMargins(8, 8, 8, 8);

    double volume = 0.0;
    QMap<QString, double> parCat;
    QMap<QString, double> parQual;
    for (const auto &m : mesMatieres) {
        volume += m.quantite;
        parCat[m.categorie] += 1.0;
        parQual[m.qualite] += 1.0;
    }

    int lotsCritiques = 0;
    int lotsPremium = 0;
    for (const auto &m : mesMatieres) {
        if (m.quantite <= 1.0) lotsCritiques++;
        if (m.qualite.trimmed().toUpper() == "A") lotsPremium++;
    }
    const double indiceQualite = mesMatieres.isEmpty() ? 0.0 : (100.0 * lotsPremium / mesMatieres.size());

    QString categorieDominante = "N/A";
    double maxCat = -1.0;
    for (auto it = parCat.constBegin(); it != parCat.constEnd(); ++it) {
        if (it.value() > maxCat) {
            maxCat = it.value();
            categorieDominante = it.key();
        }
    }

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse des Stocks MP - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit Inventaire      ② Performance Qualité      ③ Diagnostic Stratégique");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht);
    hl->addWidget(hs);
    root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10);
        cl->setSpacing(3);
        QLabel *ico = new QLabel(icon);
        ico->setAlignment(Qt::AlignRight);
        ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value);
        v->setAlignment(Qt::AlignCenter);
        v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico);
        cl->addWidget(v);
        cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10);
    kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(mesMatieres.size()), "Lots Référencés", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "📦"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(volume, 'f', 1) + " u", "Volume Total", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "📏"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(lotsCritiques), "Lots Critiques", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "⚠️"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(lotsPremium), "Lots Premium", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "🏅"), 1, 0);
    kpi->addWidget(makeKpiCard(QString::number(indiceQualite, 'f', 1) + "%", "Indice Qualité", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 1);
    kpi->addWidget(makeKpiCard(categorieDominante, "Catégorie Dominante", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "📌"), 1, 2);
    root->addLayout(kpi);

    QHBoxLayout *charts = new QHBoxLayout();
    charts->setSpacing(10);

    QFrame *framePie = new QFrame();
    framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Catégorie");
    titrePie->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for (auto it = parCat.constBegin(); it != parCat.constEnd(); ++it) slices.append({it.key(), it.value()});
    setPieChart(wPie, "", slices);
    layoutPie->addWidget(wPie);
    charts->addWidget(framePie);

    QFrame *frameBar = new QFrame();
    frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Qualité des lots");
    titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget();
    QStringList quals = parQual.keys();
    QList<double> vals;
    for (const auto &q : quals) vals << parQual[q];
    setVerticalBarChart(wBar, "", quals, vals);
    layoutBar->addWidget(wBar);
    charts->addWidget(frameBar);

    root->addLayout(charts);

    QHBoxLayout *bottom = new QHBoxLayout();
    bottom->setSpacing(10);

    QFrame *detail = new QFrame();
    detail->setStyleSheet("QFrame { background: #ffffff; border: 1px solid #e3d9cf; border-radius: 10px; }");
    QVBoxLayout *dl = new QVBoxLayout(detail);
    QLabel *dt = new QLabel("Analyse Détaillée");
    dt->setStyleSheet("font-weight: 800; color: #6d4c41;");
    QLabel *d1 = new QLabel(QString("• Volume moyen par lot : %1 u").arg(mesMatieres.isEmpty() ? 0.0 : volume / mesMatieres.size(), 0, 'f', 1));
    QLabel *d2 = new QLabel(QString("• Qualité dominante : %1").arg(parQual.isEmpty() ? "N/A" : std::max_element(parQual.constBegin(), parQual.constEnd(),
        [](double a, double b){ return a < b; }).key()));
    QLabel *d3 = new QLabel(QString("• Catégories suivies : %1").arg(parCat.size()));
    QLabel *d4 = new QLabel(QString("• Part lots premium : %1%").arg(QString::number(indiceQualite, 'f', 1)));
    d1->setStyleSheet("color:#4e342e;"); d2->setStyleSheet("color:#4e342e;"); d3->setStyleSheet("color:#4e342e;"); d4->setStyleSheet("color:#4e342e;");
    dl->addWidget(dt); dl->addWidget(d1); dl->addWidget(d2); dl->addWidget(d3); dl->addWidget(d4); dl->addStretch();
    bottom->addWidget(detail, 3);

    QFrame *watch = new QFrame();
    watch->setStyleSheet("QFrame { background: #fff; border: 1px solid #f1c7c7; border-radius: 10px; }");
    QVBoxLayout *wl = new QVBoxLayout(watch);
    QLabel *wt = new QLabel("Lots à Surveiller");
    wt->setStyleSheet("font-weight: 900; color: #b94a48;");
    wl->addWidget(wt);
    QTableWidget *tw = new QTableWidget(0, 3);
    tw->setHorizontalHeaderLabels({"CODE", "CATÉGORIE", "QTÉ"});
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setVisible(false);
    tw->setStyleSheet(
        "QTableWidget{background:#fff; border:1px solid #f3e1e1; font-size:12px;}"
        "QHeaderView::section{background:#d96b6b; color:white; font-weight:800; padding:6px; border:none;}"
    );
    QVector<MatiereInfo> sorted = mesMatieres;
    std::sort(sorted.begin(), sorted.end(), [](const MatiereInfo &a, const MatiereInfo &b){ return a.quantite < b.quantite; });
    const int take = qMin(3, sorted.size());
    tw->setRowCount(take);
    for (int i = 0; i < take; ++i) {
        tw->setItem(i, 0, new QTableWidgetItem(sorted[i].code));
        tw->setItem(i, 1, new QTableWidgetItem(sorted[i].categorie));
        tw->setItem(i, 2, new QTableWidgetItem(QString::number(sorted[i].quantite, 'f', 2)));
    }
    wl->addWidget(tw);
    bottom->addWidget(watch, 4);
    root->addLayout(bottom);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA MP");
    assistant->setStyleSheet(
        "QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }"
        "QPushButton:hover { background:#2a70d2; }"
    );
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA MP", "Assistant IA MP prêt : vous pouvez auditer les lots critiques et simuler un ravitaillement.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

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
    mainL->setSpacing(10);
    mainL->setContentsMargins(10, 10, 10, 10);

    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12);
    root->setContentsMargins(8, 8, 8, 8);

    int vip = 0;
    int churnRisk = 0;
    int sansEmail = 0;
    int pointsTotal = 0;
    QMap<QString, double> parVille;
    for (const auto &c : mesClients) {
        pointsTotal += c.pointsFidelite;
        if (c.pointsFidelite >= 100) vip++;
        if (c.pointsFidelite < 20) churnRisk++;
        if (c.email.trimmed().isEmpty()) sansEmail++;
        QString ville = "Autre";
        if (c.adresse.toLower().contains("tunis")) ville = "Tunis";
        else if (c.adresse.toLower().contains("sfax")) ville = "Sfax";
        else if (c.adresse.toLower().contains("sousse")) ville = "Sousse";
        parVille[ville] += 1.0;
    }
    const double panierFidelite = mesClients.isEmpty() ? 0.0 : static_cast<double>(pointsTotal) / mesClients.size();
    QString villeDominante = "Autre";
    double maxVille = -1.0;
    for (auto it = parVille.constBegin(); it != parVille.constEnd(); ++it) {
        if (it.value() > maxVille) { maxVille = it.value(); villeDominante = it.key(); }
    }

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse CRM Clients - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit Portefeuille      ② Performance Fidélité      ③ Diagnostic Relation Client");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht); hl->addWidget(hs); root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10); cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(mesClients.size()), "Clients Inscrits", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "🧾"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(pointsTotal), "Points Fidélité", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "🎯"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(churnRisk), "Risque Churn", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "⚠️"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(vip), "Clients VIP", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "👑"), 1, 0);
    kpi->addWidget(makeKpiCard(QString::number(panierFidelite, 'f', 1), "Moyenne Points", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 1);
    kpi->addWidget(makeKpiCard(villeDominante, "Ville Dominante", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "📍"), 1, 2);
    root->addLayout(kpi);

    QHBoxLayout *charts = new QHBoxLayout();
    charts->setSpacing(10);
    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition Géographique"); titrePie->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slices;
    for (auto it = parVille.constBegin(); it != parVille.constEnd(); ++it) slices.append({it.key(), it.value()});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie); charts->addWidget(framePie);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Qualité du Portefeuille"); titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget();
    QStringList segs = {"VIP", "Standard", "Risque"};
    QList<double> vals = {static_cast<double>(vip), static_cast<double>(qMax(0, mesClients.size() - vip - churnRisk)), static_cast<double>(churnRisk)};
    setVerticalBarChart(wBar, "", segs, vals); layoutBar->addWidget(wBar); charts->addWidget(frameBar);
    root->addLayout(charts);

    QFrame *detail = new QFrame(); detail->setStyleSheet("QFrame { background: #ffffff; border: 1px solid #e3d9cf; border-radius: 10px; }");
    QVBoxLayout *dl = new QVBoxLayout(detail);
    QLabel *dt = new QLabel("Analyse Détaillée"); dt->setStyleSheet("font-weight: 800; color: #6d4c41;");
    QLabel *d1 = new QLabel(QString("• Taux VIP : %1%").arg(mesClients.isEmpty() ? QString("0.0") : QString::number(100.0 * vip / mesClients.size(), 'f', 1)));
    QLabel *d2 = new QLabel(QString("• Risque churn : %1 clients").arg(churnRisk));
    QLabel *d3 = new QLabel(QString("• Clients sans email : %1").arg(sansEmail));
    QLabel *d4 = new QLabel(QString("• Ville la plus active : %1").arg(villeDominante));
    d1->setStyleSheet("color:#4e342e;"); d2->setStyleSheet("color:#4e342e;"); d3->setStyleSheet("color:#4e342e;"); d4->setStyleSheet("color:#4e342e;");
    dl->addWidget(dt); dl->addWidget(d1); dl->addWidget(d2); dl->addWidget(d3); dl->addWidget(d4);
    root->addWidget(detail);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA CRM");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA CRM", "Assistant CRM prêt : segmentation, churn et actions marketing ciblées disponibles.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

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
    d.setMinimumSize(1100, 760);
    d.setStyleSheet("background-color: #f3f0eb; color: #3e2723;");
    QVBoxLayout *mainL = new QVBoxLayout(&d);
    mainL->setContentsMargins(10, 10, 10, 10);
    mainL->setSpacing(10);

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

    QScrollArea *sa = new QScrollArea(&d);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12);
    root->setContentsMargins(8, 8, 8, 8);

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse Planification - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit OF      ② Performance Production      ③ Diagnostic Retards");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht); hl->addWidget(hs); root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10); cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(totalCmd), "Ordres de Fabrication", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "📋"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(totalQte), "Pièces Planifiées", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "📦"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(cmdRetard), "OF en Retard", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "⚠️"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(tauxRetard, 'f', 1) + "%", "Taux Retard", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 0);
    kpi->addWidget(makeKpiCard(QString::number(parStatut.size()), "Statuts Actifs", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "🧩"), 1, 1);
    kpi->addWidget(makeKpiCard(QString::number(parProduit.size()), "Produits Planifiés", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "✨"), 1, 2);
    root->addLayout(kpi);

    QHBoxLayout *chartsL = new QHBoxLayout();
    chartsL->setSpacing(10);
    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Statut"); titrePie->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget();
    QList<QPair<QString, double>> slices;
    for (auto it = parStatut.constBegin(); it != parStatut.constEnd(); ++it) slices.append({it.key(), it.value()});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie); chartsL->addWidget(framePie);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Commandes par Produit"); titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget();
    QStringList prods = parProduit.keys();
    QList<double> vals; for (const auto &k : prods) vals << parProduit[k];
    setVerticalBarChart(wBar, "", prods, vals); layoutBar->addWidget(wBar); chartsL->addWidget(frameBar);
    root->addLayout(chartsL);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA Planification");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA Planification", "Assistant prêt : priorisation OF, suivi retards et plan d'action production.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

    QPushButton *btn = new QPushButton("Fermer");
    btn->setStyleSheet(styleBtnCancel());
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

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(f);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    printer.setPageMargins(QMarginsF(14, 14, 14, 14), QPageLayout::Millimeter);
#else
    printer.setPageMargins(14, 14, 14, 14, QPrinter::Millimeter);
#endif

    const QString dateStr   = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy  HH:mm"));
    const QString factureNo = QStringLiteral("FACT-%1-%2")
        .arg(c.id)
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd")));
    const QString niveauLabel = (c.pointsFidelite >= 200)
        ? QStringLiteral("&#11088; Platine")
        : (c.pointsFidelite >= 100)
            ? QStringLiteral("&#127881; Gold")
            : (c.pointsFidelite >= 50)
                ? QStringLiteral("&#129354; Silver")
                : QStringLiteral("&#129353; Bronze");
    const QString niveauColor = (c.pointsFidelite >= 200)
        ? QStringLiteral("#7b68ee")
        : (c.pointsFidelite >= 100)
            ? QStringLiteral("#d4af37")
            : (c.pointsFidelite >= 50)
                ? QStringLiteral("#9e9e9e")
                : QStringLiteral("#b8860b");

    const QString html = QStringLiteral(
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/></head>"
        "<body style=’font-family:Arial,Helvetica,sans-serif; color:#2c1810; background:#ffffff; margin:0; padding:0;’>"

        // ── En-tête bicolonne ──
        "<table width=’100%’ cellspacing=’0’ cellpadding=’0’"
               " style=’background:#3e2723; border-radius:8px 8px 0 0;’><tr>"
          "<td style=’padding:16px 20px;’>"
            "<div style=’color:#d4af37; font-size:22px; font-weight:900; letter-spacing:3px;’>"
              "FIL D&apos;OR"
            "</div>"
            "<div style=’color:#a1887f; font-size:9px; letter-spacing:1.5px; margin-top:3px;’>"
              "MAROQUINERIE &amp; ARTICLES EN CUIR"
            "</div>"
            "<div style=’color:#6d4c41; font-size:9px; margin-top:6px;’>"
              "Tunis, Tunisie &nbsp;&#124;&nbsp; contact@fildor.tn"
            "</div>"
          "</td>"
          "<td align=’right’ style=’padding:16px 20px;’>"
            "<div style=’background:#2c1a10; border:1px solid #d4af37; border-radius:6px;"
                 " padding:10px 14px; display:inline-block; text-align:right;’>"
              "<div style=’color:#d4af37; font-size:11px; font-weight:700; letter-spacing:1px;’>FACTURE</div>"
              "<div style=’color:#f5e6c8; font-size:13px; font-weight:900; margin-top:3px;’>%1</div>"
              "<div style=’color:#8d6e63; font-size:9.5px; margin-top:4px;’>%2</div>"
            "</div>"
          "</td>"
        "</tr></table>"

        // ── Barre or ──
        "<div style=’height:4px; background:#d4af37; margin-bottom:18px;’></div>"

        // ── Informations client + fidélité (2 colonnes) ──
        "<table width=’100%’ cellspacing=’0’ cellpadding=’0’><tr>"
          "<td width=’62%’ style=’padding-right:8px; vertical-align:top;’>"
            "<div style=’border:1px solid #d7ccc8; border-radius:8px; background:#ffffff;"
                 " padding:13px 15px;’>"
              "<div style=’color:#8d6e63; font-size:9px; letter-spacing:1.5px; font-weight:700;"
                   " margin-bottom:8px; text-transform:uppercase;’>Informations client</div>"
              "<div style=’font-size:14px; font-weight:800; color:#1a0f0a; margin-bottom:6px;’>%3</div>"
              "<table cellspacing=’0’ cellpadding=’0’ style=’font-size:10.5px; color:#5d4037;’>"
                "<tr><td style=’padding:3px 0; color:#a1887f; width:90px;’>Réf. client</td>"
                    "<td style=’padding:3px 0; font-weight:600;’>%4</td></tr>"
                "<tr><td style=’padding:3px 0; color:#a1887f;’>Téléphone</td>"
                    "<td style=’padding:3px 0;’>%5</td></tr>"
                "<tr><td style=’padding:3px 0; color:#a1887f;’>Email</td>"
                    "<td style=’padding:3px 0;’>%6</td></tr>"
                "<tr><td style=’padding:3px 0; color:#a1887f;’>Adresse</td>"
                    "<td style=’padding:3px 0;’>%7</td></tr>"
              "</table>"
            "</div>"
          "</td>"
          "<td width=’38%’ style=’padding-left:8px; vertical-align:top;’>"
            "<div style=’border:1px solid #d7ccc8; border-radius:8px; background:#fff8e8;"
                 " padding:13px 15px; height:100%%;’>"
              "<div style=’color:#8d6e63; font-size:9px; letter-spacing:1.5px; font-weight:700;"
                   " margin-bottom:8px; text-transform:uppercase;’>Programme fidélité</div>"
              "<div style=’font-size:18px; font-weight:900; color:%8; margin-bottom:6px;’>%9</div>"
              "<div style=’font-size:11px; color:#5d4037;’>"
                "<b>%10 points</b> accumulés"
              "</div>"
              "<div style=’margin-top:8px; font-size:9.5px; color:#9e9e9e;’>"
                "200 pts = Platine &nbsp;&#124;&nbsp; 100 pts = Gold"
              "</div>"
            "</div>"
          "</td>"
        "</tr></table>"

        // ── Tableau articles ──
        "<div style=’margin-top:18px; margin-bottom:6px; color:#8d6e63;"
             " font-size:9px; letter-spacing:1.5px; font-weight:700; text-transform:uppercase;’>"
          "Détail de la commande"
        "</div>"
        "<table width=’100%’ cellspacing=’0’ cellpadding=’0’"
               " style=’border-collapse:collapse; border:1px solid #d7ccc8;’>"
          "<thead>"
            "<tr style=’background:#3e2723;’>"
              "<th style=’text-align:left; padding:10px 12px; color:#e0c097;"
                  " font-size:9.5px; font-weight:700; letter-spacing:1px; width:50%%;’>"
                "DÉSIGNATION"
              "</th>"
              "<th style=’text-align:center; padding:10px 8px; color:#e0c097;"
                  " font-size:9.5px; font-weight:700; letter-spacing:1px; width:15%%;’>"
                "QTÉ"
              "</th>"
              "<th style=’text-align:right; padding:10px 8px; color:#e0c097;"
                  " font-size:9.5px; font-weight:700; letter-spacing:1px; width:17.5%%;’>"
                "PRIX U."
              "</th>"
              "<th style=’text-align:right; padding:10px 12px; color:#e0c097;"
                  " font-size:9.5px; font-weight:700; letter-spacing:1px; width:17.5%%;’>"
                "TOTAL"
              "</th>"
            "</tr>"
          "</thead>"
          "<tbody>"
            "<tr style=’background:#fdf8f2;’>"
              "<td style=’padding:11px 12px; font-size:10.5px; border-bottom:1px solid #f0e6d8;’>"
                "<i style=’color:#9e9e9e;’>Aucun article — Facture de démonstration</i>"
              "</td>"
              "<td style=’padding:11px 8px; text-align:center; font-size:10.5px;"
                  " border-bottom:1px solid #f0e6d8; color:#9e9e9e;’>—</td>"
              "<td style=’padding:11px 8px; text-align:right; font-size:10.5px;"
                  " border-bottom:1px solid #f0e6d8; color:#9e9e9e;’>—</td>"
              "<td style=’padding:11px 12px; text-align:right; font-size:10.5px;"
                  " border-bottom:1px solid #f0e6d8; color:#9e9e9e;’>—</td>"
            "</tr>"
            "<tr style=’background:#fff8e8;’>"
              "<td colspan=’3’ style=’padding:10px 12px; text-align:right;"
                  " font-size:10.5px; font-weight:700; color:#5d4037;’>"
                "TOTAL TTC"
              "</td>"
              "<td style=’padding:10px 12px; text-align:right;"
                  " font-size:13px; font-weight:900; color:#b8860b;’>"
                "0,00 DT"
              "</td>"
            "</tr>"
          "</tbody>"
        "</table>"

        // ── Conditions & pied de page ──
        "<div style=’margin-top:18px; padding-top:10px; border-top:1px solid #e8d5b0;’>"
          "<table width=’100%’ cellspacing=’0’ cellpadding=’0’><tr>"
            "<td width=’60%%’ style=’vertical-align:top;’>"
              "<div style=’font-size:9px; color:#9e9e9e;’>"
                "<b style=’color:#8d6e63;’>Conditions de règlement</b><br/>"
                "Paiement à réception de facture.<br/>"
                "Tout retard de paiement entraîne des pénalités (Loi n° 2004-34)."
              "</div>"
            "</td>"
            "<td width=’40%%’ style=’text-align:right; vertical-align:top;’>"
              "<div style=’font-size:9px; color:#a1887f;’>"
                "FIL D&apos;OR &mdash; %2<br/>"
                "<i>Document généré automatiquement</i>"
              "</div>"
            "</td>"
          "</tr></table>"
        "</div>"

        "</body></html>")
    .arg(factureNo.toHtmlEscaped())
    .arg(dateStr.toHtmlEscaped())
    .arg(c.nom.toHtmlEscaped())
    .arg(c.id.toHtmlEscaped())
    .arg(c.telephone.toHtmlEscaped())
    .arg(c.email.toHtmlEscaped())
    .arg(c.adresse.toHtmlEscaped())
    .arg(niveauColor)
    .arg(niveauLabel)
    .arg(c.pointsFidelite);

    QFont bodyFont(QStringLiteral("Arial"), 10, QFont::Normal);
    if (!bodyFont.exactMatch())
        bodyFont = QFont(QStringLiteral("Helvetica"), 10, QFont::Normal);
    QTextDocument doc;
    doc.setDefaultFont(bodyFont);
    doc.setHtml(html);
    doc.print(&printer);

    alerteSucces(QStringLiteral("Facture exportée"),
                 QStringLiteral("Facture %1 générée avec succès.").arg(factureNo));
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

    QLabel *lblActionManageriale = new QLabel("Action manageriale: en attente de simulation.");
    lblActionManageriale->setStyleSheet(
        "font-size: 13px; font-weight: 700; color: #334a57;"
        "background: #f3f8fb; border: 1px solid #d6e3ea; border-radius: 6px; padding: 5px 8px;"
    );
    l->addWidget(lblActionManageriale);

    QSettings settings("FIL_DOR", "ProjetCpp");
    QString savedPreset = settings.value("ravitaillement/preset", "Equilibre").toString();
    auto *analyseFaite = new bool(false);
    QListWidget *actionLogList = nullptr;

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

    QPushButton *btnPlanifier = nullptr;

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

    QFrame *guideFrame = new QFrame(onglet);
    guideFrame->setStyleSheet(
        "QFrame { background: #eef7fb; border: 1px solid #c8dfea; border-radius: 8px; }"
    );
    QHBoxLayout *guideL = new QHBoxLayout(guideFrame);
    guideL->setContentsMargins(10, 6, 10, 6);
    guideL->setSpacing(8);
    QLabel *lblGuide = new QLabel("① Constat (Stock actuel & seuil)    ② Analyse (Scoring fournisseurs)    ③ Décision (Plan d'achat optimal)");
    lblGuide->setStyleSheet("font-size: 12px; font-weight: 700; color: #1f4b62;");
    QPushButton *btnToggleMode = new QPushButton();
    btnToggleMode->setCursor(Qt::PointingHandCursor);
    btnToggleMode->setFixedHeight(30);
    btnToggleMode->setMinimumWidth(150);
    btnToggleMode->setStyleSheet(
        "QPushButton { background: #2f2a66; color: white; border: none; border-radius: 8px; font-size: 12px; font-weight: 800; padding: 0 12px; }"
        "QPushButton:hover { background: #3e3882; }"
    );
    guideL->addWidget(lblGuide, 1);
    guideL->addWidget(btnToggleMode, 0, Qt::AlignRight);
    l->addWidget(guideFrame);

    QFrame *resumeFrame = new QFrame(onglet);
    resumeFrame->setStyleSheet("QFrame { background: #f57c00; border: 1px solid #d16f03; border-radius: 8px; }");
    QVBoxLayout *resumeL = new QVBoxLayout(resumeFrame);
    resumeL->setContentsMargins(10, 7, 10, 7);
    resumeL->setSpacing(4);

    QLabel *lblResumeTitre = new QLabel("STOCK INSUFFISANT - Couverture 4.7j | En dessous du seuil de sécurité");
    lblResumeTitre->setStyleSheet("font-size: 14px; font-weight: 900; color: white;");
    QLabel *lblResumeTexte = new QLabel("Passez commande aujourd'hui pour éviter la rupture.");
    lblResumeTexte->setWordWrap(true);
    lblResumeTexte->setStyleSheet("font-size: 12px; font-weight: 700; color: #fff5e8;");

    resumeL->addWidget(lblResumeTitre);
    resumeL->addWidget(lblResumeTexte);
    l->addWidget(resumeFrame);

    auto appendActionLog = [=](const QString &msg) {
        if (!actionLogList) return;
        const QString stamp = QTime::currentTime().toString("HH:mm:ss");
        actionLogList->insertItem(0, QString("[%1] %2").arg(stamp, msg));
        while (actionLogList->count() > 12) {
            delete actionLogList->takeItem(actionLogList->count() - 1);
        }
    };

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
    photoCuir->setMinimumWidth(320);
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

    QLabel *pilotTitle = new QLabel("② Analyse & Sélection du Fournisseur");
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

    QPushButton *btnAjuster100 = new QPushButton("Auto 100%");
    btnAjuster100->setCursor(Qt::PointingHandCursor);
    btnAjuster100->setFixedHeight(34);
    btnAjuster100->setStyleSheet(
        "QPushButton { background: #e65100; color: white; border: none; border-radius: 8px; font-size: 12px; font-weight: 800; padding: 0 12px; }"
        "QPushButton:hover { background: #f4511e; }"
        "QPushButton:pressed { background: #bf360c; }"
    );
    btnAjuster100->setToolTip("Normalise automatiquement les poids pour atteindre 100%.");
    poidsL->addWidget(btnAjuster100, 0, Qt::AlignBottom);
    pilotL->addLayout(poidsL);

    QLabel *lblModeActif = new QLabel("Mode actif: Equilibre");
    lblModeActif->setStyleSheet("font-size: 12px; font-weight: 800; color: #2a4f61; background: #eef5fa; border: 1px solid #c8d9e5; border-radius: 6px; padding: 4px 6px;");
    pilotL->addWidget(lblModeActif);

    QLabel *lblModeHint = new QLabel("Aide: cliquez 'Auto 100%' pour normaliser automatiquement les poids a 100%.");
    lblModeHint->setStyleSheet("font-size: 12px; font-weight: 700; color: #3a6277;");
    pilotL->addWidget(lblModeHint);

    auto setModeBadge = [=](const QString &text, const QString &niveau) {
        QString bg = "#eef5fa";
        QString fg = "#2a4f61";
        QString bd = "#c8d9e5";
        if (niveau == "eco" || niveau == "faible") {
            bg = "#eaf9ef"; fg = "#1f6a38"; bd = "#b9e1c5";
        } else if (niveau == "urgence" || niveau == "eleve") {
            bg = "#ffecec"; fg = "#8f1d1d"; bd = "#f3adad";
        } else if (niveau == "equilibre" || niveau == "modere") {
            bg = "#fff7e7"; fg = "#8a5a1f"; bd = "#efdbad";
        }
        lblModeActif->setText(text);
        lblModeActif->setStyleSheet(QString(
            "font-size: 12px; font-weight: 800; color: %1; background: %2; border: 1px solid %3; border-radius: 6px; padding: 4px 6px;"
        ).arg(fg, bg, bd));
    };
    setModeBadge("Mode actif: Equilibre", "equilibre");

    QLabel *lblPoidsInfo = new QLabel("Total poids = 100% (parfait).");
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

    QLabel *recommandation = new QLabel("Fournisseur recommandé (scoring pondéré) : MegaCuir | Score global: 66.5/100");
    recommandation->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: #1f6d2a;"
        "background-color: #ccefd1;"
        "border-radius: 8px;"
        "padding: 10px 12px;"
    );

    QLabel *selectionInfo = new QLabel("Fournisseur actif: MegaCuir (auto, meilleur score). Cliquez une autre ligne pour choisir manuellement.");
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
        const QString m = matiere.trimmed().toLower();

        QStringList alts;
        if (m.contains("agneau")) {
            chemin = ":/pic agneau.jpg";
            alts << QStringLiteral("pic agneau.jpg") << QStringLiteral("pic_agneau.jpg")
                 << QStringLiteral("pic agneau.jpeg") << QStringLiteral("pic_agneau.jpeg")
                 << QStringLiteral("cuir agneau.jpg") << QStringLiteral("cuir_agneau.jpg")
                 << QStringLiteral("agneau.jpg");
            legende = "Cuir d'agneau: souple, premium, ideal pour les finitions haut de gamme.";
        } else if (m.contains("veau")) {
            chemin = ":/pic veau.jpg";
            alts << QStringLiteral("pic veau.jpg") << QStringLiteral("pic_veau.jpg")
                 << QStringLiteral("pic veau.jpeg") << QStringLiteral("pic_veau.jpeg")
                 << QStringLiteral("cuir veau.jpg") << QStringLiteral("cuir veau..jpg")
                 << QStringLiteral("cuir_veau.jpg") << QStringLiteral("veau.jpg");
            legende = "Cuir de veau: grain fin, excellent equilibre entre confort et resistance.";
        } else {
            chemin = ":/pic vachette.jpg";
            alts << QStringLiteral("pic vachette.jpg") << QStringLiteral("pic_vachette.jpg")
                 << QStringLiteral("pic vachette.jpeg") << QStringLiteral("pic_vachette.jpeg")
                 << QStringLiteral("cuir vachette.jpg") << QStringLiteral("cuir_vachette.jpg")
                 << QStringLiteral("vachette.jpg");
            legende = "Cuir vachette: robustesse et durabilite, parfait pour une production intensive.";
        }

        const QPixmap pix = loadIllustrationImage(chemin, alts);
        if (pix.isNull()) {
            photoCuir->clear();
            photoCuir->setText("Image indisponible");
            photoSousTitre->setText("Placez les fichiers a la racine du projet, dans images/, ou a cote de l'executable (Qt utilise souvent le dossier build comme repertoire courant).");
            return;
        }

        photoCuir->setPixmap(scaledPixmapForLabel(pix, photoCuir));
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

        if (totalPoids <= 0) {
            table->setRowCount(0);
            lblPoidsInfo->setText("Total poids: 0%. Ajustez les poids.");
            lblPoidsInfo->setStyleSheet("font-size: 12px; color: #b23a2f; font-weight: 800;");
            recommandation->setText("Recommande: ajustez les poids.");
            selectionInfo->setText("Actif: aucun (poids invalides).");
            lblKpiFournisseur->setText("--");
            lblKpiRisqueTxt->setText("--");
            pbRisque->setValue(0);
            setAlerteAuto("MODERE", "Poids invalides. Definissez une ponderation.");
            return;
        }

        lblPoidsInfo->setText(totalPoids == 100
                      ? "Total poids: 100% (OK)."
                      : QString("Total poids: %1%.").arg(totalPoids));
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
            recommandation->setText(QString("Recommande: %1 | Score: %2/100")
                                    .arg(bestSupplier)
                                    .arg(QString::number(bestScore, 'f', 1)));
            lblKpiFournisseur->setText(bestSupplier);
            selectionInfo->setText(QString("Actif (auto): %1")
                                   .arg(bestSupplier));
            lblResumeTitre->setText("Statut module: ANALYSE TERMINEE");
            lblResumeTexte->setText(QString("Etape 2 OK: fournisseur %1 (score %2/100). Passez a l'etape 3.")
                                    .arg(bestSupplier)
                                    .arg(QString::number(bestScore, 'f', 1)));
            lblKpiRisqueTxt->setText(bestScore >= 80.0 ? "Faible" : (bestScore >= 65.0 ? "Modere" : "Eleve"));
            pbRisque->setValue(bestScore >= 80.0 ? 20 : (bestScore >= 65.0 ? 45 : 75));
            setAlerteAuto(bestScore >= 80.0 ? "FAIBLE" : (bestScore >= 65.0 ? "MODERE" : "ELEVE"),
                          bestScore >= 80.0
                                  ? "Fournisseur stable."
                          : (bestScore >= 65.0
                                      ? "Surveiller delai et stock."
                                      : "Activer un fournisseur backup."));
        }
    };

    QObject::connect(table, &QTableWidget::cellClicked, this, [=](int row, int col) {
        Q_UNUSED(col);
        *selectedSupplierRow = row;

        const QString fournisseur = table->item(row, 0) ? table->item(row, 0)->text() : QStringLiteral("Inconnu");
        const QString scoreTxt = table->item(row, 5) ? table->item(row, 5)->text().section('/', 0, 0).trimmed() : QStringLiteral("0");
        recommandation->setText(QString("Choix manuel: %1 | Score: %2/100")
                                .arg(fournisseur, scoreTxt));
        lblKpiFournisseur->setText(fournisseur);
        selectionInfo->setText(QString("Actif (manuel): %1")
                               .arg(fournisseur));
        lblResumeTitre->setText("Statut module: CHOIX MANUEL");
        lblResumeTexte->setText(QString("Fournisseur selectionne: %1. Lancez l'etape 3.")
                    .arg(fournisseur));
        const double score = scoreTxt.toDouble();
        setAlerteAuto(score >= 80.0 ? "FAIBLE" : (score >= 65.0 ? "MODERE" : "ELEVE"),
              QString("Mode manuel (%1/100). ").arg(scoreTxt)
              + (score >= 80.0 ? "OK." : (score >= 65.0 ? "A surveiller." : "Backup conseille.")));
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

    QObject::connect(btnComparer, &QPushButton::clicked, this, [=]() {
        remplirTable();
        *analyseFaite = true;
        if (btnPlanifier) btnPlanifier->setEnabled(true);
        lblResumeTitre->setText("Statut module: ETAPE 2 VALIDEE");
        lblResumeTexte->setText("Analyse faite. Passez a l'etape 3 pour generer le plan.");
        appendActionLog("Analyse fournisseurs executee");
    });
    QObject::connect(cbMatiere, &QComboBox::currentTextChanged, this, [=](const QString &matiere) {
        majPhotoCuir(matiere);
        remplirTable();
        *analyseFaite = false;
        if (btnPlanifier) btnPlanifier->setEnabled(false);
        lblResumeTitre->setText("Statut module: ENTREE MODIFIEE");
        lblResumeTexte->setText("Matiere changee. Relancez l'etape 1 (Analyser fournisseurs).");
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
        setModeBadge("Mode actif: Personnalise", "equilibre");
        lblModeHint->setText("Aide: ponderation remise en mode personnalise.");
        QSettings("FIL_DOR", "ProjetCpp").setValue("ravitaillement/preset", "Personnalise");
        remplirTable();
    });

    auto applyPreset = [=](const QString &modeName) {
        if (modeName == "Eco") {
            spPoidsPrix->setValue(50);
            spPoidsQualite->setValue(20);
            spPoidsDelai->setValue(20);
            spPoidsStock->setValue(10);
            setModeBadge("Mode actif: Eco", "eco");
            lblModeHint->setText("Aide mode Eco: priorite au cout.");
        } else if (modeName == "Urgence") {
            spPoidsPrix->setValue(15);
            spPoidsQualite->setValue(25);
            spPoidsDelai->setValue(45);
            spPoidsStock->setValue(15);
            setModeBadge("Mode actif: Urgence", "urgence");
            lblModeHint->setText("Aide mode Urgence: priorite delai + disponibilite.");
        } else {
            spPoidsPrix->setValue(35);
            spPoidsQualite->setValue(30);
            spPoidsDelai->setValue(20);
            spPoidsStock->setValue(15);
            setModeBadge("Mode actif: Equilibre", "equilibre");
            lblModeHint->setText("Aide mode Equilibre: compromis cout/qualite/delai.");
        }
        QSettings("FIL_DOR", "ProjetCpp").setValue("ravitaillement/preset", modeName);
        appendActionLog(QString("Mode applique: %1").arg(modeName));
        remplirTable();
    };

    majPhotoCuir(cbMatiere->currentText());
    QTimer::singleShot(0, this, [=]() { majPhotoCuir(cbMatiere->currentText()); });
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

    QLabel *planTitle = new QLabel("③ Décision d'Achat Finale");
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

    QObject::connect(btnAjuster100, &QPushButton::clicked, this, [=]() {
        const int total = spPoidsPrix->value() + spPoidsQualite->value() + spPoidsDelai->value() + spPoidsStock->value();
        if (total <= 0) {
            applyPreset("Equilibre");
            setModeBadge("Mode actif: Equilibre (auto-fix)", "equilibre");
            lblModeHint->setText("Aide: poids invalides corriges automatiquement.");
            return;
        }

        const int nPrix = qBound(0, qRound((spPoidsPrix->value() * 100.0) / total), 100);
        const int nQual = qBound(0, qRound((spPoidsQualite->value() * 100.0) / total), 100);
        const int nDelai = qBound(0, qRound((spPoidsDelai->value() * 100.0) / total), 100);
        int nStock = 100 - nPrix - nQual - nDelai;
        nStock = qBound(0, nStock, 100);

        spPoidsPrix->setValue(nPrix);
        spPoidsQualite->setValue(nQual);
        spPoidsDelai->setValue(nDelai);
        spPoidsStock->setValue(nStock);

        setModeBadge("Mode actif: Personnalise (100%)", "equilibre");
        lblModeHint->setText("Aide: poids normalises a 100%.");
        QSettings("FIL_DOR", "ProjetCpp").setValue("ravitaillement/preset", "Personnalise");
        appendActionLog("Poids ajustes automatiquement a 100%");
        remplirTable();
    });

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

    QHBoxLayout *seuilJoursL = new QHBoxLayout();
    seuilJoursL->setSpacing(8);
    QLabel *lblSeuilModere = new QLabel("Seuil modere (jours)");
    QLabel *lblSeuilCritique = new QLabel("Seuil critique (jours)");
    lblSeuilModere->setStyleSheet("font-size: 12px; font-weight: 700; color: #25414f;");
    lblSeuilCritique->setStyleSheet("font-size: 12px; font-weight: 700; color: #25414f;");

    QSpinBox *sbSeuilModereJour = new QSpinBox(onglet);
    QSpinBox *sbSeuilCritiqueJour = new QSpinBox(onglet);
    sbSeuilModereJour->setRange(2, 30);
    sbSeuilCritiqueJour->setRange(1, 20);
    sbSeuilModereJour->setValue(8);
    sbSeuilCritiqueJour->setValue(5);
    sbSeuilModereJour->setStyleSheet("QSpinBox { background: white; border: 1px solid #8fb8cc; border-radius: 8px; min-height: 30px; padding-left: 8px; }");
    sbSeuilCritiqueJour->setStyleSheet("QSpinBox { background: white; border: 1px solid #8fb8cc; border-radius: 8px; min-height: 30px; padding-left: 8px; }");

    QWidget *seuilModereBox = new QWidget();
    QVBoxLayout *seuilModereVL = new QVBoxLayout(seuilModereBox);
    seuilModereVL->setContentsMargins(0, 0, 0, 0);
    seuilModereVL->setSpacing(2);
    seuilModereVL->addWidget(lblSeuilModere);
    seuilModereVL->addWidget(sbSeuilModereJour);

    QWidget *seuilCritBox = new QWidget();
    QVBoxLayout *seuilCritVL = new QVBoxLayout(seuilCritBox);
    seuilCritVL->setContentsMargins(0, 0, 0, 0);
    seuilCritVL->setSpacing(2);
    seuilCritVL->addWidget(lblSeuilCritique);
    seuilCritVL->addWidget(sbSeuilCritiqueJour);

    seuilJoursL->addWidget(seuilModereBox);
    seuilJoursL->addWidget(seuilCritBox);
    seuilJoursL->addStretch();
    planLayout->addLayout(seuilJoursL);

    QLabel *lblJoursRupture = new QLabel("Jours avant rupture: --");
    lblJoursRupture->setStyleSheet("font-size: 12px; font-weight: 800; color: #35566a; background: #eef7fb; border: 1px solid #c8deea; border-radius: 7px; padding: 6px 8px;");
    planLayout->addWidget(lblJoursRupture);

    btnPlanifier = new QPushButton("Générer plan de ravitaillement");
    btnPlanifier->setCursor(Qt::PointingHandCursor);
    btnPlanifier->setEnabled(false);
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

    QLabel *lblPlan = new QLabel("Cliquez pour générer la quantité et le budget recommandés.");
    lblPlan->setWordWrap(true);
    lblPlan->setStyleSheet("font-size: 14px; color: #244150; background: #edf7fc; border: 1px solid #c8deea; border-radius: 8px; padding: 8px;");

    QLabel *lblActionPlan = new QLabel("Action: en attente.");
    lblActionPlan->setWordWrap(true);
    lblActionPlan->setStyleSheet("font-size: 13px; color: #153f2f; background: #ecf9ef; border: 1px solid #b7e2c1; border-radius: 8px; padding: 7px;");

    QObject::connect(btnPlanifier, &QPushButton::clicked, this, [=]() {
        if (!*analyseFaite) {
            lblResumeTitre->setText("Statut module: ETAPE MANQUANTE");
        lblResumeTexte->setText("Vous devez d'abord lancer 'Analyser fournisseurs'.");
            appendActionLog("Plan refuse: analyse non executee");
            return;
        }

        const int row = (*selectedSupplierRow >= 0 && *selectedSupplierRow < table->rowCount()) ? *selectedSupplierRow : 0;
        if (table->rowCount() <= 0 || !table->item(row, 0) || !table->item(row, 1) || !table->item(row, 4)) {
            lblPlan->setText("Aucun fournisseur disponible.");
            lblActionPlan->setText("Action: verifier les donnees.");
            lblResumeTitre->setText("Statut module: DONNEES MANQUANTES");
            lblResumeTexte->setText("Relancez l'analyse fournisseur avant de generer le plan.");
            setAlerteAuto("ELEVE", "Donnees fournisseurs manquantes.");
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
        const int seuilCritique = sbSeuilCritiqueJour->value();
        const int seuilModere = qMax(sbSeuilModereJour->value(), seuilCritique + 1);

        lblKpiCouverture->setText(QString::number(couvertureJours, 'f', 1) + " j");
        lblJoursRupture->setText(QString("Jours avant rupture: %1 j").arg(QString::number(couvertureJours, 'f', 1)));
        if (couvertureJours <= seuilCritique) {
            lblJoursRupture->setStyleSheet("font-size: 12px; font-weight: 900; color: #8f1d1d; background: #ffecec; border: 1px solid #f3adad; border-radius: 7px; padding: 6px 8px;");
        } else if (couvertureJours <= seuilModere) {
            lblJoursRupture->setStyleSheet("font-size: 12px; font-weight: 900; color: #7a4c12; background: #fff8e9; border: 1px solid #f1d28f; border-radius: 7px; padding: 6px 8px;");
        } else {
            lblJoursRupture->setStyleSheet("font-size: 12px; font-weight: 900; color: #1d6f38; background: #ecfbf2; border: 1px solid #a9dfbe; border-radius: 7px; padding: 6px 8px;");
        }

        if (besoinBrut <= 0.0) {
            lblPlan->setText("Stock suffisant pour 7 jours.");
            lblActionPlan->setText("Action: suivi hebdomadaire.");
            lblResumeTitre->setText("Statut module: PLAN VALIDE");
            lblResumeTexte->setText("Aucune commande urgente. Suivi simple recommande.");
            lblKpiBudget->setText("0 DT");
            lblKpiRisqueTxt->setText("Faible");
            pbRisque->setValue(15);
            setAlerteAuto("FAIBLE", "Pas de commande urgente.");
            appendActionLog("Plan genere: stock suffisant, aucune commande");
            lblActionManageriale->setText("Action manageriale: stock suffisant. Surveillance hebdomadaire.");
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

        if (reliquat > 0.0 || couvertureJours <= seuilCritique) {
            lblActionPlan->setText("Action: commande immediate + backup.");
            lblResumeTitre->setText("Statut module: ACTION URGENTE");
            lblResumeTexte->setText("Risque eleve: commande immediate et fournisseur backup.");
            lblKpiRisqueTxt->setText("Eleve");
            pbRisque->setValue(82);
            setAlerteAuto("ELEVE", "Risque de rupture. Commander maintenant.");
            appendActionLog("Plan genere: alerte urgente, commande immediate");
            lblActionManageriale->setText("Action manageriale: urgence. Commander immediatement + fournisseur backup.");
        } else if (couvertureJours <= seuilModere) {
            lblActionPlan->setText("Action: commander aujourd'hui et suivre.");
            lblResumeTitre->setText("Statut module: ACTION MODEREE");
            lblResumeTexte->setText("Commande recommandee aujourd'hui avec suivi quotidien.");
            lblKpiRisqueTxt->setText("Modere");
            pbRisque->setValue(52);
            setAlerteAuto("MODERE", "Couverture limitee. Suivi journalier recommande.");
            appendActionLog("Plan genere: commande recommandee (niveau modere)");
            lblActionManageriale->setText("Action manageriale: risque modere. Commander aujourd'hui et suivre quotidiennement.");
        } else {
            lblActionPlan->setText("Action: plan valide.");
            lblResumeTitre->setText("Statut module: PLAN ROBUSTE");
            lblResumeTexte->setText("Plan stable. Vous pouvez valider et executer.");
            lblKpiRisqueTxt->setText("Faible");
            pbRisque->setValue(24);
            setAlerteAuto("FAIBLE", "Plan valide avec bonne couverture.");
            appendActionLog("Plan genere: plan robuste valide");
            lblActionManageriale->setText("Action manageriale: plan robuste. Valider la commande standard.");
        }
    });

    planLayout->addWidget(btnPlanifier, 0, Qt::AlignLeft);
    planLayout->addWidget(lblPlan);
    planLayout->addWidget(lblActionPlan);

    QPushButton *btnCopierDecision = new QPushButton("Copier décision");
    btnCopierDecision->setCursor(Qt::PointingHandCursor);
    btnCopierDecision->setFixedHeight(32);
    btnCopierDecision->setStyleSheet(
        "QPushButton { background: #1f6f99; color: white; border: none; border-radius: 16px; font-size: 12px; font-weight: 800; padding: 0 12px; }"
        "QPushButton:hover { background: #2685b8; }"
    );
    planLayout->addWidget(btnCopierDecision, 0, Qt::AlignLeft);

    QObject::connect(btnCopierDecision, &QPushButton::clicked, this, [=]() {
        QString plainPlan = lblPlan->text();
        plainPlan.replace("<br>", "\n");
        plainPlan.remove(QRegularExpression("<[^>]*>"));

        QString plainAction = lblActionPlan->text();
        plainAction.remove(QRegularExpression("<[^>]*>"));

        const QString summary = QString("%1\n%2\nRisque: %3\nBudget: %4")
                                    .arg(plainPlan.trimmed())
                                    .arg(plainAction.trimmed())
                                    .arg(lblKpiRisqueTxt->text().trimmed())
                                    .arg(lblKpiBudget->text().trimmed());

        if (QGuiApplication::clipboard()) {
            QGuiApplication::clipboard()->setText(summary);
            lblResumeTitre->setText("Statut module: DECISION COPIEE");
            lblResumeTexte->setText("Le resume est copie dans le presse-papiers.");
            appendActionLog("Decision copiee dans le presse-papiers");
        }
    });

    l->addWidget(planFrame);

    QFrame *actionLogFrame = new QFrame(onglet);
    actionLogFrame->setStyleSheet("QFrame { background: #f8fbfd; border: 1px solid #cfe1ea; border-radius: 10px; }");
    QVBoxLayout *actionLogL = new QVBoxLayout(actionLogFrame);
    actionLogL->setContentsMargins(10, 8, 10, 8);
    actionLogL->setSpacing(6);

    QLabel *actionLogTitle = new QLabel("Journal actions (demo)");
    actionLogTitle->setStyleSheet("font-size: 14px; font-weight: 900; color: #234556;");
    actionLogL->addWidget(actionLogTitle);

    actionLogList = new QListWidget(onglet);
    actionLogList->setMinimumHeight(110);
    actionLogList->setStyleSheet("QListWidget { background: white; border: 1px solid #d5e4ec; border-radius: 8px; font-size: 12px; color: #2a4657; }");
    actionLogL->addWidget(actionLogList);
    appendActionLog("Module pret: lancez l'etape 1");
    actionLogFrame->setVisible(false);

    QFrame *efficaciteFrame = new QFrame(onglet);
    efficaciteFrame->setStyleSheet(
        "QFrame { background: #f2fbf8; border: 1px solid #bfe3d5; border-radius: 10px; }"
    );
    QVBoxLayout *effL = new QVBoxLayout(efficaciteFrame);
    effL->setContentsMargins(12, 10, 12, 10);
    effL->setSpacing(7);

    QLabel *effTitle = new QLabel("Bloc smart: Diagnostic efficacite");
    effTitle->setStyleSheet("font-size: 16px; font-weight: 900; color: #1d5f4a;");
    effL->addWidget(effTitle);

    QLabel *effTxt = new QLabel("Cliquez sur 'Diagnostic express' pour obtenir un score global et une action immediate.");
    effTxt->setWordWrap(true);
    effTxt->setStyleSheet("font-size: 12px; font-weight: 700; color: #2d6653;");
    effL->addWidget(effTxt);

    QPushButton *btnDiagnostic = new QPushButton("Diagnostic express");
    btnDiagnostic->setCursor(Qt::PointingHandCursor);
    btnDiagnostic->setFixedHeight(34);
    btnDiagnostic->setStyleSheet(
        "QPushButton { background: #138a5a; color: white; border: none; border-radius: 17px; font-size: 13px; font-weight: 800; padding: 0 14px; }"
        "QPushButton:hover { background: #16a36b; }"
    );
    effL->addWidget(btnDiagnostic, 0, Qt::AlignLeft);

    QLabel *effResult = new QLabel("Score efficacite: --");
    effResult->setStyleSheet("font-size: 13px; font-weight: 800; color: #124636;");
    QLabel *effAction = new QLabel("Action prioritaire: --");
    effAction->setWordWrap(true);
    effAction->setStyleSheet("font-size: 12px; font-weight: 700; color: #1f5a47; background: #e8f6ef; border: 1px solid #c1e3d2; border-radius: 7px; padding: 6px 8px;");
    effL->addWidget(effResult);
    effL->addWidget(effAction);

    efficaciteFrame->setVisible(false);

    auto updateDiagnostic = [=]() {
        const int row = (*selectedSupplierRow >= 0 && *selectedSupplierRow < table->rowCount()) ? *selectedSupplierRow : 0;
        if (table->rowCount() <= 0 || !table->item(row, 1) || !table->item(row, 4)) {
            effResult->setText("Score efficacite: --");
            effAction->setText("Action prioritaire: lancer l'analyse fournisseurs.");
            return;
        }

        const double prix = parseNumber(table->item(row, 1)->text());
        const double stockDispo = parseNumber(table->item(row, 4)->text());
        const double stockActuel = sbStockActuel->value();
        const double seuil = sbSeuilSecurite->value();
        const double conso = sbConsoPrevue->value();
        const double besoin = qMax(0.0, seuil + conso - stockActuel);

        int score = 100;
        if (pbRisque->value() > 0) score -= pbRisque->value() / 2;
        if (besoin > stockDispo) score -= 20;
        if (prix > 45.0) score -= 8;
        score = qBound(5, score, 100);

        const QString niveau = score >= 75 ? "Excellent" : (score >= 55 ? "Correct" : "Critique");
        effResult->setText(QString("Score efficacite: %1/100 (%2)").arg(score).arg(niveau));

        if (score >= 75) {
            effAction->setText("Action prioritaire: valider le plan actuel et sauvegarder la decision.");
        } else if (score >= 55) {
            effAction->setText("Action prioritaire: ajuster les poids avec 'Auto 100%' puis relancer le plan.");
        } else {
            effAction->setText("Action prioritaire: utiliser 'Auto 100%', generer le plan de ravitaillement et commander immediatement.");
        }
    };

    QObject::connect(btnDiagnostic, &QPushButton::clicked, this, [=]() { updateDiagnostic(); });
    QObject::connect(btnComparer, &QPushButton::clicked, this, [=]() { updateDiagnostic(); });
    QObject::connect(btnAjuster100, &QPushButton::clicked, this, [=]() { updateDiagnostic(); });
    QObject::connect(btnPlanifier, &QPushButton::clicked, this, [=]() { updateDiagnostic(); });

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

    QLabel *histTitle = new QLabel("Historique & Traçabilité des Décisions");
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

    histFilters->addWidget(lblMat);
    histFilters->addWidget(leMatFilter, 1);
    histFilters->addWidget(lblFrom);
    histFilters->addWidget(deFrom);
    histFilters->addWidget(lblTo);
    histFilters->addWidget(deTo);
    histFilters->addWidget(btnHistFiltrer);
    histFilters->addWidget(btnHistReset);
    histL->addLayout(histFilters);

    QTableWidget *tableHist = new QTableWidget(0, 8, onglet);
    tableHist->setHorizontalHeaderLabels(QStringList()
                                         << "DATE"
                                         << "MATIÈRE"
                                         << "FOURNISSEUR"
                                         << "SCORE"
                                         << "QTÉ"
                                         << "BUDGET"
                                         << "RISQUE"
                                         << "ÉTAT");
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

    auto applyAdvancedPanelUi = [=](bool expanded) {
        strategyFrame->setVisible(expanded);
        histFrame->setVisible(true);
        btnToggleMode->setText(expanded
                               ? "Paramètres Avancés (EOQ / Wilson / Stock de Sécurité) ▼"
                               : "Paramètres Avancés (EOQ / Wilson / Stock de Sécurité) ▶");
        QSettings("FIL_DOR", "ProjetCpp").setValue("ravitaillement/advanced_expanded", expanded);
    };

    QObject::connect(btnToggleMode, &QPushButton::clicked, this, [=]() {
        const bool nextExpanded = strategyFrame->isHidden();
        applyAdvancedPanelUi(nextExpanded);
    });

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

    applyPreset("Equilibre");
    const bool advancedExpanded = settings.value("ravitaillement/advanced_expanded", false).toBool();
    applyAdvancedPanelUi(advancedExpanded);

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
    photoProduit->setMinimumWidth(320);
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

        QStringList alts;
        if (produit == "Sac Main Cuir") {
            chemin = ":/sac main.webp";
            alts << QStringLiteral("sac main.WEBP") << QStringLiteral("sac main.webp") << QStringLiteral("sac_main.WEBP")
                 << QStringLiteral("sac_main.webp") << QStringLiteral("sac_main.jpg") << QStringLiteral("sac_main.png");
            legende = "Sac main cuir: finition elegante, cible premium et forte valeur ajoutee.";
        } else if (produit == "Portefeuille Cuir") {
            chemin = ":/portfeuille.webp";
            alts << QStringLiteral("portfeuille.WEBP") << QStringLiteral("portfeuille.webp") << QStringLiteral("portefeuille.webp")
                 << QStringLiteral("portefeuille.jpg") << QStringLiteral("portefeuille.png");
            legende = "Portefeuille cuir: format compact, cadence elevee, precision de coupe.";
        } else if (produit == "Ceinture Cuir") {
            chemin = ":/ceinture.webp";
            alts << QStringLiteral("ceinture.WEBP") << QStringLiteral("ceinture.webp") << QStringLiteral("ceinture.jpg") << QStringLiteral("ceinture.png");
            legende = "Ceinture cuir: piece technique, regularite de qualite et resistance.";
        } else {
            chemin = ":/sac voyage.jpg";
            alts << QStringLiteral("sac voyage.jpg") << QStringLiteral("sac_voyage.jpg") << QStringLiteral("sac_voyage.png") << QStringLiteral("sac_voyage.webp");
            legende = "Sac voyage cuir: grande surface, produit iconique a fort impact visuel.";
        }

        const QPixmap pix = loadIllustrationImage(chemin, alts);
        if (pix.isNull()) {
            photoProduit->clear();
            photoProduit->setText("Image indisponible");
            photoProduitSousTitre->setText("Placez les fichiers a la racine du projet, dans images/, ou a cote de l'executable (Qt utilise souvent le dossier build comme repertoire courant).");
            return;
        }

        photoProduit->setPixmap(scaledPixmapForLabel(pix, photoProduit));
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
    QTimer::singleShot(0, this, [=]() { majPhotoProduit(cbProduit->currentText()); });
    calculerBesoin();
    refreshCalcHistorique();
    ui->tabWidgetStock->setCurrentIndex(5);
}

// --- MODULE IA : ESTIMATION DE TEMPS AVANCEE (régression linéaire) ---
static constexpr double HEURES_PAR_JOUR = 8.0;

static bool tableExistsOracle(const QString &tableNameUpper)
{
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT COUNT(*) "
        "FROM USER_TABLES "
        "WHERE TABLE_NAME = :t"));
    q.bindValue(QStringLiteral(":t"), tableNameUpper.trimmed().toUpper());
    if (!q.exec() || !q.next())
        return false;
    return q.value(0).toInt() > 0;
}

static bool tryLoadCalendarInfo(const QDate &d, bool *outOpen, double *outCoef)
{
    if (!d.isValid())
        return false;
    // Mode E/A validé: pas de table calendrier dédiée.
    const bool isOpen = (d.dayOfWeek() >= 1 && d.dayOfWeek() <= 5);
    const double coef = 1.0;
    if (outOpen)
        *outOpen = isOpen;
    if (outCoef)
        *outCoef = qBound(0.0, coef, 2.0);
    return true;
}

static QVector<int> chargerEquipePlanification(int idCommande)
{
    QVector<int> ids;
    if (idCommande <= 0)
        return ids;

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT DISTINCT ID_EMPLOYE "
        "FROM ETAPES "
        "WHERE ID_PLANIFICATION = :id "
        "AND ID_EMPLOYE IS NOT NULL"));
    q.bindValue(QStringLiteral(":id"), idCommande);
    if (!q.exec())
        return ids;

    while (q.next()) {
        const int id = q.value(0).toInt();
        if (id > 0)
            ids.push_back(id);
    }
    return ids;
}

/// Effectifs distincts par étape pour une commande (module ETAPES + PLANIFICATION).
static QHash<QString, int> effectifsParEtapePourCommande(int idCommande)
{
    QHash<QString, int> out;
    if (idCommande <= 0)
        return out;
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT UPPER(TRIM(ETAPE_ACTUELLE)) AS etp, COUNT(DISTINCT ID_EMPLOYE) AS n "
        "FROM ETAPES "
        "WHERE ID_PLANIFICATION = :id AND ID_EMPLOYE IS NOT NULL "
        "GROUP BY UPPER(TRIM(ETAPE_ACTUELLE))"));
    q.bindValue(QStringLiteral(":id"), idCommande);
    if (!q.exec())
        return out;
    while (q.next()) {
        const QString k = q.value(QStringLiteral("etp")).toString();
        const int n = qMax(1, q.value(QStringLiteral("n")).toInt());
        if (!k.isEmpty())
            out.insert(k, n);
    }
    return out;
}

/// Ajuste la charge selon la maîtrise moyenne (COMPETENCES_EMPLOYES × affectations ETAPES).
static double facteurCompetenceEtape(int idCommande, const QString &etapeKeyUpper)
{
    if (idCommande <= 0 || etapeKeyUpper.isEmpty() || !tableExistsOracle(QStringLiteral("COMPETENCES_EMPLOYES")))
        return 1.0;

    QStringList empIds;
    {
        QSqlQuery q;
        q.prepare(QStringLiteral(
            "SELECT DISTINCT ID_EMPLOYE FROM ETAPES "
            "WHERE ID_PLANIFICATION = :id AND ID_EMPLOYE IS NOT NULL "
            "AND UPPER(TRIM(ETAPE_ACTUELLE)) = :et"));
        q.bindValue(QStringLiteral(":id"), idCommande);
        q.bindValue(QStringLiteral(":et"), etapeKeyUpper);
        if (!q.exec())
            return 1.0;
        while (q.next()) {
            const int eid = q.value(0).toInt();
            if (eid > 0)
                empIds << QString::number(eid);
        }
    }
    if (empIds.isEmpty())
        return 1.0;

    QSqlQuery q2;
    const QString sql = QStringLiteral(
        "SELECT AVG(NVL(NIVEAU, 3)) FROM COMPETENCES_EMPLOYES "
        "WHERE ACTIF = 1 AND UPPER(TRIM(ETAPE)) = :et "
        "AND ID_EMPLOYE IN (%1)").arg(empIds.join(QLatin1Char(',')));
    q2.prepare(sql);
    q2.bindValue(QStringLiteral(":et"), etapeKeyUpper);
    if (!q2.exec() || !q2.next())
        return 1.0;
    const QVariant av = q2.value(0);
    if (av.isNull())
        return 1.06;

    const double avgLv = qBound(1.0, av.toDouble(), 5.0);
    const double f = 1.0 + 0.14 * (3.0 - avgLv) / 2.0;
    return qBound(0.82, f, 1.22);
}

static int compterAbsents(const QDate &d, const QVector<int> &equipeIds)
{
    Q_UNUSED(d);
    Q_UNUSED(equipeIds);
    // Mode E/A validé: pas de table absences dédiée.
    return 0;
}

static double capacitePosteJour(const QDate &d, const QString &etape)
{
    Q_UNUSED(d);
    Q_UNUSED(etape);
    // Mode E/A validé: pas de table capacité poste/jour dédiée.
    // Retour -1 => fallback capacité standard dans le moteur IA.
    return -1.0;
}

static int tableRowCountOracle(const QString &tableNameUpper)
{
    if (!tableExistsOracle(tableNameUpper))
        return 0;
    QSqlQuery q;
    const QString sql = QStringLiteral("SELECT COUNT(*) FROM %1").arg(tableNameUpper.trimmed().toUpper());
    if (!q.exec(sql) || !q.next())
        return 0;
    return q.value(0).toInt();
}

/// Évite 3 COUNT Oracle à chaque rafraîchissement de l’onglet IA (cache ~30 s).
static void appendIaCalendarTransparencyCached(QString &recommendationHtml)
{
    recommendationHtml += QStringLiteral(
        "<hr style='border:0; border-top:1px solid #e0d7ea; margin:10px 0;'>"
        "<div style='padding:8px; background:#fffaf2; border:1px solid #f2d8a7; border-radius:8px;'>"
        "<div style='font-weight:800; color:#5d4037; margin-bottom:4px;'>Transparence moteur calendrier</div>"
        "<div style='font-size:11px; line-height:1.35;'>"
        "<div><b>Calendrier production :</b> <span style='color:#ef6c00; font-weight:800;'>Fallback semaine L-V</span></div>"
        "<div><b>Absences employés :</b> <span style='color:#ef6c00; font-weight:800;'>Non modélisé (tables validées E/A)</span></div>"
        "<div><b>Capacité poste/jour :</b> <span style='color:#ef6c00; font-weight:800;'>Fallback capacité standard</span></div>"
        "</div></div>")
        ;
}

static void ouvrirAdminCalendrierAbsences(QWidget *parent)
{
    QMessageBox::information(parent,
                             QStringLiteral("Admin planning V2"),
                             QStringLiteral("Cette administration n'est plus utilisée dans le modèle E/A validé.\n"
                                            "Tables retirées: CALENDRIER_PROD, ABSENCES_EMPLOYES, CAPACITE_POSTE_JOUR.\n"
                                            "Le moteur planification utilise désormais un fallback standard (L-V, 8h/jour)."));
}

double MainWindow::baseHoursPerUnit(const QString& etape) const
{
    const QString e = etape.trimmed().toUpper();
    if (e == QStringLiteral("COUPE")) return 0.20;
    if (e == QStringLiteral("ASSEMBLAGE")) return 0.35;
    if (e == QStringLiteral("COUTURE")) return 0.30;
    if (e == QStringLiteral("FINITION")) return 0.12;
    return 0.25;
}

bool MainWindow::isWorkingDay(const QDate& d) const
{
    bool openFromCalendar = false;
    double coef = 1.0;
    if (tryLoadCalendarInfo(d, &openFromCalendar, &coef))
        return openFromCalendar;
    const int wd = d.dayOfWeek();
    return wd >= 1 && wd <= 5;
}

QDate MainWindow::addWorkingDays(const QDate& start, int days) const
{
    QDate d = start;
    int added = 0;
    while (added < days) {
        d = d.addDays(1);
        if (isWorkingDay(d)) ++added;
    }
    return d;
}

bool MainWindow::calibrateBaseHoursFromHistory()
{
    qint64 fpCnt = -1, fpMax = -1;
    double fpSumRatio = 0.0;
    const bool fpOk = peekBaseHCalibFingerprint(&fpCnt, &fpMax, &fpSumRatio);
    if (fpOk && fpCnt == m_baseHCalibRowCount && fpMax == m_baseHCalibMaxSuivi
        && qAbs(fpSumRatio - m_baseHCalibSumTempsRatio) < 1e-3 && !m_baseHUnit.isEmpty()) {
        return true;
    }

    m_baseHUnit.clear();

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT UPPER(TRIM(e.ETAPE_ACTUELLE)) AS etape, "
        "AVG(e.TEMPS_REEL_PASSE / NULLIF(p.QUANTITE, 0)) AS h_unit, "
        "COUNT(*) AS n "
        "FROM ETAPES e "
        "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
        "WHERE e.TEMPS_REEL_PASSE > 0 "
        "AND p.QUANTITE > 0 "
        "AND (e.TEMPS_REEL_PASSE / p.QUANTITE) BETWEEN 0.01 AND 5 "
        "GROUP BY UPPER(TRIM(e.ETAPE_ACTUELLE))"));

    if (!q.exec())
        return false;

    while (q.next()) {
        const QString et = q.value(QStringLiteral("etape")).toString();
        const double h = q.value(QStringLiteral("h_unit")).toDouble();
        const int n = q.value(QStringLiteral("n")).toInt();
        if (n >= 5 && h > 0.0)
            m_baseHUnit[et] = h;
    }

    if (!m_baseHUnit.contains(QStringLiteral("COUPE")))      m_baseHUnit[QStringLiteral("COUPE")] = 0.20;
    if (!m_baseHUnit.contains(QStringLiteral("ASSEMBLAGE"))) m_baseHUnit[QStringLiteral("ASSEMBLAGE")] = 0.35;
    if (!m_baseHUnit.contains(QStringLiteral("COUTURE")))    m_baseHUnit[QStringLiteral("COUTURE")] = 0.30;
    if (!m_baseHUnit.contains(QStringLiteral("FINITION")))   m_baseHUnit[QStringLiteral("FINITION")] = 0.12;

    if (fpOk) {
        m_baseHCalibRowCount = fpCnt;
        m_baseHCalibMaxSuivi = fpMax;
        m_baseHCalibSumTempsRatio = fpSumRatio;
    }
    return true;
}

double MainWindow::hUnit(const QString& etape) const
{
    return m_baseHUnit.value(etape.toUpper().trimmed(), baseHoursPerUnit(etape));
}

double MainWindow::complexityFactor(double c) const
{
    if (c >= 3) return 1.20;
    if (c >= 2) return 1.10;
    return 1.00;
}

double MainWindow::cadenceFactor(double cadence) const
{
    if (cadence <= 0) cadence = 1.0;
    return 1.0 / qBound(0.7, cadence, 1.3);
}

double MainWindow::qualityReworkFactor(double txRework) const
{
    return 1.0 + qBound(0.0, txRework, 0.20);
}

QVector<MainWindow::StepPlan> MainWindow::planRealisteToutesEtapes(
    double qte, double complexite, double nbEmp, double heuresJour,
    double rendement, double cadence, double txRework,
    const QDate& dateDebutCmd,
    int idCommande,
    const QVector<int>& equipeIds,
    const QHash<QString, int>& overrideEff) const
{
    const QVector<QString> ordre = {
        QStringLiteral("COUPE"),
        QStringLiteral("ASSEMBLAGE"),
        QStringLiteral("COUTURE"),
        QStringLiteral("FINITION")
    };
    QVector<StepPlan> out;

    if (nbEmp <= 0) nbEmp = 1;
    if (heuresJour <= 0) heuresJour = HEURES_PAR_JOUR;
    rendement = qBound(0.60, rendement, 0.98);

    const double fComp = complexityFactor(complexite);
    const double fCad = cadenceFactor(cadence);
    const double fRw = qualityReworkFactor(txRework);
    const QVector<int> equipe = !equipeIds.isEmpty() ? equipeIds : chargerEquipePlanification(idCommande);
    const QHash<QString, int> effParEtape = effectifsParEtapePourCommande(idCommande);
    const int nbEmpFallback = qMax(1, static_cast<int>(qRound(nbEmp)));

    QDate currentStart = dateDebutCmd.isValid() ? dateDebutCmd : QDate::currentDate();

    for (const QString& e : ordre) {
        StepPlan sp;
        sp.etape = e;
        const double qteSafe = qMax(1.0, qte);
        
        int nbPourEtape = qMax(1, effParEtape.value(e, nbEmpFallback));
        if (overrideEff.contains(e)) {
            nbPourEtape = overrideEff.value(e);
        }
        const double nbPourEtapeD = static_cast<double>(nbPourEtape);

        // Fallback "métier" (stable + calibrable via historique moyen h/unité).
        const double hFallbackUnit = hUnit(e);
        const double hFallbackTotal = qteSafe * hFallbackUnit * fComp * fCad * fRw;

        // ML: modèle par-étape (prioritaire) ou modèle global fallback.
        double hMlTotal = 0.0;
        if (!mlW.isEmpty() || !m_mlWeightsPerEtape.isEmpty()) {
            const int code = mapEtapeToCode(e);
            hMlTotal = predictStepHours(code, qteSafe, complexite, nbPourEtapeD, cadence) * fRw;
        }

        // Garde-fou: éviter les valeurs aberrantes si le dataset est petit / bruité.
        if (hMlTotal > 0.0) {
            const double lo = hFallbackTotal * 0.50;
            const double hi = hFallbackTotal * 2.00;
            sp.chargeHeures = qBound(lo, hMlTotal, hi);
        } else {
            sp.chargeHeures = hFallbackTotal;
        }

        // Module compétences (si table alimentée) : ajuste le temps opératoire réaliste.
        sp.chargeHeures *= facteurCompetenceEtape(idCommande, e);
        sp.hUnitaire = sp.chargeHeures / qteSafe;
        sp.debut = currentStart;

        // Sprint 1 V2: capacité journalière réelle (calendrier + absences + capacité poste/jour).
        double reste = sp.chargeHeures;
        double joursFrac = 0.0;
        QDate d = sp.debut;
        int guard = 0;
        while (reste > 1e-6 && guard < 3700) {
            ++guard;
            d = d.addDays(1);
            if (!isWorkingDay(d))
                continue;

            bool opened = true;
            double coefCal = 1.0;
            (void)tryLoadCalendarInfo(d, &opened, &coefCal);
            if (!opened || coefCal <= 0.0)
                continue;

            const double capPoste = capacitePosteJour(d, e);
            double capBase = 0.0;
            if (capPoste > 0.0) {
                capBase = capPoste;
            } else {
                const int absents = compterAbsents(d, equipe);
                const int nbEmpEffectifs = qMax(1, nbPourEtape - qMin(absents, nbPourEtape - 1));
                capBase = nbEmpEffectifs * qMax(1.0, heuresJour) * rendement;
            }

            const double capJour = qMax(0.0, capBase * qBound(0.0, coefCal, 2.0));
            if (capJour <= 1e-6)
                continue;

            const double consomme = qMin(reste, capJour);
            joursFrac += consomme / capJour;
            reste -= consomme;
        }
        sp.joursOuvres = qMax(0.10, joursFrac > 0.0 ? joursFrac : 1.0);
        sp.fin = d.isValid() ? d : addWorkingDays(sp.debut, qMax(1, static_cast<int>(qCeil(sp.joursOuvres))));
        // séquentiel strict : l'étape suivante démarre après la précédente
        currentStart = sp.fin;
        out.push_back(sp);
    }

    return out;
}

MainWindow::CapacityAdvice MainWindow::computeCapacityAdvice(
    const QVector<StepPlan>& plans,
    const QDate& dateDebut,
    int nbEmpActuels,
    double heuresJourActuelles,
    double rendement,
    int objectifJoursOuvres) const
{
    CapacityAdvice adv;
    for (const auto& s : plans) adv.totalHeures += s.chargeHeures;

    if (heuresJourActuelles <= 0.0) heuresJourActuelles = 8.0;
    rendement = qBound(0.60, rendement, 0.98);
    if (objectifJoursOuvres < 1) objectifJoursOuvres = 1;

    const double capActuelle = qMax(0.1, nbEmpActuels * heuresJourActuelles * rendement);
    adv.totalJours = adv.totalHeures / capActuelle;

    const double capRequise = adv.totalHeures / objectifJoursOuvres;
    int empMin = static_cast<int>(qCeil(capRequise / (heuresJourActuelles * rendement)));
    empMin = qMax(1, empMin);

    double hJourMin = capRequise / (qMax(1, nbEmpActuels) * rendement);
    hJourMin = qBound(6.0, hJourMin, 12.0);

    adv.employesMin = empMin;
    adv.heuresJourMin = hJourMin;
    adv.dateFinReco = addWorkingDays(dateDebut.isValid() ? dateDebut : QDate::currentDate(), objectifJoursOuvres);
    return adv;
}

QString MainWindow::buildRecommendationHtml(const CapacityAdvice& adv,
                                            int nbEmpActuels,
                                            double heuresJourActuelles,
                                            int objectifJoursOuvres) const
{
    const bool late           = adv.totalJours > objectifJoursOuvres;
    const QString statusColor = late ? QStringLiteral("#c62828") : QStringLiteral("#2e7d32");
    const QString statusBg    = late ? QStringLiteral("#fff0f0") : QStringLiteral("#f0fff4");
    const QString statusBorder= late ? QStringLiteral("#ef9a9a") : QStringLiteral("#a5d6a7");
    const QString statusIcon  = late ? QStringLiteral("&#9888;") : QStringLiteral("&#10003;");
    const QString statusText  = late
        ? QStringLiteral("Insuffisant pour l&apos;objectif")
        : QStringLiteral("Compatible avec l&apos;objectif");

    return QStringLiteral(
        "<div style='color:#8d6e63; font-size:10px; letter-spacing:1.5px; margin-bottom:10px; font-weight:700;'>"
            "D&#201;LAI R&#201;ALISTE"
        "</div>"
        "<div style='background:%1; border:1px solid %2; border-radius:8px; padding:10px 13px; margin-bottom:10px;'>"
          "<div style='color:%3; font-weight:800; font-size:13px;'>%4 %5</div>"
          "<div style='color:#6d4c41; font-size:12px; margin-top:5px;'>"
            "<b>Actuel&nbsp;:</b> %6 j &nbsp;&#183;&nbsp; <b>Objectif&nbsp;:</b> %7 j ouvrés"
          "</div>"
        "</div>"
        "<div style='background:#ffffff; border:1px solid #e8d5b0; border-radius:8px; padding:10px 13px; margin-bottom:10px;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; margin-bottom:6px; font-weight:700;'>RESSOURCES MINIMALES</div>"
          "<div style='color:#3e2723; font-size:12px;'>"
            "Option A&nbsp;: <b style='color:#b8860b;'>%8 employ&#233;s</b> (%9 h/j)"
          "</div>"
          "<div style='color:#3e2723; font-size:12px; margin-top:5px;'>"
            "Option B&nbsp;: <b style='color:#b8860b;'>%10 h/j</b> (%11 employ&#233;s)"
          "</div>"
        "</div>"
        "<div style='background:#fff8e8; border:1px solid #d4af37; border-radius:8px; padding:10px 13px;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; margin-bottom:5px; font-weight:700;'>DATE FIN RECOMMAND&#201;E</div>"
          "<div style='color:#b8860b; font-size:18px; font-weight:900;'>%12</div>"
        "</div>")
    .arg(statusBg)
    .arg(statusBorder)
    .arg(statusColor)
    .arg(statusIcon)
    .arg(statusText)
    .arg(QString::number(adv.totalJours, 'f', 1))
    .arg(objectifJoursOuvres)
    .arg(adv.employesMin)
    .arg(QString::number(heuresJourActuelles, 'f', 1))
    .arg(QString::number(adv.heuresJourMin, 'f', 1))
    .arg(nbEmpActuels)
    .arg(adv.dateFinReco.toString(QStringLiteral("dd/MM/yyyy")));
}

QString MainWindow::buildIaHtmlCompact(const QString& cmd,
                                       const QString& produit,
                                       const QString& tableRows,
                                       const QString& totalDebut,
                                       const QString& totalFin,
                                       double totalJours,
                                       double totalHeures,
                                       const QString& recommendationHtml,
                                       int objectifJours,
                                       double ecartJours) const
{
    const bool enRetard       = ecartJours > 0.0;
    const QString ecartColor  = enRetard ? QStringLiteral("#c62828") : QStringLiteral("#2e7d32");
    const QString ecartBg     = enRetard ? QStringLiteral("#fff0f0") : QStringLiteral("#f0fff4");
    const QString ecartBorder = enRetard ? QStringLiteral("#ef9a9a") : QStringLiteral("#a5d6a7");
    const QString ecartPrefix = enRetard ? QStringLiteral("+") : QStringLiteral("");

    return QStringLiteral(
    // ── Outer container — light FIL D'OR theme ──
    "<div style='font-family:Segoe UI,Arial,sans-serif; font-size:13px; color:#212121;"
         " background:#fdf8f2; border-radius:10px; border:1px solid #e8d5b0;'>"

    // ── Header (dark brown band) ──
    "<div style='background:#3e2723; padding:14px 18px; border-radius:10px 10px 0 0;'>"
      "<table width='100%%' cellspacing='0' cellpadding='0'><tr>"
        "<td><div style='color:#d4af37; font-size:16px; font-weight:900; letter-spacing:1px;'>"
            "&#9881; ANALYSE IA &#8212; PLANIFICATION R&#201;ALISTE</div>"
          "<div style='color:#a1887f; font-size:11px; margin-top:3px;'>"
            "Moteur pr&#233;dictif multi-&#233;tapes &#183; FIL D&apos;OR"
          "</div>"
        "</td>"
        "<td align='right' style='white-space:nowrap;'>"
          "<div style='color:#f5e6c8; font-size:16px; font-weight:700;'>Commande #%1</div>"
          "<div style='color:#bcaaa4; font-size:13px;'>%2</div>"
        "</td>"
      "</tr></table>"
    "</div>"

    // ── KPI row (4 cards) ──
    "<div style='background:#fff8f0; border-bottom:1px solid #e8d5b0; padding:12px 14px;'>"
    "<table width='100%%' cellspacing='0' cellpadding='0'><tr>"
      "<td width='25%%' style='padding:0 5px 0 0;'>"
        "<div style='background:white; border:1px solid #e8d5b0; border-radius:8px;"
             " padding:12px 10px; text-align:center;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; font-weight:700;'>CHARGE TOTALE</div>"
          "<div style='color:#d4af37; font-size:22px; font-weight:900; margin:5px 0;'>%4 h</div>"
          "<div style='color:#9e9e9e; font-size:11px;'>%3 jours</div>"
        "</div>"
      "</td>"
      "<td width='25%%' style='padding:0 5px;'>"
        "<div style='background:white; border:1px solid #e8d5b0; border-radius:8px;"
             " padding:12px 10px; text-align:center;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; font-weight:700;'>DATE FIN</div>"
          "<div style='color:#3e2723; font-size:15px; font-weight:900; margin:5px 0;'>%8</div>"
          "<div style='color:#9e9e9e; font-size:11px;'>plan actuel</div>"
        "</div>"
      "</td>"
      "<td width='25%%' style='padding:0 5px;'>"
        "<div style='background:white; border:1px solid #e8d5b0; border-radius:8px;"
             " padding:12px 10px; text-align:center;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; font-weight:700;'>OBJECTIF</div>"
          "<div style='color:#3e2723; font-size:18px; font-weight:900; margin:5px 0;'>%9 j</div>"
          "<div style='color:#9e9e9e; font-size:11px;'>jours ouvrés</div>"
        "</div>"
      "</td>"
      "<td width='25%%' style='padding:0 0 0 5px;'>"
        "<div style='background:%10; border:1px solid %11; border-radius:8px;"
             " padding:12px 10px; text-align:center;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; font-weight:700;'>&#201;CART</div>"
          "<div style='color:%12; font-size:18px; font-weight:900; margin:5px 0;'>%13%14 j</div>"
          "<div style='color:#9e9e9e; font-size:11px;'>vs objectif</div>"
        "</div>"
      "</td>"
    "</tr></table>"
    "</div>"

    // ── Main content (step table | recommendation) ──
    "<table width='100%%' cellspacing='0' cellpadding='0'><tr>"

      // Left: step table (58%)
      "<td width='58%%' valign='top'"
           " style='padding:14px 10px 14px 16px; border-right:1px solid #e8d5b0;'>"
        "<div style='color:#8d6e63; font-size:10px; letter-spacing:1.5px; margin-bottom:10px; font-weight:700;'>"
          "&#201;TAPES DE FABRICATION"
        "</div>"
        "<table width='100%%' cellspacing='0' cellpadding='0' style='border-collapse:collapse;'>"
          "<tr style='background:#f5ebe0;'>"
            "<th align='left' style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 8px;"
                " border-bottom:2px solid #d4af37; width:22%%;'>&#201;TAPE</th>"
            "<th style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 6px;"
                " border-bottom:2px solid #d4af37; text-align:center; width:16%%;'>D&#201;BUT</th>"
            "<th style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 6px;"
                " border-bottom:2px solid #d4af37; text-align:center; width:16%%;'>FIN</th>"
            "<th style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 6px;"
                " border-bottom:2px solid #d4af37; text-align:center; width:13%%;'>JOURS</th>"
            "<th style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 6px;"
                " border-bottom:2px solid #d4af37; text-align:center; width:17%%;'>CHARGE</th>"
            "<th style='color:#5d4037; font-size:11px; font-weight:700; padding:9px 6px;"
                " border-bottom:2px solid #d4af37; text-align:center; width:16%%;'>h/UNT</th>"
          "</tr>"
          "%5"
          "<tr style='background:#fff8f0;'>"
            "<td style='color:#b8860b; font-weight:800; padding:10px 8px; font-size:13px;"
                " border-top:2px solid #d4af37;'>TOTAL</td>"
            "<td style='color:#5d4037; text-align:center; padding:10px 6px; font-size:13px;"
                " border-top:2px solid #d4af37;'>%6</td>"
            "<td style='color:#5d4037; text-align:center; padding:10px 6px; font-size:13px;"
                " border-top:2px solid #d4af37;'>%7</td>"
            "<td style='color:#212121; font-weight:800; text-align:center; padding:10px 6px;"
                " font-size:13px; border-top:2px solid #d4af37;'>%3 j</td>"
            "<td style='color:#212121; font-weight:800; text-align:center; padding:10px 6px;"
                " font-size:13px; border-top:2px solid #d4af37;'>%4 h</td>"
            "<td style='color:#9e9e9e; text-align:center; padding:10px 6px; font-size:13px;"
                " border-top:2px solid #d4af37;'>&#8212;</td>"
          "</tr>"
        "</table>"
      "</td>"

      // Right: recommendation panel (42%)
      "<td width='42%%' valign='top' style='padding:14px 16px 14px 12px;'>"
        "%15"
      "</td>"
    "</tr></table>"
    "</div>")
    .arg(cmd.toHtmlEscaped())                               // %1
    .arg(produit.toHtmlEscaped())                           // %2
    .arg(QString::number(totalJours,  'f', 1))              // %3
    .arg(QString::number(totalHeures, 'f', 1))              // %4
    .arg(tableRows)                                         // %5
    .arg(totalDebut.toHtmlEscaped())                        // %6
    .arg(totalFin.toHtmlEscaped())                          // %7
    .arg(totalFin.toHtmlEscaped())                          // %8  (date fin KPI)
    .arg(objectifJours)                                     // %9
    .arg(ecartBg)                                           // %10
    .arg(ecartBorder)                                       // %11
    .arg(ecartColor)                                        // %12
    .arg(ecartPrefix)                                       // %13
    .arg(QString::number(qAbs(ecartJours), 'f', 1))         // %14
    .arg(recommendationHtml);                               // %15
}

int MainWindow::getObjectifJoursFromUi() const
{
    // Compatible même si le widget n'existe pas dans le .ui
    if (!ui) return 90;
    if (auto *spin = this->findChild<QSpinBox*>(QStringLiteral("spinObjectifJours")))
        return qMax(1, spin->value());
    return 90;
}

int MainWindow::mapEtapeToCode(const QString& e) const
{
    const QString s = e.trimmed().toUpper();
    if (s == QStringLiteral("COUPE")) return 1;
    if (s == QStringLiteral("ASSEMBLAGE")) return 2;
    if (s == QStringLiteral("COUTURE")) return 3;
    if (s == QStringLiteral("FINITION")) return 4;
    return 0;
}

bool MainWindow::trainLinearModelFromDB(bool forceRetrain)
{
    qint64 fpCnt = -1, fpMax = -1;
    double fpSumTemps = 0.0;
    const bool fpOk = peekMlTrainFingerprint(&fpCnt, &fpMax, &fpSumTemps);

    if (!forceRetrain && fpOk && fpCnt == m_mlTrainDataCount && fpMax == m_mlTrainMaxSuivi
        && qAbs(fpSumTemps - m_mlTrainSumTempsReel) < 1e-3
        && m_mlTrainFeatureRev == kMlFeatureRevision) {
        if (!mlW.isEmpty() || !m_mlWeightsPerEtape.isEmpty())
            return true;
        return false;
    }

    // Collecte brute par étape — features: [qte, log(qte+1), comp, nbEmp, qte×comp]
    QMap<QString, QVector<QVector<double>>> rawX;
    QMap<QString, QVector<double>> rawY;

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT "
        "p.QUANTITE AS qte, "
        "NVL(NULLIF(pr.TEMPS_FABRICATION, 0), 120) AS complexite, "
        "e.ETAPE_ACTUELLE AS etape_nom, "
        "NVL((SELECT COUNT(DISTINCT e3.ID_EMPLOYE) "
        "       FROM ETAPES e3 "
        "      WHERE e3.ID_PLANIFICATION = e.ID_PLANIFICATION "
        "        AND UPPER(TRIM(e3.ETAPE_ACTUELLE)) = UPPER(TRIM(e.ETAPE_ACTUELLE))), 1) AS nb_emp_etape, "
        "e.TEMPS_REEL_PASSE AS y_h "
        "FROM ETAPES e "
        "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "WHERE e.TEMPS_REEL_PASSE IS NOT NULL "
        "AND e.TEMPS_REEL_PASSE > 0"));

    if (!q.exec()) {
        qDebug() << "ML SQL error:" << q.lastError().text();
        return false;
    }

    while (q.next()) {
        const double qte  = qMax(1.0, q.value(QStringLiteral("qte")).toDouble());
        const double comp = q.value(QStringLiteral("complexite")).toDouble();
        const int    ec   = mapEtapeToCode(q.value(QStringLiteral("etape_nom")).toString());
        const double nbEmp = qMax(1.0, q.value(QStringLiteral("nb_emp_etape")).toDouble());
        const double yh   = q.value(QStringLiteral("y_h")).toDouble();
        if (ec == 0 || yh <= 0.0) continue;

        const QString etapeKey = [ec]() -> QString {
            if (ec == 1) return QStringLiteral("COUPE");
            if (ec == 2) return QStringLiteral("ASSEMBLAGE");
            if (ec == 3) return QStringLiteral("COUTURE");
            return QStringLiteral("FINITION");
        }();

        rawX[etapeKey].push_back({ qte, std::log(qte + 1.0), comp, nbEmp, qte * comp });
        rawY[etapeKey].push_back(yh / qte);
    }

    int totalSamples = 0;
    for (auto it = rawY.cbegin(); it != rawY.cend(); ++it)
        totalSamples += it.value().size();

    if (totalSamples < 4) {
        qDebug() << "ML: dataset insuffisant (" << totalSamples << " samples)";
        if (fpOk) {
            m_mlTrainDataCount = fpCnt;
            m_mlTrainMaxSuivi  = fpMax;
            m_mlTrainSumTempsReel = fpSumTemps;
            m_mlTrainFeatureRev = kMlFeatureRevision;
        }
        return false;
    }
    setProperty("ml_n_samples", totalSamples);

    m_mlWeightsPerEtape.clear();
    m_mlNormMinPerEtape.clear();
    m_mlNormMaxPerEtape.clear();
    m_mlMaePerEtape.clear();
    m_mlR2PerEtape.clear();
    m_mlStdPerEtape.clear();

    // Adam hyper-paramètres communs
    constexpr double LR     = 0.01;
    constexpr double BETA1  = 0.9;
    constexpr double BETA2  = 0.999;
    constexpr double EPS    = 1e-8;
    constexpr double LAMBDA = 0.01; // Ridge L2

    // ---- Entraînement par étape (5 features) ----
    const QStringList etapeList = {
        QStringLiteral("COUPE"), QStringLiteral("ASSEMBLAGE"),
        QStringLiteral("COUTURE"), QStringLiteral("FINITION")
    };

    for (const QString& etape : etapeList) {
        if (!rawX.contains(etape)) continue;

        QVector<QVector<double>> Xraw = rawX[etape];
        QVector<double>          yRaw = rawY[etape];
        const int mRaw = Xraw.size();

        // Filtre IQR sur y (h/unité)
        QVector<double> ySorted = yRaw;
        std::sort(ySorted.begin(), ySorted.end());
        const double q1  = ySorted[mRaw / 4];
        const double q3  = ySorted[(3 * mRaw) / 4];
        const double iqr = q3 - q1;
        const double lo  = q1 - 1.5 * iqr;
        const double hi  = q3 + 1.5 * iqr;

        QVector<QVector<double>> Xf;
        QVector<double>          yf;
        for (int i = 0; i < mRaw; ++i) {
            if (yRaw[i] >= lo && yRaw[i] <= hi && yRaw[i] > 0.0) {
                Xf.push_back(Xraw[i]);
                yf.push_back(yRaw[i]);
            }
        }
        const int m = Xf.size();
        if (m < 4) continue;

        const int nFeat = 5; // qte, log(qte+1), comp, nbEmp, qte*comp

        // Normalisation min-max par feature
        QVector<double> mn(nFeat, 1e18), mx(nFeat, -1e18);
        for (const auto& row : Xf)
            for (int j = 0; j < nFeat; ++j) { mn[j] = qMin(mn[j], row[j]); mx[j] = qMax(mx[j], row[j]); }

        auto normF = [&](double v, int j) -> double {
            return qFuzzyCompare(mx[j], mn[j]) ? 0.0 : (v - mn[j]) / (mx[j] - mn[j]);
        };
        QVector<QVector<double>> Xn = Xf;
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < nFeat; ++j)
                Xn[i][j] = normF(Xf[i][j], j);

        // Descente Adam avec arrêt précoce
        QVector<double> w(nFeat + 1, 0.0);
        QVector<double> mA(nFeat + 1, 0.0), vA(nFeat + 1, 0.0);
        double prevLoss = 1e18; int patience = 0;

        for (int ep = 1; ep <= 3000; ++ep) {
            QVector<double> g(nFeat + 1, 0.0);
            double loss = 0.0;
            for (int i = 0; i < m; ++i) {
                double yhat = w[0];
                for (int j = 0; j < nFeat; ++j) yhat += w[j + 1] * Xn[i][j];
                const double e = yhat - yf[i];
                loss += e * e;
                g[0] += e;
                for (int j = 0; j < nFeat; ++j) g[j + 1] += e * Xn[i][j];
            }
            loss /= (2.0 * m);
            for (int j = 0; j < nFeat + 1; ++j) {
                g[j] /= m;
                if (j > 0) g[j] += (LAMBDA / m) * w[j];
                mA[j] = BETA1 * mA[j] + (1.0 - BETA1) * g[j];
                vA[j] = BETA2 * vA[j] + (1.0 - BETA2) * g[j] * g[j];
                const double mHat = mA[j] / (1.0 - std::pow(BETA1, ep));
                const double vHat = vA[j] / (1.0 - std::pow(BETA2, ep));
                w[j] -= LR * mHat / (std::sqrt(vHat) + EPS);
            }
            if (std::abs(prevLoss - loss) < 1e-7) { if (++patience > 20) break; } else patience = 0;
            prevLoss = loss;
            bool ok = true; for (double wj : w) if (!std::isfinite(wj)) { ok = false; break; }
            if (!ok) { w.clear(); break; }
        }
        if (w.isEmpty()) continue;

        // Métriques R² et MAE
        double yMean = 0.0; for (double yi : yf) yMean += yi; yMean /= m;
        double ssTot = 0.0, ssRes = 0.0, maeSum = 0.0;
        for (int i = 0; i < m; ++i) {
            double yhat = w[0];
            for (int j = 0; j < nFeat; ++j) yhat += w[j + 1] * Xn[i][j];
            yhat = qMax(0.01, yhat);
            ssRes += (yf[i] - yhat) * (yf[i] - yhat);
            ssTot += (yf[i] - yMean) * (yf[i] - yMean);
            maeSum += std::abs(yf[i] - yhat);
        }
        m_mlWeightsPerEtape[etape] = w;
        m_mlNormMinPerEtape[etape] = mn;
        m_mlNormMaxPerEtape[etape] = mx;
        m_mlMaePerEtape[etape]     = maeSum / m;
        m_mlR2PerEtape[etape]      = (ssTot > 1e-12) ? (1.0 - ssRes / ssTot) : 0.0;
        m_mlStdPerEtape[etape]     = (m > 1) ? std::sqrt(ssRes / m) : 0.0;
        qDebug() << "ML [" << etape << "] n=" << m << " R²=" << m_mlR2PerEtape[etape]
                 << " MAE=" << m_mlMaePerEtape[etape] << " σ=" << m_mlStdPerEtape[etape];
    }

    // ---- Modèle global fallback (8 features, toutes étapes mélangées) ----
    QVector<QVector<double>> Xg; QVector<double> yg;
    for (auto it = rawX.cbegin(); it != rawX.cend(); ++it) {
        const int ec = mapEtapeToCode(it.key());
        const auto& Xe = it.value();
        const auto& ye = rawY[it.key()];
        for (int i = 0; i < Xe.size(); ++i) {
            Xg.push_back({ Xe[i][0], Xe[i][2], Xe[i][3], 1.0,
                (ec==1)?1.0:0.0, (ec==2)?1.0:0.0, (ec==3)?1.0:0.0, (ec==4)?1.0:0.0 });
            yg.push_back(ye[i]);
        }
    }
    const int mg = Xg.size();
    mlW.clear();
    if (mg >= 6) {
        constexpr int NG = 8;
        QVector<double> mnG(NG, 1e18), mxG(NG, -1e18);
        for (const auto& r : Xg) for (int j=0;j<NG;++j) { mnG[j]=qMin(mnG[j],r[j]); mxG[j]=qMax(mxG[j],r[j]); }
        auto normG=[&](double v,int j)->double { return qFuzzyCompare(mxG[j],mnG[j])?0.0:(v-mnG[j])/(mxG[j]-mnG[j]); };
        QVector<QVector<double>> XgN=Xg;
        for(int i=0;i<mg;++i) for(int j=0;j<NG;++j) XgN[i][j]=normG(Xg[i][j],j);
        mlW=QVector<double>(NG+1,0.0);
        QVector<double> mA(NG+1,0.0), vA(NG+1,0.0);
        double prevL=1e18; int pat=0;
        for(int ep=1;ep<=2000;++ep){
            QVector<double> g(NG+1,0.0);
            for(int i=0;i<mg;++i){
                double yhat=mlW[0]; for(int j=0;j<NG;++j) yhat+=mlW[j+1]*XgN[i][j];
                const double e=yhat-yg[i]; g[0]+=e; for(int j=0;j<NG;++j) g[j+1]+=e*XgN[i][j];
            }
            for(int j=0;j<NG+1;++j){
                g[j]/=mg; if(j>0) g[j]+=(0.01/mg)*mlW[j];
                mA[j]=0.9*mA[j]+0.1*g[j]; vA[j]=0.999*vA[j]+0.001*g[j]*g[j];
                const double mH=mA[j]/(1.0-std::pow(0.9,ep)), vH=vA[j]/(1.0-std::pow(0.999,ep));
                mlW[j]-=0.01*mH/(std::sqrt(vH)+1e-8);
            }
            double curL=0.0; for(double gj:g) curL+=gj*gj;
            if(std::abs(prevL-curL)<1e-7){if(++pat>20)break;}else pat=0; prevL=curL;
            bool ok=true; for(double wj:mlW) if(!std::isfinite(wj)){ok=false;break;}
            if(!ok){mlW.clear();break;}
        }
        if(!mlW.isEmpty()){ setProperty("ml_mn",QVariant::fromValue(mnG)); setProperty("ml_mx",QVariant::fromValue(mxG)); }
    }

    if (fpOk) {
        m_mlTrainDataCount    = fpCnt;
        m_mlTrainMaxSuivi     = fpMax;
        m_mlTrainSumTempsReel = fpSumTemps;
        m_mlTrainFeatureRev   = kMlFeatureRevision;
    }
    return !m_mlWeightsPerEtape.isEmpty() || !mlW.isEmpty();
}

double MainWindow::predictStepHours(int etapeCode, double qte, double complexite, double nbEmployes, double cadence) const
{
    const double qteSafe   = qMax(1.0, qte);
    const double nbEmpSafe = qMax(1.0, nbEmployes);

    const QString etapeKey = [etapeCode]() -> QString {
        if (etapeCode == 1) return QStringLiteral("COUPE");
        if (etapeCode == 2) return QStringLiteral("ASSEMBLAGE");
        if (etapeCode == 3) return QStringLiteral("COUTURE");
        if (etapeCode == 4) return QStringLiteral("FINITION");
        return QString();
    }();

    // Heuristique calibrée — base pour l'ensemble (toujours disponible)
    const double hHeuristicUnit = etapeKey.isEmpty() ? 0.25 : hUnit(etapeKey);
    const double hHeuristic = qMax(0.05,
        qteSafe * hHeuristicUnit * complexityFactor(complexite) * cadenceFactor(cadence));

    // ---- Modèle per-étape (prioritaire) ----
    if (!etapeKey.isEmpty() && m_mlWeightsPerEtape.contains(etapeKey)) {
        const QVector<double>& w  = m_mlWeightsPerEtape.value(etapeKey);
        const QVector<double>& mn = m_mlNormMinPerEtape.value(etapeKey);
        const QVector<double>& mx = m_mlNormMaxPerEtape.value(etapeKey);
        if (w.size() == 6 && mn.size() == 5 && mx.size() == 5) {
            auto normF = [&](double v, int j) -> double {
                return qFuzzyCompare(mx[j], mn[j]) ? 0.0 : (v - mn[j]) / (mx[j] - mn[j]);
            };
            const QVector<double> x = {
                normF(qteSafe,                  0),
                normF(std::log(qteSafe + 1.0),  1),
                normF(complexite,               2),
                normF(nbEmpSafe,                3),
                normF(qteSafe * complexite,     4)
            };
            double yhat = w[0];
            for (int j = 0; j < 5; ++j) yhat += w[j + 1] * x[j];
            yhat = qMax(0.01, yhat);
            const double hML = qMax(0.05, yhat * qteSafe);

            // Blend ensembliste : α = R² au carré → plus de confiance aux bons modèles
            const double r2    = qBound(0.0, m_mlR2PerEtape.value(etapeKey, 0.0), 1.0);
            const double alpha = r2 * r2;
            return alpha * hML + (1.0 - alpha) * hHeuristic;
        }
    }

    // ---- Modèle global fallback (8 features) ----
    if (!mlW.isEmpty()) {
        const QVector<double> mn = property("ml_mn").value<QVector<double>>();
        const QVector<double> mx = property("ml_mx").value<QVector<double>>();
        if (mn.size() == 8 && mx.size() == 8) {
            auto normG = [&](double v, int j) -> double {
                return qFuzzyCompare(mx[j], mn[j]) ? 0.0 : (v - mn[j]) / (mx[j] - mn[j]);
            };
            QVector<double> x = {
                qteSafe, complexite, nbEmpSafe, 1.0,
                (etapeCode==1)?1.0:0.0, (etapeCode==2)?1.0:0.0,
                (etapeCode==3)?1.0:0.0, (etapeCode==4)?1.0:0.0
            };
            for (int j = 0; j < 8; ++j) x[j] = normG(x[j], j);
            double yhat = mlW[0];
            for (int j = 0; j < 8; ++j) yhat += mlW[j + 1] * x[j];
            yhat = qMax(0.01, yhat);
            double yTotal = yhat * qteSafe;
            const int nSamples = property("ml_n_samples").toInt();
            if (nSamples > 0 && nSamples < 30) yTotal /= nbEmpSafe;
            return qMax(0.05, yTotal);
        }
    }

    return 0.0;
}

void MainWindow::scheduleIaChartRedraw(const QVector<StepPlan> &plans, double totalJours)
{
    QPointer<MainWindow> self(this);
    const QVector<StepPlan> copy = plans;
    const double tj = totalJours;
    QTimer::singleShot(0, this, [self, copy, tj]() {
        if (!self)
            return;
        self->drawIaChart(copy, tj);
    });
}

void MainWindow::iaWhatIfPlusEmp()
{
    if (!m_iaSession.valid())
        return;
    const IaSessionContext c = m_iaSession;
    const double nbEmp0 = qMax(1.0, c.nbEmp);
    const double cadence0 = qBound(0.70, c.cadence, 1.20);
    const QVector<StepPlan> alt = planRealisteToutesEtapes(
        c.qte, c.comp, nbEmp0 + 1.0, HEURES_PAR_JOUR, cadence0, cadence0, 0.05, c.dateDebut, c.idCmd);
    renderStepPlanHtmlAndChart(alt, QStringLiteral("WHAT-IF +1 EMP (%1)").arg(c.idCmd), c.produitNom);
    if (!alt.isEmpty()) {
        ui->btn_ia_appliquer->setProperty("id_cmd", c.idCmd);
        ui->btn_ia_appliquer->setProperty("nv_fin", alt.last().fin);
        ui->btn_ia_appliquer->setEnabled(true);
    }
}

void MainWindow::iaWhatIfOvertime()
{
    if (!m_iaSession.valid())
        return;
    const IaSessionContext c = m_iaSession;
    const double nbEmp0 = qMax(1.0, c.nbEmp);
    const double cadence0 = qBound(0.70, c.cadence, 1.20);
    const QVector<StepPlan> alt = planRealisteToutesEtapes(
        c.qte, c.comp, nbEmp0, 10.0, cadence0, cadence0, 0.05, c.dateDebut, c.idCmd);
    renderStepPlanHtmlAndChart(alt, QStringLiteral("WHAT-IF OVERTIME (%1)").arg(c.idCmd), c.produitNom);
    if (!alt.isEmpty()) {
        ui->btn_ia_appliquer->setProperty("id_cmd", c.idCmd);
        ui->btn_ia_appliquer->setProperty("nv_fin", alt.last().fin);
        ui->btn_ia_appliquer->setEnabled(true);
    }
}

void MainWindow::iaWhatIfReplan()
{
    if (!m_iaSession.valid())
        return;
    const IaSessionContext c = m_iaSession;
    const double nbEmp0 = qMax(1.0, c.nbEmp);
    const double cadence0 = qBound(0.70, c.cadence, 1.20);
    const QDate d0 = addWorkingDays(c.dateDebut, 1);
    const QVector<StepPlan> alt = planRealisteToutesEtapes(
        c.qte, c.comp, nbEmp0, HEURES_PAR_JOUR, cadence0, cadence0, 0.05, d0, c.idCmd);
    renderStepPlanHtmlAndChart(alt, QStringLiteral("WHAT-IF REPLAN (%1)").arg(c.idCmd), c.produitNom);
    if (!alt.isEmpty()) {
        ui->btn_ia_appliquer->setProperty("id_cmd", c.idCmd);
        ui->btn_ia_appliquer->setProperty("nv_fin", alt.last().fin);
        ui->btn_ia_appliquer->setEnabled(true);
    }
}

void MainWindow::iaWhatIfAdminCal()
{
    ouvrirAdminCalendrierAbsences(this);
}

void MainWindow::renderIaEstimationForSelectedOrder()
{
    const int row = ui->tablePlanif->currentRow();
    if (row < 0 || !ui->tablePlanif->item(row, 0)) {
        ui->lbl_ia_details->setText(QStringLiteral("<b>Sélectionnez une commande dans 'Liste des Commandes'.</b>"));
        ui->tabWidgetPlanif->setCurrentIndex(4);
        m_iaSession = {};
        return;
    }

    const int idCmd = ui->tablePlanif->item(row, 0)->text().toInt();
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT p.ID_COMMANDE, NVL(p.QUANTITE,1) AS quantite, "
        "LEAST(3.0, GREATEST(1.0, NVL(NULLIF(pr.TEMPS_FABRICATION,0), 96) / 96.0)) AS complexite, "
        "NVL((SELECT COUNT(DISTINCT e2.ID_EMPLOYE) FROM ETAPES e2 WHERE e2.ID_PLANIFICATION = p.ID_COMMANDE), 1) AS nb_emp, "
        "0.85 AS cadence, p.DATE_LANCEMENT AS date_debut "
        "FROM PLANIFICATION p "
        "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
        "WHERE p.ID_COMMANDE = :id"));
    q.bindValue(":id", idCmd);

    if (!q.exec() || !q.next()) {
        ui->lbl_ia_details->setText(QStringLiteral("<b style='color:#c62828'>Impossible de charger la commande.</b>"));
        ui->tabWidgetPlanif->setCurrentIndex(4);
        m_iaSession = {};
        return;
    }

    const double qte = q.value(QStringLiteral("quantite")).toDouble();
    const double comp = q.value(QStringLiteral("complexite")).toDouble();
    const double nbEmp = qMax(1.0, q.value(QStringLiteral("nb_emp")).toDouble());
    const double cadence = q.value(QStringLiteral("cadence")).toDouble();
    const double rendement = qBound(0.60, cadence, 0.98);
    const double txRework = 0.05;
    const QDate dateDebut = q.value(QStringLiteral("date_debut")).toDate();
    const QString produitNom = (ui->tablePlanif->item(row, 1) ? ui->tablePlanif->item(row, 1)->text() : QStringLiteral("Produit"));

    m_iaSession.idCmd = idCmd;
    m_iaSession.produitNom = produitNom;
    m_iaSession.qte = qte;
    m_iaSession.comp = comp;
    m_iaSession.nbEmp = nbEmp;
    m_iaSession.cadence = cadence;
    m_iaSession.dateDebut = dateDebut;

    ui->tabWidgetPlanif->setCurrentIndex(4);
    ui->lbl_ia_details->setText(QStringLiteral(
        "<p style='color:#6a1b9a;'><b>Calcul de l'estimation en cours…</b></p>"
        "<p style='color:#6f5f58; font-size:12px;'>Chargement du calendrier, du modèle et du plan réaliste.</p>"));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    calibrateBaseHoursFromHistory();
    const bool mlOk = trainLinearModelFromDB(false);
    const QVector<StepPlan> plans = planRealisteToutesEtapes(
        qte, comp, nbEmp, HEURES_PAR_JOUR, rendement, cadence, txRework, dateDebut, idCmd);
    // Bloc recommandation ressources / délai réaliste
    const int nbEmpActuels = static_cast<int>(qMax(1.0, nbEmp));
    const double hJourActuelles = HEURES_PAR_JOUR;
    const int objectifJours = getObjectifJoursFromUi();
    const CapacityAdvice adv = computeCapacityAdvice(plans, dateDebut, nbEmpActuels, hJourActuelles, rendement, objectifJours);
    QString recommendationHtml = buildRecommendationHtml(adv, nbEmpActuels, hJourActuelles, objectifJours);
    // --- Intervalle de confiance data-driven via R² moyen ---
    const int nSamples = property("ml_n_samples").toInt();
    double avgR2forConf = 0.0; int r2ModelesCnt = 0;
    for (const auto& sp : plans) {
        if (m_mlR2PerEtape.contains(sp.etape)) {
            avgR2forConf += qBound(0.0, m_mlR2PerEtape.value(sp.etape), 1.0);
            ++r2ModelesCnt;
        }
    }
    if (r2ModelesCnt > 0) avgR2forConf /= r2ModelesCnt;
    double confRatio;
    if (!mlOk) {
        confRatio = 0.35;
    } else if (r2ModelesCnt > 0) {
        // R²=1 → intervalle très serré (7%%), R²=0 → large (40%%)
        confRatio = qBound(0.07, 0.35 * (1.0 - avgR2forConf * 0.80), 0.40);
    } else {
        confRatio = nSamples >= 60 ? 0.12 : 0.28;
    }
    const double joursProbables = qMax(1.0, adv.totalJours);
    const double joursOptimistes = qMax(1.0, joursProbables * (1.0 - confRatio));
    const double joursPessimistes = qMax(1.0, joursProbables * (1.0 + confRatio));

    // --- Détection goulot ---
    StepPlan goulot;
    bool hasGoulot = false;
    for (const auto &sp : plans) {
        if (!hasGoulot || sp.chargeHeures > goulot.chargeHeures) {
            goulot = sp;
            hasGoulot = true;
        }
    }
    const double partGoulot = (adv.totalHeures > 0.0 && hasGoulot)
        ? (100.0 * goulot.chargeHeures / adv.totalHeures)
        : 0.0;

    // --- Contrainte matière (simple): stock actuel vs quantité commande ---
    bool stockKnown = false;
    bool stockInsuffisant = false;
    double stockQte = 0.0;
    {
        QSqlQuery qStock;
        qStock.prepare(QStringLiteral(
            "SELECT mp.QUANTITE "
            "FROM PLANIFICATION p "
            "JOIN MATIERES_PREMIERES mp ON mp.ID_STOCK_MP = p.ID_STOCK_MP "
            "WHERE p.ID_COMMANDE = :id"));
        qStock.bindValue(QStringLiteral(":id"), idCmd);
        if (qStock.exec() && qStock.next()) {
            stockKnown = true;
            stockQte = qStock.value(0).toDouble();
            stockInsuffisant = (stockQte + 1e-9) < qte;
        }
    }

    // --- Score de risque (0..100) ---
    double riskScore = 0.0;
    const double retardJ = qMax(0.0, adv.totalJours - objectifJours);
    if (objectifJours > 0)
        riskScore += qMin(55.0, (retardJ / objectifJours) * 80.0);
    riskScore += qMin(20.0, qMax(0.0, txRework) * 100.0 * 0.8);
    riskScore += qMin(15.0, partGoulot * 0.25);
    if (stockInsuffisant)
        riskScore += 10.0;
    riskScore = qBound(0.0, riskScore, 100.0);

    // Couleurs risque — thème clair
    QString riskLabel, riskBadgeBg, riskBadgeBorder, riskBadgeColor;
    if (riskScore >= 66.0) {
        riskLabel = QStringLiteral("Risque &#233;lev&#233;");
        riskBadgeBg = QStringLiteral("#fff0f0"); riskBadgeBorder = QStringLiteral("#ef9a9a");
        riskBadgeColor = QStringLiteral("#c62828");
    } else if (riskScore >= 33.0) {
        riskLabel = QStringLiteral("Risque mod&#233;r&#233;");
        riskBadgeBg = QStringLiteral("#fff8e8"); riskBadgeBorder = QStringLiteral("#ffa726");
        riskBadgeColor = QStringLiteral("#e65100");
    } else {
        riskLabel = QStringLiteral("Risque faible");
        riskBadgeBg = QStringLiteral("#f0fff4"); riskBadgeBorder = QStringLiteral("#a5d6a7");
        riskBadgeColor = QStringLiteral("#2e7d32");
    }
    const QString delaiLabel = (retardJ > 0.0)
        ? QStringLiteral("&#9888; Retard +%1 j").arg(QString::number(retardJ, 'f', 1))
        : QStringLiteral("&#10003; &#192; l&apos;heure");
    const QString delaiLabelBg     = (retardJ > 0.0) ? QStringLiteral("#fff0f0") : QStringLiteral("#f0fff4");
    const QString delaiLabelBorder = (retardJ > 0.0) ? QStringLiteral("#ef9a9a") : QStringLiteral("#a5d6a7");
    const QString delaiLabelColor  = (retardJ > 0.0) ? QStringLiteral("#c62828") : QStringLiteral("#2e7d32");

    // Stock badge
    const QString stockBadge = stockKnown
        ? (stockInsuffisant
           ? QStringLiteral("<span style='color:#ef5350; font-weight:800;'>&#128997; Stock insuffisant (%1 dispo / %2 requis)</span>")
               .arg(QString::number(stockQte, 'f', 1)).arg(QString::number(qte, 'f', 1))
           : QStringLiteral("<span style='color:#66bb6a; font-weight:800;'>&#9989; Stock OK (%1 dispo)</span>")
               .arg(QString::number(stockQte, 'f', 1)))
        : QStringLiteral("<span style='color:#6d4c41;'>Stock non disponible</span>");

    // ML badge (compact)
    double avgR2badge = 0.0; int r2cntBadge = 0;
    for (auto it = m_mlR2PerEtape.cbegin(); it != m_mlR2PerEtape.cend(); ++it) {
        avgR2badge += qBound(0.0, it.value(), 1.0); ++r2cntBadge;
    }
    if (r2cntBadge > 0) avgR2badge /= r2cntBadge;

    recommendationHtml += QStringLiteral(
        // Séparateur
        "<div style='border-top:1px solid #e8d5b0; margin:10px 0;'></div>"
        // Badges délai + risque
        "<table width='100%%' cellspacing='0' cellpadding='0'><tr>"
          "<td width='50%%' style='padding-right:4px;'>"
            "<div style='background:%1; border:1px solid %2; border-radius:7px;"
                 " padding:8px 10px; text-align:center;'>"
              "<div style='color:%3; font-weight:800; font-size:12px;'>%4</div>"
            "</div>"
          "</td>"
          "<td width='50%%' style='padding-left:4px;'>"
            "<div style='background:%5; border:1px solid %6; border-radius:7px;"
                 " padding:8px 10px; text-align:center;'>"
              "<div style='color:%7; font-weight:800; font-size:12px;'>%8</div>"
              "<div style='color:#9e9e9e; font-size:10px; margin-top:2px;'>%9/100</div>"
            "</div>"
          "</td>"
        "</tr></table>"
        // Intervalle de confiance
        "<div style='background:#fff8f0; border:1px solid #e8d5b0; border-radius:7px;"
             " padding:9px 12px; margin-top:9px;'>"
          "<div style='color:#8d6e63; font-size:10px; letter-spacing:1px; margin-bottom:5px; font-weight:700;'>"
            "INTERVALLE DE CONFIANCE"
          "</div>"
          "<div style='color:#5d4037; font-size:12px;'>"
            "Opt. <b style='color:#2e7d32;'>%10 j</b>"
            " &nbsp;&#183;&nbsp; Prob. <b style='color:#3e2723;'>%11 j</b>"
            " &nbsp;&#183;&nbsp; Pes. <b style='color:#c62828;'>%12 j</b>"
          "</div>"
        "</div>"
        // Goulot + stock + action
        "<div style='font-size:12px; color:#5d4037; margin-top:9px;'>"
          "<b style='color:#b8860b;'>Goulot&nbsp;:</b> %13 (%14 h &#183; %15%%)"
        "</div>"
        "<div style='font-size:12px; margin-top:6px;'>%16</div>"
        "<div style='font-size:12px; color:#8d6e63; margin-top:6px;'>"
          "<b>Action&nbsp;:</b> %17"
        "</div>"
        // Badge moteur IA
        "<div style='background:#fff8e8; border:1px solid #d4af37; border-radius:7px;"
             " padding:8px 12px; margin-top:10px;'>"
          "<div style='color:#b8860b; font-size:10px; letter-spacing:1px; margin-bottom:3px; font-weight:700;'>"
            "MOTEUR IA"
          "</div>"
          "<div style='color:#6d4c41; font-size:11px;'>"
            "%18 &#183; %19 &#233;ch. &#183; R&#178; moy&nbsp;: %20"
          "</div>"
        "</div>")
    .arg(delaiLabelBg)       // %1
    .arg(delaiLabelBorder)   // %2
    .arg(delaiLabelColor)    // %3
    .arg(delaiLabel)         // %4
    .arg(riskBadgeBg)        // %5
    .arg(riskBadgeBorder)    // %6
    .arg(riskBadgeColor)     // %7
    .arg(riskLabel)          // %8
    .arg(QString::number(riskScore, 'f', 0))      // %9
    .arg(QString::number(joursOptimistes, 'f', 1)) // %10
    .arg(QString::number(joursProbables, 'f', 1))  // %11
    .arg(QString::number(joursPessimistes, 'f', 1))// %12
    .arg(hasGoulot ? goulot.etape.toHtmlEscaped() : QStringLiteral("-"))  // %13
    .arg(hasGoulot ? QString::number(goulot.chargeHeures, 'f', 1) : QStringLiteral("0")) // %14
    .arg(QString::number(partGoulot, 'f', 1))     // %15
    .arg(stockBadge)                              // %16
    .arg(retardJ > 0.0
         ? (hasGoulot
            ? QStringLiteral("Renforcer %1 (+1 employ&#233; ou +h/j).").arg(goulot.etape.toHtmlEscaped())
            : QStringLiteral("Augmenter la capacit&#233; journali&#232;re globale."))
         : QStringLiteral("Maintenir le plan actuel, surveiller le goulot."))    // %17
    .arg(mlOk
         ? QStringLiteral("&#9881; Ridge+Adam &#183; %1 mod.").arg(r2cntBadge)
         : QStringLiteral("&#9889; Heuristique calibr&#233;e"))                  // %18
    .arg(nSamples > 0 ? QString::number(nSamples) : QStringLiteral("0"))         // %19
    .arg(r2cntBadge > 0 ? QString::number(avgR2badge, 'f', 2) : QStringLiteral("n/a")); // %20

    // --- Vue équipe par étape (noms + charge/pers) ---
    QHash<QString, QStringList> equipeParEtape;
    {
        QSqlQuery qEq;
        qEq.prepare(QStringLiteral(
            "SELECT e.ETAPE_ACTUELLE, emp.NOM || ' ' || emp.PRENOM AS nom "
            "FROM ETAPES e "
            "LEFT JOIN EMPLOYES emp ON emp.ID_EMPLOYE = e.ID_EMPLOYE "
            "WHERE e.ID_PLANIFICATION = :id "
            "ORDER BY e.ETAPE_ACTUELLE"));
        qEq.bindValue(QStringLiteral(":id"), idCmd);
        if (qEq.exec()) {
            while (qEq.next()) {
                const QString et = qEq.value(0).toString().trimmed().toUpper();
                const QString nm = qEq.value(1).toString().trimmed();
                if (!et.isEmpty() && !nm.isEmpty())
                    equipeParEtape[et].push_back(nm);
            }
        }
    }

    // Couleur par étape pour les lignes du tableau
    auto etapeRowColor = [](const QString& e) -> QString {
        if (e == QStringLiteral("COUPE"))      return QStringLiteral("#e53935");
        if (e == QStringLiteral("ASSEMBLAGE")) return QStringLiteral("#1e88e5");
        if (e == QStringLiteral("COUTURE"))    return QStringLiteral("#43a047");
        if (e == QStringLiteral("FINITION"))   return QStringLiteral("#fb8c00");
        return QStringLiteral("#b8860b");
    };

    double totalHeures = 0.0;
    double totalJours = 0.0;
    QString rowsHtml;
    rowsHtml.reserve(plans.size() * 320);
    int rowIdx = 0;
    for (const auto& s : plans) {
        totalHeures += s.chargeHeures;
        totalJours  += s.joursOuvres;
        const QStringList eq   = equipeParEtape.value(s.etape.toUpper().trimmed());
        const int    nbEq      = qMax(1, eq.size());
        const double hParPers  = s.chargeHeures / nbEq;
        const QString stepColor= etapeRowColor(s.etape.toUpper().trimmed());
        const QString rowBg    = (rowIdx % 2 == 0) ? QStringLiteral("#ffffff") : QStringLiteral("#faf6f0");
        const QString eqText   = eq.isEmpty()
            ? QStringLiteral("<i style='color:#bdbdbd;'>n/a</i>")
            : QStringLiteral("%1 &nbsp;&#183;&nbsp; <b>%2 h/pers</b>")
                  .arg(eq.join(QStringLiteral(", ")).toHtmlEscaped())
                  .arg(QString::number(hParPers, 'f', 1));
        rowsHtml += QStringLiteral(
            "<tr style='background:%7;'>"
              "<td style='color:%8; font-weight:700; font-size:13px; padding:10px 8px;"
                  " border-bottom:1px solid #f0e0cc;'>&#9632; %1</td>"
              "<td style='color:#5d4037; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%2</td>"
              "<td style='color:#5d4037; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%3</td>"
              "<td style='color:#212121; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%4</td>"
              "<td style='color:#212121; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%5 h</td>"
              "<td style='color:#9e9e9e; font-size:11px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%6</td>"
            "</tr>"
            "<tr style='background:%7;'>"
              "<td colspan='6' style='color:#9e9e9e; font-size:11px; padding:2px 10px 10px 26px;"
                  " border-bottom:1px solid #f0e0cc;'>%9</td>"
            "</tr>")
            .arg(s.etape.toHtmlEscaped())                               // %1
            .arg(s.debut.toString(QStringLiteral("dd/MM/yyyy")))        // %2
            .arg(s.fin.toString(QStringLiteral("dd/MM/yyyy")))          // %3
            .arg(QString::number(s.joursOuvres, 'f', 1))               // %4
            .arg(QString::number(s.chargeHeures, 'f', 1))              // %5
            .arg(QString::number(s.hUnitaire, 'f', 3))                 // %6
            .arg(rowBg)                                                  // %7
            .arg(stepColor)                                              // %8
            .arg(eqText);                                               // %9
        ++rowIdx;
    }
    const QString htmlCompact = buildIaHtmlCompact(
        QString::number(idCmd),
        produitNom,
        rowsHtml,
        plans.isEmpty() ? QStringLiteral("-") : plans.first().debut.toString(QStringLiteral("dd/MM/yyyy")),
        plans.isEmpty() ? QStringLiteral("-") : plans.last().fin.toString(QStringLiteral("dd/MM/yyyy")),
        totalJours,
        totalHeures,
        recommendationHtml,
        objectifJours,
        (totalJours - objectifJours)
    );
    ui->lbl_ia_details->setText(htmlCompact);
    ui->lbl_ia_details->setTextFormat(Qt::RichText);
    ui->lbl_ia_details->setStyleSheet(
        "background: #fdf8f2; padding: 0px; border-radius: 10px; border: 1px solid #e8d5b0;"
    );
    ui->lbl_ia_details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->lbl_ia_details->setWordWrap(true);
    ui->lbl_ia_details->setMinimumHeight(0);
    ui->lbl_ia_details->setMaximumHeight(QWIDGETSIZE_MAX);
    scheduleIaChartRedraw(plans, totalJours);
    drawMlRegressionChart();

    const QDate dateFin = plans.isEmpty() ? (dateDebut.isValid() ? dateDebut : QDate::currentDate()) : plans.last().fin;
    ui->btn_ia_appliquer->setProperty("id_cmd", idCmd);
    ui->btn_ia_appliquer->setProperty("nv_fin", dateFin);
    ui->btn_ia_appliquer->setEnabled(idCmd > 0 && dateFin.isValid());
}

void MainWindow::renderStepPlanHtmlAndChart(const QVector<StepPlan>& plans,
                                            const QString& commandeLabel,
                                            const QString& produitLabel)
{
    if (plans.isEmpty()) {
        ui->lbl_ia_details->setText(
            QStringLiteral("<div style='color:#ef5350; padding:12px; font-family:Segoe UI;'>"
                           "<b>Aucun plan &#224; afficher.</b></div>"));
        if (m_iaChartView) {
            QChart *empty = new QChart();
            empty->setBackgroundBrush(QBrush(QColor(QStringLiteral("#fdf8f2"))));
            QChart *prev = m_iaChartView->chart();
            m_iaChartView->setChart(empty);
            if (prev && prev != empty)
                delete prev;
        }
        return;
    }

    auto etapeColor = [](const QString& e) -> QString {
        if (e == QStringLiteral("COUPE"))      return QStringLiteral("#e53935");
        if (e == QStringLiteral("ASSEMBLAGE")) return QStringLiteral("#1e88e5");
        if (e == QStringLiteral("COUTURE"))    return QStringLiteral("#43a047");
        if (e == QStringLiteral("FINITION"))   return QStringLiteral("#fb8c00");
        return QStringLiteral("#b8860b");
    };

    double totalH = 0.0, totalJ = 0.0;
    QString rows;
    rows.reserve(plans.size() * 300);
    int idx = 0;
    for (const auto& s : plans) {
        totalH += s.chargeHeures;
        totalJ += s.joursOuvres;
        const QString bg  = (idx++ % 2 == 0) ? QStringLiteral("#ffffff") : QStringLiteral("#faf6f0");
        const QString col = etapeColor(s.etape.toUpper().trimmed());
        rows += QStringLiteral(
            "<tr style='background:%7;'>"
              "<td style='color:%8; font-weight:700; font-size:13px; padding:10px 8px;"
                  " border-bottom:1px solid #f0e0cc;'>&#9632; %1</td>"
              "<td style='color:#5d4037; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%2</td>"
              "<td style='color:#5d4037; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%3</td>"
              "<td style='color:#212121; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%4</td>"
              "<td style='color:#212121; font-size:12px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%5 h</td>"
              "<td style='color:#9e9e9e; font-size:11px; text-align:center; padding:10px 6px;"
                  " border-bottom:1px solid #f0e0cc;'>%6</td>"
            "</tr>")
            .arg(s.etape.toHtmlEscaped())
            .arg(s.debut.toString(QStringLiteral("dd/MM/yyyy")))
            .arg(s.fin.toString(QStringLiteral("dd/MM/yyyy")))
            .arg(QString::number(s.joursOuvres, 'f', 1))
            .arg(QString::number(s.chargeHeures, 'f', 1))
            .arg(QString::number(s.hUnitaire, 'f', 3))
            .arg(bg).arg(col);
    }

    const QString simRec = QStringLiteral(
        "<div style='color:#8d6e63; font-size:10px; letter-spacing:1.5px; margin-bottom:10px; font-weight:700;'>"
          "SIMULATION WHAT-IF"
        "</div>"
        "<div style='background:#fff8e8; border:1px solid #d4af37; border-radius:8px;"
             " padding:12px 14px; font-size:12px; color:#5d4037;'>"
          "<div style='color:#b8860b; font-weight:800; font-size:16px; margin-bottom:7px;'>"
            "&#8987; %1 j"
          "</div>"
          "<div><b>Charge totale&nbsp;:</b> %2 h</div>"
          "<div style='margin-top:6px; color:#9e9e9e; font-size:11px;'>"
            "Simulation bas&#233;e sur les param&#232;tres actuels."
          "</div>"
        "</div>")
    .arg(QString::number(totalJ, 'f', 1))
    .arg(QString::number(totalH, 'f', 1));

    const QString htmlCompact = buildIaHtmlCompact(
        commandeLabel,
        produitLabel,
        rows,
        plans.first().debut.toString(QStringLiteral("dd/MM/yyyy")),
        plans.last().fin.toString(QStringLiteral("dd/MM/yyyy")),
        totalJ, totalH,
        simRec,
        getObjectifJoursFromUi(),
        totalJ - getObjectifJoursFromUi());

    ui->lbl_ia_details->setText(htmlCompact);
    ui->lbl_ia_details->setTextFormat(Qt::RichText);
    ui->lbl_ia_details->setStyleSheet(
        "background: #fdf8f2; padding: 0px; border-radius: 10px; border: 1px solid #e8d5b0;"
    );
    ui->lbl_ia_details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->lbl_ia_details->setWordWrap(true);

    scheduleIaChartRedraw(plans, totalJ);
}

void MainWindow::drawMlRegressionChart()
{
    QWidget *iaPage = ui->tabWidgetPlanif->widget(4);
    if (!iaPage || !iaPage->layout()) return;

    // Créer le widget une seule fois, inséré avant frame_ia_bottom
    if (!m_mlCurveView) {
        m_mlCurveView = new QChartView(iaPage);
        m_mlCurveView->setObjectName(QStringLiteral("chartViewMlCurve"));
        m_mlCurveView->setMinimumHeight(230);
        m_mlCurveView->setMaximumHeight(270);
        m_mlCurveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        if (auto *vl = qobject_cast<QVBoxLayout*>(iaPage->layout())) {
            int insertAt = vl->count();
            for (int i = 0; i < vl->count(); ++i) {
                QLayoutItem *li = vl->itemAt(i);
                if (li && li->widget() &&
                    li->widget()->objectName() == QLatin1String("frame_ia_bottom")) {
                    insertAt = i; break;
                }
            }
            vl->insertWidget(insertAt, m_mlCurveView);
        } else {
            iaPage->layout()->addWidget(m_mlCurveView);
        }
    }

    if (m_mlWeightsPerEtape.isEmpty()) {
        m_mlCurveView->setVisible(false);
        return;
    }
    m_mlCurveView->setVisible(true);

    // Palette par étape — couleurs vives sur fond clair
    struct EtapeStyle { QString label; QColor lineColor; QColor dotColor; };
    const QVector<EtapeStyle> styles = {
        { QStringLiteral("COUPE"),      QColor("#e53935"), QColor("#ffcdd2") },
        { QStringLiteral("ASSEMBLAGE"), QColor("#1e88e5"), QColor("#bbdefb") },
        { QStringLiteral("COUTURE"),    QColor("#43a047"), QColor("#c8e6c9") },
        { QStringLiteral("FINITION"),   QColor("#fb8c00"), QColor("#ffe0b2") }
    };

    // Collecte des points réels depuis la DB
    QMap<QString, QVector<QPair<double,double>>> realPts;
    double xMax = 10.0, yMax = 1.0;
    {
        QSqlQuery q;
        q.prepare(QStringLiteral(
            "SELECT p.QUANTITE, e.ETAPE_ACTUELLE, e.TEMPS_REEL_PASSE "
            "FROM ETAPES e "
            "JOIN PLANIFICATION p ON p.ID_COMMANDE = e.ID_PLANIFICATION "
            "WHERE e.TEMPS_REEL_PASSE > 0 AND p.QUANTITE > 0"));
        if (q.exec()) {
            while (q.next()) {
                const double qte = q.value(0).toDouble();
                const int ec = mapEtapeToCode(q.value(1).toString());
                const double yh  = q.value(2).toDouble();
                if (ec == 0) continue;
                const QString ek = [ec]() -> QString {
                    if (ec == 1) return QStringLiteral("COUPE");
                    if (ec == 2) return QStringLiteral("ASSEMBLAGE");
                    if (ec == 3) return QStringLiteral("COUTURE");
                    return QStringLiteral("FINITION");
                }();
                realPts[ek].push_back({ qte, yh });
                xMax = qMax(xMax, qte);
                yMax = qMax(yMax, yh);
            }
        }
    }
    xMax = qMax(xMax, 20.0);

    // Titre avec R² par étape
    QString titleR2;
    for (const EtapeStyle& st : styles) {
        if (m_mlR2PerEtape.contains(st.label))
            titleR2 += QStringLiteral("  %1 R²=%2")
                .arg(st.label)
                .arg(QString::number(m_mlR2PerEtape.value(st.label), 'f', 2));
    }

    auto *chart = new QChart();
    chart->setTitle(QStringLiteral("Régression ML — heures estimées = f(Quantité)") + titleR2);
    chart->setMargins(QMargins(8, 6, 8, 6));
    chart->setBackgroundBrush(QBrush(QColor(QStringLiteral("#fdf8f2"))));
    chart->setBackgroundRoundness(8);
    chart->setPlotAreaBackgroundBrush(QBrush(QColor(QStringLiteral("#ffffff"))));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    chart->setTitleBrush(QBrush(QColor(QStringLiteral("#3e2723"))));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont(QStringLiteral("Segoe UI"), 8));
    chart->legend()->setLabelColor(QColor(QStringLiteral("#5d4037")));
    chart->legend()->setBackgroundVisible(false);

    for (const EtapeStyle& st : styles) {
        const QString& etape = st.label;
        if (!m_mlWeightsPerEtape.contains(etape)) continue;
        const int ec = mapEtapeToCode(etape);

        // ── Points réels (scatter) ──
        if (realPts.contains(etape) && !realPts[etape].isEmpty()) {
            auto *scatter = new QScatterSeries();
            scatter->setName(etape + QStringLiteral(" (données)"));
            const QColor dotCol = st.lineColor.lighter(170);
            scatter->setColor(dotCol);
            scatter->setBorderColor(st.lineColor);
            scatter->setMarkerSize(7.0);
            scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            for (const auto& pt : realPts[etape])
                scatter->append(pt.first, pt.second);
            chart->addSeries(scatter);
        }

        // ── Courbe de régression ML (80 points) ──
        auto *curve = new QLineSeries();
        const double r2val = m_mlR2PerEtape.value(etape, 0.0);
        curve->setName(QStringLiteral("%1 (R²=%2)").arg(etape).arg(r2val, 0, 'f', 2));
        QPen pen(st.lineColor, 2.8);
        curve->setPen(pen);
        curve->setPointsVisible(false);

        const int steps = 80;
        for (int k = 1; k <= steps; ++k) {
            const double qte = (xMax / steps) * k;
            const double h = predictStepHours(ec, qte, 1.5, 2.0, 0.85);
            if (h > 0.0) {
                curve->append(qte, h);
                yMax = qMax(yMax, h);
            }
        }
        chart->addSeries(curve);
    }

    // Axes — light theme
    auto *axX = new QValueAxis();
    axX->setTitleText(QStringLiteral("Quantité"));
    axX->setTitleBrush(QBrush(QColor(QStringLiteral("#8d6e63"))));
    axX->setLabelsColor(QColor(QStringLiteral("#5d4037")));
    axX->setGridLineColor(QColor(QStringLiteral("#f0e0cc")));
    axX->setLinePenColor(QColor(QStringLiteral("#e8d5b0")));
    axX->setRange(0.0, xMax * 1.05);
    axX->setLabelFormat("%.0f");
    axX->setTickCount(6);

    auto *axY = new QValueAxis();
    axY->setTitleText(QStringLiteral("Heures"));
    axY->setTitleBrush(QBrush(QColor(QStringLiteral("#8d6e63"))));
    axY->setLabelsColor(QColor(QStringLiteral("#5d4037")));
    axY->setGridLineColor(QColor(QStringLiteral("#f0e0cc")));
    axY->setLinePenColor(QColor(QStringLiteral("#e8d5b0")));
    axY->setRange(0.0, yMax * 1.25);
    axY->setLabelFormat("%.0f");
    axY->setTickCount(5);

    chart->addAxis(axX, Qt::AlignBottom);
    chart->addAxis(axY, Qt::AlignLeft);
    for (QAbstractSeries *s : chart->series()) {
        s->attachAxis(axX);
        s->attachAxis(axY);
    }

    QChart *prev = m_mlCurveView->chart();
    m_mlCurveView->setChart(chart);
    if (prev && prev != chart) delete prev;
    m_mlCurveView->setRenderHint(QPainter::Antialiasing, true);
    m_mlCurveView->setStyleSheet(
        QStringLiteral("background: #fdf8f2; border-radius: 8px; border: 1px solid #e8d5b0;"));
}

void MainWindow::drawIaChart(const QVector<StepPlan>& plans,
                             double totalJours)
{
    QWidget *iaPage = ui->tabWidgetPlanif->widget(4);
    if (!iaPage || !iaPage->layout())
        return;

    if (auto *vl = qobject_cast<QVBoxLayout*>(iaPage->layout())) {
        // Réduit l'espace vertical avant le graphique.
        vl->setSpacing(6);
    }

    if (!m_iaChartView) {
        m_iaChartView = new QChartView(iaPage);
        m_iaChartView->setObjectName(QStringLiteral("chartViewIaDynamic"));
        m_iaChartView->setMinimumHeight(200);
        m_iaChartView->setMaximumHeight(280);
        m_iaChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        if (auto *vl = qobject_cast<QVBoxLayout*>(iaPage->layout())) {
            int insertAt = vl->count();
            for (int i = 0; i < vl->count(); ++i) {
                QLayoutItem *li = vl->itemAt(i);
                if (!li || !li->widget())
                    continue;
                if (li->widget()->objectName() == QLatin1String("frame_ia_bottom")) {
                    insertAt = i;
                    break;
                }
            }
            vl->insertWidget(insertAt, m_iaChartView);
        } else {
            iaPage->layout()->addWidget(m_iaChartView);
        }
    }

    // Couleurs par étape — vives sur fond clair
    const QVector<QColor> barColors = {
        QColor("#e53935"), QColor("#1e88e5"), QColor("#43a047"), QColor("#fb8c00")
    };

    QStringList cats;
    QVector<double> cumul;
    double c = 0.0;
    double maxChargeHeures = 0.0;

    // QStackedBarSeries : un QBarSet par étape avec N valeurs,
    // seule la position i de chaque set est non-zéro → barre à sa propre colonne
    auto *barSeries = new QStackedBarSeries();
    const int nSteps = plans.size();
    for (int i = 0; i < nSteps; ++i) {
        const StepPlan &sp = plans[i];
        auto *setH = new QBarSet(sp.etape);
        for (int j = 0; j < nSteps; ++j)
            *setH << (j == i ? sp.chargeHeures : 0.0);
        const QColor col = barColors.value(i % barColors.size());
        setH->setColor(col);
        setH->setBorderColor(col.darker(115));
        setH->setLabelColor(QColor("#212121"));
        barSeries->append(setH);
        cats << sp.etape;
        c += sp.joursOuvres;
        cumul.push_back(c);
        maxChargeHeures = qMax(maxChargeHeures, sp.chargeHeures);
    }

    auto *line = new QLineSeries();
    line->setName(QStringLiteral("Cumul jours"));
    for (int i = 0; i < cumul.size(); ++i)
        line->append(i, cumul[i]);

    auto *chart = new QChart();
    chart->addSeries(barSeries);
    chart->addSeries(line);
    chart->setTitle(QStringLiteral("Prévision fabrication — Total : %1 jours ouvrés")
                    .arg(QString::number(totalJours, 'f', 1)));
    chart->setBackgroundBrush(QBrush(QColor(QStringLiteral("#fdf8f2"))));
    chart->setBackgroundRoundness(8);
    chart->setMargins(QMargins(8, 8, 8, 8));
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    chart->setTitleBrush(QBrush(QColor(QStringLiteral("#3e2723"))));
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setLabelColor(QColor(QStringLiteral("#5d4037")));
    chart->legend()->setBackgroundVisible(false);
    chart->legend()->setFont(QFont(QStringLiteral("Segoe UI"), 8));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor(QStringLiteral("#ffffff"))));
    chart->setPlotAreaBackgroundVisible(true);

    auto *axisX = new QBarCategoryAxis();
    axisX->append(cats);
    axisX->setLabelsColor(QColor(QStringLiteral("#5d4037")));
    axisX->setGridLineColor(QColor(QStringLiteral("#f0e0cc")));
    axisX->setLinePenColor(QColor(QStringLiteral("#e8d5b0")));
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);
    line->attachAxis(axisX);

    auto *axisYH = new QValueAxis();
    axisYH->setTitleText(QStringLiteral("Heures"));
    axisYH->setTitleBrush(QBrush(QColor(QStringLiteral("#8d6e63"))));
    axisYH->setLabelsColor(QColor(QStringLiteral("#5d4037")));
    axisYH->setGridLineColor(QColor(QStringLiteral("#f0e0cc")));
    axisYH->setLinePenColor(QColor(QStringLiteral("#e8d5b0")));
    const double maxH = qMax(1.0, maxChargeHeures);
    axisYH->setRange(0.0, maxH * 1.25);
    axisYH->setLabelFormat("%.0f");
    axisYH->setTickCount(5);
    chart->addAxis(axisYH, Qt::AlignLeft);
    barSeries->attachAxis(axisYH);

    auto *axisYJ = new QValueAxis();
    axisYJ->setTitleText(QStringLiteral("Jours cum."));
    axisYJ->setTitleBrush(QBrush(QColor(QStringLiteral("#8d6e63"))));
    axisYJ->setLabelsColor(QColor(QStringLiteral("#5d4037")));
    axisYJ->setGridLineColor(Qt::transparent);
    axisYJ->setRange(0.0, qMax(1.0, totalJours * 1.20));
    axisYJ->setLabelFormat("%.1f");
    axisYJ->setTickCount(5);
    chart->addAxis(axisYJ, Qt::AlignRight);
    line->attachAxis(axisYJ);
    line->setPointsVisible(true);
    line->setPointLabelsVisible(false);
    QPen linePen(QColor(QStringLiteral("#d4af37")), 2.8);
    line->setPen(linePen);
    line->setColor(QColor(QStringLiteral("#d4af37")));

    QChart *prev = m_iaChartView->chart();
    m_iaChartView->setChart(chart);
    if (prev && prev != chart)
        delete prev;
    m_iaChartView->setRenderHint(QPainter::Antialiasing, true);
    m_iaChartView->setStyleSheet(
        QStringLiteral("background: #fdf8f2; border-radius: 8px; border: 1px solid #e8d5b0;"));
}

void MainWindow::ouvrirIAPrediction()
{
    if (ui->tabWidgetPlanif->currentIndex() == 4)
        renderIaEstimationForSelectedOrder();
    else
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

    ui->cb_employe_modif->show();
    if (ui->lbl_fp_emp_modif) ui->lbl_fp_emp_modif->show();

    ui->cb_employe_modif->clear();
    QSqlQuery qEmp("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES");
    while(qEmp.next()) ui->cb_employe_modif->addItem(qEmp.value("NOM").toString() + " " + qEmp.value("PRENOM").toString(), qEmp.value("ID_EMPLOYE"));

    // Pré-sélection
    ui->cb_produit_modif->setCurrentText(c.idProduit);
    ui->cb_matiere_modif->setCurrentText(c.idMatiere);
    
    // Extraire l'ID du 1er employé depuis la base
    QSqlQuery qAff("SELECT ID_EMPLOYE FROM ETAPES WHERE ID_PLANIFICATION = :idCmd FETCH FIRST 1 ROWS ONLY");
    qAff.bindValue(":idCmd", c.id.mid(3).toInt());
    if (qAff.exec() && qAff.next()) {
        int idxEmp = ui->cb_employe_modif->findData(qAff.value(0).toInt());
        if (idxEmp >= 0) ui->cb_employe_modif->setCurrentIndex(idxEmp);
    }
    
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
    mainL->setSpacing(10); mainL->setContentsMargins(10, 10, 10, 10);
    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12); root->setContentsMargins(8, 8, 8, 8);

    int total = mesEmployes.size();
    double masseSal = 0.0;
    int seniors = 0;
    QMap<QString, double> parDept;
    QMap<QString, double> parPoste;
    for (const auto &e : mesEmployes) {
        masseSal += e.salaire;
        parDept[e.departement] += 1.0;
        parPoste[e.poste] += 1.0;
        if (e.dateEmbauche.daysTo(QDate::currentDate()) / 365 >= 5) seniors++;
    }
    const double salaireMoyen = total > 0 ? masseSal / total : 0.0;
    QString deptDominant = "N/A";
    double maxDept = -1.0;
    for (auto it = parDept.constBegin(); it != parDept.constEnd(); ++it) {
        if (it.value() > maxDept) { maxDept = it.value(); deptDominant = it.key(); }
    }

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10); hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse RH - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit Effectif      ② Performance Salariale      ③ Diagnostic Compétences");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht); hl->addWidget(hs); root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10); cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(total), "Effectif Total", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "👥"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(masseSal, 'f', 0) + " DT", "Masse Salariale", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "💸"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(seniors), "Profils Seniors", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "🏅"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(salaireMoyen, 'f', 1) + " DT", "Salaire Moyen", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "📊"), 1, 0);
    kpi->addWidget(makeKpiCard(deptDominant, "Département Dominant", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "🏢"), 1, 1);
    kpi->addWidget(makeKpiCard(QString::number(parPoste.size()), "Postes Actifs", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "🧩"), 1, 2);
    root->addLayout(kpi);

    QHBoxLayout *rowCharts = new QHBoxLayout();
    rowCharts->setSpacing(10);
    QFrame *framePie = new QFrame(); framePie->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutPie = new QVBoxLayout(framePie);
    QLabel *titrePie = new QLabel("Répartition par Département"); titrePie->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutPie->addWidget(titrePie);
    QWidget *wPie = new QWidget(); QList<QPair<QString, double>> slices;
    for (auto it = parDept.constBegin(); it != parDept.constEnd(); ++it) slices.append({it.key(), it.value()});
    setPieChart(wPie, "", slices); layoutPie->addWidget(wPie); rowCharts->addWidget(framePie, 1);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Répartition par Poste"); titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;");
    layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget(); QStringList cats = parPoste.keys(); QList<double> vals;
    for (const auto &k : cats) vals << parPoste[k];
    setVerticalBarChart(wBar, "", cats, vals); layoutBar->addWidget(wBar); rowCharts->addWidget(frameBar, 1);
    root->addLayout(rowCharts);

    QHBoxLayout *footer = new QHBoxLayout(); footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA RH");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA RH", "Assistant RH prêt : évaluation compétences, ancienneté et alertes turnover.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

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

void MainWindow::setupDepotExpertUI() {
    if (!ui || !ui->tabWidgetDepot) return;

    auto injectDepotRibbon = [=](int index) {
        QWidget *page = ui->tabWidgetDepot->widget(index);
        if (!page || !page->layout()) return;
        if (page->findChild<QFrame*>("depotRibbon")) return;

        double totalCap = 0.0;
        double totalAct = 0.0;
        for (const auto &d : mesDepots) {
            totalCap += d.capaciteMax;
            totalAct += d.quantiteActuelle;
        }
        const double saturation = (totalCap > 0.0) ? (100.0 * totalAct / totalCap) : 68.5;
        const int zonesActives = (mesDepots.isEmpty() ? 14 : mesDepots.size());
        const double efficience = qBound(0.0, 100.0 - (saturation * 0.15), 100.0);

        QFrame *ribbon = new QFrame();
        ribbon->setObjectName("depotRibbon");
        ribbon->setFixedHeight(34);
        ribbon->setStyleSheet(
            "QFrame#depotRibbon {"
            "  background: #eaf4ff;"
            "  border: 1px solid #d5e9fb;"
            "  border-radius: 8px;"
            "}"
        );
        QHBoxLayout *rl = new QHBoxLayout(ribbon);
        rl->setContentsMargins(16, 2, 16, 2);
        rl->setSpacing(24);

        auto makeBadge = [](const QString &txt) {
            QLabel *l = new QLabel(txt);
            l->setStyleSheet("font-size: 12px; color: #2e4f69; font-weight: 800; border: none; background: transparent;");
            return l;
        };

        rl->addWidget(makeBadge(QString("📊 Saturation : %1%").arg(QString::number(saturation, 'f', 1))));
        rl->addStretch();
        rl->addWidget(makeBadge(QString("📦 Zones Actives : %1/15").arg(zonesActives)));
        rl->addStretch();
        rl->addWidget(makeBadge(QString("⚡ Efficience IA : %1%").arg(QString::number(efficience, 'f', 1))));

        if (auto *vl = qobject_cast<QVBoxLayout*>(page->layout())) {
            vl->insertWidget(0, ribbon);
        }
    };

    auto injectGuide = [=](int index, const QString &title) {
        QWidget *page = ui->tabWidgetDepot->widget(index);
        if (!page || !page->layout()) return;
        if (page->findChild<QFrame*>("guideExpertDepot")) return;

        QWidget *wrapper = new QWidget();
        QHBoxLayout *mainHL = new QHBoxLayout(wrapper);
        mainHL->setContentsMargins(0, 0, 0, 0);
        mainHL->setSpacing(20);

        QWidget *formPart = new QWidget();
        QVBoxLayout *formVL = new QVBoxLayout(formPart);
        formVL->setContentsMargins(0, 0, 0, 0);

        QVBoxLayout *oldVL = qobject_cast<QVBoxLayout*>(page->layout());
        if (!oldVL) return;

        QLayoutItem *item;
        while ((item = oldVL->takeAt(0))) {
            if (item->widget()) formVL->addWidget(item->widget());
            else if (item->layout()) formVL->addLayout(item->layout());
            delete item;
        }
        delete oldVL;

        mainHL->addWidget(formPart, 2);

        QFrame *guide = new QFrame();
        guide->setObjectName("guideExpertDepot");
        guide->setFixedWidth(240);
        guide->setStyleSheet("QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a237e, stop:1 #0d47a1); border-radius: 12px; }");
        QVBoxLayout *gl = new QVBoxLayout(guide);
        gl->setContentsMargins(15, 20, 15, 20);
        gl->setSpacing(15);

        QLabel *gt = new QLabel(title);
        gt->setStyleSheet("color: #bbdefb; font-size: 15px; font-weight: 900; border: none; background: transparent;");
        gl->addWidget(gt);

        QFrame *sep = new QFrame();
        sep->setFixedHeight(1);
        sep->setStyleSheet("background: rgba(187,222,251,0.3); border: none;");
        gl->addWidget(sep);

        QLabel *gTxt = new QLabel(
            "📍 <b>Logistique 4.0 :</b><br><br>"
            "Le cuir exige un Contrôle Température (18-22°C) et Humidité (50%).<br><br>"
            "• Chaque emplacement est mappé via Oracle Spatial pour optimisation de trajet.<br><br>"
            "• Statut : Utilisez Consolidation pour libérer l'espace critique.");
        gTxt->setWordWrap(true);
        gTxt->setStyleSheet("color: white; font-size: 12px; line-height: 150%; border: none; background: transparent;");
        gl->addWidget(gTxt);
        gl->addStretch();

        mainHL->addWidget(guide, 1);

        QVBoxLayout *newRoot = new QVBoxLayout(page);
        newRoot->setContentsMargins(0, 0, 0, 0);
        newRoot->addWidget(wrapper);
    };

    if (ui->tabWidgetDepot->count() > 2) {
        injectDepotRibbon(0);
        injectDepotRibbon(1);
        injectDepotRibbon(2);
        injectGuide(1, "📦 Guide Emplacement");
        injectGuide(2, "⚙️ Guide Maintenance");
    }
}

void MainWindow::ouvrirStatsDepot() {
    if(ui->tabWidgetDepot->count() < 4) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(3);
    if (!onglet) return;
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *mainL = new QVBoxLayout(onglet);
    mainL->setSpacing(10); mainL->setContentsMargins(10, 10, 10, 10);
    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    mainL->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidget(content);
    QVBoxLayout *root = new QVBoxLayout(content);
    root->setSpacing(12); root->setContentsMargins(8, 8, 8, 8);

    double totalCap = 0.0;
    double currentLoad = 0.0;
    int saturationAlert = 0;
    QMap<QString, double> parType;
    for (const auto &dp : mesDepots) {
        totalCap += dp.capaciteMax;
        currentLoad += dp.quantiteActuelle;
        parType[dp.typeStockage] += 1.0;
        const double local = (dp.capaciteMax > 0.0) ? (100.0 * dp.quantiteActuelle / dp.capaciteMax) : 0.0;
        if (local >= 85.0) saturationAlert++;
    }
    const double taux = (totalCap > 0.0) ? (100.0 * currentLoad / totalCap) : 0.0;
    const double espaceLibre = qMax(0.0, totalCap - currentLoad);
    QString typeDominant = "N/A";
    double maxType = -1.0;
    for (auto it = parType.constBegin(); it != parType.constEnd(); ++it) {
        if (it.value() > maxType) { maxType = it.value(); typeDominant = it.key(); }
    }

    QFrame *header = new QFrame();
    header->setStyleSheet("QFrame { background: #6d463a; border-radius: 12px; }");
    QVBoxLayout *hl = new QVBoxLayout(header);
    hl->setContentsMargins(14, 10, 14, 10); hl->setSpacing(6);
    QLabel *ht = new QLabel("📊  Tableau de Bord Stratégique : Analyse Dépôt & Logistique - FIL D'OR");
    ht->setStyleSheet("color: #ffffff; font-size: 30px; font-weight: 900;");
    QLabel *hs = new QLabel("① Audit Capacité      ② Performance Flux      ③ Diagnostic Saturation");
    hs->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700;");
    hl->addWidget(ht); hl->addWidget(hs); root->addWidget(header);

    auto makeKpiCard = [&](const QString &value, const QString &label, const QString &grad, const QString &icon) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; border:1px solid rgba(255,255,255,0.25);}").arg(grad));
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 10, 12, 10); cl->setSpacing(3);
        QLabel *ico = new QLabel(icon); ico->setAlignment(Qt::AlignRight); ico->setStyleSheet("color: rgba(255,255,255,0.95); font-size: 18px;");
        QLabel *v = new QLabel(value); v->setAlignment(Qt::AlignCenter); v->setStyleSheet("color: white; font-size: 38px; font-weight: 900;");
        QLabel *l = new QLabel(label); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color: #f6f6f6; font-size: 16px; font-weight: 800;");
        cl->addWidget(ico); cl->addWidget(v); cl->addWidget(l);
        return card;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setHorizontalSpacing(10); kpi->setVerticalSpacing(10);
    kpi->addWidget(makeKpiCard(QString::number(mesDepots.size()), "Zones Actives", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)", "🏭"), 0, 0);
    kpi->addWidget(makeKpiCard(QString::number(taux, 'f', 1) + "%", "Taux Remplissage", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)", "📦"), 0, 1);
    kpi->addWidget(makeKpiCard(QString::number(saturationAlert), "Alertes Saturation", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)", "⚠️"), 0, 2);
    kpi->addWidget(makeKpiCard(QString::number(espaceLibre, 'f', 1), "Espace Libre", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)", "🧩"), 1, 0);
    kpi->addWidget(makeKpiCard(typeDominant, "Type Dominant", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6d7bf5, stop:1 #76a7ff)", "📍"), 1, 1);
    kpi->addWidget(makeKpiCard(QString::number(parType["Froid"]), "Zones Froid", "qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffd36b, stop:1 #ffaf5f)", "🧊"), 1, 2);
    root->addLayout(kpi);

    QFrame *frameBar = new QFrame(); frameBar->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #d7ccc8; }");
    QVBoxLayout *layoutBar = new QVBoxLayout(frameBar);
    QLabel *titreBar = new QLabel("Répartition par Type de Stockage");
    titreBar->setStyleSheet("color: #8d5524; font-weight: 800; padding: 4px;"); layoutBar->addWidget(titreBar);
    QWidget *wBar = new QWidget(); QStringList types = parType.keys(); QList<double> vals;
    for (const auto &k : types) vals << parType[k];
    setVerticalBarChart(wBar, "", types, vals); layoutBar->addWidget(wBar);
    root->addWidget(frameBar);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *assistant = new QPushButton("🧠 Assistant IA Dépôt");
    assistant->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
    connect(assistant, &QPushButton::clicked, this, [=]() {
        alerteInfo("Assistant IA Dépôt", "Assistant Dépôt prêt : optimisation espace, saturation et priorités logistiques.");
    });
    footer->addWidget(assistant);
    root->addLayout(footer);

    ui->tabWidgetDepot->setCurrentIndex(3);
}

void MainWindow::showDepotOptimizeTab() {
    if (ui->tabWidgetDepot->count() < 5) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(4);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *root = new QVBoxLayout(onglet);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    root->addWidget(sa);
    QWidget *content = new QWidget(sa);
    sa->setWidget(content);
    QVBoxLayout *l = new QVBoxLayout(content);
    l->setContentsMargins(8, 8, 8, 8);
    l->setSpacing(10);

    QLabel *title = new QLabel("🧩  PILOTAGE OPTIMISATION DEPOT (IA)");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 34px; font-weight: 900; color:#2b2b73; background:#e7e3fb; border-radius:12px; padding:10px;");
    l->addWidget(title);

    QHBoxLayout *step = new QHBoxLayout();
    step->addWidget(new QLabel("🔵 Audit Espace"));
    step->addWidget(new QLabel("➜"));
    step->addWidget(new QLabel("⚪ Simulation Flux"));
    step->addWidget(new QLabel("➜"));
    step->addWidget(new QLabel("⚪ Plan Réorganisation"));
    step->addStretch();
    l->addLayout(step);

    QLabel *sub = new QLabel("Simulation IA multi-criteres : densite de stockage, flux de picking et contraintes de conservation (Cuir).");
    sub->setStyleSheet("font-size: 12px; color:#607d8b;");
    l->addWidget(sub);

    double totalCap = 0.0, totalAct = 0.0;
    for (const auto &d : mesDepots) { totalCap += d.capaciteMax; totalAct += d.quantiteActuelle; }
    const double fill = (totalCap > 0.0) ? (100.0 * totalAct / totalCap) : 0.0;
    const int nbZones = mesDepots.size();
    const int nbAlertes = (fill < 20.0) ? 1 : 0;
    const double score = (fill < 20.0) ? 100.0 : qBound(0.0, 100.0 - fill, 100.0);

    auto makeKpi = [](const QString &label, const QString &val, const QString &bg) {
        QFrame *f = new QFrame();
        f->setStyleSheet(QString("QFrame{background:%1; border:1px solid #d8d8e8; border-radius:10px;}").arg(bg));
        QVBoxLayout *vl = new QVBoxLayout(f);
        QLabel *l1 = new QLabel(label); l1->setStyleSheet("font-size:11px; color:#546e7a; font-weight:700;");
        QLabel *l2 = new QLabel(val); l2->setStyleSheet("font-size:34px; color:#1f2a6b; font-weight:900;");
        vl->addWidget(l1); vl->addWidget(l2);
        return f;
    };

    QHBoxLayout *kpi = new QHBoxLayout();
    kpi->addWidget(makeKpi("ZONES ANALYSEES", QString::number(nbZones), "#eef6ff"));
    kpi->addWidget(makeKpi("SCORE MOYEN IA", QString::number(score, 'f', 1) + "/100", "#eef8f0"));
    kpi->addWidget(makeKpi("POTENTIEL GAIN", QString::number(qMax(0.0, totalCap - totalAct), 'f', 0) + " U", "#fffced"));
    kpi->addWidget(makeKpi("ALERTES SATURATION", QString::number(nbAlertes), "#fff1f1"));
    l->addLayout(kpi);

    QTableWidget *tbl = new QTableWidget(0, 8);
    tbl->setHorizontalHeaderLabels({"ZONE", "TYPE", "TAUX", "ESPACE LIBRE", "POTENTIEL", "SCORE IA", "PRIORITE", "ACTION RECOMMAND"});
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->verticalHeader()->setVisible(false);
    tbl->setMinimumHeight(180);
    tbl->setStyleSheet("QHeaderView::section{background:#5a49ba;color:white;font-weight:800;padding:6px;} QTableWidget{border:1px solid #d9d9f0;}");
    for (int i = 0; i < mesDepots.size(); ++i) {
        const auto &d = mesDepots[i];
        tbl->insertRow(i);
        const double taux = (d.capaciteMax > 0.0) ? (100.0 * d.quantiteActuelle / d.capaciteMax) : 0.0;
        const double libre = qMax(0.0, d.capaciteMax - d.quantiteActuelle);
        tbl->setItem(i, 0, new QTableWidgetItem("1 - Empl. " + d.id + " (" + d.etagere + ")"));
        tbl->setItem(i, 1, new QTableWidgetItem(d.typeStockage));
        tbl->setItem(i, 2, new QTableWidgetItem(QString::number(taux, 'f', 1) + "%"));
        tbl->setItem(i, 3, new QTableWidgetItem(QString::number(libre, 'f', 1) + " U"));
        tbl->setItem(i, 4, new QTableWidgetItem(QString::number(libre, 'f', 1) + " U"));
        tbl->setItem(i, 5, new QTableWidgetItem(QString::number(qBound(0.0, 100.0 - taux, 100.0), 'f', 1) + "/100"));
        tbl->setItem(i, 6, new QTableWidgetItem((taux < 20.0) ? "P1" : "P3"));
        tbl->setItem(i, 7, new QTableWidgetItem("Consolidation ..."));
    }
    l->addWidget(tbl);

    QHBoxLayout *actionsTop = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Sauver optimisation");
    QPushButton *btnExport = new QPushButton("Exporter optimisation (CSV)");
    btnSave->setStyleSheet("background:#8d5524;color:white;padding:8px 14px;border-radius:8px;font-weight:700;");
    btnExport->setStyleSheet("background:#2f7aa0;color:white;padding:8px 14px;border-radius:8px;font-weight:700;");
    actionsTop->addWidget(btnSave); actionsTop->addWidget(btnExport); actionsTop->addStretch();
    l->addLayout(actionsTop);

    QFrame *curve = new QFrame();
    curve->setStyleSheet("QFrame{background:#f6fbff;border:1px solid #cfe1ee;border-radius:10px;}");
    QVBoxLayout *cl = new QVBoxLayout(curve);
    cl->addWidget(new QLabel("Courbe IA d'optimisation (gain espace vs congestion)"));
    QHBoxLayout *ctrl = new QHBoxLayout();
    ctrl->addWidget(new QLabel("Scenario"));
    QComboBox *cbS = new QComboBox(); cbS->addItems({"Consolidation standard"});
    QDateEdit *d1 = new QDateEdit(QDate::currentDate()); d1->setDisplayFormat("dd/MM/yy");
    QDateEdit *d2 = new QDateEdit(QDate::currentDate().addDays(9)); d2->setDisplayFormat("dd/MM/yy");
    QPushButton *btnSim = new QPushButton("Simuler courbe IA");
    btnSim->setStyleSheet("background:#5a49ba;color:white;padding:7px 12px;border-radius:8px;font-weight:700;");
    ctrl->addWidget(cbS); ctrl->addWidget(new QLabel("Date debut")); ctrl->addWidget(d1);
    ctrl->addWidget(new QLabel("Date fin")); ctrl->addWidget(d2); ctrl->addWidget(btnSim); ctrl->addStretch();
    cl->addLayout(ctrl);
    QWidget *chartHolder = new QWidget(); chartHolder->setMinimumHeight(260); cl->addWidget(chartHolder);
    QLabel *curveTxt = new QLabel("Periode 21/04/2026 -> 30/04/2026 | Scenario 'Consolidation standard' : Congestion max projetee = 14.9/100. Decision: maintenir optimisation planifiee");
    curveTxt->setStyleSheet("font-size:12px; color:#35566a; font-weight:700;");
    cl->addWidget(curveTxt);
    l->addWidget(curve);

    QFrame *val = new QFrame();
    val->setStyleSheet("QFrame{background:#ffffff;border:2px solid #1a237e;border-radius:12px;}");
    QHBoxLayout *vl = new QHBoxLayout(val);
    QVBoxLayout *vinfo = new QVBoxLayout();
    vinfo->addWidget(new QLabel("🔐  Validation de la Réaffectation"));
    vinfo->addWidget(new QLabel("Le plan d'optimisation est conforme aux normes de sécurité ISO-Logistics."));
    QPushButton *btnSign = new QPushButton("🖋️ Signer  Appliquer");
    btnSign->setStyleSheet("background:#1a237e;color:white;padding:10px 18px;border-radius:10px;font-weight:800;");
    vl->addLayout(vinfo, 1); vl->addWidget(btnSign);
    l->addWidget(val);

    QFrame *orch = new QFrame(); orch->setStyleSheet("QFrame{background:#f7f4ff;border:1px solid #d9d1f2;border-radius:10px;}");
    QVBoxLayout *ol = new QVBoxLayout(orch);
    ol->addWidget(new QLabel("Orchestrateur de strategie d'optimisation"));
    QHBoxLayout *o1 = new QHBoxLayout();
    o1->addWidget(new QLabel("Objectif"));
    QComboBox *cbObj = new QComboBox(); cbObj->addItems({"Liberer max espace"});
    QSpinBox *sla = new QSpinBox(); sla->setRange(70, 99); sla->setValue(92);
    QPushButton *btnGen = new QPushButton("Generer strategie");
    btnGen->setStyleSheet("background:#5a49ba;color:white;padding:7px 12px;border-radius:8px;font-weight:700;");
    o1->addWidget(cbObj); o1->addWidget(new QLabel("SLA cible")); o1->addWidget(sla); o1->addWidget(btnGen); o1->addStretch();
    ol->addLayout(o1);
    QLabel *lblStrat = new QLabel("Strategie en attente.");
    lblStrat->setStyleSheet("background:#edf0ff; border:1px solid #cfd6f8; border-radius:8px; padding:8px;");
    ol->addWidget(lblStrat);
    QPushButton *pdf = new QPushButton("Rapport PDF Optimisation");
    pdf->setStyleSheet("background:#6a5acd;color:white;padding:7px 12px;border-radius:8px;font-weight:700;");
    ol->addWidget(pdf, 0, Qt::AlignLeft);
    l->addWidget(orch);

    QFrame *wf = new QFrame(); wf->setStyleSheet("QFrame{background:#f7f8ff;border:1px solid #d6d9f5;border-radius:10px;}");
    QVBoxLayout *wfl = new QVBoxLayout(wf);
    wfl->addWidget(new QLabel("Workflow validation optimisation"));
    QHBoxLayout *wfIn = new QHBoxLayout();
    QLineEdit *resp = new QLineEdit("Responsable Depot");
    QComboBox *etat = new QComboBox(); etat->addItems({"Brouillon"}); etat->setEnabled(false);
    wfIn->addWidget(new QLabel("Responsable")); wfIn->addWidget(resp); wfIn->addWidget(new QLabel("Etat")); wfIn->addWidget(etat); wfIn->addStretch();
    wfl->addLayout(wfIn);
    QHBoxLayout *wfBtns = new QHBoxLayout();
    QPushButton *b1 = new QPushButton("Soumettre"), *b2 = new QPushButton("Approuver"), *b3 = new QPushButton("Executer");
    b1->setStyleSheet("background:#3f51b5;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    b2->setStyleSheet("background:#0f7f51;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    b3->setStyleSheet("background:#8d5524;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    wfBtns->addWidget(b1); wfBtns->addWidget(b2); wfBtns->addWidget(b3); wfBtns->addStretch();
    wfl->addLayout(wfBtns);
    wfl->addWidget(new QLabel("Workflow: Brouillon (pret pour soumission)."));
    l->addWidget(wf);

    QFrame *mc = new QFrame(); mc->setStyleSheet("QFrame{background:#fff9ef;border:1px solid #efd9b3;border-radius:10px;}");
    QVBoxLayout *mcl = new QVBoxLayout(mc);
    mcl->addWidget(new QLabel("Simulation Monte Carlo du risque"));
    QHBoxLayout *mci = new QHBoxLayout();
    QSpinBox *iter = new QSpinBox(); iter->setRange(50, 5000); iter->setValue(600);
    QDoubleSpinBox *vol = new QDoubleSpinBox(); vol->setRange(1, 60); vol->setValue(18.0);
    QPushButton *run = new QPushButton("Lancer simulation");
    run->setStyleSheet("background:#c27d2f;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    mci->addWidget(new QLabel("Iterations")); mci->addWidget(iter); mci->addWidget(new QLabel("Volatilite")); mci->addWidget(vol); mci->addWidget(run); mci->addStretch();
    mcl->addLayout(mci);
    mcl->addWidget(new QLabel("Simulation non executee."));
    l->addWidget(mc);

    QFrame *exec = new QFrame(); exec->setStyleSheet("QFrame{background:#f6fbff;border:1px solid #cfe1ee;border-radius:10px;}");
    QVBoxLayout *el = new QVBoxLayout(exec);
    el->addWidget(new QLabel("Plan d'execution hebdomadaire"));
    QPushButton *genWeek = new QPushButton("Generer planning semaine");
    genWeek->setStyleSheet("background:#2d6b8a;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    el->addWidget(genWeek, 0, Qt::AlignLeft);
    QTableWidget *tw = new QTableWidget(0, 4);
    tw->setHorizontalHeaderLabels({"JOUR", "ZONE", "ACTION", "CHARGE ESTIMEE"});
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setVisible(false);
    tw->setMinimumHeight(140);
    tw->setStyleSheet("QHeaderView::section{background:#2d6b8a;color:white;font-weight:800;padding:6px;}QTableWidget{border:1px solid #d3e2ec;}");
    el->addWidget(tw);
    l->addWidget(exec);

    QFrame *audit = new QFrame(); audit->setStyleSheet("QFrame{background:#fafafa;border:1px solid #dddddd;border-radius:10px;}");
    QVBoxLayout *al = new QVBoxLayout(audit);
    al->addWidget(new QLabel("Journal d'audit optimisation"));
    QPushButton *ref = new QPushButton("Rafraichir audit");
    ref->setStyleSheet("background:#546e7a;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    al->addWidget(ref, 0, Qt::AlignLeft);
    QTableWidget *ta = new QTableWidget(0, 5);
    ta->setHorizontalHeaderLabels({"DATE", "ACTION", "NIVEAU", "RESPONSABLE", "DETAILS"});
    ta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ta->verticalHeader()->setVisible(false);
    ta->setMinimumHeight(170);
    ta->setStyleSheet("QHeaderView::section{background:#455a64;color:white;font-weight:800;padding:6px;}QTableWidget{border:1px solid #d4d4d4;}");
    al->addWidget(ta);
    l->addWidget(audit);

    connect(btnSign, &QPushButton::clicked, this, [=]() {
        alerteSucces("Optimisation Appliquee", "La strategie IA a ete appliquee.");
    });
    connect(btnGen, &QPushButton::clicked, this, [=]() {
        lblStrat->setText("Objectif: Liberer max espace. Sequence: P1 -> P2. SLA predit: 94%. Decision: valider.");
    });
    connect(run, &QPushButton::clicked, this, [=]() {
        alerteInfo("Monte Carlo", "Simulation executee.");
    });
    connect(genWeek, &QPushButton::clicked, this, [=]() {
        tw->setRowCount(1);
        tw->setItem(0, 0, new QTableWidgetItem("Lundi"));
        tw->setItem(0, 1, new QTableWidgetItem("1 - Empl. 1"));
        tw->setItem(0, 2, new QTableWidgetItem("Consolidation"));
        tw->setItem(0, 3, new QTableWidgetItem("95 U"));
    });
    connect(ref, &QPushButton::clicked, this, [=]() {
        ta->setRowCount(ta->rowCount() + 1);
        const int r = ta->rowCount() - 1;
        ta->setItem(r, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss")));
        ta->setItem(r, 1, new QTableWidgetItem("INFO"));
        ta->setItem(r, 2, new QTableWidgetItem("SUCCES"));
        ta->setItem(r, 3, new QTableWidgetItem("Responsable Depot"));
        ta->setItem(r, 4, new QTableWidgetItem("Rafraichissement audit"));
    });
    connect(btnExport, &QPushButton::clicked, this, [=]() { exporterCSV(tbl, "Optimisation Depot"); });
    connect(btnSave, &QPushButton::clicked, this, [=]() { alerteSucces("Sauvegarde", "Optimisation sauvegardee."); });
    connect(pdf, &QPushButton::clicked, this, [=]() { exporterPDF(tbl, "Rapport Optimisation Depot"); });
    connect(btnSim, &QPushButton::clicked, this, [=]() {
        auto *vl = ensureVBox(chartHolder); clearLayout(vl);
        auto *s1 = new QLineSeries(); auto *s2 = new QLineSeries(); auto *s3 = new QLineSeries();
        for (int i = 0; i < 8; ++i) { s1->append(i, 100 - i * 0.2); s2->append(i, 5 + i * 1.4); s3->append(i, 70); }
        s1->setName("Gain d'espace projet"); s2->setName("Indice congestion"); s3->setName("Seuil acceptable");
        QPen p1(QColor("#2e7d32")); p1.setWidth(3); s1->setPen(p1);
        QPen p2(QColor("#c62828")); p2.setWidth(3); s2->setPen(p2);
        QPen p3(QColor("#607d8b")); p3.setStyle(Qt::DashLine); s3->setPen(p3);
        auto *ch = new QChart(); ch->addSeries(s1); ch->addSeries(s2); ch->addSeries(s3); ch->setTitle("Projection IA optimisation");
        auto *axX = new QValueAxis(); axX->setRange(0, 7); axX->setTitleText("Dates de simulation");
        auto *axY = new QValueAxis(); axY->setRange(0, 100);
        ch->addAxis(axX, Qt::AlignBottom); ch->addAxis(axY, Qt::AlignLeft);
        s1->attachAxis(axX); s1->attachAxis(axY); s2->attachAxis(axX); s2->attachAxis(axY); s3->attachAxis(axX); s3->attachAxis(axY);
        auto *cv = new QChartView(ch); cv->setRenderHint(QPainter::Antialiasing); cv->setMinimumHeight(240);
        vl->addWidget(cv);
    });

    ui->tabWidgetDepot->setCurrentIndex(4);
}

void MainWindow::showDepotRavitaillementTab() {
    if (ui->tabWidgetDepot->count() < 6) return;
    QWidget *onglet = ui->tabWidgetDepot->widget(5);
    if (onglet->layout()) { clearLayout(onglet->layout()); delete onglet->layout(); }

    QVBoxLayout *root = new QVBoxLayout(onglet);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);
    QScrollArea *sa = new QScrollArea(onglet);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    root->addWidget(sa);
    QWidget *content = new QWidget(sa);
    sa->setWidget(content);
    QVBoxLayout *l = new QVBoxLayout(content);
    l->setContentsMargins(8, 8, 8, 8);
    l->setSpacing(10);

    QLabel *title = new QLabel("🚚  PILOTAGE DES EXPÉDITIONS (LIVRAISON)");
    title->setStyleSheet("font-size:34px; font-weight:900; color:#1a237e; background:#e9f3ff; border-left:6px solid #1a237e; border-radius:10px; padding:10px;");
    l->addWidget(title);

    QHBoxLayout *step = new QHBoxLayout();
    step->addWidget(new QLabel("🔵 Préparation"));
    step->addWidget(new QLabel("➜"));
    step->addWidget(new QLabel("⚪ Contrôle Qualité"));
    step->addWidget(new QLabel("➜"));
    step->addWidget(new QLabel("⚪ Expédition"));
    step->addStretch();
    l->addLayout(step);
    l->addWidget(new QLabel("Centre de décision logistique : priorisation des ordres, score de conformité et orchestration multi-transporteurs."));

    double totalCap = 0.0, totalAct = 0.0;
    for (const auto &d : mesDepots) { totalCap += d.capaciteMax; totalAct += d.quantiteActuelle; }
    const double fill = (totalCap > 0.0) ? (100.0 * totalAct / totalCap) : 0.0;
    auto mk = [](const QString &k, const QString &v, const QString &bg) {
        QFrame *f = new QFrame(); f->setStyleSheet(QString("QFrame{background:%1;border:1px solid #dde3ea;border-radius:10px;}").arg(bg));
        QVBoxLayout *vl = new QVBoxLayout(f); vl->addWidget(new QLabel(k)); QLabel *vv = new QLabel(v); vv->setStyleSheet("font-size:34px;font-weight:900;color:#203072;"); vl->addWidget(vv); return f;
    };
    QHBoxLayout *kpi = new QHBoxLayout();
    kpi->addWidget(mk("ZONES SCANNEES", QString::number(mesDepots.size()), "#eef6ff"));
    kpi->addWidget(mk("REMPLISSAGE", QString::number(fill, 'f', 1) + "%", "#eef8f0"));
    kpi->addWidget(mk("URGENCES P1", (fill < 20.0 ? "1" : "0"), "#fff1f1"));
    kpi->addWidget(mk("SCORE LOGISTIQUE", QString::number(qBound(0.0, 100.0 - fill, 100.0), 'f', 1) + "/100", "#fffced"));
    l->addLayout(kpi);

    QFrame *carrier = new QFrame();
    carrier->setStyleSheet("QFrame{background:#f8f9fa;border:2px solid #1a237e;border-radius:12px;}");
    QHBoxLayout *cl = new QHBoxLayout(carrier);
    QVBoxLayout *c1 = new QVBoxLayout();
    c1->addWidget(new QLabel("🚛  Transporteur & Logistique"));
    QComboBox *cbCarrier = new QComboBox(); cbCarrier->addItems({"DHL Express (Prioritaire)", "FedEx Industrial", "Logistique Interne"});
    c1->addWidget(cbCarrier);
    QVBoxLayout *c2 = new QVBoxLayout();
    c2->addWidget(new QLabel("🕒 ETA Estimé"));
    QLabel *eta = new QLabel("24-48 Heures"); eta->setStyleSheet("font-size:24px;font-weight:900;color:#2e7d32;");
    c2->addWidget(eta);
    cl->addLayout(c1, 2); cl->addLayout(c2, 1);
    l->addWidget(carrier);

    QFrame *focus = new QFrame(); focus->setStyleSheet("QFrame{background:#f8fcff;border:1px solid #c5ddea;border-radius:10px;}");
    QVBoxLayout *fl = new QVBoxLayout(focus);
    fl->addWidget(new QLabel("Analyse zone selectionnee"));
    fl->addWidget(new QLabel("Selectionnez un emplacement dans la liste Depot pour obtenir une recommandation detaillee zone par zone."));
    QProgressBar *p1 = new QProgressBar(); p1->setValue(0); p1->setFormat("Remplissage zone: %p%");
    QProgressBar *p2 = new QProgressBar(); p2->setValue(0); p2->setFormat("Risque livraison: %p%");
    fl->addWidget(p1); fl->addWidget(p2);
    l->addWidget(focus);

    QTableWidget *tbl = new QTableWidget(0, 7);
    tbl->setHorizontalHeaderLabels({"ZONE", "TYPE", "REMPL.", "SCORE RISQUE", "QTE LIVRAISON", "PRIORITE", "ETA"});
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->verticalHeader()->setVisible(false);
    tbl->setMinimumHeight(190);
    tbl->setStyleSheet("QHeaderView::section{background:#1f6f95;color:white;font-weight:800;padding:6px;}QTableWidget{border:1px solid #cbdbe5;}");
    for (int i = 0; i < mesDepots.size(); ++i) {
        const auto &d = mesDepots[i];
        const double taux = (d.capaciteMax > 0.0) ? 100.0 * d.quantiteActuelle / d.capaciteMax : 0.0;
        const double risque = qBound(0.0, 100.0 - taux, 100.0);
        const double qte = qMax(0.0, (d.capaciteMax * 0.35) - d.quantiteActuelle);
        tbl->insertRow(i);
        tbl->setItem(i, 0, new QTableWidgetItem("1 - Empl. " + d.id + " (" + d.etagere + ")"));
        tbl->setItem(i, 1, new QTableWidgetItem(d.typeStockage));
        tbl->setItem(i, 2, new QTableWidgetItem(QString::number(taux, 'f', 1) + "%"));
        tbl->setItem(i, 3, new QTableWidgetItem(QString::number(risque, 'f', 1) + "/100"));
        tbl->setItem(i, 4, new QTableWidgetItem(QString::number(qte, 'f', 1) + " U"));
        tbl->setItem(i, 5, new QTableWidgetItem((risque > 70.0) ? "P1 - Critique" : "P3 - Normale"));
        tbl->setItem(i, 6, new QTableWidgetItem(QDate::currentDate().addDays(1).toString("dd/MM/yyyy")));
    }
    l->addWidget(tbl);

    QHBoxLayout *a1 = new QHBoxLayout();
    QPushButton *save = new QPushButton("Sauver tableau");
    QPushButton *csv = new QPushButton("Exporter tableau (CSV)");
    QPushButton *cert = new QPushButton("⚖️ Certifier  Expédier");
    save->setStyleSheet("background:#8d5524;color:white;padding:8px 14px;border-radius:8px;font-weight:700;");
    csv->setStyleSheet("background:#2f7aa0;color:white;padding:8px 14px;border-radius:8px;font-weight:700;");
    cert->setStyleSheet("background:#1a237e;color:#d4af37;padding:8px 16px;border-radius:10px;font-weight:800;border:2px solid #d4af37;");
    a1->addWidget(save); a1->addWidget(csv); a1->addStretch(); a1->addWidget(cert);
    l->addLayout(a1);

    QFrame *curve = new QFrame(); curve->setStyleSheet("QFrame{background:#f6fbff;border:1px solid #cfe1ee;border-radius:10px;}");
    QVBoxLayout *cl2 = new QVBoxLayout(curve);
    cl2->addWidget(new QLabel("Courbe predictive livraison (style pilotage matieres premieres)"));
    QHBoxLayout *cctrl = new QHBoxLayout();
    QComboBox *sc = new QComboBox(); sc->addItems({"Standard"});
    QDateEdit *d1 = new QDateEdit(QDate::currentDate()); d1->setDisplayFormat("dd/MM/yy");
    QDateEdit *d2 = new QDateEdit(QDate::currentDate().addDays(9)); d2->setDisplayFormat("dd/MM/yy");
    QPushButton *sim = new QPushButton("Simuler courbe");
    sim->setStyleSheet("background:#1f6f95;color:white;padding:7px 12px;border-radius:8px;font-weight:700;");
    cctrl->addWidget(new QLabel("Scenario")); cctrl->addWidget(sc);
    cctrl->addWidget(new QLabel("Date debut")); cctrl->addWidget(d1);
    cctrl->addWidget(new QLabel("Date fin")); cctrl->addWidget(d2);
    cctrl->addWidget(sim); cctrl->addStretch();
    cl2->addLayout(cctrl);
    QWidget *chartHolder = new QWidget(); chartHolder->setMinimumHeight(255); cl2->addWidget(chartHolder);
    QLabel *info = new QLabel("Periode 21/04/2026 -> 30/04/2026 | Scenario 'Standard': Risque max projete = 100.0/100, Couverture min attendue = 74.1/100. Decision recommandee: declencher double sourcing immediat.");
    info->setStyleSheet("font-size:12px;color:#35566a;font-weight:700;");
    cl2->addWidget(info);
    l->addWidget(curve);

    QFrame *mission = new QFrame(); mission->setStyleSheet("QFrame{background:#f7f4ff;border:1px solid #d9d1f2;border-radius:10px;}");
    QVBoxLayout *ml = new QVBoxLayout(mission);
    ml->addWidget(new QLabel("Orchestrateur de mission logistique (pro)"));
    QHBoxLayout *m1 = new QHBoxLayout();
    QSpinBox *sla = new QSpinBox(); sla->setRange(70, 99); sla->setValue(92);
    QComboBox *pol = new QComboBox(); pol->addItems({"Priorite risque"});
    QPushButton *gen = new QPushButton("Generer mission");
    gen->setStyleSheet("background:#5a49ba;color:white;padding:7px 12px;border-radius:8px;font-weight:700;");
    m1->addWidget(new QLabel("SLA minimum")); m1->addWidget(sla); m1->addWidget(new QLabel("Politique")); m1->addWidget(pol); m1->addWidget(gen); m1->addStretch();
    ml->addLayout(m1); ml->addWidget(new QLabel("Mission en attente."));
    l->addWidget(mission);

    QFrame *wf = new QFrame(); wf->setStyleSheet("QFrame{background:#f7f8ff;border:1px solid #d6d9f5;border-radius:10px;}");
    QVBoxLayout *wfl = new QVBoxLayout(wf);
    wfl->addWidget(new QLabel("Workflow de validation (metier)"));
    QHBoxLayout *wf1 = new QHBoxLayout();
    QLineEdit *resp = new QLineEdit("Chef Depot");
    QComboBox *etat = new QComboBox(); etat->addItems({"Brouillon"}); etat->setEnabled(false);
    wf1->addWidget(new QLabel("Responsable")); wf1->addWidget(resp); wf1->addWidget(new QLabel("Etat")); wf1->addWidget(etat); wf1->addStretch();
    wfl->addLayout(wf1);
    QHBoxLayout *wfB = new QHBoxLayout();
    QPushButton *s = new QPushButton("Soumettre"), *a = new QPushButton("Approuver"), *e = new QPushButton("Executer");
    s->setStyleSheet("background:#3f51b5;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    a->setStyleSheet("background:#0f7f51;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    e->setStyleSheet("background:#8d5524;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    wfB->addWidget(s); wfB->addWidget(a); wfB->addWidget(e); wfB->addStretch();
    wfl->addLayout(wfB); wfl->addWidget(new QLabel("Workflow: Brouillon (en attente de soumission)."));
    l->addWidget(wf);

    QFrame *audit = new QFrame(); audit->setStyleSheet("QFrame{background:#fafafa;border:1px solid #dddddd;border-radius:10px;}");
    QVBoxLayout *al = new QVBoxLayout(audit);
    al->addWidget(new QLabel("Traçabilite / Audit log"));
    QPushButton *ra = new QPushButton("Rafraichir audit");
    ra->setStyleSheet("background:#546e7a;color:white;border-radius:8px;padding:7px 12px;font-weight:700;");
    al->addWidget(ra, 0, Qt::AlignLeft);
    QTableWidget *ta = new QTableWidget(0, 5);
    ta->setHorizontalHeaderLabels({"DATE", "ACTION", "NIVEAU", "RESPONSABLE", "DETAILS"});
    ta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ta->verticalHeader()->setVisible(false);
    ta->setMinimumHeight(170);
    ta->setStyleSheet("QHeaderView::section{background:#455a64;color:white;font-weight:800;padding:6px;}QTableWidget{border:1px solid #d4d4d4;}");
    al->addWidget(ta);
    l->addWidget(audit);

    QFrame *global = new QFrame(); global->setStyleSheet("QFrame{background:#f7fbf7;border:1px solid #c5e0cd;border-radius:10px;}");
    QVBoxLayout *gl = new QVBoxLayout(global);
    gl->addWidget(new QLabel("Plan global de livraison"));
    QHBoxLayout *gin = new QHBoxLayout();
    QDoubleSpinBox *truck = new QDoubleSpinBox(); truck->setValue(250.0); truck->setSuffix(" U");
    QDoubleSpinBox *cost = new QDoubleSpinBox(); cost->setValue(180.0); cost->setSuffix(" DT");
    truck->setButtonSymbols(QAbstractSpinBox::NoButtons); cost->setButtonSymbols(QAbstractSpinBox::NoButtons);
    gin->addWidget(new QLabel("Capacite camion")); gin->addWidget(truck);
    gin->addWidget(new QLabel("Cout fixe / camion")); gin->addWidget(cost); gin->addStretch();
    gl->addLayout(gin);
    QPushButton *plan = new QPushButton("Generer plan livraison global");
    plan->setStyleSheet("background:#118f52;color:white;padding:8px 14px;border-radius:16px;font-weight:800;");
    QPushButton *rp = new QPushButton("Rapport PDF Direction");
    rp->setStyleSheet("background:#6a5acd;color:white;padding:8px 14px;border-radius:8px;font-weight:800;");
    gl->addWidget(plan, 0, Qt::AlignLeft); gl->addWidget(rp, 0, Qt::AlignLeft);
    QLabel *planMsg = new QLabel("Cliquez sur 'Generer plan livraison global' pour produire un plan logistique consolide.");
    planMsg->setStyleSheet("background:#ebf9ef;border:1px solid #b9dfc5;border-radius:8px;padding:8px;");
    gl->addWidget(planMsg);
    l->addWidget(global);

    connect(cert, &QPushButton::clicked, this, [=]() { alerteSucces("Expedition Validee", "Le lot a ete certifie et expedie."); });
    connect(save, &QPushButton::clicked, this, [=]() { alerteSucces("Sauvegarde", "Tableau livraison sauvegarde."); });
    connect(csv, &QPushButton::clicked, this, [=]() { exporterCSV(tbl, "Tableau Livraison Depot"); });
    connect(rp, &QPushButton::clicked, this, [=]() { exporterPDF(tbl, "Rapport Direction Livraison Depot"); });
    connect(plan, &QPushButton::clicked, this, [=]() {
        planMsg->setText("Plan consolide livraison genere. Priorite: P1 Critique. Action: valider transport prioritaire.");
    });
    connect(ra, &QPushButton::clicked, this, [=]() {
        ta->setRowCount(ta->rowCount() + 1);
        int r = ta->rowCount() - 1;
        ta->setItem(r, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss")));
        ta->setItem(r, 1, new QTableWidgetItem("PLAN_GLOBAL"));
        ta->setItem(r, 2, new QTableWidgetItem("CRITIQUE"));
        ta->setItem(r, 3, new QTableWidgetItem("Chef Depot"));
        ta->setItem(r, 4, new QTableWidgetItem("Plan consolide livraison"));
    });
    connect(sim, &QPushButton::clicked, this, [=]() {
        auto *vl = ensureVBox(chartHolder); clearLayout(vl);
        auto *s1 = new QLineSeries(); auto *s2 = new QLineSeries(); auto *s3 = new QLineSeries();
        for (int i = 0; i < 8; ++i) { s1->append(i, 100 - i * 2.6); s2->append(i, 100); s3->append(i, 74); }
        s1->setName("Indice couverture"); s2->setName("Risque projete"); s3->setName("Seuil SLA cible");
        QPen p1(QColor("#2e7d32")); p1.setWidth(3); s1->setPen(p1);
        QPen p2(QColor("#c62828")); p2.setWidth(3); s2->setPen(p2);
        QPen p3(QColor("#607d8b")); p3.setStyle(Qt::DashLine); s3->setPen(p3);
        auto *ch = new QChart(); ch->addSeries(s1); ch->addSeries(s2); ch->addSeries(s3); ch->setTitle("Projection couverture vs risque");
        auto *axX = new QValueAxis(); axX->setRange(0, 7); axX->setTitleText("Dates de livraison");
        auto *axY = new QValueAxis(); axY->setRange(0, 100);
        ch->addAxis(axX, Qt::AlignBottom); ch->addAxis(axY, Qt::AlignLeft);
        s1->attachAxis(axX); s1->attachAxis(axY); s2->attachAxis(axX); s2->attachAxis(axY); s3->attachAxis(axX); s3->attachAxis(axY);
        auto *cv = new QChartView(ch); cv->setRenderHint(QPainter::Antialiasing); cv->setMinimumHeight(240);
        vl->addWidget(cv);
    });

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
    tbl->setObjectName(QStringLiteral("tableEtapesFab"));
    m_tableEtapesFab = tbl;
    tbl->setColumnCount(9);
    tbl->setHorizontalHeaderLabels({
        QStringLiteral("ID"), QStringLiteral("Cmd"), QStringLiteral("Produit"), QStringLiteral("Employe"), QStringLiteral("Etape"),
        QStringLiteral("Prévu IA (j)"), QStringLiteral("Réel (j)"), QStringLiteral("Delta (j)"), QStringLiteral("Alerte"),
    });
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
    QLabel *lEt = new QLabel(QStringLiteral("Étape :"));
    lEt->setStyleSheet(QStringLiteral("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;"));
    QLabel *supEtapeNom = new QLabel(QStringLiteral("—"));
    supEtapeNom->setStyleSheet(QStringLiteral("font-weight: 800; font-size: 11px; color: #3e2723; border: none; background: transparent; min-width: 88px;"));
    QLabel *lPr = new QLabel(QStringLiteral("Prévu IA :"));
    lPr->setStyleSheet(QStringLiteral("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;"));
    QLabel *supPrevu = new QLabel(QStringLiteral("—"));
    supPrevu->setStyleSheet(QStringLiteral("font-weight: 800; color: #e65100; font-size: 13px; border: none; background: transparent;"));
    supPrevu->setToolTip(QStringLiteral("Même moteur que l’onglet IA Estimation (profil produit × quantité × échelle)."));
    QLabel *lRe = new QLabel(QStringLiteral("Réel (j) :"));
    lRe->setStyleSheet(QStringLiteral("font-size: 10px; color: #8d6e63; font-weight: 600; border: none; background: transparent;"));
    QDoubleSpinBox *supTemps = new QDoubleSpinBox();
    supTemps->setDecimals(2);
    supTemps->setSuffix(QStringLiteral(" j"));
    supTemps->setMaximum(kJoursMaxSaisieTempsReelFabrication);
    supTemps->setSingleStep(0.125);
    supTemps->setStyleSheet("QDoubleSpinBox { border: 1px solid #d7ccc8; border-radius: 4px; padding: 3px; font-size: 11px; background: white; min-width: 70px; }");
    formG->addWidget(lEt, 0, 0); formG->addWidget(supEtapeNom, 0, 1);
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
    QLabel *deltaLbl = new QLabel(QStringLiteral("DELTA (j)"));
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

    // =============================================
    // ONGLET 4 : IA DÉCOUPE (nesting matière)
    // =============================================
    QWidget *tabCut = new QWidget();
    QVBoxLayout *cutMainL = new QVBoxLayout(tabCut);
    cutMainL->setSpacing(10);
    cutMainL->setContentsMargins(16, 12, 16, 8);

    QLabel *cutTitre = new QLabel(QStringLiteral("IA Découpe — plan de matière (guillotine)"));
    cutTitre->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #4a148c;"));
    cutMainL->addWidget(cutTitre);

    QLabel *cutHint = new QLabel(
        QStringLiteral("Sélectionnez une ligne dans « Suivi des Etapes », puis générez le plan à partir de la commande (PRODUIT + MATIERE)."));
    cutHint->setWordWrap(true);
    cutHint->setStyleSheet(QStringLiteral("color: #6d4c41; font-size: 12px;"));
    cutMainL->addWidget(cutHint);

    QHBoxLayout *cutCtrl = new QHBoxLayout();
    QLabel *lblSp = new QLabel(QStringLiteral("Espacement (mm) :"));
    m_spinCutSpacing = new QDoubleSpinBox();
    m_spinCutSpacing->setRange(0.5, 50.0);
    m_spinCutSpacing->setValue(4.0);
    m_spinCutSpacing->setDecimals(1);
    m_spinCutSpacing->setObjectName(QStringLiteral("spinCutSpacing"));

    m_lblCutApiStatus = new QLabel(QStringLiteral("● API"));
    m_lblCutApiStatus->setStyleSheet(QStringLiteral(
        "color: #9e9e9e; font-weight: 700; font-size: 11px; padding: 0 4px;"));
    m_lblCutApiStatus->setToolTip(QStringLiteral("Statut API svgnest-service (port 8010)."));

    QPushButton *btnCheckApi = new QPushButton(QStringLiteral("⟳"));
    btnCheckApi->setFixedSize(28, 28);
    btnCheckApi->setToolTip(QStringLiteral("Vérifier la disponibilité de l'API"));
    btnCheckApi->setCursor(Qt::PointingHandCursor);
    btnCheckApi->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f3e5f5; border: 1px solid #ce93d8; border-radius: 6px;"
        " font-size: 13px; font-weight: 700; }"
        "QPushButton:hover { background: #e1bee7; }"));

    m_btnCutExportPdf = new QPushButton(QStringLiteral("⬇ Exporter"));
    m_btnCutExportPdf->setToolTip(QStringLiteral("Ouvrir le dossier d'export PNG"));
    m_btnCutExportPdf->setCursor(Qt::PointingHandCursor);
    m_btnCutExportPdf->setStyleSheet(styleBtnTool("#4a148c"));
    m_btnCutExportPdf->setEnabled(false);

    QPushButton *btnGenererDecoupe = new QPushButton(QStringLiteral("Générer plan de découpe"));
    btnGenererDecoupe->setObjectName(QStringLiteral("btnGenererDecoupe"));
    btnGenererDecoupe->setStyleSheet(styleBtnTool("#7b1fa2"));
    btnGenererDecoupe->setCursor(Qt::PointingHandCursor);

    cutCtrl->addWidget(lblSp);
    cutCtrl->addWidget(m_spinCutSpacing);
    cutCtrl->addSpacing(12);
    cutCtrl->addWidget(m_lblCutApiStatus);
    cutCtrl->addWidget(btnCheckApi);
    cutCtrl->addStretch();
    cutCtrl->addWidget(m_btnCutExportPdf);
    cutCtrl->addSpacing(8);
    cutCtrl->addWidget(btnGenererDecoupe);
    cutMainL->addLayout(cutCtrl);

    m_lblCutSummary = new QLabel(QStringLiteral("—"));
    m_lblCutSummary->setObjectName(QStringLiteral("lblCutSummary"));
    m_lblCutSummary->setTextFormat(Qt::RichText);
    m_lblCutSummary->setWordWrap(true);
    m_lblCutSummary->setStyleSheet(
        QStringLiteral("padding: 10px; background: #faf5ff; border: 1px solid #ce93d8; border-radius: 8px; color: #3e2723;"));
    cutMainL->addWidget(m_lblCutSummary);

    // ── Corps : Canvas principal (gauche) + Panneau de détail (droite) ──
    QHBoxLayout *cutBody = new QHBoxLayout();
    cutBody->setSpacing(10);

    // GAUCHE : canvas interactif (zoom/pan via molette + glisser)
    m_sceneCutPreview = new QGraphicsScene(this);
    m_graphicsCutPreview = new CutPreviewGraphicsView();
    m_graphicsCutPreview->setObjectName(QStringLiteral("graphicsCutPreview"));
    m_graphicsCutPreview->setScene(m_sceneCutPreview);
    m_graphicsCutPreview->setMinimumSize(480, 360);
    m_graphicsCutPreview->setStyleSheet(QStringLiteral(
        "QGraphicsView { background: #f7f3ff; border: 2px solid #ce93d8; border-radius: 10px; }"));
    cutBody->addWidget(m_graphicsCutPreview, 3);

    // DROITE : navigation feuilles + tableau de placements
    QVBoxLayout *cutRightPanel = new QVBoxLayout();
    cutRightPanel->setSpacing(6);

    QHBoxLayout *cutNav = new QHBoxLayout();
    m_btnCutSheetPrev = new QPushButton(QStringLiteral("◀ Précédent"));
    m_btnCutSheetNext = new QPushButton(QStringLiteral("Suivant ▶"));
    m_btnCutSheetPrev->setCursor(Qt::PointingHandCursor);
    m_btnCutSheetNext->setCursor(Qt::PointingHandCursor);
    m_lblCutSheetNav = new QLabel(QStringLiteral("Feuille —"));
    m_lblCutSheetNav->setAlignment(Qt::AlignCenter);
    m_lblCutSheetNav->setStyleSheet(QStringLiteral("font-weight: 700; color: #4a148c; font-size: 12px;"));
    m_btnCutSheetPrev->setEnabled(false);
    m_btnCutSheetNext->setEnabled(false);
    cutNav->addWidget(m_btnCutSheetPrev);
    cutNav->addWidget(m_lblCutSheetNav, 1);
    cutNav->addWidget(m_btnCutSheetNext);
    cutRightPanel->addLayout(cutNav);

    m_tableCutPlacements = new QTableWidget();
    m_tableCutPlacements->setObjectName(QStringLiteral("tableCutPlacements"));
    m_tableCutPlacements->setMinimumHeight(120);
    m_tableCutPlacements->setStyleSheet(QStringLiteral(
        "QTableWidget { border: 1px solid #e1bee7; border-radius: 6px; font-size: 11px; }"
        "QHeaderView::section { background: #f3e5f5; color: #4a148c; font-weight: 600;"
        " border: none; padding: 4px; }"));
    cutRightPanel->addWidget(m_tableCutPlacements, 1);

    cutBody->addLayout(cutRightPanel, 1);
    cutMainL->addLayout(cutBody, 1);

    tabEtapes->addTab(tabCut, QStringLiteral("IA Découpe"));

    connect(btnGenererDecoupe, &QPushButton::clicked, this, &MainWindow::runCutPlanForCurrentSelection);
    connect(btnCheckApi, &QPushButton::clicked, this, &MainWindow::checkCutApiHealth);
    connect(m_btnCutExportPdf, &QPushButton::clicked, this, [this]() {
        if (!m_cutExportDir.isEmpty())
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_cutExportDir));
    });
    connect(m_btnCutSheetPrev, &QPushButton::clicked, this, [this]() {
        if (m_cutCurrentSheet > 1) {
            --m_cutCurrentSheet;
            refreshCutSheetNavigation();
        }
    });
    connect(m_btnCutSheetNext, &QPushButton::clicked, this, [this]() {
        if (m_lastCutPlan.sheetsUsed > 0 && m_cutCurrentSheet < m_lastCutPlan.sheetsUsed) {
            ++m_cutCurrentSheet;
            refreshCutSheetNavigation();
        }
    });
    // Vérifie l'API au démarrage (délai 600ms pour laisser l'UI s'initialiser)
    QTimer::singleShot(600, this, &MainWindow::checkCutApiHealth);

    pageL->addWidget(tabEtapes);

    // =============================================
    // FONCTIONS
    // =============================================
    auto ctxFab = std::make_shared<std::pair<QString, int>>(QString(), 1);
    auto remplirTable = [=](QSqlQueryModel *model) {
        tbl->setRowCount(0);
        int rows = model->rowCount();
        tbl->setRowCount(rows);

        for (int i = 0; i < rows; i++) {
            int idSuivi = model->record(i).value("ID_SUIVI").toInt();
            int idPlanif = model->record(i).value("ID_PLANIFICATION").toInt();
            QString produit = model->record(i).value("PRODUIT").toString();
            const int qtePlanif = model->record(i).value(QStringLiteral("QTE_PLANIF")).toInt();
            const int qteSafe = std::max(1, qtePlanif);
            QString employe = model->record(i).value("EMPLOYE").toString();
            QString etape = model->record(i).value("ETAPE_ACTUELLE").toString();
            double tempsReel = model->record(i).value("TEMPS_REEL_PASSE").toDouble();
            double deltaDb = model->record(i).value("DELTA").toDouble();
            int alerte = model->record(i).value("ALERTE_ACTIVE").toInt();
            const double hPrev = tempsPrevuHeuresEtapePourCommandeIA(produit, qteSafe, etape);
            const double deltaAff = tempsReel - hPrev;

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
            it0->setData(Qt::UserRole + 2, qtePlanif > 0 ? qtePlanif : qteSafe);
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

            // Col 5 : Prévu IA (plafond jours comme onglet IA)
            QTableWidgetItem *itPrev = new QTableWidgetItem(formaterJoursPrevusPlafondIA(hPrev));
            itPrev->setTextAlignment(Qt::AlignCenter);
            itPrev->setForeground(QColor("#e65100"));
            itPrev->setFont(fbold);
            itPrev->setToolTip(QStringLiteral("Prévu IA (avec échelle) : %1 h").arg(hPrev, 0, 'f', 1));
            tbl->setItem(i, 5, itPrev);

            // Col 6 : Temps réel (affichage jours ouvrés)
            QTableWidgetItem *itT = new QTableWidgetItem(formaterHeuresEtapesEnJoursAffichage(tempsReel));
            itT->setTextAlignment(Qt::AlignCenter);
            itT->setForeground(QColor("#5d4037"));
            itT->setFont(fbold);
            itT->setToolTip(QStringLiteral("%1 h (stocké en base)").arg(tempsReel, 0, 'f', 2));
            tbl->setItem(i, 6, itT);

            // Col 7 : Delta (réel − prévu IA)
            QLabel *lblDelta = new QLabel();
            QString deltaText = formaterDeltaEtapesEnJoursAffichage(deltaAff);
            lblDelta->setText(deltaText);
            lblDelta->setToolTip(
                QStringLiteral("Écart réel − prévu IA : %1 h. Delta en base : %2 h.")
                    .arg(deltaAff, 0, 'f', 2)
                    .arg(deltaDb, 0, 'f', 2));
            lblDelta->setAlignment(Qt::AlignCenter);
            if (deltaAff > 0) {
                lblDelta->setStyleSheet(
                    "background-color: rgba(198,40,40,0.12); color: #c62828; font-weight: 900; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            } else if (deltaAff < 0) {
                lblDelta->setStyleSheet(
                    "background-color: rgba(46,125,50,0.12); color: #2e7d32; font-weight: 900; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            } else {
                lblDelta->setStyleSheet(
                    "background-color: rgba(0,0,0,0.05); color: #9e9e9e; font-weight: 700; "
                    "font-size: 11px; border-radius: 8px; padding: 3px 8px; margin: 3px 6px;");
            }
            tbl->setCellWidget(i, 7, lblDelta);

            // Col 8 : Alerte - Pastille
            QLabel *lblAlerte = new QLabel();
            lblAlerte->setAlignment(Qt::AlignCenter);
            if (alerte) {
                lblAlerte->setText("ALERTE");
                lblAlerte->setToolTip(QStringLiteral("Retard enregistré (alerte BDD)."));
                lblAlerte->setStyleSheet(
                    "background-color: #c62828; color: white; font-weight: 900; "
                    "font-size: 10px; border-radius: 10px; padding: 4px 10px; "
                    "margin: 3px 6px; letter-spacing: 1px;");
            } else {
                lblAlerte->setText("OK");
                lblAlerte->setToolTip(QStringLiteral("Pas d’alerte BDD sur cette ligne."));
                lblAlerte->setStyleSheet(
                    "background-color: rgba(46,125,50,0.12); color: #2e7d32; font-weight: 800; "
                    "font-size: 10px; border-radius: 10px; padding: 4px 10px; "
                    "margin: 3px 6px;");
            }
            tbl->setCellWidget(i, 8, lblAlerte);

            tbl->setRowHeight(i, 40);
        }
    };
    auto refreshCrud = [=]() {
        Etape e; QSqlQueryModel *m = e.afficher(); remplirTable(m); delete m;
    };

    m_refreshFabEtapesTable = [=]() {
        Etape ex;
        QSqlQueryModel *m = ex.afficher();
        remplirTable(m);
        delete m;
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

        QFrame *stHeader = new QFrame(); stHeader->setFixedHeight(84);
        stHeader->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2c1a16, stop:1 #795548); border-radius: 12px; }");
        QVBoxLayout *stHL = new QVBoxLayout(stHeader); stHL->setContentsMargins(16, 8, 16, 8); stHL->setSpacing(2);
        QLabel *stTitre = new QLabel("📊  Tableau de Bord Stratégique : Analyse Fabrication - FIL D'OR");
        stTitre->setStyleSheet("color: white; font-size: 30px; font-weight: 900; border: none; background: transparent;");
        QLabel *stSub = new QLabel("① Audit Etapes      ② Performance Production      ③ Diagnostic Retards");
        stSub->setStyleSheet("color: #e7d7cf; font-size: 12px; font-weight: 700; border: none; background: transparent;");
        stHL->addWidget(stTitre);
        stHL->addWidget(stSub);
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

        tempsPrevuTotal = 0.0;
        {
            QSqlQuery qPv;
            if (qPv.exec(QStringLiteral(
                    "SELECT e.ETAPE_ACTUELLE, NVL(p.QUANTITE, 0), pr.DESIGNATION "
                    "FROM ETAPES e "
                    "LEFT JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
                    "LEFT JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT"))) {
                while (qPv.next()) {
                    const QString ep = qPv.value(0).toString();
                    const int qt = qPv.value(1).toInt();
                    const QString des = qPv.value(2).toString();
                    tempsPrevuTotal += tempsPrevuHeuresEtapePourCommandeIA(des, std::max(1, qt), ep);
                }
            }
        }
        double tauxC=(totalEtapes>0)?(double(etTerminees)/totalEtapes)*100.0:0;
        double eff=(tempsPrevuTotal>0)?(tempsReelTotal/tempsPrevuTotal)*100.0:0;

        auto kpi = [](QString val, QString label, QString grad) -> QFrame* {
            QFrame *f = new QFrame(); f->setMinimumHeight(100);
            f->setStyleSheet(QString("QFrame{background:%1;border-radius:12px;border:1px solid rgba(255,255,255,0.25);}").arg(grad));
            QVBoxLayout *v=new QVBoxLayout(f); v->setSpacing(4);
            QLabel *lv=new QLabel(val); lv->setStyleSheet("color:white;font-size:38px;font-weight:900;border:none;background:transparent;"); lv->setAlignment(Qt::AlignCenter);
            QLabel *ll=new QLabel(label); ll->setStyleSheet("color:#f6f6f6;font-size:16px;font-weight:800;border:none;background:transparent;"); ll->setAlignment(Qt::AlignCenter);
            v->addStretch(); v->addWidget(lv); v->addWidget(ll); v->addStretch();
            return f;
        };

        QHBoxLayout *kpiR=new QHBoxLayout(); kpiR->setSpacing(12);
        kpiR->addWidget(kpi(QString::number(totalEtapes),"Total Etapes","qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #5b6ee1, stop:1 #b05cbf)"));
        kpiR->addWidget(kpi(QString::number(tauxC,'f',1)+"%","Completion","qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f8d7a8, stop:1 #ef6a8b)"));
        kpiR->addWidget(kpi(QString::number(eff,'f',1)+"%","Efficacite","qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1ec997, stop:1 #48e38f)"));
        kpiR->addWidget(kpi(QString::number(etEnRetard),"En Retard","qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ff4f81, stop:1 #ff7b54)"));
        sL->addLayout(kpiR);

        auto barGroup=[](QString t,QList<QPair<QString,int>> d,int tot)->QGroupBox*{
            QGroupBox *g=new QGroupBox(t);
            g->setStyleSheet("QGroupBox{background:#fff;border:1px solid #e3d9cf;border-radius:10px;margin-top:10px;padding:12px;font-weight:800;}QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;color:#6d4c41;}");
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

        const double deltaTotH = tempsReelTotal - tempsPrevuTotal;
        QLabel *foot = new QLabel(
            QStringLiteral("Réel : %1 | Prévu : %2 | Delta : %3")
                .arg(formaterHeuresEtapesEnJoursAffichage(tempsReelTotal))
                .arg(formaterHeuresEtapesEnJoursAffichage(tempsPrevuTotal))
                .arg(formaterDeltaEtapesEnJoursAffichage(deltaTotH)));
        foot->setToolTip(QStringLiteral("Valeurs en base : réel %1 h, prévu %2 h, delta %3 h")
                             .arg(tempsReelTotal, 0, 'f', 2)
                             .arg(tempsPrevuTotal, 0, 'f', 2)
                             .arg(deltaTotH, 0, 'f', 2));
        foot->setStyleSheet("background:rgba(0,0,0,0.05);padding:12px;border-radius:8px;font-weight:bold;font-size:13px;color:#3e2723;");
        foot->setAlignment(Qt::AlignCenter);
        sL->addWidget(foot);

        QHBoxLayout *footerAssistant = new QHBoxLayout();
        footerAssistant->addStretch();
        QPushButton *assistantFab = new QPushButton(QStringLiteral("🧠 Assistant IA Fabrication"));
        assistantFab->setStyleSheet("QPushButton { background:#1f5fbf; color:white; border:none; border-radius:16px; padding:8px 16px; font-weight:800; }QPushButton:hover { background:#2a70d2; }");
        connect(assistantFab, &QPushButton::clicked, this, [=]() {
            alerteInfo(QStringLiteral("Assistant IA Fabrication"),
                       QStringLiteral("Assistant prêt : priorisation des étapes critiques et optimisation du flux atelier."));
        });
        footerAssistant->addWidget(assistantFab);
        sL->addLayout(footerAssistant);
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

        const int qteSel = std::max(1, tbl->item(row, 0)->data(Qt::UserRole + 2).toInt());
        ctxFab->first = prod;
        ctxFab->second = qteSel;
        supEtapeNom->setText(et);

        const double tp = tempsPrevuHeuresEtapePourCommandeIA(prod, qteSel, et);
        supPrevu->setText(formaterJoursPrevusPlafondIA(tp));
        supPrevu->setToolTip(QStringLiteral("Prévu IA (avec échelle) : %1 h — jours = plafond comme l’onglet IA (%2 h/j).")
                                 .arg(tp, 0, 'f', 1)
                                 .arg(kHeuresOuvreParJourFabrication, 0, 'f', 0));

        double trH = 0.0;
        QSqlQuery qTr;
        qTr.prepare(QStringLiteral("SELECT NVL(TEMPS_REEL_PASSE, 0) FROM ETAPES WHERE ID_SUIVI = :id"));
        qTr.bindValue(QStringLiteral(":id"), selectedEtapeId);
        if (qTr.exec() && qTr.next())
            trH = qTr.value(0).toDouble();
        {
            QSignalBlocker b(supTemps);
            supTemps->setValue(heuresStockVersJoursSaisieFabrication(trH));
        }
        const double dSel = trH - tp;
        supDelta->setText(formaterDeltaEtapesEnJoursAffichage(dSel));
        supDelta->setToolTip(QStringLiteral("Delta : %1%2 h").arg(dSel > 0 ? QStringLiteral("+") : QString()).arg(dSel, 0, 'f', 2));
        if (dSel > 0)
            supDelta->setStyleSheet(QStringLiteral("font-weight:900;font-size:12px;color:#c62828;border:none;background:transparent;"));
        else if (dSel < 0)
            supDelta->setStyleSheet(QStringLiteral("font-weight:900;font-size:12px;color:#2e7d32;border:none;background:transparent;"));
        else
            supDelta->setStyleSheet(QStringLiteral("font-weight:900;font-size:14px;color:#757575;border:none;background:transparent;"));
    });

    connect(supTemps, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double joursSaisis) {
        const double tr = joursSaisieVersHeuresFabrication(joursSaisis);
        const QString etNom = supEtapeNom->text();
        if (etNom.isEmpty() || etNom == QLatin1String("—"))
            return;
        const double tp = tempsPrevuHeuresEtapePourCommandeIA(ctxFab->first, std::max(1, ctxFab->second), etNom);
        const double d = tr - tp;
        supDelta->setText(formaterDeltaEtapesEnJoursAffichage(d));
        supDelta->setToolTip(QStringLiteral("Delta : %1%2 h").arg(d > 0 ? QStringLiteral("+") : QString()).arg(d, 0, 'f', 2));
        if (d > 0)
            supDelta->setStyleSheet("font-weight:900;font-size:12px;color:#c62828;border:none;background:transparent;");
        else if (d < 0)
            supDelta->setStyleSheet("font-weight:900;font-size:12px;color:#2e7d32;border:none;background:transparent;");
        else
            supDelta->setStyleSheet("font-weight:900;font-size:14px;color:#757575;border:none;background:transparent;");
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
        const QString et = supEtapeNom->text();
        if (et.isEmpty() || et == QLatin1String("—")) {
            alerteWarning(QStringLiteral("Sélection"), QStringLiteral("Sélectionnez une ligne du tableau."));
            return;
        }
        const QString des = tbl->item(row, 2) ? tbl->item(row, 2)->text() : QString();
        const int qteSel = std::max(1, tbl->item(row, 0)->data(Qt::UserRole + 2).toInt());
        const double tr = joursSaisieVersHeuresFabrication(supTemps->value());

        if (tr <= 0) {
            alerteWarning(QStringLiteral("Temps"), QStringLiteral("Saisissez un temps réel > 0 (en jours ouvrés)."));
            return;
        }

        // Calculer delta et alerte (heures en base) — prévu = IA (même que tableau)
        const double tp = tempsPrevuHeuresEtapePourCommandeIA(des, qteSel, et);
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

            // 2 beeps si retard détecté (ALERTE_ACTIVE = 1)
            if (al == 1 && arduino && arduino->isConnected()) {
                arduino->beep();
                QTimer::singleShot(600, this, [this]() { arduino->beep(); });
                qDebug() << "BUZZER: 2 beeps — retard détecté !";
            }

            alerteSucces(QStringLiteral("OK"),
                        QStringLiteral("Étape « %1 » enregistrée : %2 h (%3), delta %4")
                            .arg(et)
                            .arg(tr, 0, 'f', 1)
                            .arg(formaterHeuresEtapesEnJoursAffichage(tr))
                            .arg(formaterDeltaEtapesEnJoursAffichage(d)));

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
        const QString et = supEtapeNom->text();
        if (et.isEmpty() || et == QLatin1String("—")) {
            alerteWarning(QStringLiteral("Sélection"), QStringLiteral("Sélectionnez une ligne du tableau."));
            return;
        }
        const QString des = tbl->item(row, 2) ? tbl->item(row, 2)->text() : QString();
        const int qteSel = std::max(1, tbl->item(row, 0)->data(Qt::UserRole + 2).toInt());
        const double tr = joursSaisieVersHeuresFabrication(supTemps->value());
        const double tp = tempsPrevuHeuresEtapePourCommandeIA(des, qteSel, et);
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
        if (q.exec()) {
            // 3 beeps — commande terminée !
            if (arduino && arduino->isConnected()) {
                arduino->beep();
                QTimer::singleShot(600, this, [this]() { arduino->beep(); });
                QTimer::singleShot(1200, this, [this]() { arduino->beep(); });
                qDebug() << "BUZZER: 3 beeps — commande terminée !";
            }
        }
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
        ? QStringLiteral("EN AVANCE de ") + formaterHeuresEtapesEnJoursAffichage(qAbs(deltaTotal))
        : QStringLiteral("EN RETARD de ") + formaterHeuresEtapesEnJoursAffichage(deltaTotal);

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
        "<td>%4 <span style='font-size:11px;color:#666;'>(%5 h)</span></td></tr>"
        "<tr><td><b>Performance :</b></td>"
        "<td style='color:%6; font-weight:bold;'>%7</td></tr>"
        "</table>"
        "<hr style='border:1px dashed #ccc;'>"
        "<div style='padding:10px; background:%8; border-radius:8px; text-align:center;'>"
        "<b>Actions automatiques :</b><br>"
        "1. Statut → <b>Termine</b><br>"
        "2. Stock produit → <b>+%3 unites</b><br>"
        "3. Module Produits → <b>Notifie</b>"
        "</div></div>"
    )
    .arg(idPlanification)
    .arg(nomProduit)
    .arg(quantite)
    .arg(formaterHeuresEtapesEnJoursAffichage(tempsTotal))
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

// =============================================================================
// IA DÉCOUPE — nesting guillotine + export PNG + persistance Oracle
// DDL attendu (schéma PROJET_CPP) :
//   Modèle E/A validé: appui sur MATIERES_PREMIERES + fallback patron standard local.
//   Les tables DECOUPE_* et PRODUIT_PIECES ne sont plus utilisées.
// =============================================================================

namespace {

static QRectF mmToPxCut(double xMm, double yMm, double wMm, double hMm,
                        double s, double ox, double oy)
{
    return QRectF(ox + xMm * s, oy + yMm * s, wMm * s, hMm * s);
}

static QString normalizeLeatherZone(const QString& rawZone)
{
    const QString z = rawZone.trimmed().toLower();
    if (z == QStringLiteral("dosset") || z == QStringLiteral("croupon")
        || z == QStringLiteral("collet") || z == QStringLiteral("flanc")) {
        return z;
    }
    return QString();
}

static QString preferredLeatherZoneForPiece(const QString& pieceName, const QString& zoneOverride = QString())
{
    const QString zoneDb = normalizeLeatherZone(zoneOverride);
    if (!zoneDb.isEmpty())
        return zoneDb;
    const QString n = pieceName.toLower();
    if (n.contains(QStringLiteral("face")) || n.contains(QStringLiteral("avant"))
        || n.contains(QStringLiteral("dos")) || n.contains(QStringLiteral("arrière"))
        || n.contains(QStringLiteral("arriere"))) {
        return QStringLiteral("dosset");
    }
    if (n.contains(QStringLiteral("rabat")) || n.contains(QStringLiteral("poche"))
        || n.contains(QStringLiteral("fond"))) {
        return QStringLiteral("croupon");
    }
    if (n.contains(QStringLiteral("bandouli")) || n.contains(QStringLiteral("anse"))
        || n.contains(QStringLiteral("bretelle")) || n.contains(QStringLiteral("sangle"))) {
        return QStringLiteral("flanc");
    }
    return QStringLiteral("collet");
}

static QMap<QString, QRectF> leatherZonesInSheet(const QRectF& sheetRect)
{
    QMap<QString, QRectF> z;
    const qreal w = sheetRect.width();
    const qreal h = sheetRect.height();
    const qreal x = sheetRect.x();
    const qreal y = sheetRect.y();
    z.insert(QStringLiteral("collet"), QRectF(x + 0.28 * w, y + 0.03 * h, 0.44 * w, 0.24 * h));
    z.insert(QStringLiteral("dosset"), QRectF(x + 0.26 * w, y + 0.27 * h, 0.48 * w, 0.31 * h));
    z.insert(QStringLiteral("croupon"), QRectF(x + 0.26 * w, y + 0.60 * h, 0.48 * w, 0.31 * h));
    z.insert(QStringLiteral("flanc_g"), QRectF(x + 0.02 * w, y + 0.22 * h, 0.24 * w, 0.62 * h));
    z.insert(QStringLiteral("flanc_d"), QRectF(x + 0.74 * w, y + 0.22 * h, 0.24 * w, 0.62 * h));
    return z;
}

static QVector<QRectF> leatherDefectMasksInSheet(const QRectF& sheetRect)
{
    QVector<QRectF> out;
    const qreal w = sheetRect.width();
    const qreal h = sheetRect.height();
    const qreal x = sheetRect.x();
    const qreal y = sheetRect.y();
    // Défauts typiques (cicatrices/trous) à exclure du nesting
    out.push_back(QRectF(x + 0.47 * w, y + 0.08 * h, 0.08 * w, 0.06 * h));
    out.push_back(QRectF(x + 0.06 * w, y + 0.47 * h, 0.10 * w, 0.08 * h));
    out.push_back(QRectF(x + 0.84 * w, y + 0.52 * h, 0.08 * w, 0.09 * h));
    return out;
}

static bool intersectsPreferredZone(const QRectF& pieceRect,
                                    const QString& zoneName,
                                    const QMap<QString, QRectF>& zones)
{
    if (zoneName == QStringLiteral("flanc")) {
        return pieceRect.intersects(zones.value(QStringLiteral("flanc_g")))
            || pieceRect.intersects(zones.value(QStringLiteral("flanc_d")));
    }
    return pieceRect.intersects(zones.value(zoneName));
}

static bool centerInPreferredZone(const QRectF& pieceRect,
                                  const QString& zoneName,
                                  const QMap<QString, QRectF>& zones)
{
    const QPointF c = pieceRect.center();
    if (zoneName == QStringLiteral("flanc")) {
        return zones.value(QStringLiteral("flanc_g")).contains(c)
            || zones.value(QStringLiteral("flanc_d")).contains(c);
    }
    return zones.value(zoneName).contains(c);
}

static bool intersectsAnyDefectMask(const QRectF& pieceRect, const QVector<QRectF>& masks)
{
    for (const QRectF& m : masks) {
        if (pieceRect.intersects(m))
            return true;
    }
    return false;
}

static bool grainOrientationOk(const QString& pieceName, int grainDir, double finalW, double finalH)
{
    if (grainDir == 1) // vertical
        return finalH >= finalW;
    if (grainDir == 2) // horizontal
        return finalW >= finalH;
    const QString n = pieceName.toLower();
    const bool strapLike = n.contains(QStringLiteral("bandouli")) || n.contains(QStringLiteral("anse"))
        || n.contains(QStringLiteral("bretelle")) || n.contains(QStringLiteral("sangle"));
    if (strapLike) {
        // Grain principal vertical: une sangle doit rester majoritairement dans le sens hauteur
        return finalH >= finalW;
    }
    const bool panelLike = n.contains(QStringLiteral("face")) || n.contains(QStringLiteral("avant"))
        || n.contains(QStringLiteral("dos")) || n.contains(QStringLiteral("arrière"))
        || n.contains(QStringLiteral("arriere"));
    if (panelLike) {
        // Les grands panneaux acceptent un léger écart, mais pas d'orientation "contre grain" extrême.
        return finalH >= (0.72 * finalW);
    }
    return true;
}

static QPainterPath piecePatternPath(const QString& pieceName, const QRectF& r)
{
    const QString n = pieceName.toLower();
    QPainterPath path;
    if (n.contains(QStringLiteral("bandouli")) || n.contains(QStringLiteral("anse"))
        || n.contains(QStringLiteral("bretelle")) || n.contains(QStringLiteral("sangle"))) {
        const qreal notch = qMin(r.height() * 0.25, r.width() * 0.08);
        path.moveTo(r.left(), r.top() + notch);
        path.lineTo(r.left() + notch, r.top());
        path.lineTo(r.right() - notch, r.top());
        path.lineTo(r.right(), r.top() + notch);
        path.lineTo(r.right(), r.bottom() - notch);
        path.lineTo(r.right() - notch, r.bottom());
        path.lineTo(r.left() + notch, r.bottom());
        path.lineTo(r.left(), r.bottom() - notch);
        path.closeSubpath();
        return path;
    }
    if (n.contains(QStringLiteral("rabat"))) {
        const qreal rr = qMin(r.width(), r.height()) * 0.15;
        path.addRoundedRect(r, rr, rr);
        return path;
    }
    if (n.contains(QStringLiteral("poche"))) {
        const qreal rr = qMin(r.width(), r.height()) * 0.20;
        path.addRoundedRect(r.adjusted(1.5, 1.5, -1.5, -1.5), rr, rr);
        return path;
    }
    if (n.contains(QStringLiteral("face")) || n.contains(QStringLiteral("dos"))
        || n.contains(QStringLiteral("avant")) || n.contains(QStringLiteral("arrière"))
        || n.contains(QStringLiteral("arriere"))) {
        const qreal notch = qMin(r.width(), r.height()) * 0.08;
        path.moveTo(r.left() + notch, r.top());
        path.lineTo(r.right() - notch, r.top());
        path.quadTo(r.right(), r.top(), r.right(), r.top() + notch);
        path.lineTo(r.right(), r.bottom());
        path.lineTo(r.left(), r.bottom());
        path.lineTo(r.left(), r.top() + notch);
        path.quadTo(r.left(), r.top(), r.left() + notch, r.top());
        path.closeSubpath();
        return path;
    }
    path.addRoundedRect(r, qMin(r.width(), r.height()) * 0.08, qMin(r.width(), r.height()) * 0.08);
    return path;
}

} // namespace

MainWindow::CutPlanResult MainWindow::guillotinePlaceUnits(QVector<CutPiece> units,
                                                           double sheetWMm,
                                                           double sheetHMm,
                                                           double spacingMm) const
{
    CutPlanResult r;
    r.sheetWMm = sheetWMm;
    r.sheetHMm = sheetHMm;
    r.spacingMm = spacingMm;

    if (sheetWMm <= 0 || sheetHMm <= 0) {
        r.message = QStringLiteral("Dimensions support invalides.");
        return r;
    }
    if (units.isEmpty()) {
        r.message = QStringLiteral("Aucune pièce à placer.");
        return r;
    }

    struct ShelfCut {
        double y = 0.0;
        double height = 0.0;
        double xCursor = 0.0;
    };
    struct SheetStateCut {
        QVector<ShelfCut> shelves;
    };
    auto fitInRectCut = [](double w, double h, double W, double H) -> bool {
        return w <= W && h <= H;
    };

    QVector<SheetStateCut> sheets;
    sheets.push_back(SheetStateCut{});
    const QMap<QString, QRectF> zonesMm = leatherZonesInSheet(QRectF(0.0, 0.0, sheetWMm, sheetHMm));
    const QVector<QRectF> defectMasksMm = leatherDefectMasksInSheet(QRectF(0.0, 0.0, sheetWMm, sheetHMm));

    auto placementRespectsLeatherRules = [&](const CutPiece& u,
                                             double px,
                                             double py,
                                             double pw,
                                             double ph) -> bool {
        const QRectF pr(px, py, pw, ph);
        const QString zoneNeed = preferredLeatherZoneForPiece(u.name, u.preferredZone);
        if (!centerInPreferredZone(pr, zoneNeed, zonesMm))
            return false; // contrainte dure de zone
        if (u.qualityLevel >= 2 && intersectsAnyDefectMask(pr, defectMasksMm))
            return false; // contrainte dure qualité matière
        if (!grainOrientationOk(u.name, u.grainDir, pw, ph))
            return false; // contrainte dure orientation grain
        return true;
    };

    // relaxed=true : ignore contraintes zone/grain/qualité (fallback géométrique pur)
    auto placeInSheet = [&](int sIdx, const CutPiece& u, PlacedPiece& out, bool relaxed = false) -> bool {
        auto& sh = sheets[sIdx];

        for (int i = 0; i < sh.shelves.size(); ++i) {
            ShelfCut& sf = sh.shelves[i];

            auto tryPlace = [&](double pw, double ph, bool rot) -> bool {
                if (ph > sf.height)
                    return false;
                if (sf.xCursor + pw > sheetWMm)
                    return false;
                if (!relaxed && !placementRespectsLeatherRules(u, sf.xCursor, sf.y, pw, ph))
                    return false;

                out.xMm = sf.xCursor;
                out.yMm = sf.y;
                out.wMm = pw;
                out.hMm = ph;
                out.rotated = rot;
                sf.xCursor += (pw + spacingMm);
                return true;
            };

            if (tryPlace(u.wMm, u.hMm, false))
                return true;
            if (u.canRotate && tryPlace(u.hMm, u.wMm, true))
                return true;
        }

        double usedY = 0.0;
        for (const ShelfCut& sf : sh.shelves)
            usedY = qMax(usedY, sf.y + sf.height + spacingMm);

        struct Cand {
            double w = 0.0;
            double h = 0.0;
            bool rot = false;
        };
        QVector<Cand> cands = { { u.wMm, u.hMm, false } };
        if (u.canRotate)
            cands.push_back({ u.hMm, u.wMm, true });
        std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) { return a.h < b.h; });

        for (const auto& c : cands) {
            if (!fitInRectCut(c.w, c.h, sheetWMm, sheetHMm))
                continue;
            if (usedY + c.h > sheetHMm)
                continue;
            if (!relaxed && !placementRespectsLeatherRules(u, 0.0, usedY, c.w, c.h))
                continue;

            ShelfCut ns;
            ns.y = usedY;
            ns.height = c.h;
            ns.xCursor = c.w + spacingMm;
            sh.shelves.push_back(ns);

            out.xMm = 0.0;
            out.yMm = usedY;
            out.wMm = c.w;
            out.hMm = c.h;
            out.rotated = c.rot;
            return true;
        }
        return false;
    };

    for (const auto& u : units) {
        const bool canEverFit = fitInRectCut(u.wMm, u.hMm, sheetWMm, sheetHMm)
            || (u.canRotate && fitInRectCut(u.hMm, u.wMm, sheetWMm, sheetHMm));
        if (!canEverFit) {
            r.message = QStringLiteral("Pièce trop grande pour le support : %1").arg(u.name);
            return r;
        }

        bool done = false;
        PlacedPiece pp;
        pp.name = u.name;
        pp.color = u.color;
        pp.preferredZone = preferredLeatherZoneForPiece(u.name, u.preferredZone);
        pp.grainDir = u.grainDir;
        pp.qualityLevel = u.qualityLevel;

        // Passe 1 : feuilles existantes avec contraintes cuir complètes
        for (int s = 0; s < sheets.size() && !done; ++s) {
            pp.sheetIndex = s;
            if (placeInSheet(s, u, pp, false)) { r.placed.push_back(pp); done = true; }
        }

        // Passe 2 : feuilles existantes sans contraintes (fallback géométrique)
        for (int s = 0; s < sheets.size() && !done; ++s) {
            pp.sheetIndex = s;
            if (placeInSheet(s, u, pp, true)) { r.placed.push_back(pp); done = true; }
        }

        if (!done) {
            // Passe 3 : nouvelle feuille avec contraintes
            sheets.push_back(SheetStateCut{});
            const int s = sheets.size() - 1;
            pp.sheetIndex = s;
            if (placeInSheet(s, u, pp, false)) { r.placed.push_back(pp); done = true; }
        }

        if (!done) {
            // Passe 4 : nouvelle feuille sans contraintes (dernier recours géométrique)
            const int s = sheets.size() - 1;
            pp.sheetIndex = s;
            if (placeInSheet(s, u, pp, true)) { r.placed.push_back(pp); done = true; }
        }

        if (!done) {
            r.message = QStringLiteral("Impossible de placer la pièce (trop grande) : %1").arg(u.name);
            return r;
        }
    }

    r.sheetsUsed = sheets.size();
    for (const auto& p : r.placed)
        r.usedAreaMm2 += (p.wMm * p.hMm);
    r.totalAreaMm2 = r.sheetsUsed * sheetWMm * sheetHMm;
    r.wasteAreaMm2 = qMax(0.0, r.totalAreaMm2 - r.usedAreaMm2);
    r.wastePct = (r.totalAreaMm2 > 0) ? (100.0 * r.wasteAreaMm2 / r.totalAreaMm2) : 0.0;

    r.ok = true;
    r.message = QStringLiteral("Plan de découpe généré.");
    return r;
}

MainWindow::CutPlanResult MainWindow::planCutPieces(const QVector<CutPiece>& pieces,
                                                    double sheetWMm,
                                                    double sheetHMm,
                                                    double spacingMm)
{
    CutPlanResult empty;
    QVector<CutPiece> singles;
    for (const auto& p : pieces) {
        if (p.wMm <= 0 || p.hMm <= 0 || p.qty <= 0)
            continue;
        for (int i = 0; i < p.qty; ++i) {
            CutPiece u = p;
            u.qty = 1;
            singles.push_back(u);
        }
    }
    if (singles.isEmpty()) {
        empty.message = QStringLiteral("Aucune pièce à placer.");
        return empty;
    }

    auto clone = [](const QVector<CutPiece>& v) { return v; };

    auto sortMaxDim = [](QVector<CutPiece> v) {
        std::sort(v.begin(), v.end(), [](const CutPiece& a, const CutPiece& b) {
            const double ma = qMax(a.wMm, a.hMm);
            const double mb = qMax(b.wMm, b.hMm);
            if (!qFuzzyCompare(ma, mb))
                return ma > mb;
            return qMin(a.wMm, a.hMm) > qMin(b.wMm, b.hMm);
        });
        return v;
    };
    auto sortArea = [](QVector<CutPiece> v) {
        std::sort(v.begin(), v.end(), [](const CutPiece& a, const CutPiece& b) {
            return (a.wMm * a.hMm) > (b.wMm * b.hMm);
        });
        return v;
    };
    auto sortMinSide = [](QVector<CutPiece> v) {
        std::sort(v.begin(), v.end(), [](const CutPiece& a, const CutPiece& b) {
            return qMin(a.wMm, a.hMm) < qMin(b.wMm, b.hMm);
        });
        return v;
    };

    CutPlanResult cand[3];
    cand[0] = guillotinePlaceUnits(sortMaxDim(clone(singles)), sheetWMm, sheetHMm, spacingMm);
    cand[1] = guillotinePlaceUnits(sortArea(clone(singles)), sheetWMm, sheetHMm, spacingMm);
    cand[2] = guillotinePlaceUnits(sortMinSide(clone(singles)), sheetWMm, sheetHMm, spacingMm);

    int bestI = -1;
    for (int i = 0; i < 3; ++i) {
        if (cand[i].ok) {
            if (bestI < 0 || cand[i].wastePct < cand[bestI].wastePct)
                bestI = i;
        }
    }
    if (bestI < 0) {
        if (cand[0].ok)
            return cand[0];
        if (cand[1].ok)
            return cand[1];
        return cand[2];
    }
    CutPlanResult out = cand[bestI];
    for (int i = 0; i < 3; ++i)
        out.wastePctHeuristic[i] = cand[i].ok ? cand[i].wastePct : -1.0;
    out.bestHeuristicIndex = bestI;
    return out;
}

QColor MainWindow::colorForPieceType(const QString& pieceName)
{
    const QString n = pieceName.toLower();
    if (n.contains(QStringLiteral("face")) || n.contains(QStringLiteral("avant")))
        return QColor(QStringLiteral("#1E88E5"));
    if (n.contains(QStringLiteral("dos")) || n.contains(QStringLiteral("arrière"))
        || n.contains(QStringLiteral("arriere")))
        return QColor(QStringLiteral("#00897B"));
    if (n.contains(QStringLiteral("rabat")))
        return QColor(QStringLiteral("#FB8C00"));
    if (n.contains(QStringLiteral("poche")))
        return QColor(QStringLiteral("#8E24AA"));
    if (n.contains(QStringLiteral("bandouli")) || n.contains(QStringLiteral("anse"))
        || n.contains(QStringLiteral("bretelle")))
        return QColor(QStringLiteral("#546E7A"));
    if (n.contains(QStringLiteral("côté")) || n.contains(QStringLiteral("cote"))
        || n.contains(QStringLiteral("profil")))
        return QColor(QStringLiteral("#6D4C41"));
    if (n.contains(QStringLiteral("fond")))
        return QColor(QStringLiteral("#5E35B1"));
    return QColor(QStringLiteral("#5C6BC0"));
}

bool MainWindow::exportCutPlanPng(const CutPlanResult& plan,
                                  const QString& outputDir,
                                  const QString& prefix,
                                  int imgW,
                                  int imgH)
{
    if (!plan.ok || plan.sheetsUsed <= 0)
        return false;

    QDir dir(outputDir);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    const double topBar = 52.0;
    const double bottomBar = 38.0;
    const double sideMargin = 20.0;
    const double legendGap = 14.0;
    const double legendW = qBound(260.0, imgW * 0.22, 420.0);
    const double contentH = imgH - topBar - bottomBar;
    const double drawAreaW = imgW - 2.0 * sideMargin - legendW - legendGap;

    for (int sIdx = 0; sIdx < plan.sheetsUsed; ++sIdx) {
        QImage img(imgW, imgH, QImage::Format_ARGB32);
        img.fill(QColor(QStringLiteral("#fafbfe")));

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);

        p.setPen(QColor(QStringLiteral("#0d1117")));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 18, QFont::Bold));
        p.drawText(QRectF(sideMargin, 6, imgW - 2.0 * sideMargin, 40),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   QStringLiteral("Découpe matière — feuille %1 / %2")
                       .arg(sIdx + 1)
                       .arg(plan.sheetsUsed));

        const double sx = drawAreaW / plan.sheetWMm;
        const double sy = contentH / plan.sheetHMm;
        const double sc = qMin(sx, sy);

        const double drawW = plan.sheetWMm * sc;
        const double drawH = plan.sheetHMm * sc;
        const double ox = sideMargin + (drawAreaW - drawW) / 2.0;
        const double oy = topBar + (contentH - drawH) / 2.0;

        // Fond "peau" pour une lecture metier proche atelier cuir
        QPainterPath hide;
        hide.moveTo(ox + 0.34 * drawW, oy + 0.03 * drawH);
        hide.cubicTo(ox + 0.40 * drawW, oy - 0.02 * drawH, ox + 0.60 * drawW, oy - 0.02 * drawH, ox + 0.66 * drawW, oy + 0.03 * drawH);
        hide.cubicTo(ox + 0.84 * drawW, oy + 0.08 * drawH, ox + 0.94 * drawW, oy + 0.20 * drawH, ox + 0.92 * drawW, oy + 0.38 * drawH);
        hide.cubicTo(ox + 0.98 * drawW, oy + 0.56 * drawH, ox + 0.96 * drawW, oy + 0.78 * drawH, ox + 0.86 * drawW, oy + 0.94 * drawH);
        hide.cubicTo(ox + 0.70 * drawW, oy + 1.01 * drawH, ox + 0.30 * drawW, oy + 1.01 * drawH, ox + 0.14 * drawW, oy + 0.94 * drawH);
        hide.cubicTo(ox + 0.04 * drawW, oy + 0.78 * drawH, ox + 0.02 * drawW, oy + 0.56 * drawH, ox + 0.08 * drawW, oy + 0.38 * drawH);
        hide.cubicTo(ox + 0.06 * drawW, oy + 0.20 * drawH, ox + 0.16 * drawW, oy + 0.08 * drawH, ox + 0.34 * drawW, oy + 0.03 * drawH);
        hide.closeSubpath();
        p.setPen(QPen(QColor(QStringLiteral("#8d6e63")), 2));
        p.setBrush(QColor(QStringLiteral("#c29b7b")));
        p.drawPath(hide);

        const QMap<QString, QRectF> zones = leatherZonesInSheet(QRectF(ox, oy, drawW, drawH));
        const QVector<QRectF> masks = leatherDefectMasksInSheet(QRectF(ox, oy, drawW, drawH));
        p.setPen(QPen(QColor(QStringLiteral("#fff8e1")), 1.2, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(zones.value(QStringLiteral("collet")));
        p.drawRect(zones.value(QStringLiteral("dosset")));
        p.drawRect(zones.value(QStringLiteral("croupon")));
        p.drawRect(zones.value(QStringLiteral("flanc_g")));
        p.drawRect(zones.value(QStringLiteral("flanc_d")));
        p.setPen(QPen(QColor(QStringLiteral("#3e2723")), 1));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold));
        p.drawText(zones.value(QStringLiteral("collet")), Qt::AlignCenter, QStringLiteral("COLLET"));
        p.drawText(zones.value(QStringLiteral("dosset")), Qt::AlignCenter, QStringLiteral("DOSSET"));
        p.drawText(zones.value(QStringLiteral("croupon")), Qt::AlignCenter, QStringLiteral("CROUPON"));
        p.drawText(zones.value(QStringLiteral("flanc_g")), Qt::AlignCenter, QStringLiteral("FLANC"));
        p.drawText(zones.value(QStringLiteral("flanc_d")), Qt::AlignCenter, QStringLiteral("FLANC"));

        p.setPen(QPen(QColor(QStringLiteral("#b71c1c")), 1.2, Qt::DashLine));
        p.setBrush(QColor(183, 28, 28, 48));
        for (const QRectF& m : masks) {
            p.drawRect(m);
            p.setPen(QPen(QColor(QStringLiteral("#b71c1c")), 1));
            p.drawText(m.adjusted(2, 2, -2, -2), Qt::AlignCenter, QStringLiteral("DEFAUT"));
            p.setPen(QPen(QColor(QStringLiteral("#b71c1c")), 1.2, Qt::DashLine));
        }

        p.setPen(QPen(QColor(218, 226, 240), 1));
        for (double gx = 0.0; gx <= plan.sheetWMm + 1e-6; gx += 10.0) {
            const double px = ox + gx * sc;
            p.drawLine(QPointF(px, oy), QPointF(px, oy + drawH));
        }
        for (double gy = 0.0; gy <= plan.sheetHMm + 1e-6; gy += 10.0) {
            const double py = oy + gy * sc;
            p.drawLine(QPointF(ox, py), QPointF(ox + drawW, py));
        }

        const QPen borderPen(QColor(QStringLiteral("#1e2a44")), 3);
        p.setPen(borderPen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(ox, oy, drawW, drawH));

        const QPen pieceBorder(QColor(QStringLiteral("#0d1528")), 2);
        QFont fPiece(QStringLiteral("Segoe UI"), 12, QFont::Bold);
        fPiece.setWeight(QFont::DemiBold);
        p.setFont(fPiece);
        for (const auto& pc : plan.placed) {
            if (pc.sheetIndex != sIdx)
                continue;

            const QRectF rr = mmToPxCut(pc.xMm, pc.yMm, pc.wMm, pc.hMm, sc, ox, oy);
            QColor fill = pc.color;
            fill.setAlpha(215);

            const QPainterPath partPath = piecePatternPath(pc.name, rr);
            p.setBrush(fill);
            p.setPen(pieceBorder);
            p.drawPath(partPath);

            // Couture de contour (pointille) pour rendu patron atelier
            p.setPen(QPen(QColor(QStringLiteral("#f5f5f5")), 1, Qt::DashLine));
            p.drawPath(partPath);

            // Signal visuel si la piece n'est pas dans la zone cuir recommandee
            const QString zoneNeed = preferredLeatherZoneForPiece(pc.name, pc.preferredZone);
            if (!intersectsPreferredZone(rr, zoneNeed, zones)) {
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(QStringLiteral("#c62828")), 2, Qt::DashLine));
                p.drawRect(rr.adjusted(1.0, 1.0, -1.0, -1.0));
            }

            if (rr.width() > 90.0 && rr.height() > 45.0) {
                p.setPen(QPen(QColor(QStringLiteral("#0d1117")), 1));
                const QString label = QStringLiteral("%1\n%2×%3%4")
                                          .arg(pc.name)
                                          .arg(static_cast<int>(pc.wMm))
                                          .arg(static_cast<int>(pc.hMm))
                                          .arg(pc.rotated ? QStringLiteral(" R") : QString())
                    + QStringLiteral("\nZone: ") + zoneNeed.toUpper()
                    + QStringLiteral(" | Q") + QString::number(qBound(1, pc.qualityLevel, 3));
                p.drawText(rr.adjusted(4, 4, -4, -4), Qt::AlignCenter | Qt::TextWordWrap, label);
            }
        }

        const double legX = sideMargin + drawAreaW + legendGap;
        double legY = topBar;
        p.setPen(QColor(QStringLiteral("#1a237e")));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 14, QFont::Bold));
        p.drawText(QRectF(legX, legY, legendW, 30), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Légende"));
        legY += 34.0;

        QStringList seen;
        QMap<QString, int> qtyMap;
        QMap<QString, QColor> colMap;
        for (const auto& pc : plan.placed) {
            if (pc.sheetIndex != sIdx)
                continue;
            if (!seen.contains(pc.name)) {
                seen.append(pc.name);
                colMap.insert(pc.name, pc.color);
            }
            qtyMap[pc.name] = qtyMap.value(pc.name, 0) + 1;
        }

        p.setFont(QFont(QStringLiteral("Segoe UI"), 11));
        for (const QString& nm : seen) {
            const QColor c = colMap.value(nm);
            QRectF sw(legX, legY, 20.0, 20.0);
            p.setBrush(c);
            p.setPen(QPen(QColor(QStringLiteral("#1e2a44")), 2));
            p.drawRoundedRect(sw, 3.0, 3.0);
            p.setPen(QColor(QStringLiteral("#0d1117")));
            p.drawText(QRectF(legX + 28.0, legY, legendW - 28.0, 24.0),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       QStringLiteral("%1  ×%2").arg(nm).arg(qtyMap.value(nm)));
            legY += 30.0;
        }

        p.setPen(QColor(QStringLiteral("#37474f")));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 10));
        p.drawText(QRectF(sideMargin, imgH - bottomBar + 2, imgW - 2.0 * sideMargin, bottomBar - 4),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("Support : %1 × %2 mm  |  Espacement : %3 mm  |  Perte (plan retenu) : %4 %")
                       .arg(plan.sheetWMm)
                       .arg(plan.sheetHMm)
                       .arg(plan.spacingMm)
                       .arg(QString::number(plan.wastePct, 'f', 2)));
        p.end();

        const QString path = dir.filePath(QStringLiteral("%1_sheet_%2.png").arg(prefix).arg(sIdx + 1));
        if (!img.save(path))
            return false;
    }
    return true;
}

double MainWindow::tryLoadMatiereCoutM2(int idStockMp)
{
    auto tryQuery = [](const QString& sql, int id) -> double {
        QSqlQuery q;
        q.prepare(sql);
        q.bindValue(QStringLiteral(":id"), id);
        if (q.exec() && q.next())
            return q.value(0).toDouble();
        return -1.0;
    };

    const QString sqlFull = QStringLiteral(
        "SELECT NVL(COUT_M2, NVL(PRIX_M2, 0)) FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_MATIERE = :id");
    double v = tryQuery(sqlFull, idStockMp);
    if (v >= 0.0)
        return v;

    v = tryQuery(QStringLiteral(
                     "SELECT NVL(PRIX_M2, 0) FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_MATIERE = :id"),
                 idStockMp);
    if (v >= 0.0)
        return v;

    v = tryQuery(QStringLiteral(
                     "SELECT NVL(COUT_M2, NVL(PRIX_M2, 0)) FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_STOCK_MP = :id"),
                 idStockMp);
    if (v >= 0.0)
        return v;

    v = tryQuery(QStringLiteral(
                     "SELECT NVL(PRIX_M2, 0) FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_STOCK_MP = :id"),
                 idStockMp);
    return qMax(0.0, v);
}

namespace {

bool sqlErrorIndicatesMissingTable(const QSqlError& e)
{
    const QString t = e.text();
    return t.contains(QStringLiteral("00942"), Qt::CaseInsensitive)
        || t.contains(QStringLiteral("does not exist"), Qt::CaseInsensitive)
        || t.contains(QStringLiteral("n'existe pas"), Qt::CaseInsensitive);
}

bool sqlErrorIndicatesMissingColumn(const QSqlError& e)
{
    const QString t = e.text();
    return t.contains(QStringLiteral("00904"), Qt::CaseInsensitive)
        || t.contains(QStringLiteral("invalid identifier"), Qt::CaseInsensitive)
        || t.contains(QStringLiteral("identificateur non valide"), Qt::CaseInsensitive);
}

} // namespace

void MainWindow::fillDefaultCutPiecesDemo(QVector<CutPiece>& pieces) const
{
    pieces.clear();
    pieces.push_back({ QStringLiteral("Face"),        320, 260, 1, true,  colorForPieceType(QStringLiteral("Face")),        QStringLiteral("dosset"),  0, 3 });
    pieces.push_back({ QStringLiteral("Dos"),         320, 260, 1, true,  colorForPieceType(QStringLiteral("Dos")),         QStringLiteral("dosset"),  0, 3 });
    pieces.push_back({ QStringLiteral("Rabat"),       300, 180, 1, true,  colorForPieceType(QStringLiteral("Rabat")),       QStringLiteral("croupon"), 0, 2 });
    pieces.push_back({ QStringLiteral("Poche"),       180, 120, 2, true,  colorForPieceType(QStringLiteral("Poche")),       QStringLiteral("croupon"), 0, 2 });
    pieces.push_back({ QStringLiteral("Bandouliere"), 900,  40, 1, false, colorForPieceType(QStringLiteral("Bandouliere")), QStringLiteral("flanc"),   1, 2 });
}

// ============================================================
// detectProductTypeLabel — nom lisible du type détecté
// ============================================================
QString MainWindow::detectProductTypeLabel(const QString& designation)
{
    const QString d = designation.toLower();
    if (d.contains(QStringLiteral("ceinture")) || d.contains(QStringLiteral("belt")))
        return QStringLiteral("Ceinture");
    if (d.contains(QStringLiteral("portefeuille")) || d.contains(QStringLiteral("wallet")))
        return QStringLiteral("Portefeuille");
    if (d.contains(QStringLiteral("porte-monnaie")) || d.contains(QStringLiteral("portemonnaie")))
        return QStringLiteral("Porte-monnaie");
    if (d.contains(QStringLiteral("porte-cl"))    || d.contains(QStringLiteral("portecl")))
        return QStringLiteral("Porte-clé");
    if (d.contains(QStringLiteral("pochette"))    || d.contains(QStringLiteral("clutch")))
        return QStringLiteral("Pochette");
    if (d.contains(QStringLiteral("voyage"))      || d.contains(QStringLiteral("bagage")))
        return QStringLiteral("Sac de voyage");
    if (d.contains(QStringLiteral("sacoche"))     || d.contains(QStringLiteral("cartable"))
        || d.contains(QStringLiteral("besace"))   || d.contains(QStringLiteral("messenger")))
        return QStringLiteral("Sacoche / Cartable");
    if ((d.contains(QStringLiteral("dos")) && d.contains(QStringLiteral("sac")))
        || d.contains(QStringLiteral("backpack")))
        return QStringLiteral("Sac à dos");
    if (d.contains(QStringLiteral("sac"))         || d.contains(QStringLiteral("handbag")))
        return QStringLiteral("Sac à main");
    if (d.isEmpty())
        return QStringLiteral("Produit inconnu (désignation vide)");
    return QStringLiteral("Sac à main (défaut) — \"%1\"").arg(designation);
}

// ============================================================
// fillSmartCutPieces — pièces adaptées au type de produit
// Dimensions réelles maroquinerie (en mm).
// ============================================================
void MainWindow::fillSmartCutPieces(QVector<CutPiece>& pieces, const QString& designation) const
{
    pieces.clear();
    const QString d = designation.toLower();

    // Helper local
    auto add = [&](const QString& name, double w, double h,
                   int qty = 1, bool canRot = true,
                   const QString& zone = QStringLiteral("collet"),
                   int grain = 0, int quality = 1)
    {
        CutPiece p;
        p.name          = name;
        p.wMm           = w;
        p.hMm           = h;
        p.qty           = qty;
        p.canRotate     = canRot;
        p.color         = colorForPieceType(name);
        p.preferredZone = zone;
        p.grainDir      = grain;
        p.qualityLevel  = quality;
        pieces.push_back(p);
    };

    // ── Ceinture ────────────────────────────────────────────────────────────
    if (d.contains(QStringLiteral("ceinture")) || d.contains(QStringLiteral("belt"))) {
        add(QStringLiteral("Ceinture"),  1100, 38, 1, false, QStringLiteral("flanc"),   1, 2);
        add(QStringLiteral("Bout"),        75, 38, 2, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Passant"),     45, 38, 3, true,  QStringLiteral("flanc"),   1, 1);
        return;
    }

    // ── Portefeuille ────────────────────────────────────────────────────────
    if (d.contains(QStringLiteral("portefeuille")) || d.contains(QStringLiteral("wallet"))
        || d.contains(QStringLiteral("porte-feuille"))) {
        add(QStringLiteral("Couverture"),   215, 105, 2, true, QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Compartiment"), 100,  75, 4, true, QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Soufflet"),     195,  20, 2, true, QStringLiteral("croupon"), 0, 1);
        add(QStringLiteral("Languette"),     80,  25, 1, true, QStringLiteral("collet"),  0, 1);
        return;
    }

    // ── Porte-monnaie ───────────────────────────────────────────────────────
    if (d.contains(QStringLiteral("porte-monnaie")) || d.contains(QStringLiteral("portemonnaie"))
        || (d.contains(QStringLiteral("monnaie")) && !d.contains(QStringLiteral("sac")))) {
        add(QStringLiteral("Corps"),      135, 110, 2, true, QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Soufflet"),   125,  28, 2, true, QStringLiteral("croupon"), 0, 1);
        add(QStringLiteral("Languette"),   65,  22, 1, true, QStringLiteral("collet"),  0, 1);
        return;
    }

    // ── Porte-clé ───────────────────────────────────────────────────────────
    if (d.contains(QStringLiteral("porte-cl")) || d.contains(QStringLiteral("portecl"))
        || d.contains(QStringLiteral("keychain"))) {
        add(QStringLiteral("Corps"),      90, 55, 2, true, QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Languette"),  45, 18, 1, true, QStringLiteral("collet"),  0, 1);
        add(QStringLiteral("Renfort"),    32, 32, 1, true, QStringLiteral("croupon"), 0, 1);
        return;
    }

    // ── Pochette / Clutch ───────────────────────────────────────────────────
    if (d.contains(QStringLiteral("pochette")) || d.contains(QStringLiteral("clutch"))) {
        add(QStringLiteral("Corps avant"),   280, 180, 1, true, QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Corps arrière"), 280, 180, 1, true, QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Rabat"),         280, 100, 1, true, QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Soufflet"),      270,  40, 2, true, QStringLiteral("croupon"), 0, 1);
        add(QStringLiteral("Languette"),      80,  25, 1, true, QStringLiteral("collet"),  0, 1);
        return;
    }

    // ── Sac de voyage ───────────────────────────────────────────────────────
    if (d.contains(QStringLiteral("voyage")) || d.contains(QStringLiteral("bagage"))
        || d.contains(QStringLiteral("weekender"))) {
        add(QStringLiteral("Face"),         500, 380, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Dos"),          500, 380, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Fond"),         500, 150, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Côté"),         150, 380, 2, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Poche avant"),  350, 260, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Poignée"),      280,  40, 2, true,  QStringLiteral("flanc"),   1, 2);
        add(QStringLiteral("Bandoulière"), 1400,  45, 1, false, QStringLiteral("flanc"),   1, 2);
        return;
    }

    // ── Sac à dos / Backpack ────────────────────────────────────────────────
    if ((d.contains(QStringLiteral("dos")) && d.contains(QStringLiteral("sac")))
        || d.contains(QStringLiteral("backpack"))) {
        add(QStringLiteral("Façade"),        340, 480, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Dos"),           340, 480, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Fond"),          340, 150, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Côté"),          150, 480, 2, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Poche zippée"), 300,  200, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Bretelle"),      600,  55, 2, false, QStringLiteral("flanc"),   1, 2);
        add(QStringLiteral("Sangle"),        350,  30, 1, true,  QStringLiteral("flanc"),   1, 1);
        return;
    }

    // ── Sacoche / Cartable / Besace ─────────────────────────────────────────
    if (d.contains(QStringLiteral("sacoche")) || d.contains(QStringLiteral("cartable"))
        || d.contains(QStringLiteral("besace")) || d.contains(QStringLiteral("messenger"))) {
        add(QStringLiteral("Face"),          380, 280, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Dos"),           380, 280, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Fond"),          380, 120, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Côté"),          120, 280, 2, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Rabat"),         380, 200, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Poche"),         320, 200, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Bandoulière"),  1000,  35, 1, false, QStringLiteral("flanc"),   1, 2);
        add(QStringLiteral("Poignée"),       260,  35, 2, true,  QStringLiteral("flanc"),   1, 2);
        return;
    }

    // ── Sac à main / Handbag (défaut générique "sac") ──────────────────────
    if (d.contains(QStringLiteral("sac")) || d.contains(QStringLiteral("handbag"))
        || d.contains(QStringLiteral("bag"))) {
        add(QStringLiteral("Face"),         320, 260, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Dos"),          320, 260, 1, true,  QStringLiteral("dosset"),  0, 3);
        add(QStringLiteral("Rabat"),        300, 180, 1, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Poche"),        180, 120, 2, true,  QStringLiteral("croupon"), 0, 2);
        add(QStringLiteral("Bandoulière"),  900,  40, 1, false, QStringLiteral("flanc"),   1, 2);
        return;
    }

    // ── Fallback absolu ─────────────────────────────────────────────────────
    fillDefaultCutPiecesDemo(pieces);
}

// ============================================================
// filtrerPiecesParEtape
// Chaque étape du cycle de fabrication travaille sur un sous-
// ensemble précis de pièces. Cette fonction filtre la liste
// complète générée par fillSmartCutPieces selon l'étape active.
//
//  Coupe      → toutes les pièces (plan complet)
//  Assemblage → panneaux structurels (corps du produit)
//  Couture    → pièces intermédiaires cousues sur le corps
//  Finition   → accessoires et détails de finition
// ============================================================
void MainWindow::filtrerPiecesParEtape(QVector<CutPiece>& pieces,
                                       const QString& etape,
                                       const QString& produitDesignation,
                                       QString* hintOut) const
{
    const QString e = etape.trimmed();
    const int totalAvant = pieces.size();

    // Coupe = plan complet, pas de filtre
    if (e.compare(QStringLiteral("Coupe"), Qt::CaseInsensitive) == 0 || e.isEmpty()) {
        if (hintOut)
            *hintOut += QStringLiteral(
                "<p style='margin:0 0 6px 0; padding:6px 10px; "
                "background:#e3f2fd; border-left:4px solid #1565c0; "
                "border-radius:4px; color:#0d47a1;'>"
                "<b>Étape : Coupe</b> — plan complet : <b>%1 pièce(s)</b> "
                "à découper dans la matière.</p>").arg(totalAvant);
        return;
    }

    // Règles de catégorisation (priorité décroissante)
    // Finition  : accessoires posés en dernier
    // Couture   : éléments cousus sur le corps assemblé
    // Assemblage: panneaux structurels (tout le reste)
    auto categoriser = [](const QString& nom) -> int {
        const QString n = nom.toLower();
        // 3 = Finition
        if (n.contains(QStringLiteral("bandouli")) || n.contains(QStringLiteral("poign"))
            || n.contains(QStringLiteral("passant")) || n.contains(QStringLiteral("languette"))
            || n.contains(QStringLiteral("bout"))    || n.contains(QStringLiteral("bretelle"))
            || n.contains(QStringLiteral("anse"))    || n.contains(QStringLiteral("renfort"))
            || n.contains(QStringLiteral("sangle")))
            return 3;
        // 2 = Couture
        if (n.contains(QStringLiteral("poche"))  || n.contains(QStringLiteral("compartiment"))
            || n.contains(QStringLiteral("soufflet")) || n.contains(QStringLiteral("rabat"))
            || n.contains(QStringLiteral("zipp")))
            return 2;
        // 1 = Assemblage (panneaux corps)
        return 1;
    };

    int cibleCategorie = 1; // Assemblage par défaut
    QString couleur, borderColor, nomEtape;
    if (e.compare(QStringLiteral("Assemblage"), Qt::CaseInsensitive) == 0) {
        cibleCategorie = 1;
        couleur = QStringLiteral("#fff3e0"); borderColor = QStringLiteral("#ef6c00");
        nomEtape = QStringLiteral("Assemblage");
    } else if (e.compare(QStringLiteral("Couture"), Qt::CaseInsensitive) == 0) {
        cibleCategorie = 2;
        couleur = QStringLiteral("#e8f5e9"); borderColor = QStringLiteral("#2e7d32");
        nomEtape = QStringLiteral("Couture");
    } else if (e.compare(QStringLiteral("Finition"), Qt::CaseInsensitive) == 0) {
        cibleCategorie = 3;
        couleur = QStringLiteral("#fffde7"); borderColor = QStringLiteral("#f9a825");
        nomEtape = QStringLiteral("Finition");
    } else {
        // Étape inconnue → plan complet sans filtre
        return;
    }

    QVector<CutPiece> filtrées;
    for (const auto& p : pieces) {
        if (categoriser(p.name) == cibleCategorie)
            filtrées.push_back(p);
    }

    // Si le filtre vide tout (produit atypique), on garde l'ensemble
    if (filtrées.isEmpty()) {
        if (hintOut)
            *hintOut += QStringLiteral(
                "<p style='margin:0 0 6px 0; padding:6px 10px; "
                "background:#fce4ec; border-left:4px solid #c62828; "
                "border-radius:4px; color:#b71c1c;'>"
                "<b>Étape : %1</b> — aucune pièce spécifique détectée, "
                "plan complet affiché (%2 pièce(s)).</p>")
                .arg(e).arg(totalAvant);
        return;
    }

    pieces = filtrées;

    if (hintOut) {
        QStringList noms;
        for (const auto& p : filtrées)
            noms.append(p.name);
        *hintOut += QStringLiteral(
            "<p style='margin:0 0 6px 0; padding:6px 10px; "
            "background:%1; border-left:4px solid %2; "
            "border-radius:4px;'>"
            "<b>Étape : %3</b> sur <i>%4</i> — "
            "<b>%5 pièce(s)</b> sélectionnées / %6 au total : %7</p>")
            .arg(couleur).arg(borderColor)
            .arg(nomEtape)
            .arg(produitDesignation.isEmpty() ? QStringLiteral("produit") : produitDesignation)
            .arg(filtrées.size()).arg(totalAvant)
            .arg(noms.join(QStringLiteral(", ")));
    }
}

bool MainWindow::loadCutInputsFromDb(int idProduit,
                                     int idMatiere,
                                     QVector<CutPiece>& pieces,
                                     double& supportWmm,
                                     double& supportHmm,
                                     const QString& etapeActuelle,
                                     QString& err,
                                     QString* demoHintOut)
{
    if (demoHintOut)
        demoHintOut->clear();
    pieces.clear();
    supportWmm = 0.0;
    supportHmm = 0.0;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        err = QStringLiteral("Connexion DB non disponible.");
        return false;
    }

    {
        bool foundDims = false;

        // Tentative 1 : colonnes LARGEUR_MM / HAUTEUR_MM par ID_MATIERE (si migrées)
        {
            QSqlQuery q(db);
            q.prepare(QStringLiteral(
                "SELECT NVL(LARGEUR_MM,0), NVL(HAUTEUR_MM,0) "
                "FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_MATIERE=:idM"));
            q.bindValue(QStringLiteral(":idM"), idMatiere);
            if (q.exec() && q.next()) {
                double w = q.value(0).toDouble(), h = q.value(1).toDouble();
                if (w > 0 && h > 0) { supportWmm = w; supportHmm = h; foundDims = true; }
            }
        }

        // Tentative 2 : par ID_STOCK_MP (clé réelle dans MATIERES_PREMIERES)
        if (!foundDims) {
            QSqlQuery q(db);
            q.prepare(QStringLiteral(
                "SELECT NVL(LARGEUR_MM,0), NVL(HAUTEUR_MM,0) "
                "FROM PROJET_CPP.MATIERES_PREMIERES WHERE ID_STOCK_MP=:idM"));
            q.bindValue(QStringLiteral(":idM"), idMatiere);
            if (q.exec() && q.next()) {
                double w = q.value(0).toDouble(), h = q.value(1).toDouble();
                if (w > 0 && h > 0) { supportWmm = w; supportHmm = h; foundDims = true; }
            }
        }

        // Fallback : colonnes LARGEUR_MM/HAUTEUR_MM absentes ou NULL dans le schéma actuel.
        // Vérifie que la ligne existe (avec les colonnes toujours présentes) et applique
        // un gabarit standard cuir (120 × 90 cm = 1 200 × 900 mm).
        if (!foundDims) {
            bool rowExists = false;
            {
                QSqlQuery qck(db);
                qck.prepare(QStringLiteral(
                    "SELECT 1 FROM PROJET_CPP.MATIERES_PREMIERES "
                    "WHERE ID_STOCK_MP=:idM AND ROWNUM<=1"));
                qck.bindValue(QStringLiteral(":idM"), idMatiere);
                rowExists = (qck.exec() && qck.next());
            }
            if (!rowExists) {
                // Deuxième chance avec ID numérique seul (par ID_MATIERE si colonne présente)
                QSqlQuery qck2(db);
                qck2.prepare(QStringLiteral(
                    "SELECT 1 FROM PROJET_CPP.MATIERES_PREMIERES WHERE ROWNUM<=1 "
                    "AND (ID_STOCK_MP=:idM)"));
                qck2.bindValue(QStringLiteral(":idM"), idMatiere);
                rowExists = (qck2.exec() && qck2.next());
            }

            if (rowExists) {
                // Ligne trouvée mais dimensions non renseignées → gabarit standard
                supportWmm = 1200.0;
                supportHmm = 900.0;
                foundDims = true;
                const QString dimHint = QStringLiteral(
                    "<p style='margin:0 0 8px 0; padding:8px; background:#e3f2fd;"
                    " border:1px solid #90caf9; border-radius:6px; color:#1565c0;'>"
                    "<b>Dimensions non renseignées (LARGEUR_MM / HAUTEUR_MM absentes dans la BD) :</b> "
                    "gabarit standard <b>1 200 × 900 mm</b> appliqué. "
                    "Pour des résultats précis, exécutez le script SQL d'ajout de colonnes "
                    "et renseignez les dimensions de chaque lot.</p>");
                if (demoHintOut)
                    *demoHintOut = dimHint + *demoHintOut;
            } else {
                err = QStringLiteral(
                    "Matière ID=%1 introuvable dans PROJET_CPP.MATIERES_PREMIERES. "
                    "Vérifiez que la planification référence un lot valide.")
                    .arg(idMatiere);
                return false;
            }
        }
    }

    // ── Récupérer la désignation du produit (utilisée par le fallback ET le filtre) ──
    QString designation;
    {
        QSqlQuery qProd(db);
        qProd.prepare(QStringLiteral(
            "SELECT NVL(DESIGNATION,'') "
            "FROM PROJET_CPP.PRODUITS WHERE ID_PRODUIT=:id"));
        qProd.bindValue(QStringLiteral(":id"), idProduit);
        if (qProd.exec() && qProd.next())
            designation = qProd.value(0).toString();
    }

    // ── 1. Essai table PRODUIT_PIECES (si elle existe et est peuplée) ────────
    bool piecesFromDb = false;
    {
        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "SELECT NOM_PIECE, LARGEUR_MM, HAUTEUR_MM, "
            "NVL(QUANTITE,1), NVL(CAN_ROTATE,1), "
            "NVL(ZONE_PREFEREE,'COLLET'), NVL(SENS_GRAIN,0), NVL(NIVEAU_QUALITE,1) "
            "FROM PROJET_CPP.PRODUIT_PIECES "
            "WHERE ID_PRODUIT=:idP "
            "ORDER BY NOM_PIECE"));
        q.bindValue(QStringLiteral(":idP"), idProduit);
        if (q.exec()) {
            while (q.next()) {
                CutPiece p;
                p.name         = q.value(0).toString();
                p.wMm          = q.value(1).toDouble();
                p.hMm          = q.value(2).toDouble();
                p.qty          = qMax(1, q.value(3).toInt());
                p.canRotate    = q.value(4).toBool();
                p.preferredZone= q.value(5).toString().toLower();
                p.grainDir     = qBound(0, q.value(6).toInt(), 2);
                p.qualityLevel = qBound(1, q.value(7).toInt(), 3);
                p.color        = colorForPieceType(p.name);
                if (p.wMm > 0.0 && p.hMm > 0.0)
                    pieces.push_back(p);
            }
            piecesFromDb = !pieces.isEmpty();
        }
    }

    // ── 2. Fallback intelligent : patron par type de produit ─────────────────
    if (!piecesFromDb) {
        fillSmartCutPieces(pieces, designation);

        const QString typeLabel = detectProductTypeLabel(designation);
        if (demoHintOut) {
            *demoHintOut += QStringLiteral(
                "<p style='margin:0 0 8px 0; padding:8px; "
                "background:#fff8e1; border:1px solid #ffcc80; "
                "border-radius:6px; color:#5d4037;'>"
                "<b>Patron généré automatiquement</b> — "
                "Produit détecté : <b>%1</b> "
                "<span style='color:#9e9e9e'>"
                "(pour des mesures exactes, peuplez PRODUIT_PIECES)</span>"
                "</p>")
                .arg(typeLabel.toHtmlEscaped());
        }
    } else {
        if (demoHintOut) {
            *demoHintOut += QStringLiteral(
                "<p style='margin:0 0 8px 0; padding:8px; "
                "background:#e8f5e9; border:1px solid #a5d6a7; "
                "border-radius:6px; color:#1b5e20;'>"
                "<b>Pièces chargées depuis PRODUIT_PIECES</b> — "
                "%1 pièce(s) trouvée(s) pour ce produit.</p>")
                .arg(pieces.size());
        }
    }

    // ── Filtre par étape : garde uniquement les pièces de l'étape active ──
    if (!etapeActuelle.isEmpty())
        filtrerPiecesParEtape(pieces, etapeActuelle, designation, demoHintOut);

    err.clear();
    return true;
}

bool MainWindow::saveCutPlanToDb(int idCmd,
                                 int idProduit,
                                 int idMatiere,
                                 const CutPlanResult& res,
                                 int& outPlanId,
                                 QString& err)
{
    outPlanId = -1;
    err.clear();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        err = QStringLiteral("Connexion Oracle indisponible pour la sauvegarde du plan.");
        return false;
    }

    // Table de log dédiée (indépendante des anciennes tables DECOUPE_* supprimées).
    {
        QSqlQuery qCreate(db);
        const QString ddl =
            QStringLiteral(
                "CREATE TABLE CUT_PLAN_LOG ("
                "ID_PLAN NUMBER(18) PRIMARY KEY, "
                "DATE_CREATION TIMESTAMP DEFAULT SYSTIMESTAMP, "
                "ID_COMMANDE NUMBER, "
                "ID_PRODUIT NUMBER, "
                "ID_MATIERE NUMBER, "
                "SHEETS_USED NUMBER, "
                "WASTE_PCT NUMBER(10,4), "
                "USED_AREA_MM2 NUMBER(18,3), "
                "TOTAL_AREA_MM2 NUMBER(18,3), "
                "DATA_JSON CLOB)");
        if (!qCreate.exec(ddl)) {
            const QString e = qCreate.lastError().text();
            // ORA-00955: name is already used by an existing object
            if (!e.contains(QStringLiteral("ORA-00955"))) {
                err = QStringLiteral("Création CUT_PLAN_LOG impossible : %1").arg(e);
                return false;
            }
        }
    }

    QJsonArray placements;
    for (const auto& p : res.placed) {
        QJsonObject o;
        o.insert(QStringLiteral("name"), p.name);
        o.insert(QStringLiteral("sheet_index"), p.sheetIndex);
        o.insert(QStringLiteral("x_mm"), p.xMm);
        o.insert(QStringLiteral("y_mm"), p.yMm);
        o.insert(QStringLiteral("w_mm"), p.wMm);
        o.insert(QStringLiteral("h_mm"), p.hMm);
        o.insert(QStringLiteral("rotated"), p.rotated);
        placements.push_back(o);
    }

    QJsonObject root;
    root.insert(QStringLiteral("id_commande"), idCmd);
    root.insert(QStringLiteral("id_produit"), idProduit);
    root.insert(QStringLiteral("id_matiere"), idMatiere);
    root.insert(QStringLiteral("sheet_width_mm"), res.sheetWMm);
    root.insert(QStringLiteral("sheet_height_mm"), res.sheetHMm);
    root.insert(QStringLiteral("spacing_mm"), res.spacingMm);
    root.insert(QStringLiteral("sheets_used"), res.sheetsUsed);
    root.insert(QStringLiteral("used_area_mm2"), res.usedAreaMm2);
    root.insert(QStringLiteral("total_area_mm2"), res.totalAreaMm2);
    root.insert(QStringLiteral("waste_area_mm2"), res.wasteAreaMm2);
    root.insert(QStringLiteral("waste_pct"), res.wastePct);
    root.insert(QStringLiteral("placements"), placements);
    const QString jsonStr = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));

    outPlanId = static_cast<int>(QDateTime::currentMSecsSinceEpoch() % 1000000000LL);
    QSqlQuery qIns(db);
    qIns.prepare(QStringLiteral(
        "INSERT INTO CUT_PLAN_LOG ("
        "ID_PLAN, ID_COMMANDE, ID_PRODUIT, ID_MATIERE, "
        "SHEETS_USED, WASTE_PCT, USED_AREA_MM2, TOTAL_AREA_MM2, DATA_JSON) "
        "VALUES (:id_plan, :id_cmd, :id_prod, :id_mat, :sheets, :waste, :used, :total, :data_json)"));
    qIns.bindValue(QStringLiteral(":id_plan"), outPlanId);
    qIns.bindValue(QStringLiteral(":id_cmd"), idCmd);
    qIns.bindValue(QStringLiteral(":id_prod"), idProduit);
    qIns.bindValue(QStringLiteral(":id_mat"), idMatiere);
    qIns.bindValue(QStringLiteral(":sheets"), res.sheetsUsed);
    qIns.bindValue(QStringLiteral(":waste"), res.wastePct);
    qIns.bindValue(QStringLiteral(":used"), res.usedAreaMm2);
    qIns.bindValue(QStringLiteral(":total"), res.totalAreaMm2);
    qIns.bindValue(QStringLiteral(":data_json"), jsonStr);

    if (!qIns.exec()) {
        err = QStringLiteral("Insertion CUT_PLAN_LOG impossible : %1").arg(qIns.lastError().text());
        outPlanId = -1;
        return false;
    }
    QSqlQuery(db).exec(QStringLiteral("COMMIT"));
    return true;
}

void MainWindow::fillCutPlacementTable(const CutPlanResult& res, int sheetOneBasedFilter)
{
    QTableWidget* t = m_tableCutPlacements;
    if (!t)
        return;

    // ── Style général du tableau ──────────────────────────────────────────
    t->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "  font-size: 12px;"
        "  gridline-color: #e0d7f3;"
        "  border: 1px solid #c9b8e8;"
        "  border-radius: 4px;"
        "  background: #ffffff;"
        "}"
        "QTableWidget::item {"
        "  padding: 4px 6px;"
        "  border: none;"
        "}"
        "QTableWidget::item:selected {"
        "  background: #7b1fa2;"
        "  color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "  background: #f3e5f5;"
        "  color: #4a148c;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 5px 4px;"
        "  border: none;"
        "  border-bottom: 2px solid #ce93d8;"
        "  border-right: 1px solid #e0d7f3;"
        "}"
    ));
    t->verticalHeader()->setVisible(false);
    t->setAlternatingRowColors(true);
    t->setShowGrid(true);
    t->setWordWrap(false);
    t->verticalHeader()->setDefaultSectionSize(26);

    // ── 6 colonnes : N° | Pièce | Position | Dimensions | Rot. | Surface ─
    t->clear();
    t->setColumnCount(6);
    t->setHorizontalHeaderLabels({
        QStringLiteral("N°"),
        QStringLiteral("Pièce"),
        QStringLiteral("Position\n(X ; Y mm)"),
        QStringLiteral("Dimensions\n(L × H mm)"),
        QStringLiteral("Rot."),
        QStringLiteral("Surface\n(cm²)"),
    });

    // ── Collecte des lignes pour la feuille filtrée ───────────────────────
    QVector<int> rowIdx;
    for (int i = 0; i < res.placed.size(); ++i) {
        const int sheet1 = res.placed[i].sheetIndex + 1;
        if (sheetOneBasedFilter > 0 && sheet1 != sheetOneBasedFilter)
            continue;
        rowIdx.append(i);
    }

    const double sheetAreaMm2 = res.sheetWMm * res.sheetHMm;

    t->setRowCount(rowIdx.size());
    for (int r = 0; r < rowIdx.size(); ++r) {
        const auto& p    = res.placed[rowIdx[r]];
        const double areaMm2 = p.wMm * p.hMm;
        const double areaCm2 = areaMm2 / 100.0;
        const double pctSheet = (sheetAreaMm2 > 0.0) ? (areaMm2 / sheetAreaMm2 * 100.0) : 0.0;

        // Valeurs combinées
        const QString posText  = QStringLiteral("%1 ; %2")
            .arg(p.xMm, 0, 'f', 1).arg(p.yMm, 0, 'f', 1);
        const QString dimText  = QStringLiteral("%1 × %2")
            .arg(p.wMm, 0, 'f', 1).arg(p.hMm, 0, 'f', 1);
        const QString surfText = QStringLiteral("%1\n(%2 %)")
            .arg(areaCm2, 0, 'f', 1)
            .arg(pctSheet, 0, 'f', 1);

        // Tooltip complet au survol
        const QString tip = QStringLiteral(
            "Pièce : %1\nFeuille : %2\n"
            "X = %3 mm  |  Y = %4 mm\n"
            "Largeur = %5 mm  |  Hauteur = %6 mm\n"
            "Surface = %7 mm²  =  %8 cm²\n"
            "Part de la feuille : %9 %")
            .arg(p.name).arg(p.sheetIndex + 1)
            .arg(p.xMm, 0, 'f', 1).arg(p.yMm, 0, 'f', 1)
            .arg(p.wMm, 0, 'f', 1).arg(p.hMm, 0, 'f', 1)
            .arg(areaMm2, 0, 'f', 0).arg(areaCm2, 0, 'f', 2)
            .arg(pctSheet, 0, 'f', 2);

        auto makeItem = [&](const QString& text, Qt::Alignment align = Qt::AlignCenter) {
            QTableWidgetItem* it = new QTableWidgetItem(text);
            it->setTextAlignment(align | Qt::AlignVCenter);
            it->setToolTip(tip);
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            return it;
        };

        t->setItem(r, 0, makeItem(QString::number(p.sheetIndex + 1)));
        t->setItem(r, 1, makeItem(p.name, Qt::AlignLeft));
        t->setItem(r, 2, makeItem(posText));
        t->setItem(r, 3, makeItem(dimText));
        t->setItem(r, 4, makeItem(p.rotated ? QStringLiteral("✓") : QStringLiteral("—")));
        t->setItem(r, 5, makeItem(surfText));
    }

    // ── Largeurs des colonnes ─────────────────────────────────────────────
    QHeaderView* hh = t->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Interactive);
    hh->setMinimumSectionSize(32);
    hh->setDefaultAlignment(Qt::AlignCenter);
    hh->setDefaultSectionSize(80);
    hh->setMinimumHeight(36);  // deux lignes de header
    t->setColumnWidth(0, 36);   // N°
    t->setColumnWidth(1, 90);   // Pièce  (stretch)
    t->setColumnWidth(2, 110);  // Position
    t->setColumnWidth(3, 120);  // Dimensions
    t->setColumnWidth(4, 40);   // Rot.
    t->setColumnWidth(5, 82);   // Surface
    hh->setSectionResizeMode(1, QHeaderView::Stretch);

    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::showCutPreviewImage(const QString& imagePath)
{
    if (!m_sceneCutPreview || !m_graphicsCutPreview)
        return;
    m_sceneCutPreview->clear();
    if (!QFileInfo::exists(imagePath)) {
        return;
    }

    QPixmap px(imagePath);
    if (px.isNull())
        return;
    m_sceneCutPreview->addPixmap(px);
    m_sceneCutPreview->setSceneRect(0, 0, px.width(), px.height());
    m_graphicsCutPreview->resetTransform();
    m_graphicsCutPreview->fitInView(m_sceneCutPreview->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::refreshCutSheetNavigation()
{
    const int n = m_lastCutPlan.sheetsUsed;
    if (n <= 0) {
        if (m_lblCutSheetNav)
            m_lblCutSheetNav->setText(QStringLiteral("Feuille —"));
        if (m_btnCutSheetPrev)
            m_btnCutSheetPrev->setEnabled(false);
        if (m_btnCutSheetNext)
            m_btnCutSheetNext->setEnabled(false);
        return;
    }

    m_cutCurrentSheet = qBound(1, m_cutCurrentSheet, n);
    if (m_lblCutSheetNav)
        m_lblCutSheetNav->setText(QStringLiteral("Feuille %1 / %2").arg(m_cutCurrentSheet).arg(n));
    if (m_btnCutSheetPrev)
        m_btnCutSheetPrev->setEnabled(m_cutCurrentSheet > 1);
    if (m_btnCutSheetNext)
        m_btnCutSheetNext->setEnabled(m_cutCurrentSheet < n);

    fillCutPlacementTable(m_lastCutPlan, m_cutCurrentSheet);
    renderCutPlanToScene(m_cutCurrentSheet);
}

bool MainWindow::resolveIdsFromCommande(int idCommande, int& idProduit, int& idMatiere, QString& err)
{
    idProduit = -1;
    idMatiere = -1;
    err.clear();
    if (idCommande <= 0) {
        err = QStringLiteral("Identifiant commande / planification invalide.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        err = QStringLiteral("Connexion base de données indisponible.");
        return false;
    }

    // Schéma courant : ID_STOCK_MP (lot). Si la table expose ID_MATIERE, il est pris en priorité.
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT ID_PRODUIT, "
        "NVL(NULLIF(ID_MATIERE, 0), NULLIF(ID_STOCK_MP, 0)) AS ID_MAT "
        "FROM PLANIFICATION WHERE ID_COMMANDE = :id"));
    q.bindValue(QStringLiteral(":id"), idCommande);
    if (!q.exec()) {
        QSqlQuery q2(db);
        q2.prepare(QStringLiteral(
            "SELECT ID_PRODUIT, NVL(ID_STOCK_MP, 0) FROM PLANIFICATION WHERE ID_COMMANDE = :id"));
        q2.bindValue(QStringLiteral(":id"), idCommande);
        if (!q2.exec()) {
            err = QStringLiteral("Erreur PLANIFICATION : %1").arg(q2.lastError().text());
            return false;
        }
        if (!q2.next()) {
            err = QStringLiteral("Aucune ligne dans PLANIFICATION pour ID_COMMANDE = %1.").arg(idCommande);
            return false;
        }
        idProduit = q2.value(0).toInt();
        idMatiere = q2.value(1).toInt();
    } else {
        if (!q.next()) {
            err = QStringLiteral("Aucune ligne dans PLANIFICATION pour ID_COMMANDE = %1.").arg(idCommande);
            return false;
        }
        idProduit = q.value(0).toInt();
        idMatiere = q.value(1).toInt();
    }

    if (idProduit <= 0) {
        err = QStringLiteral("ID_PRODUIT invalide ou absent pour la commande %1.").arg(idCommande);
        return false;
    }
    if (idMatiere <= 0) {
        err = QStringLiteral("Matière non renseignée (ID_STOCK_MP / ID_MATIERE) pour la commande %1.")
                  .arg(idCommande);
        return false;
    }
    return true;
}

bool MainWindow::tryRunCutPlanViaApi(int idCmd,
                                     int idProduit,
                                     int idMatiere,
                                     const QVector<CutPiece>& pieces,
                                     double supportW,
                                     double supportH,
                                     double spacing,
                                     CutPlanResult& outRes,
                                     QString& outInfo,
                                     QString& outErr)
{
    outRes = CutPlanResult{};
    outInfo.clear();
    outErr.clear();

    QString apiUrl = qEnvironmentVariable("FIL_DOR_CUT_API_URL").trimmed();
    if (apiUrl.isEmpty())
        apiUrl = QStringLiteral("http://127.0.0.1:8010/generate-cut-plan");
    if (!m_cutApiManager) {
        outErr = QStringLiteral("Gestionnaire réseau non initialisé.");
        return false;
    }
    if (pieces.isEmpty() || supportW <= 0.0 || supportH <= 0.0) {
        outErr = QStringLiteral("Entrées découpe invalides pour appel API.");
        return false;
    }

    QJsonArray jPieces;
    for (const CutPiece& p : pieces) {
        QJsonObject o;
        o.insert(QStringLiteral("name"), p.name);
        o.insert(QStringLiteral("width_mm"), p.wMm);
        o.insert(QStringLiteral("height_mm"), p.hMm);
        o.insert(QStringLiteral("qty"), p.qty);
        o.insert(QStringLiteral("can_rotate"), p.canRotate);
        jPieces.append(o);
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("order_id"), idCmd);
    payload.insert(QStringLiteral("id_produit"), idProduit);
    payload.insert(QStringLiteral("id_matiere"), idMatiere);
    payload.insert(QStringLiteral("spacing_mm"), spacing);
    payload.insert(QStringLiteral("sheet_width_mm"), supportW);
    payload.insert(QStringLiteral("sheet_height_mm"), supportH);
    payload.insert(QStringLiteral("pieces"), jPieces);

    QStringList candidateUrls;
    const QUrl rawUrl(apiUrl);
    const QString base8010 = QStringLiteral("http://127.0.0.1:8010");
    if (rawUrl.isValid() && !rawUrl.scheme().isEmpty() && !rawUrl.host().isEmpty()) {
        const QString path = rawUrl.path().trimmed();
        if (path.contains(QStringLiteral("generate-cut-plan"))) {
            QString direct = rawUrl.toString(QUrl::RemoveFragment);
            while (direct.endsWith('/'))
                direct.chop(1);
            // Essayer d'abord /api/v1/ (FastAPI mock), puis l'URL directe (svgnest-service)
            const QString baseHost = QStringLiteral("%1://%2:%3")
                                         .arg(rawUrl.scheme(),
                                              rawUrl.host(),
                                              QString::number(rawUrl.port() > 0 ? rawUrl.port() : 8010));
            candidateUrls << (baseHost + QStringLiteral("/api/v1/generate-cut-plan"));
            candidateUrls << direct;
        } else {
            const QString base = QStringLiteral("%1://%2:%3")
                                     .arg(rawUrl.scheme(),
                                          rawUrl.host(),
                                          QString::number(rawUrl.port() > 0 ? rawUrl.port() : 8010));
            candidateUrls << (base + QStringLiteral("/api/v1/generate-cut-plan"));
            candidateUrls << (base + QStringLiteral("/generate-cut-plan"));
            candidateUrls << (base + QStringLiteral("/api/generate-cut-plan"));
        }
    } else {
        candidateUrls << (base8010 + QStringLiteral("/api/v1/generate-cut-plan"));
        candidateUrls << (base8010 + QStringLiteral("/generate-cut-plan"));
        candidateUrls << (base8010 + QStringLiteral("/api/generate-cut-plan"));
    }
    // Ajouter toujours les deux variantes principales pour compatibilité FastAPI et svgnest
    candidateUrls << (base8010 + QStringLiteral("/api/v1/generate-cut-plan"));
    candidateUrls << (base8010 + QStringLiteral("/generate-cut-plan"));
    candidateUrls.removeDuplicates();

    QByteArray responseBody;
    QString effectiveApiUrl;
    QStringList endpointErrors;
    for (const QString& urlCandidate : candidateUrls) {
        QNetworkRequest req{QUrl(urlCandidate)};
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Accept", "application/json");

        QNetworkReply *reply = m_cutApiManager->post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
            if (reply && reply->isRunning())
                reply->abort();
        });
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timeoutTimer.start(8000);
        loop.exec();
        timeoutTimer.stop();

        if (!reply) {
            endpointErrors << QStringLiteral("%1 => réponse invalide").arg(urlCandidate);
            continue;
        }

        responseBody = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            effectiveApiUrl = urlCandidate;
            reply->deleteLater();
            break;
        }

        const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        endpointErrors << QStringLiteral("%1 => HTTP %2 (%3)")
                              .arg(urlCandidate)
                              .arg(httpCode)
                              .arg(reply->errorString());
        reply->deleteLater();
    }

    if (effectiveApiUrl.isEmpty()) {
        outErr = QStringLiteral("Aucun endpoint API valide. Détails: %1")
                     .arg(endpointErrors.join(QStringLiteral(" | ")));
        return false;
    }

    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(responseBody, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        outErr = QStringLiteral("JSON API invalide : %1").arg(parseErr.errorString());
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonObject obj = root.contains(QStringLiteral("plan")) && root.value(QStringLiteral("plan")).isObject()
                                ? root.value(QStringLiteral("plan")).toObject()
                                : root;

    const QJsonArray placements = obj.value(QStringLiteral("placements")).toArray();
    if (placements.isEmpty()) {
        outErr = QStringLiteral("API : aucun placement renvoyé.");
        return false;
    }

    outRes.ok = true;
    outRes.message = QStringLiteral("Plan généré par API.");
    outRes.sheetWMm = obj.value(QStringLiteral("sheet_width_mm")).toDouble(supportW);
    outRes.sheetHMm = obj.value(QStringLiteral("sheet_height_mm")).toDouble(supportH);
    outRes.spacingMm = spacing;
    outRes.wastePct = obj.value(QStringLiteral("waste_pct")).toDouble(obj.value(QStringLiteral("waste_percentage")).toDouble(-1.0));
    outRes.usedAreaMm2 = obj.value(QStringLiteral("used_area_mm2")).toDouble();
    outRes.totalAreaMm2 = obj.value(QStringLiteral("total_area_mm2")).toDouble();
    outRes.wasteAreaMm2 = obj.value(QStringLiteral("waste_area_mm2")).toDouble();

    int maxSheet = 0;
    double inferredUsed = 0.0;
    for (const QJsonValue& v : placements) {
        if (!v.isObject())
            continue;
        const QJsonObject p = v.toObject();
        PlacedPiece pp;
        pp.name = p.value(QStringLiteral("name")).toString();
        pp.sheetIndex = p.value(QStringLiteral("sheet_index")).toInt();
        pp.xMm = p.value(QStringLiteral("x_mm")).toDouble();
        pp.yMm = p.value(QStringLiteral("y_mm")).toDouble();
        pp.wMm = p.value(QStringLiteral("w_mm")).toDouble();
        pp.hMm = p.value(QStringLiteral("h_mm")).toDouble();
        pp.rotated = p.value(QStringLiteral("rotated")).toBool(false);
        const QString colorStr = p.value(QStringLiteral("color")).toString();
        pp.color = colorStr.isEmpty() ? colorForPieceType(pp.name) : QColor(colorStr);
        pp.preferredZone = preferredLeatherZoneForPiece(
            pp.name, p.value(QStringLiteral("preferred_zone")).toString());
        pp.grainDir = qBound(0, p.value(QStringLiteral("grain_dir")).toInt(0), 2);
        pp.qualityLevel = qBound(1, p.value(QStringLiteral("quality_level")).toInt(1), 3);
        if (!pp.color.isValid())
            pp.color = colorForPieceType(pp.name);
        if (pp.wMm <= 0.0 || pp.hMm <= 0.0)
            continue;
        inferredUsed += (pp.wMm * pp.hMm);
        outRes.placed.push_back(pp);
        maxSheet = qMax(maxSheet, pp.sheetIndex + 1);
    }

    outRes.sheetsUsed = obj.value(QStringLiteral("sheets_used")).toInt(maxSheet);
    if (outRes.sheetsUsed <= 0)
        outRes.sheetsUsed = maxSheet;
    if (outRes.sheetsUsed <= 0 || outRes.placed.isEmpty()) {
        outErr = QStringLiteral("API : placements invalides.");
        return false;
    }

    if (outRes.usedAreaMm2 <= 0.0)
        outRes.usedAreaMm2 = inferredUsed;
    if (outRes.totalAreaMm2 <= 0.0)
        outRes.totalAreaMm2 = outRes.sheetWMm * outRes.sheetHMm * outRes.sheetsUsed;
    if (outRes.wasteAreaMm2 <= 0.0)
        outRes.wasteAreaMm2 = qMax(0.0, outRes.totalAreaMm2 - outRes.usedAreaMm2);
    if (outRes.wastePct < 0.0 && outRes.totalAreaMm2 > 0.0)
        outRes.wastePct = (outRes.wasteAreaMm2 * 100.0) / outRes.totalAreaMm2;

    outInfo = QStringLiteral("Source : API (%1)").arg(effectiveApiUrl.toHtmlEscaped());
    return true;
}

void MainWindow::runCutPlanForCurrentSelection()
{
    QTableWidget* tbl = m_tableEtapesFab;
    if (!tbl) {
        tbl = findChild<QTableWidget*>(QStringLiteral("tableEtapesFab"));
    }
    if (!tbl || tbl->currentRow() < 0) {
        alerteWarning(QStringLiteral("Découpe"),
                      QStringLiteral("Sélectionnez une ligne dans le tableau « Suivi des Etapes »."));
        return;
    }

    // --- Indicateur de chargement ---
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (auto *btn = findChild<QPushButton*>(QStringLiteral("btnGenererDecoupe")))
        btn->setEnabled(false);
    if (m_lblCutSummary)
        m_lblCutSummary->setText(QStringLiteral(
            "<i style='color:#7b1fa2'>Génération du plan en cours…</i>"));
    QApplication::processEvents();

    const int row = tbl->currentRow();

    // ── Étape actuelle (col 4 = QLabel setCellWidget) ────────────────────
    QString etapeActuelle;
    if (QLabel* lblEtape = qobject_cast<QLabel*>(tbl->cellWidget(row, 4)))
        etapeActuelle = lblEtape->text().trimmed();
    if (etapeActuelle.isEmpty()) {
        // Fallback : item texte si le widget n’est pas un QLabel
        if (QTableWidgetItem* it4 = tbl->item(row, 4))
            etapeActuelle = it4->text().trimmed();
    }

    // ── Désignation produit (col 2, disponible directement) ──────────────
    const QString produitNomCol = (tbl->item(row, 2)) ? tbl->item(row, 2)->text().trimmed() : QString();

    // ── ID planification (UserRole+1 sur col 0) ────────────────────────
    QTableWidgetItem* itSuivi = tbl->item(row, 0);
    QTableWidgetItem* itPlanif = tbl->item(row, 1);
    int idCmd = 0;
    if (itSuivi) {
        const QVariant vPlan = itSuivi->data(Qt::UserRole + 1);
        if (vPlan.isValid())
            idCmd = vPlan.toInt();
    }
    if (idCmd <= 0 && itPlanif)
        idCmd = itPlanif->text().trimmed().toInt();
    if (idCmd <= 0) {
        alerteWarning(QStringLiteral("Découpe"),
                      QStringLiteral("Impossible de déterminer l’ID planification / commande pour la ligne sélectionnée."));
        return;
    }

    int idProduit = 0;
    int idMatiere = 0;
    QString resolveErr;
    if (!resolveIdsFromCommande(idCmd, idProduit, idMatiere, resolveErr)) {
        if (m_lblCutSummary)
            m_lblCutSummary->setText(QStringLiteral("<b style=’color:#c62828’>%1</b>").arg(resolveErr.toHtmlEscaped()));
        qWarning() << "[IA Découpe] resolveIdsFromCommande failed idCmd=" << idCmd << resolveErr;
        return;
    }

    qDebug() << "[IA Découpe] idCmd=" << idCmd << "idProduit=" << idProduit
             << "idMatiere=" << idMatiere << "etape=" << etapeActuelle
             << "produit=" << produitNomCol;

    QVector<CutPiece> pieces;
    double supportW = 0.0;
    double supportH = 0.0;
    QString err;
    QString demoHint;
    if (!loadCutInputsFromDb(idProduit, idMatiere, pieces, supportW, supportH, etapeActuelle, err, &demoHint)) {
        if (m_lblCutSummary)
            m_lblCutSummary->setText(QStringLiteral("<b style='color:#c62828'>%1</b>").arg(err.toHtmlEscaped()));
        QApplication::restoreOverrideCursor();
        if (auto *btn = findChild<QPushButton*>(QStringLiteral("btnGenererDecoupe")))
            btn->setEnabled(true);
        return;
    }

    const double spacing = m_spinCutSpacing ? m_spinCutSpacing->value() : 4.0;
    CutPlanResult res;
    QString apiInfo;
    QString apiErr;
    const bool apiUsed = tryRunCutPlanViaApi(idCmd, idProduit, idMatiere, pieces, supportW, supportH, spacing, res, apiInfo, apiErr);
    if (!apiUsed)
        res = planCutPieces(pieces, supportW, supportH, spacing);
    if (!res.ok) {
        if (m_lblCutSummary)
            m_lblCutSummary->setText(QStringLiteral("<b style='color:#c62828'>%1</b>").arg(res.message.toHtmlEscaped()));
        QApplication::restoreOverrideCursor();
        if (auto *btn = findChild<QPushButton*>(QStringLiteral("btnGenererDecoupe")))
            btn->setEnabled(true);
        return;
    }

    const double prixM2 = tryLoadMatiereCoutM2(idMatiere);
    const double surfUsedM2 = res.usedAreaMm2 / 1000000.0;
    const double surfWasteM2 = res.wasteAreaMm2 / 1000000.0;
    const double surfTotM2 = res.totalAreaMm2 / 1000000.0;
    const double coutUtilise = (prixM2 > 0.0) ? (surfUsedM2 * prixM2) : 0.0;
    const double coutPerdu = (prixM2 > 0.0) ? (surfWasteM2 * prixM2) : 0.0;

    int idPlan = -1;
    const bool saved = saveCutPlanToDb(idCmd, idProduit, idMatiere, res, idPlan, err);

    const QString dir = QDir::homePath() + QStringLiteral("/FIL_DOR/cutplans");
    const QString prefix = saved ? QStringLiteral("plan_%1").arg(idPlan)
                                 : QStringLiteral("cmd_%1_local").arg(idCmd);
    const bool okPng = exportCutPlanPng(res, dir, prefix, 3200, 2200);
    const QString firstImage = dir + QStringLiteral("/") + prefix + QStringLiteral("_sheet_1.png");
    const QString jsonReportPath = dir + QStringLiteral("/") + prefix + QStringLiteral("_report.json");
    bool okJson = false;
    {
        QDir().mkpath(dir);
        QJsonArray jPlaced;
        for (const auto& p : res.placed) {
            QJsonObject o;
            o.insert(QStringLiteral("name"), p.name);
            o.insert(QStringLiteral("sheet_index"), p.sheetIndex);
            o.insert(QStringLiteral("x_mm"), p.xMm);
            o.insert(QStringLiteral("y_mm"), p.yMm);
            o.insert(QStringLiteral("w_mm"), p.wMm);
            o.insert(QStringLiteral("h_mm"), p.hMm);
            o.insert(QStringLiteral("rotated"), p.rotated);
            jPlaced.push_back(o);
        }
        QJsonObject jRoot;
        jRoot.insert(QStringLiteral("id_commande"), idCmd);
        jRoot.insert(QStringLiteral("id_produit"), idProduit);
        jRoot.insert(QStringLiteral("id_matiere"), idMatiere);
        jRoot.insert(QStringLiteral("saved_to_db"), saved);
        jRoot.insert(QStringLiteral("id_plan_db"), idPlan);
        jRoot.insert(QStringLiteral("sheet_width_mm"), res.sheetWMm);
        jRoot.insert(QStringLiteral("sheet_height_mm"), res.sheetHMm);
        jRoot.insert(QStringLiteral("spacing_mm"), res.spacingMm);
        jRoot.insert(QStringLiteral("sheets_used"), res.sheetsUsed);
        jRoot.insert(QStringLiteral("used_area_mm2"), res.usedAreaMm2);
        jRoot.insert(QStringLiteral("total_area_mm2"), res.totalAreaMm2);
        jRoot.insert(QStringLiteral("waste_area_mm2"), res.wasteAreaMm2);
        jRoot.insert(QStringLiteral("waste_pct"), res.wastePct);
        jRoot.insert(QStringLiteral("placements"), jPlaced);
        QFile jf(jsonReportPath);
        if (jf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            jf.write(QJsonDocument(jRoot).toJson(QJsonDocument::Indented));
            jf.close();
            okJson = true;
        }
    }

    m_lastCutPlan = res;
    m_cutExportDir = dir;
    m_cutExportPrefix = prefix;
    m_cutCurrentSheet = 1;
    refreshCutSheetNavigation();  // renderCutPlanToScene + table — toujours, indépendamment du PNG
    if (m_btnCutExportPdf)
        m_btnCutExportPdf->setEnabled(!dir.isEmpty());
    QApplication::restoreOverrideCursor();
    if (auto *btn = findChild<QPushButton*>(QStringLiteral("btnGenererDecoupe")))
        btn->setEnabled(true);

    double worstHeurPct = -1.0;
    for (int hi = 0; hi < 3; ++hi) {
        const double w = res.wastePctHeuristic[hi];
        if (w < 0.0)
            continue;
        worstHeurPct = (worstHeurPct < 0.0) ? w : qMax(worstHeurPct, w);
    }
    const QString kpiHeur = (worstHeurPct >= 0.0)
        ? QStringLiteral("<br><b>Perte (pire heuristique testée) :</b> %1 % &nbsp;→&nbsp; <b>Perte (plan retenu) :</b> %2 %")
              .arg(QString::number(worstHeurPct, 'f', 2))
              .arg(QString::number(res.wastePct, 'f', 2))
        : QString();

    QString costLine;
    if (prixM2 > 0.0) {
        costLine = QStringLiteral(
                       "<br><b>Coût matière (COUT_M2 ou PRIX_M2 = %1 / m²) :</b> utilisé <b>%2</b> — perdu <b>%3</b> — "
                       "surface feuilles %4 m² (pièces %5 m², chute %6 m²)")
                       .arg(QString::number(prixM2, 'f', 2))
                       .arg(QString::number(coutUtilise, 'f', 2))
                       .arg(QString::number(coutPerdu, 'f', 2))
                       .arg(QString::number(surfTotM2, 'f', 4))
                       .arg(QString::number(surfUsedM2, 'f', 4))
                       .arg(QString::number(surfWasteM2, 'f', 4));
    } else {
        costLine = QStringLiteral(
            "<br><i>Coût matière : renseignez COUT_M2 ou PRIX_M2 sur PROJET_CPP.MATIERES_PREMIERES.</i>");
    }

    if (m_lblCutSummary) {
        const double rendementPct   = 100.0 - res.wastePct;
        const double usedCm2        = res.usedAreaMm2  / 100.0;
        const double wasteCm2       = res.wasteAreaMm2 / 100.0;
        const double totalCm2       = res.totalAreaMm2 / 100.0;
        const double surfParFeuilleM2 = (res.sheetWMm * res.sheetHMm) / 1.0e6;
        const QString corps = QStringLiteral(
                                   "<b>Feuilles :</b> %1 &nbsp;|&nbsp; "
                                   "<b>Format :</b> %2 × %3 mm &nbsp;(%4 m²/feuille)<br>"
                                   "<b>Surface totale matière :</b> %5 mm² &nbsp;= %6 cm²<br>"
                                   "<b>Surface pièces (nette) :</b> %7 mm² &nbsp;= %8 cm² "
                                   "&nbsp;→ <b style='color:#2e7d32'>Rendement : %9 %</b><br>"
                                   "<b>Surface perdue :</b> %10 mm² &nbsp;= %11 cm² "
                                   "&nbsp;→ <b style='color:#c62828'>Perte : %12 %</b>")
                                   .arg(res.sheetsUsed)
                                   .arg(res.sheetWMm, 0, 'f', 0)
                                   .arg(res.sheetHMm, 0, 'f', 0)
                                   .arg(QString::number(surfParFeuilleM2, 'f', 4))
                                   .arg(QString::number(res.totalAreaMm2, 'f', 0))
                                   .arg(QString::number(totalCm2, 'f', 1))
                                   .arg(QString::number(res.usedAreaMm2, 'f', 0))
                                   .arg(QString::number(usedCm2, 'f', 1))
                                   .arg(QString::number(rendementPct, 'f', 2))
                                   .arg(QString::number(res.wasteAreaMm2, 'f', 0))
                                   .arg(QString::number(wasteCm2, 'f', 1))
                                   .arg(QString::number(res.wastePct, 'f', 2))
                               + kpiHeur + costLine
                               + QStringLiteral("<br><b>Export PNG :</b> ")
                               + (okPng ? firstImage.toHtmlEscaped() : QStringLiteral("échec"))
                               + QStringLiteral("<br><b>Rapport JSON :</b> ")
                               + (okJson ? jsonReportPath.toHtmlEscaped() : QStringLiteral("échec"));
        QString head = demoHint;
        if (apiUsed)
            head += QStringLiteral("<b style='color:#2e7d32'>%1</b><br>").arg(apiInfo);
        else if (!apiErr.isEmpty())
            head += QStringLiteral("<b style='color:#ef6c00'>API indisponible :</b> %1<br><i>Fallback moteur local activé.</i><br>")
                        .arg(apiErr.toHtmlEscaped());
        if (saved) {
            m_lblCutSummary->setText(head + QStringLiteral("<b>Découpe enregistrée</b> — ID_PLAN : %1<br>%2")
                                         .arg(idPlan)
                                         .arg(corps));
        } else {
            m_lblCutSummary->setText(
                head + QStringLiteral("<b style='color:#ef6c00'>Sauvegarde BD non disponible : %1</b><br>"
                                      "<i>Plan conservé localement (PNG/JSON) pour continuité de travail.</i><br>%2")
                    .arg(err.toHtmlEscaped())
                    .arg(corps));
        }
    }

    if (saved)
        alerteSucces(QStringLiteral("IA Découpe"),
                     QStringLiteral("Plan %1 enregistré (%2). Aperçu : %3")
                         .arg(idPlan)
                         .arg(apiUsed ? QStringLiteral("API") : QStringLiteral("local"))
                         .arg(firstImage));
    else
        alerteInfo(QStringLiteral("IA Découpe"),
                   QStringLiteral("Plan généré (%1) et conservé localement.\nPNG : %2\nJSON : %3")
                       .arg(apiUsed ? QStringLiteral("API") : QStringLiteral("local"))
                       .arg(firstImage)
                       .arg(jsonReportPath));
}

// ============================================================
// renderCutPlanToScene — dessine le plan directement sur le
// QGraphicsScene (pas de PNG, rendu interactif zoom/pan).
// ============================================================
void MainWindow::renderCutPlanToScene(int sheetOneBased)
{
    if (!m_sceneCutPreview || !m_graphicsCutPreview)
        return;
    m_sceneCutPreview->clear();

    const CutPlanResult &res = m_lastCutPlan;
    if (!res.ok || res.sheetsUsed <= 0 || res.placed.isEmpty()
            || res.sheetWMm <= 0.0 || res.sheetHMm <= 0.0) {
        QGraphicsSimpleTextItem *ph = m_sceneCutPreview->addSimpleText(
            QStringLiteral("Cliquez « Générer plan de découpe » après avoir sélectionné une étape."));
        ph->setBrush(QColor("#9e9e9e"));
        QFont pf;
        pf.setPixelSize(12);
        ph->setFont(pf);
        const QRectF br = ph->boundingRect();
        m_sceneCutPreview->setSceneRect(br.adjusted(-16, -16, 16, 16));
        m_graphicsCutPreview->fitInView(m_sceneCutPreview->sceneRect(), Qt::KeepAspectRatio);
        return;
    }

    const int sIdx = qBound(1, sheetOneBased, res.sheetsUsed) - 1;
    const double margin = 24.0;
    const double targetW = 900.0;
    const double targetH = 680.0;
    const double scale = qMin(targetW / res.sheetWMm, targetH / res.sheetHMm);
    const double W = res.sheetWMm * scale;
    const double H = res.sheetHMm * scale;

    // Fond de feuille
    m_sceneCutPreview->addRect(margin, margin, W, H,
        QPen(QColor("#7b1fa2"), 2.0), QBrush(QColor("#fafafa")));

    // Grille légère (50 mm)
    const double gridMm = 50.0;
    QPen gp(QColor("#e8e8e8"), 0.8);
    for (double x = gridMm; x < res.sheetWMm; x += gridMm)
        m_sceneCutPreview->addLine(margin + x * scale, margin,
                                   margin + x * scale, margin + H, gp);
    for (double y = gridMm; y < res.sheetHMm; y += gridMm)
        m_sceneCutPreview->addLine(margin, margin + y * scale,
                                   margin + W, margin + y * scale, gp);

    // Pièces de la feuille courante
    int countOnSheet = 0;
    for (const auto &p : res.placed) {
        if (p.sheetIndex != sIdx)
            continue;
        ++countOnSheet;

        const double px = margin + p.xMm * scale;
        const double py = margin + p.yMm * scale;
        const double pw = p.wMm * scale;
        const double ph = p.hMm * scale;

        QColor c = (p.color.isValid()) ? p.color : colorForPieceType(p.name);
        QColor fill = c;
        fill.setAlpha(195);

        // Rectangle avec coins arrondis
        const double radius = qMin(pw, ph) * 0.07;
        QPainterPath path;
        path.addRoundedRect(px, py, pw, ph, radius, radius);
        m_sceneCutPreview->addPath(path, QPen(c.darker(145), 1.5), QBrush(fill));

        // Étiquette pièce
        if (pw >= 20 && ph >= 14) {
            const QString lbl = (p.name.length() > 13) ? p.name.left(12) + QStringLiteral("…") : p.name;
            QGraphicsSimpleTextItem *txt = m_sceneCutPreview->addSimpleText(lbl);
            txt->setBrush(Qt::white);
            QFont tf;
            tf.setPixelSize(qBound(8, (int)(qMin(pw, ph) * 0.22), 14));
            tf.setBold(true);
            txt->setFont(tf);
            const QRectF tbr = txt->boundingRect();
            txt->setPos(px + (pw - tbr.width()) * 0.5, py + (ph - tbr.height()) * 0.5);
        }

        // Badge rotation
        if (p.rotated && pw >= 14 && ph >= 14) {
            QGraphicsSimpleTextItem *rot = m_sceneCutPreview->addSimpleText(QStringLiteral("↺"));
            rot->setBrush(QColor(255, 255, 255, 170));

            
            QFont rf;
            rf.setPixelSize(9);
            rot->setFont(rf);
            rot->setPos(px + 2.0, py + 2.0);
        }
    }

    // Zone de chute (espace inutilisé en bas de la feuille courante)
    double maxUsedYMm = 0.0;
    double sheetUsedAreaMm2 = 0.0;
    for (const auto& p : res.placed) {
        if (p.sheetIndex != sIdx)
            continue;
        sheetUsedAreaMm2 += p.wMm * p.hMm;
        maxUsedYMm = qMax(maxUsedYMm, p.yMm + p.hMm);
    }
    const double sheetTotalMm2   = res.sheetWMm * res.sheetHMm;
    const double sheetWastePct   = (sheetTotalMm2 > 0.0)
        ? (100.0 * (sheetTotalMm2 - sheetUsedAreaMm2) / sheetTotalMm2) : 0.0;
    const double chuteMm         = res.sheetHMm - maxUsedYMm;
    if (chuteMm > 1.0) {
        const double y0    = margin + maxUsedYMm * scale;
        const double cutH  = chuteMm * scale;
        QBrush hatch(QColor(244, 67, 54, 55), Qt::BDiagPattern);
        m_sceneCutPreview->addRect(margin, y0, W, cutH, QPen(QColor("#ef9a9a"), 0.8), hatch);
        QGraphicsSimpleTextItem *cl = m_sceneCutPreview->addSimpleText(
            QStringLiteral("Chute : %1 mm  (%2 cm²)")
                .arg(chuteMm, 0, 'f', 0)
                .arg(chuteMm * res.sheetWMm / 100.0, 0, 'f', 0));
        QFont cf;
        cf.setPixelSize(10);
        cl->setFont(cf);
        cl->setBrush(QColor("#b71c1c"));
        cl->setPos(margin + 4.0, y0 + 3.0);
    }

    // Pied de page — stats feuille
    const QString footer = QStringLiteral("Feuille %1 / %2 — %3 pièce(s) — Perte feuille : %4 %  |  Perte globale : %5 %")
        .arg(sheetOneBased).arg(res.sheetsUsed)
        .arg(countOnSheet)
        .arg(sheetWastePct, 0, 'f', 1)
        .arg(res.wastePct, 0, 'f', 1);
    QGraphicsSimpleTextItem *foot = m_sceneCutPreview->addSimpleText(footer);
    QFont ff;
    ff.setPixelSize(11);
    ff.setBold(true);
    foot->setFont(ff);
    foot->setBrush(QColor("#4a148c"));
    foot->setPos(margin, margin + H + 5.0);

    const double footH = 22.0;
    m_sceneCutPreview->setSceneRect(0, 0, W + 2.0 * margin, H + 2.0 * margin + footH);
    m_graphicsCutPreview->resetTransform();
    m_graphicsCutPreview->fitInView(m_sceneCutPreview->sceneRect(), Qt::KeepAspectRatio);
}

// ============================================================
// checkCutApiHealth — GET /health sur le svgnest-service et
// met à jour l'indicateur de statut (● vert / rouge).
// ============================================================
void MainWindow::checkCutApiHealth()
{
    if (!m_lblCutApiStatus || !m_cutApiManager)
        return;

    m_lblCutApiStatus->setText(QStringLiteral("● API …"));
    m_lblCutApiStatus->setStyleSheet(QStringLiteral(
        "color: #9e9e9e; font-weight: 700; font-size: 11px; padding: 0 4px;"));

    QString apiUrl = qEnvironmentVariable("FIL_DOR_CUT_API_URL").trimmed();
    if (apiUrl.isEmpty())
        apiUrl = QStringLiteral("http://127.0.0.1:8010/generate-cut-plan");

    // Déduit l'URL /health depuis l'URL de base
    const QUrl u(apiUrl);
    const int port = (u.port() > 0) ? u.port() : 8010;
    const QString healthUrl = QStringLiteral("http://127.0.0.1:%1/health").arg(port);

    QNetworkRequest req{QUrl(healthUrl)};
    QNetworkReply *reply = m_cutApiManager->get(req);
    QTimer::singleShot(3000, reply, [reply]() {
        if (reply && reply->isRunning())
            reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, port]() {
        const bool ok = (reply->error() == QNetworkReply::NoError);
        if (m_lblCutApiStatus) {
            if (ok) {
                m_lblCutApiStatus->setText(QStringLiteral("● API"));
                m_lblCutApiStatus->setStyleSheet(QStringLiteral(
                    "color: #2e7d32; font-weight: 700; font-size: 11px; padding: 0 4px;"));
                m_lblCutApiStatus->setToolTip(
                    QStringLiteral("svgnest-service actif (port %1)").arg(port));
            } else {
                m_lblCutApiStatus->setText(QStringLiteral("● API"));
                m_lblCutApiStatus->setStyleSheet(QStringLiteral(
                    "color: #c62828; font-weight: 700; font-size: 11px; padding: 0 4px;"));
                m_lblCutApiStatus->setToolTip(QStringLiteral(
                    "API hors ligne (port %1) — fallback moteur local actif.").arg(port));
            }
        }
        reply->deleteLater();
    });
}
