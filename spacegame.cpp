#include "spacegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QWidget> 
#include <QFile>
#include <QTextStream>
#include <QDate>

const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TIME_CYCLE_SEC = 120; // 2分钟一轮

SpaceGame::SpaceGame(QObject* parent) : GameBase(parent) {
    // 1. 加载资源
    m_bgPixmap.load(":/img/space_background.png");
    m_menuBgPixmap.load(":/img/space_mainmenu_bg.png");
    m_playerPixmap.load(":/img/space_ship.png");
    m_enemyPixmap.load(":/img/space_enemy_0.png");
    m_meteorPixmap.load(":/img/space_enemy_4.png");
    m_bulletPixmap.load(":/img/space_bomb.png");
    m_explosionPixmap.load(":/img/space_explosion_0.png");

    // 加载 HUD 资源
    m_hudLabelScore.load(":/img/space_label_score.png");
    m_hudLabelLife.load(":/img/space_label_life.png");
    m_hudLabelTime.load(":/img/space_label_time.png");
    m_hudLifeIcon.load(":/img/space_life.png");

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
    delete m_btnStart; delete m_btnReturn; delete m_btnOption;
    delete m_btnHiscore; delete m_btnExit; delete m_btnGamePause;
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

    // 布局计算 (下移1.5按钮高度)
    m_btnStart->setFixedSizeToPixmap();
    int btnW = m_btnStart->width();
    int btnH = m_btnStart->height();
    int count = 4; int spacing = 10;
    int totalHeight = count * btnH + (count - 1) * spacing;
    int startY = (SCREEN_HEIGHT - totalHeight) / 2 + (int)(1.5 * btnH);
    int centerX = (SCREEN_WIDTH - btnW) / 2;

    m_btnStart->move(centerX, startY);
    m_btnReturn->move(centerX, startY);
    m_btnOption->move(centerX, startY + btnH + spacing);
    m_btnHiscore->move(centerX, startY + 2 * (btnH + spacing));
    m_btnExit->move(centerX, startY + 3 * (btnH + spacing));

    m_btnGamePause = createBtn("exit");
    m_btnGamePause->move(20, 530);
    connect(m_btnGamePause, &ImageButton::clicked, this, &SpaceGame::onBtnGamePauseClicked);

    hideMenuUI();
    hideGameUI();
}

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

    // 初始化难度参数
    m_difficultyLevel = m_settings.difficulty;
    m_spawnInterval = 80 - (m_difficultyLevel - 1) * 5;
    if (m_spawnInterval < 20) m_spawnInterval = 20;
    m_spawnTimer = 0;

    // 倒计时 2 分钟
    m_gameTimeFrames = TIME_CYCLE_SEC * GAME_FPS;

    // 玩家初始移动方向
    m_playerDir = 1.0;

    emit scoreChanged(0);
    showMenuUI(false);
    hideGameUI();
}

void SpaceGame::startGame() {
    m_state = GameState::Playing;
    hideMenuUI();
    showGameUI();
    m_physicsTimer->start();
    m_bgMusic->play();
}

// ... resumeGame, pauseGame, stopGame 保持不变 ...
void SpaceGame::resumeGame() {
    if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        hideMenuUI(); showGameUI(); m_physicsTimer->start(); m_bgMusic->play();
    }
}
void SpaceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop(); m_bgMusic->stop(); hideGameUI(); showMenuUI(true);
    }
}
void SpaceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop(); m_bgMusic->stop(); hideMenuUI(); hideGameUI();
}

