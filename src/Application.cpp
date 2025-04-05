#include "Application.h"
#include "AudioCapturer.h"
#include "AudioConverter.h"
#include "TextTranslator.h"
#include "SubtitleWidget.h"
#include "Logger.h"

Application::Application(int argc, char **argv)
    : QApplication(argc, argv)
{
    Logger::instance()->init("audio_subtitle.log");
}

Application::~Application()
{
    delete mainWindow;
}

void Application::init()
{
    mainWindow = new SubtitleWidget();
    audioCapturer = new AudioCapturer(this);
    audioConverter = new AudioConverter(this);
    textTranslator = new TextTranslator(this);

    audioConverter->setLanguage("ja");
    textTranslator->setLanguage("ja", "zh");

    connect(audioCapturer, &AudioCapturer::readReady, this, &Application::onAudioReady);
    connect(this, &Application::aboutToConvert, audioConverter, &AudioConverter::convert);
    connect(audioConverter, &AudioConverter::completed, mainWindow, &SubtitleWidget::setOriginalText);
    connect(audioConverter, &AudioConverter::completed, this, &Application::onConvertCompleted);
    connect(this, &Application::aboutToTranslate, textTranslator, &TextTranslator::translate);
    connect(textTranslator, &TextTranslator::completed, mainWindow, &SubtitleWidget::setTranslatedText);
    connect(textTranslator, &TextTranslator::completed, this, &Application::onTranslateCompleted);

    audioCapturer->start(2000);
    audioConverter->start();
    textTranslator->start();
    mainWindow->show();
}

void Application::onAudioReady(const QByteArray& data)
{
    if (isConverting == false) {
        isConverting = true;
        emit aboutToConvert(data);
    }
}

void Application::onConvertCompleted(const QString& text)
{
    isConverting = false;
    if (isTranslating == false) {
        isTranslating = true;
        emit aboutToTranslate(text);
    }
}

void Application::onTranslateCompleted()
{
    isTranslating = false;
}
