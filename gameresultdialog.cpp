#include "gameresultdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>

GameResultDialog::GameResultDialog(GameTheme theme, QWidget* parent)
    : QDialog(parent), m_currentTheme(theme), m_selectedAction(Action_None)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 1. 根据主题加载背景
    if (m_currentTheme == Theme_Apple) {
        m_bgPixmap.load(":/img/apple_dlg_bg.png"); // 苹果背景
    }
    else {
        m_bgPixmap.load(":/img/mole_dlg_bg.bmp");  // 默认为鼠背景
    }

    if (!m_bgPixmap.isNull()) {
        setFixedSize(m_bgPixmap.size());
    }
    else {
        setFixedSize(400, 250);
    }

    setupUI();
}

void GameResultDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 60, 40, 40);
    mainLayout->setSpacing(10);

    // 文本区域
    m_messageLabel = new QLabel(this);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 18px; font-weight: bold; color: #5D4037;");

    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("font-family: 'Arial'; font-size: 14px; font-weight: bold; color: #E65100;");

    mainLayout->addWidget(m_messageLabel);
    mainLayout->addWidget(m_scoreLabel);
    mainLayout->addStretch();

    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    btnLayout->addStretch();

    // 定义资源基础路径
    QString replayBase, nextBase, endBase;
    QString suffix; // 后缀名 (.bmp 或 .png)

    if (m_currentTheme == Theme_Apple) {
        // 苹果主题资源
        replayBase = ":/img/apple_dlg_replay";
        nextBase = ":/img/apple_dlg_next";
        endBase = ":/img/apple_dlg_end";
        suffix = ".png";
    }
    else {
        // 鼠主题资源
        replayBase = ":/img/mole_dlg_replay";
        nextBase = ":/img/mole_dlg_next";
        endBase = ":/img/mole_dlg_end";
        suffix = ".bmp";
    }

    // 辅助 Lambda：根据基础名生成三种状态的路径
    auto createButton = [&](const QString& baseName, QWidget* parent) -> ImageButton* {
        return new ImageButton(
            baseName + suffix,                  // Normal: xxx.png
            baseName + "_hover" + suffix,       // Hover:  xxx_hover.png
            baseName + "_pressed" + suffix,     // Pressed: xxx_pressed.png
            parent
        );
        };

    // 1. 继续/重玩按钮
    m_btnReplay = createButton(replayBase, this);
    connect(m_btnReplay, &ImageButton::clicked, this, &GameResultDialog::onReplayClicked);

    // 2. 下一关按钮
    m_btnNext = createButton(nextBase, this);
    connect(m_btnNext, &ImageButton::clicked, this, &GameResultDialog::onNextLevelClicked);

    // 3. 结束按钮
    m_btnEnd = createButton(endBase, this);
    connect(m_btnEnd, &ImageButton::clicked, this, &GameResultDialog::onEndClicked);

    btnLayout->addWidget(m_btnReplay);
    btnLayout->addWidget(m_btnNext);
    btnLayout->addWidget(m_btnEnd);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);
}

// ... paintEvent, setGameResult, 槽函数保持不变 ...
void GameResultDialog::setGameResult(int score, bool isWin) {
    m_scoreLabel->setText(QStringLiteral("本局得分: ") + QString::number(score));
    if (isWin) {
        m_messageLabel->setText(QStringLiteral("恭喜，您通过了！"));
        m_btnNext->setVisible(true);
    }
    else {
        m_messageLabel->setText(QStringLiteral("您认输吧！"));
        m_btnNext->setVisible(false);
    }
}

void GameResultDialog::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    }
    else {
        painter.fillRect(rect(), Qt::white);
    }
}

void GameResultDialog::onReplayClicked() { m_selectedAction = Action_Replay; accept(); }
void GameResultDialog::onNextLevelClicked() { m_selectedAction = Action_NextLevel; accept(); }
void GameResultDialog::onEndClicked() { m_selectedAction = Action_End; accept(); }