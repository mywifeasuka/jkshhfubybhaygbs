    #include "applegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

// 游戏配置
const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FLOOR_Y = 520; // 判定落地的Y坐标 (篮子大概在这个高度)

AppleGame::AppleGame(QObject *parent) : GameBase(parent) {
    // 1. 加载资源 (请确保 qrc 中有这些图片)
    // 注意：根据之前的资源列表，背景图路径可能需要根据你的实际 qrc 调整
    m_bgPixmap.load(":/img/apple_background.png");
    m_applePixmap.load(":/img/apple_normal.png");
    m_basketPixmap.load(":/img/apple_basket.png");

    // 2. 音效
    m_catchSound = new QSoundEffect(this);
    m_catchSound->setSource(QUrl::fromLocalFile(":/snd/apple_in.wav"));
    
    m_bgMusic = new QSoundEffect(this);
    m_bgMusic->setSource(QUrl::fromLocalFile(":/snd/apple_bg.wav"));
    m_bgMusic->setLoopCount(QSoundEffect::Infinite);

    // 3. 物理定时器 (60 FPS)
    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &AppleGame::onGameTick);
}

AppleGame::~AppleGame() {
    qDeleteAll(m_apples);
    m_apples.clear();
}

void AppleGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_lives = 5; // 初始5条命
    
    qDeleteAll(m_apples);
    m_apples.clear();

    // 篮子初始在中间
    m_basketPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);
    
    m_spawnTimer = 0;
    m_spawnInterval = 100; // 初始约1.6秒生成一个
    m_currentBaseSpeed = 1.5; // 初始速度 (像素/帧)

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
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

void AppleGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();
}

void AppleGame::onGameTick() {
    // 1. 生成逻辑
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnApple();
        m_spawnTimer = 0;
        
        // 难度动态增加逻辑：
        // 每生成一个，间隔缩短一点点，最快 30帧(0.5秒)一个
        if (m_spawnInterval > 40) m_spawnInterval -= 1; 
        
        // 速度也会微量增加
        if (m_currentBaseSpeed < 5.0) m_currentBaseSpeed += 0.02;
    }

    // 2. 更新所有苹果位置
    updateApples();
}

void AppleGame::spawnApple() {
    // 随机X坐标，留出边距防止画在屏幕外
    int margin = 60;
    int x = QRandomGenerator::global()->bounded(margin, SCREEN_WIDTH - margin);
    
    // 随机字母
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    
    // 生成：Y坐标从 -50 开始掉落
    // 速度在基础速度上有微小波动，显得更自然
    double speedVariance = QRandomGenerator::global()->bounded(0.5);
    Apple* apple = new Apple(QPointF(x, -50), m_currentBaseSpeed + speedVariance, QString(letter));
    
    m_apples.append(apple);
}

void AppleGame::updateApples() {
    for (Apple* apple : m_apples) {
        if (!apple->active) continue;

        // *** 核心修改：匀速下落 ***
        apple->pos.setY(apple->pos.y() + apple->speed);

        // 落地检测 (漏接)
        if (apple->pos.y() > SCREEN_HEIGHT) {
            apple->active = false;
            m_lives--;
            
            // 这里可以加一个 "apple_bad" 的音效
            
            if (m_lives <= 0) {
                stopGame();
                emit gameFinished(m_score, false); // false 代表失败
            }
        }
    }

    // 内存清理：移除不活跃的苹果
    for (auto it = m_apples.begin(); it != m_apples.end(); ) {
        if (!(*it)->active) {
            delete *it;
            it = m_apples.erase(it);
        } else {
            ++it;
        }
    }
}

void AppleGame::handleKeyPress(QKeyEvent *event) {
    if (m_state != GameState::Playing) return;

    QString text = event->text().toUpper();
    if (text.isEmpty()) return;

    // 寻找匹配的苹果
    // 策略：如果有多个相同的字母，优先消除离地面最近的那个 (Y值最大)
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
        // 接住成功
        target->active = false; // 标记为死亡，下一帧清理
        m_score += 10;
        m_catchSound->play();
        
        // 移动篮子到该苹果的 X 轴位置，增加视觉反馈
        m_basketPos.setX(target->pos.x());
        
        emit scoreChanged(m_score);
    }
}

void AppleGame::draw(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_bgPixmap);
    } else {
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(135, 206, 235)); // 没图就画个天蓝色
    }

    // 2. 绘制篮子 (在底部跟随)
    // Y轴固定在底部附近
    double basketY = SCREEN_HEIGHT - 80;
    if (!m_basketPixmap.isNull()) {
        painter.drawPixmap(m_basketPos.x() - m_basketPixmap.width()/2, 
                           basketY, 
                           m_basketPixmap);
    } else {
        // 没图画个框
        painter.setBrush(Qt::yellow);
        painter.drawRect(m_basketPos.x() - 40, basketY, 80, 40);
    }

    // 3. 绘制苹果
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    
    for (Apple* apple : m_apples) {
        if (!apple->active) continue;

        // 绘制苹果图片
        if (!m_applePixmap.isNull()) {
            painter.drawPixmap(apple->pos.x() - m_applePixmap.width()/2, 
                               apple->pos.y() - m_applePixmap.height()/2, 
                               m_applePixmap);
        } else {
            painter.setBrush(Qt::red);
            painter.drawEllipse(apple->pos, 20, 20);
        }
        
        // 绘制字母 (白色，居中)
        painter.setPen(Qt::white);
        QRect textRect(apple->pos.x() - 20, apple->pos.y() - 20, 40, 40);
        painter.drawText(textRect, Qt::AlignCenter, apple->letter);
    }

    // 4. UI 信息 (使用 QStringLiteral 避免中文乱码)
    painter.setPen(Qt::black);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    
    // 左上角显示生命
    QString lifeStr = QStringLiteral("生命值: ");
    painter.drawText(20, 40, lifeStr + QString::number(m_lives));
    
    // 右上角显示分数
    QString scoreStr = QStringLiteral("得分: ");
    painter.drawText(SCREEN_WIDTH - 150, 40, scoreStr + QString::number(m_score));
}