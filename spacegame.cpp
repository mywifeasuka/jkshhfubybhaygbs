#include "spacegame.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>

const int GAME_FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SpaceGame::SpaceGame(QObject *parent) : GameBase(parent) {
    // 1. 加载资源 (根据你的 resources.qrc 命名)
    m_bgPixmap.load(":/img/space_background.png");
    m_playerPixmap.load(":/img/space_ship.png");
    m_enemyPixmap.load(":/img/space_enemy_0.png");
    m_meteorPixmap.load(":/img/space_enemy_4.png");
    m_bulletPixmap.load(":/img/space_bomb.png");
    m_explosionPixmap.load(":/img/space_explosion_0.png");

    // 2. 加载音效
    m_shootSound = new QSoundEffect(this);
    m_shootSound->setSource(QUrl::fromLocalFile(":/snd/space_shoot.wav"));
    
    m_explodeSound = new QSoundEffect(this);
    m_explodeSound->setSource(QUrl::fromLocalFile(":/snd/space_blast.wav"));

    m_bgMusic = new QSoundEffect(this);
    m_bgMusic->setSource(QUrl::fromLocalFile(":/snd/space_bg.wav"));
    m_bgMusic->setLoopCount(QSoundEffect::Infinite);

    // 3. 物理定时器
    m_physicsTimer = new QTimer(this);
    m_physicsTimer->setInterval(1000 / GAME_FPS);
    connect(m_physicsTimer, &QTimer::timeout, this, &SpaceGame::onGameTick);
}

SpaceGame::~SpaceGame() {
    qDeleteAll(m_entities);
    m_entities.clear();
}

void SpaceGame::initGame() {
    m_state = GameState::Ready;
    m_score = 0;
    qDeleteAll(m_entities);
    m_entities.clear();

    m_playerPos = QPointF(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);
    
    m_spawnTimer = 0;
    m_spawnInterval = 60; // 初始约1秒生成一个
    m_difficultyTimer = 0;

    emit scoreChanged(0);
}

void SpaceGame::startGame() {
    if (m_state == GameState::Ready || m_state == GameState::GameOver) {
        initGame();
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

void SpaceGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
        m_physicsTimer->stop();
        m_bgMusic->stop();
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        m_physicsTimer->start();
        m_bgMusic->play();
    }
}

void SpaceGame::stopGame() {
    m_state = GameState::GameOver;
    m_physicsTimer->stop();
    m_bgMusic->stop();
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