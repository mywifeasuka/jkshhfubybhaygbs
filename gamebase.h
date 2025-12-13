#ifndef GAMEBASE_H
#define GAMEBASE_H

#include <QObject>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>

// 定义游戏状态
enum class GameState {
    Ready,
    Playing,
    Paused,
    GameOver,
    Victory
};

class GameBase : public QObject {
    Q_OBJECT
public:
    explicit GameBase(QObject *parent = nullptr) : QObject(parent), m_state(GameState::Ready) {}
    virtual ~GameBase() {}

    virtual void initGame() = 0;              // 初始化/重置游戏
    virtual void startGame() = 0;             // 开始
    virtual void pauseGame() = 0;             // 暂停/继续
    virtual void stopGame() = 0;              // 结束
    virtual void draw(QPainter &painter) = 0; // 绘制游戏画面
    virtual void handleKeyPress(QKeyEvent *event) = 0; // 处理按键

    // 通用状态获取
    GameState getState() const { return m_state; }

protected:
    GameState m_state;
    int m_score = 0;

signals:
    void gameFinished(int score, bool win); // 游戏结束信号
    void scoreChanged(int newScore);        // 分数变化信号
};

#endif // GAMEBASE_H