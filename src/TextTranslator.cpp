#include "TextTranslator.h"
#include "logger.h"
#include <QThread>
#include <QDir>
#include <sentencepiece_processor.h>
#include <ctranslate2/translator.h>

class TextTranslatorPrivate
{
public:
    TextTranslatorPrivate() = default;
    ~TextTranslatorPrivate()
    {
        cleanUp();
    }
    void setLanguage(const QString& src, const QString& dst)
    {
        const QString prefix = "models/";
        const QString sp_suffix = "/sentencepiece.model";
        const QString tr_suffix = "/model";
        cleanUp();
    
        // 如果源语言和目标语言都不是英语,需要加载中间模型
        if (src != "en" && dst != "en") {
            
            // 先加载src->en的模型
            QString src_en_dir = QString("%1translate-%2_en").arg(prefix).arg(src);
            if (!QDir(src_en_dir).exists()) {
                LOG(err) << "翻译模型不存在: " << src_en_dir;
                return;
            }
            
            // 加载en->dst的模型
            QString en_dst_dir = QString("%1translate-en_%2").arg(prefix).arg(dst);
            if (!QDir(en_dst_dir).exists()) {
                LOG(err) << "翻译模型不存在: " << en_dst_dir;
                return;
            }

            // 加载第一个模型(src->en)
            tokenizer = new sentencepiece::SentencePieceProcessor();
            auto status = tokenizer->Load((src_en_dir + sp_suffix).toUtf8().constData());
            if (status.ok()) {
                LOG(info) << "加载翻译模型成功: " << src_en_dir;
            } else {
                LOG(err) << "加载翻译模型失败: " << src_en_dir << " " << status.ToString();
                return;
            }

            translator = new ctranslate2::Translator((src_en_dir + tr_suffix).toStdString(), ctranslate2::Device::CPU);
                
            // 加载第二个模型(en->dst)
            tokenizer2 = new sentencepiece::SentencePieceProcessor();
            status = tokenizer2->Load((en_dst_dir+sp_suffix).toUtf8().constData());
            if (status.ok()) {
                LOG(info) << "加载翻译模型成功: " << en_dst_dir;
            } else {
                LOG(err) << "加载翻译模型失败: " << en_dst_dir << " " << status.ToString();
                return;
            }

            translator2 = new ctranslate2::Translator((en_dst_dir + tr_suffix).toStdString(), ctranslate2::Device::CPU);
        } else {
            // 直接加载src->dst的模型
            QString model_dir = QString("%1translate-%2_%3").arg(prefix).arg(src).arg(dst);
            if (!QDir(model_dir).exists()) {
                LOG(err) << "翻译模型不存在: " << model_dir;
                return;
            }
            tokenizer = new sentencepiece::SentencePieceProcessor();
            auto status = tokenizer->Load((model_dir+sp_suffix).toUtf8().constData());
            if (status.ok()) {
                LOG(info) << "加载翻译模型成功: " << model_dir;
            } else {
                LOG(err) << "加载翻译模型失败: " << model_dir << " " << status.ToString();
                return;
            }

            translator = new ctranslate2::Translator((model_dir + tr_suffix).toStdString(), ctranslate2::Device::CPU);
        }
    }
    void cleanUp()
    {
        if (tokenizer) {
            delete tokenizer;
            tokenizer = nullptr;
        }
        if (tokenizer2) {
            delete tokenizer2;
            tokenizer2 = nullptr;
        }
        if (translator) {
            delete translator;
            translator = nullptr;
        }
        if (translator2) {
            delete translator2;
            translator2 = nullptr;
        }
    }

public:
    sentencepiece::SentencePieceProcessor* tokenizer = nullptr;
    sentencepiece::SentencePieceProcessor* tokenizer2 = nullptr;
    ctranslate2::Translator* translator = nullptr;
    ctranslate2::Translator* translator2 = nullptr;
};

TextTranslator::TextTranslator()
    : QObject(nullptr)
    , thread(new QThread(this))
    , d(new TextTranslatorPrivate())
{
    thread->setObjectName("TextTranslatorThread");
    moveToThread(thread);

    thread->start();
}

TextTranslator::~TextTranslator()
{
    thread->quit();
    thread->wait();
    delete d;
}

void TextTranslator::setLanguage(const QString& src, const QString& dst)
{
    LOG(info) << "设置翻译语言: " << src << " -> " << dst;
    thread->quit();
    thread->wait();
    d->setLanguage(src, dst);
    thread->start();
}

void TextTranslator::translate(const QString& text)
{
    if (text.isEmpty()) {
        emit completed(QString());
        return;
    }
    std::vector<std::vector<std::string>> batch = {{""}};
    d->tokenizer->Encode(text.toUtf8().constData(), &batch[0]);
    ctranslate2::TranslationOptions options;
    options.beam_size = 1;
    options.length_penalty = 0.2f;
    options.replace_unknowns = true;
    auto ret_batch = d->translator->translate_batch(batch, options);
    if (d->translator2) {
        ret_batch = d->translator2->translate_batch(ret_batch[0].hypotheses, options);
    }
    std::string std_str;
    if (d->translator2) {
        d->tokenizer2->Decode(ret_batch[0].output(), &std_str);
    } else {
        d->tokenizer->Decode(ret_batch[0].output(), &std_str);
    }
    auto ret_str = QString::fromStdString(std_str);
    ret_str.replace("▁", " ");
    ret_str = ret_str.trimmed();
    emit completed(ret_str);
}
