#ifndef FROGGAME_H
#define FROGGAME_H

#include "gamebase.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

// 荷叶实体
struct LotusLeaf {
    int id;
    int row;         // 所在的行 (0, 1, 2)
    double x;        // X轴位置
    double speed;    // 移动速度
    QString word;    // 携带单词

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

private slots:
    void onGameTick();

private:
    void spawnLeaves();
    void checkFrogStatus();
    void resetFrog();
    void checkInput();

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_frogPixmap;
    QPixmap m_leafPixmap;
    // Removed m_bankPixmap (背景图已包含)

    // --- 音效 ---
    QSoundEffect* m_jumpSound;
    QSoundEffect* m_bgMusic;
    QSoundEffect* m_splashSound;
    QSoundEffect* m_successSound;

    // --- 游戏数据 ---
    QList<LotusLeaf*> m_leaves;
    QStringList m_wordList;

    int m_frogCount;
    int m_currentRow;
    LotusLeaf* m_currentLeaf;
    QPointF m_frogPos;

    QString m_goalWord;
    QString m_inputBuffer;

    QTimer* m_physicsTimer;
};

#endif // FROGGAME_H