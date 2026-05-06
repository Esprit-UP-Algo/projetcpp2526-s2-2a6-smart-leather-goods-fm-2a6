#include "chatbotwidget.h"
#include "ui_chatbotwidget.h"
#include "connexion.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextCursor>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextDocument>

namespace {
constexpr int kTimeoutMs = 20000;

/// Connexion Oracle du projet (évite QSqlDatabase::database() si la connexion par défaut n'est pas enregistrée).
static QSqlDatabase filDorOracleDb()
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx || !cnx->estConnecte())
        return {};
    const QSqlDatabase db = cnx->getDatabase();
    return db.isOpen() ? db : QSqlDatabase{};
}

QString readEnvTrimmed(const char *name)
{
    return qEnvironmentVariable(name).trimmed();
}

/// Clé LLM + fournisseur (Groq = API OpenAI-compatible, sans OpenRouter).
struct ApiKeyBundle {
    enum class Provider { None, Groq, OpenRouter, Anthropic };
    QString key;
    Provider provider = Provider::None;
};

static ApiKeyBundle resolveApiKeyBundle()
{
    ApiKeyBundle b;
    b.key = readEnvTrimmed("GROQ_API_KEY");
    if (!b.key.isEmpty()) {
        b.provider = ApiKeyBundle::Provider::Groq;
        return b;
    }
    b.key = readEnvTrimmed("OPENROUTER_API_KEY");
    if (!b.key.isEmpty()) {
        b.provider = ApiKeyBundle::Provider::OpenRouter;
        return b;
    }
    b.key = readEnvTrimmed("ANTHROPIC_API_KEY");
    if (!b.key.isEmpty()) {
        b.provider = ApiKeyBundle::Provider::Anthropic;
        return b;
    }
    b.key = readEnvTrimmed("CLAUDE_API_KEY");
    if (!b.key.isEmpty()) {
        b.provider = ApiKeyBundle::Provider::Anthropic;
        return b;
    }
    b.key = readEnvTrimmed("API_KEY");
    if (!b.key.isEmpty()) {
        if (b.key.startsWith(QLatin1String("gsk_")))
            b.provider = ApiKeyBundle::Provider::Groq;
        else if (b.key.startsWith(QLatin1String("sk-or-v1-")))
            b.provider = ApiKeyBundle::Provider::OpenRouter;
        else if (b.key.startsWith(QLatin1String("sk-ant")))
            b.provider = ApiKeyBundle::Provider::Anthropic;
        else
            b.provider = ApiKeyBundle::Provider::Anthropic;
    }
    return b;
}

static QString groqChatModel()
{
    const QString m = readEnvTrimmed("GROQ_MODEL");
    if (!m.isEmpty())
        return m;
    return QStringLiteral("llama-3.3-70b-versatile");
}

QString sanitizeAssistantText(QString text)
{
    text.replace(QRegularExpression(QStringLiteral(R"(\*\*(.*?)\*\*)")), QStringLiteral("\\1"));
    text.replace(QRegularExpression(QStringLiteral(R"(`(.*?)`)")), QStringLiteral("\\1"));
    text.replace(QRegularExpression(QStringLiteral(R"(^\s*[-•]\s+)"),
                                    QRegularExpression::MultilineOption),
                 QStringLiteral("• "));
    text.replace(QRegularExpression(QStringLiteral(R"(\n{3,})")), QStringLiteral("\n\n"));
    text = text.trimmed();
    if (text.size() > 700)
        text = text.left(700).trimmed() + QStringLiteral("...");
    return text;
}

/// OpenRouter / certains modèles renvoient `content` comme chaîne OU tableau de blocs {type,text}.
QString openRouterAssistantContent(const QJsonObject &messageObj)
{
    const QJsonValue cv = messageObj.value(QStringLiteral("content"));
    if (cv.isString())
        return cv.toString().trimmed();
    if (!cv.isArray())
        return {};

    QString acc;
    for (const QJsonValue &v : cv.toArray()) {
        if (v.isObject()) {
            const QJsonObject o = v.toObject();
            const QString typ = o.value(QStringLiteral("type")).toString();
            if (typ == QStringLiteral("text") || typ.isEmpty())
                acc += o.value(QStringLiteral("text")).toString();
        } else if (v.isString()) {
            acc += v.toString();
        }
    }
    return acc.trimmed();
}

/// Fautes / variantes fréquentes (clavier, STT) pour ne pas rejeter une question valide.
QString normalizeBusinessTypos(QString q)
{
    const QList<QPair<QString, QString>> subs = {
        { QStringLiteral("palinification"), QStringLiteral("planification") },
        { QStringLiteral("plannification"), QStringLiteral("planification") },
        { QStringLiteral("planifcation"), QStringLiteral("planification") },
    };
    for (const auto &p : subs) {
        const int flen = p.first.length();
        int i = 0;
        while ((i = q.indexOf(p.first, i, Qt::CaseInsensitive)) >= 0) {
            q.replace(i, flen, p.second);
            i += p.second.length();
        }
    }
    return q.trimmed();
}
} // namespace

