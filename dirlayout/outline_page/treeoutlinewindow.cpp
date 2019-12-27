#include "treeoutlinewindow.h"

TreeOutlineWindow::TreeOutlineWindow(QString content, QWidget *parent) : QDialog(parent)
{
    initView();
    initData(content);
}

void TreeOutlineWindow::initView()
{
    tree = new QTreeWidget(this);

    QVBoxLayout* main_vlayout = new QVBoxLayout;
    main_vlayout->addWidget(tree);
    setLayout(main_vlayout);
}

void TreeOutlineWindow::initData(QString content)
{
    paras = content.split("\n", QString::SkipEmptyParts);
    levels.clear();

    // 去掉空白的
    for (int i = 0; i < paras.length(); i++)
    {
        if (isAllBlank(paras.at(i)))
            paras.removeAt(i--);
    }

    //添加层级
    int min = 0x3f3f3f3f;
    for (int i = 0; i < paras.length(); i++)
    {
        QString& para = paras[i];
        int len = para.length(), level = 0;
        while (level < len && isBlankChar2(para.mid(level, 1)))
            level++;
        para = para.right(len-level);
        levels.append(level);

        if (level < min)
            min = level;
    }

    // 缩减所有层级，使得最小为1
    if (min > 0 && min < 0x3f3f3f3f)
    {
        for (int i = 0; i < levels.length(); i++)
            levels[i] -= min;
    }

    // 遍历每一行，设置到对应的层级
    for (int i = 0; i < paras.length(); i++)
    {
        QString para = paras.at(i);
        int level = levels.at(i);

        if (level == 0)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(tree, QStringList{para});
        }
    }
}

void TreeOutlineWindow::addNextLevel(int index)
{

}
