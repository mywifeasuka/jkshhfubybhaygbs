#include "gameresultdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>

GameResultDialog::GameResultDialog(QWidget *parent) 
    : QDialog(parent), m_selectedAction(Action_None) 
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载背景 [cite: 735]
    m_bgPixmap.load(":/img/mole_dlg_bg.bmp");
    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    } else {
        setFixedSize(400, 250); // 容错尺寸
    }

    setupUI();
}

void GameResultDialog::setupUI() {
    // 使用垂直布局，增加边距以适应背景图的"内容区"
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 60, 40, 40); // 根据图片实际留白调整
    mainLayout->setSpacing(10);

    // 1. 文本消息区域
    m_messageLabel = new QLabel(this);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    // 模仿原版字体风格
    m_messageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 18px; font-weight: bold; color: #5D4037;");
    
    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("font-family: 'Arial'; font-size: 14px; font-weight: bold; color: #E65100;");

    mainLayout->addWidget(m_messageLabel);
    mainLayout->addWidget(m_scoreLabel);
    mainLayout->addStretch(); // 弹簧，把按钮顶到底部

    // 2. 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    btnLayout->addStretch();

    // 加载按钮资源 [cite: 736, 737, 738]
    // 注意：如果有 hover/click 资源请替换对应的参数
    m_btnReplay = new ImageButton(":/img/mole_dlg_replay.bmp", ":/img/mole_dlg_replay_hover.bmp", ":/img/mole_dlg_replay_pressed.bmp", this);
    connect(m_btnReplay, &ImageButton::clicked, this, &GameResultDialog::onReplayClicked);

    m_btnNext = new ImageButton(":/img/mole_dlg_next.bmp", ":/img/mole_dlg_next_hover.bmp", ":/img/mole_dlg_next_pressed.bmp", this);
    connect(m_btnNext, &ImageButton::clicked, this, &GameResultDialog::onNextLevelClicked);

    m_btnEnd = new ImageButton(":/img/mole_dlg_end.bmp", ":/img/mole_dlg_end_hover.bmp", ":/img/mole_dlg_end_pressed.bmp", this);
    connect(m_btnEnd, &ImageButton::clicked, this, &GameResultDialog::onEndClicked);

    btnLayout->addWidget(m_btnReplay);
    btnLayout->addWidget(m_btnNext);
    btnLayout->addWidget(m_btnEnd);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);
}

void GameResultDialog::setGameResult(int score, bool isWin) {
    m_scoreLabel->setText(QStringLiteral("本局得分: ") + QString::number(score));

    if (isWin) {
        // 胜利状态
        m_messageLabel->setText(QStringLiteral("恭喜，您通过了！"));
        m_btnNext->setVisible(true); // 显示下一关按钮
    } else {
        // 失败状态
        m_messageLabel->setText(QStringLiteral("您认输吧！"));
        m_btnNext->setVisible(false); // 失败不能进入下一关
    }
}

void GameResultDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), Qt::white);
        painter.setPen(Qt::black);
        painter.drawRect(0, 0, width()-1, height()-1);
    }
}

// 槽函数处理
void GameResultDialog::onReplayClicked() {
    m_selectedAction = Action_Replay;
    accept(); // 关闭对话框并返回 QDialog::Accepted
}

void GameResultDialog::onNextLevelClicked() {
    m_selectedAction = Action_NextLevel;
    accept();
}

void GameResultDialog::onEndClicked() {
    m_selectedAction = Action_End;
    accept();
}