void SpaceGame::onGameTick() {
    // 1. 倒计时逻辑
    m_gameTimeFrames--;
    if (m_gameTimeFrames <= 0) {
        // 时间到，增加难度，重置时间
        m_gameTimeFrames = TIME_CYCLE_SEC * GAME_FPS;
        m_difficultyLevel++;
        // 加快生成
        m_spawnInterval = qMax(20, m_spawnInterval - 5);
        // 可以播放一个提示音效或显示文字 "Difficulty Up!" (此处略)
    }

    // 2. 玩家自动左右移动 (慢速)
    double playerSpeed = 1.0; // 慢速
    m_playerPos.rx() += playerSpeed * m_playerDir;

    // 边界反弹
    if (m_playerPos.x() < 50) {
        m_playerPos.setX(50);
        m_playerDir = 1.0;
    }
    else if (m_playerPos.x() > SCREEN_WIDTH - 50) {
        m_playerPos.setX(SCREEN_WIDTH - 50);
        m_playerDir = -1.0;
    }

    // 3. 生成敌人
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnEnemy();
        m_spawnTimer = 0;
    }

    // 4. 实体更新
    for (SpaceEntity* e : m_entities) {
        if (!e->active) continue;

        // 移动逻辑
        if (e->type == Type_Enemy) {
            // S型移动: y 匀速增加, x = initialX + sin(y * freq) * amp
            e->pos.ry() += e->velocity.y(); // Y轴速度
            double waveAmp = 80.0; // 摆动幅度
            double waveFreq = 0.015; // 摆动频率
            e->pos.rx() = e->initialX + sin(e->pos.y() * waveFreq) * waveAmp;
        }
        else {
            // 子弹/爆炸 普通移动
            e->pos += e->velocity;
        }

        // 边界检查
        if (e->type == Type_Enemy && e->pos.y() > SCREEN_HEIGHT + 50) {
            e->active = false;
            // 漏怪不扣血，直接移除
        }
        else if (e->type == Type_Bullet && (e->pos.y() < -50 || e->pos.y() > SCREEN_HEIGHT)) {
            e->active = false;
        }
        else if (e->type == Type_Explosion) {
            e->lifeTime++;
            if (e->lifeTime > 20) e->active = false;
        }
    }

    // 5. 碰撞检测
    checkCollisions();

    // 6. 清理实体
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

void SpaceGame::checkCollisions() {
    // 1. 子弹击中敌人
    for (SpaceEntity* bullet : m_entities) {
        if (bullet->type == Type_Bullet && bullet->active) {
            for (SpaceEntity* enemy : m_entities) {
                if (enemy->type == Type_Enemy && enemy->active) {
                    QLineF line(bullet->pos, enemy->pos);
                    if (line.length() < 40) { // 命中半径
                        bullet->active = false;
                        enemy->active = false;
                        createExplosion(enemy->pos);
                        m_explodeSound->play();
                        m_score += 100;
                        emit scoreChanged(m_score);
                        break; // 子弹只能打一个
                    }
                }
            }
        }
    }

    // 2. 敌人撞击玩家
    // 定义玩家判定区 (简单圆形或矩形)
    double playerRadius = 30.0;
    for (SpaceEntity* enemy : m_entities) {
        if (enemy->type == Type_Enemy && enemy->active) {
            QLineF line(m_playerPos, enemy->pos);
            if (line.length() < (playerRadius + 25)) { // 敌人半径约25
                // 发生碰撞
                enemy->active = false;
                createExplosion(enemy->pos);
                m_lives--; // 扣血
                m_explodeSound->play(); // 撞击音效

                if (m_lives <= 0) {
                    handleGameOver();
                    return; // 游戏结束，停止本帧逻辑
                }
            }
        }
    }
}

void SpaceGame::handleGameOver() {
    stopGame();
    // 弹出输入名字对话框
    SpaceNameDialog dlg(m_score, qobject_cast<QWidget*>(parent()));
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.getName();
        if (name.isEmpty()) name = "无名英雄";
        saveScore(name, m_score);
    }
    // 回到主菜单 (SpaceGame 内部菜单)
    initGame();
}

void SpaceGame::saveScore(const QString& name, int score) {
    // 简单追加保存到 text 文件
    QFile file("hiscore.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // 格式: Name,Score,Date
        out << name << "," << score << "," << QDate::currentDate().toString("yyyy-MM-dd") << "\n";
        file.close();
    }
}

