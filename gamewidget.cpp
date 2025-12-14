#include "gamewidget.h"
#include "gameresultdialog.h"
#include "confirmationdialog.h"
#include "policegamesettings.h"
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

    m_moleGame = new MoleGame(this);
    m_policeGame = new PoliceGame(this);
    m_spaceGame = new SpaceGame(this);
    m_appleGame = new AppleGame(this);
    m_frogGame = new FrogGame(this);
    m_policeSettingsDialog = new PoliceGameSettings(this);

    m_settingsDialog = new GameSettings(this);
    m_appleSettingsDialog = new AppleGameSettings(this);
    m_frogSettingsDialog = new FrogGameSettings(this);

    setupMainMenu();
    setupGameUI(); // 先创建但不显示
    
    // 默认显示主菜单
    onReturnToMenu();

    this->setFocusPolicy(Qt::StrongFocus);

    
}

GameWidget::~GameWidget() {
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
    m_btnStart = new ImageButton(":/img/public_start.bmp", ":/img/public_start_on.bmp", ":/img/public_start_clicked.bmp", this);
    m_btnStart->move(490, 530);
    connect(m_btnStart, &ImageButton::clicked, this, &GameWidget::onStartGame);

    m_btnPause = new ImageButton(":/img/public_pause.bmp", ":/img/public_pause_on.bmp", ":/img/public_pause_clicked.bmp", this);
    m_btnPause->move(400, 510);
    connect(m_btnPause, &ImageButton::clicked, this, &GameWidget::onPauseGame);

    m_btnEnd = new ImageButton(":/img/public_end.bmp", ":/img/public_end_on.bmp", ":/img/public_end_clicked.bmp", this);
    m_btnEnd->move(450, 480); // 放在控制面板区域
    connect(m_btnEnd, &ImageButton::clicked, this, &GameWidget::onStopGameRound);

    m_btnSettings = new ImageButton(":/img/public_settings.bmp", ":/img/public_settings_on.bmp", ":/img/public_settings_clicked.bmp", this);
    m_btnSettings->move(440, 550);
    connect(m_btnSettings, &ImageButton::clicked, this, &GameWidget::onShowSettings);

    m_btnQuitGame = new ImageButton(":/img/public_exit.bmp", ":/img/public_exit_on.bmp", ":/img/public_exit_clicked.bmp", this);
    m_btnQuitGame->move(40, 550); // 左下角位置
    connect(m_btnQuitGame, &ImageButton::clicked, this, &GameWidget::onReturnToMenu);
}

void GameWidget::onStopGameRound() {
    if (m_currentGame) {
        m_currentGame->stopGame();

        update();

        // 更新按钮状态
        updateButtons();
    }
}

