#include "assistant_voice.h"

#include <QObject>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSource>
#include <QMediaDevices>
#include <QIODevice>
#include <QFile>
#include <QDataStream>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QtMath>
#include <QDebug>
#include <QDateTime>
#include <QPointer>
#include <QRegularExpression>
#include <QCoreApplication>

#include <cmath>
#include <cstring>

namespace {

QString tempWavPath()
{
    return QDir::temp().filePath(QStringLiteral("fildor_assistant_%1.wav")
                                     .arg(QDateTime::currentMSecsSinceEpoch()));
}

static qreal rmsMonoS16Block(const char *data, int nbytes)
{
    if (nbytes < 2)
        return 0;
    const int nSamp = nbytes / 2;
    const qint16 *s = reinterpret_cast<const qint16 *>(data);
    double acc = 0;
    for (int i = 0; i < nSamp; ++i) {
        const double v = s[i] / 32768.0;
        acc += v * v;
    }
    return qMin(1.0, sqrt(acc / qMax(1, nSamp)));
}

static void removeDcAndNoiseFloorMonoS16(QByteArray &pcm, qreal gateNorm)
{
    const int n = pcm.size() / 2;
    if (n <= 0)
        return;
    qint16 *s = reinterpret_cast<qint16 *>(pcm.data());
    qint64 sum = 0;
    for (int i = 0; i < n; ++i)
        sum += s[i];
    const qreal dc = qreal(sum) / qMax(1, n);
    const qint32 gate = qint32(qBound(0.0, gateNorm, 0.1) * 32768.0);
    for (int i = 0; i < n; ++i) {
        qint32 v = qint32(qRound(qreal(s[i]) - dc));
        if (qAbs(v) < gate)
            v = 0;
        s[i] = qint16(qBound(-32768, v, 32767));
    }
}

/// AGC léger : augmente les voix faibles sans sur-amplifier le bruit.
static void normalizeMonoS16Agc(QByteArray &pcm, qreal targetRmsNorm, qreal maxGain)
{
    const int n = pcm.size() / 2;
    if (n <= 0)
        return;
    qint16 *s = reinterpret_cast<qint16 *>(pcm.data());
    qint32 peak = 0;
    double acc = 0;
    for (int i = 0; i < n; ++i) {
        const qint32 a = qAbs(int(s[i]));
        peak = qMax(peak, a);
        const double v = s[i] / 32768.0;
        acc += v * v;
    }
    if (peak < 900)
        return;
    const qreal rms = qreal(sqrt(acc / qMax(1, n)));
    if (rms <= 0.0005)
        return;
    qreal gain = qBound<qreal>(1.0, targetRmsNorm / rms, maxGain);
    gain = qMin(gain, qreal(31000.0 / qMax(1, peak)));
    if (gain <= 1.02)
        return;
    for (int i = 0; i < n; ++i) {
        const qint32 v = qint32(qRound(s[i] * gain));
        s[i] = qint16(qBound(-32768, v, 32767));
    }
}

static QString postCorrectRecognizedText(QString text)
{
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    text = text.trimmed();
    if (text.isEmpty())
        return text;

    // Nettoie les petits fragments parasites au début ("euh", "tf", etc.).
    QStringList parts = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    while (!parts.isEmpty()) {
        const QString p = parts.first().toLower();
        const bool filler = (p == QStringLiteral("euh") || p == QStringLiteral("heu")
                             || p == QStringLiteral("hum") || p == QStringLiteral("ben"));
        const bool tinyNoise = (p.size() <= 2 && QRegularExpression(QStringLiteral("^[a-z]+$")).match(p).hasMatch()
                                && p != QStringLiteral("rh"));
        if (!filler && !tinyNoise)
            break;
        parts.removeFirst();
    }
    text = parts.join(QLatin1Char(' '));
    if (text.isEmpty())
        return text;

    // Corrections très ciblées seulement (évite les sur-corrections).
    const QList<QPair<QRegularExpression, QString>> rules = {
        {QRegularExpression(QStringLiteral("\\bles\\s+anglais\\b"), QRegularExpression::CaseInsensitiveOption),
         QStringLiteral("les employes")},
        {QRegularExpression(QStringLiteral("\\bliste\\s+des\\s+anglais\\b"), QRegularExpression::CaseInsensitiveOption),
         QStringLiteral("liste des employes")},
        {QRegularExpression(QStringLiteral("\\bemploye[eé]s?\\b"), QRegularExpression::CaseInsensitiveOption),
         QStringLiteral("employes")},
        {QRegularExpression(QStringLiteral("\\blister\\s+les\\s+employe[eé]s?\\b"), QRegularExpression::CaseInsensitiveOption),
         QStringLiteral("liste des employes")},
        {QRegularExpression(QStringLiteral("\\bindice\\s+des\\s+employe[eé]s?\\b"), QRegularExpression::CaseInsensitiveOption),
         QStringLiteral("liste des employes")}
    };
    for (const auto &r : rules)
        text.replace(r.first, r.second);

    // Supprime les répétitions directes de mots ("test test", "liste liste").
    QStringList out;
    QString prev;
    const QStringList toks = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &rawTok : toks) {
        QString tok = rawTok.trimmed();
        QString norm = tok.toLower();
        norm.remove(QRegularExpression(QStringLiteral("[^a-z0-9éèêëàâîïôùûüç]")));
        if (!norm.isEmpty() && norm == prev)
            continue;
        out << tok;
        if (!norm.isEmpty())
            prev = norm;
    }
    text = out.join(QLatin1Char(' ')).trimmed();
    return text;
}

