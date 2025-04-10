#pragma once

#include <QApplication>
#include <QMap>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int argc, char **argv);
    ~Application();

    void initLater();

public slots:
    void onWindowMoved(const QPoint& pos);
    void onOriginalLangChanged(const QString& text);
    void onTranslatedLangChanged(const QString& text);
    void onDurationChanged(const QString& text);

private slots:
    void init();
    
private:
    void loadConfig();
    void saveConfig();

private:
    int windowX = 500;  // 默认值
    int windowY = 500; // 默认值
    QString duration = "2.0";
    QString originalLang = "ja";    // 默认值
    QString translationLang = "zh"; // 默认值
    QMap<QString, QString> langMap;
    
    class SubtitleWidget* mainWindow = nullptr;
    class AudioCapturer* audioCapturer = nullptr;
    class AudioConverter* audioConverter = nullptr;
    class TextTranslator* textTranslator = nullptr;
}; 