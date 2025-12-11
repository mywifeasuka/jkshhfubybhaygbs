#ifndef MOLE_H
#define MOLE_H

#include <QObject> // 改为继承 QObject
#include <QPixmap>
#include <QTimer>
#include <QPainter>
#include <QPoint>

class Mole : public QObject { // 不再是 QWidget
    Q_OBJECT

public:
    enum MoleState {
        Hidden,
        Visible,
        Hit,
        Escaping
    };

    explicit Mole(QObject* parent = nullptr);

    // 设置位置（替代原来的 move）
    void setPos(const QPoint& pos);
    QPoint getPos() const { return m_pos; }

    QString getLetter() const { return currentLetter; }
    bool isVisible() const { return currentState != Hidden; }

    // 核心绘制函数（替代 paintEvent）
    void draw(QPainter& painter);

    // 逻辑控制
    void showMole(const QString& letter, int stayTime);
    void hideMole();
    void hitByUser();
    void pause();
    void resume();

signals:
    void escaped();
    void hitSuccess();

private slots:
    void onVisualCountdown();
    void onStayTimerTimeout();
    void onAnimationTimerTimeout();

private:
    QPoint m_pos; // 记录地鼠坐标

    int remainingStayTimeMs;
    MoleState currentState;
    QString currentLetter;

    QPixmap normalPixmap;
    QPixmap hitPixmap;

    QTimer* stayTimer;
    QTimer* animationTimer;
    QTimer* visualCountdownTimer;
    int remainingDisplayTime;
};

#endif // MOLE_H