#include "applegamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>

AppleGameSettings::AppleGameSettings(QWidget *parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载苹果游戏专属背景
    m_bgPixmap.load(":/img/apple_setup.bmp");
    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    } else {
        setFixedSize(400, 300);
    }

    setupUI();

    connect(m_btnOk, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
    connect(m_btnDefault, &ImageButton::clicked, this, &AppleGameSettings::onDefaultClicked);

    connect(m_sliderLevel, &QSlider::valueChanged, this, &AppleGameSettings::onLevelChanged);
    connect(m_sliderTarget, &QSlider::valueChanged, this, &AppleGameSettings::onTargetChanged);
    connect(m_sliderFail, &QSlider::valueChanged, this, &AppleGameSettings::onFailChanged);
}

void AppleGameSettings::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // 根据背景图调整边距 
    mainLayout->setContentsMargins(120, 60, 40, 30); 
    mainLayout->setSpacing(20);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(15);
    formLayout->setVerticalSpacing(20);

    //  游戏等级 (1-10)
    m_sliderLevel = new QSlider(Qt::Horizontal);
    m_sliderLevel->setRange(1, 10);
    setupSliderStyle(m_sliderLevel);
    
    m_labelLevel = new QLabel("3");
    m_labelLevel->setFixedWidth(50);
    m_labelLevel->setAlignment(Qt::AlignCenter);
    
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->addWidget(m_sliderLevel);
    row1->addWidget(m_labelLevel);
    
    QLabel* l1 = new QLabel(QStringLiteral("游戏等级:"));
    l1->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l1, row1);

    // 过关苹果 (10-200)
    m_sliderTarget = new QSlider(Qt::Horizontal);
    m_sliderTarget->setRange(10, 200);
    m_sliderTarget->setSingleStep(10);
    setupSliderStyle(m_sliderTarget);

    m_labelTarget = new QLabel("100");
    m_labelTarget->setFixedWidth(50);
    m_labelTarget->setAlignment(Qt::AlignCenter);

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->addWidget(m_sliderTarget);
    row2->addWidget(m_labelTarget);

    QLabel* l2 = new QLabel(QStringLiteral("过关苹果:"));
    l2->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l2, row2);

    //  失败苹果 (1-20)
    m_sliderFail = new QSlider(Qt::Horizontal);
    m_sliderFail->setRange(1, 20);
    setupSliderStyle(m_sliderFail);

    m_labelFail = new QLabel("10");
    m_labelFail->setFixedWidth(50);
    m_labelFail->setAlignment(Qt::AlignCenter);

    QHBoxLayout* row3 = new QHBoxLayout();
    row3->addWidget(m_sliderFail);
    row3->addWidget(m_labelFail);

    QLabel* l3 = new QLabel(QStringLiteral("失败苹果:"));
    l3->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l3, row3);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    m_btnOk = new ImageButton(":/img/ok.bmp", ":/img/ok_hover.bmp", ":/img/ok_pressed.bmp", this);
    m_btnCancel = new ImageButton(":/img/cancel.bmp", ":/img/cancel_hover.bmp", ":/img/cancel_pressed.bmp", this);
    m_btnDefault = new ImageButton(":/img/default.bmp", ":/img/default_hover.bmp", ":/img/default_pressed.bmp", this);

    btnLayout->addWidget(m_btnOk);
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnDefault);
    mainLayout->addLayout(btnLayout);
}

void AppleGameSettings::setupSliderStyle(QSlider* slider) {
    slider->setStyleSheet(
        "QSlider::groove:horizontal { border: 0px; height: 8px; background: transparent; border-image: url(:/img/slider_bg.bmp) 0 0 0 0 stretch stretch; }"
        "QSlider::handle:horizontal { border: 0px; width: 15px; height: 20px; margin: -6px 0; image: url(:/img/slider_slider.bmp); }"
        "QSlider::handle:horizontal:pressed { image: url(:/img/slider_slider_down.bmp); }"
        "QSlider::sub-page:horizontal { background: transparent; }"
    );
}

void AppleGameSettings::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    else painter.fillRect(rect(), Qt::lightGray);
}

void AppleGameSettings::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}
void AppleGameSettings::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void AppleGameSettings::setSettings(const AppleSettingsData &s) {
    m_sliderLevel->setValue(s.level);
    m_sliderTarget->setValue(s.targetCount);
    m_sliderFail->setValue(s.failCount);
    onLevelChanged(s.level);
    onTargetChanged(s.targetCount);
    onFailChanged(s.failCount);
}

AppleSettingsData AppleGameSettings::getSettings() const {
    AppleSettingsData s;
    s.level = m_sliderLevel->value();
    s.targetCount = m_sliderTarget->value();
    s.failCount = m_sliderFail->value();
    return s;
}

void AppleGameSettings::onLevelChanged(int v) { m_labelLevel->setText(QString::number(v)); }
void AppleGameSettings::onTargetChanged(int v) { m_labelTarget->setText(QString::number(v)); }
void AppleGameSettings::onFailChanged(int v) { m_labelFail->setText(QString::number(v)); }

void AppleGameSettings::onDefaultClicked() {
    AppleSettingsData d;
    setSettings(d);
}