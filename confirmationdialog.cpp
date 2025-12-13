#include "confirmationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

ConfirmationDialog::ConfirmationDialog(Mode mode, QWidget *parent) 
    : QDialog(parent) 
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载通用背景
    m_bgPixmap.load(":/img/main_dlg_bg.bmp");
    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    } else {
        setFixedSize(400, 250);
    }

    setupUI(mode);
}

void ConfirmationDialog::setupUI(Mode mode) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 60, 40, 40); // 根据背景图调整边距
    mainLayout->setSpacing(20);

    // 1. 提示文字
    m_messageLabel = new QLabel(this);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 16px; font-weight: bold; color: #5D4037;");
    
    // 2. 按钮布局
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);
    btnLayout->addStretch();

    if (mode == Mode_ExitGame) {
        m_messageLabel->setText(QStringLiteral("你真的要退出吗？"));
        
        // 确认退出按钮 (MAIN_DLG_EXIT)
        m_btnConfirm = new ImageButton(":/img/main_dlg_exit.bmp", ":/img/main_dlg_exit_hover.bmp", ":/img/main_dlg_exit_pressed.bmp", this);
        
        // 取消/继续按钮 (MAIN_DLG_REPLAY - 原版这里用Replay表示留在游戏里)
        m_btnCancel = new ImageButton(":/img/main_dlg_replay.bmp", ":/img/main_dlg_replay_hover.bmp", ":/img/main_dlg_replay_pressed.bmp", this);
    } 
    else if (mode == Mode_ApplySettings) {
        m_messageLabel->setText(QStringLiteral("设置改变，立即生效吗？"));
        
        // 是 (YES)
        m_btnConfirm = new ImageButton(":/img/yes.bmp", ":/img/yes_hover.bmp", ":/img/yes_pressed.bmp", this);
        
        // 否 (NO)
        m_btnCancel = new ImageButton(":/img/no.bmp", ":/img/no_hover.bmp", ":/img/no_pressed.bmp", this);
    }

    // 连接信号：Confirm -> Accept, Cancel -> Reject
    connect(m_btnConfirm, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);

    btnLayout->addWidget(m_btnConfirm);
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addStretch();

    mainLayout->addWidget(m_messageLabel);
    mainLayout->addLayout(btnLayout);
}

void ConfirmationDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), Qt::white);
        painter.setPen(Qt::black);
        painter.drawRect(0, 0, width()-1, height()-1);
    }
}