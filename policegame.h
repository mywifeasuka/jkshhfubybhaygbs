#ifndef POLICEGAME_H
#define POLICEGAME_H

#include "gamebase.h"
#include "policegamesettings.h" // 引入设置
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QPointF>

class PoliceGame : public GameBase {
    Q_OBJECT
public:
    explicit PoliceGame(QObject* parent = nullptr);
    ~PoliceGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter& painter) override;
    void handleKeyPress(QKeyEvent* event) override;

    // 更新设置接口
    void updateSettings(const PoliceSettingsData& settings);

private slots:
    void onGameTick();

private:
    void initMapPath();
    void loadResources();
    void loadArticle();

    // 计算车辆在赛道上的位置和朝向
    void getCarState(double distance, const QVector<QPixmap>& sprites,
        QPointF& outPos, QPixmap& outSprite);

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_uiInputBg;     // 输入框背景
    QPixmap m_uiProgressBar; // 进度条滑块

    // 精灵图组
    QVector<QPixmap> m_policeSprites;
    QVector<QPixmap> m_thiefSprites;

    // --- 游戏数据 ---
    QString m_targetText;
    int m_currentIndex;
    bool m_isTypingError;

    // --- 物理数据 ---
    double m_totalMapLength;
    double m_playerDistance; // 玩家跑过的距离
    double m_enemyDistance;  // 敌人跑过的距离
    double m_playerSpeed;    // 当前瞬时速度（受打字影响）
    double m_playerBaseSpeed;// 基础速度（受载具影响）
    double m_enemySpeed;     // 敌人恒定速度（受难度影响）

    QVector<QPointF> m_pathPoints;
    QTimer* m_physicsTimer;

    PoliceSettingsData m_settings; // 当前设置
};

#endif // POLICEGAME_H