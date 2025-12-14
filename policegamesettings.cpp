#include "policegamesettings.h"
#include <QPainter>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

// 强制 UTF-8 防止乱码
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma execution_character_set("utf-8")
#endif

PoliceGameSettings::PoliceGameSettings(QWidget* parent) : QDialog(parent), m_isDragging(false) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 1. 设置背景图和窗口固定大小 (800x600)
    m_bgPixmap.load(":/img/police_setting_bg.png");
    if (m_bgPixmap.isNull()) {
        QPixmap temp(":/img/mole_setup.bmp");
        if (!temp.isNull()) m_bgPixmap = temp.scaled(800, 600);
        else m_bgPixmap = QPixmap(800, 600); // 纯黑保底
    }
    setFixedSize(800, 600);

    setupUI();
    loadArticleList();

    // 信号连接
    connect(m_btnStart, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
}

void PoliceGameSettings::setupUI() {
    // 【绝对定位模式】根据精确坐标设置

    // ================= 1. 角色选择 (250x160) =================
    // 警察: (10, 20), 小偷: (300, 20)

    m_groupRole = new QButtonGroup(this);

    // 警察按钮
    QToolButton* btnPolice = createRoleButton("police_police", 0, m_groupRole);
    btnPolice->setGeometry(10, 20, 250, 160);
    btnPolice->setChecked(true);

    // 小偷按钮
    QToolButton* btnThief = createRoleButton("police_thief", 1, m_groupRole);
    btnThief->setGeometry(300, 20, 250, 160);

    // ================= 2. 道具选择 (框大小 250x110) =================
    // 需求：每个框内提供两种贴图选择 (汽车/自行车)
    // 道具贴图要小于边框。我们在框内放置两个较小的按钮。
    // 框内空间: 250宽。 两个按钮平分，每个约 125宽。
    // 按钮位置微调以居中。

    // --- 左侧道具框 (警察的道具) ---
    // 框位置: (10, 190)
    m_groupPoliceItem = new QButtonGroup(this);

    // 警察-汽车
    QToolButton* btnPolCar = createItemButton("police_car", 0, m_groupPoliceItem);
    // 放在框的左半边: x=10+5, y=190+10, w=115, h=90
    btnPolCar->setGeometry(15, 200, 115, 90);
    btnPolCar->setChecked(true);

    // 警察-自行车
    QToolButton* btnPolBike = createItemButton("police_bike", 1, m_groupPoliceItem);
    // 放在框的右半边: x=10+125+5
    btnPolBike->setGeometry(135, 200, 115, 90);

    // --- 右侧道具框 (小偷的道具) ---
    // 框位置: (300, 190)
    m_groupThiefItem = new QButtonGroup(this);

    // 小偷-汽车
    QToolButton* btnThiefCar = createItemButton("police_car", 0, m_groupThiefItem);
    btnThiefCar->setGeometry(305, 200, 115, 90);
    btnThiefCar->setChecked(true);

    // 小偷-自行车
    QToolButton* btnThiefBike = createItemButton("police_bike", 1, m_groupThiefItem);
    btnThiefBike->setGeometry(425, 200, 115, 90);


    // ================= 3. 文章列表 (220x560) =================
    // 坐标: (570, 20)

    m_listArticles = new QListWidget(this);
    m_listArticles->setGeometry(570, 20, 220, 560);

    // 样式美化
    m_listArticles->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: #FFD700;" // 金色
        "   font-family: 'Microsoft YaHei';"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   padding-top: 30px;" // 避开背景标题
        "   padding-left: 5px;"
        "}"
        "QListWidget::item {"
        "   height: 28px;"
        "   border-bottom: 1px solid rgba(255, 215, 0, 30);"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(255, 102, 0, 180);" // 选中背景橙色
        "   color: white;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: rgba(255, 255, 255, 40);"
        "}"
    );
    m_listArticles->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_listArticles->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_listArticles, &QListWidget::itemClicked, this, &PoliceGameSettings::onArticleClicked);

    // ================= 4. 游戏说明 =================
    // 位于道具下方。道具框结束于 Y=300。
    // 说明文字区域大约 X=20, Y=330, W=520, H=150

    QLabel* lblDescText = new QLabel(this);
    lblDescText->setText("说明：\n您可选择扮演 警察或小偷 两个不同的角色。\n\n下方的道具栏分别对应警察和小偷的装备：\n左侧为警察道具，右侧为小偷道具。\n选择不同的加速工具可调节游戏难度（汽车较快，自行车较慢）。");
    lblDescText->setGeometry(20, 330, 520, 150);
    lblDescText->setWordWrap(true);
    lblDescText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblDescText->setStyleSheet("color: #CCCCCC; font-size: 13px; font-family: 'Microsoft YaHei'; background: transparent;");

    // ================= 5. 底部按钮 (开始/取消) =================
    // 居中放置在左侧区域底部 Y=520

    int btnWidth = 120;
    int btnHeight = 40;

    m_btnStart = new ImageButton(":/img/police_start.png", ":/img/police_start_hover.png", ":/img/police_start_pressed.png", this);
    // 资源加载保护
    if (m_btnStart->width() <= 1) m_btnStart->setFixedSize(btnWidth, btnHeight);

    m_btnCancel = new ImageButton(":/img/police_cancel.png", ":/img/police_cancel_hover.png", ":/img/police_cancel_pressed.png", this);
    if (m_btnCancel->width() <= 1) m_btnCancel->setFixedSize(btnWidth, btnHeight);

    // 绝对定位
    m_btnStart->move(130, 520);
    m_btnCancel->move(310, 520);
}

