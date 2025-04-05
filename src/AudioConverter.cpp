#include "AudioConverter.h"
#include "Logger.h"
#include "vosk_api.h"
#include <QDir>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

AudioConverter::AudioConverter(QObject* parent)
    : QObject(parent)
    , thread(new QThread(this))
{
    thread->setObjectName("AudioConverterThread");
    moveToThread(thread);
}

void AudioConverter::setLanguage(const QString& lang)
{
    const auto model_path = QString("models/vosk-%1").arg(lang);
    if (QDir(model_path).exists()) {
        if (model) {
            vosk_model_free(model);
        }
        model = vosk_model_new(model_path.toUtf8().constData());
    } else {
        LOG(err) << "语音模型不存在: " << model_path;
    }
}

void AudioConverter::start()
{
    if (model != nullptr) {
        thread->start();
    }
}

AudioConverter::~AudioConverter()
{
    if (recognizer) {
        vosk_recognizer_free(recognizer);
    }
    vosk_model_free(model);
    thread->quit();
}

void AudioConverter::convert(const QByteArray &data)
{
    if (data.isEmpty()) {
        emit completed(QString());
        return;
    }
    if (recognizer == nullptr) {
        recognizer = vosk_recognizer_new(model, sampleRate);
    }
    vosk_recognizer_accept_waveform(recognizer, data.data(), data.size());
    QByteArray ret = vosk_recognizer_final_result(recognizer);
    vosk_recognizer_free(recognizer);
    recognizer = nullptr;
    emit completed(QJsonDocument::fromJson(ret).object().value("text").toString().trimmed().remove(QRegularExpression("\\s+")));
}