void SpaceGame::draw(QPainter& painter) {
    if (m_state == GameState::Playing) {
        // 背景铺满
        if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
        else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);

        // 玩家
        painter.drawPixmap(m_playerPos.x() - m_playerPixmap.width() / 2, m_playerPos.y() - m_playerPixmap.height() / 2, m_playerPixmap);

        // 实体
        for (SpaceEntity* e : m_entities) {
            if (!e->active) continue;
            QPointF dp = e->pos;
            if (e->type == Type_Enemy) {
                painter.drawPixmap(dp.x() - m_enemyPixmap.width() / 2, dp.y() - m_enemyPixmap.height() / 2, m_enemyPixmap);
                // 绘制字母背景框和字母
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

        // 绘制 HUD
        drawHUD(painter);
    }
    else {
        // 菜单背景
        if (!m_menuBgPixmap.isNull()) painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_menuBgPixmap);
        else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
    }
}

void SpaceGame::drawHUD(QPainter& painter) {
    // 顶部栏背景 (可选，半透明黑条)
    painter.fillRect(0, 0, SCREEN_WIDTH, 50, QColor(0, 0, 0, 100));

    int y = 10;
    // 1. 分数 (左)
    if (!m_hudLabelScore.isNull()) painter.drawPixmap(20, y, m_hudLabelScore);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(80, y + 25, QString::number(m_score));

    // 2. 生命 (中) - 假设 label_life 是图标，后面画条
    int midX = 300;
    if (!m_hudLabelLife.isNull()) painter.drawPixmap(midX, y, m_hudLabelLife);
    // 画生命条: 每一条命一个图标
    for (int i = 0; i < m_lives; i++) {
        if (!m_hudLifeIcon.isNull()) {
            painter.drawPixmap(midX + 70 + i * 35, y, 30, 30, m_hudLifeIcon);
        }
        else {
            // 没图画红框
            painter.setBrush(Qt::red);
            painter.drawRect(midX + 70 + i * 35, y + 5, 20, 20);
        }
    }

    // 3. 倒计时 (右)
    int rightX = 600;
    if (!m_hudLabelTime.isNull()) painter.drawPixmap(rightX, y, m_hudLabelTime);

    // 计算分秒
    int totalSec = m_gameTimeFrames / GAME_FPS;
    int mm = totalSec / 60;
    int ss = totalSec % 60;
    QString timeStr = QString("%1:%2").arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));

    painter.drawText(rightX + 60, y + 25, timeStr);
}

// 辅助函数
void SpaceGame::spawnEnemy() {
    int x = QRandomGenerator::global()->bounded(50, SCREEN_WIDTH - 50);
    // 难度越高，下落越快
    int speed = QRandomGenerator::global()->bounded(2 + m_difficultyLevel / 2, 5 + m_difficultyLevel / 2);
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    // 传入初始位置和速度
    m_entities.append(new SpaceEntity(Type_Enemy, QPointF(x, -50), QPointF(0, speed), QString(letter)));
}
// ... spawnBullet, createExplosion, 按钮槽函数等保持不变 ...
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
        // 更新难度
        m_difficultyLevel = m_settings.difficulty;
        m_spawnInterval = 80 - (m_difficultyLevel - 1) * 5;
    }
}
void SpaceGame::onBtnHiscoreClicked() { qDebug() << "High Score clicked"; }
void SpaceGame::onBtnExitClicked() {
    stopGame();
    emit requestReturnToMenu();
}
void SpaceGame::onBtnGamePauseClicked() { pauseGame(); }
// handleKeyPress 已包含在类定义中，此处实现略，同上文
void SpaceGame::handleKeyPress(QKeyEvent* event) {
    if (m_state == GameState::Playing) {
        if (event->key() == Qt::Key_Escape) { pauseGame(); return; }
        QString text = event->text().toUpper();
        if (text.isEmpty()) return;
        SpaceEntity* target = nullptr;
        double maxY = -1000;
        for (SpaceEntity* e : m_entities) {
            if (e->type == Type_Enemy && e->active && e->letter == text) {
                if (e->pos.y() > maxY) { maxY = e->pos.y(); target = e; }
            }
        }
        if (target) { spawnBullet(target->pos); m_shootSound->play(); }
    }
    else if (m_state == GameState::Paused) {
        if (event->key() == Qt::Key_Escape) resumeGame();
    }
}