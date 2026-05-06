#include "classificationiawidget.h"
#include "knn_graph_widget.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QIODevice>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPushButton>
#include <QSettings>
#include <QScrollArea>
#include <QSqlQuery>
#include <QTimer>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSslSocket>
#include <QSslError>
#include <QVariant>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

namespace {

QString envUtf8(const char *name)
{
    return QString::fromUtf8(qgetenv(name)).trimmed();
}

static bool looksLikeGroqApiKey(const QString &v)
{
    if (v.isEmpty() || v.contains(QLatin1Char(' ')))
        return false;
    return v.startsWith(QLatin1String("gsk_")) && v.size() > 20;
}

/** Clé Groq : `GROQ_API_KEY` (recommandé) ou fichier groq_api_key.txt à côté de l’exe. */
static QString pickGroqApiKey()
{
    const QString fromEnv = QString::fromUtf8(qgetenv("GROQ_API_KEY")).trimmed();
    if (looksLikeGroqApiKey(fromEnv))
        return fromEnv;

    const QString dir = QCoreApplication::applicationDirPath();
    const QStringList fileNames = {QStringLiteral("groq_api_key.txt"), QStringLiteral(".groq_api_key")};
    for (const QString &fn : fileNames) {
        QFile f(dir + QLatin1Char('/') + fn);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QString line = QString::fromUtf8(f.readLine()).trimmed();
        if (line.startsWith(QLatin1Char('\xfeff')))
            line.remove(0, 1);
        if (line.startsWith(QLatin1Char('"')) && line.endsWith(QLatin1Char('"')))
            line = line.mid(1, line.size() - 2);
        if (looksLikeGroqApiKey(line))
            return line;
    }
    return {};
}

/**
 * Modèle Groq — surcharge par GROQ_MODEL.
 * Les IDs changent sur console.groq.com/docs/models ; l’ancien llama3-8b-8192 provoque souvent HTTP 400.
 */
static QString groqModelId()
{
    const QString m = envUtf8("GROQ_MODEL");
    if (!m.isEmpty())
        return m;
    return QStringLiteral("llama-3.1-8b-instant");
}

static int groqMaxTokens()
{
    bool ok = false;
    const int t = envUtf8("GROQ_MAX_TOKENS").toInt(&ok);
    if (ok && t >= 128 && t <= 8192)
        return t;
    return 1024;
}

static QUrl groqChatCompletionsUrl()
{
    const QString u = envUtf8("GROQ_CHAT_COMPLETIONS_URL");
    if (!u.isEmpty())
        return QUrl(u);
    return QUrl(QStringLiteral("https://api.groq.com/openai/v1/chat/completions"));
}

/** Réponse OpenAI-compatible : choices[0].message.content ; repli format Inference HF si présent. */
static QString extractChatCompletionContent(const QByteArray &raw)
{
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || doc.isNull())
        return QString::fromUtf8(raw);

    if (doc.isObject()) {
        const QJsonObject root = doc.object();
        const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
        if (!choices.isEmpty()) {
            const QJsonObject msg = choices.at(0).toObject().value(QStringLiteral("message")).toObject();
            const QString c = msg.value(QStringLiteral("content")).toString();
            if (!c.isEmpty())
                return c;
        }
        if (root.contains(QStringLiteral("generated_text")))
            return root.value(QStringLiteral("generated_text")).toString();
    }
    if (doc.isArray()) {
        const QJsonArray arr = doc.array();
        if (!arr.isEmpty()) {
            const QJsonValue v0 = arr.at(0);
            if (v0.isObject()) {
                const QString g = v0.toObject().value(QStringLiteral("generated_text")).toString();
                if (!g.isEmpty())
                    return g;
            }
        }
    }
    return QString();
}

/** Erreur JSON (OpenAI / Groq : error.message). */
static QString formatApiJsonError(const QJsonValue &errVal)
{
    if (errVal.isString())
        return errVal.toString();
    if (errVal.isObject()) {
        const QJsonObject o = errVal.toObject();
        const QString msg = o.value(QStringLiteral("message")).toString();
        if (!msg.isEmpty())
            return msg;
        return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
    }
    if (errVal.isArray()) {
        QStringList parts;
        for (const QJsonValue &v : errVal.toArray())
            parts << v.toString();
        return parts.join(QLatin1String(" | "));
    }
    return QString();
}

void setStatBadge(QLabel *lb, const QString &label, const QString &value, bool scoreLarge)
{
    if (!lb)
        return;
    lb->setTextFormat(Qt::RichText);
    const QString esc = QString(value).toHtmlEscaped();
    const QString valPx = scoreLarge ? QStringLiteral("22px") : QStringLiteral("14px");
    const QString valColor = scoreLarge ? QStringLiteral("#8B6914") : QStringLiteral("#3A3228");
    lb->setText(QStringLiteral(
                      "<div align='center'>"
                      "<span style='color:#6B5D4A;font-size:9px;letter-spacing:0.18em;font-weight:700;'>%1</span><br/>"
                      "<span style='color:%4;font-size:%3;font-weight:800;'>%2</span>"
                      "</div>")
                      .arg(QString(label).toHtmlEscaped(), esc, valPx, valColor));
}

static QString niveauLetter(const QString &niveau)
{
    const QString t = niveau.trimmed();
    if (t.isEmpty())
        return QStringLiteral("—");
    return t.left(1).toUpper();
}

static int jsonIntVal(const QJsonObject &o, const QString &k, int fallback = 0)
{
    const QJsonValue v = o.value(k);
    if (v.isDouble())
        return qRound(v.toDouble());
    if (v.isString())
        return v.toString().toInt();
    return v.toInt(fallback);
}

/** Y performance (0–100) pour le graphe KNN : score_global, puis score, sinon moyenne des 5 critères. */
static double chartPerformanceY(const QJsonObject &o)
{
    const int sg = jsonIntVal(o, QStringLiteral("score_global"), -1);
    if (sg >= 0 && sg <= 100)
        return static_cast<double>(sg);
    const int sc = jsonIntVal(o, QStringLiteral("score"), -1);
    if (sc >= 0 && sc <= 100)
        return static_cast<double>(sc);
    const double avg = (jsonIntVal(o, QStringLiteral("experience"), 0) + jsonIntVal(o, QStringLiteral("performance"), 0)
                        + jsonIntVal(o, QStringLiteral("formation"), 0) + jsonIntVal(o, QStringLiteral("discipline"), 0)
                        + jsonIntVal(o, QStringLiteral("productivite"), 0))
                       / 5.0;
    return qBound(0.0, avg, 100.0);
}

static constexpr double kDemoPerfY()
{
    return (1.0 + 66.0 + 38.0 + 56.0 + 67.0) / 5.0;
}

/** Réduit la synthèse en puces courtes (phrases). */
static QString bulletsFromSynthese(const QString &raw, int maxBullets = 4)
{
    const QString t = raw.trimmed();
    if (t.isEmpty())
        return QString();
    const QRegularExpression re(QStringLiteral("[\\.!?]\\s+"));
    const QStringList parts = t.split(re, Qt::SkipEmptyParts);
    QStringList out;
    for (QString p : parts) {
        p = p.trimmed();
        if (p.size() < 6)
            continue;
        if (p.size() > 110)
            p = p.left(107).trimmed() + QStringLiteral("…");
        out << p;
        if (out.size() >= maxBullets)
            break;
    }
    if (out.isEmpty()) {
        const QString one = t.size() > 130 ? t.left(127).trimmed() + QStringLiteral("…") : t;
        return QStringLiteral("• ") + one;
    }
    QString joined;
    for (int i = 0; i < out.size(); ++i) {
        if (i)
            joined += QLatin1Char('\n');
        joined += QStringLiteral("• ") + out[i];
    }
    return joined;
}

