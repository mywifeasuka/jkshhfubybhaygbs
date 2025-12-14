#include "spacehighscoredialog.h"
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QPainter>
#include <algorithm>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma execution_character_set("utf-8")
#endif

SpaceHighscoreDialog::SpaceHighscoreDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    m_bgPixmap.load(":/img/space_hiscore_bg.png");
    setFixedSize(800, 600);

    loadScores();
    setupUI();
}

void SpaceHighscoreDialog::loadScores() {
    QFile file("hiscore.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 3) {
            HighScoreEntry entry;
            entry.name = parts[0];
            entry.score = parts[1].toInt();
            entry.date = parts[2];
            m_scores.append(entry);
        }
    }
    file.close();

    std::sort(m_scores.begin(), m_scores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
        return a.score > b.score;
        });

    while (m_scores.size() > 9) {
        m_scores.removeLast();
    }
}

void SpaceHighscoreDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();

    m_btnClose = new ImageButton(":/img/ok.bmp", ":/img/ok_hover.bmp", ":/img/ok_pressed.bmp", this);
    connect(m_btnClose, &ImageButton::clicked, this, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnClose);
    btnLayout->addSpacing(50);

    layout->addLayout(btnLayout);
    layout->setContentsMargins(0, 0, 30, 40);
}

void SpaceHighscoreDialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    }
    else {
        painter.fillRect(rect(), Qt::black);
    }

    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

    int startX = 240;
    int startY = 155;
    int rectW = 350;
    int rectH = 30;
    int spacing = 35;

    for (int i = 0; i < m_scores.size(); ++i) {
        const auto& s = m_scores[i];
        int currentY = startY + i * spacing;
        QRect textRect(startX, currentY, rectW, rectH);

        if (i == 0) painter.setPen(QColor(255, 215, 0));
        else if (i == 1) painter.setPen(QColor(192, 192, 192));
        else if (i == 2) painter.setPen(QColor(205, 127, 50));
        else painter.setPen(Qt::white);

        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, s.name);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, QString::number(s.score));
    }
}