# LiveSubtitles
## 概述
这是一个由C++编写的实时音频转字幕程序，主要功能是录制电脑播放的声音，通过语音识别显示字幕，以及通过机器翻译显示翻译后字幕。

支持浏览器、游戏、播放器以及任何有声音的软件，并且也支持完全离线；但由于语音识别和文本翻译都是使用本地CPU处理，延迟会比较大（但如果是使用cuda版本，则翻译速度可以显著提升，基本能做到秒翻）。目前选用的都是最小的模型，所以准确度还比较低。

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


## 编译环境
### 编译工具链
- CMake >= 3.30
- Visual Studio 2022（勾选`使用C++进行桌面开发`）

### vcpkg
用于导入第三方库openblas、spdlog
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat
```
然后将vcpkg.exe的路径添加到系统环境变量PATH里

### Qt6.8.2
可以直接使用在线安装包，不过还是建议自己下载源码来构建。目前只需编译qt-base模块即可。

#### 参考
https://doc.qt.io/qt-6/getting-sources-from-git.html

https://doc.qt.io/qt-6/windows-building.html

https://doc.qt.io/qt-6/configure-options.html

#### 下载源码
`git clone --branch v6.8.2 git://code.qt.io/qt/qt5.git .\src`

#### 生成
```
mkdir build
cd build
..\src\configure.bat -init-submodules -submodules qtbase
..\src\configure.bat -debug-and-release -prefix <path-to-qt>
cmake --build . --parallel
cmake --install .
cmake --install . --config debug
```
*上述步骤有两次install，第一次默认是只安装release*
最后将`<path-to-qt>\bin`添加到系统环境变量PATH中里


## 构建
拉取源码注意把submodule也完整拉取
```cmd
git clone --recurse-submodules git@github.com:SihaoH/LiveSubtitles.git
cd LiveSubtitles
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
```

最后打开build/LiveSubtitles.sln进行编译和调试


## 题外话
win11的实时辅助字幕真牛逼啊，也能离线，也是纯CPU处理，延时低准确率高。怎么样才能接上LiveCaptions.exe的接口呢。。。