#include "spacegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QWidget> // for qobject_cast

const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SpaceGame::SpaceGame(QObject* parent) : GameBase(parent) {
    // 1. 加载资源
    m_bgPixmap.load(":/img/space_background.png");
    m_menuBgPixmap.load(":/img/space_mainmenu_bg.png"); // 菜单背景
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

    // 初始化设置窗口
    QWidget* widgetParent = qobject_cast<QWidget*>(parent);
    m_settingsDialog = new SpaceGameSettings(widgetParent);

    // 初始化内部UI按钮 (挂载到 GameWidget 上)
    setupInternalUI();
}

SpaceGame::~SpaceGame() {
    qDeleteAll(m_entities);
    m_entities.clear();
    // 按钮和设置对话框是 GameWidget 的子控件，会被自动清理，或者在这里手动清理也可
    delete m_btnStart;
    delete m_btnReturn;
    delete m_btnOption;
    delete m_btnHiscore;
    delete m_btnExit;
}

void SpaceGame::setupInternalUI() {
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    // 辅助 Lambda
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

    // 设置位置 (假设垂直居中排列，按钮宽约200高50，根据实际图调整)
    int centerX = (SCREEN_WIDTH - 200) / 2; // 假设按钮宽200
    int startY = 250;
    int gap = 60;

    m_btnStart->move(centerX, startY);
    m_btnReturn->move(centerX, startY); // 位置相同，互斥显示
    m_btnOption->move(centerX, startY + gap);
    m_btnHiscore->move(centerX, startY + gap * 2);
    m_btnExit->move(centerX, startY + gap * 3);

    // 初始全部隐藏
    hideMenuUI();
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

    // 根据难度设置初始生成间隔
    // 难度1 -> 80帧, 难度10 -> 35帧
    m_spawnInterval = 80 - (m_settings.difficulty - 1) * 5;
    m_spawnTimer = 0;
    m_difficultyTimer = 0;

    emit scoreChanged(0);

    // 显示主菜单
    showMenuUI(false);
}

void SpaceGame::startGame() {
    // 真正的开始逻辑
    m_state = GameState::Playing;
    hideMenuUI();
    m_physicsTimer->start();
    m_bgMusic->play();
}

void SpaceGame::resumeGame() {
    if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        hideMenuUI();
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

void SpaceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop();
        m_bgMusic->stop();
        showMenuUI(true); // 显示带"返回"的菜单
    }
}

void SpaceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();
    hideMenuUI(); // 彻底退出时隐藏自己的UI
}



