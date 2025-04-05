#include "SubtitleWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>

SubtitleWidget::SubtitleWidget(QWidget *parent)
    : QWidget(parent)
    , isDragging(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_WState_WindowOpacitySet);
    setWindowOpacity(0.7);
    QPalette pal = palette(); 
    pal.setColor(QPalette::Window, QColor(0, 0, 0)); 
    setPalette(pal);

    initUI();
    setOriginalText("请播放任意带音频的内容");
}

void SubtitleWidget::initUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建关闭按钮
    closeButton = new QPushButton("×", this);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #FF0000;"
        "}"
    );
    QFont font = closeButton->font();
    font.setPointSize(20);
    closeButton->setFont(font);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    
    // 创建文本标签
    originalLabel = new QLabel(this);
    translatedLabel = new QLabel(this);
    originalLabel->setMargin(10);
    originalLabel->setFixedHeight(60);
    translatedLabel->setMargin(10);
    translatedLabel->setFixedHeight(60);
    QFont labelFont;
    labelFont.setPointSize(30);
    originalLabel->setFont(labelFont);
    labelFont.setPointSize(24);
    translatedLabel->setFont(labelFont);
    
    // 设置字体颜色为白色
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::white);
    originalLabel->setPalette(palette);
    translatedLabel->setPalette(palette);

    // 设置文本对齐方式
    originalLabel->setAlignment(Qt::AlignCenter);
    translatedLabel->setAlignment(Qt::AlignCenter);
    
    // 添加部件到布局
    mainLayout->addWidget(closeButton, 0, Qt::AlignRight);
    mainLayout->addWidget(originalLabel);
    mainLayout->addWidget(translatedLabel);
    
    // 添加一个1px高度的占位widget以保持最小尺寸
    QWidget* spacer = new QWidget(this);
    spacer->setFixedSize(600, 1);
    mainLayout->addWidget(spacer);
    
    setLayout(mainLayout);
}

void SubtitleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void SubtitleWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && isDragging) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void SubtitleWidget::mouseReleaseEvent(QMouseEvent *event)
{
    isDragging = false;
    event->accept();
}

void SubtitleWidget::setOriginalText(const QString &text)
{
    originalLabel->setText(text);
}

void SubtitleWidget::setTranslatedText(const QString &text)
{
    translatedLabel->setText(text);
}

void SubtitleWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 获取大小变化前后的差值
    QSize oldSize = event->oldSize();
    QSize newSize = event->size();
    
    // 只有在有效的旧尺寸时才需要调整
    if (oldSize.isValid()) {
        // 计算宽度变化值的一半
        int xOffset = (newSize.width() - oldSize.width()) / 2;
        // 移动窗口，使中心点保持不变
        move(x() - xOffset, y());
    }
} 