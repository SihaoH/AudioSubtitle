#pragma once
#include <QObject>

class AudioConverter : public QObject
{
    Q_OBJECT
public:
    AudioConverter(const QString& model_path, float sample_rate, QObject* parent = nullptr);
    ~AudioConverter();

public slots:
    void convert(const QByteArray& data);

signals:
    void completed(const QString& result);

private:
    class QThread* thread = nullptr;
    class VoskModel* model = nullptr;
    class VoskRecognizer* recognizer = nullptr;
    struct whisper_context* wctx = nullptr;
    float sampleRate = 16000.f;
};
