#include "molegame.h"
#include <QRandomGenerator>
#include <QDebug>

const QPoint molePositions[8] = {
    QPoint(30, 330), QPoint(230, 330), QPoint(430, 330), QPoint(640, 330),
    QPoint(30, 140), QPoint(230, 140), QPoint(430, 140), QPoint(640, 140)
};

MoleGame::MoleGame(QObject* parent) : GameBase(parent) {
    m_backgroundPixmap.load(":/img/background.bmp");
    m_carrotPixmap.load(":/img/carrot.bmp");

    m_hitSound = new QSoundEffect(this);
    m_hitSound->setSource(QUrl::fromLocalFile(":/snd/hit.wav"));

    m_missSound = new QSoundEffect(this);
    m_missSound->setSource(QUrl::fromLocalFile(":/snd/miss.wav"));

    m_backgroundMusic = new QSoundEffect(this);
    m_backgroundMusic->setSource(QUrl::fromLocalFile(":/snd/background.wav"));
    m_backgroundMusic->setLoopCount(QSoundEffect::Infinite);

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout, this, &MoleGame::onGameTimerTick);

    m_spawnTimer = new QTimer(this);
    connect(m_spawnTimer, &QTimer::timeout, this, &MoleGame::onSpawnTimerTick);

    for (int i = 0; i < 8; ++i) {
        Mole* mole = new Mole(this);
        mole->setPos(molePositions[i]);
        connect(mole, &Mole::hitSuccess, this, &MoleGame::onMoleHit);
        connect(mole, &Mole::escaped, this, &MoleGame::onMoleEscaped);
        m_moles.append(mole);
    }
}

MoleGame::~MoleGame() {
    qDeleteAll(m_moles);
    m_moles.clear();
}

void MoleGame::updateSettings(const GameSettingsData& data) { m_settings = data; }
void MoleGame::increaseDifficulty() {
    m_settings.spawnIntervalMs = qMax(300, m_settings.spawnIntervalMs - 100);
    m_settings.stayTimeMs = qMax(500, m_settings.stayTimeMs - 200);
}

void MoleGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
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

    emit scoreChanged(m_score);
}

void MoleGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;

        m_gameTimer->start();
        m_backgroundMusic->play();

        maintainMoleCount();

        // 启动保底定时器，每500ms检查一次，防止场上地鼠意外变空
        m_spawnTimer->start(500);
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
        m_spawnTimer->start(); // 恢复保底定时器
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

void MoleGame::draw(QPainter& painter) {
    painter.drawPixmap(0, 0, m_backgroundPixmap);
    for (auto mole : m_moles) {
        mole->draw(painter);
    }
    for (int i = 0; i < m_lives; ++i) {
        painter.drawPixmap(140 + i * 35, 540, 30, 77, m_carrotPixmap);
    }

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));

    painter.drawText(603, 540, QString::number(m_settings.gameTimeSec / 60));
    painter.drawText(720, 540, QString("%1s").arg(m_remainingTimeSec));

    double accuracy = (m_totalSpawns == 0) ? 0.0 : (double)m_hitCount / m_totalSpawns * 100.0;
    painter.drawText(610, 570, QString("%1%").arg((int)accuracy));
    painter.drawText(730, 570, QString::number(m_hitCount));
}

void MoleGame::handleKeyPress(QKeyEvent* event) {
    if (m_state != GameState::Playing || event->isAutoRepeat()) return;

    QString key = event->text().toUpper();
    if (key.isEmpty()) return;

    for (auto mole : m_moles) {
        // 只有 Visible 状态的才能被打
        if (mole->isActive() && mole->getLetter() == key) {
            mole->hitByUser();
            break;
        }
    }
}

void MoleGame::maintainMoleCount() {
    if (m_state != GameState::Playing) return;

    int activeCount = 0;
    QVector<int> freeIndices;

    for (int i = 0; i < m_moles.size(); ++i) {
        if (m_moles[i]->isActive()) {
            activeCount++;
        }
        else if (m_moles[i]->isFree()) {
            freeIndices.append(i);
        }
    }

    // 目标保持 3 只
    int needed = 3 - activeCount;
    while (needed > 0 && !freeIndices.isEmpty()) {
        int randIdx = QRandomGenerator::global()->bounded(freeIndices.size());
        int moleIdx = freeIndices[randIdx];

        char letter = 'A' + QRandomGenerator::global()->bounded(26);
        m_moles[moleIdx]->showMole(QString(letter), m_settings.stayTimeMs);
        m_totalSpawns++;

        freeIndices.removeAt(randIdx);
        needed--;
    }
}

void MoleGame::onGameTimerTick() {
    m_remainingTimeSec--;
    if (m_remainingTimeSec <= 0) {
        stopGame();
        emit gameFinished(m_hitCount * 10, true);
    }
}

void MoleGame::onSpawnTimerTick() {
    // 保底机制：如果场上数量意外不对，补充地鼠
    maintainMoleCount();
}

void MoleGame::onMoleHit() {
    m_hitCount++;
    m_score += 10;
    m_hitSound->play();
    emit scoreChanged(m_score);

    // 打掉一只，立马补一只
    maintainMoleCount();
}

void MoleGame::onMoleEscaped() {
    m_lives--;


    if (m_lives <= 0) {
        stopGame();
        emit gameFinished(m_hitCount * 10, false);
    }
    else {
        // 跑掉一只，立马补一只
        maintainMoleCount();
    }
}