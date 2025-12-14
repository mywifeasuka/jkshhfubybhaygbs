#ifndef GAMERESULTDIALOG_H
#define GAMERESULTDIALOG_H

#include <QDialog>
#include <QLabel>
#include "imagebutton.h"

class GameResultDialog : public QDialog {
    Q_OBJECT

public:
    // 游戏主题类型
    enum GameTheme {
        Theme_Mole,
        Theme_Apple,
        Theme_Frog,
        Theme_Police // 生死时速主题
    };

    enum ResultAction {
        Action_None,
        Action_Replay,
        Action_NextLevel,
        Action_End
    };

    explicit GameResultDialog(GameTheme theme, QWidget* parent = nullptr);

    void setGameResult(int score, bool isWin);

    // 【新增】设置角色 (针对生死时速)
    void setRole(int role);

    ResultAction getSelectedAction() const { return m_selectedAction; }

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onReplayClicked();
    void onNextLevelClicked();
    void onEndClicked();

private:
    void setupUI();

    GameTheme m_currentTheme;
    int m_role; // 0: Police, 1: Thief
    QPixmap m_bgPixmap;
    ResultAction m_selectedAction;

    QLabel* m_messageLabel;
    QLabel* m_scoreLabel;

    ImageButton* m_btnReplay;
    ImageButton* m_btnNext;
    ImageButton* m_btnEnd;
};

#endif // GAMERESULTDIALOG_H