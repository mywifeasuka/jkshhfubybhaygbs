#include "spacenamedialog.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QIntValidator>

SpaceNameDialog::SpaceNameDialog(int score, QWidget *parent) 
    : QDialog(parent), m_score(score) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    m_bgPixmap.load(":/img/space_caption_back.png");
    if (m_bgPixmap.isNull()) {
        QPixmap tmp(":/img/space_mainmenu_bg.png");
        if (!tmp.isNull()) m_bgPixmap = tmp.scaled(400, 300);
    }
    
    if (!m_bgPixmap.isNull()) setFixedSize(m_bgPixmap.size());
    else setFixedSize(400, 300);

    setupUI();
}

void SpaceNameDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(50, 60, 50, 40);
    layout->setSpacing(15);

    // 标题
    QLabel* title = new QLabel("请留下您的大名", this);
    title->setStyleSheet("color: yellow; font-size: 20px; font-weight: bold; font-family: 'Microsoft YaHei';");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // 分数展示
    QLabel* scoreLbl = new QLabel(QString("最终得分: %1").arg(m_score), this);
    scoreLbl->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    scoreLbl->setAlignment(Qt::AlignCenter);
    layout->addWidget(scoreLbl);

    // 输入框
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText("英雄");
    m_nameEdit->setMaxLength(10);
    m_nameEdit->setAlignment(Qt::AlignCenter);
    m_nameEdit->setStyleSheet(
        "QLineEdit { "
        "  border: 2px solid #4A90E2; "
        "  border-radius: 5px; "
        "  padding: 5px; "
        "  background-color: rgba(0, 0, 0, 150); "
        "  color: white; "
        "  font-size: 16px; "
        "}"
    );
    layout->addWidget(m_nameEdit);

    layout->addStretch();

    m_btnOk = new ImageButton(":/img/ok.bmp", ":/img/ok_hover.bmp", ":/img/ok_pressed.bmp", this);
    connect(m_btnOk, &ImageButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnOk);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);
}

QString SpaceNameDialog::getName() const {
    return m_nameEdit->text();
}

void SpaceNameDialog::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), QColor(0, 0, 50, 200));
        painter.setPen(Qt::white);
        painter.drawRect(0, 0, width()-1, height()-1);
    }
}