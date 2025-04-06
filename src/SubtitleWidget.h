#pragma once

#include <QWidget>
#include <QMap>

class SubtitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SubtitleWidget(QWidget *parent = nullptr);
    ~SubtitleWidget() = default;

    void setLanguage(const QString &src_lang, const QString &target_lang);
    void setDuration(const QString &duration);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void originalLangChanged(const QString &text);
    void translatedLangChanged(const QString &text);
    void durationChanged(const QString &text);
    void positionChanged(const QPoint &pos);

public slots:
    void setOriginalText(const QString &text);
    void setTranslatedText(const QString &text);

private:
    void initUI();

    class QComboBox *originalLangCombo;
    class QComboBox *translatedLangCombo;
    class QComboBox *durationCombo;
    class QLabel *originalLangLabel;
    class QLabel *translatedLangLabel;
    class QLabel *durationLabel;

    class QLabel *originalLabel = nullptr;
    class QLabel *translatedLabel = nullptr;
    class QPushButton *closeButton = nullptr;
    QPoint dragPosition;
    bool isDragging = false;

    const QMap<QString, QString> langMap = {{"zh", "中"}, {"ja", "日"}, {"ko", "韩"}, {"en", "英"}};
};
