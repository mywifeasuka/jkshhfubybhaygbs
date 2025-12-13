#include "mole.h"
#include <QDebug>

Mole::Mole(QObject* parent)
    : QObject(parent),
    currentState(Hidden) {

    // 1. 加载资源
    normalPixmap.load(":/img/mole_normal.bmp");
    hitPixmap.load(":/img/mole_hit.bmp");

    // 尝试加载两帧逃跑动画，如果资源里没有专门的1/2，则用通用的代替
    escapePixmap1.load(":/img/mole_hide_1.bmp");
    if (escapePixmap1.isNull()) escapePixmap1.load(":/img/mole_hide.bmp");

    escapePixmap2.load(":/img/mole_hide_2.bmp");
    if (escapePixmap2.isNull()) escapePixmap2.load(":/img/mole_hide.bmp");

    // 2. 初始化音效
    escapeSound = new QSoundEffect(this);
    escapeSound->setSource(QUrl::fromLocalFile(":/snd/mouse_away.wav"));

    // 3. 初始化计时器
    stayTimer = new QTimer(this);
    stayTimer->setSingleShot(true);
    connect(stayTimer, &QTimer::timeout, this, &Mole::onStayTimerTimeout);

    animationTimer = new QTimer(this);
    animationTimer->setSingleShot(true);
    // 注意：Hit和Escape复用这个Timer，通过 connect 动态绑定或状态判断处理
    connect(animationTimer, &QTimer::timeout, this, &Mole::onEscapeAnimation);

    visualCountdownTimer = new QTimer(this);
    visualCountdownTimer->setInterval(1000);
    connect(visualCountdownTimer, &QTimer::timeout, this, &Mole::onVisualCountdown);
}

void Mole::setPos(const QPoint& pos) {
    m_pos = pos;
}

void Mole::draw(QPainter& painter) {
    if (currentState == Hidden) return;

    if (currentState == Visible) {
        painter.drawPixmap(m_pos, normalPixmap);

        QRect letterRect(m_pos.x() + 70, m_pos.y() + 20, 40, 30);
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.setPen(Qt::black);
        painter.drawText(letterRect, Qt::AlignCenter, currentLetter);

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
    else if (currentState == Escaping_1) {
        painter.drawPixmap(m_pos, escapePixmap1);
    }
    else if (currentState == Escaping_2) {
        painter.drawPixmap(m_pos, escapePixmap2);
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
}

void Mole::hitByUser() {
    if (currentState != Visible) return;

    stayTimer->stop();
    visualCountdownTimer->stop();

    currentState = Hit;

    // 切换 Timer 连接到 Hit 逻辑
    animationTimer->disconnect(this);
    connect(animationTimer, &QTimer::timeout, this, &Mole::onHitAnimationFinished);
    animationTimer->start(500);

    emit hitSuccess(); // 抛出信号通知 GameWidget 补充新地鼠
}

void Mole::onHitAnimationFinished() {
    hideMole();
}

void Mole::onStayTimerTimeout() {
    if (currentState != Visible) return;

    // 时间到，开始逃跑
    currentState = Escaping_1;
    visualCountdownTimer->stop();
    escapeSound->play(); // 播放逃跑音效

    // 切换 Timer 连接到 Escape 逻辑
    animationTimer->disconnect(this);
    connect(animationTimer, &QTimer::timeout, this, &Mole::onEscapeAnimation);

    // 每帧显示 150ms
    animationTimer->start(150);

    emit escaped(); // 抛出信号通知 GameWidget 扣分并补充新地鼠
}

void Mole::onEscapeAnimation() {
    if (currentState == Escaping_1) {
        currentState = Escaping_2;
        animationTimer->start(150);
    }
    else if (currentState == Escaping_2) {
        hideMole();
    }
}

void Mole::hideMole() {
    currentState = Hidden;
    currentLetter.clear();
    stayTimer->stop();
    animationTimer->stop();
    visualCountdownTimer->stop();

    emit finished();
}

void Mole::pause() {
    if (currentState == Visible && stayTimer->isActive()) {
        remainingStayTimeMs = stayTimer->remainingTime();
        stayTimer->stop();
    }
    if (visualCountdownTimer->isActive()) {
        visualCountdownTimer->stop();
    }
    if (animationTimer->isActive()) {
        animationTimer->stop();
    }
}

void Mole::resume() {
    if (currentState == Visible && remainingStayTimeMs > 0) {
        stayTimer->start(remainingStayTimeMs);
        if (remainingDisplayTime > 0 && !visualCountdownTimer->isActive()) {
            visualCountdownTimer->start();
        }
    }
    // 如果正在播放动画，恢复播放
    if (currentState == Hit || currentState == Escaping_1 || currentState == Escaping_2) {
        animationTimer->start();
    }
}