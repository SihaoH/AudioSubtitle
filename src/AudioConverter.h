#pragma once
#include <QObject>

class AudioConverter : public QObject
{
    Q_OBJECT
public:
    AudioConverter(QObject* parent = nullptr);
    ~AudioConverter();

    void setLanguage(const QString& lang);

public slots:
    void convert(const QByteArray& data);

signals:
    void completed(const QString& result);

private:
    class QThread* thread = nullptr;
    class VoskModel* model = nullptr;
    class VoskRecognizer* recognizer = nullptr;
    float sampleRate = 16000.f;
};