ChatbotWidget::ChatbotWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatbotWidget)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Assistant FIL D'OR"));
    setModal(false);
    resize(760, 560);

    // ── Empêche toute sélection persistante ──────────────────────────────────
    ui->tbConversation->setTextInteractionFlags(Qt::NoTextInteraction);
    ui->tbConversation->setFocusPolicy(Qt::NoFocus);

    // FIX sélection bleue : override stylesheet pour la sélection
    ui->tbConversation->setStyleSheet(
        ui->tbConversation->styleSheet()
        + QStringLiteral("QTextBrowser { selection-background-color: transparent;"
                          " selection-color: #f5f5f3; }"));

    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ChatbotWidget::onRequestTimeout);
    connect(ui->btnSend, &QPushButton::clicked, this, &ChatbotWidget::onSendClicked);
    connect(ui->leQuestion, &QLineEdit::returnPressed, this, &ChatbotWidget::onSendClicked);

    // Initialisation assistant vocal et synthèse vocale
    m_voiceCtrl = new AssistantVoiceController(this);
    m_tts = new QTextToSpeech(this);

    connect(ui->btnVoice, &QPushButton::clicked, m_voiceCtrl, &AssistantVoiceController::toggleListening);
    connect(ui->btnSpeak, &QPushButton::clicked, this, &ChatbotWidget::onSpeakClicked);

    connect(m_voiceCtrl, &AssistantVoiceController::stateChanged, this, &ChatbotWidget::onVoiceStateChanged);
    connect(m_voiceCtrl, &AssistantVoiceController::audioLevelChanged, this, &ChatbotWidget::onVoiceLevelChanged);
    connect(m_voiceCtrl, &AssistantVoiceController::textRecognized, this, &ChatbotWidget::onVoiceTextRecognized);
    connect(m_voiceCtrl, &AssistantVoiceController::voiceError, this, &ChatbotWidget::onVoiceError);

    connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
        if (state == QTextToSpeech::Speaking) {
            m_voiceCtrl->onTtsStarted();
        } else if (state == QTextToSpeech::Ready) {
            m_voiceCtrl->onTtsEnded();
        }
    });

    appendBotMessage(QStringLiteral(
        "Bonjour. Je suis l'assistant FIL D'OR. "
        "Posez une question sur vos modules RH, produits, clients, "
        "dépôt, planification ou matières premières."));
}

ChatbotWidget::~ChatbotWidget()
{
    if (m_voiceCtrl) {
        m_voiceCtrl->interruptAll();
    }
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
    }
    delete ui;
}

// ── Voice slots ──────────────────────────────────────────────────────────────

void ChatbotWidget::onVoiceStateChanged(AssistantVoiceController::VoiceState s)
{
    if (s == AssistantVoiceController::Listening) {
        ui->btnVoice->setStyleSheet(QStringLiteral("background-color: #d9534f; color: white;")); // Rouge
        ui->lblStatus->setText(QStringLiteral("Écoute en cours..."));
    } else if (s == AssistantVoiceController::Processing) {
        ui->btnVoice->setStyleSheet(QStringLiteral("background-color: #f0ad4e; color: white;")); // Orange
        ui->lblStatus->setText(QStringLiteral("Analyse de la voix..."));
    } else if (s == AssistantVoiceController::Speaking) {
        ui->btnVoice->setStyleSheet(QStringLiteral("background-color: #5bc0de; color: white;")); // Bleu
        ui->lblStatus->setText(QStringLiteral("L'assistant parle..."));
    } else {
        ui->btnVoice->setStyleSheet(QStringLiteral("")); // Default
        ui->lblStatus->setText(QStringLiteral("Prêt"));
    }
}

void ChatbotWidget::onVoiceLevelChanged(qreal level)
{
    if (m_voiceCtrl && m_voiceCtrl->state() == AssistantVoiceController::Listening) {
        if (level > 0.015) {
            ui->lblStatus->setText(QStringLiteral("Écoute en cours... 🔊"));
        } else {
            ui->lblStatus->setText(QStringLiteral("Écoute en cours..."));
        }
    }
}

void ChatbotWidget::onVoiceTextRecognized(const QString &text)
{
    const ApiKeyBundle bundle = resolveApiKeyBundle();
    if (bundle.key.isEmpty()) {
        ui->leQuestion->setText(text);
        onSendClicked();
        return;
    }

    const QString correctionPrompt = QStringLiteral(
        "Tu corriges une transcription vocale automatique (souvent erronée en français) pour un logiciel FIL D'OR : "
        "RH, employés, produits, clients, dépôt, planification, matières premières.\n"
        "Restitue une phrase en français correct. Si tu reconnais une intention du type liste des employés, liste des produits, "
        "donner la planification, reformule ainsi.\n"
        "Réponds UNIQUEMENT par le texte corrigé, sans guillemets ni explication.");

    auto finishCorrection = [this, text](const QString &corrected) {
        const QString use = corrected.trimmed().isEmpty() ? text : corrected.trimmed();
        ui->leQuestion->setText(use);
        onSendClicked();
    };

    ui->lblStatus->setText(QStringLiteral("Correction IA en cours..."));

    if (bundle.provider == ApiKeyBundle::Provider::Groq) {
        QNetworkRequest req(QUrl(QStringLiteral("https://api.groq.com/openai/v1/chat/completions")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + bundle.key.toUtf8());

        QJsonObject payload;
        payload[QStringLiteral("model")] = QStringLiteral("llama-3.1-8b-instant");
        payload[QStringLiteral("temperature")] = 0.0;
        payload[QStringLiteral("max_tokens")] = 400;

        QJsonArray messages;
        QJsonObject sysMsg;
        sysMsg[QStringLiteral("role")] = QStringLiteral("system");
        sysMsg[QStringLiteral("content")] = correctionPrompt;
        messages.append(sysMsg);
        QJsonObject userMsg;
        userMsg[QStringLiteral("role")] = QStringLiteral("user");
        userMsg[QStringLiteral("content")] = QStringLiteral("Transcription : ") + text;
        messages.append(userMsg);
        payload[QStringLiteral("messages")] = messages;

        QNetworkReply *reply = m_network.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply, text, finishCorrection]() {
            reply->deleteLater();
            QString corrected;
            if (reply->error() == QNetworkReply::NoError) {
                const QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
                const QJsonArray choices = res[QStringLiteral("choices")].toArray();
                if (!choices.isEmpty())
                    corrected = openRouterAssistantContent(
                        choices[0].toObject().value(QStringLiteral("message")).toObject());
            }
            finishCorrection(corrected);
        });
        return;
    }

    if (bundle.provider == ApiKeyBundle::Provider::OpenRouter) {
        QNetworkRequest req(QUrl(QStringLiteral("https://openrouter.ai/api/v1/chat/completions")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + bundle.key.toUtf8());
        req.setRawHeader("HTTP-Referer", "https://fil-dor.local");

        QJsonObject payload;
        payload[QStringLiteral("model")] = QStringLiteral("anthropic/claude-3.5-haiku");
        payload[QStringLiteral("temperature")] = 0.0;
        payload[QStringLiteral("max_tokens")] = 400;

        QJsonArray messages;
        QJsonObject sysMsg;
        sysMsg[QStringLiteral("role")] = QStringLiteral("system");
        sysMsg[QStringLiteral("content")] = correctionPrompt;
        messages.append(sysMsg);
        QJsonObject userMsg;
        userMsg[QStringLiteral("role")] = QStringLiteral("user");
        userMsg[QStringLiteral("content")] = QStringLiteral("Transcription : ") + text;
        messages.append(userMsg);
        payload[QStringLiteral("messages")] = messages;

        QNetworkReply *reply = m_network.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply, text, finishCorrection]() {
            reply->deleteLater();
            QString corrected;
            if (reply->error() == QNetworkReply::NoError) {
                const QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
                const QJsonArray choices = res[QStringLiteral("choices")].toArray();
                if (!choices.isEmpty())
                    corrected = openRouterAssistantContent(
                        choices[0].toObject().value(QStringLiteral("message")).toObject());
            }
            finishCorrection(corrected);
        });
        return;
    }

    if (bundle.provider == ApiKeyBundle::Provider::Anthropic) {
        QNetworkRequest req(QUrl(QStringLiteral("https://api.anthropic.com/v1/messages")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("x-api-key", bundle.key.toUtf8());
        req.setRawHeader("anthropic-version", "2023-06-01");

        QJsonObject payload;
        payload[QStringLiteral("model")] = QStringLiteral("claude-3-5-haiku-20241022");
        payload[QStringLiteral("max_tokens")] = 400;
        payload[QStringLiteral("system")] = correctionPrompt;
        QJsonArray messages;
        QJsonObject userMsg;
        userMsg[QStringLiteral("role")] = QStringLiteral("user");
        userMsg[QStringLiteral("content")] = QStringLiteral("Transcription : ") + text;
        messages.append(userMsg);
        payload[QStringLiteral("messages")] = messages;

        QNetworkReply *reply = m_network.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply, text, finishCorrection]() {
            reply->deleteLater();
            QString corrected;
            if (reply->error() == QNetworkReply::NoError) {
                const QJsonArray content =
                    QJsonDocument::fromJson(reply->readAll()).object().value(QStringLiteral("content")).toArray();
                for (const QJsonValue &v : content) {
                    const QJsonObject obj = v.toObject();
                    if (obj.value(QStringLiteral("type")).toString() == QStringLiteral("text"))
                        corrected += obj.value(QStringLiteral("text")).toString();
                }
            }
            finishCorrection(corrected);
        });
        return;
    }

    ui->leQuestion->setText(text);
    onSendClicked();
}

