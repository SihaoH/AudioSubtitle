# AudioSubtitle
## 概述
这是一个由C++编写的实时音频转字幕程序，主要功能是录制电脑播放的声音，通过语音识别显示字幕，以及通过机器翻译显示翻译后字幕。

支持完全离线，但由于语音识别和文本翻译都是使用本地CPU处理，延迟会比较大。而且都是选用最小的模型，所以准确度无法保证。

*目前支持中、英、日、韩 4种语言；因为翻译模型的限制，非英文的翻译中间会多转一次，如日→中，实际则是日→英→中*

## 组成
- 语音识别：[vosk-api](https://github.com/alphacep/vosk-api)
- 机器翻译：
	- 句子分词：[sentencepiece](https://github.com/google/sentencepiece)
	- 翻译引擎：[CTranslate2](https://github.com/OpenNMT/CTranslate2)
- 模型来源：
	- 语音识别：https://alphacephei.com/vosk/models
	- 翻译模型：https://www.argosopentech.com/argospm/index  （直接解压）
- 界面&其他：[Qt6](https://github.com/qt/qt5)

