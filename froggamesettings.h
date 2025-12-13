#ifndef FROGGAMESETTINGS_H
#define FROGGAMESETTINGS_H

#include <QDialog>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QMouseEvent>
#include "imagebutton.h"

// 激流勇进设置数据
struct FrogSettingsData {
    int difficulty = 1;      // 难度 1-9
    QString dictionaryFile;  // 词库文件名 (e.g., "4W.DAT")
    QString dictionaryName;  // 显示名称 (e.g., "大学英语4级")
};

class FrogGameSettings : public QDialog {
    Q_OBJECT

public:
    explicit FrogGameSettings(QWidget *parent = nullptr);
    
    void setSettings(const FrogSettingsData &settings);
    FrogSettingsData getSettings() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onDifficultyChanged(int value);
    void onDefaultClicked();

private:
    void setupUI();
    void setupSliderStyle(QSlider* slider);

    // UI 组件
    QComboBox* m_comboCourse; // 课程选择
    QSlider* m_sliderDiff;    // 难度滑块
    QLabel* m_labelDiff;      // 难度数值显示

    ImageButton* m_btnOk;
    ImageButton* m_btnCancel;
    ImageButton* m_btnDefault;

    QPixmap m_bgPixmap;
    QPoint m_dragPosition;
    bool m_isDragging;
    
    // 存储文件名映射
    QList<QPair<QString, QString>> m_courseMap;
};

#endif // FROGGAMESETTINGS_H