void ChatbotWidget::onVoiceError(const QString &code, const QString &message)
{
    appendBotMessage(QStringLiteral("Erreur micro : %1").arg(message));
}

void ChatbotWidget::onSpeakClicked()
{
    if (m_tts && !m_lastBotMessage.isEmpty()) {
        m_tts->say(m_lastBotMessage);
    }
}

// ── Scope guard ──────────────────────────────────────────────────────────────

bool ChatbotWidget::isInScopeQuestion(const QString &question) const
{
    static const QStringList keywords = {
        QStringLiteral("employe"),   QStringLiteral("employé"),
        QStringLiteral("personnel"), QStringLiteral("rh"),
        QStringLiteral("ressource humaine"), QStringLiteral("recrutement"),
        QStringLiteral("candidat"),  QStringLiteral("offre"),
        QStringLiteral("dossier"),   QStringLiteral("competence"),
        QStringLiteral("compétence"),QStringLiteral("anciennete"),
        QStringLiteral("ancienneté"),QStringLiteral("pointage"),
        QStringLiteral("presence"),  QStringLiteral("présence"),
        QStringLiteral("absence"),   QStringLiteral("produit"),
        QStringLiteral("matiere"),   QStringLiteral("matière"),
        QStringLiteral("stock"),     QStringLiteral("client"),
        QStringLiteral("depot"),     QStringLiteral("dépôt"),
        QStringLiteral("dépot"),     QStringLiteral("depôt"),
        QStringLiteral("planification"), QStringLiteral("etape"),
        QStringLiteral("étape"),     QStringLiteral("workflow"),
        QStringLiteral("verification"), QStringLiteral("vérification"),
        QStringLiteral("verifier"),  QStringLiteral("vérifier"),
        QStringLiteral("controle"),  QStringLiteral("contrôle"),
        QStringLiteral("detail"),    QStringLiteral("détail"),
        QStringLiteral("analyse"),   QStringLiteral("audit"),
        QStringLiteral("matiére"),   QStringLiteral("premier"),
        QStringLiteral("premiér"),   QStringLiteral("premièr"),
        QStringLiteral("salaire"),   QStringLiteral("poste"),
        QStringLiteral("departement"), QStringLiteral("département"),
        QStringLiteral("موظف"),      QStringLiteral("الموظف"),
        QStringLiteral("رواتب"),     QStringLiteral("راتب"),
        QStringLiteral("توظيف"),     QStringLiteral("موارد بشرية"),
        QStringLiteral("حضور"),      QStringLiteral("غياب"),
        QStringLiteral("منتج"),      QStringLiteral("منتجات"),
        QStringLiteral("عميل"),      QStringLiteral("عملاء"),
        QStringLiteral("مخزن"),      QStringLiteral("مستودع"),
        QStringLiteral("تخطيط"),     QStringLiteral("مادة اولية"),
        QStringLiteral("مواد اولية"),QStringLiteral("مراحل"),
    };

    const QString normalized = question.toLower();
    for (const QString &k : keywords) {
        if (normalized.contains(k))
            return true;
    }
    return false;
}

