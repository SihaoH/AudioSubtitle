#include "SubtitleWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QComboBox>

SubtitleWidget::SubtitleWidget(QWidget *parent)
    : QWidget(nullptr)
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
    auto main_layout = new QVBoxLayout(this);
    main_layout->setSizeConstraint(QLayout::SetFixedSize);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    
    // 创建水平布局用于放置下拉框
    auto first_layout = new QHBoxLayout();
    first_layout->setSpacing(10);
    main_layout->setContentsMargins(10, 0, 0, 0);
    
    // 创建并设置原文语言选择
    originalLangLabel = new QLabel("原文:", this);
    originalLangLabel->setStyleSheet("color: white;");
    originalLangCombo = new QComboBox(this);
    originalLangCombo->setFixedWidth(60);
    originalLangCombo->setCursor(Qt::PointingHandCursor);
    for (auto i = langMap.cbegin(), end = langMap.cend(); i != end; ++i) {
        originalLangCombo->addItem(i.value(), i.key());
    }
    connect(originalLangCombo, &QComboBox::currentIndexChanged, [this](int index) {
        emit originalLangChanged(originalLangCombo->itemData(index).toString());
    });
    
    // 创建并设置译文语言选择
    translatedLangLabel = new QLabel("译文:", this);
    translatedLangLabel->setStyleSheet("color: white;");
    translatedLangCombo = new QComboBox(this);
    translatedLangCombo->setFixedWidth(60);
    translatedLangCombo->setCursor(Qt::PointingHandCursor);
    for (auto i = langMap.cbegin(), end = langMap.cend(); i != end; ++i) {
        translatedLangCombo->addItem(i.value(), i.key());
    }
    connect(translatedLangCombo, &QComboBox::currentIndexChanged, [this](int index) {
        emit translatedLangChanged(translatedLangCombo->itemData(index).toString());
    });

    // 创建并设置识别间隔选择
    durationLabel = new QLabel("识别间隔:", this);
    durationLabel->setStyleSheet("color: white;");
    durationCombo = new QComboBox(this);
    durationCombo->setFixedWidth(60);
    durationCombo->setCursor(Qt::PointingHandCursor);
    durationCombo->addItem("1.0");
    durationCombo->addItem("1.5");
    durationCombo->addItem("2.0");
    durationCombo->addItem("2.5");
    durationCombo->addItem("3.0");
    connect(durationCombo, &QComboBox::currentTextChanged, this, &SubtitleWidget::durationChanged);

    // 设置下拉框样式
    QString comboStyle = 
        "QComboBox {"
        "    background-color: #333333;"
        "    color: white;"
        "    border: 1px solid #666666;"
        "    padding: 2px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #333333;"
        "    color: white;"
        "    selection-background-color: #666666;"
        "}";
    originalLangCombo->setStyleSheet(comboStyle);
    translatedLangCombo->setStyleSheet(comboStyle);
    durationCombo->setStyleSheet(comboStyle);

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
    
    // 添加到水平布局
    first_layout->addWidget(originalLangLabel);
    first_layout->addWidget(originalLangCombo);
    first_layout->addSpacing(10);
    first_layout->addWidget(translatedLangLabel);
    first_layout->addWidget(translatedLangCombo);
    first_layout->addSpacing(10);
    first_layout->addWidget(durationLabel);
    first_layout->addWidget(durationCombo);
    first_layout->addStretch();
    first_layout->addWidget(closeButton);
    
    // 将水平布局添加到主布局
    main_layout->addLayout(first_layout);

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
    main_layout->addWidget(originalLabel);
    main_layout->addWidget(translatedLabel);
    
    // 添加一个1px高度的占位widget以保持最小尺寸
    QWidget* spacer = new QWidget(this);
    spacer->setFixedSize(600, 1);
    main_layout->addWidget(spacer);
    
    setLayout(main_layout);
}

void SubtitleWidget::setLanguage(const QString& src_lang, const QString& target_lang)
{
    originalLangCombo->setCurrentText(langMap[src_lang]);
    translatedLangCombo->setCurrentText(langMap[target_lang]);
}

void SubtitleWidget::setDuration(const QString& text)
{
    durationCombo->setCurrentText(text);
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
    emit positionChanged(pos());
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
    // 移动窗口，使中心点保持不变
    QSize oldSize = event->oldSize();
    QSize newSize = event->size();
    if (oldSize.isValid()) {
        int xOffset = (newSize.width() - oldSize.width()) / 2;
        move(x() - xOffset, y());
    }
    emit positionChanged(pos());
}
