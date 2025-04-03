#pragma once
#include <QObject>

class AudioCapturer : public QObject
{
    Q_OBJECT
public:
    AudioCapturer(QObject *parent = nullptr);
    ~AudioCapturer();

    void start(int msec);
    void stop();

private slots:
    void onCaptured();

signals:
    void readReady(QByteArray data);

private:
    class AudioCapturerPrivate* d;
};

