#include "gamewidget.h"
#include "gameresultdialog.h"
#include "confirmationdialog.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPainter>

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent), m_appState(MainMenu), m_currentGame(nullptr)
{
    setFixedSize(800, 600); // 固定窗口大小
    setWindowTitle(QStringLiteral("金山打字通重制版 - C++实战"));

    m_renderTimer = new QTimer(this);
    m_renderTimer->setInterval(16); // 约 60 FPS
    connect(m_renderTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));

    // 1. 初始化游戏实例
    m_moleGame = new MoleGame(this);
    m_policeGame = new PoliceGame(this);
    m_spaceGame = new SpaceGame(this);
    m_appleGame = new AppleGame(this);
    m_frogGame = new FrogGame(this);

    // 2. 初始化设置窗口
    m_settingsDialog = new GameSettings(this);

    // 3. 初始化UI
    setupMainMenu();
    setupGameUI(); // 先创建但不显示
    
    // 默认显示主菜单
    onReturnToMenu();

    
}

GameWidget::~GameWidget() {
    // m_moleGame 和 m_policeGame 是 this 的子对象，会自动析构
}

void GameWidget::setupMainMenu() {
    m_titleLabel = new QLabel(QStringLiteral("请选择游戏"), this);
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    m_titleLabel->move(350, 100);

    m_btnMole = new QPushButton(QStringLiteral("鼠的故事"), this);
    m_btnMole->setGeometry(300, 200, 200, 50);
    connect(m_btnMole, &QPushButton::clicked, this, &GameWidget::onSelectMoleGame);

    m_btnPolice = new QPushButton(QStringLiteral("生死时速"), this);
    m_btnPolice->setGeometry(300, 260, 200, 50);
    connect(m_btnPolice, &QPushButton::clicked, this, &GameWidget::onSelectPoliceGame);

    m_btnSpace = new QPushButton(QStringLiteral("太空大战"), this);
    m_btnSpace->setGeometry(300, 320, 200, 50); 
    connect(m_btnSpace, &QPushButton::clicked, this, &GameWidget::onSelectSpaceGame);

    m_btnApple = new QPushButton(QStringLiteral("拯救苹果"), this);
    m_btnApple->setGeometry(300, 380, 200, 50); 
    connect(m_btnApple, &QPushButton::clicked, this, &GameWidget::onSelectAppleGame);

    m_btnFrog = new QPushButton(QStringLiteral("激流勇进"), this);
    m_btnFrog->setGeometry(300, 440, 200, 50); // 调整位置
    connect(m_btnFrog, &QPushButton::clicked, this, &GameWidget::onSelectFrogGame);

    m_btnExit = new QPushButton(QStringLiteral("退出程序"), this);
    m_btnExit->setGeometry(300, 500, 200, 50);
    connect(m_btnExit, &QPushButton::clicked, this, &GameWidget::onExitApp);
}

void GameWidget::setupGameUI() {
    // 1. 开始按钮 (控制面板)
    m_btnStart = new ImageButton(":/img/public_start.bmp", ":/img/public_start_on.bmp", ":/img/public_start_clicked.bmp", this);
    m_btnStart->move(490, 530);
    connect(m_btnStart, &ImageButton::clicked, this, &GameWidget::onStartGame);

    // 2. 暂停按钮 (控制面板)
    m_btnPause = new ImageButton(":/img/public_pause.bmp", ":/img/public_pause_on.bmp", ":/img/public_pause_clicked.bmp", this);
    m_btnPause->move(400, 510);
    connect(m_btnPause, &ImageButton::clicked, this, &GameWidget::onPauseGame);

    // 3. 结束按钮 (控制面板) - 功能：停止当前局，重置为Ready
    m_btnEnd = new ImageButton(":/img/public_end.bmp", ":/img/public_end_on.bmp", ":/img/public_end_clicked.bmp", this);
    m_btnEnd->move(450, 480); // 放在控制面板区域
    connect(m_btnEnd, &ImageButton::clicked, this, &GameWidget::onStopGameRound);

    // 4. 设置按钮 (控制面板)
    m_btnSettings = new ImageButton(":/img/public_settings.bmp", ":/img/public_settings_on.bmp", ":/img/public_settings_clicked.bmp", this);
    m_btnSettings->move(440, 550);
    connect(m_btnSettings, &ImageButton::clicked, this, &GameWidget::onShowSettings);

    // 5. 退出按钮 (左下角) - 功能：返回主菜单
    m_btnQuitGame = new ImageButton(":/img/public_exit.bmp", ":/img/public_exit_on.bmp", ":/img/public_exit_clicked.bmp", this);
    m_btnQuitGame->move(40, 550); // 左下角位置
    connect(m_btnQuitGame, &ImageButton::clicked, this, &GameWidget::onReturnToMenu);
}

