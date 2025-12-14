#ifndef SPACEHIGHSCOREDIALOG_H
#define SPACEHIGHSCOREDIALOG_H

#include <QDialog>
#include <QList>
#include "imagebutton.h"

struct HighScoreEntry {
    QString name;
    int score;
    QString date;
};

class SpaceHighscoreDialog : public QDialog {
    Q_OBJECT
public:
    explicit SpaceHighscoreDialog(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void loadScores();
    void setupUI();

    QList<HighScoreEntry> m_scores;
    QPixmap m_bgPixmap;
    ImageButton* m_btnClose;
};

#endif