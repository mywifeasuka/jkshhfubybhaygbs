#ifndef MOLE_H
#define MOLE_H

#include <QObject>
#include <QPixmap>
#include <QTimer>
#include <QPainter>
#include <QPoint>
#include <QtMultimedia/QSoundEffect>

class Mole : public QObject {
    Q_OBJECT

public:
    enum MoleState {
        Hidden,
        Visible,
        Hit,
        Escaping_1, // 逃跑动画帧1
        Escaping_2  // 逃跑动画帧2
    };

    explicit Mole(QObject* parent = nullptr);

    void setPos(const QPoint& pos);
    QPoint getPos() const { return m_pos; }

    QString getLetter() const { return currentLetter; }

    // 只有 Visible 状态才算"在场活跃"，被打中或逃跑中都不算
    bool isActive() const { return currentState == Visible; }
    // 只有 Hidden 状态才算"空闲"，可以生成新地鼠
    bool isFree() const { return currentState == Hidden; }

    void draw(QPainter& painter);

    void showMole(const QString& letter, int stayTime);
    void hideMole();
    void hitByUser();
    void pause();
    void resume();

signals:
    void escaped();    // 逃跑开始信号
    void hitSuccess(); // 被击中信号
    void finished();   // 动画结束变为空闲信号

private slots:
    void onVisualCountdown();
    void onStayTimerTimeout();
    void onEscapeAnimation(); // 逃跑动画循环
    void onHitAnimationFinished();

private:
    QPoint m_pos;
    int remainingStayTimeMs;
    MoleState currentState;
    QString currentLetter;

    // 资源
    QPixmap normalPixmap;
    QPixmap hitPixmap;
    QPixmap escapePixmap1; // 逃跑图1
    QPixmap escapePixmap2; // 逃跑图2

    // 音效
    QSoundEffect* escapeSound;

    // 计时器
    QTimer* stayTimer;
    QTimer* animationTimer;
    QTimer* visualCountdownTimer;
    int remainingDisplayTime;
};

#endif // MOLE_H