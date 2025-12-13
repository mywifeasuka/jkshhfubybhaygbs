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

    // 更新设置
    void updateSettings(const FrogSettingsData& settings);

private slots:
    void onGameTick();

private:
    void spawnLeaves();
    void checkFrogStatus();
    void resetFrog();
    void checkInput();

    // 加载词库文件
    void loadDictionary(const QString& filename);

    // 资源 
    QPixmap m_bgPixmap;
    QPixmap m_frogPixmap;
    QPixmap m_leafPixmap;

    // 音效
    QSoundEffect* m_jumpSound;
    QSoundEffect* m_bgMusic;
    QSoundEffect* m_splashSound;
    QSoundEffect* m_successSound;

    //游戏数据 
    QList<LotusLeaf*> m_leaves;
    QStringList m_wordList; // 当前词库

    int m_frogCount;
    int m_currentRow;
    LotusLeaf* m_currentLeaf;
    QPointF m_frogPos;

    QString m_goalWord;
    QString m_inputBuffer;

    QTimer* m_physicsTimer;

    // 当前设置
    FrogSettingsData m_settings;
};

#endif // FROGGAME_H