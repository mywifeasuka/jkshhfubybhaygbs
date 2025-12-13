#ifndef APPLEGAMESETTINGS_H
#define APPLEGAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QMouseEvent>
#include "imagebutton.h"

// 苹果游戏的设置数据结构
struct AppleSettingsData {
    int level = 3;          // 游戏等级 (1-10)
    int targetCount = 100;  // 过关苹果数量 (10-200)
    int failCount = 10;     // 失败苹果数量/生命值 (1-20)
};

class AppleGameSettings : public QDialog {
    Q_OBJECT

public:
    explicit AppleGameSettings(QWidget *parent = nullptr);
    
    void setSettings(const AppleSettingsData &settings);
    AppleSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onLevelChanged(int value);
    void onTargetChanged(int value);
    void onFailChanged(int value);
    void onDefaultClicked();

private:
    void setupUI();
    void setupSliderStyle(QSlider* slider); 

    // UI 组件
    QSlider* m_sliderLevel;
    QSlider* m_sliderTarget;
    QSlider* m_sliderFail;

    QLabel* m_labelLevel;
    QLabel* m_labelTarget;
    QLabel* m_labelFail;

    ImageButton* m_btnOk;
    ImageButton* m_btnCancel;
    ImageButton* m_btnDefault;

    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;
};

#endif // APPLEGAMESETTINGS_H