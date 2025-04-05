#pragma once

#include <QApplication>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int argc, char **argv);
    ~Application();

    void init();

signals:
    void aboutToConvert(const QByteArray& data);
    void aboutToTranslate(const QString& text);

public slots:
    void onAudioReady(const QByteArray& data);
    void onConvertCompleted(const QString& text);
    void onTranslateCompleted();
    
private:
    class SubtitleWidget* mainWindow = nullptr;
    class AudioCapturer* audioCapturer = nullptr;
    class AudioConverter* audioConverter = nullptr;
    class TextTranslator* textTranslator = nullptr;
    bool isConverting = false;
    bool isTranslating = false;
}; 