#ifndef SPACEGAME_H
#define SPACEGAME_H

#include "gamebase.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

// 定义游戏中的物体类型
enum EntityType {
    Type_Enemy,
    Type_Bullet,
    Type_Explosion
};

struct SpaceEntity {
    EntityType type;
    QPointF pos;       // 位置
    QPointF velocity;  // 速度向量
    QString letter;    // 携带的字母（仅敌机有效）
    int lifeTime;      // 存活时间（仅爆炸有效）
    bool active;       // 是否存活
    
    // 构造函数
    SpaceEntity(EntityType t, QPointF p, QPointF v, QString l = "")
        : type(t), pos(p), velocity(v), letter(l), lifeTime(0), active(true) {}
};

class SpaceGame : public GameBase {
    Q_OBJECT
public:
    explicit SpaceGame(QObject *parent = nullptr);
    ~SpaceGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter &painter) override;
    void handleKeyPress(QKeyEvent *event) override;

private slots:
    void onGameTick(); // 物理刷新

private:
    void spawnEnemy(); // 生成敌机
    void spawnBullet(const QPointF& targetPos); // 发射子弹
    void createExplosion(const QPointF& pos);   // 生成爆炸

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_playerPixmap;
    QPixmap m_enemyPixmap;
    QPixmap m_meteorPixmap; // 陨石图片 [cite: 515]
    QPixmap m_bulletPixmap;
    QPixmap m_explosionPixmap;

    // --- 音效 ---
    QSoundEffect* m_shootSound;
    QSoundEffect* m_explodeSound;
    QSoundEffect* m_bgMusic;

    // --- 游戏数据 ---
    QList<SpaceEntity*> m_entities; // 所有实体集合
    QPointF m_playerPos;            // 玩家位置
    int m_spawnTimer;               // 生成计时器
    int m_spawnInterval;            // 生成间隔（随难度减少）
    int m_difficultyTimer;          // 难度增加计时
    
    QTimer *m_physicsTimer;
};

#endif // SPACEGAME_H