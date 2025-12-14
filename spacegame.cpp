#include "spacegame.h"
#include "spacehighscoredialog.h" 
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
    m_bgPixmap.load(":/img/space_background.png");
    m_menuBgPixmap.load(":/img/space_mainmenu_bg.png");
    m_playerPixmap.load(":/img/space_ship.png");
    m_enemyPixmap.load(":/img/space_enemy_0.png");
    m_meteorPixmap.load(":/img/space_enemy_4.png");
    m_bulletPixmap.load(":/img/space_bomb.png");
    m_explosionPixmap.load(":/img/space_explosion_0.png");

    m_inputBgPixmap.load(":/img/space_hiscore_bg.png");
    if (m_inputBgPixmap.isNull()) {
        m_inputBgPixmap.load(":/img/space_mainmenu_bg.png");
    }

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

    m_isInputActive = false;
    m_inputName = "";
}

SpaceGame::~SpaceGame() {
    qDeleteAll(m_entities);
    m_entities.clear();
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
    int spacing = 10;
    int totalHeight = 4 * btnH + 3 * spacing;
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
    m_isInputActive = false;

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
    showMenuUI(false);
    hideGameUI();

    // 确保主窗口重绘
    QWidget* parent = qobject_cast<QWidget*>(this->parent());
    if (parent) parent->update();
}

void SpaceGame::startGame() {
    m_state = GameState::Playing;
    hideMenuUI();
    showGameUI();
    m_physicsTimer->start();
    m_bgMusic->play();

    // 确保获得焦点以便接收键盘事件
    QWidget* parent = qobject_cast<QWidget*>(this->parent());
    if (parent) parent->setFocus();
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
    m_physicsTimer->stop();
    m_bgMusic->stop();
    hideMenuUI();
    hideGameUI();
    m_isInputActive = false;
}

void SpaceGame::onGameTick() {
    m_gameTimeFrames--;
    if (m_gameTimeFrames <= 0) {
        m_gameTimeFrames = TIME_CYCLE_SEC * GAME_FPS;
        m_difficultyLevel++;
        m_spawnInterval = qMax(20, m_spawnInterval - 5);
    }

    double playerSpeed = 4.0;
    m_playerPos.rx() += playerSpeed * m_playerDir;
    if (m_playerPos.x() < 50) { m_playerPos.setX(50); m_playerDir = 1.0; }
    else if (m_playerPos.x() > SCREEN_WIDTH - 50) { m_playerPos.setX(SCREEN_WIDTH - 50); m_playerDir = -1.0; }

    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnEnemy();
        m_spawnTimer = 0;
    }

    for (SpaceEntity* e : m_entities) {
        if (!e->active) continue;

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

    if (checkCollisions()) {
        handleGameOver();
        return;
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

bool SpaceGame::checkCollisions() {
    int count = m_entities.size();

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
                    return true;
                }
            }
        }
    }
    return false;
}

void SpaceGame::handleGameOver() {
    m_physicsTimer->stop();
    m_bgMusic->stop();
    hideGameUI();

    m_isInputActive = true;
    m_inputName = "Hero";

    QWidget* parent = qobject_cast<QWidget*>(this->parent());
    if (parent) parent->update();
}

void SpaceGame::saveScore(const QString& name, int score) {
    QFile file("hiscore.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // 使用 UTF-8 写入
        out.setCodec("UTF-8");
        out << name << "," << score << "," << QDate::currentDate().toString("yyyy-MM-dd") << "\n";
        file.close();
    }
}

void SpaceGame::handleKeyPress(QKeyEvent* event) {
    if (m_isInputActive) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            saveScore(m_inputName, m_score);
            initGame();
        }
        else if (event->key() == Qt::Key_Backspace) {
            if (!m_inputName.isEmpty()) m_inputName.chop(1);
        }
        else {
            QString text = event->text();
            if (!text.isEmpty() && m_inputName.length() < 10) {
                if (text.at(0).isPrint()) {
                    m_inputName += text;
                }
            }
        }

        QWidget* parent = qobject_cast<QWidget*>(this->parent());
        if (parent) parent->update();
        return;
    }

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

        if (m_isInputActive) {
            drawNameInput(painter);
        }
    }
    else {
        if (!m_menuBgPixmap.isNull()) painter.drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, m_menuBgPixmap);
        else painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
    }
}

void SpaceGame::drawNameInput(QPainter& painter) {
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(0, 0, 0, 150));

    int boxW = 400; int boxH = 300;
    int boxX = (SCREEN_WIDTH - boxW) / 2;
    int boxY = (SCREEN_HEIGHT - boxH) / 2;

    if (!m_inputBgPixmap.isNull()) {
        painter.drawPixmap(boxX, boxY, boxW, boxH, m_inputBgPixmap);
    }
    else {
        painter.fillRect(boxX, boxY, boxW, boxH, Qt::darkBlue);
        painter.setPen(Qt::white);
        painter.drawRect(boxX, boxY, boxW, boxH);
    }

    painter.setPen(Qt::yellow);
    painter.setFont(QFont("Microsoft YaHei", 20, QFont::Bold));
    painter.drawText(QRect(boxX, boxY + 40, boxW, 40), Qt::AlignCenter, QString::fromWCharArray(L"请留下您的大名"));

    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 16));
    painter.drawText(QRect(boxX, boxY + 90, boxW, 30), Qt::AlignCenter, QString::fromWCharArray(L"最终得分: %1").arg(m_score));

    int editW = 240; int editH = 40;
    int editX = (SCREEN_WIDTH - editW) / 2;
    int editY = boxY + 140;

    painter.setBrush(QColor(0, 0, 0, 100));
    painter.setPen(QPen(QColor(74, 144, 226), 2));
    painter.drawRect(editX, editY, editW, editH);

    painter.setPen(Qt::white);
    painter.drawText(QRect(editX, editY, editW, editH), Qt::AlignCenter, m_inputName + "|");

    painter.setPen(Qt::lightGray);
    painter.setFont(QFont("Microsoft YaHei", 12));
    painter.drawText(QRect(boxX, boxY + 220, boxW, 30), Qt::AlignCenter, QString::fromWCharArray(L"按 Enter 键确认提交"));
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

void SpaceGame::onBtnHiscoreClicked() {
    SpaceHighscoreDialog dlg(qobject_cast<QWidget*>(parent()));
    dlg.exec();
}

void SpaceGame::onBtnExitClicked() {
    stopGame();
    emit requestReturnToMenu();
}
void SpaceGame::onBtnGamePauseClicked() {
    if (m_isInputActive) return;
    pauseGame();
}