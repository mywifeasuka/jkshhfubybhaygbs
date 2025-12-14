#include "gameresultdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>

GameResultDialog::GameResultDialog(GameTheme theme, QWidget* parent)
    : QDialog(parent), m_currentTheme(theme), m_selectedAction(Action_None), m_role(0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    QString bgPath;
    if (m_currentTheme == Theme_Apple) {
        bgPath = ":/img/apple_dlg_bg.png";
    }
    else if (m_currentTheme == Theme_Frog) {
        bgPath = ":/img/frog_dlg_bg.png";
    }
    else if (m_currentTheme == Theme_Mole) {
        bgPath = ":/img/mole_dlg_bg.bmp";
    }

    if (!bgPath.isEmpty()) {
        m_bgPixmap.load(bgPath);
        if (!m_bgPixmap.isNull()) setFixedSize(m_bgPixmap.size());
        else setFixedSize(400, 250);
    }
    else {
        setFixedSize(400, 250);
    }

    setupUI();
}

void GameResultDialog::setRole(int role) {
    m_role = role;
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
        replayBase = ":/img/frog_dlg_replay";
        nextBase = ":/img/frog_dlg_next";
        endBase = ":/img/frog_dlg_end";
        suffix = ".png";
    }
    else {
        replayBase = ":/img/mole_dlg_replay";
        nextBase = ":/img/mole_dlg_next";
        endBase = ":/img/mole_dlg_end";
        suffix = ".bmp";
    }

    auto createButton = [&](const QString& baseName, QWidget* parent) -> ImageButton* {
        return new ImageButton(
            baseName + suffix,
            baseName + "_hover" + suffix,
            baseName + "_pressed" + suffix,
            parent
        );
        };

    m_btnReplay = createButton(replayBase, this);
    connect(m_btnReplay, &ImageButton::clicked, this, &GameResultDialog::onReplayClicked);

    m_btnNext = createButton(nextBase, this);
    connect(m_btnNext, &ImageButton::clicked, this, &GameResultDialog::onNextLevelClicked);

    m_btnEnd = createButton(endBase, this);
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
        m_messageLabel->setText(QStringLiteral("恭喜，您通过了！"));
        m_btnNext->setVisible(true);
    }
    else {
        m_messageLabel->setText(QStringLiteral("您认输吧！"));
        m_btnNext->setVisible(false);
    }

    if (m_currentTheme == Theme_Police) {
        QString bgPath;

        if (m_role == 0) { // 警察
            bgPath = isWin ? ":/img/police_win_0.png" : ":/img/police_lost_0.png";
        }
        else { // 小偷
            bgPath = isWin ? ":/img/police_win_1.png" : ":/img/police_lost_1.png";
        }

        if (!bgPath.isEmpty()) {
            m_bgPixmap.load(bgPath);
            if (!m_bgPixmap.isNull()) {
                setFixedSize(m_bgPixmap.size()); // 调整窗口大小适应背景
                update(); // 强制重绘
            }
        }

        m_btnNext->setVisible(false);

        m_messageLabel->setVisible(false);

        m_scoreLabel->setStyleSheet(
            "font-family: 'Arial'; font-size: 16px; font-weight: bold; "
            "color: white; background-color: rgba(0, 0, 0, 150); "
            "border-radius: 4px; padding: 4px;"
        );
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