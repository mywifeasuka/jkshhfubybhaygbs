#include "froggame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

// 配置
const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Y轴坐标定义 (请根据你的背景图实际的河流位置微调这些值)
const int START_BANK_Y = 500; // 起点岸边 (青蛙等待的位置)
const int ROW_Y[] = { 400, 300, 200 }; // 三条河道荷叶的中心Y坐标
const int GOAL_BANK_Y = 100;  // 终点单词显示的位置

// 单词库
const QStringList DEFAULT_WORDS = {
    "DATA", "NODE", "TREE", "LIST", "CODE", "BYTE", "BIT", "LOOP",
    "IF", "ELSE", "VOID", "INT", "CHAR", "BOOL", "TRUE", "FALSE",
    "CLASS", "THIS", "NEW", "CONST", "GAME", "QT", "SLOT", "EMIT"
};

FrogGame::FrogGame(QObject* parent) : GameBase(parent) {
    // 1. 加载资源
    m_bgPixmap.load(":/img/frog_background.png");
    m_frogPixmap.load(":/img/frog_back_1.png");
    m_leafPixmap.load(":/img/frog_leaf.png");
    // 不需要加载 frog_start.png，因为背景图里已经画好了岸边

    // 2. 音效
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

    m_wordList = DEFAULT_WORDS;
}

FrogGame::~FrogGame() {
    qDeleteAll(m_leaves);
    m_leaves.clear();
}

void FrogGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    m_frogCount = 5;

    qDeleteAll(m_leaves);
    m_leaves.clear();

    // 【修改点】：初始化时直接铺满荷叶
    // 不需要等待它们飘进来，直接在屏幕内的固定位置生成
    double speeds[] = { 1.5, -2.0, 2.5 };

    for (int r = 0; r < 3; r++) {
        // 在屏幕宽度的 20%, 50%, 80% 处生成三个荷叶
        int positions[] = { 160, 400, 640 };

        for (int x : positions) {
            QString w = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
            m_leaves.append(new LotusLeaf(r, x, speeds[r], w));
        }
    }

    resetFrog();
    emit scoreChanged(0);
}

void FrogGame::resetFrog() {
    m_currentRow = -1;
    m_currentLeaf = nullptr;
    m_inputBuffer.clear();
    m_frogPos = QPointF(SCREEN_WIDTH / 2, START_BANK_Y);
    m_goalWord = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
}

void FrogGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
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

void FrogGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();
}

void FrogGame::onGameTick() {
    // 1. 生成新荷叶 (补充移出屏幕的空缺)
    spawnLeaves();

    // 2. 移动荷叶
    for (auto it = m_leaves.begin(); it != m_leaves.end(); ) {
        LotusLeaf* leaf = *it;
        leaf->x += leaf->speed;

        // 移除屏幕外的荷叶
        // 只有完全离开很远才删除，保证视觉连贯
        if (leaf->x < -200 || leaf->x > SCREEN_WIDTH + 200) {
            // 死亡判定：青蛙在被移除的荷叶上
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

    // 3. 青蛙跟随
    if (m_currentLeaf) {
        m_frogPos.setX(m_currentLeaf->x);
        m_frogPos.setY(ROW_Y[m_currentLeaf->row]);
    }
}

void FrogGame::spawnLeaves() {
    double speeds[] = { 1.5, -2.0, 2.5 };
    double minGap = 260.0; // 最小间距，确保一排最多3个

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

        if (speeds[r] > 0) { // 向右移，从左边补
            if (!hasLeaf || leftMost > (minGap - 100)) {
                spawnX = -100;
                needSpawn = true;
            }
        }
        else { // 向左移，从右边补
            if (!hasLeaf || rightMost < (SCREEN_WIDTH - minGap + 100)) {
                spawnX = SCREEN_WIDTH + 100;
                needSpawn = true;
            }
        }

        if (needSpawn) {
            if (QRandomGenerator::global()->bounded(100) < 10) {
                QString w = m_wordList[QRandomGenerator::global()->bounded(m_wordList.size())];
                m_leaves.append(new LotusLeaf(r, spawnX, speeds[r], w));
            }
        }
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

    // 目标：终点
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

    // 目标：下一排荷叶
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

    // 1. 绘制背景 (背景图中包含了草地、河流、终点岸边)
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
    }
    else {
        // 容错：没图才画色块
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(65, 105, 225));
    }

    // 2. 绘制终点单词
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.drawText(QRect(0, GOAL_BANK_Y - 20, SCREEN_WIDTH, 40), Qt::AlignCenter, m_goalWord);

    // 终点高亮
    if (m_currentRow == 2) {
        int w = painter.fontMetrics().horizontalAdvance(m_goalWord);
        int startX = (SCREEN_WIDTH - w) / 2;
        painter.setPen(Qt::yellow);
        painter.drawText(startX, GOAL_BANK_Y + 10, m_inputBuffer);
    }

    // 3. 绘制荷叶与文字
    for (LotusLeaf* leaf : m_leaves) {
        // (1) 绘制荷叶
        if (!m_leafPixmap.isNull()) {
            painter.drawPixmap(leaf->x - m_leafPixmap.width() / 2,
                ROW_Y[leaf->row] - m_leafPixmap.height() / 2,
                m_leafPixmap);
        }

        // (2) 绘制文字 (直接写在荷叶上，不需要画框)
        // 假设荷叶图片本身带有边框，我们只需居中绘制文字
        painter.setFont(QFont("Arial", 14, QFont::Bold));

        bool isTarget = (leaf->row == m_currentRow + 1) && leaf->word.startsWith(m_inputBuffer);

        // 微调文字高度，使其视觉居中
        int textY = ROW_Y[leaf->row] - 20;

        if (isTarget && !m_inputBuffer.isEmpty()) {
            QFontMetrics fm(painter.font());
            int totalW = fm.horizontalAdvance(leaf->word);
            int startX = leaf->x - totalW / 2;

            // 已输入 (黄色)
            painter.setPen(Qt::yellow);
            painter.drawText(startX, textY, m_inputBuffer);

            // 未输入
            int typedW = fm.horizontalAdvance(m_inputBuffer);
            painter.setPen(Qt::black);
            painter.drawText(startX + typedW, textY, leaf->word.mid(m_inputBuffer.length()));
        }
        else {
            // 普通状态
            painter.setPen(Qt::black);
            // 直接居中绘制
            QRect textRect(leaf->x - 60, ROW_Y[leaf->row] - 20, 120, 40);
            painter.drawText(textRect, Qt::AlignCenter, leaf->word);
        }
    }

    // 4. 绘制青蛙
    if (!m_frogPixmap.isNull()) {
        painter.drawPixmap(m_frogPos.x() - m_frogPixmap.width() / 2,
            m_frogPos.y() - m_frogPixmap.height() / 2,
            m_frogPixmap);
    }

    // 5. UI: 剩余青蛙
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