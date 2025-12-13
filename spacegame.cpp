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
const int TIME_CYCLE_SEC = 120;

SpaceGame::SpaceGame(QObject* parent) : GameBase(parent) {
    // 1. 加载资源
    m_bgPixmap.load(":/img/space_background.png");
    m_menuBgPixmap.load(":/img/space_mainmenu_bg.png");
    m_playerPixmap.load(":/img/space_ship.png");
    m_enemyPixmap.load(":/img/space_enemy_0.png");
    m_meteorPixmap.load(":/img/space_enemy_4.png");
    m_bulletPixmap.load(":/img/space_bomb.png");
    m_explosionPixmap.load(":/img/space_explosion_0.png");

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
    // 注意：不要 delete 按钮，它们由 GameWidget 自动管理
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
    if (m_btnGamePause) m_btnGamePause->show();
}
void SpaceGame::hideGameUI() {
    if (m_btnGamePause) m_btnGamePause->hide();
}

void SpaceGame::showMenuUI(bool isPauseMode) {
    if (m_btnOption) m_btnOption->show();
    if (m_btnHiscore) m_btnHiscore->show();
    if (m_btnExit) m_btnExit->show();

    if (isPauseMode) {
        if (m_btnStart) m_btnStart->hide();
        if (m_btnReturn) m_btnReturn->show();
    }
    else {
        if (m_btnStart) m_btnStart->show();
        if (m_btnReturn) m_btnReturn->hide();
    }
}
void SpaceGame::hideMenuUI() {
    if (m_btnStart) m_btnStart->hide();
    if (m_btnReturn) m_btnReturn->hide();
    if (m_btnOption) m_btnOption->hide();
    if (m_btnHiscore) m_btnHiscore->hide();
    if (m_btnExit) m_btnExit->hide();
}

void SpaceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    qDeleteAll(m_entities);
    m_entities.clear();

    m_playerPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);
    m_lives = m_settings.lives;

    m_difficultyLevel = m_settings.difficulty;
    m_spawnInterval = 80 - (m_difficultyLevel - 1) * 5;
    if (m_spawnInterval < 20) m_spawnInterval = 20;
    m_spawnTimer = 0;

    m_gameTimeFrames = TIME_CYCLE_SEC * GAME_FPS;
    m_playerDir = 1.0;

    emit scoreChanged(0);

    // 【修复】确保按钮在初始化时正确显示
    hideGameUI();
    showMenuUI(false);
}

void SpaceGame::startGame() {
    m_state = GameState::Playing;
    hideMenuUI();
    showGameUI();
    m_physicsTimer->start();
    m_bgMusic->play();
}

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
    // 1. 倒计时
    m_gameTimeFrames--;
    if (m_gameTimeFrames <= 0) {
        m_gameTimeFrames = TIME_CYCLE_SEC * GAME_FPS;
        m_difficultyLevel++;
        m_spawnInterval = qMax(20, m_spawnInterval - 5);
    }

    // 2. 玩家移动
    double playerSpeed = 4.0;
    m_playerPos.rx() += playerSpeed * m_playerDir;
    if (m_playerPos.x() < 50) { m_playerPos.setX(50); m_playerDir = 1.0; }
    else if (m_playerPos.x() > SCREEN_WIDTH - 50) { m_playerPos.setX(SCREEN_WIDTH - 50); m_playerDir = -1.0; }

    // 3. 生成敌人
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnEnemy();
        m_spawnTimer = 0;
    }

    // 4. 实体更新 (使用索引循环更安全，但这里只是移动，通常没事)
    for (SpaceEntity* e : m_entities) {
        if (!e->active) continue;

        // 子弹追踪
        if (e->type == Type_Bullet) {
            SpaceEntity* target = nullptr;
            if (!e->targetLetter.isEmpty()) {
                for (SpaceEntity* cand : m_entities) {
                    if (cand->type == Type_Enemy && cand->active && cand->letter == e->targetLetter) {
                        target = cand; break;
                    }
                }
            }
            if (target && target->active) {
                QPointF dir = target->pos - e->pos;
                double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
                if (len > 0.1) {
                    dir /= len;
                    e->velocity = dir * 15.0;
                }
            }
        }

        // S型移动
        if (e->type == Type_Enemy) {
            e->pos.ry() += e->velocity.y();
            double waveAmp = 80.0;
            double waveFreq = 0.015;
            e->pos.rx() = e->initialX + sin(e->pos.y() * waveFreq) * waveAmp;
        }
        else {
            e->pos += e->velocity;
        }

        if (e->type == Type_Enemy && e->pos.y() > SCREEN_HEIGHT + 50) e->active = false;
        else if (e->type == Type_Bullet && (e->pos.y() < -50 || e->pos.y() > SCREEN_HEIGHT)) e->active = false;
        else if (e->type == Type_Explosion) {
            e->lifeTime++;
            if (e->lifeTime > 20) e->active = false;
        }
    }

    // 5. 【关键修复】碰撞检测
    // 如果检测到死亡，立即返回，切断后续逻辑
    if (checkCollisions()) {
        handleGameOver();
        return;
    }

    // 6. 清理
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