/// Coupe les bords très silencieux (bruit de fond / démarrage micro) pour aider System.Speech.
static void trimSilenceMonoS16Edges(QByteArray &pcm, int sampleRate, qreal gate)
{
    const int n = pcm.size() / 2;
    if (n < sampleRate / 4 || sampleRate <= 0)
        return;
    const qint16 *d = reinterpret_cast<const qint16 *>(pcm.constData());
    const int hop = qMax(1, sampleRate / 50);
    int firstIdx = 0;
    for (int i = 0; i + hop <= n; i += hop) {
        if (rmsMonoS16Block(reinterpret_cast<const char *>(d + i), hop * 2) >= gate) {
            firstIdx = i;
            break;
        }
    }
    int lastIdx = n - 1;
    for (int j = ((n - 1) / hop) * hop; j >= 0; j -= hop) {
        const int len = qMin(hop, n - j);
        if (len < 2)
            continue;
        if (rmsMonoS16Block(reinterpret_cast<const char *>(d + j), len * 2) >= gate) {
            lastIdx = j + len - 1;
            break;
        }
    }
    if (lastIdx <= firstIdx)
        return;
    const int pad = qMin(n, sampleRate * 150 / 1000);
    firstIdx = qMax(0, firstIdx - pad);
    lastIdx = qMin(n - 1, lastIdx + pad);
    const int nOut = lastIdx - firstIdx + 1;
    if (nOut < n && nOut > 0)
        pcm = pcm.mid(firstIdx * 2, nOut * 2);
}

} // namespace

AssistantVoiceController::AssistantVoiceController(QObject *parent)
    : QObject(parent)
{
    m_pollTimer.setInterval(120);
    connect(&m_pollTimer, &QTimer::timeout, this, &AssistantVoiceController::onSilencePoll);
}

AssistantVoiceController::~AssistantVoiceController()
{
    closeMicrophone();
    if (m_sttProcess) {
        QObject::disconnect(m_sttProcess, nullptr, this, nullptr);
        m_sttProcess->kill();
        m_sttProcess->deleteLater();
        m_sttProcess = nullptr;
    }
}

void AssistantVoiceController::setState(VoiceState s)
{
    if (m_state == s)
        return;
    m_state = s;
    emit stateChanged(s);
}

void AssistantVoiceController::onTtsStarted()
{
    m_ttsExterneActif = true;
    if (m_state == Listening)
        stopListening(false);
    if (m_state == Processing && m_sttProcess) {
        QObject::disconnect(m_sttProcess, nullptr, this, nullptr);
        m_sttProcess->kill();
        m_sttProcess->deleteLater();
        m_sttProcess = nullptr;
        setState(Idle);
    }
    setState(Speaking);
}

