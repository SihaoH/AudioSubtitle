#pragma once
#include <QString>

class AudioConverter
{
public:
    AudioConverter(const QString& model_path, float sample_rate);
    ~AudioConverter();

    QString convert(const QByteArray& data);

private:
    class VoskModel* model = nullptr;
    class VoskRecognizer* recognizer = nullptr;
    float sampleRate = 16000.f;
};
