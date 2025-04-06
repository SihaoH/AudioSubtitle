#include "Application.h"
#include "AudioCapturer.h"
#include "AudioConverter.h"
#include "TextTranslator.h"
#include "SubtitleWidget.h"
#include "Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

Application::Application(int argc, char **argv)
    : QApplication(argc, argv)
{
    Logger::instance()->init("audio_subtitle.log");
    loadConfig();
}

Application::~Application()
{
    delete mainWindow;
}

void Application::loadConfig()
{
    QFile configFile("config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        LOG(err) << "无法打开配置文件：" << configFile.errorString();
        return;
    }

    QByteArray configData = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(configData);
    
    if (doc.isNull()) {
        LOG(err) << "配置文件格式错误";
        return;
    }

    QJsonObject config = doc.object();
    
    // 读取窗口大小
    if (config.contains("window")) {
        auto size = config["window"].toArray();
        windowX = size[0].toInt();
        windowY = size[1].toInt();
    }
    
    // 读取持续时间
    if (config.contains("duration")) {
        duration = config["duration"].toString(); // 转换为毫秒
    }
    
    // 读取语言设置
    originalLang = config["original"].toString();
    translationLang = config["translation"].toString();
}

void Application::saveConfig()
{
    QFile configFile("config.json");
    if (!configFile.open(QIODevice::WriteOnly)) {
        LOG(err) << "无法打开配置文件：" << configFile.errorString();
        return;
    }

    QJsonObject config;
    config["window"] = QJsonArray{windowX, windowY};
    config["duration"] = duration;
    config["original"] = originalLang;
    config["translation"] = translationLang;

    QJsonDocument doc(config);
    configFile.write(doc.toJson());
}

void Application::init()
{
    mainWindow = new SubtitleWidget();
    if (windowX > 0 && windowY > 0) {
        mainWindow->move(windowX, windowY);
    }
    mainWindow->setLanguage(originalLang, translationLang);
    mainWindow->setDuration(duration);
    connect(mainWindow, &SubtitleWidget::positionChanged, this, &Application::onWindowMoved);
    connect(mainWindow, &SubtitleWidget::originalLangChanged, this, &Application::onOriginalLangChanged);
    connect(mainWindow, &SubtitleWidget::translatedLangChanged, this, &Application::onTranslatedLangChanged);
    connect(mainWindow, &SubtitleWidget::durationChanged, this, &Application::onDurationChanged);

    audioCapturer = new AudioCapturer(this);
    audioConverter = new AudioConverter(this);
    textTranslator = new TextTranslator(this);

    connect(audioCapturer, &AudioCapturer::readReady, this, &Application::onAudioReady);
    connect(this, &Application::aboutToConvert, audioConverter, &AudioConverter::convert);
    connect(audioConverter, &AudioConverter::completed, mainWindow, &SubtitleWidget::setOriginalText);
    connect(audioConverter, &AudioConverter::completed, this, &Application::onConvertCompleted);
    connect(this, &Application::aboutToTranslate, textTranslator, &TextTranslator::translate);
    connect(textTranslator, &TextTranslator::completed, mainWindow, &SubtitleWidget::setTranslatedText);
    connect(textTranslator, &TextTranslator::completed, this, &Application::onTranslateCompleted);

    audioConverter->setLanguage(originalLang);
    textTranslator->setLanguage(originalLang, translationLang);
    audioCapturer->start(duration.toFloat() * 1000);
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

void Application::onWindowMoved(const QPoint& pos)
{
    windowX = pos.x();
    windowY = pos.y();
    saveConfig();
}

void Application::onOriginalLangChanged(const QString& text)
{
    originalLang = text;
    audioConverter->setLanguage(originalLang);
    textTranslator->setLanguage(originalLang, translationLang);
    saveConfig();
}

void Application::onTranslatedLangChanged(const QString& text)
{
    translationLang = text;
    textTranslator->setLanguage(originalLang, translationLang);
    saveConfig();
}

void Application::onDurationChanged(const QString& text)
{
    duration = text;
    audioCapturer->stop();
    audioCapturer->start(duration.toFloat() * 1000);
    saveConfig();
}
