#include "AudioCapturer.h"
#include "AudioConverter.h"
#include "Logger.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    Logger::instance()->init("audio_subtitle.log");
    QCoreApplication app(argc, argv);
    
    AudioCapturer capture;
    capture.start(3000);
    AudioConverter converter("vosk-model-small-cn-0.22", 11025);
    QObject::connect(&capture, &AudioCapturer::readReady, [&](QByteArray data) {
        QString result = converter.convert(data);
        LOG(info) << result;
    });

    return app.exec();
}
