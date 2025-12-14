#include "policegame.h"
#include "datamanager.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

const int GAME_FPS = 60;
const double SCREEN_WIDTH = 800.0;
const double SCREEN_HEIGHT = 600.0;
const double START_GAP = 350.0;
const double MAP_SCALE = 1.5;

PoliceGame::PoliceGame(QObject* parent) : GameBase(parent) {
    // 初始化变量，防止野指针
    m_pathPoints.clear();
    m_totalMapLength = 1000.0;
    m_currentIndex = 0;
    m_isTypingError = false;
    m_playerSpeed = 0;
    m_playerDistance = 0;
    m_enemyDistance = 0;

    loadResources();
    initMapPath();

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &PoliceGame::onGameTick);

    // 默认配置
    m_settings.role = 0;
    m_settings.vehicle = 0;
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

    m_policeSprites[0].load(":/img/police_0_3_0.png");
    m_policeSprites[1].load(":/img/police_0_3_1.png");
    m_policeSprites[2].load(":/img/police_0_3_2.png");
    m_policeSprites[3].load(":/img/police_0_3_3.png");

    m_thiefSprites[0].load(":/img/police_1_3_0.png");
    m_thiefSprites[1].load(":/img/police_1_3_1.png");
    m_thiefSprites[2].load(":/img/police_1_3_2.png");
    m_thiefSprites[3].load(":/img/police_1_3_3.png");

    // 资源容错
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
    static bool isLoaded = false;
    if (!isLoaded) {
        QString dataPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General";
        DataManager::instance().loadArticlesFromDir(dataPath);
        isLoaded = true;
    }

    m_targetText = DataManager::instance().getRandomArticle();

    // 【关键修复】确保文本非空且经过清洗
    m_targetText = m_targetText.simplified();
    if (m_targetText.isEmpty() || m_targetText.isNull()) {
        m_targetText = "Ready Go";
    }

    // 限制长度
    if (m_targetText.length() > 300) m_targetText = m_targetText.left(300);

    m_currentIndex = 0;
}

void PoliceGame::initMapPath() {
    m_pathPoints.clear();
    m_pathPoints << QPointF(40, 800) << QPointF(280, 680) << QPointF(200, 620)
        << QPointF(540, 420) << QPointF(360, 320) << QPointF(620, 200)
        << QPointF(860, 320) << QPointF(1140, 200) << QPointF(1440, 360)
        << QPointF(300, 940) << QPointF(40, 800);

    m_totalMapLength = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF line(m_pathPoints[i], m_pathPoints[i + 1]);
        m_totalMapLength += line.length();
    }
    if (m_totalMapLength <= 1.0) m_totalMapLength = 1000.0; // 防止除以零
}