/** 1–2 phrases d’action maximum. */
static QString shortActionLines(const QString &raw, int maxLines = 2)
{
    const QString t = raw.trimmed();
    if (t.isEmpty())
        return QString();
    const QRegularExpression re(QStringLiteral("[\\.!?]\\s+"));
    const QStringList parts = t.split(re, Qt::SkipEmptyParts);
    QStringList out;
    for (QString p : parts) {
        p = p.trimmed();
        if (p.size() < 8)
            continue;
        if (p.size() > 100)
            p = p.left(97).trimmed() + QStringLiteral("…");
        out << p;
        if (out.size() >= maxLines)
            break;
    }
    if (out.isEmpty())
        return t.size() > 90 ? t.left(87) + QStringLiteral("…") : t;
    QString joined;
    for (int i = 0; i < out.size(); ++i) {
        if (i)
            joined += QLatin1Char('\n');
        joined += QStringLiteral("› ") + out[i];
    }
    return joined;
}

/** Badges HTML pour voisins KNN (tronqués). */
static QString chipsHtmlFromKnn(const QString &raw, int maxChips = 5)
{
    const QString t = raw.trimmed();
    if (t.isEmpty())
        return QString();
    const QStringList parts = t.split(QRegularExpression(QStringLiteral("[,;·|]")), Qt::SkipEmptyParts);
    QString html;
    int n = 0;
    for (QString p : parts) {
        p = p.trimmed();
        if (p.size() < 2)
            continue;
        if (p.size() > 36)
            p = p.left(33).trimmed() + QStringLiteral("…");
        html += QStringLiteral(
                    "<span style='display:inline-block;background:rgba(166,124,82,0.14);color:#3A3228;"
                    "border:1px solid rgba(139,105,20,0.35);border-radius:999px;padding:3px 10px;margin:3px 5px 0 0;"
                    "font-size:11px;font-weight:600;'>%1</span> ")
                    .arg(p.toHtmlEscaped());
        ++n;
        if (n >= maxChips)
            break;
    }
    return html;
}

/** Synthèse → HTML court (3 puces max). */
static QString synthToHtml(const QString &raw, int maxBullets = 3)
{
    const QString plain = bulletsFromSynthese(raw, maxBullets);
    if (plain.isEmpty())
        return QStringLiteral("<span style='color:#6B5D4A;font-size:12px;'>—</span>");
    const QStringList lines = plain.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QString inner;
    for (const QString &ln : lines) {
        if (!inner.isEmpty())
            inner += QStringLiteral("<br/>");
        inner += ln.trimmed().toHtmlEscaped();
    }
    return QStringLiteral("<span style='color:#4A4036;font-size:12px;line-height:1.5;'>%1</span>").arg(inner);
}

/** Action → HTML court (2 phrases). */
static QString actionToHtml(const QString &raw)
{
    const QString plain = shortActionLines(raw, 2);
    if (plain.isEmpty())
        return QStringLiteral("<span style='color:#6B5D4A;font-size:12px;'>—</span>");
    const QStringList lines = plain.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QString inner;
    for (const QString &ln : lines) {
        if (!inner.isEmpty())
            inner += QStringLiteral("<br/>");
        inner += ln.trimmed().toHtmlEscaped();
    }
    return QStringLiteral("<span style='color:#3A3228;font-size:12px;line-height:1.45;font-weight:600;'>%1</span>")
        .arg(inner);
}

/** Cellule métrique (bandeau 3 colonnes). */
static QString metricCellHtml(const QString &title, const QString &value, bool scoreBig = false)
{
    const QString v = value.trimmed().isEmpty() ? QStringLiteral("—") : value.trimmed();
    const QString vesc = v.toHtmlEscaped();
    if (scoreBig) {
        return QStringLiteral(
                   "<div align='center'>"
                   "<span style='color:#6B5D4A;font-size:9px;font-weight:700;letter-spacing:0.14em;'>%1</span><br/>"
                   "<span style='color:#8B6914;font-size:26px;font-weight:900;line-height:1.05;'>%2</span>"
                   "</div>")
            .arg(title.toHtmlEscaped(), vesc);
    }
    return QStringLiteral(
               "<div align='center'>"
               "<span style='color:#6B5D4A;font-size:9px;font-weight:700;letter-spacing:0.14em;'>%1</span><br/>"
               "<span style='color:#3A3228;font-size:12px;font-weight:700;'>%2</span>"
               "</div>")
        .arg(title.toHtmlEscaped(), vesc);
}

/** CONTEXTE : jamais de dump JSON/HTTP — message humain seul. */
static QString contextErrorHtml(const QString &shortMsg)
{
    return QStringLiteral(
               "<span style='color:#fca5a5;font-size:12px;line-height:1.35;font-weight:600;'>%1</span>")
        .arg(shortMsg.toHtmlEscaped());
}

static QString friendlyHttpHint(int status)
{
    switch (status) {
    case 400:
        return QStringLiteral(
            "Requête Groq invalide (HTTP 400) — vérifiez GROQ_MODEL sur console.groq.com/docs/models "
            "(ex. llama-3.1-8b-instant) et GROQ_MAX_TOKENS.");
    case 401:
        return QStringLiteral("Clé Groq refusée ou expirée — vérifiez GROQ_API_KEY.");
    case 402:
        return QStringLiteral("Quota API insuffisant (Groq).");
    case 403:
        return QStringLiteral("Accès refusé (HTTP 403) — clé Groq ou droits du compte à vérifier sur console.groq.com.");
    case 404:
        return QStringLiteral("Point d’accès ou modèle introuvable — vérifiez l’URL et GROQ_MODEL.");
    case 429:
        return QStringLiteral("Trop de requêtes — réessayez plus tard.");
    case 500:
    case 502:
        return QStringLiteral("Erreur serveur Hugging Face.");
    case 503:
        return QStringLiteral("Modèle en chargement ou service surchargé — réessayez dans une minute.");
    default:
        break;
    }
    if (status >= 400 && status < 500)
        return QStringLiteral("Erreur requête (HTTP %1).").arg(status);
    if (status >= 500)
        return QStringLiteral("Erreur serveur (HTTP %1).").arg(status);
    return QStringLiteral("Analyse indisponible.");
}

static QString truncateForStatus(const QString &s, int maxLen = 140)
{
    QString t = s;
    t.replace(QLatin1Char('\n'), QLatin1Char(' '));
    t = t.simplified();
    if (t.size() > maxLen)
        return t.left(maxLen - 1) + QStringLiteral("…");
    return t;
}

/** Dernière analyse JSON mise en cache localement par ID employé. */
static QJsonObject loadCachedAnalysis(int empId)
{
    QSettings st;
    st.beginGroup(QStringLiteral("ClassificationIA"));
    const QByteArray raw = st.value(QStringLiteral("cache_%1").arg(empId)).toByteArray();
    st.endGroup();
    if (raw.isEmpty())
        return {};
    const QJsonDocument d = QJsonDocument::fromJson(raw);
    return d.isObject() ? d.object() : QJsonObject{};
}

/** Scores stables si aucune analyse n’a été enregistrée pour l’employé. */
static QJsonObject pseudoAnalysisCompare(int empId)
{
    QJsonObject o;
    const quint32 h = qHash(static_cast<quint32>(empId) * 7919u + 13u);
    auto sk = [h](int i) { return int(25 + (qHash(h + static_cast<quint32>(i * 131)) % 74)); };
    const int e = sk(1), p = sk(2), f = sk(3), d = sk(4), pr = sk(5);
    o.insert(QStringLiteral("experience"), e);
    o.insert(QStringLiteral("performance"), p);
    o.insert(QStringLiteral("formation"), f);
    o.insert(QStringLiteral("discipline"), d);
    o.insert(QStringLiteral("productivite"), pr);
    const int avg = (e + p + f + d + pr) / 5;
    o.insert(QStringLiteral("score_global"), avg);
    o.insert(QStringLiteral("score"), avg);
    return o;
}

} // namespace

