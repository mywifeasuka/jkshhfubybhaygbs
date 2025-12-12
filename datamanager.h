#pragma once
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>

class DataManager {
public:
    static DataManager& instance() {
        static DataManager instance;
        return instance;
    }

    // 加载指定目录下的所有文章
    void loadArticlesFromDir(const QString& dirPath);

    // 随机获取一篇文章
    QString getRandomArticle();

private:
    DataManager() {}
    QStringList m_articles;
};

#endif // DATAMANAGER_H
