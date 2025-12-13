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
    m_appleBadPixmap.load(":/img/apple_bad.png");

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

void AppleGame::updateSettings(const AppleSettingsData& settings) {
    m_settings = settings;
}

void AppleGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_caughtCount = 0;
    m_lives = m_settings.failCount;

    qDeleteAll(m_apples);
    m_apples.clear();

    m_basketPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);

    m_spawnTimer = 0;

    // 【修改点 1】：大幅提高初始难度（加快生成频率）
    // 原版: 120帧(2s) -> 现在: 60帧(1s) 起步，每升一级减少 4帧
    // Level 3 (默认) -> 52帧 (不到1秒一波)
    m_spawnInterval = 60 - (m_settings.level - 1) * 4;
    if (m_spawnInterval < 20) m_spawnInterval = 20; // 极限每0.3秒一波

    // 基础速度: Level 0.5 -> 1.5, Level 10 -> 2.3
    m_currentBaseSpeed = 0.5 + (m_settings.level - 1) * 0.2;

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

void AppleGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();

    qDeleteAll(m_apples);
    m_apples.clear();
}

void AppleGame::onGameTick() {
    // 1. 胜利判定
    if (m_caughtCount >= m_settings.targetCount) {
        stopGame();
        emit gameFinished(m_score, true);
        return;
    }

    // 2. 生成逻辑
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        // --- 多重生成逻辑 ---
        int spawnCount = 1;

        // 根据等级计算暴击概率 (1级0%, 3级20%, 10级90%)
        int randomVal = QRandomGenerator::global()->bounded(100);
        int extraChance = (m_settings.level - 1) * 10;

        if (randomVal < extraChance) {
            spawnCount++; // 变成 2 个
        }
        // 高等级(5级以上)才有可能出 3 个
        if (m_settings.level >= 5 && randomVal < (extraChance - 40)) {
            spawnCount++; // 变成 3 个
        }

        for (int i = 0; i < spawnCount; i++) {
            spawnApple();
        }

        m_spawnTimer = 0;

        // --- 【关键修改点】难度控制优化 ---

        // 1. 设置更合理的极限值：最低 35 帧 (约0.6秒一波)，防止太快
        int minInterval = 35;

        // 2. 减缓增速：不再是每次生成都加速，而是有 10% 的概率加速
        // 这样难度增长速度会变慢 10 倍，给玩家更多反应时间
        if (m_spawnInterval > minInterval) {
            if (QRandomGenerator::global()->bounded(10) == 0) {
                m_spawnInterval--;
            }
        }
    }

    // 3. 更新物理
    updateApples();
}

void AppleGame::spawnApple() {
    // 随机X坐标
    int margin = 60;
    int x = QRandomGenerator::global()->bounded(margin, SCREEN_WIDTH - margin);

    char letter = 'A' + QRandomGenerator::global()->bounded(26);

    // 【修改点 3】：增加 Y 轴随机偏移
    // 这样如果一次生成多个，它们会有高低差，不会重叠在一起
    int yOffset = QRandomGenerator::global()->bounded(100); // 0~100 的偏移

    // 速度也有更大波动
    double speedVariance = QRandomGenerator::global()->bounded(1.0); // 0~1.0 波动

    Apple* apple = new Apple(QPointF(x, -50 - yOffset), m_currentBaseSpeed + speedVariance, QString(letter));
    m_apples.append(apple);
}

void AppleGame::updateApples() {
    for (Apple* apple : m_apples) {
        if (!apple->active) continue;

        // 如果已经是烂苹果，只处理停留计时
        if (apple->isBad) {
            apple->removeTimer--;
            if (apple->removeTimer <= 0) {
                apple->active = false; // 时间到，彻底移除
            }
            continue; // 烂苹果不移动，跳过移动逻辑
        }

        // 正常下落
        apple->pos.setY(apple->pos.y() + apple->speed);

        // 落地检测 (接触地面)
        // 假设地面 Y 坐标约为 520 (篮子位置)
        if (apple->pos.y() > 520) {
            // 【修改】变为烂苹果状态，而不是直接 active=false
            apple->isBad = true;
            apple->removeTimer = 30; // 停留 30 帧 (约0.5秒)

            // 扣血逻辑
            m_lives--;

            if (m_lives <= 0) {
                stopGame();
                emit gameFinished(m_score, false);
                return; // 防止崩溃
            }
        }
    }

    // 清理逻辑不变 (active=false 的会被删掉)
    if (m_state != GameState::Playing) return;
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

    // 优先消除离地面最近的
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
        m_caughtCount++;
        m_catchSound->play();
        m_basketPos.setX(target->pos.x());

        emit scoreChanged(m_score);
    }
}

void AppleGame::draw(QPainter& painter) {
    // ... 前面的背景、篮子绘制不变 ...
    painter.setRenderHint(QPainter::Antialiasing);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, m_bgPixmap);
    else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(135, 206, 235));

    double basketY = SCREEN_HEIGHT - 80;
    if (!m_basketPixmap.isNull()) painter.drawPixmap(m_basketPos.x() - m_basketPixmap.width() / 2, basketY, m_basketPixmap);
    else { painter.setBrush(Qt::yellow); painter.drawRect(m_basketPos.x() - 40, basketY, 80, 40); }

    // 绘制苹果
    painter.setFont(QFont("Arial", 16, QFont::Bold));

    for (Apple* apple : m_apples) {
        if (!apple->active) continue;

        // 【修改】根据状态选择图片
        if (apple->isBad) {
            // 绘制烂苹果
            if (!m_appleBadPixmap.isNull()) {
                painter.drawPixmap(apple->pos.x() - m_appleBadPixmap.width() / 2,
                    apple->pos.y() - m_appleBadPixmap.height() / 2,
                    m_appleBadPixmap);
            }
            // 烂苹果不需要画字母
        }
        else {
            // 绘制正常苹果
            if (!m_applePixmap.isNull()) {
                painter.drawPixmap(apple->pos.x() - m_applePixmap.width() / 2,
                    apple->pos.y() - m_applePixmap.height() / 2,
                    m_applePixmap);
            }
            // 绘制字母
            painter.setPen(Qt::white);
            QRect textRect(apple->pos.x() - 20, apple->pos.y() - 20, 40, 40);
            painter.drawText(textRect, Qt::AlignCenter, apple->letter);
        }
    }

    // ... UI 文字绘制不变 ...
    painter.setPen(Qt::black);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    QString lifeStr = QStringLiteral("生命: ");
    painter.drawText(20, 40, lifeStr + QString::number(m_lives));
    QString targetStr = QStringLiteral("目标: %1/%2").arg(m_caughtCount).arg(m_settings.targetCount);
    painter.drawText(20, 70, targetStr);
    QString scoreStr = QStringLiteral("得分: ");
    painter.drawText(SCREEN_WIDTH - 150, 40, scoreStr + QString::number(m_score));
}