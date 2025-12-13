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
    QString bgPath;
    if (m_currentTheme == Theme_Apple) {
        bgPath = ":/img/apple_dlg_bg.png";
    }
    else if (m_currentTheme == Theme_Frog) {
        bgPath = ":/img/frog_dlg_bg.png"; // 【新增】
    }
    else {
        bgPath = ":/img/mole_dlg_bg.bmp";
    }
    m_bgPixmap.load(bgPath);

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
    QString suffix;

    if (m_currentTheme == Theme_Apple) {
        replayBase = ":/img/apple_dlg_replay";
        nextBase = ":/img/apple_dlg_next";
        endBase = ":/img/apple_dlg_end";
        suffix = ".png";
    }
    else if (m_currentTheme == Theme_Frog) {
        // 【新增】激流勇进资源
        replayBase = ":/img/frog_dlg_replay";
        nextBase = ":/img/frog_dlg_next"; // 注意：虽然只有成功界面，但可能仍需要“下一关”或“重玩”
        endBase = ":/img/frog_end";      // 或者是 frog_dlg_end
        suffix = ".png";

        // 修正：根据资源列表，激流勇进的按钮似乎没有统一的 dlg 前缀？
        // 让我们检查资源... 
        // 资源列表中有 frog_dlg_replay.png, frog_dlg_next.png, frog_dlg_end.png
        // 确认无误。
        endBase = ":/img/frog_dlg_end";
    }
    else {
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