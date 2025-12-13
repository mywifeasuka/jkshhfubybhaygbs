#ifndef FROGGAME_H
#define FROGGAME_H

#include "gamebase.h"
#include "froggamesettings.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

struct LotusLeaf {
    int id;
    int row;
    double x;
    double speed;
    QString word;

    LotusLeaf(int r, double startX, double s, QString w)
        : row(r), x(startX), speed(s), word(w) {
    }
};

class FrogGame : public GameBase {
    Q_OBJECT
public:
    explicit FrogGame(QObject* parent = nullptr);
    ~FrogGame();

    void initGame() override;
    void startGame() override;
    void pauseGame() override;
    void stopGame() override;
    void draw(QPainter& painter) override;
    void handleKeyPress(QKeyEvent* event) override;
    void updateSettings(const FrogSettingsData& settings);

private slots:
    void onGameTick();
    void onAnimTick();

private:
    void spawnLeaves();
    void resetFrog();           // 重置青蛙位置（准备下一只）
    void retreatFrog();
    void checkInput(const QString& key); // 接收字符进行判定
    void loadDictionary(const QString& filename);

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_leafPixmap;

    QPixmap m_frogBack1;
    QPixmap m_frogBack2;
    QPixmap m_frogFront1;
    QPixmap m_frogFront2;

    // --- 音效 ---
    QSoundEffect* m_jumpSound;
    QSoundEffect* m_bgMusic;
    QSoundEffect* m_splashSound;
    QSoundEffect* m_successSound;

    // --- 游戏数据 ---
    QList<LotusLeaf*> m_leaves;
    QStringList m_wordList;

    int m_frogsRemaining; // 剩余待出场的青蛙总数 (初始5)
    int m_successCount;   // 成功到达对岸的数量

    int m_currentRow;
    LotusLeaf* m_currentLeaf;
    QPointF m_frogPos;

    QString m_goalWord;
    QString m_inputBuffer;

    // 输入锁定机制
    LotusLeaf* m_lockedLeaf; // 当前锁定的荷叶
    bool m_isGoalLocked;     // 当前是否锁定了终点单词

    QTimer* m_physicsTimer;
    QTimer* m_animTimer;
    bool m_isCroaking;

    FrogSettingsData m_settings;
};

#endif // FROGGAME_H