ClassificationIAWidget::ClassificationIAWidget(QWidget *parent)
    : QWidget(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(20, 16, 20, 16);
    mainLay->setSpacing(0);

    m_statusLabel = new QLabel(QStringLiteral(""));
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(QStringLiteral(
        "QLabel{color:#8B6914;font-size:11px;font-weight:700;background:transparent;padding:2px 8px;}"));
    mainLay->addWidget(m_statusLabel);

    auto *pageScroll = new QScrollArea(this);
    pageScroll->setWidgetResizable(true);
    pageScroll->setFrameShape(QFrame::NoFrame);
    pageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pageScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pageScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pageScroll->setStyleSheet(QStringLiteral(
        "QScrollArea{background:transparent;border:none;}"
        "QScrollBar:vertical{width:10px;background:rgba(180,160,130,0.45);border-radius:5px;margin:2px;}"
        "QScrollBar::handle:vertical{min-height:36px;background:#A67C52;border-radius:5px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"));
    mainLay->addWidget(pageScroll, 1);

    auto *content = new QWidget;
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    pageScroll->setWidget(content);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(8, 8, 8, 24);
    col->setSpacing(16);

    m_badgeTop = new QLabel(QStringLiteral("IA RH · FIL D’OR"));
    m_badgeTop->setAlignment(Qt::AlignCenter);
    m_badgeTop->setStyleSheet(QStringLiteral(
        "QLabel{background:rgba(255,252,247,0.9);border:1px solid rgba(166,124,82,0.45);border-radius:14px;"
        "padding:5px 20px;color:#8B6914;font-size:10px;font-weight:800;letter-spacing:0.22em;}"));
    col->addWidget(m_badgeTop, 0, Qt::AlignHCenter);

    m_titleAtlas = new QLabel(QStringLiteral("Classification IA"));
    m_titleAtlas->setAlignment(Qt::AlignCenter);
    {
        QFont f = m_titleAtlas->font();
        f.setPixelSize(22);
        f.setWeight(QFont::Black);
        m_titleAtlas->setFont(f);
    }
    m_titleAtlas->setStyleSheet(QStringLiteral("QLabel{color:#3A3228;background:transparent;}"));
    col->addWidget(m_titleAtlas);

    m_sepHeader = new QFrame;
    m_sepHeader->setFrameShape(QFrame::HLine);
    m_sepHeader->setFixedHeight(1);
    m_sepHeader->setStyleSheet(QStringLiteral(
        "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 transparent,stop:0.5 rgba(139,105,20,0.35),stop:1 transparent);"
        "border:none;max-height:1px;}"));
    col->addWidget(m_sepHeader);

    m_employeeNameLine = new QLabel;
    m_employeeNameLine->setAlignment(Qt::AlignCenter);
    m_employeeNameLine->setTextFormat(Qt::RichText);
    m_employeeNameLine->setStyleSheet(QStringLiteral("QLabel{background:transparent;}"));
    col->addWidget(m_employeeNameLine);

    auto *empBadges = new QHBoxLayout;
    empBadges->setSpacing(10);
    empBadges->addStretch();
    m_deptBadge = new QLabel(QStringLiteral("—"));
    m_deptBadge->setAlignment(Qt::AlignCenter);
    m_deptBadge->setStyleSheet(QStringLiteral(
        "QLabel{background:rgba(255,252,247,0.95);color:#6B5D4A;border:1px solid rgba(166,124,82,0.5);"
        "border-radius:10px;padding:6px 14px;font-size:12px;font-weight:700;}"));
    m_quickScoreBadge = new QLabel(QStringLiteral("—"));
    m_quickScoreBadge->setAlignment(Qt::AlignCenter);
    m_quickScoreBadge->setStyleSheet(QStringLiteral(
        "QLabel{background:rgba(255,248,235,0.98);color:#8B6914;border:1px solid rgba(166,124,82,0.45);"
        "border-radius:10px;padding:6px 14px;font-size:13px;font-weight:800;}"));
    empBadges->addWidget(m_deptBadge);
    empBadges->addWidget(m_quickScoreBadge);
    empBadges->addStretch();
    col->addLayout(empBadges);

    auto *stats = new QHBoxLayout;
    stats->setSpacing(8);
    auto mkStatBadge = [](const QString &k, const QString &v, bool scoreBig) {
        auto *lb = new QLabel;
        lb->setAlignment(Qt::AlignCenter);
        lb->setMinimumWidth(116);
        lb->setStyleSheet(QStringLiteral(
            "QLabel{background:rgba(255,252,247,0.92);border:1px solid rgba(200,180,150,0.55);border-radius:12px;"
            "padding:12px 16px;}"));
        setStatBadge(lb, k, v, scoreBig);
        return lb;
    };
    m_badgeClasse = mkStatBadge(QStringLiteral("CLASSE"), QStringLiteral("—"), false);
    m_badgeScore = mkStatBadge(QStringLiteral("SCORE"), QStringLiteral("0"), true);
    m_badgeNiveau = mkStatBadge(QStringLiteral("NIVEAU"), QStringLiteral("—"), false);
    stats->addStretch();
    stats->addWidget(m_badgeClasse);
    stats->addWidget(m_badgeScore);
    stats->addWidget(m_badgeNiveau);
    stats->addStretch();
    col->addLayout(stats);

    m_knnGraph = new KnnGraphWidget(content);
    m_knnGraph->setMinimumSize(500, 400);
    m_knnGraph->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    col->addWidget(m_knnGraph, 1);

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    btnRow->addStretch();
    m_badgeClassLetter = new QLabel(QStringLiteral("C"));
    m_badgeClassLetter->setFixedSize(38, 38);
    m_badgeClassLetter->setAlignment(Qt::AlignCenter);
    m_badgeClassLetter->setStyleSheet(QStringLiteral(
        "QLabel{background:rgba(255,252,247,0.98);color:#3A3228;font-weight:900;border-radius:19px;font-size:15px;"
        "border:1px solid rgba(166,124,82,0.55);}"));
    m_btnFullAnalysis = new QPushButton(QStringLiteral("Analyser"));
    m_btnFullAnalysis->setStyleSheet(QStringLiteral(
        "QPushButton{background:#A67C52;color:#1A0F05;font-weight:800;font-size:13px;border:none;"
        "border-radius:10px;padding:10px 22px;border:1px solid #C8972A;}"));
    m_btnDetails = new QPushButton(QStringLiteral("Détails"));
    m_btnCompare = new QPushButton(QStringLiteral("Comparer"));
    const QString darkOutlineBtn = QStringLiteral(
        "QPushButton{background:rgba(255,252,247,0.95);border:1px solid rgba(166,124,82,0.65);color:#3A3228;"
        "border-radius:10px;padding:10px 18px;font-weight:700;}");
    m_btnDetails->setStyleSheet(darkOutlineBtn);
    m_btnCompare->setStyleSheet(darkOutlineBtn);
    btnRow->addWidget(m_badgeClassLetter);
    btnRow->addWidget(m_btnFullAnalysis);
    btnRow->addWidget(m_btnDetails);
    btnRow->addWidget(m_btnCompare);
    btnRow->addStretch();
    col->addLayout(btnRow);

    m_resumeInner = new QWidget(content);
    m_resumeInner->setStyleSheet(QStringLiteral(
        "QWidget#resumeDecisionnel{"
        "background:rgba(255,252,247,0.92);border:1px solid rgba(200,180,150,0.55);border-radius:14px;}"));
    m_resumeInner->setObjectName(QStringLiteral("resumeDecisionnel"));
    auto *rv = new QVBoxLayout(m_resumeInner);
    rv->setContentsMargins(14, 14, 14, 14);
    rv->setSpacing(12);

    m_resumeTitle = new QLabel(QStringLiteral("Décision IA"));
    m_resumeTitle->setStyleSheet(QStringLiteral(
        "QLabel{color:#8B6914;font-size:11px;font-weight:800;letter-spacing:0.2em;background:transparent;}"));
    rv->addWidget(m_resumeTitle);

    m_lblScoreGlobalRow = new QLabel;
    m_lblNiveauRow = new QLabel;
    m_lblClasseRow = new QLabel;
    m_lblActionRow = new QLabel;
    const QString metricHtmlStyle = QStringLiteral(
        "QLabel{background:rgba(248,242,232,0.98);border:1px solid rgba(200,185,155,0.55);border-radius:10px;"
        "padding:10px 8px;font-size:13px;}");
    m_lblScoreGlobalRow->setStyleSheet(metricHtmlStyle);
    m_lblNiveauRow->setStyleSheet(metricHtmlStyle);
    m_lblClasseRow->setStyleSheet(metricHtmlStyle);
    m_lblActionRow->setStyleSheet(QStringLiteral(
        "QLabel{background:transparent;font-size:12px;padding:2px 0;}"));
    m_lblScoreGlobalRow->setTextFormat(Qt::RichText);
    m_lblNiveauRow->setTextFormat(Qt::RichText);
    m_lblClasseRow->setTextFormat(Qt::RichText);
    m_lblActionRow->setTextFormat(Qt::RichText);
    auto *metricsStrip = new QHBoxLayout;
    metricsStrip->setSpacing(8);
    metricsStrip->addWidget(m_lblScoreGlobalRow, 1);
    metricsStrip->addWidget(m_lblNiveauRow, 1);
    metricsStrip->addWidget(m_lblClasseRow, 1);
    rv->addLayout(metricsStrip);

    auto *actionTitle = new QLabel(QStringLiteral("PRIORITÉ"));
    actionTitle->setStyleSheet(QStringLiteral(
        "QLabel{color:#6B5D4A;font-size:10px;font-weight:800;letter-spacing:0.18em;background:transparent;}"));
    rv->addWidget(actionTitle);
    m_lblActionRow->setWordWrap(true);
    rv->addWidget(m_lblActionRow);

    m_resumeSep = new QFrame;
    m_resumeSep->setFrameShape(QFrame::HLine);
    m_resumeSep->setFixedHeight(1);
    m_resumeSep->setStyleSheet(QStringLiteral(
        "QFrame{background:rgba(200,151,42,0.25);border:none;max-height:1px;}"));
    rv->addWidget(m_resumeSep);

    m_lblPreuveTitle = new QLabel(QStringLiteral("CONTEXTE"));
    m_lblPreuveTitle->setStyleSheet(QStringLiteral(
        "QLabel{color:#6B5D4A;font-size:10px;font-weight:800;letter-spacing:0.18em;background:transparent;}"));
    m_lblPreuveBody = new QLabel;
    m_lblPreuveBody->setWordWrap(true);
    m_lblPreuveBody->setTextFormat(Qt::RichText);
    m_lblPreuveBody->setMaximumHeight(120);
    m_lblPreuveBody->setStyleSheet(QStringLiteral("QLabel{background:transparent;font-size:12px;}"));
    rv->addWidget(m_lblPreuveTitle);
    rv->addWidget(m_lblPreuveBody);

    m_lblVoisinsTitle = new QLabel(QStringLiteral("SIMILAIRES"));
    m_lblVoisinsTitle->setStyleSheet(QStringLiteral(
        "QLabel{color:#6B5D4A;font-size:10px;font-weight:800;letter-spacing:0.18em;background:transparent;}"));
    m_lblVoisinsBody = new QLabel;
    m_lblVoisinsBody->setWordWrap(true);
    m_lblVoisinsBody->setTextFormat(Qt::RichText);
    m_lblVoisinsBody->setStyleSheet(QStringLiteral("QLabel{background:transparent;padding:2px 0;}"));
    rv->addWidget(m_lblVoisinsTitle);
    rv->addWidget(m_lblVoisinsBody);

    col->addWidget(m_resumeInner);

    connect(m_btnFullAnalysis, &QPushButton::clicked, this, &ClassificationIAWidget::requestFullAnalysis);
    connect(m_btnDetails, &QPushButton::clicked, this, [this]() { showDetailsDialog(); });
    connect(m_btnCompare, &QPushButton::clicked, this, &ClassificationIAWidget::showCompareDialog);

    resetResumePlaceholder();
}

void ClassificationIAWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient grad(0, 0, width(), height());
    grad.setColorAt(0.0, QColor(QStringLiteral("#EBE2D4")));
    grad.setColorAt(0.5, QColor(QStringLiteral("#DED3C2")));
    grad.setColorAt(1.0, QColor(QStringLiteral("#D2C4B0")));
    p.fillRect(rect(), grad);
}

void ClassificationIAWidget::setEmployeeId(int employeeId)
{
    m_employeeId = employeeId;
    loadEmployeeData(employeeId);
}

bool ClassificationIAWidget::loadEmployeeData(int id)
{
    if (id <= 0)
        return false;
    QSqlQuery q;
    q.prepare(
        QStringLiteral("SELECT NOM, PRENOM, POSTE, DEPARTEMENT, SALAIRE, DATE_EMBAUCHE FROM EMPLOYES WHERE ID_EMPLOYE = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!q.exec() || !q.next())
        return false;

    m_emp.id = id;
    m_emp.nom = q.value(0).toString();
    m_emp.prenom = q.value(1).toString();
    m_emp.poste = q.value(2).toString();
    m_emp.departement = q.value(3).toString();
    m_emp.salaire = q.value(4).toDouble();
    m_emp.dateEmbauche = q.value(5).toDate();
    m_emp.anciennete = qMax(0, m_emp.dateEmbauche.daysTo(QDate::currentDate()) / 365);

    const QString fullName = (m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed();
    m_employeeNameLine->setText(QStringLiteral(
                                      "<span style='font-size:16px;font-weight:700;color:#3A3228;'>%1</span>"
                                      "<span style='font-size:13px;color:#6B5D4A;'> &nbsp;· #%2</span>")
                                      .arg(fullName.toHtmlEscaped(), QString::number(m_emp.id)));

    m_deptBadge->setText(QStringLiteral("%1 | %2").arg(m_emp.departement, m_emp.poste));
    m_quickScoreBadge->setText(QStringLiteral("—"));

    setStatBadge(m_badgeClasse, QStringLiteral("CLASSE"), QStringLiteral("✦ Profil a risque"), false);
    setStatBadge(m_badgeScore, QStringLiteral("SCORE"), QStringLiteral("75"), true);
    setStatBadge(m_badgeNiveau, QStringLiteral("NIVEAU"), QStringLiteral("C · Critique"), false);
    m_badgeClassLetter->setText(QStringLiteral("C"));

    if (m_knnGraph) {
        m_knnGraph->setEmploye(fullName, static_cast<double>(m_emp.anciennete), kDemoPerfY());
        m_knnGraph->setClasseIa(QString());
    }

    resetResumePlaceholder();
    return true;
}

void ClassificationIAWidget::resetResumePlaceholder()
{
    m_lblScoreGlobalRow->setText(metricCellHtml(QStringLiteral("SCORE"), QStringLiteral("—"), true));
    m_lblNiveauRow->setText(metricCellHtml(QStringLiteral("NIVEAU"), QStringLiteral("—"), false));
    m_lblClasseRow->setText(metricCellHtml(QStringLiteral("CLASSE"), QStringLiteral("—"), false));
    m_lblActionRow->setText(actionToHtml(QString()));
    m_lblPreuveBody->setText(synthToHtml(QString(), 3));
    m_lblVoisinsBody->setText(QStringLiteral("<span style='color:#6B5D4A;font-size:12px;'>—</span>"));
    if (m_statusLabel)
        m_statusLabel->clear();
}

void ClassificationIAWidget::setAnalysisErrorUi(const QString &contextShortFr, const QString &technicalDetail)
{
    m_lblPreuveBody->setText(contextErrorHtml(contextShortFr));
    if (!m_statusLabel)
        return;
    if (technicalDetail.isEmpty())
        m_statusLabel->clear();
    else
        m_statusLabel->setText(truncateForStatus(technicalDetail));
}

QString ClassificationIAWidget::buildUserMessage() const
{
    return QStringLiteral("Employé : %1 %2, Poste : %3, Département : %4, Salaire : %5 DT, Ancienneté : %6 ans, Date embauche : %7.")
        .arg(m_emp.nom, m_emp.prenom, m_emp.poste, m_emp.departement)
        .arg(QString::number(m_emp.salaire, 'f', 2))
        .arg(m_emp.anciennete)
        .arg(m_emp.dateEmbauche.toString(QStringLiteral("yyyy-MM-dd")));
}

void ClassificationIAWidget::requestFullAnalysis()
{
    sendGroqClassification();
}

void ClassificationIAWidget::sendGroqClassification()
{
    const QString apiKey = pickGroqApiKey();
    if (apiKey.isEmpty()) {
        updateResumeSection(QJsonObject());
        setAnalysisErrorUi(QStringLiteral("Clé Groq manquante."),
                           QStringLiteral("Variable d’environnement GROQ_API_KEY ou fichier groq_api_key.txt à côté de l’exécutable "
                                          "(clé commençant par gsk_)."));
        return;
    }

    if (!QSslSocket::supportsSsl()) {
        setAnalysisErrorUi(QStringLiteral("HTTPS indisponible sur cette installation."),
                           QStringLiteral("Copiez le dossier tls / plugins Qt à côté de ProjetCpp.exe."));
        return;
    }

    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    setLoading(true);

    const QString sys =
        QStringLiteral("Tu es un système IA de classification RH pour FIL D'OR. Retourne UNIQUEMENT un JSON valide sans markdown ni "
                       "backticks avec cette structure : "
                       "{\"classe\":string,\"score\":int,\"niveau\":string,\"experience\":int,\"performance\":int,\"formation\":int,"
                       "\"discipline\":int,\"productivite\":int,\"score_global\":int,\"synthese\":string,\"action\":string,\"voisins_knn\":string}. "
                       "Classes possibles : Profil junior, Profil confirmé, Profil expert, Profil a risque, Profil senior. "
                       "Niveaux : D·Débutant, C·Critique, B·Confirmé, A·Expert, S·Senior.");

    const QString userContent = QStringLiteral("Données employé :\n") + buildUserMessage()
        + QStringLiteral("\n\nRéponds uniquement avec le JSON demandé, sans texte avant ni après.");

    QJsonArray messages;
    QJsonObject msgSystem;
    msgSystem.insert(QStringLiteral("role"), QStringLiteral("system"));
    msgSystem.insert(QStringLiteral("content"), sys);
    QJsonObject msgUser;
    msgUser.insert(QStringLiteral("role"), QStringLiteral("user"));
    msgUser.insert(QStringLiteral("content"), userContent);
    messages.append(msgSystem);
    messages.append(msgUser);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), groqModelId());
    payload.insert(QStringLiteral("messages"), messages);
    payload.insert(QStringLiteral("temperature"), 0.2);
    payload.insert(QStringLiteral("max_tokens"), groqMaxTokens());

    QNetworkRequest req(groqChatCompletionsUrl());
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", "FIL-DOR-Qt/1.0 (Classification-IA-Groq)");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, QVariant(false));

    m_activeReply = m_nam->post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));

