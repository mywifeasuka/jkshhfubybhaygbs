#ifndef POLICEGAME_H
#define POLICEGAME_H

#include "gamebase.h"
#include "policegamesettings.h"
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

    void updateSettings(const PoliceSettingsData& settings);

private slots:
    void onGameTick();

private:
    void initMapPath();
    void loadResources();
    void loadArticle(const QString& filename = "");

    // 增加 direction 参数
    void getCarState(double distance, int direction, const QVector<QPixmap>& sprites,
        QPointF& outPos, QPixmap& outSprite);

    QPixmap m_bgPixmap;
    QPixmap m_uiInputBg;
    QPixmap m_uiProgressBar;

    QVector<QPixmap> m_policeSprites;
    QVector<QPixmap> m_thiefSprites;

    QString m_targetText;
    int m_currentIndex;
    bool m_isTypingError;

    double m_totalMapLength;
    double m_playerDistance;
    double m_enemyDistance;
    double m_playerSpeed;
    double m_playerBaseSpeed;
    double m_enemySpeed;

    // 【新增】行进方向：1 为正向，-1 为反向
    int m_direction;

    QVector<QPointF> m_pathPoints;
    QTimer* m_physicsTimer;
    PoliceSettingsData m_settings;
};

#endif // POLICEGAME_H