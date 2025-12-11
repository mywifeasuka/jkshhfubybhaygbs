#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

struct GameSettingsData {
    int gameTimeSec = 60;     // 游戏时间
    int spawnIntervalMs = 1000; // 出现间隔
    int stayTimeMs = 3000;      // 停留时间
};

class GameSettings : public QDialog {
    Q_OBJECT

public:
    void resetToDefaults();
    explicit GameSettings(QWidget *parent = nullptr);
    void setSettings(const GameSettingsData &settings);
    GameSettingsData getSettings() const;

private slots:
    // 当滑块值变化时，更新旁边的标签
    void onGameTimeSliderChanged(int value);
    void onSpawnIntervalSliderChanged(int value);
    void onStayTimeSliderChanged(int value);
    void onDefaultButtonClicked();

private:
    void setupUI();

    // 成员变量
    QSlider* gameTimeSlider;
    QSlider* spawnIntervalSlider;
    QSlider* stayTimeSlider;

    QLabel* gameTimeLabel;
    QLabel* spawnIntervalLabel;
    QLabel* stayTimeLabel;

    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* defaultButton;

    GameSettingsData currentSettings; 
};

#endif // GAMESETTINGS_H