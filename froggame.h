#ifndef FROGGAME_H
#define FROGGAME_H

#include "gamebase.h"
#include "froggamesettings.h"
#include <QPixmap>
#include <QList>
#include <QPointF>
#include <QtMultimedia/QSoundEffect>

// LotusLeaf 结构体保持不变
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
    void onAnimTick(); // 【新增】动画定时器槽

private:
    void spawnLeaves();
    void resetFrog();
    void checkInput();
    void loadDictionary(const QString& filename);

    // --- 资源 ---
    QPixmap m_bgPixmap;
    QPixmap m_leafPixmap;

    // 【修改】青蛙贴图组
    QPixmap m_frogBack1;  // 背面-正常
    QPixmap m_frogBack2;  // 背面-鼓气
    QPixmap m_frogFront1; // 正面-正常 (对岸)
    QPixmap m_frogFront2; // 正面-鼓气 (对岸)

    // --- 音效 ---
    QSoundEffect* m_jumpSound;
    QSoundEffect* m_bgMusic;
    QSoundEffect* m_splashSound;
    QSoundEffect* m_successSound;

    // --- 游戏数据 ---
    QList<LotusLeaf*> m_leaves;
    QStringList m_wordList;

    int m_frogCount;     // 剩余青蛙 (岸边等待的)
    int m_successCount;  // 【新增】成功到达对岸的青蛙

    int m_currentRow;
    LotusLeaf* m_currentLeaf;
    QPointF m_frogPos;

    QString m_goalWord;
    QString m_inputBuffer;

    QTimer* m_physicsTimer;
    QTimer* m_animTimer; // 【新增】控制呼吸动画
    bool m_isCroaking;   // 【新增】当前是否处于鼓气状态

    FrogSettingsData m_settings;
};

#endif // FROGGAME_H