#ifndef QT_NO_SSL
    connect(m_activeReply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError> &errs) {
        QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
        if (!r)
            return;
        if (qEnvironmentVariableIsSet("FIL_DOR_TLS_INSECURE")) {
            r->ignoreSslErrors();
            return;
        }
        QStringList parts;
        for (const QSslError &e : errs)
            parts << e.errorString();
        setAnalysisErrorUi(QStringLiteral("Certificat SSL rejeté."),
                           QStringLiteral("%1 — FIL_DOR_TLS_INSECURE=1 pour test.").arg(parts.join(QLatin1String(" | "))));
    });
#endif

    auto *timeoutTimer = new QTimer(m_activeReply);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(120000);
    connect(timeoutTimer, &QTimer::timeout, m_activeReply, [this]() {
        if (m_activeReply)
            m_activeReply->abort();
    });
    timeoutTimer->start();

    connect(m_activeReply, &QNetworkReply::finished, this, [this]() {
        QNetworkReply *reply = m_activeReply;
        m_activeReply = nullptr;
        setLoading(false);
        handleLlmApiReply(reply);
        reply->deleteLater();
    });
}

QJsonObject ClassificationIAWidget::parseResponseJson(QString content) const
{
    if (!content.isEmpty() && content.at(0).unicode() == 0xFEFF)
        content.remove(0, 1);
    content = content.trimmed();
    if (content.startsWith(QLatin1String("```"))) {
        content.remove(0, content.indexOf(QLatin1Char('\n')) + 1);
        const int cut = content.lastIndexOf(QLatin1String("```"));
        if (cut > 0)
            content = content.left(cut).trimmed();
    }
    if (content.startsWith(QLatin1String("```json"), Qt::CaseInsensitive)) {
        content.remove(0, 7);
        const int cut = content.lastIndexOf(QLatin1String("```"));
        if (cut > 0)
            content = content.left(cut).trimmed();
    }
    const int b0 = content.indexOf(QLatin1Char('{'));
    const int b1 = content.lastIndexOf(QLatin1Char('}'));
    if (b0 >= 0 && b1 > b0)
        content = content.mid(b0, b1 - b0 + 1);
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject())
        return {};
    return doc.object();
}

