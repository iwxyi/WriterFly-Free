#include "thememanagewindow.h"

ThemeManageWindow::ThemeManageWindow(QWidget *parent) : MyDialog(parent)
{
	initView();
    initMenu();
}

void ThemeManageWindow::initView()
{
    if (us->round_view)
        save_btn = new WaterFloatButton("保存当前主题", this);
    else
        save_btn = new InteractiveButtonBase("保存当前主题", this);
	theme_list = new QListWidget(this);

    save_btn->setFixedForeSize(true);
    save_btn->setBorderColor(QColor(0, 0, 0, 0));
    QFont font = save_btn->font();
    QFontMetrics fm(font);
    save_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    theme_list->setIconSize(QSize(128, 72));
    theme_list->setViewMode(QListView::IconMode);
    theme_list->setResizeMode(QListWidget::Adjust); // 列数自动适应宽度
    theme_list->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(theme_list,SIGNAL(customContextMenuRequested (const QPoint&)),this,SLOT(OnListContextMenu(const QPoint&)));
    connect(theme_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemClicked(QListWidgetItem*)));

	QVBoxLayout* main_vlayout = main_layout;
    {
        QHBoxLayout* save_hlayout = new QHBoxLayout;
        save_hlayout->addWidget(save_btn);
        save_hlayout->addStretch(1);
        save_hlayout->setMargin(5);
        main_vlayout->addLayout(save_hlayout);
    }
	main_vlayout->addWidget(theme_list);
    theme_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(save_btn, SIGNAL(clicked()), this, SLOT(saveCurrentTheme()));

    if (this->width() < 600)
        setMinimumWidth(600);
    if (this->height() < 400)
        setMinimumHeight(400);
}

void ThemeManageWindow::showEvent(QShowEvent * e)
{
    thm->setWidgetStyleSheet(theme_list, "theme_list");
    thm->setWidgetStyleSheet(menu, "menu");
    theme_list->setPalette(QPalette());

    save_btn->setTextColor(us->global_font_color);
    save_btn->setBgColor(us->getOpacityColor(us->accent_color, 128));
    save_btn->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);

    initList();
    return MyDialog::showEvent(e);
}

void ThemeManageWindow::initList()
{
    theme_list->clear();
	ensureDirExist(rt->THEME_PATH);
    QDir all_dir(rt->THEME_PATH);
    QStringList theme_paths = all_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QString current_theme = thm->getThemeName();

    // 遍历每个主题文件夹
    foreach (QString path, theme_paths)
    {
        path = rt->THEME_PATH + path;
        QDir dir(path);
        QString theme_name = dir.dirName();
        QString cover = "";
        if (isFileExist(path + "/cover.png"))
            cover = path + "/cover.png";
        else if(isFileExist(path + "/cover.jpg"))
            cover = path + "/cover.jpg";
        else
            cover = thm->iconPath("theme");

        QListWidgetItem* item = new QListWidgetItem(QIcon(cover), theme_name, theme_list);

        if (theme_name == current_theme) // 选中当前主题
            theme_list->setCurrentItem(item);
    }

}

