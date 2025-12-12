#include "policegame.h"
#include "datamanager.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

// 游戏配置
const int GAME_FPS = 60;
const double SCREEN_WIDTH = 800.0;
const double SCREEN_HEIGHT = 600.0;

PoliceGame::PoliceGame(QObject* parent) : GameBase(parent) {
    loadResources(); // 加载资源
    initMapPath();   // 初始化路径

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &PoliceGame::onGameTick);
}

PoliceGame::~PoliceGame() {}

void PoliceGame::loadResources() {
// 1. 加载背景 (使用 qrc 中定义的别名) [cite: 371]
    m_bgPixmap.load(":/img/police_background.png");

    // 初始化数组大小
    m_policeSprites.resize(4);
    m_thiefSprites.resize(4);

// 2. 加载警察车辆 (Police Car 对应资源名为 police_0_3_x) [cite: 371]
    // 0:左上, 1:左下, 2:右上, 3:右下
    m_policeSprites[0].load(":/img/police_0_3_0.png");
    m_policeSprites[1].load(":/img/police_0_3_1.png");
    m_policeSprites[2].load(":/img/police_0_3_2.png");
    m_policeSprites[3].load(":/img/police_0_3_3.png");

// 3. 加载小偷车辆 (Thief Car 对应资源名为 police_1_3_x) [cite: 371]
    m_thiefSprites[0].load(":/img/police_1_3_0.png");
    m_thiefSprites[1].load(":/img/police_1_3_1.png");
    m_thiefSprites[2].load(":/img/police_1_3_2.png");
    m_thiefSprites[3].load(":/img/police_1_3_3.png");

    // 容错：如果图片加载失败，创建一个红/蓝方块代替，防止崩溃
    if (m_policeSprites[0].isNull()) {
        QPixmap temp(80, 40); temp.fill(Qt::blue);
        for (int i = 0; i < 4; i++) m_policeSprites[i] = temp;
    }
    if (m_thiefSprites[0].isNull()) {
        QPixmap temp(80, 40); temp.fill(Qt::red);
        for (int i = 0; i < 4; i++) m_thiefSprites[i] = temp;
    }
}

void PoliceGame::loadArticle() {
    // 第一次运行时加载
    static bool isLoaded = false;
    if (!isLoaded) {
        QString dataPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General";
        DataManager::instance().loadArticlesFromDir(dataPath);
        isLoaded = true;
    }

    m_targetText = DataManager::instance().getRandomArticle();

    // 重置进度
    m_currentIndex = 0;
}

void PoliceGame::initMapPath() {
    m_pathPoints.clear();

    // 根据你描述的坐标录入 [用户描述: 起点在40，800...]
    m_pathPoints << QPointF(40, 800);    // 起点
    m_pathPoints << QPointF(280, 680);   // 路口1 (左转)
    m_pathPoints << QPointF(200, 620);   // 路口2 (右转)
    m_pathPoints << QPointF(540, 420);   // 路口3 (左转)
    m_pathPoints << QPointF(360, 320);   // 路口4 (右转)
    m_pathPoints << QPointF(620, 200);   // 路口5 (右转)
    m_pathPoints << QPointF(860, 320);   // 路口6 (左转)
    m_pathPoints << QPointF(1140, 200);  // 路口7 (右转)
    m_pathPoints << QPointF(1440, 360);  // 路口8 (右转)
    m_pathPoints << QPointF(300, 940);   // 路口9 (右转，大直道)
    m_pathPoints << QPointF(40, 800);    // 回到起点 (闭环)

    // 计算地图总长度
    m_totalMapLength = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF line(m_pathPoints[i], m_pathPoints[i + 1]);
        m_totalMapLength += line.length();
    }
}

void PoliceGame::getCarState(double distance, const QVector<QPixmap>& sprites,
    QPointF& outPos, QPixmap& outSprite)
{
    // 循环处理 distance
    while (distance > m_totalMapLength) distance -= m_totalMapLength;
    while (distance < 0) distance += m_totalMapLength;

    double currentDist = 0;

    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF segment(m_pathPoints[i], m_pathPoints[i + 1]);
        double segmentLen = segment.length();

        if (distance <= currentDist + segmentLen) {
            // 线性插值计算坐标
            double ratio = (distance - currentDist) / segmentLen;
            outPos = segment.pointAt(ratio);

            // 计算角度 (Qt 坐标系中: 0=右, 90=上, 180=左, 270=下)
            // 注意: QLineF::angle() 返回 0-360
            double angle = segment.angle();

            // 根据角度选择贴图索引
            // 0:左上(90-180), 1:左下(180-270), 2:右上(0-90), 3:右下(270-360)
            int spriteIndex = 0;

            if (angle >= 0 && angle < 90) {
                spriteIndex = 2; // 右上
            }
            else if (angle >= 90 && angle < 180) {
                spriteIndex = 0; // 左上
            }
            else if (angle >= 180 && angle < 270) {
                spriteIndex = 1; // 左下
            }
            else {
                spriteIndex = 3; // 右下
            }

            // 边界检查
            if (spriteIndex >= 0 && spriteIndex < sprites.size()) {
                outSprite = sprites[spriteIndex];
            }
            return;
        }
        currentDist += segmentLen;
    }

    // 默认回退
    outPos = m_pathPoints.last();
    outSprite = sprites[3];
}

void PoliceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;

    m_playerDistance = 0.0;
    m_enemyDistance = 400.0; // 敌人领先一段距离

    m_playerSpeed = 0.0;
    m_enemySpeed = 2.0;      // 敌人恒定速度

    m_currentIndex = 0;
    m_isTypingError = false;

    //m_targetText = "The quick brown fox jumps over the lazy dog.";
    loadArticle();
    emit scoreChanged(0);
}

void PoliceGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_physicsTimer->start();
    }
}

void PoliceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop();
    }
    else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_physicsTimer->start();
    }
}

void PoliceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
}

void PoliceGame::onGameTick() {
    // 移动逻辑
    m_enemyDistance += m_enemySpeed;
    m_playerDistance += m_playerSpeed;

    // 阻力
    m_playerSpeed *= 0.96;
    if (m_playerSpeed < 0.1) m_playerSpeed = 0;

    // 胜负判定
    if (m_playerDistance >= m_enemyDistance) {
        m_state = GameState::Victory;
        stopGame();
        emit gameFinished(m_score + 500, true);
    }
}

void PoliceGame::draw(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 获取状态
    QPointF playerPos, enemyPos;
    QPixmap playerSprite, enemySprite;

    getCarState(m_playerDistance, m_policeSprites, playerPos, playerSprite);
    getCarState(m_enemyDistance, m_thiefSprites, enemyPos, enemySprite);

    // 2. 摄像机逻辑 (以玩家为中心)
    QPointF cameraOffset(SCREEN_WIDTH / 2 - playerPos.x(), SCREEN_HEIGHT / 2 - playerPos.y());

    painter.save();
    painter.translate(cameraOffset);

    // 3. 绘制背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_bgPixmap);
    }
    else {
        // Debug 模式: 画线
        painter.setPen(QPen(Qt::gray, 100));
        painter.drawPolyline(m_pathPoints.data(), m_pathPoints.size());
    }

    // 4. 绘制车辆 (居中绘制)
    // 敌人
    if (!enemySprite.isNull()) {
        painter.drawPixmap(enemyPos.x() - enemySprite.width() / 2,
            enemyPos.y() - enemySprite.height() / 2,
            enemySprite);
    }

    // 玩家
    if (!playerSprite.isNull()) {
        painter.drawPixmap(playerPos.x() - playerSprite.width() / 2,
            playerPos.y() - playerSprite.height() / 2,
            playerSprite);
    }

    painter.restore(); // 结束摄像机

    // 5. 绘制 UI
    QRect uiRect(0, 480, 800, 120);
    painter.fillRect(uiRect, QColor(0, 0, 0, 180));
    painter.setFont(QFont("Arial", 18, QFont::Bold)); // 用通用字体防止乱码

    int textX = 50; int textY = 550;
    QString typed = m_targetText.left(m_currentIndex);

    painter.setPen(Qt::green);
    painter.drawText(textX, textY, typed);

    int w = painter.fontMetrics().horizontalAdvance(typed);
    painter.setPen(Qt::white);
    painter.drawText(textX + w, textY, m_targetText.mid(m_currentIndex));

    painter.fillRect(textX + w, textY - 20, 3, 25, m_isTypingError ? Qt::red : Qt::yellow);
}

void PoliceGame::handleKeyPress(QKeyEvent* event) {
    if (m_state != GameState::Playing) return;
    QString text = event->text();
    if (text.isEmpty()) return;

    if (text.at(0) == m_targetText.at(m_currentIndex)) {
        m_currentIndex++;
        m_score += 10;
        m_playerSpeed += 8.0; // 打对加速
        m_isTypingError = false;
        emit scoreChanged(m_score);

        if (m_currentIndex >= m_targetText.length()) {
            m_currentIndex = 0;
            m_playerSpeed += 20.0;
        }
    }
    else {
        m_isTypingError = true;
        m_playerSpeed *= 0.8;
    }
}