void ClassificationIAWidget::handleLlmApiReply(QNetworkReply *reply)
{
    if (!reply) {
        setAnalysisErrorUi(QStringLiteral("Réponse réseau invalide."), QString());
        return;
    }

    const QByteArray raw = reply->readAll();
    const QNetworkReply::NetworkError nerr = reply->error();
    const QVariant stAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    const int httpStatus = stAttr.isValid() ? stAttr.toInt() : 0;
    const QString httpReason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    if (nerr != QNetworkReply::NoError) {
        QString userShort = QStringLiteral("Problème réseau ou délai — vérifiez Internet et la clé GROQ_API_KEY.");
        if (nerr == QNetworkReply::OperationCanceledError)
            userShort = QStringLiteral("Requête annulée ou délai dépassé (120 s).");
        // Qt peut signaler une « erreur transfert » alors que le serveur renvoie un HTTP explicite (ex. 403).
        if (httpStatus >= 400)
            userShort = friendlyHttpHint(httpStatus);
        QString tech = reply->errorString();
        if (httpStatus > 0)
            tech += QStringLiteral(" · HTTP %1 %2").arg(httpStatus).arg(httpReason);
        if (!raw.isEmpty())
            tech += QStringLiteral(" · ") + QString::fromUtf8(raw.left(400));
        if (httpStatus == 402)
            userShort = friendlyHttpHint(402);
        else if (httpStatus == 503)
            userShort = friendlyHttpHint(503);
        setAnalysisErrorUi(userShort, tech);
        return;
    }

    if (httpStatus >= 400) {
        QString userShort = friendlyHttpHint(httpStatus);
        QString tech =
            QStringLiteral("HTTP %1 %2").arg(httpStatus).arg(httpReason.isEmpty() ? QStringLiteral("—") : httpReason);
        if (!raw.isEmpty())
            tech += QStringLiteral(" · ") + QString::fromUtf8(raw.left(400));

        const QJsonDocument errDoc = QJsonDocument::fromJson(raw);
        if (errDoc.isObject()) {
            const QJsonObject oroot = errDoc.object();
            if (oroot.contains(QStringLiteral("error"))) {
                const QString apiErr = formatApiJsonError(oroot.value(QStringLiteral("error")));
                if (!apiErr.isEmpty())
                    tech = apiErr.left(400);
            }
        }
        setAnalysisErrorUi(userShort, tech);
        return;
    }

    QJsonParseError pe;
    const QJsonDocument rootDoc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || rootDoc.isNull()) {
        setAnalysisErrorUi(QStringLiteral("Réponse JSON invalide."), QString::fromUtf8(raw.left(300)));
        return;
    }

    if (rootDoc.isObject()) {
        const QJsonObject root = rootDoc.object();
        if (root.contains(QStringLiteral("error"))) {
            const QString apiErr = formatApiJsonError(root.value(QStringLiteral("error")));
            const QString tech = apiErr.isEmpty() ? QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact)).left(400)
                                                  : apiErr;
            setAnalysisErrorUi(QStringLiteral("L’API Groq a renvoyé une erreur."), tech);
            return;
        }
    }

    QString content = extractChatCompletionContent(raw).trimmed();

    if (content.isEmpty() && rootDoc.isObject()) {
        const QJsonObject root = rootDoc.object();
        const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
        if (!choices.isEmpty()) {
            const QJsonObject msg = choices.at(0).toObject().value(QStringLiteral("message")).toObject();
            const QJsonValue cv = msg.value(QStringLiteral("content"));
            if (cv.isString())
                content = cv.toString().trimmed();
        }
    }

    const QJsonObject parsed = parseResponseJson(content);
    if (parsed.isEmpty()) {
        setAnalysisErrorUi(QStringLiteral("Réponse IA illisible (pas de JSON valide)."),
                           truncateForStatus(content.isEmpty() ? QStringLiteral("(vide)") : content, 200));
        return;
    }

    updateUI(parsed);
}

