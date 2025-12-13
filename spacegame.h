#ifndef SPACEGAME_H
#define SPACEGAME_H

#include "gamebase.h"
#include "spacegamesettings.h"
#include "imagebutton.h"
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
    QString letter;
    int lifeTime;
    bool active;

    SpaceEntity(EntityType t, QPointF p, QPointF v, QString l = "")
        : type(t), pos(p), velocity(v), letter(l), lifeTime(0), active(true) {
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

    // 继续游戏 (从菜单返回)
    void resumeGame();

signals:
    void requestReturnToMenu(); // 请求返回主程序菜单

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

    // 辅助：初始化内部UI
    void setupInternalUI();
    void showMenuUI(bool isPauseMode); // isPauseMode=true显示返回键，false显示开始键
    void hideMenuUI();

    void showGameUI();
    void hideGameUI();

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_menuBgPixmap; // 【新增】菜单背景
    QPixmap m_playerPixmap;
    QPixmap m_enemyPixmap;
    QPixmap m_meteorPixmap;
    QPixmap m_bulletPixmap;
    QPixmap m_explosionPixmap;

    // --- 内部 UI 按钮 ---
    ImageButton* m_btnStart;
    ImageButton* m_btnReturn; // 暂停后显示的“返回”
    ImageButton* m_btnOption;
    ImageButton* m_btnHiscore;
    ImageButton* m_btnExit;

    ImageButton* m_btnGamePause;

    SpaceGameSettings* m_settingsDialog;
    SpaceSettingsData m_settings;

    // --- 音效 ---
    QSoundEffect* m_shootSound;
    QSoundEffect* m_explodeSound;
    QSoundEffect* m_bgMusic;

    // --- 游戏数据 ---
    QList<SpaceEntity*> m_entities;
    QPointF m_playerPos;
    int m_spawnTimer;
    int m_spawnInterval;
    int m_difficultyTimer;
    int m_lives; // 生命值

    QTimer* m_physicsTimer;
};

#endif // SPACEGAME_H