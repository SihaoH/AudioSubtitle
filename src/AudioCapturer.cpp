#include "AudioCapturer.h"
#include "Logger.h"
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#define CHECK_HR(hr, msg) if (FAILED(hr)) { \
    LOG(err) << QString("调用%1失败，错误码：%2") \
             .arg(msg) \
             .arg(hr); \
    return; \
}

class AudioCapturerPrivate
{
public:
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;

    const REFERENCE_TIME REFTIMES_PER_SEC = 10000000;
    const REFERENCE_TIME REFTIMES_PER_MILLISEC = 10000;
    const REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration = 0;
    WAVEFORMATEX wfx;

    int timerInterval = 0;
    int totalDuration = 0;
    int needDuration = 0;
    QTimer* captureTimer = nullptr;
    QByteArray savedData;
};

AudioCapturer::AudioCapturer(QObject *parent)
    : QObject(parent)
    , d(new AudioCapturerPrivate)
{
    // 初始化COM
    CoInitialize(nullptr);
    
    // 创建设备枚举器
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&d->pEnumerator
    );
    CHECK_HR(hr, "CoCreateInstance");

    // 获取默认音频渲染设备
    hr = d->pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &d->pDevice);
    CHECK_HR(hr, "GetDefaultAudioEndpoint");

    // 创建音频客户端
    hr = d->pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&d->pAudioClient);
    CHECK_HR(hr, "Activate");

    // 设置音频格式
    d->wfx.wFormatTag = WAVE_FORMAT_PCM;
    d->wfx.nChannels = 1;
    d->wfx.nSamplesPerSec = 11025;
    d->wfx.wBitsPerSample = 16;
    d->wfx.nBlockAlign = d->wfx.nChannels * d->wfx.wBitsPerSample / 8;
    d->wfx.nAvgBytesPerSec = d->wfx.nSamplesPerSec * d->wfx.nBlockAlign;
    d->wfx.cbSize = 0;

    // 初始化音频客户端
    hr = d->pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
        d->hnsRequestedDuration,
        0,
        &d->wfx,
        nullptr
    );
    CHECK_HR(hr, "Initialize");

    // 获取缓冲区大小
    UINT32 buffer_frame_count = 0;
    hr = d->pAudioClient->GetBufferSize(&buffer_frame_count);
    CHECK_HR(hr, "GetBufferSize");

    // 获取捕获客户端
    hr = d->pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&d->pCaptureClient);
    CHECK_HR(hr, "GetService");

    // 设置定时器
    d->hnsActualDuration = (double)d->REFTIMES_PER_SEC * buffer_frame_count / d->wfx.nSamplesPerSec;
    d->timerInterval = d->hnsActualDuration/d->REFTIMES_PER_MILLISEC/2;
    
    d->captureTimer = new QTimer(this);
    connect(d->captureTimer, &QTimer::timeout, this, &AudioCapturer::onCaptured);
}

AudioCapturer::~AudioCapturer()
{
    stop();
    if (d->pCaptureClient) d->pCaptureClient->Release();
    if (d->pAudioClient) d->pAudioClient->Release();
    if (d->pDevice) d->pDevice->Release();
    if (d->pEnumerator) d->pEnumerator->Release();
    CoUninitialize();
    delete d;
}

void AudioCapturer::start(int msec)
{
    HRESULT hr = d->pAudioClient->Start();
    CHECK_HR(hr, "Start");

    d->needDuration = msec;
    d->captureTimer->start(d->timerInterval);
}

void AudioCapturer::stop()
{
    d->captureTimer->stop();
    d->pAudioClient->Stop();
}

void AudioCapturer::onCaptured()
{
    UINT32 packetLength = 0;
    BYTE* pData;
    UINT32 numFramesAvailable;
    DWORD flags;

    HRESULT hr = d->pCaptureClient->GetNextPacketSize(&packetLength);
    CHECK_HR(hr, "GetNextPacketSize");
    
    while (packetLength != 0) {
        hr = d->pCaptureClient->GetBuffer(
            &pData,
            &numFramesAvailable,
            &flags,
            nullptr,
            nullptr
        );
        CHECK_HR(hr, "GetBuffer");

        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            int bytes_to_write = numFramesAvailable * d->wfx.nBlockAlign;
            QByteArray data((char*)pData, bytes_to_write);
            d->savedData.append(data);
        } else {
            QByteArray data(numFramesAvailable * d->wfx.nBlockAlign, 0);
            d->savedData.append(data);
        }

        hr = d->pCaptureClient->ReleaseBuffer(numFramesAvailable);
        CHECK_HR(hr, "ReleaseBuffer");

        hr = d->pCaptureClient->GetNextPacketSize(&packetLength);
        CHECK_HR(hr, "GetNextPacketSize");
    }

    d->totalDuration += d->timerInterval;
    if (d->totalDuration >= d->needDuration) {
        d->totalDuration = 0;
        emit readReady(d->savedData);
        d->savedData.clear();
    }
}
