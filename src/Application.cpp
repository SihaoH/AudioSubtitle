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
#include <QTimer>

Application::Application(int argc, char **argv)
    : QApplication(argc, argv)
{
    Logger::instance()->init("audio_subtitle.log");
    loadConfig();
}

Application::~Application()
{
    delete mainWindow;
    delete audioConverter;
    delete textTranslator;
}

void Application::initLater()
{
    QTimer::singleShot(0, this, &Application::init);
}

void Application::loadConfig()
{
    QFile config_file("config.json");
    if (!config_file.open(QIODevice::ReadOnly)) {
        LOG(err) << "无法打开配置文件：" << config_file.errorString();
        return;
    }

    auto doc = QJsonDocument::fromJson(config_file.readAll());
    if (doc.isNull()) {
        LOG(err) << "配置文件格式错误";
        return;
    }

    auto config = doc.object();
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

    // 读取模型配置文件
    QFile manifest_file("./models/manifest");
    if (!manifest_file.open(QIODevice::ReadOnly)) {
        LOG(err) << "无法打开模型配置文件：" << manifest_file.errorString();
        return;
    }

    auto manifest_doc = QJsonDocument::fromJson(manifest_file.readAll());
    if (manifest_doc.isNull()) {
        LOG(err) << "模型配置文件格式错误";
        return;
    }
    auto manifest = manifest_doc.object();
    for (auto it = manifest.begin(); it != manifest.end(); ++it) {
        langMap.insert(it.key(), it.value().toString());
    }
}

void Application::saveConfig()
{
    QFile config_file("config.json");
    if (!config_file.open(QIODevice::WriteOnly)) {
        LOG(err) << "无法打开配置文件：" << config_file.errorString();
        return;
    }

    QJsonObject config;
    config["window"] = QJsonArray{windowX, windowY};
    config["duration"] = duration;
    config["original"] = originalLang;
    config["translation"] = translationLang;

    QJsonDocument doc(config);
    config_file.write(doc.toJson());
}

void Application::init()
{
    mainWindow = new SubtitleWidget();
    mainWindow->show();
    if (windowX > 0 && windowY > 0) {
        mainWindow->move(windowX - mainWindow->width()*0.5f, windowY);
    }
    mainWindow->setLangMap(langMap);
    mainWindow->setLanguage(originalLang, translationLang);
    mainWindow->setDuration(duration);
    connect(mainWindow, &SubtitleWidget::positionChanged, this, &Application::onWindowMoved);
    connect(mainWindow, &SubtitleWidget::originalLangChanged, this, &Application::onOriginalLangChanged);
    connect(mainWindow, &SubtitleWidget::translatedLangChanged, this, &Application::onTranslatedLangChanged);
    connect(mainWindow, &SubtitleWidget::durationChanged, this, &Application::onDurationChanged);

    audioCapturer = new AudioCapturer(this);
    audioConverter = new AudioConverter();
    textTranslator = new TextTranslator();

    // 使用标志值，让转换和翻译只响应最新的（中间来不及处理的会丢失）
    connect(audioCapturer, &AudioCapturer::readReady, this, &Application::onAudioReady);
    connect(this, &Application::aboutToConvert, audioConverter, &AudioConverter::convert, Qt::QueuedConnection);
    connect(audioConverter, &AudioConverter::completed, mainWindow, &SubtitleWidget::setOriginalText, Qt::QueuedConnection);
    connect(audioConverter, &AudioConverter::completed, this, &Application::onConvertCompleted, Qt::QueuedConnection);
    connect(this, &Application::aboutToTranslate, textTranslator, &TextTranslator::translate, Qt::QueuedConnection);
    connect(textTranslator, &TextTranslator::completed, mainWindow, &SubtitleWidget::setTranslatedText, Qt::QueuedConnection);
    connect(textTranslator, &TextTranslator::completed, this, &Application::onTranslateCompleted, Qt::QueuedConnection);

    audioConverter->setLanguage(originalLang);
    textTranslator->setLanguage(originalLang, translationLang);
    audioCapturer->start(duration.toFloat() * 1000);
    
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
