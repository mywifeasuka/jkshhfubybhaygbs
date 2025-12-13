#include "spacegamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>

// 【修改点 1】解决中文乱码
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma execution_character_set("utf-8")
#endif

SpaceGameSettings::SpaceGameSettings(QWidget* parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    m_bgPixmap.load(":/img/apple_setup.bmp");
    if (!m_bgPixmap.isNull()) setFixedSize(m_bgPixmap.size());
    else setFixedSize(400, 300);

    setupUI();

    connect(m_btnOk, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
    connect(m_btnDefault, &ImageButton::clicked, this, &SpaceGameSettings::onDefaultClicked);

    connect(m_sliderDiff, &QSlider::valueChanged, this, &SpaceGameSettings::onDiffChanged);
    connect(m_sliderLives, &QSlider::valueChanged, this, &SpaceGameSettings::onLivesChanged);
}

void SpaceGameSettings::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(120, 60, 40, 30);
    mainLayout->setSpacing(15); //稍微调小间距以容纳更多选项

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(15);
    formLayout->setVerticalSpacing(15);

    // 1. 难度
    m_sliderDiff = new QSlider(Qt::Horizontal);
    m_sliderDiff->setRange(1, 10);
    setupSliderStyle(m_sliderDiff);
    m_labelDiff = new QLabel("1");
    m_labelDiff->setFixedWidth(50);
    // 强制黑色字体
    m_labelDiff->setStyleSheet("color: black; font-weight: bold;");

    QHBoxLayout* row1 = new QHBoxLayout();
    row1->addWidget(m_sliderDiff);
    row1->addWidget(m_labelDiff);

    QLabel* l1 = new QLabel("游戏难度:");
    l1->setStyleSheet("font-family: 'SimSun'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l1, row1);

    // 2. 生命
    m_sliderLives = new QSlider(Qt::Horizontal);
    m_sliderLives->setRange(1, 5);
    setupSliderStyle(m_sliderLives);
    m_labelLives = new QLabel("3");
    m_labelLives->setFixedWidth(50);
    m_labelLives->setStyleSheet("color: black; font-weight: bold;");

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->addWidget(m_sliderLives);
    row2->addWidget(m_labelLives);

    QLabel* l2 = new QLabel("初始生命:");
    l2->setStyleSheet("font-family: 'SimSun'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l2, row2);

    // 3. 【新增】奖励模式
    m_checkBonus = new QCheckBox("开启");
    // 使用 QSS 美化 CheckBox，利用 checkbox_button.bmp
    m_checkBonus->setStyleSheet(
        "QCheckBox { color: black; font-weight: bold; }"
        "QCheckBox::indicator { width: 20px; height: 20px; border: 1px solid gray; background: white; }"
        "QCheckBox::indicator:checked { image: url(:/img/checkbox_button.bmp); }" // 选中时显示图片
    );

    QLabel* l3 = new QLabel("奖励模式:");
    l3->setStyleSheet("font-family: 'SimSun'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l3, m_checkBonus);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
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

// ... setupSliderStyle, paintEvent, mouseEvents 保持不变 ...

void SpaceGameSettings::setupSliderStyle(QSlider* slider) {
    slider->setStyleSheet(
        "QSlider::groove:horizontal { border: 0px; height: 8px; background: transparent; border-image: url(:/img/slider_bg.bmp) 0 0 0 0 stretch stretch; }"
        "QSlider::handle:horizontal { border: 0px; width: 15px; height: 20px; margin: -6px 0; image: url(:/img/slider_slider.bmp); }"
        "QSlider::handle:horizontal:pressed { image: url(:/img/slider_slider_down.bmp); }"
        "QSlider::sub-page:horizontal { background: transparent; }"
    );
}

void SpaceGameSettings::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    else painter.fillRect(rect(), Qt::white);
}

void SpaceGameSettings::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}
void SpaceGameSettings::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void SpaceGameSettings::setSettings(const SpaceSettingsData& s) {
    m_sliderDiff->setValue(s.difficulty);
    m_sliderLives->setValue(s.lives);
    m_checkBonus->setChecked(s.bonusMode); // 【新增】
    m_labelDiff->setText(QString::number(s.difficulty));
    m_labelLives->setText(QString::number(s.lives));
}

SpaceSettingsData SpaceGameSettings::getSettings() const {
    SpaceSettingsData s;
    s.difficulty = m_sliderDiff->value();
    s.lives = m_sliderLives->value();
    s.bonusMode = m_checkBonus->isChecked(); // 【新增】
    return s;
}

void SpaceGameSettings::onDiffChanged(int v) { m_labelDiff->setText(QString::number(v)); }
void SpaceGameSettings::onLivesChanged(int v) { m_labelLives->setText(QString::number(v)); }
void SpaceGameSettings::onDefaultClicked() {
    SpaceSettingsData d;
    d.difficulty = 1;
    d.lives = 3;
    d.bonusMode = false;
    setSettings(d);
}