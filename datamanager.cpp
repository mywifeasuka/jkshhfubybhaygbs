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

    // 过滤 .txt 文件
    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);

    QFileInfoList fileList = dir.entryInfoList();

    // 为了兼容旧版中文Windows生成的txt，尝试使用 GBK
    // Qt5 需要 #include <QTextCodec>，Qt6 则不同
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    if (!codec) codec = QTextCodec::codecForName("UTF-8");

    for (const QFileInfo& fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            // 转换为 Unicode
            QString content = codec->toUnicode(data);

            // 简单的清洗：去掉过多的换行符，变成一行或者几行
            content = content.replace("\r\n", " ").replace("\n", " ").simplified();

            if (!content.isEmpty()) {
                m_articles.append(content);
            }
            file.close();
        }
    }

    // 如果没读到文件，加几个默认的保底
    if (m_articles.isEmpty()) {
        m_articles << "Technology is best when it brings people together.";
        m_articles << "The art of programming is the art of organizing complexity.";
    }

    qDebug() << "Loaded" << m_articles.size() << "articles from" << dirPath;
}

QString DataManager::getRandomArticle() {
    if (m_articles.isEmpty()) return "No articles loaded.";
    int index = QRandomGenerator::global()->bounded(m_articles.size());
    return m_articles[index];
}