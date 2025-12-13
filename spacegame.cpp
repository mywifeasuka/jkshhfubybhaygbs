#include "spacegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QWidget> // for qobject_cast

const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SpaceGame::SpaceGame(QObject* parent) : GameBase(parent) {
    // ... (资源加载代码保持不变) ...
    m_bgPixmap.load(":/img/space_background.png");
    m_menuBgPixmap.load(":/img/space_mainmenu_bg.png");
    // ... 其他贴图加载 ...
    m_playerPixmap.load(":/img/space_ship.png");
    m_enemyPixmap.load(":/img/space_enemy_0.png");
    m_meteorPixmap.load(":/img/space_enemy_4.png");
    m_bulletPixmap.load(":/img/space_bomb.png");
    m_explosionPixmap.load(":/img/space_explosion_0.png");

    m_shootSound = new QSoundEffect(this);
    m_shootSound->setSource(QUrl::fromLocalFile(":/snd/space_shoot.wav"));
    m_explodeSound = new QSoundEffect(this);
    m_explodeSound->setSource(QUrl::fromLocalFile(":/snd/space_blast.wav"));
    m_bgMusic = new QSoundEffect(this);
    m_bgMusic->setSource(QUrl::fromLocalFile(":/snd/space_bg.wav"));
    m_bgMusic->setLoopCount(QSoundEffect::Infinite);

    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &SpaceGame::onGameTick);

    QWidget* widgetParent = qobject_cast<QWidget*>(parent);
    m_settingsDialog = new SpaceGameSettings(widgetParent);

    setupInternalUI();
}

SpaceGame::~SpaceGame() {
    qDeleteAll(m_entities);
    m_entities.clear();
    // 按钮是子控件，会自动清理
    delete m_btnStart;
    delete m_btnReturn;
    delete m_btnOption;
    delete m_btnHiscore;
    delete m_btnExit;
    delete m_btnGamePause; // 【新增】
}

void SpaceGame::setupInternalUI() {
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    auto createBtn = [&](const QString& name) {
        return new ImageButton(
            ":/img/space_" + name + ".png",
            ":/img/space_" + name + "_hover.png",
            ":/img/space_" + name + "_pressed.png",
            parentWidget
        );
        };

    m_btnStart = createBtn("start");
    connect(m_btnStart, &ImageButton::clicked, this, &SpaceGame::onBtnStartClicked);

    m_btnReturn = createBtn("return");
    connect(m_btnReturn, &ImageButton::clicked, this, &SpaceGame::onBtnReturnClicked);

    m_btnOption = createBtn("option");
    connect(m_btnOption, &ImageButton::clicked, this, &SpaceGame::onBtnOptionClicked);

    m_btnHiscore = createBtn("hiscore");
    connect(m_btnHiscore, &ImageButton::clicked, this, &SpaceGame::onBtnHiscoreClicked);

    m_btnExit = createBtn("exit");
    connect(m_btnExit, &ImageButton::clicked, this, &SpaceGame::onBtnExitClicked);

    // --- 计算布局 ---
    // 假设有4个按钮的位置 (Start/Return 占同一个位置)
    // 获取按钮尺寸 (假设所有按钮尺寸一致，取Start的大小)
    m_btnStart->setFixedSizeToPixmap(); // 确保尺寸正确
    int btnW = m_btnStart->width();
    int btnH = m_btnStart->height();

    int count = 4; // Start, Option, Hiscore, Exit
    int spacing = 10; // 紧密排列，间距设为 10px
    int totalHeight = count * btnH + (count - 1) * spacing;

    int startY = (SCREEN_HEIGHT - totalHeight) / 2; // 垂直居中
    int centerX = (SCREEN_WIDTH - btnW) / 2;        // 水平居中

    // 1. 开始 / 返回 (位置重叠)
    m_btnStart->move(centerX, startY);
    m_btnReturn->move(centerX, startY);

    // 2. 选项
    m_btnOption->move(centerX, startY + btnH + spacing);

    // 3. 高分榜
    m_btnHiscore->move(centerX, startY + 2 * (btnH + spacing));

    // 4. 退出
    m_btnExit->move(centerX, startY + 3 * (btnH + spacing));

    // --- 游戏内暂停按钮 (保持左下角) ---
    m_btnGamePause = createBtn("pause");
    m_btnGamePause->move(20, 530);
    connect(m_btnGamePause, &ImageButton::clicked, this, &SpaceGame::onBtnGamePauseClicked);

    hideMenuUI();
    hideGameUI();
}