void ThemeManageWindow::initMenu()
{
	menu = new QMenu("主题");
    locate_action = new QAction(thm->icon("menu/locate"), "打开位置", this);
    rename_action = new QAction(thm->icon("menu/rename"), "重命名", this);
    delete_action = new QAction(thm->icon("menu/delete"), "删除主题", this);

	menu->addAction(locate_action);
    menu->addAction(rename_action);
    menu->addAction(delete_action);

    connect(locate_action, SIGNAL(triggered()), this, SLOT(actionOpenExplore()));
    connect(rename_action, SIGNAL(triggered()), this, SLOT(actionRename()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(actionDelete()));

}

/**
 * 获取随机主题名字
 * 目前只是按序号来的
 * 但是以后可能会根据主色调的色彩来判断颜色
 */
QString ThemeManageWindow::getRandomThemeName()
{
    QString def_name = "主题";

    // TODO: 将主色调、点缀色套入色彩表，获取合适的主题名称


    // 判断主题序号
    if (isFileExist(rt->THEME_PATH + def_name))
    {
        int index = 1;
        while (isFileExist(rt->THEME_PATH + def_name + QString::number(++index)));
        def_name += QString::number(index);
    }

    return def_name;
}

void ThemeManageWindow::slotItemClicked(QListWidgetItem* item)
{
    QString name = item->text();
    if (!isFileExist(rt->THEME_PATH + name))
    {
        QMessageBox::warning(this, "加载主题失败", "主题【" + name + "】不存在");
        return ;
    }

    if (name == NIGHT_THEME) // 夜间模式
    {
        // thm->saveCurrentTheme(USER_THEME); // 已经在主题中设置了，不能重复设置（否则会删除素材）
    }
    else if (theme_list->count() == 1) // 只有一个主题，自动备份，免得用户后悔无法备份
    {
        if (QMessageBox::information(this, "提示", "是否保存当前用户自定义主题？\n\n注意：若不保存，将无法切换回来", "勇往直前不保存", "小心翼翼先保存", 0, 1) == 1)
            thm->saveCurrentTheme("自定义");
    }

    thm->loadTheme(name);

    showEvent(new QShowEvent());
}

void ThemeManageWindow::OnListContextMenu(const QPoint& point)
{
    menu->exec(theme_list->mapToGlobal(point));
}

void ThemeManageWindow::saveCurrentTheme()
{
    bool ok;
    QString name = QInputDialog::getText(this, "取名", "取个适合主题的名字吧？", QLineEdit::Normal, getRandomThemeName(), &ok);
    if (!ok || name.isEmpty()) return ;
    if (name != fnEncode(name))
    {
        QMessageBox::warning(this, "名字不正规", "主题【" + name + "】包含特殊字符，不受系统支持，请更换名字~");
        return ;
    }
    if (isFileExist(rt->THEME_PATH + name))
    {
        QMessageBox::warning(this, "主题已存在", "主题【" + name + "】已存在，请更换名字");
        return ;
    }

    // 保存主题
    thm->saveCurrentTheme(name);

    initList();
}

void ThemeManageWindow::actionOpenExplore()
{
    if (theme_list->currentItem() == nullptr)
        return ;
    QString name = theme_list->currentItem()->text();
    QDesktopServices::openUrl(QUrl::fromLocalFile(rt->THEME_PATH+name));
}

void ThemeManageWindow::actionRename()
{
    if (theme_list->currentItem() == nullptr)
        return ;
    QString old_name = theme_list->currentItem()->text();
    if (old_name == USER_THEME)
    {
        QMessageBox::warning(this, "禁止此操作", "【自定义】保存切换夜间模式前的主题，更改后日间模式将丢失。");
        return ;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "取名", "取个适合主题的名字吧？", QLineEdit::Normal, old_name, &ok);
    if (!ok || name.isEmpty()) return ;
    if (name != fnEncode(name))
    {
        QMessageBox::warning(this, "名字不正规", "主题【" + name + "】包含特殊字符，不受系统支持，请更换名字~");
        return ;
    }
    else if (name == NIGHT_THEME)
    {
        QMessageBox::warning(this, "禁止此操作", "【夜间模式】禁止修改！");
        return ;
    }
    if (isFileExist(rt->THEME_PATH + name))
    {
        QMessageBox::warning(this, "主题已存在", "主题【" + name + "】已存在，请更换名字");
        return ;
    }

    QDir dir(rt->THEME_PATH + old_name);
    if (!dir.exists()) return ;
    dir.rename(rt->THEME_PATH+old_name, rt->THEME_PATH+name);

    theme_list->currentItem()->setText(name);

    if (thm->getThemeName() == old_name) // 如果当前主题修改名字了
    {
        thm->theme_name = name;
        us->setVal("recent/theme_name", name);
    }
}

void ThemeManageWindow::actionDelete()
{
    if (theme_list->currentItem() == nullptr)
        return ;

    QString name = theme_list->currentItem()->text();
    if (name == USER_THEME)
    {
        QMessageBox::warning(this, "禁止此操作", "【自定义】保存切换夜间模式前的主题，更改后日间模式将丢失。");
        return ;
    }
    else if (name == NIGHT_THEME)
    {
        QMessageBox::warning(this, "禁止此操作", "【夜间模式】禁止删除！");
        return ;
    }
    if (QMessageBox::warning(this, "警告", "是否删除主题【" + name + "】？\n该删除无法恢复，需手动配置主题", "删除", "取消", 0, 1) != 0)
        return;

    deleteDir(rt->THEME_PATH+name);
    QListWidgetItem* item = theme_list->currentItem();
    theme_list->removeItemWidget(item);
    delete item;
}
