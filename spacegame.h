#ifndef SPACEGAME_H
#define SPACEGAME_H

#include "gamebase.h"
#include "spacegamesettings.h"
#include "imagebutton.h"
#include "spacenamedialog.h" 
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
    double initialX;
    QString letter;       // 敌机显示的字母
    QString targetLetter; // 【新增】子弹追踪的目标字母
    int lifeTime;
    bool active;

    SpaceEntity(EntityType t, QPointF p, QPointF v, QString l = "")
        : type(t), pos(p), velocity(v), initialX(p.x()), letter(l), targetLetter(""), lifeTime(0), active(true) {
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

    void onBtnStartClicked();
    void onBtnReturnClicked();
    void onBtnOptionClicked();
    void onBtnHiscoreClicked();
    void onBtnExitClicked();
    void onBtnGamePauseClicked();

private:
    void spawnEnemy();
    // 【修改】生成子弹时传入目标字母
    void spawnBullet(const QPointF& startPos, const QString& targetLetter);
    void createExplosion(const QPointF& pos);

    // 【修改】返回 bool，true 表示游戏结束
    bool checkCollisions();

    void drawHUD(QPainter& painter);
    void handleGameOver();
    void saveScore(const QString& name, int score);

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

    QPixmap m_hudLabelScore;
    QPixmap m_hudLabelLife;
    QPixmap m_hudLabelTime;
    QPixmap m_hudLifeIcon;

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

    int m_spawnTimer;
    int m_spawnInterval;
    int m_lives;

    int m_gameTimeFrames;
    int m_difficultyLevel;
    double m_playerDir;

    QTimer* m_physicsTimer;
};

#endif // SPACEGAME_H