bool ChatbotWidget::isPrivateDataRequest(const QString &question) const
{
    QRegularExpression re(QStringLiteral("\\b(id|ids|identifiant|identifiants|email|emails|e-mail|e-mails|mail|mails|telephone|téléphone|telephones|téléphones|mot de passe|password|mdp|privé|privées|prive|salaire|salaires)\\b"), QRegularExpression::CaseInsensitiveOption);
    return re.match(question).hasMatch();
}

bool ChatbotWidget::isEmployeeNamesListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasEmp = q.contains(QStringLiteral("employ")) || q.contains(QStringLiteral("personnel"))
                        || q.contains(QStringLiteral("موظف")) || q.contains(QStringLiteral("الموظف"))
                        || q.contains(QStringLiteral("موظفين")) || q.contains(QStringLiteral("الموظفين"));

    // Comme pour les autres listes : « donner les employés » / « donner les employées » doit déclencher la requête SQL.
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("montrer")) || q.contains(QStringLiteral("montre"))
                         || q.contains(QStringLiteral("عرض")) || q.contains(QStringLiteral("قائمة"));

    const bool asksNames = q.contains(QStringLiteral("nom")) || q.contains(QStringLiteral("prénom"))
                           || q.contains(QStringLiteral("prenom")) || q.contains(QStringLiteral("اسم"));

    return hasEmp && (hasList || asksNames);
}

bool ChatbotWidget::isProductListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasProd = q.contains(QStringLiteral("produit")) || q.contains(QStringLiteral("article"))
                         || q.contains(QStringLiteral("منتج")) || q.contains(QStringLiteral("منتجات"));

    // « donner » autorisé ici (ex. « donner la liste des produits ») — exclu seulement pour la liste des employés.
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("disponible")) || q.contains(QStringLiteral("عرض"))
                         || q.contains(QStringLiteral("قائمة"));

    return hasProd && hasList;
}

bool ChatbotWidget::isClientListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasClient = q.contains(QStringLiteral("client")) || q.contains(QStringLiteral("عميل")) || q.contains(QStringLiteral("عملاء"));
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("عرض")) || q.contains(QStringLiteral("قائمة"));
    return hasClient && hasList;
}

bool ChatbotWidget::isMatiereListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasMatiere = q.contains(QStringLiteral("matiere")) || q.contains(QStringLiteral("matière")) || q.contains(QStringLiteral("matiére")) || q.contains(QStringLiteral("premier")) || q.contains(QStringLiteral("premiér")) || q.contains(QStringLiteral("premièr")) || q.contains(QStringLiteral("stock")) || q.contains(QStringLiteral("مادة اولية")) || q.contains(QStringLiteral("مواد اولية"));
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("disponible")) || q.contains(QStringLiteral("عرض"))
                         || q.contains(QStringLiteral("قائمة"));
    return hasMatiere && hasList;
}

bool ChatbotWidget::isDepotListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasDepot = q.contains(QStringLiteral("depot")) || q.contains(QStringLiteral("dépôt")) || q.contains(QStringLiteral("dépot")) || q.contains(QStringLiteral("depôt")) || q.contains(QStringLiteral("emplacement")) || q.contains(QStringLiteral("مخزن")) || q.contains(QStringLiteral("مستودع"));
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("عرض")) || q.contains(QStringLiteral("قائمة"));
    return hasDepot && hasList;
}

bool ChatbotWidget::isPlanifListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasPlanif = q.contains(QStringLiteral("planif")) || q.contains(QStringLiteral("commande")) || q.contains(QStringLiteral("ordre")) || q.contains(QStringLiteral("تخطيط")) || q.contains(QStringLiteral("طلبية"));
    const bool hasList = q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
                         || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
                         || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
                         || q.contains(QStringLiteral("عرض")) || q.contains(QStringLiteral("قائمة"));
    return hasPlanif && hasList;
}

bool ChatbotWidget::isEtapeListIntent(const QString &question) const
{
    const QString q = question.toLower().trimmed();
    const bool hasEtape = q.contains(QStringLiteral("étape")) || q.contains(QStringLiteral("etape")) || q.contains(QStringLiteral("مرحلة")) || q.contains(QStringLiteral("مراحل"));
    const bool etapesParProduit = hasEtape
        && (q.contains(QStringLiteral("produit")) || q.contains(QStringLiteral("produits"))
            || q.contains(QStringLiteral("article")) || q.contains(QStringLiteral("articles")));
    const bool hasList = etapesParProduit
        || q.contains(QStringLiteral("liste")) || q.contains(QStringLiteral("list"))
        || q.contains(QStringLiteral("lister")) || q.contains(QStringLiteral("donner"))
        || q.contains(QStringLiteral("affiche")) || q.contains(QStringLiteral("afficher"))
        || q.contains(QStringLiteral("montrer")) || q.contains(QStringLiteral("montre"))
        || q.contains(QStringLiteral("عرض")) || q.contains(QStringLiteral("قائمة"));
    return hasEtape && hasList;
}

