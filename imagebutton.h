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
    // ���캯������������״̬ͼ
    explicit ImageButton(const QString& normalPath,
        const QString& hoverPath,
        const QString& pressedPath,
        QWidget* parent = nullptr);

    // ���ù̶���СΪͼƬ��С
    void setFixedSizeToPixmap();

signals:
    void clicked();

protected:
    // ��д�¼���ʵ���Զ�����ƺ�״̬�л�
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override; // ������
    void leaveEvent(QEvent* event) override; // ����뿪

private:
    enum ButtonState {
        StateNormal,
        StateHover,
        StatePressed
    };

    // ��Ա����
    ButtonState currentState;
    QPixmap normalPixmap;
    QPixmap hoverPixmap;
    QPixmap pressedPixmap;
    bool isPressed;
};

#endif // IMAGEBUTTON_H
