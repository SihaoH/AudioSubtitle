#include "AudioConverter.h"
#include "Logger.h"
#include "vosk_api.h"
#include <QDir>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

AudioConverter::AudioConverter()
    : QObject(nullptr)
    , thread(new QThread(this))
{
    thread->setObjectName("AudioConverterThread");
    moveToThread(thread);
}

void AudioConverter::setLanguage(const QString& lang)
{
    LOG(info) << "设置语音识别的语言: " << lang;
    language = lang;
    cleanUp();
    const auto model_path = QString("models/vosk-%1").arg(lang);
    if (QDir(model_path).exists()) {
        model = vosk_model_new(model_path.toUtf8().constData());
        recognizer = vosk_recognizer_new(model, sampleRate);
        LOG(info) << "语音模型加载成功: " << model_path;
        thread->start();
    } else {
        LOG(err) << "语音模型不存在: " << model_path;
    }
}

void AudioConverter::cleanUp()
{
    thread->quit();
    thread->wait();
    if (model) {
        vosk_model_free(model);
        model = nullptr;
    }
    if (recognizer) {
        vosk_recognizer_free(recognizer);
        recognizer = nullptr;
    }
}

AudioConverter::~AudioConverter()
{
    cleanUp();
}

void AudioConverter::convert(const QByteArray &data)
{
    if (data.isEmpty()) {
        emit completed(QString());
        return;
    }
    vosk_recognizer_reset(recognizer);
    vosk_recognizer_accept_waveform(recognizer, data.data(), data.size());
    QByteArray ret = vosk_recognizer_final_result(recognizer);
    auto ret_str = QJsonDocument::fromJson(ret).object().value("text").toString().trimmed();
    if (language != "en") {
        ret_str.remove(QRegularExpression("\\s+"));
    }
    emit completed(ret_str);
}
