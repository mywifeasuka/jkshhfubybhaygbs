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
    "data", "node", "tree", "list", "code", "byte", "bit", "loop"
};

FrogGame::FrogGame(QObject* parent) : GameBase(parent) {
    // 资源加载
    m_bgPixmap.load(":/img/frog_background.png");
    m_leafPixmap.load(":/img/frog_leaf.png");

    m_frogBack1.load(":/img/frog_back_1.png");
    m_frogBack2.load(":/img/frog_back_2.png");
    m_frogFront1.load(":/img/frog_front_1.png");
    m_frogFront2.load(":/img/frog_front_2.png");

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

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(1000);
    connect(m_animTimer, &QTimer::timeout, this, &FrogGame::onAnimTick);
    m_isCroaking = false;

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

void FrogGame::retreatFrog() {
    // 逻辑：回到上一行
    // Row 2 -> Row 1 (最新)
    // Row 1 -> Row 0 (最新)
    // Row 0 -> Bank (-1)

    int targetRow = m_currentRow - 1;

    if (targetRow < 0) {
        // 回到岸边
        m_currentRow = -1;
        m_currentLeaf = nullptr;
        m_frogPos = QPointF(SCREEN_WIDTH / 2, START_BANK_Y);
        // 输入缓冲清空
        m_inputBuffer.clear();
        m_lockedLeaf = nullptr;
        return;
    }

    // 寻找 targetRow 中“最新生成”的荷叶
    // 由于 m_leaves 是 append 的，最新的在列表末尾
    LotusLeaf* targetLeaf = nullptr;

    for (int i = m_leaves.size() - 1; i >= 0; --i) {
        if (m_leaves[i]->row == targetRow) {
            // 还需要确保它在屏幕内，或者至少青蛙能站上去
            targetLeaf = m_leaves[i];
            break; // 找到最新的一个就停止
        }
    }

    if (targetLeaf) {
        m_currentRow = targetRow;
        m_currentLeaf = targetLeaf;
        m_frogPos.setX(targetLeaf->x);
        m_frogPos.setY(ROW_Y[targetRow]);
        // 重置锁定状态，因为换了荷叶
        m_inputBuffer.clear();
        m_lockedLeaf = nullptr;
    }
    else {
        // 极端情况：上一排居然没叶子？那只能回岸边了
        m_currentRow = -1;
        m_currentLeaf = nullptr;
        m_frogPos = QPointF(SCREEN_WIDTH / 2, START_BANK_Y);
        m_inputBuffer.clear();
        m_lockedLeaf = nullptr;
    }
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
                // 【修改点】强制转为小写 (toLower)
                newWords.append(currentWord.toLower());
            }
            currentWord.clear();
        }
    }
    if (currentWord.length() >= 2) newWords.append(currentWord.toLower());

    if (!newWords.isEmpty()) m_wordList = newWords;
    else if (m_wordList.isEmpty()) m_wordList = DEFAULT_WORDS;
}

void FrogGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;

    m_successCount = 0;

    if (m_wordList.size() <= DEFAULT_WORDS.size()) {
        loadDictionary(m_settings.dictionaryFile);
    }

    qDeleteAll(m_leaves);
    m_leaves.clear();
    resetFrog();
    emit scoreChanged(0);
}

void FrogGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_animTimer->start();
        m_bgMusic->play();

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
    m_animTimer->stop();
    m_bgMusic->stop();
    qDeleteAll(m_leaves);
    m_leaves.clear();
}

void FrogGame::onAnimTick() {
    m_isCroaking = !m_isCroaking;
}

// spawnLeaves 逻辑保持不变
void FrogGame::spawnLeaves() {
    double baseSpeed = 0.5 + (m_settings.difficulty - 1) * 0.4;
    double speeds[] = { baseSpeed, -baseSpeed * 1.3, baseSpeed * 1.6 };
    double minGap = 260.0;
    for (int r = 0; r < 3; ++r) {
        double rightMost = -9999; double leftMost = 9999; bool hasLeaf = false;
        for (LotusLeaf* leaf : m_leaves) {
            if (leaf->row == r) {
                if (leaf->x > rightMost) rightMost = leaf->x;
                if (leaf->x < leftMost) leftMost = leaf->x;
                hasLeaf = true;
            }
        }
        bool needSpawn = false; double spawnX = 0;
        if (speeds[r] > 0) {
            if (!hasLeaf || leftMost > (minGap - 100)) { spawnX = -100; needSpawn = true; }
        }
        else {
            if (!hasLeaf || rightMost < (SCREEN_WIDTH - minGap + 100)) { spawnX = SCREEN_WIDTH + 100; needSpawn = true; }
        }
        if (needSpawn) {
            if (QRandomGenerator::global()->bounded(100) < 15) {
                QString w = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
                m_leaves.append(new LotusLeaf(r, spawnX, speeds[r], w));
            }
        }
    }
}

