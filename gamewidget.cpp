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
    m_appleSettingsDialog = new AppleGameSettings(this);
    m_frogSettingsDialog = new FrogGameSettings(this);

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

    // 隐藏主菜单
    m_titleLabel->hide();
    m_btnMole->hide();
    m_btnPolice->hide();
    m_btnSpace->hide();
    m_btnApple->hide();
    m_btnFrog->hide();
    m_btnExit->hide();

    // 【修改点】判断是否为太空大战
    SpaceGame* spaceGame = dynamic_cast<SpaceGame*>(game);

    if (spaceGame) {
        // === 太空大战 (特殊处理) ===
        // 隐藏所有通用 HUD 按钮 (因为 SpaceGame 有自己的内部 UI)
        m_btnStart->hide();
        m_btnPause->hide();
        m_btnEnd->hide();
        m_btnSettings->hide();
        m_btnQuitGame->hide();

        // 连接 SpaceGame 特有的退出信号 (请求返回主菜单)
        // 先断开以防重复连接
        disconnect(spaceGame, &SpaceGame::requestReturnToMenu, this, &GameWidget::onReturnToMenu);
        connect(spaceGame, &SpaceGame::requestReturnToMenu, this, &GameWidget::onReturnToMenu);
    }
    else {
        // === 其他游戏 (显示并配置通用按钮) ===
        m_btnStart->show();
        m_btnPause->show();
        m_btnEnd->show();
        m_btnSettings->show();
        m_btnQuitGame->show();

        if (game == m_frogGame) {
            // --- 激流勇进 ---
            // 切换为青蛙专属皮肤
            m_btnStart->loadImages(":/img/frog_start.png", ":/img/frog_start_hover.png", ":/img/frog_start_pressed.png");
            m_btnPause->loadImages(":/img/frog_pause.png", ":/img/frog_pause_hover.png", ":/img/frog_pause_pressed.png");
            m_btnEnd->loadImages(":/img/frog_end.png", ":/img/frog_end_hover.png", ":/img/frog_end_pressed.png");
            m_btnSettings->loadImages(":/img/frog_setting.png", ":/img/frog_setting_hover.png", ":/img/frog_setting_pressed.png");
            m_btnQuitGame->loadImages(":/img/frog_exit.png", ":/img/frog_exit_hover.png", ":/img/frog_exit_pressed.png");

            // 调整坐标
            m_btnStart->move(160, 480);
            m_btnPause->move(120, 510);
            m_btnEnd->move(200, 530);
            m_btnSettings->move(150, 550);
            m_btnQuitGame->move(20, 550);
        }
        else {
            // --- 其他游戏 (鼠、苹果、生死时速) ---
            // 恢复为通用灰色皮肤
            m_btnStart->loadImages(":/img/public_start.bmp", ":/img/public_start_on.bmp", ":/img/public_start_clicked.bmp");
            m_btnPause->loadImages(":/img/public_pause.bmp", ":/img/public_pause_on.bmp", ":/img/public_pause_clicked.bmp");
            m_btnEnd->loadImages(":/img/public_end.bmp", ":/img/public_end_on.bmp", ":/img/public_end_clicked.bmp");
            m_btnSettings->loadImages(":/img/public_settings.bmp", ":/img/public_settings_on.bmp", ":/img/public_settings_clicked.bmp");
            m_btnQuitGame->loadImages(":/img/public_exit.bmp", ":/img/public_exit_on.bmp", ":/img/public_exit_clicked.bmp");

            // 调整坐标
            if (game == m_moleGame) {
                // 鼠的故事
                m_btnEnd->move(450, 480);
                m_btnPause->move(400, 510);
                m_btnStart->move(490, 530);
                m_btnSettings->move(440, 550);
                m_btnQuitGame->move(40, 550);
            }
            else if (game == m_appleGame) {
                // 拯救苹果
                m_btnStart->move(160, 480);
                m_btnPause->move(120, 510);
                m_btnEnd->move(200, 530);
                m_btnSettings->move(150, 550);
                m_btnQuitGame->move(20, 550);
            }
            else {
                // 生死时速 (默认位置)
                m_btnEnd->move(450, 480);
                m_btnPause->move(400, 510);
                m_btnStart->move(490, 530);
                m_btnSettings->move(440, 550);
                m_btnQuitGame->move(40, 550);
            }
        }
    }

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
    // 1. 判断是否是鼠的游戏
    MoleGame* moleGame = dynamic_cast<MoleGame*>(m_currentGame);
    if (moleGame) {
        // ... 原有的鼠的游戏设置逻辑 ...
        GameSettingsData oldSettings = m_settingsDialog->getSettings();
        if (m_settingsDialog->exec() == QDialog::Accepted) {
            GameSettingsData newSettings = m_settingsDialog->getSettings();
            bool changed = (oldSettings.gameTimeSec != newSettings.gameTimeSec) ||
                 (oldSettings.spawnIntervalMs != newSettings.spawnIntervalMs) ||
                 (oldSettings.stayTimeMs != newSettings.stayTimeMs);
            // ... 比较 ...
            if (changed) {
                ConfirmationDialog dlg(ConfirmationDialog::Mode_ApplySettings, this);
                if (dlg.exec() == QDialog::Accepted) {
                    moleGame->updateSettings(newSettings);
                    moleGame->initGame();
                    onStartGame();
                }
                else {
                    moleGame->updateSettings(newSettings);
                }
            }
        }
        return;
    }

    // 判断是否是苹果游戏
    AppleGame* appleGame = dynamic_cast<AppleGame*>(m_currentGame);
    if (appleGame) {
        AppleSettingsData oldSettings = m_appleSettingsDialog->getSettings();

        if (m_appleSettingsDialog->exec() == QDialog::Accepted) {
            AppleSettingsData newSettings = m_appleSettingsDialog->getSettings();

            // 简单比较是否改变
            bool changed = (oldSettings.level != newSettings.level) ||
                (oldSettings.targetCount != newSettings.targetCount) ||
                (oldSettings.failCount != newSettings.failCount);

            if (changed) {
                ConfirmationDialog dlg(ConfirmationDialog::Mode_ApplySettings, this);
                if (dlg.exec() == QDialog::Accepted) {
                    appleGame->updateSettings(newSettings);
                    appleGame->initGame();
                    onStartGame();
                }
                else {
                    appleGame->updateSettings(newSettings);
                }
            }
        }
        return;
    }

    FrogGame* frogGame = dynamic_cast<FrogGame*>(m_currentGame);
    if (frogGame) {
        FrogSettingsData oldSettings = m_frogSettingsDialog->getSettings();

        if (m_frogSettingsDialog->exec() == QDialog::Accepted) {
            FrogSettingsData newSettings = m_frogSettingsDialog->getSettings();

            // 判断是否需要重启
            bool changed = (oldSettings.difficulty != newSettings.difficulty) ||
                (oldSettings.dictionaryFile != newSettings.dictionaryFile);

            if (changed) {
                ConfirmationDialog dlg(ConfirmationDialog::Mode_ApplySettings, this);
                if (dlg.exec() == QDialog::Accepted) {
                    frogGame->updateSettings(newSettings); // 传入设置
                    frogGame->initGame(); // 重置游戏
                    onStartGame();
                }
                else {
                    frogGame->updateSettings(newSettings); // 仅更新参数
                }
            }
        }
        return;
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
    if (m_renderTimer->isActive()) {
        m_renderTimer->stop();
    }

    // 【修改】判断当前游戏类型
    GameResultDialog::GameTheme theme = GameResultDialog::Theme_Mole; // 默认

    // 如果是苹果游戏，切换主题
    if (dynamic_cast<AppleGame*>(m_currentGame)) {
        theme = GameResultDialog::Theme_Apple;
    }
    else if (dynamic_cast<FrogGame*>(m_currentGame)) {
        theme = GameResultDialog::Theme_Frog; 
    }

    // 创建对话框并传入主题
    GameResultDialog dlg(theme, this);
    dlg.setGameResult(score, win);

    dlg.exec();

    // ... 后续处理逻辑不变 ...
    GameResultDialog::ResultAction action = dlg.getSelectedAction();
    if (action == GameResultDialog::Action_Replay) {
        onStartGame();
    }
    else if (action == GameResultDialog::Action_NextLevel) {
        MoleGame* moleGame = dynamic_cast<MoleGame*>(m_currentGame);
        if (moleGame) moleGame->increaseDifficulty();
        onStartGame();
    }
    else {
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