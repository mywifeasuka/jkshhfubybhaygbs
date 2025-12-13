#ifndef POLICEGAME_H
#define POLICEGAME_H

#include "gamebase.h"
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

private slots:
    void onGameTick();

private:
    void initMapPath();   
    void loadResources(); // 加载多方向贴图
    void loadArticle();//加载文章

    // 获取当前位置、角度，并返回对应的贴图引用
    void getCarState(double distance, const QVector<QPixmap>& sprites,
        QPointF& outPos, QPixmap& outSprite);

    // --- 资源 ---
    QPixmap m_bgPixmap;

    // 存储4个方向的贴图：0:左上, 1:左下, 2:右上, 3:右下
    QVector<QPixmap> m_policeSprites;
    QVector<QPixmap> m_thiefSprites;

    // --- 游戏数据 ---
    QString m_targetText;
    int m_currentIndex;
    bool m_isTypingError;

    // --- 物理数据 ---
    double m_totalMapLength;
    double m_playerDistance;
    double m_enemyDistance;
    double m_playerSpeed;
    double m_enemySpeed;

    QVector<QPointF> m_pathPoints;
    QTimer* m_physicsTimer;
};

#endif // POLICEGAME_H