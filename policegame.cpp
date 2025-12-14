#include "policegame.h"
#include "datamanager.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QFile>

const int GAME_FPS = 60;
const double SCREEN_WIDTH = 800.0;
const double SCREEN_HEIGHT = 600.0;
const double START_GAP = 300.0;
const double MAP_SCALE = 1.3;

PoliceGame::PoliceGame(QObject* parent) : GameBase(parent) {
    m_pathPoints.clear();
    m_totalMapLength = 1000.0; 
    m_currentIndex = 0;
    m_isTypingError = false;
    m_direction = 1; // 默认正向 (1: 顺时针/前进, -1: 逆时针/后退)
    m_playerSpeed = 0.0;
    m_playerBaseSpeed = 0.0;
    m_enemySpeed = 0.0;
    m_playerDistance = 0.0;
    m_enemyDistance = 0.0;

    loadResources();
    initMapPath();

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &PoliceGame::onGameTick);

    // 默认设置初始化
    m_settings.role = 0;    // 警察
    m_settings.vehicle = 0; // 汽车
    m_settings.articleName = ""; // 随机
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

    m_policeSprites[0].load(":/img/police_0_3_0.png");
    m_policeSprites[1].load(":/img/police_0_3_1.png");
    m_policeSprites[2].load(":/img/police_0_3_2.png");
    m_policeSprites[3].load(":/img/police_0_3_3.png");

    m_thiefSprites[0].load(":/img/police_1_3_0.png");
    m_thiefSprites[1].load(":/img/police_1_3_1.png");
    m_thiefSprites[2].load(":/img/police_1_3_2.png");
    m_thiefSprites[3].load(":/img/police_1_3_3.png");

    // 资源加载容错保护
    if (m_policeSprites[0].isNull()) {
        QPixmap temp(80, 40); temp.fill(Qt::blue);
        for (int i = 0; i < 4; i++) m_policeSprites[i] = temp;
    }
    if (m_thiefSprites[0].isNull()) {
        QPixmap temp(80, 40); temp.fill(Qt::red);
        for (int i = 0; i < 4; i++) m_thiefSprites[i] = temp;
    }
}

void PoliceGame::loadArticle(const QString& filename) {
    QString targetFile = filename;
    bool loadSuccess = false;

    if (!targetFile.isEmpty()) {
        QString fullPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General/" + targetFile;
        // 如果没有后缀，补上 .txt 
        if (!fullPath.endsWith(".txt", Qt::CaseInsensitive)) {
            fullPath += ".txt";
        }

        QFile file(fullPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            // 简单转码，这里假设是 UTF-8 或 Local8Bit
            m_targetText = QString::fromLocal8Bit(data);
            // 文本清洗：去除换行符，简化空白
            m_targetText = m_targetText.simplified();
            if (!m_targetText.isEmpty()) {
                loadSuccess = true;
            }
            file.close();
        }
    }

    if (!loadSuccess) {
        static bool isDataLoaded = false;
        if (!isDataLoaded) {
            QString dataPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General";
            DataManager::instance().loadArticlesFromDir(dataPath);
            isDataLoaded = true;
        }
        m_targetText = DataManager::instance().getRandomArticle().simplified();
    }

    if (m_targetText.isEmpty()) m_targetText = "Ready Go";
    if (m_targetText.length() > 300) m_targetText = m_targetText.left(300);

    m_currentIndex = 0;
}

void PoliceGame::initMapPath() {
    m_pathPoints.clear();
    // 闭环地图点
    m_pathPoints << QPointF(40, 800) << QPointF(280, 680) << QPointF(200, 620)
        << QPointF(540, 420) << QPointF(360, 320) << QPointF(620, 200)
        << QPointF(860, 320) << QPointF(1140, 200) << QPointF(1440, 360)
        << QPointF(300, 940) << QPointF(40, 800);

    m_totalMapLength = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF line(m_pathPoints[i], m_pathPoints[i + 1]);
        m_totalMapLength += line.length();
    }
    if (m_totalMapLength <= 1.0) m_totalMapLength = 1000.0;
}

