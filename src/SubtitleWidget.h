#pragma once

#include <QWidget>

class SubtitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SubtitleWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void setOriginalText(const QString &text);
    void setTranslatedText(const QString &text);

private:
    void initUI();

    class QLabel *originalLabel = nullptr;
    class QLabel *translatedLabel = nullptr;
    class QPushButton *closeButton = nullptr;
    QPoint dragPosition;
    bool isDragging = false;
};
