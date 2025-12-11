#include "policegame.h"
#include <QDebug>
#include <QFile>

PoliceGame::PoliceGame(QObject *parent) : GameBase(parent) {
    // 假设地图是斜45度的长图，这里暂时用简单的矩形代替逻辑
    // 请确保资源路径正确，或者先用纯色填充测试
    m_bgPixmap.load(":/img/police_bg.png"); 
    m_policePixmap.load(":/img/police_car.png");
    
    m_timer = new QTimer(this);
    m_timer->setInterval(30); // 约30FPS
    connect(m_timer, &QTimer::timeout, this, &PoliceGame::onGameTick);
}

PoliceGame::~PoliceGame() {}

void PoliceGame::initGame() {
    m_state = GameState::Ready;
    m_currentIndex = 0;
    m_progress = 0.0;
    m_enemyProgress = 0.0;
    m_mapScrollX = 0;
    m_score = 0;
    
    loadArticle(); // 加载一篇文章
}

void PoliceGame::loadArticle() {
    // 这里暂时硬编码，后续应从文件读取
    m_targetText = "Located in the southeastern Philippines, Davao is a city totally given.";
}

void PoliceGame::startGame() {
    if(m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_timer->start();
    }
}

void PoliceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_timer->stop();
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_timer->start();
    }
}

void PoliceGame::stopGame() {
    m_state = GameState::GameOver;
    m_timer->stop();
}

void PoliceGame::handleKeyPress(QKeyEvent *event) {
    if (m_state != GameState::Playing) return;

    QString text = event->text();
    if (text.isEmpty()) return;

    QChar key = text.at(0);
    
    // 核心打字逻辑
    if (m_currentIndex < m_targetText.length()) {
        if (key == m_targetText.at(m_currentIndex)) {
            // 输入正确
            m_currentIndex++;
            m_score += 10;
            
            // 增加玩家速度或瞬间位移
            m_progress += 0.005; // 假设每打对一个字前进一点
            
            emit scoreChanged(m_score);
        } else {
            // 输入错误，可以播放音效或惩罚
        }
    }
    
    // 检查是否完成
    if (m_currentIndex >= m_targetText.length()) {
        m_state = GameState::Victory;
        stopGame();
        emit gameFinished(m_score, true);
    }
}

void PoliceGame::onGameTick() {
    // 敌人自动前进逻辑 (模拟电脑)
    m_enemyProgress += 0.001; // 匀速前进

    // 简单的追逐判定逻辑
    if (m_progress >= 1.0) {
        // 玩家到达终点
        stopGame();
        emit gameFinished(m_score, true);
    }
    
    // 滚动地图逻辑：让地图跟随玩家位置
    // 假设地图总长 2000 像素
    double mapLength = 2000.0;
    m_mapScrollX = m_progress * (mapLength - 800); // 800是屏幕宽度
}

void PoliceGame::draw(QPainter &painter) {
    // 1. 绘制背景 (支持滚动)
    // 这里使用 save/restore 和 translate 来模拟摄像头移动
    painter.save();
    painter.translate(-m_mapScrollX, 0); 
    
    // 如果没有图，先画个背景色
    if(m_bgPixmap.isNull()) {
        painter.fillRect(0, 0, 2000, 600, Qt::darkGray);
        // 画条路
        painter.fillRect(0, 300, 2000, 100, Qt::gray);
    } else {
        painter.drawPixmap(0, 0, m_bgPixmap);
    }

    // 2. 绘制角色 (根据进度计算 x 坐标)
    double mapLength = 2000.0;
    int playerX = (int)(m_progress * mapLength);
    int enemyX = (int)(m_enemyProgress * mapLength);

    painter.setBrush(Qt::blue);
    painter.drawRect(playerX, 320, 50, 50); // 玩家(警察)
    painter.drawText(playerX, 310, "Player");

    painter.setBrush(Qt::red);
    painter.drawRect(enemyX, 320, 50, 50); // 敌人(小偷)
    painter.drawText(enemyX, 310, "Thief");
    
    painter.restore(); // 恢复坐标系，绘制UI

    // 3. 绘制UI (打字条)
    painter.fillRect(0, 500, 800, 100, QColor(255, 255, 255, 200)); // 半透明底板
    
    painter.setFont(QFont("Arial", 16));
    
    // 绘制已输入的部分 (绿色)
    painter.setPen(Qt::green);
    QString typed = m_targetText.left(m_currentIndex);
    painter.drawText(50, 550, typed);
    
    // 计算已输入部分的宽度，以便接着画未输入部分
    int typedWidth = painter.fontMetrics().horizontalAdvance(typed);
    
    // 绘制未输入的部分 (黑色)
    painter.setPen(Qt::black);
    painter.drawText(50 + typedWidth, 550, m_targetText.mid(m_currentIndex));
    
    // 绘制光标
    painter.fillRect(50 + typedWidth, 535, 2, 20, Qt::black);
}