void GameWidget::onReturnToMenu() {
    if (m_appState == InGame) {
        bool wasRunning = false;
        if (m_renderTimer->isActive()) {
            m_renderTimer->stop();
            wasRunning = true;
        }

        // 弹出确认对话框
        ConfirmationDialog dlg(ConfirmationDialog::Mode_ExitGame, this);
        if (dlg.exec() == QDialog::Rejected) {
            if (wasRunning) m_renderTimer->start(); // 恢复游戏
            return;
        }
    }

    m_renderTimer->stop();

    m_appState = MainMenu;
    if (m_currentGame) {
        m_currentGame->stopGame();
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

    update(); 
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

    SpaceGame* spaceGame = dynamic_cast<SpaceGame*>(game);



    if (spaceGame) {
        m_btnStart->hide();
        m_btnPause->hide();
        m_btnEnd->hide();
        m_btnSettings->hide();
        m_btnQuitGame->hide();

        // 连接 SpaceGame 特有的退出信号
        disconnect(spaceGame, &SpaceGame::requestReturnToMenu, this, &GameWidget::onReturnToMenu);
        connect(spaceGame, &SpaceGame::requestReturnToMenu, this, &GameWidget::onReturnToMenu);
    }
    else if (game == m_policeGame) {
        m_btnStart->hide();
        m_btnPause->hide();
        m_btnEnd->hide();
        m_btnSettings->hide();

        m_btnQuitGame->show();
        // 加载通用退出按钮资源
        m_btnQuitGame->loadImages(":/img/public_exit.bmp", ":/img/public_exit_on.bmp", ":/img/public_exit_clicked.bmp");
        m_btnQuitGame->move(40, 550); // 左下角位置
    }
    else {
        m_btnStart->show();
        m_btnPause->show();
        m_btnEnd->show();
        m_btnSettings->show();
        m_btnQuitGame->show();

        if (game == m_frogGame) {
            m_btnStart->loadImages(":/img/frog_start.png", ":/img/frog_start_hover.png", ":/img/frog_start_pressed.png");
            m_btnPause->loadImages(":/img/frog_pause.png", ":/img/frog_pause_hover.png", ":/img/frog_pause_pressed.png");
            m_btnEnd->loadImages(":/img/frog_end.png", ":/img/frog_end_hover.png", ":/img/frog_end_pressed.png");
            m_btnSettings->loadImages(":/img/frog_setting.png", ":/img/frog_setting_hover.png", ":/img/frog_setting_pressed.png");
            m_btnQuitGame->loadImages(":/img/frog_exit.png", ":/img/frog_exit_hover.png", ":/img/frog_exit_pressed.png");

            m_btnStart->move(160, 480);
            m_btnPause->move(120, 510);
            m_btnEnd->move(200, 530);
            m_btnSettings->move(150, 550);
            m_btnQuitGame->move(20, 550);
        }
        else {
            m_btnStart->loadImages(":/img/public_start.bmp", ":/img/public_start_on.bmp", ":/img/public_start_clicked.bmp");
            m_btnPause->loadImages(":/img/public_pause.bmp", ":/img/public_pause_on.bmp", ":/img/public_pause_clicked.bmp");
            m_btnEnd->loadImages(":/img/public_end.bmp", ":/img/public_end_on.bmp", ":/img/public_end_clicked.bmp");
            m_btnSettings->loadImages(":/img/public_settings.bmp", ":/img/public_settings_on.bmp", ":/img/public_settings_clicked.bmp");
            m_btnQuitGame->loadImages(":/img/public_exit.bmp", ":/img/public_exit_on.bmp", ":/img/public_exit_clicked.bmp");

            if (game == m_moleGame) {
                m_btnEnd->move(450, 480);
                m_btnPause->move(400, 510);
                m_btnStart->move(490, 530);
                m_btnSettings->move(440, 550);
                m_btnQuitGame->move(40, 550);
            }
            else {
                m_btnStart->move(160, 480);
                m_btnPause->move(120, 510);
                m_btnEnd->move(200, 530);
                m_btnSettings->move(150, 550);
                m_btnQuitGame->move(20, 550);
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
    // 创建并显示设置窗口
    PoliceGameSettings settingsDlg(this);

    if (settingsDlg.exec() == QDialog::Accepted) {
        // 获取设置数据
        PoliceSettingsData data = settingsDlg.getSettings();

        // 切换到游戏界面
        switchToGame(m_policeGame);

        // 应用设置并初始化
        m_policeGame->updateSettings(data);
        m_policeGame->initGame();

    }
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
    MoleGame* moleGame = dynamic_cast<MoleGame*>(m_currentGame);
    if (moleGame) {
        GameSettingsData oldSettings = m_settingsDialog->getSettings();
        if (m_settingsDialog->exec() == QDialog::Accepted) {
            GameSettingsData newSettings = m_settingsDialog->getSettings();
            bool changed = (oldSettings.gameTimeSec != newSettings.gameTimeSec) ||
                 (oldSettings.spawnIntervalMs != newSettings.spawnIntervalMs) ||
                 (oldSettings.stayTimeMs != newSettings.stayTimeMs);
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

    PoliceGame* policeGame = dynamic_cast<PoliceGame*>(m_currentGame);
    if (policeGame) {
        // 弹出设置框
        if (m_policeSettingsDialog->exec() == QDialog::Accepted) {
            PoliceSettingsData data = m_policeSettingsDialog->getSettings();
            ConfirmationDialog dlg(ConfirmationDialog::Mode_ApplySettings, this);
            if (dlg.exec() == QDialog::Accepted) {
                policeGame->updateSettings(data);
                policeGame->initGame(); // 重置游戏应用新角色/难度
                onStartGame();
            }
        }
        return;
    }
}

void GameWidget::updateButtons() {
    if (!m_currentGame) return;

    GameState state = m_currentGame->getState();

    m_btnStart->setEnabled(state == GameState::Ready || state == GameState::GameOver);

    m_btnPause->setEnabled(state == GameState::Playing || state == GameState::Paused);

    m_btnEnd->setEnabled(state == GameState::Playing || state == GameState::Paused);

    m_btnSettings->setEnabled(state == GameState::Ready || state == GameState::GameOver);

    m_btnQuitGame->setEnabled(true);
}

void GameWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    if (m_appState == MainMenu) {
        // 绘制简单的菜单背景
        painter.fillRect(rect(), QColor(240, 240, 240));
    }
    else if (m_appState == InGame && m_currentGame) {
        m_currentGame->draw(painter);
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event) {
    if (m_appState == InGame && m_currentGame) {
        if (event->key() == Qt::Key_Space) {
            m_currentGame->handleKeyPress(event);
            return; // 阻止事件传播
        }

        m_currentGame->handleKeyPress(event);
        update();
    }
}

void GameWidget::onGameFinished(int score, bool win) {
    if (m_renderTimer->isActive()) {
        m_renderTimer->stop();
    }

    GameResultDialog::GameTheme theme = GameResultDialog::Theme_Mole; // 默认

    if (dynamic_cast<AppleGame*>(m_currentGame)) {
        theme = GameResultDialog::Theme_Apple;
    }
    else if (dynamic_cast<FrogGame*>(m_currentGame)) {
        theme = GameResultDialog::Theme_Frog;
    }
    else if (dynamic_cast<PoliceGame*>(m_currentGame)) {
        theme = GameResultDialog::Theme_Police; // 识别警察游戏
    }

    GameResultDialog dlg(theme, this);

    PoliceGame* policeGame = dynamic_cast<PoliceGame*>(m_currentGame);
    if (policeGame) {
        dlg.setRole(policeGame->getRole());
    }

    dlg.setGameResult(score, win);
    dlg.exec();

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

}

void GameWidget::onExitApp() {
    close();
}