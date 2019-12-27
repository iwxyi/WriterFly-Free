#ifndef SHORTCUTWINDOW_H
#define SHORTCUTWINDOW_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include "globalvar.h"
#include "fileutil.h"
#include "stringutil.h"
#include "shortcutinfoeditor.h"


/**
 * 快捷键设置界面
 */

class ShortcutWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ShortcutWindow(QWidget *parent = nullptr);

private:
    void initView();
    void refreshList();
    void initEvent();
    void setRow(int index, ShortcutEntry se);

    QString getEnvName(ShortcutEnvironment env);

signals:

public slots:
    void slotFilterChanged(const QString& text);
    void slotAddNewShortcut();
    void slotCellClicked(int row, int col);
    void slotShowMenu(QPoint p);

    void actionEdit(int row, int col);
    void actionInsert(int row);
    void actionDelete(int row);

private:
    QList<ShortcutEntry>& shrt = gd->shortcuts.shortcuts;
    QLabel* search_label;
    QLineEdit* search_edit;
    QPushButton* search_btn;
    QTableWidget* list_widget;
    QMenu* menu;
};

/*
<SHORTCUT>
    <KEY>ctrl+p</KEY>
    <CMD>综合搜索</CMD>
    <ENV>global<ENV>
</SHORTCUT>
<SHORTCUT>
    <KEY>ctrl+shift+v</KEY>
    <CMD>粘贴纯文本</CMD>
    <CMD>排版</CMD>
    <ENV>editor</ENV>
</SHORTCUT>
<SHORTCUT>
    <KEY>ctrl+‘</KEY>
    <CMD>插入文本 “|”</CMD>
    <CMD>排版</CMD>
    <ENV>editor</ENV>
    <LEFT>^$|[^“”‘’]$</LEFT>
    <RIGHT>^$|^[^“”‘’]</RIGHT>
</SHORTCUT>
*/
/*
[
    {
        "key": "ctrl+P",
        "command":"综合搜索"
    },
    {
        "key": "ctrl+shift+V",
        "command": [
            {
                "command": "粘贴纯文本",
            },
            {
                "command": "排版"
            }
        ]
    }
]
*/

#endif // SHORTCUTWINDOW_H