void ClassificationIAWidget::updateUI(const QJsonObject &o)
{
    m_lastResult = o;
    if (m_employeeId > 0) {
        QSettings st;
        st.beginGroup(QStringLiteral("ClassificationIA"));
        st.setValue(QStringLiteral("cache_%1").arg(m_employeeId), QJsonDocument(o).toJson(QJsonDocument::Compact));
        st.endGroup();
    }
    if (m_statusLabel)
        m_statusLabel->clear();

    const QString classe = o.value(QStringLiteral("classe")).toString();
    const int score =
        jsonIntVal(o, QStringLiteral("score"), jsonIntVal(o, QStringLiteral("score_global"), 0));
    const QString niveau = o.value(QStringLiteral("niveau")).toString();

    m_quickScoreBadge->setText(QString::number(score));

    setStatBadge(m_badgeClasse, QStringLiteral("CLASSE"), classe.isEmpty() ? QStringLiteral("—") : classe, false);
    setStatBadge(m_badgeScore, QStringLiteral("SCORE"), QString::number(score), true);
    setStatBadge(m_badgeNiveau, QStringLiteral("NIVEAU"), niveau.isEmpty() ? QStringLiteral("—") : niveau, false);

    m_badgeClassLetter->setText(niveauLetter(niveau));

    if (m_knnGraph) {
        const QString nom =
            ((m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed().isEmpty()
                 ? QStringLiteral("Employé")
                 : (m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed());
        m_knnGraph->setEmploye(nom, static_cast<double>(m_emp.anciennete), chartPerformanceY(o));
        const QString letter = niveauLetter(niveau);
        m_knnGraph->setClasseIa(letter == QStringLiteral("—") ? QString() : letter);
    }

    updateResumeSection(o);
}

void ClassificationIAWidget::updateResumeSection(const QJsonObject &o)
{
    if (o.isEmpty()) {
        resetResumePlaceholder();
        return;
    }

    const int sg = jsonIntVal(o, QStringLiteral("score_global"), jsonIntVal(o, QStringLiteral("score"), 0));
    const QString niveau = o.value(QStringLiteral("niveau")).toString().trimmed();
    const QString classe = o.value(QStringLiteral("classe")).toString().trimmed();

    m_lblScoreGlobalRow->setText(metricCellHtml(QStringLiteral("SCORE"), QString::number(sg), true));
    m_lblNiveauRow->setText(metricCellHtml(QStringLiteral("NIVEAU"), niveau.isEmpty() ? QStringLiteral("—") : niveau, false));
    m_lblClasseRow->setText(metricCellHtml(QStringLiteral("CLASSE"), classe.isEmpty() ? QStringLiteral("—") : classe, false));

    m_lblActionRow->setText(actionToHtml(o.value(QStringLiteral("action")).toString()));

    m_lblPreuveBody->setText(synthToHtml(o.value(QStringLiteral("synthese")).toString(), 3));

    const QString chipHtml = chipsHtmlFromKnn(o.value(QStringLiteral("voisins_knn")).toString());
    m_lblVoisinsBody->setText(chipHtml.isEmpty()
                                  ? QStringLiteral("<span style='color:#6B5D4A;font-size:12px;'>—</span>")
                                  : chipHtml);
}

void ClassificationIAWidget::setLoading(bool loading)
{
    m_loading = loading;
    m_btnFullAnalysis->setEnabled(!loading);
    m_btnDetails->setEnabled(!loading);
    m_btnCompare->setEnabled(!loading);
    m_statusLabel->setText(loading ? QStringLiteral("Analyse en cours...") : QStringLiteral(""));
}

void ClassificationIAWidget::showDetailsDialog()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("Détails — Classification IA"));
    dlg->resize(640, 580);
    dlg->setMinimumWidth(520);
    dlg->setStyleSheet(QStringLiteral(
        "QDialog{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #EDE4D4,stop:0.5 #E2D6C4,stop:1 #D8CAB4);"
        "}"
        "QPushButton{"
        "background:#A67C52;color:#FEFDFB;border:1px solid #8B6914;border-radius:10px;"
        "padding:9px 22px;font-weight:700;"
        "}"
        "QPushButton:hover{background:#8B663F;}"
        "QFrame#neoCard{"
        "background:rgba(255,252,247,0.98);border:1px solid rgba(180,165,135,0.65);border-radius:14px;"
        "}"
        "QScrollArea{border:none;background:transparent;}"
        "QScrollBar:vertical{width:9px;background:rgba(200,185,160,0.5);border-radius:5px;margin:2px;}"
        "QScrollBar::handle:vertical{min-height:26px;background:#A67C52;border-radius:5px;}"));

    auto *rootLay = new QVBoxLayout(dlg);
    rootLay->setContentsMargins(16, 14, 16, 10);
    rootLay->setSpacing(8);

    auto *scroll = new QScrollArea(dlg);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *content = new QWidget;
    auto *cl = new QVBoxLayout(content);
    cl->setContentsMargins(4, 4, 8, 10);
    cl->setSpacing(12);

    auto sectionTitle = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet(QStringLiteral(
            "QLabel{color:#8B6914;font-size:10px;font-weight:800;letter-spacing:0.2em;background:transparent;}"));
        return l;
    };

    auto makeCard = [](const QString &keyLabel, const QString &value) {
        auto *f = new QFrame;
        f->setObjectName(QStringLiteral("neoCard"));
        auto *vl = new QVBoxLayout(f);
        vl->setContentsMargins(12, 10, 12, 10);
        vl->setSpacing(2);
        auto *kl = new QLabel(keyLabel);
        kl->setStyleSheet(QStringLiteral(
            "QLabel{color:#6B5D4A;font-size:9px;font-weight:700;letter-spacing:0.06em;background:transparent;}"));
        auto *val = new QLabel(value.isEmpty() ? QStringLiteral("—") : value);
        val->setWordWrap(true);
        val->setStyleSheet(QStringLiteral(
            "QLabel{color:#3A3228;font-size:14px;font-weight:800;background:transparent;}"));
        vl->addWidget(kl);
        vl->addWidget(val);
        return f;
    };

    auto glassBox = [] {
        auto *w = new QWidget;
        w->setStyleSheet(QStringLiteral(
            "QWidget{background:rgba(248,242,232,0.95);border:1px solid rgba(200,185,155,0.55);"
            "border-radius:12px;padding:2px;}"));
        return w;
    };

    if (m_lastResult.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("Aucune analyse.\nLancez « Analyse IA complète »."));
        empty->setAlignment(Qt::AlignCenter);
        empty->setWordWrap(true);
        empty->setStyleSheet(QStringLiteral(
            "QLabel{color:#6B5D4A;font-size:13px;padding:36px;background:transparent;}"));
        cl->addWidget(empty);
    } else {
        const QJsonObject &o = m_lastResult;

        const QString classeStr = o.value(QStringLiteral("classe")).toString().trimmed();
        const QString niveauStr = o.value(QStringLiteral("niveau")).toString().trimmed();
        const int scoreVal =
            jsonIntVal(o, QStringLiteral("score_global"), jsonIntVal(o, QStringLiteral("score"), 0));

        auto *cards = new QHBoxLayout;
        cards->setSpacing(8);
        cards->addWidget(makeCard(QStringLiteral("CLASSE"), classeStr));
        cards->addWidget(makeCard(QStringLiteral("NIVEAU"), niveauStr));
        cards->addWidget(makeCard(QStringLiteral("SCORE"), QString::number(scoreVal)));
        cl->addLayout(cards);

        const QString nomEmp =
            ((m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed().isEmpty()
                 ? QStringLiteral("Employé")
                 : (m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed());
        auto *graph = new KnnGraphWidget(content);
        graph->setMinimumSize(480, 260);
        graph->setMaximumHeight(320);
        graph->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        graph->setEmploye(nomEmp, static_cast<double>(m_emp.anciennete), chartPerformanceY(o));
        const QString nvLet = niveauLetter(niveauStr);
        graph->setClasseIa(nvLet == QStringLiteral("—") ? QString() : nvLet);
        cl->addWidget(graph);

        const int e = jsonIntVal(o, QStringLiteral("experience"), 0);
        const int pe = jsonIntVal(o, QStringLiteral("performance"), 0);
        const int fo = jsonIntVal(o, QStringLiteral("formation"), 0);
        const int di = jsonIntVal(o, QStringLiteral("discipline"), 0);
        const int pr = jsonIntVal(o, QStringLiteral("productivite"), 0);
        auto *metrics = new QLabel(QStringLiteral("E %1% · P %2% · F %3% · D %4% · Pr %5%")
                                       .arg(e)
                                       .arg(pe)
                                       .arg(fo)
                                       .arg(di)
                                       .arg(pr));
        metrics->setStyleSheet(QStringLiteral(
            "QLabel{color:#8B6914;font-size:11px;font-weight:600;letter-spacing:0.04em;"
            "background:rgba(248,242,232,0.98);border:1px solid rgba(200,185,155,0.45);border-radius:8px;padding:8px 12px;}"));
        metrics->setAlignment(Qt::AlignCenter);
        cl->addWidget(metrics);

        cl->addWidget(sectionTitle(QStringLiteral("POINTS CLÉS")));
        auto *synWrap = glassBox();
        auto *synLay = new QVBoxLayout(synWrap);
        synLay->setContentsMargins(12, 10, 12, 10);
        auto *synLbl = new QLabel(bulletsFromSynthese(o.value(QStringLiteral("synthese")).toString()));
        synLbl->setWordWrap(true);
        synLbl->setStyleSheet(QStringLiteral(
            "QLabel{color:#4A4036;font-size:12px;line-height:1.45;background:transparent;}"));
        synLbl->setMaximumHeight(130);
        if (synLbl->text().isEmpty())
            synLbl->setText(QStringLiteral("—"));
        synLay->addWidget(synLbl);
        cl->addWidget(synWrap);

        cl->addWidget(sectionTitle(QStringLiteral("PRIORITÉ")));
        auto *actWrap = glassBox();
        auto *actLay = new QVBoxLayout(actWrap);
        actLay->setContentsMargins(12, 10, 12, 10);
        auto *actLbl = new QLabel(shortActionLines(o.value(QStringLiteral("action")).toString()));
        actLbl->setWordWrap(true);
        actLbl->setStyleSheet(QStringLiteral(
            "QLabel{color:#3A3228;font-size:12px;line-height:1.4;background:transparent;}"));
        actLbl->setMaximumHeight(88);
        if (actLbl->text().isEmpty())
            actLbl->setText(QStringLiteral("—"));
        actLay->addWidget(actLbl);
        cl->addWidget(actWrap);

        cl->addWidget(sectionTitle(QStringLiteral("SIMILAIRES")));
        const QString chipHtml = chipsHtmlFromKnn(o.value(QStringLiteral("voisins_knn")).toString());
        auto *knnLbl = new QLabel;
        knnLbl->setWordWrap(true);
        knnLbl->setTextFormat(Qt::RichText);
        knnLbl->setText(chipHtml.isEmpty() ? QStringLiteral("<span style='color:#6B5D4A;font-size:12px;'>—</span>")
                                           : chipHtml);
        knnLbl->setStyleSheet(QStringLiteral("QLabel{background:transparent;padding:4px 2px;}"));
        cl->addWidget(knnLbl);
    }

    scroll->setWidget(content);
    rootLay->addWidget(scroll, 1);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    rootLay->addWidget(bb);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}

void ClassificationIAWidget::showCompareDialog()
{
    if (m_employeeId <= 0) {
        QMessageBox::information(this, QStringLiteral("Comparer"),
                                 QStringLiteral("Choisissez un employé dans la liste RH."));
        return;
    }

    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("Comparer"));
    dlg->resize(780, 520);
    dlg->setMinimumWidth(560);
    dlg->setStyleSheet(QStringLiteral(
        "QDialog{background:#EAE0D2;}"
        "QLabel{color:#3A3228;background:transparent;}"
        "QComboBox{background:#FFFDF9;color:#2D2618;border:1px solid #C9B99A;border-radius:8px;padding:8px 12px;min-width:240px;}"
        "QComboBox QAbstractItemView{"
        "  border:1px solid #A89272;background:#FFFDF9;color:#2D2618;"
        "  selection-background-color:#B8954A;selection-color:#1A1208;outline:0;}"
        "QComboBox QAbstractItemView::item{min-height:26px;padding:6px 12px;color:#2D2618;background:#FFFDF9;}"
        "QComboBox QAbstractItemView::item:hover{background:#EDE5D8;color:#1A1208;}"
        "QComboBox QAbstractItemView::item:selected{background:#B8954A;color:#1A1208;}"
        "QComboBox QListView{background:#FFFDF9;color:#2D2618;outline:0;}"
        "QPushButton{background:#A67C52;color:#FEFDFB;border:1px solid #8B6914;border-radius:8px;padding:8px 20px;font-weight:700;}"));

    auto *vl = new QVBoxLayout(dlg);
    vl->setContentsMargins(18, 16, 18, 12);
    vl->setSpacing(12);

    auto *hdr = new QLabel(QStringLiteral("Comparaison décisionnelle"));
    hdr->setStyleSheet(QStringLiteral("font-size:18px;font-weight:800;color:#8B6914;background:transparent;"));
    vl->addWidget(hdr);

    auto *pickRow = new QHBoxLayout;
    pickRow->addWidget(new QLabel(QStringLiteral("Comparer avec :")));
    auto *cb = new QComboBox(dlg);
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYES WHERE ID_EMPLOYE <> :id ORDER BY NOM, PRENOM"));
    q.bindValue(QStringLiteral(":id"), m_employeeId);
    if (q.exec()) {
        while (q.next()) {
            const int oid = q.value(0).toInt();
            const QString nom = q.value(1).toString().trimmed();
            const QString prenom = q.value(2).toString().trimmed();
            cb->addItem(QStringLiteral("%1 %2 (ID %3)").arg(nom, prenom, QString::number(oid)), oid);
        }
    }
    pickRow->addWidget(cb, 1);
    vl->addLayout(pickRow);

    if (QAbstractItemView *listView = cb->view()) {
        listView->setStyleSheet(QStringLiteral(
            "QAbstractItemView { background: #FFFDF9; color: #2D2618; border: 1px solid #A89272; outline: 0; }"
            "QAbstractItemView::item { min-height: 28px; padding: 4px 10px; color: #2D2618; background: #FFFDF9; }"
            "QAbstractItemView::item:selected { background: #B8954A; color: #1A1208; }"
            "QAbstractItemView::item:hover { background: #EDE5D8; color: #1A1208; }"));
    }

    if (cb->count() == 0) {
        QMessageBox::information(this, QStringLiteral("Comparer"),
                                 QStringLiteral("Aucun autre employé dans la base pour comparer."));
        dlg->deleteLater();
        return;
    }

    auto *sumLbl = new QLabel(dlg);
    sumLbl->setStyleSheet(QStringLiteral("color:#5C4D3A;font-size:14px;font-weight:700;background:transparent;"));
    sumLbl->setWordWrap(true);
    vl->addWidget(sumLbl);

    auto *chartHost = new QWidget(dlg);
    auto *chartLay = new QVBoxLayout(chartHost);
    chartLay->setContentsMargins(0, 4, 0, 0);
    vl->addWidget(chartHost, 1);

    auto *legHint = new QLabel(QStringLiteral(
        "Scores : dernière analyse enregistrée par employé (cache local). Sinon estimation stable pour la démo."));
    legHint->setWordWrap(true);
    legHint->setStyleSheet(QStringLiteral("color:#6B5D4A;font-size:10px;background:transparent;"));
    vl->addWidget(legHint);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    vl->addWidget(bb);

    const auto rebuild = [this, cb, chartLay, sumLbl]() {
        QLayoutItem *lit = nullptr;
        while ((lit = chartLay->takeAt(0)) != nullptr) {
            if (QWidget *w = lit->widget())
                w->deleteLater();
            delete lit;
        }

        const int otherId = cb->currentData().toInt();
        if (otherId <= 0)
            return;

        QJsonObject cur = m_lastResult;
        if (cur.isEmpty())
            cur = loadCachedAnalysis(m_employeeId);
        if (cur.isEmpty())
            cur = pseudoAnalysisCompare(m_employeeId);

        QJsonObject oth = loadCachedAnalysis(otherId);
        if (oth.isEmpty())
            oth = pseudoAnalysisCompare(otherId);

        const QString nCur = (m_emp.nom + QLatin1Char(' ') + m_emp.prenom).trimmed();
        QString nameOth = cb->currentText();
        const int lp = nameOth.indexOf(QLatin1Char('('));
        if (lp > 0)
            nameOth = nameOth.left(lp).trimmed();

        const int sCur = jsonIntVal(cur, QStringLiteral("score_global"), jsonIntVal(cur, QStringLiteral("score"), 0));
        const int sOth = jsonIntVal(oth, QStringLiteral("score_global"), jsonIntVal(oth, QStringLiteral("score"), 0));
        sumLbl->setText(QStringLiteral("%1 (%2) vs %3 (%4)")
                            .arg(nCur.isEmpty() ? QStringLiteral("—") : nCur)
                            .arg(sCur)
                            .arg(nameOth.isEmpty() ? QStringLiteral("—") : nameOth)
                            .arg(sOth));

        auto *setSel = new QBarSet(QStringLiteral("Employé sélectionné"));
        *setSel << jsonIntVal(cur, QStringLiteral("experience"), 0) << jsonIntVal(cur, QStringLiteral("performance"), 0)
                << jsonIntVal(cur, QStringLiteral("formation"), 0) << jsonIntVal(cur, QStringLiteral("discipline"), 0)
                << jsonIntVal(cur, QStringLiteral("productivite"), 0);
        setSel->setBrush(QColor(QStringLiteral("#B89B3E")));
        setSel->setPen(QPen(QColor(QStringLiteral("#8B7355")), 1));

        auto *setCmp = new QBarSet(QStringLiteral("Comparé"));
        *setCmp << jsonIntVal(oth, QStringLiteral("experience"), 0) << jsonIntVal(oth, QStringLiteral("performance"), 0)
                << jsonIntVal(oth, QStringLiteral("formation"), 0) << jsonIntVal(oth, QStringLiteral("discipline"), 0)
                << jsonIntVal(oth, QStringLiteral("productivite"), 0);
        setCmp->setBrush(QColor(QStringLiteral("#C17F59")));
        setCmp->setPen(QPen(QColor(QStringLiteral("#6B3A2A")), 1));

        auto *series = new QBarSeries();
        series->append(setSel);
        series->append(setCmp);
        series->setBarWidth(0.78);

        auto *chart = new QChart();
        chart->addSeries(series);
        chart->setBackgroundBrush(QBrush(QColor(QStringLiteral("#EAE0D2"))));
        chart->setPlotAreaBackgroundBrush(QBrush(QColor(QStringLiteral("#F3EBDE"))));
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setLabelColor(QColor(QStringLiteral("#4A4036")));

        auto *axisX = new QBarCategoryAxis();
        axisX->append(QStringList() << QStringLiteral("Exp") << QStringLiteral("Perf") << QStringLiteral("Form")
                                    << QStringLiteral("Disc") << QStringLiteral("Prod"));
        axisX->setLabelsColor(QColor(QStringLiteral("#5C4D3A")));
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        auto *axisY = new QValueAxis();
        axisY->setRange(0, 100);
        axisY->setLabelFormat(QStringLiteral("%i"));
        axisY->setLabelsColor(QColor(QStringLiteral("#5C4D3A")));
        axisY->setGridLineColor(QColor(QStringLiteral("#D4C9B8")));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        auto *view = new QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);
        view->setMinimumHeight(300);
        chartLay->addWidget(view);
    };

    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), dlg, [rebuild](int) { rebuild(); });
    cb->setCurrentIndex(0);
    rebuild();

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}
