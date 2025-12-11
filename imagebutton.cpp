#include "imagebutton.h"

ImageButton::ImageButton(const QString& normalPath,
    const QString& hoverPath,
    const QString& pressedPath,
    QWidget* parent)
    : QWidget(parent),
    currentState(StateNormal),
    isPressed(false) {

    normalPixmap.load(normalPath);
    hoverPixmap.load(hoverPath);
    pressedPixmap.load(pressedPath);

    setFixedSizeToPixmap();
    setCursor(Qt::PointingHandCursor);
}

void ImageButton::setFixedSizeToPixmap() {
    if (!normalPixmap.isNull()) {
        setFixedSize(normalPixmap.size());
    }
}

void ImageButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    switch (currentState) {
    case StateHover:
        painter.drawPixmap(rect(), hoverPixmap);
        break;
    case StatePressed:
        painter.drawPixmap(rect(), pressedPixmap);
        break;
    case StateNormal:
    default:
        painter.drawPixmap(rect(), normalPixmap);
        break;
    }
}

void ImageButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
        currentState = StatePressed;
        update(); // �����ػ�
    }
}

void ImageButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isPressed) {
        isPressed = false;
        // �������Ƿ��ڰ�ť��
        if (rect().contains(event->pos())) {
            currentState = StateHover;
            emit clicked(); // ���͵���ź�
        }
        else {
            currentState = StateNormal;
        }
        update();
    }
}

void ImageButton::enterEvent(QEvent* event) {
    currentState = StateHover;
    update();
}

void ImageButton::leaveEvent(QEvent* event) {
    currentState = StateNormal;
    update();
}