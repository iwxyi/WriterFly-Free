#ifndef TREEOUTLINEWINDOW_H
#define TREEOUTLINEWINDOW_H

#include <QObject>
#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "stringutil.h"

class TreeOutlineWindow : public QDialog
{
    Q_OBJECT
public:
    TreeOutlineWindow(QString content, QWidget *parent = nullptr);

private:
    void initView();
    void initData(QString content);

    void addNextLevel(int index);

signals:

public slots:

private:
    QTreeWidget* tree;

    QStringList paras;
    QList<int> levels;
};

#endif // TREEOUTLINEWINDOW_H