// 获取车辆状态（支持方向翻转）
void PoliceGame::getCarState(double distance, int direction, const QVector<QPixmap>& sprites,
    QPointF& outPos, QPixmap& outSprite)
{
    if (m_totalMapLength <= 0) return;

    while (distance > m_totalMapLength) distance -= m_totalMapLength;
    while (distance < 0) distance += m_totalMapLength;

    double currentDist = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF segment(m_pathPoints[i], m_pathPoints[i + 1]);
        double segmentLen = segment.length();

        if (distance <= currentDist + segmentLen) {
            double ratio = (distance - currentDist) / segmentLen;
            outPos = segment.pointAt(ratio);

            double angle = segment.angle();
            int spriteIndex = 0;
            // 基础方向映射
            if (angle >= 0 && angle < 90) spriteIndex = 2; // 右上
            else if (angle >= 90 && angle < 180) spriteIndex = 0; // 左上
            else if (angle >= 180 && angle < 270) spriteIndex = 1; // 左下
            else spriteIndex = 3; // 右下

            if (direction == -1) {
                spriteIndex = 3 - spriteIndex; // 0<->3, 1<->2
            }

            if (spriteIndex >= 0 && spriteIndex < sprites.size()) {
                outSprite = sprites[spriteIndex];
            }
            return;
        }
        currentDist += segmentLen;
    }
    outPos = m_pathPoints.last();
    if (!sprites.isEmpty()) outSprite = sprites[0];
}

void PoliceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_direction = 1; // 重置为正向

    // 初始化位置：根据角色设定追逐关系
    if (m_settings.role == 0) { // 我是警察
        m_playerDistance = 0.0;
        m_enemyDistance = START_GAP; // 小偷在前方
    }
    else { // 我是小偷
        m_playerDistance = START_GAP;
        m_enemyDistance = 0.0;
    }

    if (m_settings.vehicle == 0) {
        m_playerBaseSpeed = 0.35; // 汽车基础速度较快
    }
    else {
        m_playerBaseSpeed = 0.15; // 自行车基础速度较慢
    }


    m_enemySpeed = 0.30;

    m_playerSpeed = 0.0;
    m_currentIndex = 0;
    m_isTypingError = false;

    loadArticle(m_settings.articleName);
    emit scoreChanged(0);
}

void PoliceGame::startGame() {
    // 逻辑由按键触发，此处仅做状态保护
    if (m_state == GameState::Ready) {
    }
}

void PoliceGame::pauseGame() {} // 禁用暂停

void PoliceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
}

void PoliceGame::onGameTick() {
    m_enemyDistance += (m_enemySpeed * m_direction);

    double totalPlayerSpeed = 0;
    if (m_playerSpeed > 0) {
        totalPlayerSpeed = m_playerBaseSpeed + m_playerSpeed;
        m_playerSpeed *= 0.92; // 阻力衰减
        if (m_playerSpeed < 0.05) m_playerSpeed = 0;
    }
    else {
        totalPlayerSpeed = m_playerBaseSpeed * 0.5; // 基础移动
    }
    m_playerDistance += (totalPlayerSpeed * m_direction);


    double rawDiff = m_enemyDistance - m_playerDistance;

    // 取绝对距离差
    double absDiff = qAbs(rawDiff);

    double uTurnThreshold = m_totalMapLength - 200.0;


    if (absDiff > uTurnThreshold) {
        m_direction *= -1;
    }


    double pMod = m_playerDistance;
    double tMod = m_enemyDistance;
    // 规范化到 [0, 1000]
    while (pMod < 0) pMod += m_totalMapLength;
    while (pMod > m_totalMapLength) pMod -= m_totalMapLength;
    while (tMod < 0) tMod += m_totalMapLength;
    while (tMod > m_totalMapLength) tMod -= m_totalMapLength;

    double loopDist = qAbs(pMod - tMod);
    if (loopDist > m_totalMapLength / 2) loopDist = m_totalMapLength - loopDist;

    double catchRange = 40.0; // 判定相遇的距离

    if (loopDist < catchRange) {
        // 相遇了！
        if (m_settings.role == 0) { // 我是警察
            // 只要相遇就算抓获（因为有掉头机制，相遇即碰撞）
            m_state = GameState::Victory;
            stopGame();
            emit gameFinished(m_score + 1000, true);
        }
        else { // 我是小偷
            // 被警察抓到了
            m_state = GameState::GameOver;
            stopGame();
            emit gameFinished(m_score, false);
        }
    }
}

