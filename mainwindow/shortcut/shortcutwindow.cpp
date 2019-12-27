#include "shortcutwindow.h"

ShortcutWindow::ShortcutWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("自定义快捷键");
    setWindowModality(Qt::WindowModal);
    setMinimumWidth(400);

    initView();
    refreshList();
    initEvent();
}

void ShortcutWindow::initView()
{
    search_label = new QLabel(this);
    search_edit = new QLineEdit(this);
    search_btn = new QPushButton("添加", this);
    list_widget = new QTableWidget(this);

    search_btn->setToolTip("新建快捷键，即刻生效");
    search_edit->setPlaceholderText("查找");
    list_widget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止双击编辑

    QHBoxLayout *search_hlayout = new QHBoxLayout;
    search_hlayout->addWidget(search_label);
    search_hlayout->addWidget(search_edit);
    search_hlayout->addWidget(search_btn);
    QVBoxLayout *main_vlayout = new QVBoxLayout;
    main_vlayout->addLayout(search_hlayout);
    main_vlayout->addWidget(list_widget);
    setLayout(main_vlayout);

    QStringList headers;
    headers << "命令"
            << "快捷键"
            << "位置"
            << "条件";
    list_widget->setHorizontalHeaderLabels(headers);
    list_widget->setColumnCount(headers.length());
    list_widget->setContextMenuPolicy (Qt::CustomContextMenu);
}

void ShortcutWindow::refreshList()
{
    list_widget->setRowCount(shrt.length());
    for (int row = 0; row < shrt.size(); row++)
    {
        setRow(row, shrt.at(row));
    }
}

void ShortcutWindow::initEvent()
{
    connect(search_edit, SIGNAL(textChanged(const QString &)), this, SLOT(slotFilterChanged(const QString &)));
    connect(search_btn, SIGNAL(clicked()), this, SLOT(slotAddNewShortcut()));
    connect(list_widget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotCellClicked(int, int)));
    connect(list_widget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(slotShowMenu(QPoint)));

    menu = new QMenu(this);
    QAction* edit_action = new QAction("编辑", this);
    QAction* insert_action = new QAction("插入", this);
    QAction* delete_action = new QAction("删除", this);
    menu->addAction(edit_action);
    menu->addAction(insert_action);
    menu->addAction(delete_action);
    connect(edit_action, &QAction::triggered, [=]{
        if (list_widget->currentRow() < 0 || list_widget->currentColumn() < 0)
            return ;
        actionEdit(list_widget->currentRow(), list_widget->currentColumn());
    });
    connect(insert_action, &QAction::triggered, [=]{
        if (list_widget->currentRow() < 0 || list_widget->currentColumn() < 0)
            return ;
        actionInsert(list_widget->currentRow());
    });
    connect(delete_action, &QAction::triggered, [=]{
        if (list_widget->currentRow() < 0 || list_widget->currentColumn() < 0)
            return ;
        actionDelete(list_widget->currentRow());
    });

}

void ShortcutWindow::setRow(int index, ShortcutEntry se)
{
    list_widget->setItem(index, 0, new QTableWidgetItem(se.cmds.join("\n")));
    list_widget->setItem(index, 1, new QTableWidgetItem(se.keys));
    list_widget->setItem(index, 2, new QTableWidgetItem(getEnvName(se.env)));
    if (se.env == SCK_EDITOR)
    {
        QStringList when;
        if (se.left != "")
            when << "left: " + se.left;
        if (se.right != "")
            when << "right: " + se.right;
        list_widget->setItem(index, 3, new QTableWidgetItem(when.join("\n")));
    }
}

QString ShortcutWindow::getEnvName(ShortcutEnvironment env)
{
    switch (env)
    {
    case SCK_GLOBAL:
        return "全局";
    case SCK_EDITOR:
        return "编辑器";
    case SCK_DIR:
        return "目录";
    case SCK_OUTLINE:
        return "大纲";
    case SCK_EDITORS:
        return "所有";
    }
    return "未知";
}

void ShortcutWindow::slotFilterChanged(const QString &text)
{
    if (text.isEmpty())
    {
        for (int row = 0; row < shrt.length(); row++)
        {
            list_widget->setRowHidden(row, false);
        }
        return;
    }
    for (int row = 0; row < shrt.length(); row++)
    {
        ShortcutEntry& se = shrt[row];
        if (se.cmds.filter(text).length()>0
            || se.keys.contains(text)
            || se.left.contains(text)
            || se.right.contains(text))
            list_widget->setRowHidden(row, false);
        else
            list_widget->setRowHidden(row, true);
    }
}

void ShortcutWindow::slotAddNewShortcut()
{
    ShortcutEntry se;
    ShortcutInfoEditor editor(se, 0, this);
    auto result = editor.exec();
    if (result == QDialog::Accepted)
    {
        se.full_key = "\n\t<KEY>" + se.keys + "</KEY>"
             + "\n\t<CMD>" + se.cmds.join("\n") + "</CMD>"
             + "\n\t<ENV>" + getEnvName(se.env) + "</ENV>";
        shrt.append(se);
        list_widget->setRowCount(shrt.size());
        setRow(shrt.size()-1, se);
        list_widget->scrollToItem(list_widget->item(list_widget->rowCount()-1, 0));
        gd->shortcuts.saveToFile();
    }
}

void ShortcutWindow::slotCellClicked(int row, int col)
{
    ShortcutEntry& se = shrt[row];
    ShortcutInfoEditor editor(se, col, this);
    auto result = editor.exec();
    if (result == QDialog::Accepted)
    {
        se.full_key = "\t<KEY>" + se.keys + "</KEY>"
             + "\n\t<CMD>" + se.cmds.join("\n") + "</CMD>"
             + "\n\t<ENV>" + getEnvName(se.env) + "</ENV>";
        gd->shortcuts.saveToFile();
        setRow(row, se);
    }
}

void ShortcutWindow::slotShowMenu(QPoint p)
{
    menu->exec(cursor().pos());
}

void ShortcutWindow::actionEdit(int row, int col)
{
    slotCellClicked(row, col);
}

void ShortcutWindow::actionInsert(int row)
{
    ShortcutEntry se;
    ShortcutInfoEditor editor(se, 0, this);
    auto result = editor.exec();
    if (result == QDialog::Accepted)
    {
        se.full_key = "\n\t<KEY>" + se.keys + "</KEY>"
             + "\n\t<CMD>" + se.cmds.join("\n") + "</CMD>"
             + "\n\t<ENV>" + getEnvName(se.env) + "</ENV>";
        shrt.insert(row, se);
        list_widget->insertRow(row);
        setRow(row, se);
        list_widget->scrollToItem(list_widget->item(row, 0));
        gd->shortcuts.saveToFile();
    }
}

void ShortcutWindow::actionDelete(int row)
{
    shrt.removeAt(row);
    list_widget->removeRow(row);
    gd->shortcuts.saveToFile();
}
