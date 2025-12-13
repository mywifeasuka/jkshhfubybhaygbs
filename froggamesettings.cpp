#include "froggamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>
#include <QDebug>

FrogGameSettings::FrogGameSettings(QWidget *parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载背景
    m_bgPixmap.load(":/img/frog_setup.png");
    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    } else {
        setFixedSize(400, 300);
    }

    // 初始化课程映射 (显示名 -> 文件名)
    m_courseMap << qMakePair(QString("新版小学英语"), QString("Grade.DAT"));
    m_courseMap << qMakePair(QString("新版中学英语"), QString("Middle.DAT"));
    m_courseMap << qMakePair(QString("新版高中英语"), QString("High.DAT"));
    m_courseMap << qMakePair(QString("大学英语1~4级常用词汇"), QString("4W.DAT"));
    m_courseMap << qMakePair(QString("大学英语5~6级常用词汇"), QString("6W.DAT"));
    m_courseMap << qMakePair(QString("新编GMAT词汇"), QString("GMAT.DAT"));
    m_courseMap << qMakePair(QString("GRE词汇进阶"), QString("GRE.DAT"));
    m_courseMap << qMakePair(QString("托福单词"), QString("TOEFL.DAT"));

    setupUI();

    connect(m_btnOk, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
    connect(m_btnDefault, &ImageButton::clicked, this, &FrogGameSettings::onDefaultClicked);
    connect(m_sliderDiff, &QSlider::valueChanged, this, &FrogGameSettings::onDifficultyChanged);
}

void FrogGameSettings::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // 调整边距以适应背景图 (左侧留白多一些，避开左边的蓝色装饰)
    mainLayout->setContentsMargins(180, 80, 50, 40); 
    mainLayout->setSpacing(20);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(15);
    formLayout->setVerticalSpacing(25);

    // 1. 选择课程 (ComboBox)
    m_comboCourse = new QComboBox();
    m_comboCourse->setFixedSize(180, 25);
    for (const auto& pair : m_courseMap) {
        m_comboCourse->addItem(pair.first, pair.second); // Data存文件名
    }
    // 简单的QSS美化
    m_comboCourse->setStyleSheet(
        "QComboBox { border: 1px solid gray; border-radius: 3px; padding: 1px 18px 1px 3px; background: white; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: darkgray; border-left-style: solid; }"
        "QComboBox::down-arrow { image: url(:/img/dropdown_button.bmp); }" // 如果没有下拉箭头图，可不写这行
    );

    QLabel* l1 = new QLabel("选择课程:");
    l1->setStyleSheet("font-family: 'SimSun'; font-size: 14px; color: black;");
    formLayout->addRow(l1, m_comboCourse);

    // 2. 难度等级 (Slider 1-9)
    m_sliderDiff = new QSlider(Qt::Horizontal);
    m_sliderDiff->setRange(1, 9);
    setupSliderStyle(m_sliderDiff);

    m_labelDiff = new QLabel("1");
    m_labelDiff->setFixedWidth(30);
    
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->addWidget(m_sliderDiff);
    row2->addWidget(m_labelDiff);

    QLabel* l2 = new QLabel("难度等级:");
    l2->setStyleSheet("font-family: 'SimSun'; font-size: 14px; color: black;");
    formLayout->addRow(l2, row2);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // 按钮区域
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

void FrogGameSettings::setupSliderStyle(QSlider* slider) {
    slider->setStyleSheet(
        "QSlider::groove:horizontal { border: 0px; height: 8px; background: transparent; border-image: url(:/img/slider_bg.bmp) 0 0 0 0 stretch stretch; }"
        "QSlider::handle:horizontal { border: 0px; width: 15px; height: 20px; margin: -6px 0; image: url(:/img/slider_slider.bmp); }"
        "QSlider::handle:horizontal:pressed { image: url(:/img/slider_slider_down.bmp); }"
        "QSlider::sub-page:horizontal { background: transparent; }"
    );
}

void FrogGameSettings::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    else painter.fillRect(rect(), Qt::white);
}

void FrogGameSettings::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}
void FrogGameSettings::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void FrogGameSettings::setSettings(const FrogSettingsData &s) {
    m_sliderDiff->setValue(s.difficulty);
    m_labelDiff->setText(QString::number(s.difficulty));
    
    // 根据文件名找到对应的 ComboBox 索引
    int idx = m_comboCourse->findData(s.dictionaryFile);
    if (idx != -1) m_comboCourse->setCurrentIndex(idx);
}

FrogSettingsData FrogGameSettings::getSettings() const {
    FrogSettingsData s;
    s.difficulty = m_sliderDiff->value();
    s.dictionaryFile = m_comboCourse->currentData().toString();
    s.dictionaryName = m_comboCourse->currentText();
    return s;
}

void FrogGameSettings::onDifficultyChanged(int value) {
    m_labelDiff->setText(QString::number(value));
}

void FrogGameSettings::onDefaultClicked() {
    FrogSettingsData d;
    d.difficulty = 1;
    d.dictionaryFile = "4W.DAT"; // 默认4级
    setSettings(d);
}