void AssistantVoiceController::onTtsEnded()
{
    m_ttsExterneActif = false;
    if (m_state == Speaking)
        setState(Idle);
}

void AssistantVoiceController::toggleListening()
{
    if (m_state == Speaking || m_ttsExterneActif)
        return;
    if (m_state == Processing)
        return;
    if (m_state == Listening) {
        // Re-clic micro = terminer la capture et lancer la transcription.
        stopListening(true);
        return;
    }
    startListening();
}

void AssistantVoiceController::startListening()
{
    if (m_state == Speaking || m_ttsExterneActif)
        return;
    if (m_state == Processing)
        return;
    if (m_state == Listening)
        return;

    if (!openMicrophone()) {
        emit voiceError(QStringLiteral("no_device"),
                        QString::fromUtf8(u8"Micro non détecté ou format audio non supporté. Vérifiez Paramètres Windows > Son > Entrée."));
        setState(Idle);
        return;
    }

    resetCapture();
    setState(Listening);
    m_listenElapsed.start();
    m_pollTimer.start();
}

void AssistantVoiceController::stopListening(bool processAudio)
{
    if (m_state != Listening)
        return;
    finalizeListening(processAudio);
}

void AssistantVoiceController::tryStartListeningAuto()
{
    if (m_ttsExterneActif || m_state == Speaking)
        return;
    if (m_state != Idle)
        return;
    startListening();
}

void AssistantVoiceController::interruptAll()
{
    m_ttsExterneActif = false;
    m_pollTimer.stop();
    closeMicrophone();
    if (m_sttProcess) {
        QObject::disconnect(m_sttProcess, nullptr, this, nullptr);
        m_sttProcess->kill();
        m_sttProcess->deleteLater();
        m_sttProcess = nullptr;
    }
    m_pcmBuffer.clear();
    m_sawVoice = false;
    m_lastVoiceMs = 0;
    m_lastLevel = 0;
    setState(Idle);
    emit audioLevelChanged(0);
}

void AssistantVoiceController::resetCapture()
{
    m_pcmBuffer.clear();
    m_lastVoiceMs = 0;
    m_sawVoice = false;
    m_lastLevel = 0;
}

bool AssistantVoiceController::openMicrophone()
{
    closeMicrophone();

    QAudioDevice dev = QMediaDevices::defaultAudioInput();
    if (dev.isNull()) {
        qWarning() << "AssistantVoice: no input device";
        return false;
    }

    static const int preferredRates[] = {48000, 44100, 32000, 24000, 16000};
    QAudioFormat fmt;
    bool found = false;
    for (int rate : preferredRates) {
        QAudioFormat want;
        want.setSampleRate(rate);
        want.setChannelCount(1);
        want.setSampleFormat(QAudioFormat::Int16);
        if (dev.isFormatSupported(want)) {
            fmt = want;
            found = true;
            break;
        }
    }
    if (!found) {
        fmt = dev.preferredFormat();
        if (fmt.sampleRate() <= 0 || fmt.channelCount() <= 0) {
            qWarning() << "AssistantVoice: bad preferred format";
            return false;
        }
    }

    m_sampleRate = fmt.sampleRate();
    m_captureFormat = fmt;
    m_audioSource = new QAudioSource(dev, fmt, this);
    m_io = m_audioSource->start();
    if (!m_io) {
        delete m_audioSource;
        m_audioSource = nullptr;
        return false;
    }
    connect(m_io, &QIODevice::readyRead, this, &AssistantVoiceController::onInputReadyRead);
    return true;
}

void AssistantVoiceController::closeMicrophone()
{
    m_pollTimer.stop();
    if (m_audioSource) {
        m_audioSource->stop();
        if (m_io)
            disconnect(m_io, nullptr, this, nullptr);
        m_audioSource->deleteLater();
        m_audioSource = nullptr;
        m_io = nullptr;
    }
}