void FrogGame::onGameTick() {
    spawnLeaves();
    for (auto it = m_leaves.begin(); it != m_leaves.end(); ) {
        LotusLeaf* leaf = *it;
        leaf->x += leaf->speed;

        if (leaf->x < -200 || leaf->x > SCREEN_WIDTH + 200) {
            // 【修改点】荷叶出界
            if (m_currentLeaf == leaf) {
                // 青蛙在上面 -> 触发撤退
                m_splashSound->play(); // 可以换个“惊吓”音效
                retreatFrog(); // 回到上一步
                // 注意：retreatFrog 内部已经把 m_currentLeaf 指向别的了
            }

            // 如果锁定的荷叶出去了，解锁
            if (m_lockedLeaf == leaf) {
                m_lockedLeaf = nullptr;
                m_inputBuffer.clear();
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
        // 退格键逻辑：如果想允许修正，可以保留；如果严格锁定则可移除
        // 原版通常允许退格，但这里我们采用了“锁定且不可错”逻辑
        // 根据“打错字母停留”，退格可能没必要，或者只能退格当前锁定的进度
        if (!m_inputBuffer.isEmpty()) m_inputBuffer.chop(1);
        return;
    }

    QString text = event->text(); // 【修改】大小写敏感，不转大写
    if (text.isEmpty() || text.length() > 1) return;

    // 只接受字母输入 (可选)
    // if (!text.at(0).isLetter()) return; 

    checkInput(text);
}

void FrogGame::resetFrog() {
    m_currentRow = -1;
    m_currentLeaf = nullptr;
    m_inputBuffer.clear();

    // 【新增】重置锁定状态
    m_lockedLeaf = nullptr;
    m_isGoalLocked = false;

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


// 【重写】输入判定与锁定逻辑
void FrogGame::checkInput(const QString& key) {
    int targetRow = m_currentRow + 1;

    // 1. 锁定状态处理 (保持不变)
    if (m_isGoalLocked) {
        // ... (原代码保持不变) ...
        int nextIdx = m_inputBuffer.length();
        if (nextIdx < m_goalWord.length() && m_goalWord.at(nextIdx) == key.at(0)) {
            m_inputBuffer += key;
            if (m_inputBuffer == m_goalWord) {
                m_successSound->play();
                m_score += 500;
                m_successCount++;
                emit scoreChanged(m_score);

                // 成功了一只，是否全部完成？
                if (m_successCount >= 5) { // 总共5只
                    stopGame();
                    emit gameFinished(m_score, true); // 唯一的结算界面
                }
                else {
                    resetFrog(); // 下一只
                }
            }
        }
        return;
    }

    if (m_lockedLeaf) {
        // ... (原代码保持不变) ...
        int nextIdx = m_inputBuffer.length();
        if (nextIdx < m_lockedLeaf->word.length() && m_lockedLeaf->word.at(nextIdx) == key.at(0)) {
            m_inputBuffer += key;
            if (m_inputBuffer == m_lockedLeaf->word) {
                // Jump!
                m_currentRow = m_lockedLeaf->row;
                m_currentLeaf = m_lockedLeaf;
                m_frogPos.setX(m_currentLeaf->x);
                m_frogPos.setY(ROW_Y[m_currentLeaf->row]);

                m_score += m_lockedLeaf->word.length() * 10;
                m_jumpSound->play();
                emit scoreChanged(m_score);

                m_inputBuffer.clear();
                m_lockedLeaf = nullptr;
            }
        }
        return;
    }

    // 2. 未锁定状态 - 寻找目标

    // Case A: 终点 (Row 3)
    if (targetRow == 3) {
        if (m_goalWord.startsWith(key)) {
            m_isGoalLocked = true;
            m_inputBuffer += key;
            if (m_inputBuffer == m_goalWord) {
                // Instant win logic...
                m_successSound->play();
                m_score += 500;
                m_successCount++;
                emit scoreChanged(m_score);
                if (m_successCount >= 5) { stopGame(); emit gameFinished(m_score, true); }
                else resetFrog();
            }
        }
        return;
    }

    // Case B: 下一排荷叶 (修改为倒序遍历)
    // 【修改点】从后往前遍历，实现“优先绑定后生成的单词”
    for (int i = m_leaves.size() - 1; i >= 0; --i) {
        LotusLeaf* leaf = m_leaves[i];

        if (leaf->row == targetRow && leaf->x > 50 && leaf->x < SCREEN_WIDTH - 50) {
            if (leaf->word.startsWith(key)) {
                m_lockedLeaf = leaf; // 锁定
                m_inputBuffer += key;
                if (m_inputBuffer == leaf->word) {
                    // Instant jump logic...
                    m_currentRow = leaf->row;
                    m_currentLeaf = leaf;
                    m_frogPos.setX(leaf->x);
                    m_frogPos.setY(ROW_Y[leaf->row]);
                    m_score += 10;
                    m_jumpSound->play();
                    emit scoreChanged(m_score);
                    m_inputBuffer.clear();
                    m_lockedLeaf = nullptr;
                }
                return;
            }
        }
    }
}

void FrogGame::draw(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
    }
    else {
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(65, 105, 225));
    }

    // 绘制终点单词
    painter.setFont(QFont("Arial", 20, QFont::Bold));

    // 判定是否绘制高亮
    bool isGoalActive = (m_currentRow == 2) && (m_isGoalLocked || (m_inputBuffer.isEmpty() && m_lockedLeaf == nullptr));

    if (isGoalActive && !m_inputBuffer.isEmpty() && m_isGoalLocked) {
        // 终点被锁定且正在输入
        int w = painter.fontMetrics().horizontalAdvance(m_goalWord);
        int startX = (SCREEN_WIDTH - w) / 2;
        int textY = GOAL_BANK_Y + 10;

        // 已输入：深蓝
        painter.setPen(Qt::darkBlue);
        painter.drawText(startX, textY, m_inputBuffer);

        // 未输入：红
        int typedW = painter.fontMetrics().horizontalAdvance(m_inputBuffer);
        painter.setPen(Qt::red);
        painter.drawText(startX + typedW, textY, m_goalWord.mid(m_inputBuffer.length()));
    }
    else {
        // 普通显示 (白色)
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, GOAL_BANK_Y - 20, SCREEN_WIDTH, 40), Qt::AlignCenter, m_goalWord);
    }

    // 绘制荷叶
    for (LotusLeaf* leaf : m_leaves) {
        if (!m_leafPixmap.isNull()) {
            painter.drawPixmap(leaf->x - m_leafPixmap.width() / 2,
                ROW_Y[leaf->row] - m_leafPixmap.height() / 2,
                m_leafPixmap);
        }

        // 字体改小
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        int textY = ROW_Y[leaf->row] + 5;

        // 判定该荷叶是否被锁定高亮
        // 条件：它是锁定的荷叶，或者 输入为空且符合行号要求
        bool isTarget = (leaf == m_lockedLeaf);

        if (isTarget) {
            QFontMetrics fm(painter.font());
            int totalW = fm.horizontalAdvance(leaf->word);
            int startX = leaf->x - totalW / 2;

            // 已输入：深蓝
            painter.setPen(Qt::darkBlue);
            painter.drawText(startX, textY, m_inputBuffer);

            // 未输入：红
            int typedW = fm.horizontalAdvance(m_inputBuffer);
            painter.setPen(Qt::red);
            painter.drawText(startX + typedW, textY, leaf->word.mid(m_inputBuffer.length()));
        }
        else {
            // 普通：黑色
            painter.setPen(Qt::black);
            // 扩大文本框宽度
            QRect textRect(leaf->x - 70, ROW_Y[leaf->row] - 20, 140, 40);
            painter.drawText(textRect, Qt::AlignCenter, leaf->word);
        }
    }

    // 绘制青蛙
    QPixmap* currentFrogPix = m_isCroaking ? &m_frogBack2 : &m_frogBack1;
    if (currentFrogPix && !currentFrogPix->isNull()) {
        painter.drawPixmap(m_frogPos.x() - currentFrogPix->width() / 2,
            m_frogPos.y() - currentFrogPix->height() / 2,
            *currentFrogPix);
    }

    // 绘制剩余青蛙（岸边等待）
    // 注意：m_frogsRemaining 包含当前正在跳的这一只。
    // 所以岸边等待的数量是 m_frogsRemaining - 1
    int waitingFrogs = 5 - m_successCount - 1;
    if (waitingFrogs < 0) waitingFrogs = 0; // 保护一下

    for (int i = 0; i < waitingFrogs; ++i) {
        int fx = SCREEN_WIDTH - 40 - (i * 35);
        int fy = SCREEN_HEIGHT - 40;
        if (currentFrogPix && !currentFrogPix->isNull())
            painter.drawPixmap(fx, fy, 25, 25, *currentFrogPix);
    }

    // 绘制对岸青蛙
    QPixmap* goalFrogPix = m_isCroaking ? &m_frogFront2 : &m_frogFront1;
    int startGoalX = 50;
    int gapGoal = 50;
    for (int i = 0; i < m_successCount; ++i) {
        int gx = startGoalX + i * gapGoal;
        if (goalFrogPix && !goalFrogPix->isNull()) {
            painter.drawPixmap(gx, 40, 40, 40, *goalFrogPix);
        }
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(20, 40, QString("Score: %1").arg(m_score));
}