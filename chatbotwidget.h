#ifndef CHATBOTWIDGET_H
#define CHATBOTWIDGET_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QTimer>
#include <QTextToSpeech>
#include "assistant_voice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatbotWidget; }
class QNetworkReply;
QT_END_NAMESPACE

class ChatbotWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ChatbotWidget(QWidget *parent = nullptr);
    ~ChatbotWidget() override;

private slots:
    void onSendClicked();
    void onReplyFinished();
    void onReplyError();
    void onRequestTimeout();

    // Voice slots
    void onVoiceStateChanged(AssistantVoiceController::VoiceState s);
    void onVoiceLevelChanged(qreal level);
    void onVoiceTextRecognized(const QString &text);
    void onVoiceError(const QString &code, const QString &message);
    void onSpeakClicked();

private:
    // Scope
    bool    isInScopeQuestion(const QString &question) const;
    bool    isPrivateDataRequest(const QString &question) const;
    bool    isEmployeeNamesListIntent(const QString &question) const;
    bool    isProductListIntent(const QString &question) const;
    bool    isClientListIntent(const QString &question) const;
    bool    isMatiereListIntent(const QString &question) const;
    bool    isDepotListIntent(const QString &question) const;
    bool    isPlanifListIntent(const QString &question) const;
    bool    isEtapeListIntent(const QString &question) const;
    bool    tryAnswerFromDatabase(const QString &question, QString *outAnswer);
    QString outOfScopeMessage() const;
    QString clarifyOffScopeSystemPrompt() const;
    void    postAssistantChat(const QString &userQuestion, const QString &systemPromptText);
    QString systemPrompt() const;

    // UI helpers
    void appendMessageBubble(const QString &message, bool isUser);
    void appendUserMessage(const QString &message);
    void appendBotMessage(const QString &message);
    void forceScrollToBottom();   // FIX: replace clearConversationSelection
    void setLoading(bool loading);

    Ui::ChatbotWidget        *ui;
    QNetworkAccessManager     m_network;
    QPointer<QNetworkReply>   m_activeReply;
    QTimer                    m_timeoutTimer;

    AssistantVoiceController *m_voiceCtrl = nullptr;
    QTextToSpeech            *m_tts = nullptr;
    QString                   m_lastBotMessage;
};

#endif // CHATBOTWIDGET_H