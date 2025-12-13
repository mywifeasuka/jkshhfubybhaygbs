#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "gamebase.h"
#include "gamesettings.h"
#include "imagebutton.h"

// 引入具体游戏类
#include "molegame.h"
#include "policegame.h"
#include "spacegame.h"
#include "applegame.h"
#include "froggame.h"

class GameWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // 菜单按钮槽
    void onSelectMoleGame();
    void onSelectPoliceGame();
    void onSelectSpaceGame();
    void onSelectAppleGame();
    void onSelectFrogGame();
    void onExitApp();
    
    // 游戏内按钮槽
    void onStartGame();
    void onPauseGame();
    void onReturnToMenu(); // 返回主菜单
    void onStopGameRound(); // 对应控制面板结束
    void onShowSettings();

    // 游戏反馈槽
    void onGameFinished(int score, bool win);
    void onScoreChanged(int score);

private:
    void setupMainMenu();  // 初始化主菜单界面
    void setupGameUI();    // 初始化游戏内UI（按钮等）
    void switchToGame(GameBase* game); // 切换到游戏模式
    void updateButtons();  // 更新按钮状态

    // 状态定义
    enum AppState {
        MainMenu, // 选择游戏界面
        InGame    // 游戏中
    };

    AppState m_appState;
    GameBase* m_currentGame; // 当前运行的游戏（多态）

    // 具体游戏实例
    MoleGame* m_moleGame;
    PoliceGame* m_policeGame;
    SpaceGame* m_spaceGame;
    AppleGame* m_appleGame;
    FrogGame* m_frogGame;

    // --- UI 元素 ---
    // 1. 主菜单元素
    QLabel* m_titleLabel;
    QPushButton* m_btnMole;
    QPushButton* m_btnPolice;
    QPushButton* m_btnSpace;
    QPushButton* m_btnApple;
    QPushButton* m_btnFrog;
    QPushButton* m_btnExit;

    // 2. 游戏内通用元素 (HUD)
    ImageButton* m_btnStart;
    ImageButton* m_btnPause;
    ImageButton* m_btnSettings;
    ImageButton* m_btnEnd; // 返回主菜单
    ImageButton* m_btnQuitGame;
    
    GameSettings* m_settingsDialog;

    QTimer* m_renderTimer;
};

#endif // GAMEWIDGET_H