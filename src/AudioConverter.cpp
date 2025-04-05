#include "AudioConverter.h"
#include "vosk_api.h"
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

AudioConverter::AudioConverter(const QString& model_path, float sample_rate, QObject* parent)
    : QObject(parent)
    , thread(new QThread(this))
    , sampleRate(sample_rate)
{
    thread->setObjectName("AudioConverterThread");
    moveToThread(thread);
    model = vosk_model_new(model_path.toUtf8().constData());

    thread->start();
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