// 【新增】显示游戏内UI (暂停按钮)
void SpaceGame::showGameUI() {
    m_btnGamePause->show();
}

// 【新增】隐藏游戏内UI
void SpaceGame::hideGameUI() {
    m_btnGamePause->hide();
}

void SpaceGame::showMenuUI(bool isPauseMode) {
    m_btnOption->show();
    m_btnHiscore->show();
    m_btnExit->show();

    if (isPauseMode) {
        m_btnStart->hide();
        m_btnReturn->show();
    }
    else {
        m_btnStart->show();
        m_btnReturn->hide();
    }
}

void SpaceGame::hideMenuUI() {
    m_btnStart->hide();
    m_btnReturn->hide();
    m_btnOption->hide();
    m_btnHiscore->hide();
    m_btnExit->hide();
}

void SpaceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    qDeleteAll(m_entities);
    m_entities.clear();

    m_playerPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);
    m_lives = m_settings.lives;

    m_spawnInterval = 80 - (m_settings.difficulty - 1) * 5;
    m_spawnTimer = 0;
    m_difficultyTimer = 0;

    emit scoreChanged(0);

    showMenuUI(false);
    hideGameUI(); // 准备阶段不显示暂停按钮
}

void SpaceGame::startGame() {
    m_state = GameState::Playing;
    hideMenuUI();
    showGameUI(); // 【修改】显示左下角暂停按钮
    m_physicsTimer->start();
    m_bgMusic->play();
}

void SpaceGame::resumeGame() {
    if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        hideMenuUI();
        showGameUI(); // 【修改】恢复显示
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

void SpaceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop();
        m_bgMusic->stop();

        hideGameUI(); // 【修改】暂停时隐藏左下角按钮 (避免和菜单重叠)
        showMenuUI(true);
    }
}

void SpaceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();
    hideMenuUI();
    hideGameUI(); // 【修改】彻底隐藏
}

// 【新增】暂停按钮槽函数
void SpaceGame::onBtnGamePauseClicked() {
    pauseGame();
}

void SpaceGame::handleKeyPress(QKeyEvent* event) {
    if (m_state == GameState::Playing) {
        if (event->key() == Qt::Key_Escape) {
            pauseGame();
            return;
        }

        QString text = event->text().toUpper();
        if (text.isEmpty()) return;

        SpaceEntity* target = nullptr;
        double maxY = -1000;
        for (SpaceEntity* e : m_entities) {
            if (e->type == Type_Enemy && e->active && e->letter == text) {
                if (e->pos.y() > maxY) { maxY = e->pos.y(); target = e; }
            }
        }
        if (target) {
            spawnBullet(target->pos);
            m_shootSound->play();
        }
    }
    else if (m_state == GameState::Paused) {
        if (event->key() == Qt::Key_Escape) {
            resumeGame();
        }
    }
}

// ... onGameTick, draw, 其他按钮槽函数, spawnEnemy 等辅助函数保持不变 ...
// (为了代码完整性，请确保保留之前实现的所有其他函数)

