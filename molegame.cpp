#include "molegame.h"
#include <QRandomGenerator>
#include <QDebug>

// 地鼠洞坐标配置
const QPoint molePositions[9] = {
    QPoint(130, 290), QPoint(320, 290), QPoint(510, 290),
    QPoint(100, 330), QPoint(290, 330), QPoint(480, 330),
    QPoint(140, 370), QPoint(330, 370), QPoint(520, 370)
};

MoleGame::MoleGame(QObject *parent) : GameBase(parent) {
    // 1. 加载资源
    m_backgroundPixmap.load(":/img/background.bmp");
    m_carrotPixmap.load(":/img/carrot.bmp");

    // 2. 初始化音效
    m_hitSound = new QSoundEffect(this);
    m_hitSound->setSource(QUrl::fromLocalFile(":/snd/hit.wav"));
    
    m_missSound = new QSoundEffect(this);
    m_missSound->setSource(QUrl::fromLocalFile(":/snd/miss.wav"));
    
    m_backgroundMusic = new QSoundEffect(this);
    m_backgroundMusic->setSource(QUrl::fromLocalFile(":/snd/background.wav"));
    m_backgroundMusic->setLoopCount(QSoundEffect::Infinite);

    // 3. 初始化计时器
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout, this, &MoleGame::onGameTimerTick);

    m_spawnTimer = new QTimer(this);
    connect(m_spawnTimer, &QTimer::timeout, this, &MoleGame::onSpawnTimerTick);

    // 4. 创建地鼠 (注意：Mole 需要是 QWidget 吗？如果不显示在布局中，建议改为纯逻辑对象或自行绘制。
    // 为了兼容你原本的 Mole 代码，这里需要特殊的父对象处理。
    // 但因为 GameBase 不是 Widget，我们暂时不给 Mole 指定父 Widget，而是手动管理内存)
    for (int i = 0; i < 9; ++i) {
        Mole* mole = new Mole(this); // 父对象设为 this，自动管理内存
        mole->setPos(molePositions[i]); // 使用 setPos 替代 move
        connect(mole, &Mole::hitSuccess, this, &MoleGame::onMoleHit);
        connect(mole, &Mole::escaped, this, &MoleGame::onMoleEscaped);
        m_moles.append(mole);
    }
}

MoleGame::~MoleGame() {
    qDeleteAll(m_moles);
    m_moles.clear();
}

void MoleGame::updateSettings(const GameSettingsData &data) {
    m_settings = data;
}

void MoleGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0; // GameBase 成员
    m_lives = 5;
    m_hitCount = 0;
    m_totalSpawns = 0;
    m_remainingTimeSec = m_settings.gameTimeSec;

    m_gameTimer->stop();
    m_spawnTimer->stop();
    m_backgroundMusic->stop();

    for (auto mole : m_moles) {
        mole->hideMole();
    }
    
    // 发送初始分数更新信号
    emit scoreChanged(m_score);
}

void MoleGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame(); // 确保数据重置
        m_state = GameState::Playing;
        
        m_gameTimer->start();
        m_spawnTimer->setInterval(m_settings.spawnIntervalMs);
        m_spawnTimer->start();
        m_backgroundMusic->play();
    }
}

void MoleGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_gameTimer->stop();
        m_spawnTimer->stop();
        m_backgroundMusic->stop();
        for (auto mole : m_moles) mole->pause();
    } 
    else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_gameTimer->start();
        m_spawnTimer->start();
        m_backgroundMusic->play();
        for (auto mole : m_moles) mole->resume();
    }
}

void MoleGame::stopGame() {
    m_state = GameState::GameOver;
    m_gameTimer->stop();
    m_spawnTimer->stop();
    m_backgroundMusic->stop();
    for (auto mole : m_moles) mole->hideMole();
}

void MoleGame::draw(QPainter &painter) {
    // 1. 绘制背景
    painter.drawPixmap(0, 0, m_backgroundPixmap);

    // 2. 绘制地鼠 (因为 Mole 是 QWidget，但我们现在手动控制绘制)
    // 注意：如果 Mole 仍然是 QWidget 且没有添加到布局，它不会自动显示。
    // 我们需要调用 Mole 的 render 或者修改 Mole 的逻辑让它支持 painter 绘制。
    // 为了兼容现有代码，我们这里使用一个技巧：让 Mole 自己画到 painter 上。
    // **重要修改**：这需要 Mole 类有一个 paint(QPainter*) 接口，或者我们手动移动 painter 绘制。
    
    for (auto mole : m_moles) {
        mole->draw(painter);
    }

    // 3. 绘制萝卜 (生命值)
    for (int i = 0; i < m_lives; ++i) {
        painter.drawPixmap(140 + i * 35, 540, 30, 77, m_carrotPixmap);
    }

    // 4. 绘制 HUD 文字 (时间、分数)
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    
    // 时间
    painter.drawText(600, 520, "限时:");
    painter.drawText(640, 520, QString::number(m_settings.gameTimeSec / 60) + " 分");
    painter.drawText(720, 520, QString("%1s").arg(m_remainingTimeSec));

    // 正确率
    painter.drawText(610, 545, "正确率:");
    double accuracy = (m_totalSpawns == 0) ? 0.0 : (double)m_hitCount / m_totalSpawns * 100.0;
    painter.drawText(670, 545, QString("%1%").arg((int)accuracy));

    // 击中次数
    painter.drawText(730, 550, QString::number(m_hitCount));
}

void MoleGame::handleKeyPress(QKeyEvent *event) {
    if (m_state != GameState::Playing || event->isAutoRepeat()) return;

    QString key = event->text().toUpper();
    if (key.isEmpty()) return;

    for (auto mole : m_moles) {
        if (mole->isVisible() && mole->getLetter() == key) {
            mole->hitByUser();
            // hitSuccess 信号会触发 onMoleHit
            break;
        }
    }
}

void MoleGame::spawnMole() {
    // 简单的生成逻辑
    QVector<int> availableIndices;
    for (int i = 0; i < m_moles.size(); ++i) {
        // 这里依赖 Mole::isVisible()，确保 Mole 即使没有父窗口也能正确管理状态
        if (!m_moles[i]->isVisible()) {
            availableIndices.append(i);
        }
    }

    if (availableIndices.isEmpty()) return;

    int index = availableIndices.at(QRandomGenerator::global()->bounded(availableIndices.size()));
    char letter = 'A' + QRandomGenerator::global()->bounded(26);

    m_moles[index]->showMole(QString(letter), m_settings.stayTimeMs);
    m_totalSpawns++;
}

void MoleGame::onGameTimerTick() {
    m_remainingTimeSec--;
    if (m_remainingTimeSec <= 0) {
        stopGame();
        emit gameFinished(m_hitCount * 10, true); // 假设分数就是命中数*10
    }
}

void MoleGame::onSpawnTimerTick() {
    spawnMole();
}

void MoleGame::onMoleHit() {
    m_hitCount++;
    m_score += 10;
    m_hitSound->play();
    emit scoreChanged(m_score);
}

void MoleGame::onMoleEscaped() {
    m_lives--;
    m_missSound->play();
    if (m_lives <= 0) {
        stopGame();
        emit gameFinished(m_hitCount * 10, false);
    }
}