#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include "imagebutton.h" 

struct GameSettingsData {
    int gameTimeSec = 60;
    int spawnIntervalMs = 1000;
    int stayTimeMs = 5000;
};

class GameSettings : public QDialog {
    Q_OBJECT

public:
    explicit GameSettings(QWidget* parent = nullptr);

    void resetToDefaults();
    void setSettings(const GameSettingsData& settings);
    GameSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
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