qreal AssistantVoiceController::rmsNormalized(const char *data, int nbytes)
{
    if (nbytes < 2)
        return 0;
    /// Traite mono Int16 ; si autre format, approximation via échantillonnage court.
    const int nSamp = nbytes / 2;
    if (nSamp <= 0)
        return 0;
    const qint16 *s = reinterpret_cast<const qint16 *>(data);
    double acc = 0;
    for (int i = 0; i < nSamp; ++i) {
        const double v = s[i] / 32768.0;
        acc += v * v;
    }
    return qMin(1.0, sqrt(acc / nSamp));
}

void AssistantVoiceController::onInputReadyRead()
{
    if (m_state != Listening || !m_io)
        return;

    const QByteArray chunk = m_io->readAll();
    if (chunk.isEmpty())
        return;

    const QAudioFormat fmt = m_captureFormat;
    QByteArray monoS16;

    if (fmt.sampleFormat() == QAudioFormat::Float) {
        const int ch = qMax(1, fmt.channelCount());
        const int nf = chunk.size() / (int(sizeof(float)) * ch);
        monoS16.resize(nf * 2);
        qint16 *out = reinterpret_cast<qint16 *>(monoS16.data());
        const float *f = reinterpret_cast<const float *>(chunk.constData());
        for (int i = 0; i < nf; ++i) {
            float acc = 0;
            for (int c = 0; c < ch; ++c)
                acc += f[i * ch + c];
            acc /= float(ch);
            const float x = qBound(-1.f, acc, 1.f);
            out[i] = qint16(x * 32767.f);
        }
    } else if (fmt.sampleFormat() == QAudioFormat::Int16) {
        const int ch = qMax(1, fmt.channelCount());
        const int nFrames = chunk.size() / (2 * ch);
        monoS16.resize(nFrames * 2);
        qint16 *out = reinterpret_cast<qint16 *>(monoS16.data());
        const qint16 *in = reinterpret_cast<const qint16 *>(chunk.constData());
        if (ch == 1) {
            memcpy(out, in, size_t(nFrames * 2));
        } else {
            for (int i = 0; i < nFrames; ++i) {
                qint32 acc = 0;
                for (int c = 0; c < ch; ++c)
                    acc += in[i * ch + c];
                out[i] = qint16(acc / ch);
            }
        }
    } else {
        emit voiceError(QStringLiteral("no_device"),
                        QString::fromUtf8(u8"Format micro non géré (utilisez entrée 16 bits ou flottant)."));
        stopListening(false);
        return;
    }

    const qreal r = rmsNormalized(monoS16.constData(), monoS16.size());
    m_lastLevel = r;
    emit audioLevelChanged(r);

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (r >= RMS_VOICE_GATE) {
        m_sawVoice = true;
        m_lastVoiceMs = now;
    }

    m_pcmBuffer.append(monoS16);
}

/// Rééchantillonnage mono S16 LE vers 16 kHz (linear) — System.Speech attend surtout du 16 kHz 16 bits mono.
static QByteArray resampleMonoS16To16kHz(const QByteArray &pcmIn, int fromRate)
{
    if (pcmIn.isEmpty() || fromRate <= 0)
        return pcmIn;
    constexpr int kTarget = 16000;
    if (fromRate == kTarget)
        return pcmIn;

    const int nIn = pcmIn.size() / 2;
    if (nIn <= 1)
        return pcmIn;
    const qint16 *in = reinterpret_cast<const qint16 *>(pcmIn.constData());
    const qint64 nOut64 = (qint64(nIn) * kTarget + fromRate / 2) / fromRate;
    const int nOut = int(qBound(qint64(1), nOut64, qint64(10000000)));
    QByteArray out;
    out.resize(nOut * 2);
    qint16 *o = reinterpret_cast<qint16 *>(out.data());
    for (int j = 0; j < nOut; ++j) {
        const double srcPos = (j + 0.5) * double(fromRate) / double(kTarget) - 0.5;
        const int i0 = int(floor(srcPos));
        const int i1 = i0 + 1;
        const double t = srcPos - double(i0);
        const qint16 v0 = (i0 >= 0 && i0 < nIn) ? in[i0] : 0;
        const qint16 v1 = (i1 >= 0 && i1 < nIn) ? in[i1] : 0;
        const double v = (1.0 - t) * double(v0) + t * double(v1);
        o[j] = qint16(qBound(-32768.0, v, 32767.0));
    }
    return out;
}

