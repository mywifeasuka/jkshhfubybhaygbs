#include "policegame.h"
#include "datamanager.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

const int GAME_FPS = 60;
const double SCREEN_WIDTH = 800.0;
const double SCREEN_HEIGHT = 600.0;
const double START_GAP = 400.0; // 初始追逐间距

PoliceGame::PoliceGame(QObject* parent) : GameBase(parent) {
    loadResources();
    initMapPath();

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &PoliceGame::onGameTick);

    // 默认配置
    m_settings.role = 0; // 警察
    m_settings.vehicle = 0; // 汽车
    m_settings.difficulty = 3;
}

PoliceGame::~PoliceGame() {}

void PoliceGame::updateSettings(const PoliceSettingsData& settings) {
    m_settings = settings;
}

void PoliceGame::loadResources() {
    m_bgPixmap.load(":/img/police_background.png");
    m_uiInputBg.load(":/img/police_input.png");
    m_uiProgressBar.load(":/img/police_blue.png");

    m_policeSprites.resize(4);
    m_thiefSprites.resize(4);

    // 加载资源 (根据资源列表)
    m_policeSprites[0].load(":/img/police_0_3_0.png");
    m_policeSprites[1].load(":/img/police_0_3_1.png");
    m_policeSprites[2].load(":/img/police_0_3_2.png");
    m_policeSprites[3].load(":/img/police_0_3_3.png");

    m_thiefSprites[0].load(":/img/police_1_3_0.png");
    m_thiefSprites[1].load(":/img/police_1_3_1.png");
    m_thiefSprites[2].load(":/img/police_1_3_2.png");
    m_thiefSprites[3].load(":/img/police_1_3_3.png");

    // 资源加载防崩溃保护
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
    // 确保只加载一次文章库
    static bool isLoaded = false;
    if (!isLoaded) {
        QString dataPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General";
        DataManager::instance().loadArticlesFromDir(dataPath);
        isLoaded = true;
    }
    m_targetText = DataManager::instance().getRandomArticle();

    // 清洗文本，去除多余换行，限制最大长度防止溢出
    m_targetText = m_targetText.simplified();
    if (m_targetText.length() > 300) m_targetText = m_targetText.left(300);

    m_currentIndex = 0;
}

void PoliceGame::initMapPath() {
    m_pathPoints.clear();
    // 定义地图路径点 (闭环)
    m_pathPoints << QPointF(40, 800) << QPointF(280, 680) << QPointF(200, 620)
        << QPointF(540, 420) << QPointF(360, 320) << QPointF(620, 200)
        << QPointF(860, 320) << QPointF(1140, 200) << QPointF(1440, 360)
        << QPointF(300, 940) << QPointF(40, 800);

    m_totalMapLength = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF line(m_pathPoints[i], m_pathPoints[i + 1]);
        m_totalMapLength += line.length();
    }
}

void PoliceGame::getCarState(double distance, const QVector<QPixmap>& sprites,
    QPointF& outPos, QPixmap& outSprite)
{
    // 处理地图循环逻辑
    while (distance > m_totalMapLength) distance -= m_totalMapLength;
    while (distance < 0) distance += m_totalMapLength;

    double currentDist = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF segment(m_pathPoints[i], m_pathPoints[i + 1]);
        double segmentLen = segment.length();

        if (distance <= currentDist + segmentLen) {
            double ratio = (distance - currentDist) / segmentLen;
            outPos = segment.pointAt(ratio);

            double angle = segment.angle(); // 0-360
            int spriteIndex = 0;
            // 简单根据角度选择4个方向的贴图
            if (angle >= 0 && angle < 90) spriteIndex = 2; // 右上
            else if (angle >= 90 && angle < 180) spriteIndex = 0; // 左上
            else if (angle >= 180 && angle < 270) spriteIndex = 1; // 左下
            else spriteIndex = 3; // 右下

            if (spriteIndex >= 0 && spriteIndex < sprites.size()) {
                outSprite = sprites[spriteIndex];
            }
            return;
        }
        currentDist += segmentLen;
    }
    // 默认情况
    outPos = m_pathPoints.last();
    outSprite = sprites[0];
}

void PoliceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;

    // 初始化位置：如果是警察，我在后；如果是小偷，我在前
    if (m_settings.role == 0) { // 警察
        m_playerDistance = 0.0;
        m_enemyDistance = START_GAP;
    }
    else { // 小偷
        m_playerDistance = START_GAP;
        m_enemyDistance = 0.0;
    }

    // 速度配置
    // 汽车(0)基础速度快，自行车(1)慢
    m_playerBaseSpeed = (m_settings.vehicle == 0) ? 2.5 : 1.5;

    // 敌人速度随难度增加
    m_enemySpeed = 1.0 + (m_settings.difficulty * 0.25);

    m_playerSpeed = 0.0;
    m_currentIndex = 0;
    m_isTypingError = false;

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
    // 1. 移动逻辑
    m_enemyDistance += m_enemySpeed;

    // 玩家速度 = 基础速度(载具) + 打字加速 - 阻力
    if (m_playerSpeed > 0) {
        m_playerDistance += (m_playerBaseSpeed + m_playerSpeed);
        m_playerSpeed *= 0.94; // 模拟阻力减速
        if (m_playerSpeed < 0.1) m_playerSpeed = 0;
    }

    // 2. 胜负判定
    // 距离差 (考虑地图循环，简单处理为线性距离追击)
    double distDiff = m_playerDistance - m_enemyDistance;

    if (m_settings.role == 0) {
        // --- 我是警察 ---
        // 胜利条件：追上小偷 (距离差 >= 0)
        // 失败条件：被小偷甩开太远 (比如一圈 m_totalMapLength) - 这里暂不判负，让玩家打完字
        if (distDiff >= 0 && distDiff < 200) {
            m_state = GameState::Victory;
            stopGame();
            emit gameFinished(m_score + 1000, true);
        }
    }
    else {
        // --- 我是小偷 ---
        // 失败条件：被警察追上 (enemy >= player)
        if (m_enemyDistance >= m_playerDistance) {
            m_state = GameState::GameOver;
            stopGame();
            emit gameFinished(m_score, false);
        }
        // 胜利条件：打完字（在 handleKeyPress 中处理）
    }
}