// 辅助函数：创建【角色】按钮
// 要求：贴图占满整个区域，通过切换贴图实现高亮
QToolButton* PoliceGameSettings::createRoleButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

    // 使用 border-image 确保图片填满 geometry 设置的区域
    // 选中时: baseName_selected.png
    // 未选中: baseName.png
    QString qss = QString(
        "QToolButton {"
        "    border: none;"
        "    background: transparent;"
        "    border-image: url(:/img/%1.png) 0 0 0 0 stretch stretch;"
        "}"
        "QToolButton:checked {"
        "    border-image: url(:/img/%1_selected.png) 0 0 0 0 stretch stretch;"
        "}"
    ).arg(baseName);

    btn->setStyleSheet(qss);
    group->addButton(btn, id);
    return btn;
}

// 辅助函数：创建【道具】按钮
// 要求：大小小于边框，有选中和正常两个状态
QToolButton* PoliceGameSettings::createItemButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

    // 道具贴图本身较小，不需要 border-image 拉伸，使用 image 居中显示即可
    // 选中时切换图片 _selected.png
    QString qss = QString(
        "QToolButton {"
        "    border: none;"
        "    background: transparent;"
        "    image: url(:/img/%1.png);"
        "}"
        "QToolButton:checked {"
        "    image: url(:/img/%1_selected.png);"
        "}"
        "QToolButton:hover {"
        "    image: url(:/img/%1_selected.png);" // 悬停预览选中态
        "}"
    ).arg(baseName);

    btn->setStyleSheet(qss);
    group->addButton(btn, id);
    return btn;
}

void PoliceGameSettings::loadArticleList() {
    QString dirPath = QCoreApplication::applicationDirPath() + "/Data/English/E_General";
    QDir dir(dirPath);
    if (!dir.exists()) {
        m_listArticles->addItem("未找到文章目录");
        return;
    }
    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo& fileInfo : fileList) {
        QString displayName = fileInfo.baseName();
        QListWidgetItem* item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, fileInfo.fileName());
        m_listArticles->addItem(item);
    }
    if (m_listArticles->count() > 0) {
        m_listArticles->setCurrentRow(0);
        onArticleClicked(m_listArticles->item(0));
    }
}

void PoliceGameSettings::onArticleClicked(QListWidgetItem* item) {
    if (item) m_selectedArticle = item->data(Qt::UserRole).toString();
}

void PoliceGameSettings::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) painter.drawPixmap(0, 0, width(), height(), m_bgPixmap);
    else painter.fillRect(rect(), QColor(40, 40, 40));
}

void PoliceGameSettings::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}
void PoliceGameSettings::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void PoliceGameSettings::setSettings(const PoliceSettingsData& s) {
    // 恢复角色选中
    if (auto btn = m_groupRole->button(s.role)) btn->setChecked(true);

    // 恢复道具选中 (注意这里有两个组)
    if (auto btn = m_groupPoliceItem->button(s.policeVehicle)) btn->setChecked(true);
    if (auto btn = m_groupThiefItem->button(s.thiefVehicle)) btn->setChecked(true);

    // 恢复文章选中
    QList<QListWidgetItem*> items = m_listArticles->findItems(s.articleName, Qt::MatchContains);
    if (!items.isEmpty()) {
        m_listArticles->setCurrentItem(items.first());
        m_selectedArticle = s.articleName;
    }
}

PoliceSettingsData PoliceGameSettings::getSettings() const {
    PoliceSettingsData s;
    s.role = m_groupRole->checkedId();
    s.policeVehicle = m_groupPoliceItem->checkedId();
    s.thiefVehicle = m_groupThiefItem->checkedId();

    // 根据当前角色决定最终的 vehicle 参数 (兼容旧逻辑)
    if (s.role == 0) s.vehicle = s.policeVehicle;
    else s.vehicle = s.thiefVehicle;

    s.articleName = m_selectedArticle;
    return s;
}

void PoliceGameSettings::onDefaultClicked() {
    if (auto btn = m_groupRole->button(0)) btn->setChecked(true);
    if (auto btn = m_groupPoliceItem->button(0)) btn->setChecked(true);
    if (auto btn = m_groupThiefItem->button(0)) btn->setChecked(true);

    if (m_listArticles->count() > 0) {
        m_listArticles->setCurrentRow(0);
        onArticleClicked(m_listArticles->item(0));
    }
}