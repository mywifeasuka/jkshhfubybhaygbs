#ifndef SPACENAMEDIALOG_H
#define SPACENAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include "imagebutton.h"

class SpaceNameDialog : public QDialog {
    Q_OBJECT

public:
    explicit SpaceNameDialog(int score, QWidget *parent = nullptr);
    QString getName() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();

    int m_score;
    QLineEdit* m_nameEdit;
    ImageButton* m_btnOk;
    QPixmap m_bgPixmap;
};

#endif // SPACENAMEDIALOG_H