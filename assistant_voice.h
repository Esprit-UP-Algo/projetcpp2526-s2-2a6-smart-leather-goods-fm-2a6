#ifndef ASSISTANT_VOICE_H
#define ASSISTANT_VOICE_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>
#include <QAudioFormat>

class QAudioSource;
class QIODevice;
class QProcess;

/// Half-duplex : micro OU TTS, jamais les deux. Écoute via QAudioSource (Qt Multimedia 6.4+) + seuil + silence ~3,1 s ; WAV 16 kHz pour STT.
class AssistantVoiceController : public QObject
{
    Q_OBJECT
public:
    enum VoiceState {
        Idle = 0,
        Listening = 1,
        Processing = 2,
        Speaking = 3
    };
    Q_ENUM(VoiceState)

    explicit AssistantVoiceController(QObject *parent = nullptr);
    ~AssistantVoiceController() override;

    VoiceState state() const { return m_state; }

    void onTtsStarted();
    void onTtsEnded();

    void toggleListening();
    void startListening();
    void stopListening(bool processAudio);

    void tryStartListeningAuto();

    /// Arrête micro + reconnaissance en cours (ex. fermeture du dialogue assistant).
    void interruptAll();

    qreal lastAudioLevel() const { return m_lastLevel; }

signals:
    void stateChanged(AssistantVoiceController::VoiceState s);
    void audioLevelChanged(qreal level);
    void textRecognized(const QString &text);
    void voiceError(const QString &code, const QString &message);

private slots:
    void onInputReadyRead();
    void onSilencePoll();

private:
    void setState(VoiceState s);
    bool openMicrophone();
    void closeMicrophone();
    void resetCapture();
    void finalizeListening(bool processAudio);
    void runSttOnWav(const QString &wavPath);
    static void writePcmWav16(const QString &path, const QByteArray &pcmS16leMono, int sampleRate);
    static qreal rmsNormalized(const char *data, int nbytes);

    VoiceState m_state = Idle;
    bool m_ttsExterneActif = false;

    QAudioSource *m_audioSource = nullptr;
    QIODevice *m_io = nullptr;
    QByteArray m_pcmBuffer;
    int m_sampleRate = 16000;
    QAudioFormat m_captureFormat;

    QTimer m_pollTimer;
    qint64 m_lastVoiceMs = 0;
    bool m_sawVoice = false;
    qreal m_lastLevel = 0;

    QElapsedTimer m_listenElapsed;
    QProcess *m_sttProcess = nullptr;

    static constexpr int SILENCE_MS = 4200;
    static constexpr int MAX_LISTEN_MS = 60000;
    static constexpr qreal RMS_VOICE_GATE = 0.0045;
    /// Durée minimale après rééchantillonnage 16 kHz (~100 ms).
    static constexpr int MIN_VOICE_BYTES_16K = 3200;
};

#endif