void GameWidget::onStopGameRound() {
    if (m_currentGame) {
        // 停止游戏逻辑（计时器停止、清理地鼠/实体）
        m_currentGame->stopGame();

        // 强制重绘一次，确保画面清除干净
        update();

        // 更新按钮状态
        updateButtons();
    }
}

void GameWidget::onReturnToMenu() {
    if (m_appState == InGame) {
        // 暂停游戏，防止背景还在跑
        bool wasRunning = false;
        if (m_renderTimer->isActive()) {
            m_renderTimer->stop();
            wasRunning = true;
        }

        // 弹出确认对话框
        ConfirmationDialog dlg(ConfirmationDialog::Mode_ExitGame, this);
        if (dlg.exec() == QDialog::Rejected) {
            // 用户点击了“继续” (取消退出)
            if (wasRunning) m_renderTimer->start(); // 恢复游戏
            return;
        }
        // 用户点击了“退出”，继续执行下面的返回菜单逻辑
    }

    m_renderTimer->stop();

    m_appState = MainMenu;
    if (m_currentGame) {
        m_currentGame->stopGame();
        // 断开旧连接，防止信号重复
        m_currentGame->disconnect(this);
        m_currentGame = nullptr;
    }

    // 显示菜单控件
    m_titleLabel->show();
    m_btnMole->show();
    m_btnPolice->show();
    m_btnSpace->show();
    m_btnApple->show();
    m_btnFrog->show();
    m_btnExit->show();

    // 隐藏游戏控件
    m_btnStart->hide();
    m_btnPause->hide();
    m_btnEnd->hide();       
    m_btnSettings->hide();
    m_btnQuitGame->hide();  

    update(); // 重绘（清除游戏背景）
}

void GameWidget::switchToGame(GameBase* game) {
    m_appState = InGame;
    m_currentGame = game;

    // 连接信号
    connect(m_currentGame, &GameBase::gameFinished, this, &GameWidget::onGameFinished);
    connect(m_currentGame, &GameBase::scoreChanged, this, &GameWidget::onScoreChanged);

    // 隐藏菜单
    m_titleLabel->hide();
    m_btnMole->hide();
    m_btnPolice->hide();
    m_btnSpace->hide();
    m_btnApple->hide();
    m_btnFrog->hide();
    m_btnExit->hide();

    // 显示游戏控件
    m_btnStart->show();
    m_btnPause->show();
    m_btnEnd->show();       
    m_btnSettings->show();
    m_btnQuitGame->show();

    // 初始化游戏
    m_currentGame->initGame();

    m_renderTimer->start();
    
    updateButtons();
    update(); // 触发重绘
}

void GameWidget::onSelectMoleGame() {
    switchToGame(m_moleGame);
}

void GameWidget::onSelectPoliceGame() {
    switchToGame(m_policeGame);
}

void GameWidget::onSelectSpaceGame() {
    switchToGame(m_spaceGame);
}

void GameWidget::onSelectAppleGame() {
    switchToGame(m_appleGame);
}

void GameWidget::onSelectFrogGame() {
    switchToGame(m_frogGame);
}

void GameWidget::onStartGame() {
    if (m_currentGame) {
        m_currentGame->startGame();

        if (!m_renderTimer->isActive()) {
            m_renderTimer->start();
        }

        updateButtons();
        setFocus(); // 确保能接收键盘事件
    }
}

void GameWidget::onPauseGame() {
    if (m_currentGame) {
        m_currentGame->pauseGame();

        // 根据游戏状态控制渲染
        if (m_currentGame->getState() == GameState::Paused) {
            m_renderTimer->stop(); // 暂停时不刷新，画面静止
            update(); // 额外刷新一次以显示暂停状态（如果有）
        }
        else {
            m_renderTimer->start();
        }
        updateButtons();
    }
}

