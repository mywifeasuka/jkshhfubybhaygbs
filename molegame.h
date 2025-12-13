#ifndef MOLEGAME_H
#define MOLEGAME_H

#include "gamebase.h"
#include "mole.h"
#include "gamesettings.h"
#include <QVector>
#include <QLabel>
#include <QTimer>
#include <QtMultimedia/QSoundEffect>

class MoleGame : public GameBase {
    Q_OBJECT

public:
    explicit MoleGame(QObject* parent = nullptr);
    ~MoleGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter& painter) override;
    void handleKeyPress(QKeyEvent* event) override;

    void updateSettings(const GameSettingsData& data);
    void increaseDifficulty();

private slots:
    void onGameTimerTick();
    void onSpawnTimerTick();
    void onMoleHit();
    void onMoleEscaped();

private:
    void maintainMoleCount(); // 维持场上地鼠数量
    void checkGameOver();

    QPixmap m_backgroundPixmap;
    QPixmap m_carrotPixmap;
    QVector<Mole*> m_moles;

    QSoundEffect* m_hitSound;
    QSoundEffect* m_missSound;
    QSoundEffect* m_backgroundMusic;

    QTimer* m_gameTimer;
    QTimer* m_spawnTimer;

    GameSettingsData m_settings;
    int m_lives;
    int m_remainingTimeSec;
    int m_hitCount;
    int m_totalSpawns;
};

#endif // MOLEGAME_H