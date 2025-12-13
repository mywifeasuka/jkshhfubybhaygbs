#pragma once
#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>

class ImageButton : public QWidget {
    Q_OBJECT

public:
    explicit ImageButton(const QString& normalPath,
        const QString& hoverPath,
        const QString& pressedPath,
        QWidget* parent = nullptr);

    void loadImages(const QString& normalPath,
        const QString& hoverPath,
        const QString& pressedPath);

    void setFixedSizeToPixmap();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override; 
    void leaveEvent(QEvent* event) override; 

private:
    enum ButtonState {
        StateNormal,
        StateHover,
        StatePressed
    };

    ButtonState currentState;
    QPixmap normalPixmap;
    QPixmap hoverPixmap;
    QPixmap pressedPixmap;
    bool isPressed;
};

#endif // IMAGEBUTTON_H
