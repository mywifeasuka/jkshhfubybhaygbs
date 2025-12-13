#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include "imagebutton.h"

class ConfirmationDialog : public QDialog {
    Q_OBJECT

public:
    // 对话框模式
    enum Mode {
        Mode_ExitGame,    // 退出确认
        Mode_ApplySettings // 设置生效确认
    };

    explicit ConfirmationDialog(Mode mode, QWidget *parent = nullptr);

    // 获取用户的选择：Accepted (是/退出) 或 Rejected (否/继续)
    // 我们复用 QDialog 的 exec() 返回值

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI(Mode mode);

    QPixmap m_bgPixmap;
    QLabel* m_messageLabel;
    ImageButton* m_btnConfirm; // 确认按钮 (退出 / 是)
    ImageButton* m_btnCancel;  // 取消按钮 (继续 / 否)
};

#endif // CONFIRMATIONDIALOG_H