bool ChatbotWidget::tryAnswerFromDatabase(const QString &question, QString *outAnswer)
{
    if (!outAnswer)
        return false;
    outAnswer->clear();

    if (isEmployeeNamesListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher les noms/prénoms des employés, mais la base de données n'est pas connectée.");
            return true;
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "SELECT NOM, PRENOM FROM ("
            "  SELECT NOM, PRENOM FROM EMPLOYES "
            "  WHERE NOM IS NOT NULL OR PRENOM IS NOT NULL "
            "  ORDER BY UPPER(NOM), UPPER(PRENOM)"
            ") WHERE ROWNUM <= 25"));

        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture employés) : %1").arg(q.lastError().text());
            return true;
        }

        QStringList lines;
        while (q.next()) {
            const QString nom = q.value(0).toString().trimmed();
            const QString prenom = q.value(1).toString().trimmed();
            const QString full = QStringLiteral("%1 %2").arg(prenom, nom).trimmed();
            if (!full.isEmpty())
                lines << full;
        }

        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucun employé trouvé en base (table EMPLOYES).");
            return true;
        }

        *outAnswer = QStringLiteral("Noms des employés (limité à 25) :\n• %1")
                         .arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isEtapeListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher les étapes, mais la base de données n'est pas connectée.");
            return true;
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "SELECT * FROM ("
            "  SELECT e.ETAPE_ACTUELLE, pr.DESIGNATION "
            "  FROM ETAPES e "
            "  JOIN PLANIFICATION p ON e.ID_PLANIFICATION = p.ID_COMMANDE "
            "  JOIN PRODUITS pr ON p.ID_PRODUIT = pr.ID_PRODUIT "
            "  ORDER BY e.ID_SUIVI DESC"
            ") WHERE ROWNUM <= 25"
        ));

        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture étapes) : %1").arg(q.lastError().text());
            return true;
        }

        QStringList lines;
        while (q.next()) {
            const QString etape = q.value(0).toString().trimmed();
            const QString prod = q.value(1).toString().trimmed();
            lines << QStringLiteral("Étape: %1 (Produit: %2)").arg(etape, prod);
        }

        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Les étapes générales de production sont : Coupe, Assemblage, Couture, Finition.\n(Aucune étape en cours trouvée en base de données).");
            return true;
        }

        *outAnswer = QStringLiteral("Dernières étapes de production enregistrées :\n• %1")
                         .arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isProductListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher la liste des produits, mais la base de données n'est pas connectée.");
            return true;
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "SELECT DESIGNATION, COLLECTION FROM ("
            "  SELECT DESIGNATION, COLLECTION FROM PRODUITS "
            "  WHERE DESIGNATION IS NOT NULL "
            "  ORDER BY UPPER(DESIGNATION)"
            ") WHERE ROWNUM <= 25"));

        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture produits) : %1").arg(q.lastError().text());
            return true;
        }

        QStringList lines;
        while (q.next()) {
            const QString desig = q.value(0).toString().trimmed();
            const QString coll = q.value(1).toString().trimmed();
            QString full = desig;
            if (!coll.isEmpty())
                full += QStringLiteral(" (Collection: %1)").arg(coll);
            if (!full.isEmpty())
                lines << full;
        }

        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucun produit trouvé en base (table PRODUITS).");
            return true;
        }

        *outAnswer = QStringLiteral("Liste des produits disponibles (limité à 25) :\n• %1")
                         .arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isClientListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher la liste des clients, mais la base de données n'est pas connectée.");
            return true;
        }
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT NOM FROM (SELECT NOM FROM CLIENTS WHERE NOM IS NOT NULL ORDER BY UPPER(NOM)) WHERE ROWNUM <= 25"));
        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture clients) : %1").arg(q.lastError().text());
            return true;
        }
        QStringList lines;
        while (q.next()) {
            lines << q.value(0).toString().trimmed();
        }
        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucun client trouvé en base (table CLIENTS).");
            return true;
        }
        *outAnswer = QStringLiteral("Liste des clients (sans données privées, limité à 25) :\n• %1").arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isMatiereListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher la liste des matières, mais la base n'est pas connectée.");
            return true;
        }
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT CODE_MP, CATEGORIE_MP, QUANTITE FROM (SELECT CODE_MP, CATEGORIE_MP, QUANTITE FROM MATIERES_PREMIERES ORDER BY UPPER(CODE_MP)) WHERE ROWNUM <= 25"));
        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture matières) : %1").arg(q.lastError().text());
            return true;
        }
        QStringList lines;
        while (q.next()) {
            lines << QStringLiteral("%1 (Cat: %2) - Qté: %3").arg(q.value(0).toString().trimmed(), q.value(1).toString().trimmed(), q.value(2).toString());
        }
        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucune matière première trouvée en base.");
            return true;
        }
        *outAnswer = QStringLiteral("Liste des matières premières (limité à 25) :\n• %1").arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isDepotListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher la liste des dépôts, mais la base n'est pas connectée.");
            return true;
        }
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT ETAGERE, TYPE_STOCKAGE FROM (SELECT ETAGERE, TYPE_STOCKAGE FROM DEPOTS ORDER BY UPPER(ETAGERE)) WHERE ROWNUM <= 25"));
        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture dépôts) : %1").arg(q.lastError().text());
            return true;
        }
        QStringList lines;
        while (q.next()) {
            lines << QStringLiteral("%1 (Type: %2)").arg(q.value(0).toString().trimmed(), q.value(1).toString().trimmed());
        }
        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucun dépôt trouvé en base.");
            return true;
        }
        *outAnswer = QStringLiteral("Liste des emplacements de dépôt :\n• %1").arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }
    else if (isPlanifListIntent(question)) {
        QSqlDatabase db = filDorOracleDb();
        if (!db.isValid() || !db.isOpen()) {
            *outAnswer = QStringLiteral("Je peux afficher la liste des commandes, mais la base n'est pas connectée.");
            return true;
        }
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT ID_COMMANDE, STATUT FROM (SELECT ID_COMMANDE, STATUT FROM PLANIFICATION ORDER BY ID_COMMANDE DESC) WHERE ROWNUM <= 25"));
        if (!q.exec()) {
            *outAnswer = QStringLiteral("Erreur Oracle (lecture planification) : %1").arg(q.lastError().text());
            return true;
        }
        QStringList lines;
        while (q.next()) {
            lines << QStringLiteral("Commande #%1 - Statut: %2").arg(q.value(0).toString(), q.value(1).toString().trimmed());
        }
        if (lines.isEmpty()) {
            *outAnswer = QStringLiteral("Aucune commande trouvée en base.");
            return true;
        }
        *outAnswer = QStringLiteral("Dernières commandes en cours (limité à 25) :\n• %1").arg(lines.join(QStringLiteral("\n• ")));
        return true;
    }

    return false;
}

