#ifndef APPLEGAME_H
#define APPLEGAME_H

#include "gamebase.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

// 定义苹果实体
struct Apple {
    QPointF pos;       // 位置
    double speed;      // 下落速度 (匀速)
    QString letter;    // 携带的字母
    bool active;       // 是否存活

    Apple(QPointF p, double s, QString l) 
        : pos(p), speed(s), letter(l), active(true) {}
};

class AppleGame : public GameBase {
    Q_OBJECT
public:
    explicit AppleGame(QObject *parent = nullptr);
    ~AppleGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter &painter) override;
    void handleKeyPress(QKeyEvent *event) override;

private slots:
    void onGameTick(); // 游戏主循环

private:
    void spawnApple();     // 生成苹果
    void updateApples();   // 更新位置状态

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_applePixmap;
    QPixmap m_basketPixmap;

    // --- 音效 ---
    QSoundEffect* m_catchSound; // 接住音效
    QSoundEffect* m_bgMusic;    // 背景音乐

    // --- 游戏数据 ---
    QList<Apple*> m_apples;
    QPointF m_basketPos;    // 篮子位置
    
    int m_spawnTimer;       // 生成计时器
    int m_spawnInterval;    // 生成间隔 (随难度减小)
    double m_currentBaseSpeed; // 当前基础下落速度 (随难度增加)
    
    int m_lives;            // 生命值
    
    QTimer *m_physicsTimer;
};

#endif // APPLEGAME_H