void PoliceGame::getCarState(double distance, const QVector<QPixmap>& sprites,
    QPointF& outPos, QPixmap& outSprite)
{
    // 防止 distance 异常
    if (m_totalMapLength <= 0) return;

    int safeLoop = 0;
    while (distance > m_totalMapLength) {
        distance -= m_totalMapLength;
        if (++safeLoop > 100) { distance = 0; break; }
    }
    while (distance < 0) {
        distance += m_totalMapLength;
        if (++safeLoop > 100) { distance = 0; break; }
    }

    double currentDist = 0;
    for (int i = 0; i < m_pathPoints.size() - 1; ++i) {
        QLineF segment(m_pathPoints[i], m_pathPoints[i + 1]);
        double segmentLen = segment.length();

        if (distance <= currentDist + segmentLen) {
            double ratio = (distance - currentDist) / segmentLen;
            outPos = segment.pointAt(ratio);
            double angle = segment.angle();
            int spriteIndex = 0;
            if (angle >= 0 && angle < 90) spriteIndex = 2;
            else if (angle >= 90 && angle < 180) spriteIndex = 0;
            else if (angle >= 180 && angle < 270) spriteIndex = 1;
            else spriteIndex = 3;

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

    if (m_settings.role == 0) {
        m_playerDistance = 0.0;
        m_enemyDistance = START_GAP;
    }
    else {
        m_playerDistance = START_GAP;
        m_enemyDistance = 0.0;
    }

    m_playerBaseSpeed = (m_settings.vehicle == 0) ? 0.3 : 0.15;
    m_enemySpeed = 0.35 + (m_settings.difficulty * 0.1);

    m_playerSpeed = 0.0;
    m_currentIndex = 0;
    m_isTypingError = false;

    loadArticle();
    emit scoreChanged(0);
}

void PoliceGame::startGame() {
    if (m_state == GameState::Ready) {
        // 等待按键开始
    }
}

void PoliceGame::pauseGame() {}

void PoliceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
}

void PoliceGame::onGameTick() {
    m_enemyDistance += m_enemySpeed;

    if (m_playerSpeed > 0) {
        m_playerDistance += (m_playerBaseSpeed + m_playerSpeed);
        m_playerSpeed *= 0.92;
        if (m_playerSpeed < 0.05) m_playerSpeed = 0;
    }
    else {
        m_playerDistance += m_playerBaseSpeed * 0.5;
    }

    double rawDiff = m_playerDistance - m_enemyDistance;
    double distDiff = rawDiff;
    if (m_totalMapLength > 0) {
        while (distDiff > m_totalMapLength / 2) distDiff -= m_totalMapLength;
        while (distDiff < -m_totalMapLength / 2) distDiff += m_totalMapLength;
    }

    double catchRange = 80.0;

    if (m_settings.role == 0) {
        if (distDiff >= 0 && distDiff < catchRange) {
            m_state = GameState::Victory;
            stopGame();
            emit gameFinished(m_score + 1000, true);
        }
    }
    else {
        if (distDiff <= 0 && distDiff > -catchRange) {
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

    getCarState(m_playerDistance, mySprites, playerPos, playerSprite);
    getCarState(m_enemyDistance, targetSprites, enemyPos, enemySprite);

    painter.save();
    painter.translate(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    painter.scale(MAP_SCALE, MAP_SCALE);
    painter.translate(-playerPos);

    // 填充底色防止黑边
    painter.fillRect(-8000, -8000, 16000, 16000, QColor(34, 139, 34));

    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_bgPixmap);
    }
    else {
        painter.setPen(QPen(Qt::gray, 50));
        painter.drawPolyline(m_pathPoints.data(), m_pathPoints.size());
    }

    if (!enemySprite.isNull())
        painter.drawPixmap(enemyPos.x() - enemySprite.width() / 2, enemyPos.y() - enemySprite.height() / 2, enemySprite);

    painter.setPen(QPen(Qt::yellow, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(playerPos, 45, 25);

    if (!playerSprite.isNull())
        painter.drawPixmap(playerPos.x() - playerSprite.width() / 2, playerPos.y() - playerSprite.height() / 2, playerSprite);

    painter.restore();

    // UI
    int uiH = 150;
    int inputBgY = SCREEN_HEIGHT - uiH + 40;

    if (!m_uiInputBg.isNull()) {
        int bgW = m_uiInputBg.width();
        int bgH = m_uiInputBg.height();
        inputBgY = SCREEN_HEIGHT - bgH - 20;
        painter.drawPixmap((SCREEN_WIDTH - bgW) / 2, inputBgY, m_uiInputBg);
    }

    painter.setFont(QFont("Arial", 16, QFont::Bold));
    int textStartX = (SCREEN_WIDTH - 600) / 2 + 40;
    int textY = inputBgY + 45;

    // 【防崩溃】防止除以零
    int totalLen = m_targetText.length();
    if (totalLen == 0) totalLen = 1;

    int showLen = 45;
    int startIdx = qMax(0, m_currentIndex - 15);

    QString typeStr = m_targetText.mid(startIdx, m_currentIndex - startIdx);
    painter.setPen(Qt::black);
    painter.drawText(textStartX, textY, typeStr);

    int typedWidth = painter.fontMetrics().horizontalAdvance(typeStr);
    QString remainStr = m_targetText.mid(m_currentIndex, showLen - (m_currentIndex - startIdx));

    painter.setPen(Qt::darkGray);
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
    // 【关键修复】处理空字符或非法输入
    if (text.isEmpty() || m_targetText.isEmpty()) return;
    if (m_currentIndex >= m_targetText.length()) return;

    // 获取当前目标字符
    QChar targetChar = m_targetText.at(m_currentIndex);

    // 【关键修复】宽松匹配：如果是空格，允许任意空白字符匹配
    QChar inputChar = text.at(0);
    bool match = (inputChar == targetChar);

    // 如果目标是空格，允许输入回车或普通空格
    if (!match && targetChar.isSpace()) {
        if (inputChar == ' ' || inputChar == '\r' || inputChar == '\n') {
            match = true;
        }
    }

    if (match) {
        m_currentIndex++;
        m_score += 10;
        m_isTypingError = false;
        m_playerSpeed += 0.5;

        if (m_currentIndex >= m_targetText.length()) {
            if (m_settings.role == 1) {
                m_state = GameState::Victory;
                stopGame();
                emit gameFinished(m_score + 2000, true);
            }
            else {
                m_currentIndex = 0;
                loadArticle();
                m_playerSpeed += 3.0;
            }
        }
    }
    else {
        m_isTypingError = true;
        m_playerSpeed = qMax(0.0, m_playerSpeed - 1.5);
    }
}