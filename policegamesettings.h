#ifndef POLICEGAMESETTINGS_H
#define POLICEGAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include "imagebutton.h"

// 定义设置数据结构
struct PoliceSettingsData {
    int difficulty = 3;     // 难度 1-10
    int role = 0;           // 0: 警察, 1: 小偷
    int vehicle = 0;        // 0: 汽车, 1: 自行车 (影响玩家基础速度)
};

class PoliceGameSettings : public QDialog {
    Q_OBJECT

public:
    explicit PoliceGameSettings(QWidget* parent = nullptr);

    void setSettings(const PoliceSettingsData& settings);
    PoliceSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    // 支持窗口拖拽
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onDefaultClicked();

private:
    void setupUI();

    // UI 组件
    QSlider* m_sliderDiff;
    QLabel* m_labelDiff;

    // 角色与载具选择组
    QButtonGroup* m_groupRole;
    QButtonGroup* m_groupVehicle;

    // 按钮
    ImageButton* m_btnOk;
    ImageButton* m_btnCancel;
    ImageButton* m_btnDefault;

    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;
};

#endif // POLICEGAMESETTINGS_H