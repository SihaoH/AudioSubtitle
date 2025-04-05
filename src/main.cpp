#include "AudioCapturer.h"
#include "AudioConverter.h"
#include "Logger.h"
#include "TextTranslator.h"
#include "SubtitleWidget.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    Logger::instance()->init("audio_subtitle.log");
    QApplication app(argc, argv);

    SubtitleWidget widget;
    widget.show();

    float sample_rate = 16000;
    AudioCapturer capture(sample_rate);
    capture.start(2000);
    AudioConverter converter("vosk-model-small-cn-0.22", sample_rate);
    TextTranslator translator("translate-zh_en-1_9");
    translator.setLanguage("zh", "en");
    QObject::connect(&capture, &AudioCapturer::readReady, &converter, &AudioConverter::convert);
    QObject::connect(&converter, &AudioConverter::completed, &widget, &SubtitleWidget::setOriginalText);
    QObject::connect(&converter, &AudioConverter::completed, &translator, &TextTranslator::translate);
    QObject::connect(&translator, &TextTranslator::completed, &widget, &SubtitleWidget::setTranslatedText);

    return app.exec();
}