void SpaceGame::onGameTick() {
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnEnemy();
        m_spawnTimer = 0;
    }
    for (int i = 0; i < m_entities.size(); ++i) {
        SpaceEntity* e = m_entities[i];
        if (!e->active) continue;
        e->pos += e->velocity;

        if (e->type == Type_Enemy && e->pos.y() > SCREEN_HEIGHT) {
            e->active = false;
        }
        if (e->type == Type_Bullet) {
            for (SpaceEntity* target : m_entities) {
                if (target->type == Type_Enemy && target->active) {
                    QLineF line(e->pos, target->pos);
                    if (line.length() < 40) {
                        e->active = false;
                        target->active = false;
                        createExplosion(target->pos);
                        m_explodeSound->play();
                        m_score += 100;
                        emit scoreChanged(m_score);
                        break;
                    }
                }
            }
        }
        else if (e->type == Type_Explosion) {
            e->lifeTime++;
            if (e->lifeTime > 20) e->active = false;
        }
    }
    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (!(*it)->active) {
            delete* it;
            it = m_entities.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SpaceGame::draw(QPainter& painter) {
    if (m_state == GameState::Playing) {
        // 绘制游戏背景 (拉伸铺满)
        if (!m_bgPixmap.isNull()) {
            painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
        }
        else {
            painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
        }

        // 玩家
        QPointF shipPos(m_playerPos.x() - m_playerPixmap.width() / 2, m_playerPos.y() - m_playerPixmap.height() / 2);
        painter.drawPixmap(shipPos, m_playerPixmap);

        // 实体
        for (SpaceEntity* e : m_entities) {
            if (!e->active) continue;
            QPointF dp = e->pos;
            if (e->type == Type_Enemy) {
                painter.drawPixmap(dp.x() - m_enemyPixmap.width() / 2, dp.y() - m_enemyPixmap.height() / 2, m_enemyPixmap);
                painter.setBrush(Qt::white); painter.setPen(Qt::black);
                painter.drawRect(dp.x() - 15, dp.y() + 20, 30, 20);
                painter.setFont(QFont("Arial", 12, QFont::Bold));
                painter.drawText(QRect(dp.x() - 15, dp.y() + 20, 30, 20), Qt::AlignCenter, e->letter);
            }
            else if (e->type == Type_Bullet) {
                painter.drawPixmap(dp.x() - m_bulletPixmap.width() / 2, dp.y() - m_bulletPixmap.height() / 2, m_bulletPixmap);
            }
            else if (e->type == Type_Explosion) {
                painter.drawPixmap(dp.x() - m_explosionPixmap.width() / 2, dp.y() - m_explosionPixmap.height() / 2, m_explosionPixmap);
            }
        }

        // HUD
        painter.setPen(Qt::white); painter.setFont(QFont("Arial", 16));
        painter.drawText(10, 30, QString("Score: %1").arg(m_score));
        painter.drawText(10, 60, QString("Lives: %1").arg(m_lives));
    }
    else {
        // 绘制菜单背景 (拉伸铺满)
        if (!m_menuBgPixmap.isNull()) {
            painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_menuBgPixmap);
        }
        else {
            painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
        }
    }
}

void SpaceGame::onBtnStartClicked() {
    m_score = 0;
    m_lives = m_settings.lives;
    qDeleteAll(m_entities);
    m_entities.clear();
    startGame();
}
void SpaceGame::onBtnReturnClicked() { resumeGame(); }
void SpaceGame::onBtnOptionClicked() {
    m_settingsDialog->setSettings(m_settings);
    if (m_settingsDialog->exec() == QDialog::Accepted) {
        m_settings = m_settingsDialog->getSettings();
        m_spawnInterval = 80 - (m_settings.difficulty - 1) * 5;
    }
}
void SpaceGame::onBtnHiscoreClicked() { qDebug() << "High Score clicked"; }
void SpaceGame::onBtnExitClicked() {
    stopGame();
    emit requestReturnToMenu();
}
void SpaceGame::spawnEnemy() {
    int x = QRandomGenerator::global()->bounded(50, SCREEN_WIDTH - 50);
    int speed = QRandomGenerator::global()->bounded(2, 5);
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    m_entities.append(new SpaceEntity(Type_Enemy, QPointF(x, -50), QPointF(0, speed), QString(letter)));
}
void SpaceGame::spawnBullet(const QPointF& targetPos) {
    QPointF startPos = m_playerPos;
    QPointF dir = targetPos - startPos;
    double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len > 0) dir /= len;
    m_entities.append(new SpaceEntity(Type_Bullet, startPos, dir * 15.0));
}
void SpaceGame::createExplosion(const QPointF& pos) {
    m_entities.append(new SpaceEntity(Type_Explosion, pos, QPointF(0, 0)));
}