void PoliceGame::draw(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 计算双方位置
    QPointF playerPos, enemyPos;
    QPixmap playerSprite, enemySprite;

    // 根据角色决定谁用哪套贴图
    const auto& mySprites = (m_settings.role == 0) ? m_policeSprites : m_thiefSprites;
    const auto& targetSprites = (m_settings.role == 0) ? m_thiefSprites : m_policeSprites;

    getCarState(m_playerDistance, mySprites, playerPos, playerSprite);
    getCarState(m_enemyDistance, targetSprites, enemyPos, enemySprite);

    // 2. 摄像机跟随玩家
    QPointF cameraOffset(SCREEN_WIDTH / 2 - playerPos.x(), SCREEN_HEIGHT / 2 - playerPos.y());

    painter.save();
    painter.translate(cameraOffset);

    // 绘制背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_bgPixmap);
    }
    else {
        // Debug 模式画线
        painter.setPen(QPen(Qt::gray, 50));
        painter.drawPolyline(m_pathPoints.data(), m_pathPoints.size());
    }

    // 绘制角色
    if (!enemySprite.isNull())
        painter.drawPixmap(enemyPos.x() - enemySprite.width() / 2, enemyPos.y() - enemySprite.height() / 2, enemySprite);

    // 绘制光圈标记自己
    painter.setPen(QPen(Qt::yellow, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(playerPos, 40, 20);

    if (!playerSprite.isNull())
        painter.drawPixmap(playerPos.x() - playerSprite.width() / 2, playerPos.y() - playerSprite.height() / 2, playerSprite);

    painter.restore();

    // 3. 绘制 UI (HUD)
    int uiY = SCREEN_HEIGHT - 120;

    // 背景图
    if (!m_uiInputBg.isNull()) {
        painter.drawPixmap((SCREEN_WIDTH - m_uiInputBg.width()) / 2, uiY, m_uiInputBg);
    }
    else {
        painter.fillRect(0, uiY + 40, SCREEN_WIDTH, 80, QColor(0, 0, 0, 160));
    }

    // 文字渲染配置
    painter.setFont(QFont("Consolas", 18, QFont::Bold));
    int textStartX = 160;
    int textY = uiY + 65;

    // 计算滚动的文字窗口 (显示当前字符前后的一段)
    int showLen = 45;
    int startIdx = qMax(0, m_currentIndex - 15);
    QString visibleText = m_targetText.mid(startIdx, showLen);

    // 绘制已输入部分 (绿色)
    QString typeStr = m_targetText.mid(startIdx, m_currentIndex - startIdx);
    painter.setPen(Qt::green);
    painter.drawText(textStartX, textY, typeStr);

    // 绘制未输入部分 (白色)
    int typedWidth = painter.fontMetrics().horizontalAdvance(typeStr);
    QString remainStr = m_targetText.mid(m_currentIndex, showLen - (m_currentIndex - startIdx));
    painter.setPen(Qt::white);
    painter.drawText(textStartX + typedWidth, textY, remainStr);

    // 光标指示
    if (m_isTypingError) {
        painter.fillRect(textStartX + typedWidth, textY - 22, 12, 28, Qt::red);
    }
    else {
        painter.fillRect(textStartX + typedWidth, textY - 22, 2, 28, Qt::yellow);
    }

    // 4. 进度条绘制
    if (!m_uiProgressBar.isNull() && m_targetText.length() > 0) {
        int barX = 150;
        int barY = uiY + 80; // 放在文字下方
        int barMaxW = 500;   // 进度条总长

        // 绘制底槽
        painter.fillRect(barX, barY + 5, barMaxW, 4, Qt::gray);

        // 计算滑块位置
        double progress = (double)m_currentIndex / m_targetText.length();
        int sliderX = barX + (int)(progress * barMaxW);

        // 绘制蓝色滑块 (POLICE_BLUE)
        painter.drawPixmap(sliderX, barY, m_uiProgressBar);
    }
}

void PoliceGame::handleKeyPress(QKeyEvent* event) {
    if (m_state != GameState::Playing) return;

    QString text = event->text();
    if (text.isEmpty()) return;

    if (m_currentIndex >= m_targetText.length()) return;

    // 判定输入
    if (text.at(0) == m_targetText.at(m_currentIndex)) {
        m_currentIndex++;
        m_score += 10;
        m_isTypingError = false;

        // 打对获得爆发速度
        m_playerSpeed += 4.0;

        // 检查是否完成文章
        if (m_currentIndex >= m_targetText.length()) {
            if (m_settings.role == 1) { // 我是小偷，打完字即胜利逃脱
                m_state = GameState::Victory;
                stopGame();
                emit gameFinished(m_score + 2000, true);
            }
            else {
                // 我是警察，打完一段没抓到，换文章继续追，并给予大加速
                m_currentIndex = 0;
                loadArticle();
                m_playerSpeed += 15.0;
            }
        }
    }
    else {
        m_isTypingError = true;
        // 打错惩罚：减速
        m_playerSpeed = qMax(0.0, m_playerSpeed - 3.0);
    }
}