void GameWidget::onShowSettings() {
    // 只有鼠的故事支持详细设置，这里做个转换判断
    MoleGame* moleGame = dynamic_cast<MoleGame*>(m_currentGame);
    if (!moleGame) return; // 其他游戏暂不支持或不弹窗

    // 记录旧设置，用于比较是否发生了改变
    GameSettingsData oldSettings = m_settingsDialog->getSettings(); // 假设你在 GameSettings 中实现了 getSettings

    // 显示设置窗口
    if (m_settingsDialog->exec() == QDialog::Accepted) {
        GameSettingsData newSettings = m_settingsDialog->getSettings();

        // 比较设置是否改变 (简单比较几个关键字段)
        bool changed = (oldSettings.gameTimeSec != newSettings.gameTimeSec) ||
            (oldSettings.spawnIntervalMs != newSettings.spawnIntervalMs) ||
            (oldSettings.stayTimeMs != newSettings.stayTimeMs);

        if (changed) {
            // 弹出“立即生效吗？”确认框
            ConfirmationDialog dlg(ConfirmationDialog::Mode_ApplySettings, this);

            if (dlg.exec() == QDialog::Accepted) {
                // --- 点击“是”：应用设置并立即开始 ---
                moleGame->updateSettings(newSettings);
                moleGame->initGame(); // 重置游戏
                onStartGame();        // 立即开始
            }
            else {
                // --- 点击“否”：仅应用设置，不重启 ---
                // 或者什么都不做？根据原版“否”通常是保存但不重置
                moleGame->updateSettings(newSettings);
                // 保持当前状态 (可能在 Ready 或 Paused)
            }
        }
    }
}

void GameWidget::updateButtons() {
    if (!m_currentGame) return;

    GameState state = m_currentGame->getState();

    // 开始按钮：不在游戏中时可用
    m_btnStart->setEnabled(state == GameState::Ready || state == GameState::GameOver);

    // 暂停按钮：游戏中或暂停时可用
    m_btnPause->setEnabled(state == GameState::Playing || state == GameState::Paused);

    // 结束本局按钮：游戏中或暂停时可用 (点了就重置)
    m_btnEnd->setEnabled(state == GameState::Playing || state == GameState::Paused);

    // 设置按钮：未开始时可用
    m_btnSettings->setEnabled(state == GameState::Ready || state == GameState::GameOver);

    // 退出菜单按钮：始终可用
    m_btnQuitGame->setEnabled(true);
}

void GameWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    if (m_appState == MainMenu) {
        // 绘制简单的菜单背景
        painter.fillRect(rect(), QColor(240, 240, 240));
    }
    else if (m_appState == InGame && m_currentGame) {
        // *** 核心：委托给当前游戏绘制 ***
        m_currentGame->draw(painter);
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event) {
    if (m_appState == InGame && m_currentGame) {
        // *** 核心：委托给当前游戏处理按键 ***
        m_currentGame->handleKeyPress(event);
        update(); // 某些操作可能需要重绘
    }
}

void GameWidget::onGameFinished(int score, bool win) {
    // 1. 停止渲染循环 (确保背景静止)
    if (m_renderTimer->isActive()) {
        m_renderTimer->stop();
    }

    // 2. 创建并显示自定义结算对话框
    GameResultDialog dlg(this);
    dlg.setGameResult(score, win);

    // 阻塞式显示对话框，等待用户点击
    dlg.exec();

    // 3. 处理用户选择
    GameResultDialog::ResultAction action = dlg.getSelectedAction();

    if (action == GameResultDialog::Action_Replay) {
        // --- 选项：继续 (重玩) ---
        // 直接重新开始，参数不变
        onStartGame();
    }
    else if (action == GameResultDialog::Action_NextLevel) {
        // --- 选项：下一关 ---
        // 仅针对鼠的游戏增加难度 (需要类型转换或在GameBase加接口)
        // 这里使用 dynamic_cast 安全转换
        MoleGame* moleGame = dynamic_cast<MoleGame*>(m_currentGame);
        if (moleGame) {
            moleGame->increaseDifficulty();
        }
        onStartGame();
    }
    else {
        // --- 选项：结束 (默认) ---
        // 调用结束逻辑，重置画面
        onStopGameRound();
    }
}

void GameWidget::onScoreChanged(int score) {
    // 如果需要显示在 GameWidget 这一层的 HUD，可以在这里处理
    // 目前绘制工作都下放给 GameBase 了，所以这里暂时留空或者打印日志
    // qDebug() << "Score:" << score;
}

void GameWidget::onExitApp() {
    close();
}