static void writePcmWav16File(const QString &path, const QByteArray &pcmS16leMono, int sampleRate)
{
    const quint32 dataSize = quint32(pcmS16leMono.size());
    const quint32 riffChunkSize = 36 + dataSize;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    auto w4 = [&](const char *c) { f.write(c, 4); };

    w4("RIFF");
    {
        QDataStream ds(&f);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << quint32(riffChunkSize);
    }
    w4("WAVE");
    w4("fmt ");
    {
        QDataStream ds(&f);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << quint32(16);
        ds << quint16(1);
        ds << quint16(1);
        ds << quint32(quint32(sampleRate));
        const quint32 br = quint32(sampleRate * 2);
        ds << br;
        ds << quint16(2);
        ds << quint16(16);
    }
    w4("data");
    {
        QDataStream ds(&f);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << dataSize;
    }
    f.write(pcmS16leMono);
    f.close();
}

void AssistantVoiceController::onSilencePoll()
{
    if (m_state != Listening)
        return;

    if (m_listenElapsed.hasExpired(MAX_LISTEN_MS)) {
        finalizeListening(m_sawVoice);
        if (!m_sawVoice) {
            emit voiceError(QStringLiteral("no_voice"),
                            QString::fromUtf8(u8"Aucune voix détectée (parlez plus fort ou rapprochez le micro)."));
        }
        return;
    }

    if (!m_sawVoice)
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastVoiceMs > 0 && (now - m_lastVoiceMs) >= SILENCE_MS)
        finalizeListening(true);
}

void AssistantVoiceController::finalizeListening(bool processAudio)
{
    m_pollTimer.stop();
    /// Hors « Listening » avant fermeture du device : évite readyRead en file d’attente pendant la destruction.
    if (!processAudio) {
        setState(Idle);
        closeMicrophone();
        emit audioLevelChanged(0);
        return;
    }
    setState(Processing);
    closeMicrophone();

    trimSilenceMonoS16Edges(m_pcmBuffer, m_sampleRate, RMS_VOICE_GATE * 0.65);
    QByteArray pcm16k = resampleMonoS16To16kHz(m_pcmBuffer, m_sampleRate);
    removeDcAndNoiseFloorMonoS16(pcm16k, RMS_VOICE_GATE * 0.55);
    normalizeMonoS16Agc(pcm16k, 0.17, 4.0);
    if (!m_sawVoice || pcm16k.size() < AssistantVoiceController::MIN_VOICE_BYTES_16K) {
        setState(Idle);
        emit audioLevelChanged(0);
        emit voiceError(QStringLiteral("too_quiet"),
                        QString::fromUtf8(u8"Niveau audio trop faible ou trop court. Réessayez en parlant plus fort."));
        return;
    }

    const QString wav = tempWavPath();
    writePcmWav16File(wav, pcm16k, 16000);
    m_pcmBuffer.clear();

    emit audioLevelChanged(0);
    runSttOnWav(wav);
}

void AssistantVoiceController::writePcmWav16(const QString &path, const QByteArray &pcmS16leMono, int sampleRate)
{
    writePcmWav16File(path, pcmS16leMono, sampleRate);
}

