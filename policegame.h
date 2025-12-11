#ifndef POLICEGAME_H
#define POLICEGAME_H

#include "gamebase.h"
#include <QVector>
#include <QStringList>

class PoliceGame : public GameBase {
    Q_OBJECT
public:
    explicit PoliceGame(QObject *parent = nullptr);
    ~PoliceGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter &painter) override;
    void handleKeyPress(QKeyEvent *event) override;

private slots:
    void onGameTick(); // 游戏主循环逻辑

private:
    void loadArticle();        // 加载文章
    void checkInput(QChar inputChar); // 检查输入
    void updatePositions();    // 更新角色位置

    // 资源
    QPixmap m_bgPixmap;        // 地图背景（长图）
    QPixmap m_policePixmap;    // 警察贴图
    QPixmap m_thiefPixmap;     // 小偷贴图

    // 游戏逻辑数据
    QString m_targetText;      // 当前需要输入的完整文章
    int m_currentIndex;        // 当前输入到的字符索引
    QString m_typedText;       // 已经输入正确的文本（用于高亮）
    
    // 物理/位置数据
    double m_progress;         // 玩家进度 (0.0 - 1.0)
    double m_enemyProgress;    // 敌人(电脑)进度
    double m_mapScrollX;       // 地图滚动位置
    double m_playerSpeed;      // 玩家速度（根据打字速度变化）
    
    QTimer *m_timer;           // 游戏主定时器
};

#endif // POLICEGAME_H