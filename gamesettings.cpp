#include "gamesettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout> 

void GameSettings::resetToDefaults() {
    onDefaultButtonClicked();
}

GameSettings::GameSettings(QWidget *parent) : QDialog(parent) {
    setWindowTitle("功能设置");
    setupUI();

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(defaultButton, &QPushButton::clicked, this, &GameSettings::onDefaultButtonClicked);

    connect(gameTimeSlider, &QSlider::valueChanged, this, &GameSettings::onGameTimeSliderChanged);
    connect(spawnIntervalSlider, &QSlider::valueChanged, this, &GameSettings::onSpawnIntervalSliderChanged);
    connect(stayTimeSlider, &QSlider::valueChanged, this, &GameSettings::onStayTimeSliderChanged);
}

void GameSettings::setupUI() {
    QFormLayout *formLayout = new QFormLayout();
     
    gameTimeSlider = new QSlider(Qt::Horizontal);
    gameTimeSlider->setRange(10, 300); 
    gameTimeLabel = new QLabel();
    formLayout->addRow("游戏时间 (秒):", gameTimeSlider);
    formLayout->addRow("", gameTimeLabel); 

    spawnIntervalSlider = new QSlider(Qt::Horizontal);
    spawnIntervalSlider->setRange(300, 2000); 
    spawnIntervalLabel = new QLabel();
    formLayout->addRow("鼹鼠出现间隔 (毫秒):", spawnIntervalSlider);
    formLayout->addRow("", spawnIntervalLabel);

    stayTimeSlider = new QSlider(Qt::Horizontal);
    stayTimeSlider->setRange(500, 5000); 
    stayTimeLabel = new QLabel();
    formLayout->addRow("停留时间 (毫秒):", stayTimeSlider);
    formLayout->addRow("", stayTimeLabel);

    okButton = new QPushButton("确定");
    cancelButton = new QPushButton("取消");
    defaultButton = new QPushButton("默认");
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(defaultButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void GameSettings::setSettings(const GameSettingsData &settings) {
    currentSettings = settings;
    gameTimeSlider->setValue(settings.gameTimeSec);
    spawnIntervalSlider->setValue(settings.spawnIntervalMs);
    stayTimeSlider->setValue(settings.stayTimeMs);

    onGameTimeSliderChanged(settings.gameTimeSec);
    onSpawnIntervalSliderChanged(settings.spawnIntervalMs);
    onStayTimeSliderChanged(settings.stayTimeMs);
}

GameSettingsData GameSettings::getSettings() const {
    GameSettingsData newSettings;
    newSettings.gameTimeSec = gameTimeSlider->value();
    newSettings.spawnIntervalMs = spawnIntervalSlider->value();
    newSettings.stayTimeMs = stayTimeSlider->value();
    return newSettings;
}

void GameSettings::onGameTimeSliderChanged(int value) {
    gameTimeLabel->setText(QString("%1 秒").arg(value));
}

void GameSettings::onSpawnIntervalSliderChanged(int value) {
    spawnIntervalLabel->setText(QString("%1 毫秒").arg(value));
}

void GameSettings::onStayTimeSliderChanged(int value) {
    stayTimeLabel->setText(QString("%1 毫秒").arg(value));
}

void GameSettings::onDefaultButtonClicked() {
    GameSettingsData defaultSettings;
    setSettings(defaultSettings);
}