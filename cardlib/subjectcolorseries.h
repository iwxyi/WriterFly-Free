#ifndef SUBJECTCOLORSERIES_H
#define SUBJECTCOLORSERIES_H

#include <QObject>
#include <QList>
#include <QColor>
#include <QRegExp>
#include "stringutil.h"
#include "fileutil.h"

class SubjectColorSeries : public QObject
{
    Q_OBJECT
public:
    explicit SubjectColorSeries(QString path, QObject *parent = nullptr);

    void loadFromFile();
    void saveToFile();

    int count();
    QList<QColor> getColors();
    QStringList getNames();
    QColor getColor(int index); // 下标要-1，直接获取颜色可用 getColors().at(index)
    QString getColorName(int index);

    void insertItem(int x);
    void appendItem(int x);
    void renameItem(int x, QString name);
    void modifyItem(int x, QColor color);
    void deleteItem(int x);

private:
    void createDefaultSeries();

signals:
    void signalListInserted(int index); // 记住emit的索引要+1
    void signalListModified(int index);
    void signalListDeleted(int index);

public slots:

private:
    QStringList cnames;
    QList<QColor> colors;

    QString file_path;
};

#endif // SUBJECTCOLORSERIES_H
