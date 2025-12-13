#ifndef APPLEGAME_H
#define APPLEGAME_H

#include "gamebase.h"
#include "applegamesettings.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

struct Apple {
    QPointF pos;
    double speed;
    QString letter;
    bool active;

    // 落地状态
    bool isBad;      // 是否已经摔烂
    int removeTimer; // 摔烂后停留的帧数

    Apple(QPointF p, double s, QString l)
        : pos(p), speed(s), letter(l), active(true), isBad(false), removeTimer(0) {
    }
};

class AppleGame : public GameBase {
    Q_OBJECT
        
public:
    explicit AppleGame(QObject* parent = nullptr);
    ~AppleGame();
    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter& painter) override;
    void handleKeyPress(QKeyEvent* event) override;
    void updateSettings(const AppleSettingsData& settings);

private slots:
    void onGameTick();

private:
    void spawnApple();
    void updateApples();

    // 资源
    QPixmap m_bgPixmap;
    QPixmap m_applePixmap;
    QPixmap m_appleBadPixmap; // 烂苹果图片
    QPixmap m_basketPixmap;

    QSoundEffect* m_catchSound;
    QSoundEffect* m_bgMusic;
    QList<Apple*> m_apples;
    QPointF m_basketPos;
    QTimer* m_physicsTimer;
    int m_spawnTimer;
    int m_spawnInterval;
    double m_currentBaseSpeed;
    int m_lives;
    int m_caughtCount;
    AppleSettingsData m_settings;
};

#endif // APPLEGAME_H