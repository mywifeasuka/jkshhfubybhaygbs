#include "applegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

// ... 常量定义保持不变 ...
const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

AppleGame::AppleGame(QObject* parent) : GameBase(parent) {
    // ... 资源加载代码保持不变 ...
    m_bgPixmap.load(":/img/apple_background.png");
    m_applePixmap.load(":/img/apple_normal.png");
    m_basketPixmap.load(":/img/apple_basket.png");

    m_catchSound = new QSoundEffect(this);
    m_catchSound->setSource(QUrl::fromLocalFile(":/snd/apple_in.wav"));

    m_bgMusic = new QSoundEffect(this);
    m_bgMusic->setSource(QUrl::fromLocalFile(":/snd/apple_bg.wav"));
    m_bgMusic->setLoopCount(QSoundEffect::Infinite);

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &AppleGame::onGameTick);
}

AppleGame::~AppleGame() {
    qDeleteAll(m_apples);
    m_apples.clear();
}

// 【新增】更新设置
void AppleGame::updateSettings(const AppleSettingsData& settings) {
    m_settings = settings;
}

void AppleGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_caughtCount = 0; // 重置接住数量

    // 【修改】使用设置中的生命值
    m_lives = m_settings.failCount;

    qDeleteAll(m_apples);
    m_apples.clear();

    m_basketPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);

    m_spawnTimer = 0;

    // 【修改】根据“游戏等级”设置初始难度
    // 等级越高，生成越快，下落越快
    // 假设等级 1-10
    // 生成间隔: Level 1 -> 120帧(2s), Level 10 -> 40帧(0.6s)
    m_spawnInterval = 120 - (m_settings.level - 1) * 8;
    if (m_spawnInterval < 30) m_spawnInterval = 30;

    // 基础速度: Level 1 -> 1.5, Level 10 -> 4.5
    m_currentBaseSpeed = 1.5 + (m_settings.level - 1) * 0.3;

    emit scoreChanged(0);
}

void AppleGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

// ... pauseGame 保持不变 ...
void AppleGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop();
        m_bgMusic->stop();
    }
    else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

// 【修复】结束游戏时清空屏幕
void AppleGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();

    // 清除所有苹果，确保重绘时屏幕干净
    qDeleteAll(m_apples);
    m_apples.clear();
}

void AppleGame::onGameTick() {
    // 1. 胜利判定 (接住数量达到设定值)
    if (m_caughtCount >= m_settings.targetCount) {
        stopGame();
        emit gameFinished(m_score, true); // 胜利！
        return;
    }

    // 2. 生成逻辑
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnApple();
        m_spawnTimer = 0;
        // 动态难度微调 (在当前等级基础上微调)
        // 注意不要减得太快，否则高等级下瞬间没法玩
        if (m_spawnInterval > 20) m_spawnInterval--;
    }

    // 3. 更新物理
    updateApples();
}

void AppleGame::spawnApple() {
    // ... 保持原有逻辑 ...
    int margin = 60;
    int x = QRandomGenerator::global()->bounded(margin, SCREEN_WIDTH - margin);
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    double speedVariance = QRandomGenerator::global()->bounded(0.5);
    Apple* apple = new Apple(QPointF(x, -50), m_currentBaseSpeed + speedVariance, QString(letter));
    m_apples.append(apple);
}

void AppleGame::updateApples() {
    for (Apple* apple : m_apples) {
        if (!apple->active) continue;

        apple->pos.setY(apple->pos.y() + apple->speed);

        // 落地检测 (失败判定)
        if (apple->pos.y() > SCREEN_HEIGHT) {
            apple->active = false;
            m_lives--;
            // 这里可以播放 miss 音效

            if (m_lives <= 0) {
                stopGame();
                emit gameFinished(m_score, false); // 失败
            }
        }
    }

    // 清理
    for (auto it = m_apples.begin(); it != m_apples.end(); ) {
        if (!(*it)->active) {
            delete* it;
            it = m_apples.erase(it);
        }
        else {
            ++it;
        }
    }
}

void AppleGame::handleKeyPress(QKeyEvent* event) {
    if (m_state != GameState::Playing) return;

    QString text = event->text().toUpper();
    if (text.isEmpty()) return;

    Apple* target = nullptr;
    double maxY = -1000.0;

    for (Apple* apple : m_apples) {
        if (apple->active && apple->letter == text) {
            if (apple->pos.y() > maxY) {
                maxY = apple->pos.y();
                target = apple;
            }
        }
    }

    if (target) {
        target->active = false;
        m_score += 10;
        m_caughtCount++; // 增加接住计数
        m_catchSound->play();
        m_basketPos.setX(target->pos.x());

        emit scoreChanged(m_score);
    }
}

void AppleGame::draw(QPainter& painter) {
    // ... 绘图逻辑保持不变 ...
    // 注意：UI绘制可以稍微修改一下，显示"目标"信息
    // ... (背景、篮子、苹果绘制代码不变) ...
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, m_bgPixmap);
    else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(135, 206, 235));

    double basketY = SCREEN_HEIGHT - 80;
    if (!m_basketPixmap.isNull()) painter.drawPixmap(m_basketPos.x() - m_basketPixmap.width() / 2, basketY, m_basketPixmap);
    else { painter.setBrush(Qt::yellow); painter.drawRect(m_basketPos.x() - 40, basketY, 80, 40); }

    painter.setFont(QFont("Arial", 16, QFont::Bold));
    for (Apple* apple : m_apples) {
        if (!apple->active) continue;
        if (!m_applePixmap.isNull()) painter.drawPixmap(apple->pos.x() - m_applePixmap.width() / 2, apple->pos.y() - m_applePixmap.height() / 2, m_applePixmap);
        else { painter.setBrush(Qt::red); painter.drawEllipse(apple->pos, 20, 20); }
        painter.setPen(Qt::white);
        QRect textRect(apple->pos.x() - 20, apple->pos.y() - 20, 40, 40);
        painter.drawText(textRect, Qt::AlignCenter, apple->letter);
    }

    // UI 信息更新
    painter.setPen(Qt::black);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

    // 显示生命和进度
    QString lifeStr = QStringLiteral("生命: ");
    painter.drawText(20, 40, lifeStr + QString::number(m_lives));

    // 显示目标进度
    QString targetStr = QStringLiteral("目标: %1/%2").arg(m_caughtCount).arg(m_settings.targetCount);
    painter.drawText(20, 70, targetStr);

    QString scoreStr = QStringLiteral("得分: ");
    painter.drawText(SCREEN_WIDTH - 150, 40, scoreStr + QString::number(m_score));
}