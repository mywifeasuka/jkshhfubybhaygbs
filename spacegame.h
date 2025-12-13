#ifndef SPACEGAME_H
#define SPACEGAME_H

#include "gamebase.h"
#include "spacegamesettings.h"
#include "imagebutton.h"
#include "spacenamedialog.h" // 【新增】
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

enum EntityType {
    Type_Enemy,
    Type_Bullet,
    Type_Explosion
};

struct SpaceEntity {
    EntityType type;
    QPointF pos;
    QPointF velocity;
    double initialX;   // 【新增】用于S型移动的基准X坐标
    QString letter;
    int lifeTime;
    bool active;

    SpaceEntity(EntityType t, QPointF p, QPointF v, QString l = "")
        : type(t), pos(p), velocity(v), initialX(p.x()), letter(l), lifeTime(0), active(true) {
    }
};

class SpaceGame : public GameBase {
    Q_OBJECT
public:
    explicit SpaceGame(QObject* parent = nullptr);
    ~SpaceGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter& painter) override;
    void handleKeyPress(QKeyEvent* event) override;

    void resumeGame();

signals:
    void requestReturnToMenu();

private slots:
    void onGameTick();

    // 菜单按钮槽
    void onBtnStartClicked();
    void onBtnReturnClicked();
    void onBtnOptionClicked();
    void onBtnHiscoreClicked();
    void onBtnExitClicked();
    void onBtnGamePauseClicked();

private:
    void spawnEnemy();
    void spawnBullet(const QPointF& targetPos);
    void createExplosion(const QPointF& pos);
    void checkCollisions(); // 【新增】碰撞检测
    void drawHUD(QPainter& painter); // 【新增】绘制顶部条
    void handleGameOver(); // 【新增】游戏结束处理
    void saveScore(const QString& name, int score); // 【新增】保存成绩

    void setupInternalUI();
    void showMenuUI(bool isPauseMode);
    void hideMenuUI();
    void showGameUI();
    void hideGameUI();

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_menuBgPixmap;
    QPixmap m_playerPixmap;
    QPixmap m_enemyPixmap;
    QPixmap m_meteorPixmap;
    QPixmap m_bulletPixmap;
    QPixmap m_explosionPixmap;

    // 【新增】HUD 资源
    QPixmap m_hudLabelScore;
    QPixmap m_hudLabelLife;
    QPixmap m_hudLabelTime;
    QPixmap m_hudLifeIcon; // 生命条图标

    ImageButton* m_btnStart;
    ImageButton* m_btnReturn;
    ImageButton* m_btnOption;
    ImageButton* m_btnHiscore;
    ImageButton* m_btnExit;
    ImageButton* m_btnGamePause;

    SpaceGameSettings* m_settingsDialog;
    SpaceSettingsData m_settings;

    QSoundEffect* m_shootSound;
    QSoundEffect* m_explodeSound;
    QSoundEffect* m_bgMusic;

    QList<SpaceEntity*> m_entities;
    QPointF m_playerPos;

    // 计时器与难度
    int m_spawnTimer;
    int m_spawnInterval;
    int m_lives;

    // 【新增】
    int m_gameTimeFrames; // 倒计时 (帧)
    int m_difficultyLevel; // 当前难度等级
    double m_playerDir;   // 玩家移动方向 (1.0 或 -1.0)

    QTimer* m_physicsTimer;
};

#endif // SPACEGAME_H