#include "TextTranslator.h"
#include "logger.h"
#include <QThread>
#include <sentencepiece_processor.h>
#include <ctranslate2/translator.h>

class TextTranslatorPrivate
{
public:
    TextTranslatorPrivate(const QString& model_path)
    {
        tokenizer = new sentencepiece::SentencePieceProcessor();
        const auto status = tokenizer->Load((model_path+"/spiece.model").toUtf8().constData());
        if (!status.ok()) {
            LOG(err) << "加载翻译模型失败: " << status.ToString();
        }
        ctranslate2::ReplicaPoolConfig ct2_cfg = {};
        ct2_cfg.num_threads_per_replica = 4;
        translator = new ctranslate2::Translator((model_path + "/model").toStdString(), ctranslate2::Device::CPU, ctranslate2::ComputeType::DEFAULT, {0}, false, ct2_cfg);
    }
    ~TextTranslatorPrivate()
    {
        delete tokenizer;
        delete translator;
    }

public:
    sentencepiece::SentencePieceProcessor* tokenizer = nullptr;
    ctranslate2::Translator* translator = nullptr;
};

TextTranslator::TextTranslator(const QString& model_path, QObject* parent)
    : QObject(parent)
    , thread(new QThread(this))
    , d(new TextTranslatorPrivate(model_path))
{
    thread->setObjectName("TextTranslatorThread");
    moveToThread(thread);

    thread->start();
}

TextTranslator::~TextTranslator()
{
    delete d;
    thread->quit();
    thread->wait();
}

void TextTranslator::setLanguage(const QString& src, const QString& dst)
{
    srcLang = src;
    dstLang = dst;
}

void TextTranslator::translate(const QString& text)
{
    if (text.isEmpty()) {
        emit completed(QString());
        return;
    }
    //auto prefix_text = QString("%1%2%3: %4").arg(srcLang).arg("2").arg(dstLang).arg(text);
    auto prefix_text = text;
    std::vector<std::vector<std::string>> batch = {{""}};
    d->tokenizer->Encode(prefix_text.toUtf8().constData(), &batch[0]);
    ctranslate2::TranslationOptions options;
    options.beam_size = 1;
    options.use_vmap = true;
    auto ret_batch = d->translator->translate_batch(batch, options);
    std::string std_str;
    d->tokenizer->Decode(ret_batch[0].output(), &std_str);
    auto ret_str = QString::fromStdString(std_str);
    ret_str.replace("▁", " ");
    ret_str = ret_str.trimmed();
    emit completed(ret_str);
}
