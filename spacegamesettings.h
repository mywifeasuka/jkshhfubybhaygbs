#ifndef SPACEGAMESETTINGS_H
#define SPACEGAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QCheckBox> // 【新增】
#include <QMouseEvent>
#include "imagebutton.h"

struct SpaceSettingsData {
    int difficulty = 1; // 难度 1-10
    int lives = 3;      // 初始生命 1-5
    bool bonusMode = false; // 【新增】奖励模式开关
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
    QCheckBox* m_checkBonus; // 【新增】奖励模式复选框

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