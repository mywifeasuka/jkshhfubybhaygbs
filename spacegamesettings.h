#ifndef SPACEGAMESETTINGS_H
#define SPACEGAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include "imagebutton.h"

// 【新增】自定义图片复选框类
// 支持4种状态：
// 0: 未选中 (Normal)
// 1: 未选中悬停 (Hover) -> _1
// 2: 选中 (Checked) -> _2
// 3: 选中悬停 (Checked Hover) -> _3
class ImageCheckBox : public QWidget {
    Q_OBJECT
public:
    explicit ImageCheckBox(QWidget* parent = nullptr);

    // 加载四态图片
    // basePath 例如 ":/img/checkbox"
    // 会自动加载 basePath + ".bmp", basePath + "_1.bmp" 等
    void loadImages(const QString& basePath);

    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

signals:
    void stateChanged(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_checked;
    bool m_isHover;
    QPixmap m_pixmaps[4]; // 存储4张图
};

struct SpaceSettingsData {
    int difficulty = 1;
    int lives = 3;
    bool bonusMode = false;
};

class SpaceGameSettings : public QDialog {
    Q_OBJECT

public:
    explicit SpaceGameSettings(QWidget* parent = nullptr);
    void setSettings(const SpaceSettingsData& settings);
    SpaceSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onDiffChanged(int value);
    void onLivesChanged(int value);
    void onDefaultClicked();

private:
    void setupUI();
    void setupSliderStyle(QSlider* slider);

    QSlider* m_sliderDiff;
    QSlider* m_sliderLives;

    // 【修改】使用自定义控件
    ImageCheckBox* m_checkBonus;
    QLabel* m_labelBonusDesc; // 说明文字

    QLabel* m_labelDiff;
    QLabel* m_labelLives;

    ImageButton* m_btnOk;
    ImageButton* m_btnCancel;
    ImageButton* m_btnDefault;

    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;
};

#endif // SPACEGAMESETTINGS_H