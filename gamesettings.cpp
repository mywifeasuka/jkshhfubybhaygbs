#include "gamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout> 
#include <QPainter>
#include <QStyleOption>

GameSettings::GameSettings(QWidget* parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground); // 允许透明背景（如果有的话）

    m_bgPixmap.load(":/img/mole_setup.bmp");
    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    }
    else {
        setFixedSize(400, 300); // 容错大小
    }

    setupUI();

    // 连接信号
    connect(okButton, &ImageButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &ImageButton::clicked, this, &QDialog::reject);
    connect(defaultButton, &ImageButton::clicked, this, &GameSettings::onDefaultButtonClicked);

    connect(gameTimeSlider, &QSlider::valueChanged, this, &GameSettings::onGameTimeSliderChanged);
    connect(spawnIntervalSlider, &QSlider::valueChanged, this, &GameSettings::onSpawnIntervalSliderChanged);
    connect(stayTimeSlider, &QSlider::valueChanged, this, &GameSettings::onStayTimeSliderChanged);
}

void GameSettings::setupUI() {
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(120, 60, 40, 30);
    mainLayout->setSpacing(20); // 控件垂直间距

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight); // 标签右对齐
    formLayout->setHorizontalSpacing(15);
    formLayout->setVerticalSpacing(20);

    gameTimeSlider = new QSlider(Qt::Horizontal);
    gameTimeSlider->setRange(10, 300);
    setupSliderStyle(gameTimeSlider); 

    gameTimeLabel = new QLabel("60");
    gameTimeLabel->setFixedWidth(50); 
    gameTimeLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* row1 = new QHBoxLayout();
    row1->addWidget(gameTimeSlider);
    row1->addWidget(gameTimeLabel);

    QLabel* label1 = new QLabel(QStringLiteral("游戏时间:"));

    label1->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333333;");
    formLayout->addRow(label1, row1);

    spawnIntervalSlider = new QSlider(Qt::Horizontal);
    spawnIntervalSlider->setRange(300, 2000);
    setupSliderStyle(spawnIntervalSlider);

    spawnIntervalLabel = new QLabel("1000");
    spawnIntervalLabel->setFixedWidth(50);
    spawnIntervalLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->addWidget(spawnIntervalSlider);
    row2->addWidget(spawnIntervalLabel);

    QLabel* label2 = new QLabel(QStringLiteral("鼹鼠出现间隔:"));
    label2->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333333;");
    formLayout->addRow(label2, row2);

    stayTimeSlider = new QSlider(Qt::Horizontal);
    stayTimeSlider->setRange(500, 5000);
    setupSliderStyle(stayTimeSlider);

    stayTimeLabel = new QLabel("5000");
    stayTimeLabel->setFixedWidth(50);
    stayTimeLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* row3 = new QHBoxLayout();
    row3->addWidget(stayTimeSlider);
    row3->addWidget(stayTimeLabel);

    QLabel* label3 = new QLabel(QStringLiteral("停留时间:"));
    label3->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333333;");
    formLayout->addRow(label3, row3);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch(); 

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->addStretch(); 

    okButton = new ImageButton(":/img/ok.bmp", ":/img/ok_hover.bmp", ":/img/ok_pressed.bmp", this);
    cancelButton = new ImageButton(":/img/cancel.bmp", ":/img/cancel_hover.bmp", ":/img/cancel_pressed.bmp", this);
    defaultButton = new ImageButton(":/img/default.bmp", ":/img/default_hover.bmp", ":/img/default_pressed.bmp", this);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(defaultButton);

    mainLayout->addLayout(buttonLayout);
}

void GameSettings::setupSliderStyle(QSlider* slider) {
    slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    border: 0px;"
        "    height: 8px;" 
        "    background: transparent;"
        "    border-image: url(:/img/slider_bg.bmp) 0 0 0 0 stretch stretch;"
        "}"
        "QSlider::handle:horizontal {"
        "    border: 0px;"
        "    width: 15px;" 
        "    height: 20px;"
        "    margin: -6px 0;" 
        "    image: url(:/img/slider_slider.bmp);" 
        "}"
        "QSlider::handle:horizontal:pressed {"
        "    image: url(:/img/slider_slider_down.bmp);" 
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: transparent;" 
        "}"
    );
}

// 绘制背景图
void GameSettings::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    }
    else {
        painter.fillRect(rect(), QColor(220, 220, 220));
        painter.setPen(Qt::black);
        painter.drawRect(0, 0, width() - 1, height() - 1);
    }
}

// 鼠标拖拽逻辑
void GameSettings::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void GameSettings::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void GameSettings::resetToDefaults() {
    onDefaultButtonClicked();
}

void GameSettings::setSettings(const GameSettingsData& settings) {
    currentSettings = settings;

    // 阻断信号防止递归调用或不必要的 UI 刷新
    bool oldState1 = gameTimeSlider->blockSignals(true);
    bool oldState2 = spawnIntervalSlider->blockSignals(true);
    bool oldState3 = stayTimeSlider->blockSignals(true);

    gameTimeSlider->setValue(settings.gameTimeSec);
    spawnIntervalSlider->setValue(settings.spawnIntervalMs);
    stayTimeSlider->setValue(settings.stayTimeMs);

    gameTimeSlider->blockSignals(oldState1);
    spawnIntervalSlider->blockSignals(oldState2);
    stayTimeSlider->blockSignals(oldState3);

    // 手动更新文字
    onGameTimeSliderChanged(settings.gameTimeSec);
    onSpawnIntervalSliderChanged(settings.spawnIntervalMs);
    onStayTimeSliderChanged(settings.stayTimeMs);
}

GameSettingsData GameSettings::getSettings() const {
    GameSettingsData newSettings;
    newSettings.gameTimeSec = gameTimeSlider->value();
    newSettings.spawnIntervalMs = spawnIntervalSlider->value();
    newSettings.stayTimeMs = stayTimeSlider->value();
    return newSettings;
}

void GameSettings::onGameTimeSliderChanged(int value) {
    gameTimeLabel->setText(QString::number(value));
}

void GameSettings::onSpawnIntervalSliderChanged(int value) {
    spawnIntervalLabel->setText(QString::number(value));
}

void GameSettings::onStayTimeSliderChanged(int value) {
    stayTimeLabel->setText(QString::number(value));
}

void GameSettings::onDefaultButtonClicked() {
    GameSettingsData defaultSettings;
    setSettings(defaultSettings);
}