#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include "imagebutton.h" // 引用你之前实现的 ImageButton

struct GameSettingsData {
    int gameTimeSec = 60;
    int spawnIntervalMs = 1000;
    int stayTimeMs = 3000;
};

class GameSettings : public QDialog {
    Q_OBJECT

public:
    explicit GameSettings(QWidget* parent = nullptr);

    void resetToDefaults();
    void setSettings(const GameSettingsData& settings);
    GameSettingsData getSettings() const;

protected:
    // 重写绘图事件以绘制背景
    void paintEvent(QPaintEvent* event) override;
    // 重写鼠标事件以支持拖拽
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onGameTimeSliderChanged(int value);
    void onSpawnIntervalSliderChanged(int value);
    void onStayTimeSliderChanged(int value);
    void onDefaultButtonClicked();

private:
    void setupUI();
    void setupSliderStyle(QSlider* slider); // 辅助函数：设置滑块样式

    // UI 组件
    QSlider* gameTimeSlider;
    QSlider* spawnIntervalSlider;
    QSlider* stayTimeSlider;

    QLabel* gameTimeLabel;
    QLabel* spawnIntervalLabel;
    QLabel* stayTimeLabel;

    // 改用 ImageButton
    ImageButton* okButton;
    ImageButton* cancelButton;
    ImageButton* defaultButton;

    GameSettingsData currentSettings;

    // 资源与拖拽逻辑
    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;
};

#endif // GAMESETTINGS_H