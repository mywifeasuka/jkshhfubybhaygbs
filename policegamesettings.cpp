#include "policegamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>

// 【修复】强制使用UTF-8编码，解决MSVC编译器下的中文乱码问题
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma execution_character_set("utf-8")
#endif

PoliceGameSettings::PoliceGameSettings(QWidget* parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    m_bgPixmap.load(":/img/police_setting_bg.png");
    // 如果没有专用背景，使用默认图
    if (m_bgPixmap.isNull()) m_bgPixmap.load(":/img/mole_setup.bmp");

    if (!m_bgPixmap.isNull()) setFixedSize(m_bgPixmap.size());
    else setFixedSize(450, 350);

    setupUI();

    // 只有 OK 按钮（对应“结束”设置并开始游戏）
    connect(m_btnOk, &ImageButton::clicked, this, &QDialog::accept);
    // Cancel 对应退出或返回
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
    connect(m_btnDefault, &ImageButton::clicked, this, &PoliceGameSettings::onDefaultClicked);
}

void PoliceGameSettings::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 80, 50, 40);
    mainLayout->setSpacing(10);

    // 1. 角色选择
    QLabel* lblRole = new QLabel("选择角色:", this);
    lblRole->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");

    QHBoxLayout* roleLayout = new QHBoxLayout();
    m_groupRole = new QButtonGroup(this);
    QRadioButton* rbPolice = new QRadioButton("警察");
    QRadioButton* rbThief = new QRadioButton("小偷");
    QString radioStyle = "QRadioButton { font-weight: bold; color: #333; }";
    rbPolice->setStyleSheet(radioStyle);
    rbThief->setStyleSheet(radioStyle);

    m_groupRole->addButton(rbPolice, 0);
    m_groupRole->addButton(rbThief, 1);
    rbPolice->setChecked(true);

    roleLayout->addWidget(rbPolice);
    roleLayout->addWidget(rbThief);
    roleLayout->addStretch();

    // 2. 载具选择
    QLabel* lblVehicle = new QLabel("选择载具:", this);
    lblVehicle->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");

    QHBoxLayout* vehicleLayout = new QHBoxLayout();
    m_groupVehicle = new QButtonGroup(this);
    QRadioButton* rbCar = new QRadioButton("汽车");
    QRadioButton* rbBike = new QRadioButton("自行车");
    rbCar->setStyleSheet(radioStyle);
    rbBike->setStyleSheet(radioStyle);

    m_groupVehicle->addButton(rbCar, 0);
    m_groupVehicle->addButton(rbBike, 1);
    rbCar->setChecked(true);

    vehicleLayout->addWidget(rbCar);
    vehicleLayout->addWidget(rbBike);
    vehicleLayout->addStretch();

    // 3. 难度滑块
    QLabel* lblDiff = new QLabel("游戏难度:", this);
    lblDiff->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");

    m_sliderDiff = new QSlider(Qt::Horizontal);
    m_sliderDiff->setRange(1, 10);
    m_sliderDiff->setValue(3);
    m_sliderDiff->setStyleSheet(
        "QSlider::groove:horizontal { border: 0px; height: 8px; background: transparent; border-image: url(:/img/slider_bg.bmp) 0 0 0 0 stretch stretch; }"
        "QSlider::handle:horizontal { border: 0px; width: 15px; height: 20px; margin: -6px 0; image: url(:/img/slider_slider.bmp); }"
    );

    m_labelDiff = new QLabel("3");
    m_labelDiff->setStyleSheet("font-weight: bold; color: blue; font-size: 14px;");

    QHBoxLayout* diffLayout = new QHBoxLayout();
    diffLayout->addWidget(m_sliderDiff);
    diffLayout->addWidget(m_labelDiff);
    connect(m_sliderDiff, &QSlider::valueChanged, [=](int v) { m_labelDiff->setText(QString::number(v)); });

    mainLayout->addWidget(lblRole);
    mainLayout->addLayout(roleLayout);
    mainLayout->addWidget(lblVehicle);
    mainLayout->addLayout(vehicleLayout);
    mainLayout->addWidget(lblDiff);
    mainLayout->addLayout(diffLayout);
    mainLayout->addStretch();

    // 底部按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    // 使用“确定”作为进入游戏的按钮
    m_btnOk = new ImageButton(":/img/ok.bmp", ":/img/ok_hover.bmp", ":/img/ok_pressed.bmp", this);
    m_btnCancel = new ImageButton(":/img/cancel.bmp", ":/img/cancel_hover.bmp", ":/img/cancel_pressed.bmp", this);
    m_btnDefault = new ImageButton(":/img/default.bmp", ":/img/default_hover.bmp", ":/img/default_pressed.bmp", this);

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnOk);     // 结束设置，进入游戏
    btnLayout->addWidget(m_btnCancel); // 取消
    btnLayout->addWidget(m_btnDefault);
    mainLayout->addLayout(btnLayout);
}

void PoliceGameSettings::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    else painter.fillRect(rect(), QColor(240, 240, 240));
}

void PoliceGameSettings::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}
void PoliceGameSettings::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void PoliceGameSettings::setSettings(const PoliceSettingsData& s) {
    m_sliderDiff->setValue(s.difficulty);
    if (auto btn = m_groupRole->button(s.role)) btn->setChecked(true);
    if (auto btn = m_groupVehicle->button(s.vehicle)) btn->setChecked(true);
    m_labelDiff->setText(QString::number(s.difficulty));
}

PoliceSettingsData PoliceGameSettings::getSettings() const {
    PoliceSettingsData s;
    s.difficulty = m_sliderDiff->value();
    s.role = m_groupRole->checkedId();
    s.vehicle = m_groupVehicle->checkedId();
    return s;
}

void PoliceGameSettings::onDefaultClicked() {
    PoliceSettingsData d;
    setSettings(d);
}