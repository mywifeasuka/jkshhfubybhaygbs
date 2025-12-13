#ifndef GAMERESULTDIALOG_H
#define GAMERESULTDIALOG_H

#include <QDialog>
#include <QLabel>
#include "imagebutton.h"

class GameResultDialog : public QDialog {
    Q_OBJECT

public:
    // 结果类型枚举
    enum ResultAction {
        Action_None,
        Action_Replay,    // 继续 (重玩当前难度)
        Action_NextLevel, // 下一关 (提高难度)
        Action_End        // 结束 (回到游戏待机界面)
    };

    explicit GameResultDialog(QWidget *parent = nullptr);

    // 设置游戏结果：分数和是否胜利
    void setGameResult(int score, bool isWin);
    
    // 获取用户点击的操作
    ResultAction getSelectedAction() const { return m_selectedAction; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onReplayClicked();
    void onNextLevelClicked();
    void onEndClicked();

private:
    void setupUI();

    QPixmap m_bgPixmap;
    ResultAction m_selectedAction;

    // UI 元素
    QLabel* m_messageLabel; // 显示"恭喜"或"认输"
    QLabel* m_scoreLabel;   // 显示分数
    
    ImageButton* m_btnReplay; // 继续
    ImageButton* m_btnNext;   // 下一关
    ImageButton* m_btnEnd;    // 结束
};

#endif // GAMERESULTDIALOG_H