// *** 核心物理循环 ***
void SpaceGame::onGameTick() {
    // 1. 生成敌机
    m_spawnTimer++;
    if (m_spawnTimer >= m_spawnInterval) {
        spawnEnemy();
        m_spawnTimer = 0;
    }

    // 2. 增加难度
    m_difficultyTimer++;
    if (m_difficultyTimer > 600) { // 每10秒增加难度
        m_spawnInterval = qMax(10, m_spawnInterval - 5); // 加快生成
        m_difficultyTimer = 0;
    }

    // 3. 更新所有实体
    for (int i = 0; i < m_entities.size(); ++i) {
        SpaceEntity* e = m_entities[i];
        if (!e->active) continue;

        // 移动
        e->pos += e->velocity;

        // 逻辑处理
        if (e->type == Type_Enemy) {
            // 敌机飞出屏幕底部 -> 游戏结束或扣分
            if (e->pos.y() > SCREEN_HEIGHT) {
                e->active = false;
                // 这里简单处理：漏掉一个扣分，或者太严苛就直接结束
                // m_score = qMax(0, m_score - 50); 
                // emit scoreChanged(m_score);
            }
        }
        else if (e->type == Type_Bullet) {
            // 子弹飞出屏幕
            if (e->pos.y() < -50 || e->pos.y() > SCREEN_HEIGHT || e->pos.x() < 0 || e->pos.x() > SCREEN_WIDTH) {
                e->active = false;
            }
            
            // 子弹碰撞检测 (简单的距离检测)
            for (SpaceEntity* target : m_entities) {
                if (target->type == Type_Enemy && target->active) {
                    QLineF line(e->pos, target->pos);
                    if (line.length() < 40) { // 命中半径
                        e->active = false;      // 子弹消失
                        target->active = false; // 敌机消失
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
            if (e->lifeTime > 20) e->active = false; // 爆炸持续20帧
        }
    }

    // 4. 清理死亡实体 (简单的垃圾回收)
    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (!(*it)->active) {
            delete *it;
            it = m_entities.erase(it);
        } else {
            ++it;
        }
    }
}

void SpaceGame::onBtnStartClicked() {
    // "开始" -> 新游戏
    // 注意：initGame 会重置数据，但我们已经在 initGame 里显示了 UI
    // 这里我们只需要重置必要的状态并开始
    // 简单起见，调用 initGame 重置一次数据（不重置状态），然后 startGame
    // 为了保留 "initGame" 仅作初始化用，我们在这里手动重置数据
    m_score = 0;
    m_lives = m_settings.lives;
    qDeleteAll(m_entities);
    m_entities.clear();
    startGame();
}

void SpaceGame::onBtnReturnClicked() {
    resumeGame();
}

void SpaceGame::onBtnOptionClicked() {
    SpaceSettingsData oldS = m_settings;
    m_settingsDialog->setSettings(m_settings);

    if (m_settingsDialog->exec() == QDialog::Accepted) {
        m_settings = m_settingsDialog->getSettings();
        // 如果难度变了，可能需要实时调整 m_spawnInterval
        m_spawnInterval = 80 - (m_settings.difficulty - 1) * 5;
    }
}

void SpaceGame::onBtnHiscoreClicked() {
    // 暂未实现，或者弹出一个简单的 MessageBox
    qDebug() << "High Score clicked";
}

void SpaceGame::onBtnExitClicked() {
    stopGame(); // 停止定时器，隐藏 SpaceGame 按钮
    emit requestReturnToMenu(); // 通知 GameWidget 切换界面
}

void SpaceGame::spawnEnemy() {
    int x = QRandomGenerator::global()->bounded(50, SCREEN_WIDTH - 50);
    int speed = QRandomGenerator::global()->bounded(2, 5); // 随机速度
    char letter = 'A' + QRandomGenerator::global()->bounded(26);
    
    // 随机决定是飞机还是陨石
    EntityType type = Type_Enemy; 
    
    SpaceEntity* enemy = new SpaceEntity(type, QPointF(x, -50), QPointF(0, speed), QString(letter));
    m_entities.append(enemy);
}

void SpaceGame::spawnBullet(const QPointF& targetPos) {
    QPointF startPos = m_playerPos;
    
    // 计算向量，让子弹飞向目标
    QLineF trajectory(startPos, targetPos);
    double angle = trajectory.angle(); // 0-360
    double radian = qDegreesToRadians(angle);
    
    double speed = 15.0;
    QPointF velocity(cos(radian) * speed, -sin(radian) * speed); // Qt坐标系Y轴向下，sin需取反? QLineF angle 0 is 3 o'clock.
    // 简单点：直接计算差值向量并归一化
    QPointF dir = targetPos - startPos;
    double len = std::sqrt(dir.x()*dir.x() + dir.y()*dir.y());
    if (len > 0) {
        dir /= len;
        m_entities.append(new SpaceEntity(Type_Bullet, startPos, dir * speed));
    }
}

void SpaceGame::createExplosion(const QPointF& pos) {
    m_entities.append(new SpaceEntity(Type_Explosion, pos, QPointF(0,0)));
}

// *** 核心输入处理 ***
void SpaceGame::handleKeyPress(QKeyEvent *event) {
    if (m_state != GameState::Playing) return;
    
    QString text = event->text().toUpper();
    if (text.isEmpty()) return;
    
    // 寻找屏幕上最下方的、匹配该字母的敌机
    SpaceEntity* target = nullptr;
    double maxY = -1000;
    
    for (SpaceEntity* e : m_entities) {
        if (e->type == Type_Enemy && e->active && e->letter == text) {
            if (e->pos.y() > maxY) {
                maxY = e->pos.y();
                target = e;
            }
        }
    }
    
    if (target) {
        // 找到了目标，发射子弹！
        spawnBullet(target->pos);
        m_shootSound->play();
    } else {
        // 打错了，可以扣分或播放错误音效
    }
}

// *** 绘制 ***
void SpaceGame::draw(QPainter &painter) {
    // 1. 背景
    painter.drawPixmap(0, 0, m_bgPixmap);

    // 2. 玩家飞机
    QPointF shipPos(m_playerPos.x() - m_playerPixmap.width()/2, m_playerPos.y() - m_playerPixmap.height()/2);
    painter.drawPixmap(shipPos, m_playerPixmap);

    // 3. 实体绘制
    for (SpaceEntity* e : m_entities) {
        if (!e->active) continue;
        
        QPointF drawPos = e->pos;
        
        if (e->type == Type_Enemy) {
            // 随机选陨石图或敌机图，这里简化统一用敌机图，你可以根据需要扩展
            QPixmap& p = m_enemyPixmap; 
            painter.drawPixmap(drawPos.x() - p.width()/2, drawPos.y() - p.height()/2, p);
            
            // 绘制字母气泡
            painter.setBrush(Qt::white);
            painter.setPen(Qt::black);
            painter.drawRect(drawPos.x() - 15, drawPos.y() + 20, 30, 20);
            painter.setFont(QFont("Arial", 12, QFont::Bold));
            painter.drawText(QRect(drawPos.x() - 15, drawPos.y() + 20, 30, 20), Qt::AlignCenter, e->letter);
        }
        else if (e->type == Type_Bullet) {
            painter.drawPixmap(drawPos.x() - m_bulletPixmap.width()/2, drawPos.y() - m_bulletPixmap.height()/2, m_bulletPixmap);
        }
        else if (e->type == Type_Explosion) {
            painter.drawPixmap(drawPos.x() - m_explosionPixmap.width()/2, drawPos.y() - m_explosionPixmap.height()/2, m_explosionPixmap);
        }
    }
    
    // 4. UI
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16));
    painter.drawText(10, 30, QString("Score: %1").arg(m_score));
}