QString ChatbotWidget::outOfScopeMessage() const
{
    return QStringLiteral(
        "Je ne relie pas votre phrase aux sujets FIL D’OR (RH, employés, produits, clients, dépôt, planification, matières premières).\n\n"
        "Si vous parliez au micro : la transcription est souvent en cause. Choisissez le bon micro dans "
        "Paramètres Windows → Son, parlez distinctement près du micro, ou écrivez la question au clavier.\n\n"
        "Pour une bonne reconnaissance, il faut Python avec faster-whisper et le fichier assistant_vocal.py "
        "à côté de l’application — sinon Windows transcrit très mal le français.\n\n"
        "Exemples : « liste des employés », « donner la liste des produits », « donner la planification ».");
}

QString ChatbotWidget::clarifyOffScopeSystemPrompt() const
{
    return QStringLiteral(
        "Tu es l’assistant FIL D’OR (gestion maroquinerie : RH, employés, produits, clients, dépôt, planification, matières premières).\n"
        "Le message utilisateur ne correspond pas aux mots-clés automatiques OU ressemble à une transcription vocale erronée.\n"
        "Réponds en français en 4 phrases maximum, ton bref et utile : "
        "explique le périmètre FIL D’OR ; suggère que le micro a peut-être mal transcrit ; "
        "invite à reformuler ou à taper au clavier ; donne deux exemples de phrases (« liste des employés », « liste des produits »). "
        "Évite de répéter exactement le même long avertissement qu’un robot. Pas de markdown.");
}

QString ChatbotWidget::systemPrompt() const
{
    return QStringLiteral(
        "Tu es l'assistant virtuel de l'application de gestion \"FIL D'OR\".\n"
        "Tu as accès aux informations générales sur TOUTES les entités de l'entreprise : "
        "Employés, RH, Produits, Matières Premières, Clients, Dépôt, Planification, Étapes.\n"
        "Tu dois répondre à l'utilisateur sur n'importe laquelle de ces entités pour l'aider du mieux possible.\n"
        "RÈGLE STRICTE: Ne donne JAMAIS de données privées ou sensibles (comme les salaires exacts, les mots de passe, les numéros de téléphone, les adresses email, ou les identifiants personnels).\n"
        "Réponds en français, de manière amicale, concise et professionnelle, "
        "en 3-5 phrases maximum, en texte simple (sans markdown).");
}

