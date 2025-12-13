#ifndef APPLEGAME_H
#define APPLEGAME_H

#include "gamebase.h"
#include "applegamesettings.h" // 引用新的设置头文件
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

// Apple 结构体保持不变...
struct Apple {
    QPointF pos;
    double speed;
    QString letter;
    bool active;
    Apple(QPointF p, double s, QString l) : pos(p), speed(s), letter(l), active(true) {}
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

    // 【新增】更新设置的接口
    void updateSettings(const AppleSettingsData& settings);

private slots:
    void onGameTick();

private:
    void spawnApple();
    void updateApples();

    // 资源
    QPixmap m_bgPixmap;
    QPixmap m_applePixmap;
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
    int m_caughtCount; // 【新增】已接住的数量，用于判断过关

    // 【新增】设置数据
    AppleSettingsData m_settings;
};

#endif // APPLEGAME_H