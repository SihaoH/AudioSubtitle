#pragma once
#include <QObject>

class TextTranslator : public QObject
{
    Q_OBJECT
public:
    TextTranslator();
    ~TextTranslator();

    void setLanguage(const QString& src, const QString& dst);

public slots:
    void translate(const QString& text);

signals:
    void completed(const QString& result);

private:
    class QThread* thread = nullptr;
    class TextTranslatorPrivate* d = nullptr;
    QString srcLang = "jp";
    QString dstLang = "zh";
};
