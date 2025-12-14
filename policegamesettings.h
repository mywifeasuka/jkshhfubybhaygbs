#ifndef POLICEGAMESETTINGS_H
#define POLICEGAMESETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QButtonGroup>
#include <QToolButton>
#include <QListWidget>
#include "imagebutton.h"

struct PoliceSettingsData {
    int role = 0;           // 0: 警察, 1: 小偷
    int vehicle = 0;        // 0: 汽车, 1: 自行车
    QString articleName;    // 选中的文章文件名
};

class PoliceGameSettings : public QDialog {
    Q_OBJECT

public:
    explicit PoliceGameSettings(QWidget* parent = nullptr);

    void setSettings(const PoliceSettingsData& settings);
    PoliceSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onDefaultClicked();
    void onArticleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void loadArticleList();

    // 【关键修复】在此处声明辅助函数，否则cpp文件会报错
    QToolButton* createRoleButton(const QString& baseName, int id, QButtonGroup* group);
    QToolButton* createItemButton(const QString& baseName, int id, QButtonGroup* group);

    // --- 逻辑组件 ---
    QButtonGroup* m_groupRole;
    QButtonGroup* m_groupVehicle;

    // --- UI组件 ---
    QListWidget* m_listArticles;
    ImageButton* m_btnStart;
    ImageButton* m_btnCancel;

    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;

    QString m_selectedArticle;
};

#endif // POLICEGAMESETTINGS_H