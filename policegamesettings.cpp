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

    // 设置背景图和窗口固定大小 (800x600)
    m_bgPixmap.load(":/img/police_setting_bg.png");
    if (m_bgPixmap.isNull()) {
        QPixmap temp(":/img/mole_setup.bmp");
        if (!temp.isNull()) m_bgPixmap = temp.scaled(800, 600);
        else m_bgPixmap = QPixmap(800, 600); 
    }
    setFixedSize(800, 600);

    setupUI();
    loadArticleList();

    // 信号连接
    connect(m_btnStart, &ImageButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &ImageButton::clicked, this, &QDialog::reject);
}

void PoliceGameSettings::setupUI() {

    m_groupRole = new QButtonGroup(this);

    QToolButton* btnPolice = createRoleButton("police_police", 0, m_groupRole);
    btnPolice->setGeometry(10, 20, 250, 160);
    btnPolice->setChecked(true);

    QToolButton* btnThief = createRoleButton("police_thief", 1, m_groupRole);
    btnThief->setGeometry(300, 20, 250, 160);

    m_groupPoliceItem = new QButtonGroup(this);

    QToolButton* btnPolCar = createItemButton("police_car", 0, m_groupPoliceItem);
    btnPolCar->setGeometry(15, 200, 115, 90);
    btnPolCar->setChecked(true);

    QToolButton* btnPolBike = createItemButton("police_bike", 1, m_groupPoliceItem);

    btnPolBike->setGeometry(135, 200, 115, 90);

    m_groupThiefItem = new QButtonGroup(this);

    QToolButton* btnThiefCar = createItemButton("police_car", 0, m_groupThiefItem);
    btnThiefCar->setGeometry(305, 200, 115, 90);
    btnThiefCar->setChecked(true);

    QToolButton* btnThiefBike = createItemButton("police_bike", 1, m_groupThiefItem);
    btnThiefBike->setGeometry(425, 200, 115, 90);



    m_listArticles = new QListWidget(this);
    m_listArticles->setGeometry(570, 20, 220, 560);

    m_listArticles->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: #FFD700;" 
        "   font-family: 'Microsoft YaHei';"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   padding-top: 30px;" 
        "   padding-left: 5px;"
        "}"
        "QListWidget::item {"
        "   height: 28px;"
        "   border-bottom: 1px solid rgba(255, 215, 0, 30);"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(255, 102, 0, 180);" 
        "   color: white;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: rgba(255, 255, 255, 40);"
        "}"
    );
    m_listArticles->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_listArticles->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_listArticles, &QListWidget::itemClicked, this, &PoliceGameSettings::onArticleClicked);


    QLabel* lblDescText = new QLabel(this);
    lblDescText->setText("说明：\n您可选择扮演 警察或小偷 两个不同的角色。\n\n下方的道具栏分别对应警察和小偷的装备：\n左侧为警察道具，右侧为小偷道具。\n选择不同的加速工具可调节游戏难度（汽车较快，自行车较慢）。");
    lblDescText->setGeometry(20, 330, 520, 150);
    lblDescText->setWordWrap(true);
    lblDescText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblDescText->setStyleSheet("color: #CCCCCC; font-size: 13px; font-family: 'Microsoft YaHei'; background: transparent;");


    int btnWidth = 120;
    int btnHeight = 40;

    m_btnStart = new ImageButton(":/img/police_start.png", ":/img/police_start_hover.png", ":/img/police_start_pressed.png", this);
    if (m_btnStart->width() <= 1) m_btnStart->setFixedSize(btnWidth, btnHeight);

    m_btnCancel = new ImageButton(":/img/police_cancel.png", ":/img/police_cancel_hover.png", ":/img/police_cancel_pressed.png", this);
    if (m_btnCancel->width() <= 1) m_btnCancel->setFixedSize(btnWidth, btnHeight);

    m_btnStart->move(130, 520);
    m_btnCancel->move(310, 520);
}

QToolButton* PoliceGameSettings::createRoleButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

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

QToolButton* PoliceGameSettings::createItemButton(const QString& baseName, int id, QButtonGroup* group) {
    QToolButton* btn = new QToolButton(this);
    btn->setCheckable(true);
    btn->setAutoRaise(true);

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
    if (auto btn = m_groupRole->button(s.role)) btn->setChecked(true);

    if (auto btn = m_groupPoliceItem->button(s.policeVehicle)) btn->setChecked(true);
    if (auto btn = m_groupThiefItem->button(s.thiefVehicle)) btn->setChecked(true);

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