#include "mole.h"

Mole::Mole(QObject* parent)
    : QObject(parent), // 调用 QObject 构造
    currentState(Hidden) {

    // 加载资源
    normalPixmap.load(":/img/mole_normal.bmp");
    hitPixmap.load(":/img/mole_hit.bmp");

    // 初始化定时器
    stayTimer = new QTimer(this);
    stayTimer->setSingleShot(true);
    connect(stayTimer, &QTimer::timeout, this, &Mole::onStayTimerTimeout);

    animationTimer = new QTimer(this);
    animationTimer->setSingleShot(true);
    connect(animationTimer, &QTimer::timeout, this, &Mole::onAnimationTimerTimeout);

    visualCountdownTimer = new QTimer(this);
    visualCountdownTimer->setInterval(1000);
    connect(visualCountdownTimer, &QTimer::timeout, this, &Mole::onVisualCountdown);
}

void Mole::setPos(const QPoint& pos) {
    m_pos = pos;
}

// 核心绘制逻辑
void Mole::draw(QPainter& painter) {
    if (currentState == Hidden) return;

    if (currentState == Visible) {
        // 在指定位置绘制地鼠
        painter.drawPixmap(m_pos, normalPixmap);

        // 绘制字母 (相对坐标调整)
        QRect letterRect(m_pos.x() + 70, m_pos.y() + 20, 40, 30);
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.setPen(Qt::black);
        painter.drawText(letterRect, Qt::AlignCenter, currentLetter);

        // 绘制倒计时
        if (remainingDisplayTime > 0) {
            QRect countdownRect(m_pos.x() + 70, m_pos.y() + 110, 20, 20);
            painter.setFont(QFont("Arial", 14, QFont::Bold));
            painter.setPen(Qt::white);
            painter.drawText(countdownRect, Qt::AlignCenter, QString::number(remainingDisplayTime));
        }
    }
    else if (currentState == Hit) {
        painter.drawPixmap(m_pos, hitPixmap);
    }
}

void Mole::showMole(const QString& letter, int stayTime) {
    if (currentState != Hidden) return;

    currentState = Visible;
    currentLetter = letter;

    stayTimer->start(stayTime);
    remainingStayTimeMs = 0;

    remainingDisplayTime = stayTime / 1000;
    visualCountdownTimer->start();
}

void Mole::onVisualCountdown() {
    remainingDisplayTime--;
    if (remainingDisplayTime <= 0) {
        visualCountdownTimer->stop();
    }
    // 注意：不再调用 update()，因为 GameWidget 会负责重绘
}

void Mole::hitByUser() {
    if (currentState != Visible) return;

    stayTimer->stop();
    currentState = Hit;
    animationTimer->start(500);
    emit hitSuccess();
}

void Mole::onStayTimerTimeout() {
    if (currentState != Visible) return;

    currentState = Escaping;
    animationTimer->start(200);
    emit escaped();
}

void Mole::onAnimationTimerTimeout() {
    hideMole();
}

void Mole::hideMole() {
    currentState = Hidden;
    currentLetter.clear();
    stayTimer->stop();
    animationTimer->stop();
    visualCountdownTimer->stop();
}

void Mole::pause() {
    if (currentState == Visible && stayTimer->isActive()) {
        remainingStayTimeMs = stayTimer->remainingTime();
        stayTimer->stop();
    }
    if (visualCountdownTimer->isActive()) {
        visualCountdownTimer->stop();
    }
}

void Mole::resume() {
    if (currentState == Visible && remainingStayTimeMs > 0) {
        stayTimer->start(remainingStayTimeMs);
        if (remainingDisplayTime > 0 && !visualCountdownTimer->isActive()) {
            visualCountdownTimer->start();
        }
        remainingStayTimeMs = 0;
    }
}