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
        Theme_Apple
    };

    enum ResultAction {
        Action_None,
        Action_Replay,
        Action_NextLevel,
        Action_End
    };

    // 构造函数增加 theme 参数
    explicit GameResultDialog(GameTheme theme, QWidget* parent = nullptr);

    void setGameResult(int score, bool isWin);
    ResultAction getSelectedAction() const { return m_selectedAction; }

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onReplayClicked();
    void onNextLevelClicked();
    void onEndClicked();

private:
    void setupUI();

    GameTheme m_currentTheme; // 当前主题
    QPixmap m_bgPixmap;
    ResultAction m_selectedAction;

    QLabel* m_messageLabel;
    QLabel* m_scoreLabel;

    ImageButton* m_btnReplay;
    ImageButton* m_btnNext;
    ImageButton* m_btnEnd;
};

#endif // GAMERESULTDIALOG_H