void AssistantVoiceController::runSttOnWav(const QString &wavPath)
{
    if (m_sttProcess) {
        QObject::disconnect(m_sttProcess, nullptr, this, nullptr);
        m_sttProcess->kill();
        m_sttProcess->deleteLater();
        m_sttProcess = nullptr;
    }

    const auto onTextReady = [this](const QString &rawText) {
        const QString text = postCorrectRecognizedText(rawText);
        if (text.trimmed().isEmpty()) {
            emit voiceError(QStringLiteral("no_text"),
                            QString::fromUtf8(u8"Aucune voix détectée (texte vide après reconnaissance)."));
            setState(Idle);
            return;
        }
        emit textRecognized(text);
        setState(Idle);
    };

    std::function<void()> startPowerShell;
    startPowerShell = [this, wavPath, onTextReady]() {
        m_sttProcess = new QProcess(this);
        QString p = QDir::toNativeSeparators(wavPath);
        p.replace('\\', QStringLiteral("\\\\"));
        p.replace(QLatin1Char('"'), QStringLiteral("\\\""));
        const QString cmd = QStringLiteral(
            "$ErrorActionPreference='Stop';"
            "try{"
            "Add-Type -AssemblyName System.Speech;"
            "$p=\"%1\";"
            "$fs=[System.IO.FileStream]::new($p,[System.IO.FileMode]::Open,[System.IO.FileAccess]::Read,[System.IO.FileShare]::Read);"
            "try{"
            "$all=@([System.Speech.Recognition.SpeechRecognitionEngine]::InstalledRecognizers());"
            "if($all.Count -lt 1){[Console]::Error.WriteLine('NO_ENGINE');exit 2}"
            "$ri=$all|Where-Object{$_.Culture.Name-eq'fr-FR'}|Select-Object -First 1;"
            "if(-not $ri){$ri=$all|Select-Object -First 1}"
            "$rec=New-Object System.Speech.Recognition.SpeechRecognitionEngine($ri);"
            "try{$rec.MaxAlternates=5}catch{}"
            "$rec.SetInputToWaveStream($fs);"
            "$rec.InitialSilenceTimeout=[TimeSpan]::FromSeconds(20);"
            "$rec.BabbleTimeout=[TimeSpan]::FromSeconds(10);"
            "$rec.EndSilenceTimeout=[TimeSpan]::FromMilliseconds(700);"
            "try{$rec.EndSilenceTimeoutAmbiguous=[TimeSpan]::FromMilliseconds(950)}catch{}"
            "$dg=New-Object System.Speech.Recognition.DictationGrammar;"
            "$rec.LoadGrammar($dg);"
            "$res=$rec.Recognize();"
            "[Console]::OutputEncoding=[System.Text.Encoding]::UTF8;"
            "if($res -and $res.Text){Write-Output ($res.Text.Trim())}else{[Console]::Error.WriteLine('NO_TEXT');exit 3}"
            "}finally{$fs.Close()}"
            "}catch{[Console]::Error.WriteLine($_.Exception.Message);exit 1}").arg(p);

        m_sttProcess->start(QStringLiteral("powershell"),
                            QStringList() << QStringLiteral("-NoProfile") << QStringLiteral("-ExecutionPolicy")
                                          << QStringLiteral("Bypass") << QStringLiteral("-Command") << cmd);

        QPointer<AssistantVoiceController> selfGuard(this);
        connect(m_sttProcess, &QProcess::finished, this,
                [this, selfGuard, wavPath, onTextReady](int exitCode, QProcess::ExitStatus) {
                    QFile::remove(wavPath);
                    if (!selfGuard)
                        return;
                    QProcess *proc = m_sttProcess;
                    m_sttProcess = nullptr;
                    if (!proc)
                        return;
                    const QByteArray out = proc->readAllStandardOutput().trimmed();
                    const QByteArray err = proc->readAllStandardError().trimmed();
                    proc->deleteLater();

                    if (m_ttsExterneActif) {
                        setState(Speaking);
                        return;
                    }
                    if (exitCode != 0 || out.isEmpty()) {
                        QString msg;
                        if (err.contains("NO_TEXT"))
                            msg = QString::fromUtf8(
                                u8"Aucune voix détectée dans l’enregistrement (réessayez, phrase plus nette).");
                        else if (err.contains("NO_ENGINE"))
                            msg = QString::fromUtf8(
                                u8"Reconnaissance vocale Windows indisponible. Installez la langue / reconnaissance dans Paramètres.");
                        else if (!err.isEmpty())
                            msg = QString::fromUtf8(err);
                        else
                            msg = QString::fromUtf8(u8"Échec de la reconnaissance (code %1).").arg(exitCode);
                        emit voiceError(QStringLiteral("stt_error"), msg);
                        setState(Idle);
                        return;
                    }
                    onTextReady(QString::fromUtf8(out));
                });
    };

    auto resolveTranscribeScript = []() -> QString {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString cwd = QDir::currentPath();
        const QStringList candidates = {
            QDir(appDir).filePath(QStringLiteral("assistant_vocal.py")),
            QDir(cwd).filePath(QStringLiteral("assistant_vocal.py")),
            QDir(appDir).filePath(QStringLiteral("../assistant_vocal.py")),
            QDir(appDir).filePath(QStringLiteral("../../assistant_vocal.py")),
            QDir(appDir).filePath(QStringLiteral("../../../assistant_vocal.py"))
        };
        for (const QString &p : candidates) {
            if (QFileInfo::exists(p))
                return QDir::toNativeSeparators(QFileInfo(p).absoluteFilePath());
        }
        return QString();
    };

    const QString scriptPath = resolveTranscribeScript();
    qDebug() << "[STT] script assistant_vocal.py =" << scriptPath;

    auto startPython = [this, wavPath, onTextReady, startPowerShell, scriptPath](const QString &program,
                                                                                    const QStringList &prefixArgs) -> bool {
        if (scriptPath.isEmpty()) {
            qDebug() << "[STT] assistant_vocal.py introuvable, fallback Windows.";
            return false;
        }
        m_sttProcess = new QProcess(this);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
        env.insert(QStringLiteral("PYTHONUTF8"), QStringLiteral("1"));
        m_sttProcess->setProcessEnvironment(env);

        QStringList args = prefixArgs;
        args << scriptPath
             << QStringLiteral("--transcribe-file")
             << QDir::toNativeSeparators(wavPath)
             << QStringLiteral("--model")
             << QStringLiteral("base")
             << QStringLiteral("--compute-type")
             << QStringLiteral("int8");
        qDebug() << "[STT] lancement python:" << program << args;
        m_sttProcess->start(program, args);
        if (!m_sttProcess->waitForStarted(1200)) {
            qDebug() << "[STT] echec start python:" << program << m_sttProcess->errorString();
            m_sttProcess->deleteLater();
            m_sttProcess = nullptr;
            return false;
        }

        QPointer<AssistantVoiceController> selfGuard(this);
        connect(m_sttProcess, &QProcess::finished, this,
                [this, selfGuard, wavPath, onTextReady, startPowerShell](int exitCode, QProcess::ExitStatus) {
                    if (!selfGuard)
                        return;
                    QProcess *proc = m_sttProcess;
                    m_sttProcess = nullptr;
                    if (!proc)
                        return;
                    const QByteArray out = proc->readAllStandardOutput().trimmed();
                    const QByteArray err = proc->readAllStandardError().trimmed();
                    proc->deleteLater();

                    if (m_ttsExterneActif) {
                        QFile::remove(wavPath);
                        setState(Speaking);
                        return;
                    }
                    if (exitCode == 0 && !out.isEmpty()) {
                        qDebug() << "[STT] python ok:" << QString::fromUtf8(out);
                        QFile::remove(wavPath);
                        onTextReady(QString::fromUtf8(out));
                        return;
                    }
                    qDebug() << "[STT] python fail, fallback Windows. exitCode=" << exitCode
                             << "stderr=" << QString::fromUtf8(err);
                    // Fallback robuste : STT Windows si Python indisponible / module non installé.
                    startPowerShell();
                });

        // Anti-blocage: modèle base + 1er chargement disque peut dépasser 25s sur CPU lent.
        QTimer::singleShot(55000, this, [this, selfGuard, startPowerShell]() {
            if (!selfGuard || !m_sttProcess)
                return;
            if (m_sttProcess->state() == QProcess::Running) {
                qDebug() << "[STT] timeout python, fallback Windows.";
                QObject::disconnect(m_sttProcess, nullptr, this, nullptr);
                m_sttProcess->kill();
                m_sttProcess->deleteLater();
                m_sttProcess = nullptr;
                startPowerShell();
            }
        });
        return true;
    };

    if (startPython(QStringLiteral("py"), QStringList{QStringLiteral("-3")}))
        return;
    if (startPython(QStringLiteral("python"), QStringList{}))
        return;
    startPowerShell();
}
