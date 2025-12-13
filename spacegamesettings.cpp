#include "spacegamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>
#include <QDebug>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma execution_character_set("utf-8")
#endif

// ================= ImageCheckBox 实现 =================

ImageCheckBox::ImageCheckBox(QWidget* parent)
    : QWidget(parent), m_checked(false), m_isHover(false)
{
    setFixedSize(20, 20); // 默认大小，加载图片后会更新
    setCursor(Qt::PointingHandCursor);
}

void ImageCheckBox::loadImages(const QString& basePath) {
    // 假设资源后缀是 .bmp (根据原版资源习惯)
    // 0: 初始 -> .bmp
    m_pixmaps[0].load(basePath + ".bmp");
    // 1: 移入 -> _1.bmp
    m_pixmaps[1].load(basePath + "_1.bmp");
    // 2: 选中 -> _2.bmp
    m_pixmaps[2].load(basePath + "_2.bmp");
    // 3: 选中移入 -> _3.bmp
    m_pixmaps[3].load(basePath + "_3.bmp");

    // 根据第一张图调整大小
    if (!m_pixmaps[0].isNull()) {
        setFixedSize(m_pixmaps[0].size());
    }
    update();
}

void ImageCheckBox::setChecked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;
        emit stateChanged(m_checked);
        update();
    }
}

void ImageCheckBox::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    int index = 0;

    // 逻辑：0=关, 1=关hover, 2=开, 3=开hover
    if (m_checked) {
        index = m_isHover ? 3 : 2;
    }
    else {
        index = m_isHover ? 1 : 0;
    }

    // 容错：如果对应的状态图不存在，回退到基础图(0)
    if (m_pixmaps[index].isNull()) {
        if (m_checked && !m_pixmaps[2].isNull()) index = 2;
        else index = 0;
    }

    if (!m_pixmaps[index].isNull()) {
        painter.drawPixmap(0, 0, m_pixmaps[index]);
    }
}

void ImageCheckBox::enterEvent(QEvent*) {
    m_isHover = true;
    update();
}

void ImageCheckBox::leaveEvent(QEvent*) {
    m_isHover = false;
    update();
}

void ImageCheckBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setChecked(!m_checked); // 切换状态
    }
}

// ================= SpaceGameSettings 实现 =================

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
    // 调整边距以适配背景图 (左, 上, 右, 下)
    mainLayout->setContentsMargins(120, 50, 40, 20);
    mainLayout->setSpacing(10);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(15);
    formLayout->setVerticalSpacing(12);

    // 1. 难度
    m_sliderDiff = new QSlider(Qt::Horizontal);
    m_sliderDiff->setRange(1, 10);
    setupSliderStyle(m_sliderDiff);
    m_labelDiff = new QLabel("1");
    m_labelDiff->setFixedWidth(30);
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
    m_labelLives->setFixedWidth(30);
    m_labelLives->setStyleSheet("color: black; font-weight: bold;");

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->addWidget(m_sliderLives);
    row2->addWidget(m_labelLives);

    QLabel* l2 = new QLabel("初始生命:");
    l2->setStyleSheet("font-family: 'SimSun'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l2, row2);

    // 3. 奖励模式 (ImageCheckBox)
    QHBoxLayout* row3 = new QHBoxLayout();

    m_checkBonus = new ImageCheckBox(this);
    // 加载资源：checkbox_button.bmp, checkbox_button_1.bmp 等
    m_checkBonus->loadImages(":/img/checkbox_button");

    row3->addWidget(m_checkBonus);
    row3->addStretch();

    QLabel* l3 = new QLabel("奖励模式:");
    l3->setStyleSheet("font-family: 'SimSun'; font-size: 14px; font-weight: bold; color: #333;");
    formLayout->addRow(l3, row3);

    // 4. 奖励模式说明文字
    // 插入一个单独的布局来显示说明，使其与复选框对齐
    m_labelBonusDesc = new QLabel("选中此项，游戏过程中\n将出现加分奖励物体。", this);
    m_labelBonusDesc->setStyleSheet("color: #666; font-size: 12px; font-family: 'SimSun';");

    // 为了对齐，我们在左侧加一个 spacer，宽度等于Label宽度+间距
    QHBoxLayout* descLayout = new QHBoxLayout();
    descLayout->addSpacing(80); // 这里的80大概是 "奖励模式:" Label 的宽度
    descLayout->addWidget(m_labelBonusDesc);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(descLayout); // 加在 FormLayout 下方

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
    m_checkBonus->setChecked(s.bonusMode);
    m_labelDiff->setText(QString::number(s.difficulty));
    m_labelLives->setText(QString::number(s.lives));
}

SpaceSettingsData SpaceGameSettings::getSettings() const {
    SpaceSettingsData s;
    s.difficulty = m_sliderDiff->value();
    s.lives = m_sliderLives->value();
    s.bonusMode = m_checkBonus->isChecked();
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