void PoliceGame::draw(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF playerPos, enemyPos;
    QPixmap playerSprite, enemySprite;
    const auto& mySprites = (m_settings.role == 0) ? m_policeSprites : m_thiefSprites;
    const auto& targetSprites = (m_settings.role == 0) ? m_thiefSprites : m_policeSprites;

    getCarState(m_playerDistance, m_direction, mySprites, playerPos, playerSprite);
    getCarState(m_enemyDistance, m_direction, targetSprites, enemyPos, enemySprite);

    painter.save();

    // 摄像机
    painter.translate(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    painter.scale(MAP_SCALE, MAP_SCALE);
    painter.translate(-playerPos);

    // 背景填充
    painter.fillRect(-8000, -8000, 16000, 16000, QColor(34, 139, 34));

    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_bgPixmap);
    }
    else {
        painter.setPen(QPen(Qt::gray, 50));
        painter.drawPolyline(m_pathPoints.data(), m_pathPoints.size());
    }

    // 绘制角色
    if (!enemySprite.isNull())
        painter.drawPixmap(enemyPos.x() - enemySprite.width() / 2, enemyPos.y() - enemySprite.height() / 2, enemySprite);

    // 玩家光圈
    painter.setPen(QPen(Qt::yellow, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(playerPos, 45, 25);

    if (!playerSprite.isNull())
        painter.drawPixmap(playerPos.x() - playerSprite.width() / 2, playerPos.y() - playerSprite.height() / 2, playerSprite);

    painter.restore();

    int uiH = 150;
    int inputBgY = SCREEN_HEIGHT - 100;

    if (!m_uiInputBg.isNull()) {
        int bgW = m_uiInputBg.width();
        int bgH = m_uiInputBg.height();
        inputBgY = SCREEN_HEIGHT - bgH - 20;
        painter.drawPixmap((SCREEN_WIDTH - bgW) / 2, inputBgY, m_uiInputBg);
    }

    painter.setFont(QFont("Arial", 16, QFont::Bold));
    // 文本位置调整
    int textStartX = (SCREEN_WIDTH - 600) / 2 + 30;
    int textY = inputBgY + 45;

    int totalLen = m_targetText.length();
    if (totalLen == 0) totalLen = 1;

    int showLen = 45;
    int startIdx = qMax(0, m_currentIndex - 15);

    QString typeStr = m_targetText.mid(startIdx, m_currentIndex - startIdx);

    painter.setPen(Qt::black);
    painter.drawText(textStartX, textY, typeStr);

    int typedWidth = painter.fontMetrics().horizontalAdvance(typeStr);
    QString remainStr = m_targetText.mid(m_currentIndex, showLen - (m_currentIndex - startIdx));

    painter.setPen(QColor(60, 60, 60));
    painter.drawText(textStartX + typedWidth, textY, remainStr);

    if (m_isTypingError) {
        painter.fillRect(textStartX + typedWidth, textY - 22, 3, 28, Qt::red);
    }
    else {
        painter.fillRect(textStartX + typedWidth, textY - 22, 3, 28, Qt::blue);
    }

    if (!m_uiProgressBar.isNull()) {
        int barX = textStartX;
        int barY = textY + 20;
        int barMaxW = 500;

        painter.fillRect(barX, barY + 4, barMaxW, 4, Qt::lightGray);

        double progress = (double)m_currentIndex / totalLen;
        int sliderX = barX + (int)(progress * barMaxW);

        painter.drawPixmap(sliderX, barY, m_uiProgressBar);
    }
}

void PoliceGame::handleKeyPress(QKeyEvent* event) {
    if (m_state == GameState::Ready) {
        m_state = GameState::Playing;
        m_physicsTimer->start();
    }
    if (m_state != GameState::Playing) return;

    QString text = event->text();
    // 严密的空值检查
    if (text.isEmpty() || m_targetText.isEmpty()) return;
    if (m_currentIndex >= m_targetText.length()) return;

    QChar targetChar = m_targetText.at(m_currentIndex);
    QChar inputChar = text.at(0);
    bool match = (inputChar == targetChar);

    if (!match && targetChar.isSpace()) {
        if (inputChar == ' ' || inputChar == '\r') match = true;
    }

    if (match) {
        m_currentIndex++;
        m_score += 10;
        m_isTypingError = false;
        // 打对加速
        m_playerSpeed += 0.4;

        if (m_currentIndex >= m_targetText.length()) {
            if (m_settings.role == 1) { // 小偷：打完字即胜利
                m_state = GameState::Victory;
                stopGame();
                emit gameFinished(m_score + 2000, true);
            }
            else { // 警察：继续追
                m_currentIndex = 0;
                // 继续加载下一段
                loadArticle(m_settings.articleName);
                m_playerSpeed += 3.0; // 奖励加速
            }
        }
    }
    else {
        m_isTypingError = true;
        m_playerSpeed = qMax(0.0, m_playerSpeed - 1.5);
    }
}