#pragma once
#include <QObject>

class AudioConverter : public QObject
{
    Q_OBJECT
public:
    AudioConverter();
    ~AudioConverter();

    void setLanguage(const QString& lang);

public slots:
    void convert(const QByteArray& data);

signals:
    void completed(const QString& result);

private:
    void clear();

private:
    class QThread* thread = nullptr;
    class VoskModel* model = nullptr;
    class VoskRecognizer* recognizer = nullptr;
    QString language;
    const float sampleRate = 16000.f;
};
