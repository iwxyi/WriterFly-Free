#ifndef BOARDBASE_H
#define BOARDBASE_H

#include <QObject>
#include "boarditem.h"
#include "fileutil.h"


/**
 * 板板类，剪贴板、收藏版、回收版的基类
 */
class BoardBase : public QObject
{
    Q_OBJECT
public:
    BoardBase(QString path);
    friend class boardpage; // 友元类，界面直接显示

    void addItem(QString str, qint64 timestamp);
    void deleteItem(QString str);
    void deleteItem(int index);
    QList<BoardItem> getRecent();
    QList<BoardItem> getAll();
    QStringList getRecentSting();
    QStringList getAllSting();
    void clear();
    int count();

    void setCount(int x);

private:
	void readFromFile();
	void writeToFile();

private:
    QString file_path;
    QList<BoardItem> contents;

    int max_count;
};

#endif // BOARDBASE_H
