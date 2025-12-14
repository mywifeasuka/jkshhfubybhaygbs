#ifndef POLICEGAMESETTINGS_H
#define POLICEGAMESETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QButtonGroup>
#include <QToolButton>
#include <QListWidget>
#include "imagebutton.h"

// 更新后的设置数据结构，支持分别记录双方的道具
struct PoliceSettingsData {
    int role = 0;           // 0: 警察, 1: 小偷

    int policeVehicle = 0;  // 警察的载具 (0: 汽车, 1: 自行车)
    int thiefVehicle = 0;   // 小偷的载具 (0: 汽车, 1: 自行车)

    int vehicle = 0;        // 【只读】当前玩家角色的载具 (用于游戏逻辑)
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

    // 辅助函数声明
    QToolButton* createRoleButton(const QString& baseName, int id, QButtonGroup* group);
    QToolButton* createItemButton(const QString& baseName, int id, QButtonGroup* group);

    // --- 逻辑组件 ---
    QButtonGroup* m_groupRole;          // 角色选择 (互斥)
    QButtonGroup* m_groupPoliceItem;    // 警察道具选择 (互斥)
    QButtonGroup* m_groupThiefItem;     // 小偷道具选择 (互斥)

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