void ChatbotWidget::postAssistantChat(const QString &userQuestion, const QString &systemPromptText)
{
    const ApiKeyBundle bundle = resolveApiKeyBundle();
    if (bundle.key.isEmpty()) {
        appendBotMessage(QStringLiteral(
            "⚠ Clé API introuvable.\n"
            "Définissez la variable d'environnement GROQ_API_KEY (clé Groq, préfixe gsk_…), "
            "ou à défaut OPENROUTER_API_KEY / ANTHROPIC_API_KEY. "
            "Optionnel : GROQ_MODEL (ex. llama-3.3-70b-versatile). Puis relancez l'application."));
        return;
    }

    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply.clear();
    }

    QNetworkRequest req;
    QJsonObject payload;
    QJsonArray  messages;
    QJsonObject userMsg;
    userMsg[QStringLiteral("role")]    = QStringLiteral("user");
    userMsg[QStringLiteral("content")] = userQuestion;
    messages.append(userMsg);

    QString providerTag;

    if (bundle.provider == ApiKeyBundle::Provider::Groq) {
        providerTag = QStringLiteral("groq");
        ui->lblStatus->setText(QStringLiteral("Envoi (Groq)…"));
        req.setUrl(QUrl(QStringLiteral("https://api.groq.com/openai/v1/chat/completions")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("User-Agent", "FIL-DOR-Qt/1.0");
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + bundle.key.toUtf8());
        payload[QStringLiteral("model")]       = groqChatModel();
        payload[QStringLiteral("max_tokens")]  = 450;
        payload[QStringLiteral("temperature")] = 0.15;
        QJsonObject sysMsg;
        sysMsg[QStringLiteral("role")]    = QStringLiteral("system");
        sysMsg[QStringLiteral("content")] = systemPromptText;
        messages.prepend(sysMsg);
        payload[QStringLiteral("messages")] = messages;
    } else if (bundle.provider == ApiKeyBundle::Provider::OpenRouter) {
        providerTag = QStringLiteral("openrouter");
        ui->lblStatus->setText(QStringLiteral("Envoi (OpenRouter)…"));
        req.setUrl(QUrl(QStringLiteral("https://openrouter.ai/api/v1/chat/completions")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("User-Agent", "FIL-DOR-Qt/1.0");
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + bundle.key.toUtf8());
        req.setRawHeader("HTTP-Referer",  "https://fil-dor.local");
        req.setRawHeader("X-Title",       "FIL D'OR Assistant");
        payload[QStringLiteral("model")]       = QStringLiteral("anthropic/claude-sonnet-4");
        payload[QStringLiteral("max_tokens")]  = 450;
        payload[QStringLiteral("temperature")] = 0.15;
        QJsonObject sysMsg;
        sysMsg[QStringLiteral("role")]    = QStringLiteral("system");
        sysMsg[QStringLiteral("content")] = systemPromptText;
        messages.prepend(sysMsg);
        payload[QStringLiteral("messages")] = messages;
    } else {
        providerTag = QStringLiteral("anthropic");
        ui->lblStatus->setText(QStringLiteral("Envoi (Anthropic)…"));
        req.setUrl(QUrl(QStringLiteral("https://api.anthropic.com/v1/messages")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("User-Agent", "FIL-DOR-Qt/1.0");
        req.setRawHeader("x-api-key",         bundle.key.toUtf8());
        req.setRawHeader("anthropic-version",  "2023-06-01");
        payload[QStringLiteral("model")]      = QStringLiteral("claude-sonnet-4-20250514");
        payload[QStringLiteral("max_tokens")] = 450;
        payload[QStringLiteral("system")]     = systemPromptText;
        payload[QStringLiteral("messages")] = messages;
    }

    setLoading(true);
    m_activeReply = m_network.post(req,
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
    m_activeReply->setProperty("provider", providerTag);

    connect(m_activeReply, &QNetworkReply::finished,
            this, &ChatbotWidget::onReplyFinished);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_activeReply, &QNetworkReply::sslErrors,
            this, [this](const QList<QSslError> &errors) {
        QStringList msgs;
        for (const QSslError &e : errors)
            msgs << e.errorString();
        appendBotMessage(QStringLiteral("Erreur SSL : ") + msgs.join(QStringLiteral(" | ")));
    });
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_activeReply, &QNetworkReply::errorOccurred,
            this, &ChatbotWidget::onReplyError);
#else
    connect(m_activeReply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &ChatbotWidget::onReplyError);
#endif

    m_timeoutTimer.start(kTimeoutMs);
}

// ── Message rendering ────────────────────────────────────────────────────────

/*
 * CORRECTION PRINCIPALE :
 *   - On utilise insertHtml() au lieu de append() → évite le saut de ligne
 *     parasite et le re-focus automatique qui crée la sélection bleue.
 *   - On remplace le SVG inline (non supporté par QTextBrowser) par un emoji
 *     Unicode encadré d'un <span> coloré : 🤖 pour le bot, 👤 pour l'user.
 *   - On force le curseur en fin de document SANS sélection après chaque insert.
 */
void ChatbotWidget::appendMessageBubble(const QString &message, bool isUser)
{
    const QString stamp =
        QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    const QString safe =
        message.toHtmlEscaped().replace(QLatin1String("\n"), QLatin1String("<br>"));

    // Icônes texte (emoji) — QTextBrowser les affiche parfaitement
    // contrairement aux SVG inline
    const QString icon      = isUser ? QStringLiteral("&#128100;") // 👤
                                     : QStringLiteral("&#129302;"); // 🤖
    const QString iconColor = isUser ? QStringLiteral("#d4af37") : QStringLiteral("#5b9bd5");
    const QString nameColor = isUser ? QStringLiteral("#d4af37") : QStringLiteral("#8ec7ff");
    const QString nameTxt   = isUser ? QStringLiteral("Vous")
                                     : QStringLiteral("Assistant FIL D'OR");
    const QString bubbleBg  = isUser ? QStringLiteral("#3a3a35") : QStringLiteral("#2f3a46");
    const QString bubbleFg  = isUser ? QStringLiteral("#f5f5f3") : QStringLiteral("#f1f7ff");

    // Pour l'utilisateur : icône à droite ; pour le bot : icône à gauche
    QString html;
    if (isUser) {
        html = QStringLiteral(
            "<table width='100%' cellspacing='0' cellpadding='0'>"
            "<tr>"
            "  <td align='right' style='padding-right:6px;'>"
            "    <span style='color:%1;font-weight:800;'>%2</span>"
            "    <span style='color:#9f9f9f;font-size:11px;'> (%3)</span><br>"
            "    <table align='right' border='0' cellspacing='0' cellpadding='8' style='margin-top:4px; max-width:480px;' bgcolor='%4'>"
            "      <tr><td style='color:%5; line-height:1.45;'>%6</td></tr>"
            "    </table>"
            "  </td>"
            "  <td width='36' valign='top' align='center'>"
            "    <span style='font-size:22px;color:%7;'>%8</span>"
            "  </td>"
            "</tr>"
            "</table>")
            .arg(nameColor, nameTxt, stamp,
                 bubbleBg, bubbleFg, safe,
                 iconColor, icon);
    } else {
        html = QStringLiteral(
            "<table width='100%' cellspacing='0' cellpadding='0'>"
            "<tr>"
            "  <td width='36' valign='top' align='center'>"
            "    <span style='font-size:22px;color:%1;'>%2</span>"
            "  </td>"
            "  <td align='left' style='padding-left:6px;'>"
            "    <span style='color:%3;font-weight:800;'>%4</span>"
            "    <span style='color:#9f9f9f;font-size:11px;'> (%5)</span><br>"
            "    <table align='left' border='0' cellspacing='0' cellpadding='8' style='margin-top:4px; max-width:480px;' bgcolor='%6'>"
            "      <tr><td style='color:%7; line-height:1.45;'>%8</td></tr>"
            "    </table>"
            "  </td>"
            "</tr>"
            "</table>")
            .arg(iconColor, icon,
                 nameColor, nameTxt, stamp,
                 bubbleBg, bubbleFg, safe);
    }

    // Ajoute un bloc vide de séparation entre messages
    ui->tbConversation->insertHtml(html);
    ui->tbConversation->insertHtml(QStringLiteral("<br>"));

    // FIX sélection bleue : forcer curseur en fin, sans sélection
    forceScrollToBottom();
}

void ChatbotWidget::forceScrollToBottom()
{
    QTextCursor c = ui->tbConversation->textCursor();
    c.clearSelection();
    c.movePosition(QTextCursor::End);
    ui->tbConversation->setTextCursor(c);
    ui->tbConversation->verticalScrollBar()->setValue(
        ui->tbConversation->verticalScrollBar()->maximum());
}

void ChatbotWidget::appendUserMessage(const QString &msg) { appendMessageBubble(msg, true);  }
// Filtre les données privées (ID, email, téléphone, etc.) dans la réponse du bot
static QString maskPrivateData(const QString &text) {
    QString filtered = text;
    // Masque les suites de chiffres probables d'ID (6 chiffres ou plus)
    filtered.replace(QRegularExpression("\\b\\d{6,}\\b"), "[donnée masquée]");
    // Masque les emails
    filtered.replace(QRegularExpression("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}"), "[email masqué]");
    // Masque les numéros de téléphone (formats courants)
    filtered.replace(QRegularExpression("\\b(\\+?\\d{1,3}[ .-]?)?(\\d{2,3}[ .-]?){3,5}\\b"), "[téléphone masqué]");
    // Masque les adresses (motifs simples)
    filtered.replace(QRegularExpression("\\b\\d{1,4} ?(rue|avenue|bd|boulevard|impasse|allée|chemin|street|road|avenida|strada)\\b", QRegularExpression::CaseInsensitiveOption), "[adresse masquée]");
    return filtered;
}

void ChatbotWidget::appendBotMessage(const QString &msg)  {
    QString safeMsg = maskPrivateData(msg);
    if (safeMsg.trimmed().isEmpty()) {
        safeMsg = QStringLiteral(
            "Réponse indisponible (texte vide). Si vous utilisez OpenRouter, mettez à jour l’application "
            "ou reformulez ; en cas de micro, vérifiez la transcription.");
    }
    appendMessageBubble(safeMsg, false);

    if (m_tts) {
        QTextDocument doc;
        doc.setHtml(safeMsg);
        QString cleanText = doc.toPlainText();
        cleanText.replace(QRegularExpression(QStringLiteral("[\n•]")), QStringLiteral(" "));
        m_lastBotMessage = cleanText;
    }
}

// ── UI state ─────────────────────────────────────────────────────────────────

void ChatbotWidget::setLoading(bool loading)
{
    ui->btnSend->setEnabled(!loading);
    ui->leQuestion->setEnabled(!loading);
    ui->lblStatus->setText(loading ? QStringLiteral("Analyse en cours…")
                                   : QStringLiteral("Prêt"));
}

// ── Send ─────────────────────────────────────────────────────────────────────

void ChatbotWidget::onSendClicked()
{
    const QString rawInput = ui->leQuestion->text().trimmed();
    if (rawInput.isEmpty())
        return;

    /* Logique (périmètre, SQL, API) sur texte corrigé ; la bulle affiche ce que l’utilisateur a saisi. */
    const QString question = normalizeBusinessTypos(rawInput);

    appendUserMessage(rawInput);
    ui->leQuestion->clear();

    // Retire le focus du TextBrowser immédiatement → pas de sélection bleue
    ui->leQuestion->setFocus();

    if (isPrivateDataRequest(question)) {
        appendBotMessage(QStringLiteral("Je suis désolé, mais pour des raisons de sécurité et de confidentialité, je ne suis pas autorisé à divulguer ou à traiter des données privées telles que les identifiants, mots de passe, numéros de téléphone, adresses e-mail ou salaires."));
        return;
    }

    if (!isInScopeQuestion(question)) {
        /// Phrases sans mot-clé métier (souvent mauvaise transcription vocale) : aide contextuelle via IA si possible.
        if (!resolveApiKeyBundle().key.isEmpty())
            postAssistantChat(question, clarifyOffScopeSystemPrompt());
        else
            appendBotMessage(outOfScopeMessage());
        return;
    }

    QString localAnswer;
    if (tryAnswerFromDatabase(question, &localAnswer)) {
        appendBotMessage(localAnswer);
        return;
    }

    postAssistantChat(question, systemPrompt());
}

// ── Network callbacks ────────────────────────────────────────────────────────

void ChatbotWidget::onReplyFinished()
{
    if (!m_activeReply)
        return;

    m_timeoutTimer.stop();
    const int      statusCode = m_activeReply
        ->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body     = m_activeReply->readAll();
    const bool hasError =
        (m_activeReply->error() != QNetworkReply::NoError)
        || statusCode < 200 || statusCode >= 300;

    if (hasError) {
        QString details = m_activeReply->errorString();
        QJsonParseError jerr;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
        if (jerr.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject root = doc.object();
            const QJsonObject errObj = root.value(QStringLiteral("error")).toObject();
            const QString msg = errObj.value(QStringLiteral("message")).toString().trimmed();
            if (!msg.isEmpty())
                details = msg;
        }
        const QString provider = m_activeReply->property("provider").toString();
        appendBotMessage(QStringLiteral("Erreur API %1 (%2) : %3")
            .arg(provider).arg(statusCode).arg(details));
    } else {
        QString answer;
        QJsonParseError jerr;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);
        if (jerr.error != QJsonParseError::NoError) {
            appendBotMessage(QStringLiteral("Réponse API illisible (JSON invalide) : %1").arg(jerr.errorString()));
        } else if (doc.isObject()) {
            const QString provider = m_activeReply->property("provider").toString();
            if (provider == QStringLiteral("openrouter") || provider == QStringLiteral("groq")) {
                const QJsonArray choices =
                    doc.object().value(QStringLiteral("choices")).toArray();
                if (!choices.isEmpty())
                    answer = openRouterAssistantContent(
                        choices.first().toObject().value(QStringLiteral("message")).toObject());
            } else {
                // Anthropic format
                const QJsonArray content =
                    doc.object().value(QStringLiteral("content")).toArray();
                for (const QJsonValue &v : content) {
                    const QJsonObject obj = v.toObject();
                    if (obj.value(QStringLiteral("type")).toString()
                        == QStringLiteral("text")) {
                        const QString txt =
                            obj.value(QStringLiteral("text")).toString().trimmed();
                        if (!txt.isEmpty()) {
                            if (!answer.isEmpty())
                                answer += QLatin1Char('\n');
                            answer += txt;
                        }
                    }
                }
            }
        }
        answer = sanitizeAssistantText(answer);
        if (answer.isEmpty())
            answer = QStringLiteral("Aucune réponse reçue de l'API.");
        appendBotMessage(answer);
    }

    m_activeReply->deleteLater();
    m_activeReply.clear();
    setLoading(false);
}

void ChatbotWidget::onReplyError()
{
    if (!m_activeReply)
        return;
    const QString provider = m_activeReply->property("provider").toString();
    appendBotMessage(QStringLiteral("Erreur réseau (%1) : %2")
                         .arg(provider, m_activeReply->errorString()));
    ui->lblStatus->setText(QStringLiteral("Erreur réseau"));
}

void ChatbotWidget::onRequestTimeout()
{
    if (!m_activeReply)
        return;
    m_activeReply->abort();
    appendBotMessage(QStringLiteral(
        "Délai dépassé (%1 s). Vérifiez votre connexion réseau et réessayez.")
        .arg(kTimeoutMs / 1000));
    m_activeReply->deleteLater();
    m_activeReply.clear();
    setLoading(false);
}