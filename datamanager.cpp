#include "datamanager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QRandomGenerator>
#include <QDebug>

void DataManager::loadArticlesFromDir(const QString& dirPath) {
    m_articles.clear();
    QDir dir(dirPath);

    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);

    QFileInfoList fileList = dir.entryInfoList();

    QTextCodec* codec = QTextCodec::codecForName("GBK");
    if (!codec) codec = QTextCodec::codecForName("UTF-8");
    if (!codec) codec = QTextCodec::codecForLocale();

    for (const QFileInfo& fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();

            QString content;
            if (codec) {
                content = codec->toUnicode(data);
            }
            else {
                content = QString::fromLocal8Bit(data); 
            }

            // 清洗文本：替换换行符为空格，去除首尾空白
            content = content.replace("\r\n", " ").replace("\n", " ").replace("\r", " ").simplified();

            if (!content.isEmpty()) {
                m_articles.append(content);
            }
            file.close();
        }
    }

    // 保底数据，防止文件读取全失败导致后续逻辑除以零
    if (m_articles.isEmpty()) {
        m_articles << "Technology is best when it brings people together";
        m_articles << "Stay hungry stay foolish";
        m_articles << "Knowledge is power";
    }
}

QString DataManager::getRandomArticle() {
    if (m_articles.isEmpty()) {
        m_articles << "Ready Go";
    }
    int index = QRandomGenerator::global()->bounded(m_articles.size());
    return m_articles[index];
}