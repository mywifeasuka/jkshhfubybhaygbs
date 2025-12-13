#include "froggame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QFile>
#include <QCoreApplication>
#include <QDir>

// 配置
const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int START_BANK_Y = 500;
const int ROW_Y[] = { 400, 300, 200 };
const int GOAL_BANK_Y = 100;

const QStringList DEFAULT_WORDS = {
    "DATA", "NODE", "TREE", "LIST", "CODE", "BYTE", "BIT", "LOOP"
};

FrogGame::FrogGame(QObject* parent) : GameBase(parent) {
    // 资源加载
    m_bgPixmap.load(":/img/frog_background.png");
    m_frogPixmap.load(":/img/frog_back_1.png");
    m_leafPixmap.load(":/img/frog_leaf.png");

    m_jumpSound = new QSoundEffect(this);
    m_jumpSound->setSource(QUrl::fromLocalFile(":/snd/frog_jump.wav"));
    m_splashSound = new QSoundEffect(this);
    m_splashSound->setSource(QUrl::fromLocalFile(":/snd/mouse_away.wav"));
    m_successSound = new QSoundEffect(this);
    m_successSound->setSource(QUrl::fromLocalFile(":/snd/upgrade.wav"));
    m_bgMusic = new QSoundEffect(this);
    m_bgMusic->setSource(QUrl::fromLocalFile(":/snd/frog_bg.wav"));
    m_bgMusic->setLoopCount(QSoundEffect::Infinite);

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &FrogGame::onGameTick);

    // 初始设置
    m_settings.difficulty = 1;
    m_settings.dictionaryFile = "4W.ID";
    m_wordList = DEFAULT_WORDS;
}

FrogGame::~FrogGame() {
    qDeleteAll(m_leaves);
    m_leaves.clear();
}

void FrogGame::updateSettings(const FrogSettingsData& settings) {
    m_settings = settings;
    loadDictionary(m_settings.dictionaryFile);
}

void FrogGame::loadDictionary(const QString& filename) {
    QString path = QCoreApplication::applicationDirPath() + "/Data/English/T_WORD/Dictionary/" + filename;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (m_wordList.isEmpty()) m_wordList = DEFAULT_WORDS;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QStringList newWords;
    QString currentWord;

    for (char c : data) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '\'' || c == '-') {
            currentWord.append(c);
        }
        else {
            if (currentWord.length() >= 2) {
                newWords.append(currentWord.toUpper());
            }
            currentWord.clear();
        }
    }
    if (currentWord.length() >= 2) {
        newWords.append(currentWord.toUpper());
    }

    if (!newWords.isEmpty()) {
        m_wordList = newWords;
    }
    else {
        if (m_wordList.isEmpty()) m_wordList = DEFAULT_WORDS;
    }
}

void FrogGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_frogCount = 5;

    if (m_wordList.size() <= DEFAULT_WORDS.size()) {
        loadDictionary(m_settings.dictionaryFile);
    }

    // 【修改点 1】初始化时只清空，不生成。实现“打开游戏时没有荷叶”
    qDeleteAll(m_leaves);
    m_leaves.clear();

    resetFrog();
    emit scoreChanged(0);
}

void FrogGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        // 先调用 initGame 重置分数和青蛙状态
        initGame();

        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();

        // 【修改点 2】点击开始后，手动生成初始的一批荷叶
        double baseSpeed = 0.5 + (m_settings.difficulty - 1) * 0.4;
        double speeds[] = { baseSpeed, -baseSpeed * 1.3, baseSpeed * 1.6 };

        for (int r = 0; r < 3; r++) {
            int positions[] = { 160, 400, 640 };
            for (int x : positions) {
                QString w = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
                m_leaves.append(new LotusLeaf(r, x, speeds[r], w));
            }
        }
    }
}

void FrogGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();

    // 【修改点 3】结束时清屏
    qDeleteAll(m_leaves);
    m_leaves.clear();
}

void FrogGame::spawnLeaves() {
    double baseSpeed = 0.5 + (m_settings.difficulty - 1) * 0.4;
    double speeds[] = { baseSpeed, -baseSpeed * 1.3, baseSpeed * 1.6 };
    double minGap = 260.0;

    for (int r = 0; r < 3; ++r) {
        double rightMost = -9999;
        double leftMost = 9999;
        bool hasLeaf = false;

        for (LotusLeaf* leaf : m_leaves) {
            if (leaf->row == r) {
                if (leaf->x > rightMost) rightMost = leaf->x;
                if (leaf->x < leftMost) leftMost = leaf->x;
                hasLeaf = true;
            }
        }

        bool needSpawn = false;
        double spawnX = 0;

        if (speeds[r] > 0) {
            if (!hasLeaf || leftMost > (minGap - 100)) {
                spawnX = -100;
                needSpawn = true;
            }
        }
        else {
            if (!hasLeaf || rightMost < (SCREEN_WIDTH - minGap + 100)) {
                spawnX = SCREEN_WIDTH + 100;
                needSpawn = true;
            }
        }

        if (needSpawn) {
            if (QRandomGenerator::global()->bounded(100) < 15) {
                QString w = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
                m_leaves.append(new LotusLeaf(r, spawnX, speeds[r], w));
            }
        }
    }
}

void FrogGame::resetFrog() {
    m_currentRow = -1;
    m_currentLeaf = nullptr;
    m_inputBuffer.clear();
    m_frogPos = QPointF(SCREEN_WIDTH / 2, START_BANK_Y);
    m_goalWord = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
}