// 【关键修复】使用索引遍历代替迭代器遍历，防止 createExplosion 导致迭代器失效
bool SpaceGame::checkCollisions() {
    int count = m_entities.size(); // 缓存当前大小
    for (int i = 0; i < count; ++i) {
        SpaceEntity* bullet = m_entities[i];
        if (bullet->type == Type_Bullet && bullet->active) {
            for (int j = 0; j < count; ++j) {
                SpaceEntity* enemy = m_entities[j];
                if (enemy->type == Type_Enemy && enemy->active) {
                    QLineF line(bullet->pos, enemy->pos);
                    if (line.length() < 40) {
                        bullet->active = false;
                        enemy->active = false;
                        // 这里 createExplosion 会 append 到 list，但不会影响 0~count 范围内的索引
                        createExplosion(enemy->pos);
                        m_explodeSound->play();
                        m_score += 100;
                        emit scoreChanged(m_score);
                        break;
                    }
                }
            }
        }
    }

    // 敌人撞玩家
    double playerRadius = 30.0;
    for (int i = 0; i < count; ++i) {
        SpaceEntity* enemy = m_entities[i];
        if (enemy->type == Type_Enemy && enemy->active) {
            QLineF line(m_playerPos, enemy->pos);
            if (line.length() < (playerRadius + 25)) {
                enemy->active = false;
                createExplosion(enemy->pos);
                m_lives--;
                m_explodeSound->play();

                if (m_lives <= 0) {
                    return true; // 触发游戏结束
                }
            }
        }
    }
    return false;
}

void SpaceGame::handleGameOver() {
    stopGame(); // 停止物理更新

    // 显示结算对话框
    SpaceNameDialog dlg(m_score, qobject_cast<QWidget*>(parent()));
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.getName();
        if (name.isEmpty()) name = "无名英雄";
        saveScore(name, m_score);
    }
    // 重置回主菜单
    initGame();
}

void SpaceGame::saveScore(const QString& name, int score) {
    QFile file("hiscore.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << name << "," << score << "," << QDate::currentDate().toString("yyyy-MM-dd") << "\n";
        file.close();
    }
}

void SpaceGame::draw(QPainter& painter) {
    if (m_state == GameState::Playing) {
        if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_bgPixmap);
        else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);

        painter.drawPixmap(m_playerPos.x() - m_playerPixmap.width() / 2, m_playerPos.y() - m_playerPixmap.height() / 2, m_playerPixmap);

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
        drawHUD(painter);
    }
    else {
        if (!m_menuBgPixmap.isNull()) painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_menuBgPixmap);
        else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
    }
}

void SpaceGame::drawHUD(QPainter& painter) {
    painter.fillRect(0, 0, SCREEN_WIDTH, 50, QColor(0, 0, 0, 100));

    int y = 10;
    if (!m_hudLabelScore.isNull()) painter.drawPixmap(20, y, m_hudLabelScore);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(80, y + 25, QString::number(m_score));

    int midX = 300;
    if (!m_hudLabelLife.isNull()) painter.drawPixmap(midX, y, m_hudLabelLife);
    for (int i = 0; i < m_lives; i++) {
        if (!m_hudLifeIcon.isNull()) painter.drawPixmap(midX + 70 + i * 35, y, 30, 30, m_hudLifeIcon);
    }

    int rightX = 600;
    if (!m_hudLabelTime.isNull()) painter.drawPixmap(rightX, y, m_hudLabelTime);
    int totalSec = m_gameTimeFrames / GAME_FPS;
    int mm = totalSec / 60;
    int ss = totalSec % 60;
    QString timeStr = QString("%1:%2").arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
    painter.drawText(rightX + 60, y + 25, timeStr);
}

void SpaceGame::spawnEnemy() {
    int x = QRandomGenerator::global()->bounded(50, SCREEN_WIDTH - 50);
    int speed = QRandomGenerator::global()->bounded(1 + m_difficultyLevel / 2, 3 + m_difficultyLevel / 2);
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    m_entities.append(new SpaceEntity(Type_Enemy, QPointF(x, -50), QPointF(0, speed), QString(letter)));
}

void SpaceGame::spawnBullet(const QPointF& startPos, const QString& targetLetter) {
    SpaceEntity* bullet = new SpaceEntity(Type_Bullet, startPos, QPointF(0, -15.0));
    bullet->targetLetter = targetLetter;
    m_entities.append(bullet);
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
        m_spawnInterval = 80 - (m_settings.difficulty - 1) * 5;
    }
}
void SpaceGame::onBtnHiscoreClicked() { qDebug() << "High Score clicked"; }
void SpaceGame::onBtnExitClicked() {
    stopGame();
    emit requestReturnToMenu();
}
void SpaceGame::onBtnGamePauseClicked() { pauseGame(); }

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
        QString tLetter = target ? target->letter : "";
        spawnBullet(m_playerPos, tLetter);
        m_shootSound->play();

    }
    else if (m_state == GameState::Paused) {
        if (event->key() == Qt::Key_Escape) resumeGame();
    }
}