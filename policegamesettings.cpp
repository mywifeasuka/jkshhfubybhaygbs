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
    }

    // 强制设置窗口大小为背景图大小（800x600）
    if (!m_bgPixmap.isNull()) setFixedSize(m_bgPixmap.size());
    else setFixedSize(800, 600);

    setupUI();
    loadArticleList();

    // 信号连接
    connect(m_btnStart, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
}

void PoliceGameSettings::setupUI() {
    // 【绝对定位模式】根据您提供的像素坐标设置

    // ================= 1. 角色选择 (250x160) =================
    // 坐标：(10, 20) 和 (300, 20)

    m_groupRole = new QButtonGroup(this);

    // 警察按钮
    QToolButton* btnPolice = createRoleButton("police_police", 0, m_groupRole);
    btnPolice->setGeometry(10, 20, 250, 160);
    btnPolice->setChecked(true);

    // 小偷按钮
    QToolButton* btnThief = createRoleButton("police_thief", 1, m_groupRole);
    btnThief->setGeometry(300, 20, 250, 160);

    // ================= 2. 道具选择 (框大小 250x110) =================
    // 坐标：(10, 190) 和 (300, 190)

    m_groupVehicle = new QButtonGroup(this);

    // 汽车按钮
    QToolButton* btnCar = createItemButton("police_car", 0, m_groupVehicle);
    btnCar->setGeometry(10, 190, 250, 110);
    btnCar->setChecked(true);

    // 自行车按钮
    QToolButton* btnBike = createItemButton("police_bike", 1, m_groupVehicle);
    btnBike->setGeometry(300, 190, 250, 110);

    // ================= 3. 文章列表 (220x560) =================
    // 坐标：(570, 20)

    m_listArticles = new QListWidget(this);
    m_listArticles->setGeometry(570, 20, 220, 560);

    // 样式：透明背景，金色文字，稍微调整内边距让文字不贴边
    m_listArticles->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: #FFD700;" // 金色
        "   font-family: 'Microsoft YaHei';"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   padding-top: 30px;" // 避开背景图上的“可选文章列表”标题
        "   padding-left: 5px;"
        "}"
        "QListWidget::item {"
        "   height: 28px;"
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
    // 位于道具下方。道具框结束于 Y=190+110=300。
    // 说明文字大约从 Y=340 开始

    QLabel* lblDescText = new QLabel(this);
    lblDescText->setText("您可选择扮演 警察或小偷 两个不同的角色\n\n选择不同的加速工具调节游戏难度");
    lblDescText->setGeometry(20, 340, 520, 100); // 宽度覆盖左侧两列
    lblDescText->setWordWrap(true);
    lblDescText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblDescText->setStyleSheet("color: #CCCCCC; font-size: 14px; font-family: 'Microsoft YaHei'; background: transparent;");

    // ================= 5. 底部按钮 (开始/取消) =================
    // 放在左侧区域的底部居中
    // 整个左侧宽度约 550，中心点 275
    // Y坐标靠近底部，约 520

    int btnWidth = 120;
    int btnHeight = 40;

    m_btnStart = new ImageButton(":/img/police_start.png", ":/img/police_start.png", ":/img/police_start.png", this);
    if (m_btnStart->width() == 0) m_btnStart->setFixedSize(btnWidth, btnHeight);

    m_btnCancel = new ImageButton(":/img/police_cancel.png", ":/img/police_cancel.png", ":/img/police_cancel.png", this);
    if (m_btnCancel->width() == 0) m_btnCancel->setFixedSize(btnWidth, btnHeight);

    // 绝对定位放置按钮
    m_btnStart->move(130, 520);
    m_btnCancel->move(310, 520);
}

// 辅助函数：创建【角色】按钮 (填满框，带边框)
QToolButton* PoliceGameSettings::createRoleButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

    // 角色逻辑：选中和未选中通过 image 切换图片
    // 并且图片会铺满整个按钮区域 (250x160)
    // QSS 中 image 默认是 contain，border-image 是 stretch
    // 为了填满，我们使用 border-image 或者 image + padding:0
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

// 辅助函数：创建【道具】按钮 (小于框，居中)
QToolButton* PoliceGameSettings::createItemButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

    // 道具逻辑：图片小于边框，居中显示
    // 我们使用 image 属性（保持比例）配合 padding 来缩小图片显示区域
    // 选中时切换为 _selected.png
    QString qss = QString(
        "QToolButton {"
        "    border: none;"
        "    background: transparent;"
        "    image: url(:/img/%1.png);"
        "    padding: 20px;" // 【关键】增加内边距，使贴图看起来比按钮小
        "}"
        "QToolButton:checked {"
        "    image: url(:/img/%1_selected.png);"
        "}"
    ).arg(baseName);

    btn->setStyleSheet(qss);
    group->addButton(btn, id);
    return btn;
}

// ... 后面这部分代码保持不变 ...
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
    if (auto btn = m_groupRole->button(s.role)) btn->setChecked(true);
    if (auto btn = m_groupVehicle->button(s.vehicle)) btn->setChecked(true);
    QList<QListWidgetItem*> items = m_listArticles->findItems(s.articleName, Qt::MatchContains);
    if (!items.isEmpty()) {
        m_listArticles->setCurrentItem(items.first());
        m_selectedArticle = s.articleName;
    }
}

PoliceSettingsData PoliceGameSettings::getSettings() const {
    PoliceSettingsData s;
    s.role = m_groupRole->checkedId();
    s.vehicle = m_groupVehicle->checkedId();
    s.articleName = m_selectedArticle;
    return s;
}

void PoliceGameSettings::onDefaultClicked() {
    if (auto btn = m_groupRole->button(0)) btn->setChecked(true);
    if (auto btn = m_groupVehicle->button(0)) btn->setChecked(true);
    if (m_listArticles->count() > 0) {
        m_listArticles->setCurrentRow(0);
        onArticleClicked(m_listArticles->item(0));
    }
}