void FrogGame::pauseGame() {
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

void FrogGame::onGameTick() {
    spawnLeaves();
    for (auto it = m_leaves.begin(); it != m_leaves.end(); ) {
        LotusLeaf* leaf = *it;
        leaf->x += leaf->speed;

        if (leaf->x < -200 || leaf->x > SCREEN_WIDTH + 200) {
            if (m_currentLeaf == leaf) {
                m_frogCount--;
                m_splashSound->play();
                if (m_frogCount > 0) {
                    resetFrog();
                }
                else {
                    stopGame();
                    emit gameFinished(m_score, false);
                }
            }
            delete leaf;
            it = m_leaves.erase(it);
        }
        else {
            ++it;
        }
    }
    if (m_currentLeaf) {
        m_frogPos.setX(m_currentLeaf->x);
        m_frogPos.setY(ROW_Y[m_currentLeaf->row]);
    }
}

void FrogGame::handleKeyPress(QKeyEvent* event) {
    if (m_state != GameState::Playing) return;
    if (event->key() == Qt::Key_Backspace) {
        if (!m_inputBuffer.isEmpty()) m_inputBuffer.chop(1);
        return;
    }
    QString text = event->text().toUpper();
    if (text.isEmpty() || text.length() > 1) return;
    if (text.at(0) >= 'A' && text.at(0) <= 'Z') {
        m_inputBuffer += text;
        checkInput();
    }
}

void FrogGame::checkInput() {
    int targetRow = m_currentRow + 1;
    if (targetRow == 3) {
        if (m_goalWord.startsWith(m_inputBuffer)) {
            if (m_inputBuffer == m_goalWord) {
                m_successSound->play();
                m_score += 500;
                emit scoreChanged(m_score);
                resetFrog();
            }
        }
        else {
            m_inputBuffer.clear();
        }
        return;
    }
    QList<LotusLeaf*> candidates;
    for (LotusLeaf* leaf : m_leaves) {
        if (leaf->row == targetRow) {
            if (leaf->x > 50 && leaf->x < SCREEN_WIDTH - 50) {
                if (leaf->word.startsWith(m_inputBuffer)) {
                    candidates.append(leaf);
                }
            }
        }
    }
    if (candidates.isEmpty()) {
        m_inputBuffer.clear();
    }
    else {
        for (LotusLeaf* leaf : candidates) {
            if (leaf->word == m_inputBuffer) {
                m_currentRow = targetRow;
                m_currentLeaf = leaf;
                m_frogPos.setX(leaf->x);
                m_frogPos.setY(ROW_Y[targetRow]);
                m_inputBuffer.clear();
                m_score += leaf->word.length() * 10;
                m_jumpSound->play();
                emit scoreChanged(m_score);
                return;
            }
        }
    }
}

void FrogGame::draw(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    // 背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
    }
    else {
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(65, 105, 225));
    }

    // 终点单词
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.drawText(QRect(0, GOAL_BANK_Y - 20, SCREEN_WIDTH, 40), Qt::AlignCenter, m_goalWord);

    if (m_currentRow == 2) {
        int w = painter.fontMetrics().horizontalAdvance(m_goalWord);
        int startX = (SCREEN_WIDTH - w) / 2;
        painter.setPen(Qt::yellow);
        painter.drawText(startX, GOAL_BANK_Y + 10, m_inputBuffer);
    }

    // 绘制荷叶
    for (LotusLeaf* leaf : m_leaves) {
        if (!m_leafPixmap.isNull()) {
            painter.drawPixmap(leaf->x - m_leafPixmap.width() / 2,
                ROW_Y[leaf->row] - m_leafPixmap.height() / 2,
                m_leafPixmap);
        }

        painter.setFont(QFont("Arial", 14, QFont::Bold));
        bool isTarget = (leaf->row == m_currentRow + 1) && leaf->word.startsWith(m_inputBuffer);
        int textY = ROW_Y[leaf->row] + 6;

        // 【修改点 4】文字颜色：普通状态黑色，选中输入时红色+黑色
        if (isTarget && !m_inputBuffer.isEmpty()) {
            QFontMetrics fm(painter.font());
            int totalW = fm.horizontalAdvance(leaf->word);
            int startX = leaf->x - totalW / 2;

            // 已输入部分：红色 (高亮显示进度)
            painter.setPen(Qt::red);
            painter.drawText(startX, textY, m_inputBuffer);

            // 未输入部分：黑色 (保持默认清晰度)
            int typedW = fm.horizontalAdvance(m_inputBuffer);
            painter.setPen(Qt::black);
            painter.drawText(startX + typedW, textY, leaf->word.mid(m_inputBuffer.length()));
        }
        else {
            // 普通状态：黑色
            painter.setPen(Qt::black);
            QRect textRect(leaf->x - 60, ROW_Y[leaf->row] - 20, 120, 40);
            painter.drawText(textRect, Qt::AlignCenter, leaf->word);
        }
    }

    // 青蛙绘制
    if (!m_frogPixmap.isNull()) {
        painter.drawPixmap(m_frogPos.x() - m_frogPixmap.width() / 2,
            m_frogPos.y() - m_frogPixmap.height() / 2,
            m_frogPixmap);
    }

    // 剩余生命
    for (int i = 0; i < m_frogCount; ++i) {
        int fx = SCREEN_WIDTH - 40 - (i * 35);
        int fy = SCREEN_HEIGHT - 40;
        if (!m_frogPixmap.isNull())
            painter.drawPixmap(fx, fy, 25, 25, m_frogPixmap);
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(20, 40, QString("Score: %1").arg(m_score));
}