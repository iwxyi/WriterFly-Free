#include "outlinegroup.h"

OutlineGroup::OutlineGroup(QWidget *parent) : QWidget(parent)
{
    used = false; // 没有被打开过，就不加载
    os = nullptr;
    editor_search_bar = nullptr;

    initFrame();
    initView(); // 应该放到 showEvent 里面初始化的……但是不这样编辑框位置不对啊
}

void OutlineGroup::initFrame()
{
    this->setMinimumSize(10, 50);
}

void OutlineGroup::initView()
{
    outline_switch_combo = new GeneralComboBox(this);
    if (us->round_view)
        menu_button = new WaterCircleButton(QPixmap(thm->iconPath("point_menu")), this);
    else
        menu_button = new PointMenuButton(this);
    divider_line = new QWidget(this);
    edit = new GeneralEditor(this, EDIT_TYPE_OUTLINE);

    menu_button->setShowAni(true);
    menu_button->setFixedSize(us->widget_size, us->widget_size);
    divider_line->setFixedHeight(1);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* main_vlayout = new QVBoxLayout;
    QHBoxLayout* title_hlayout = new QHBoxLayout;

    title_hlayout->addWidget(outline_switch_combo);
    title_hlayout->addWidget(menu_button);
    main_vlayout->addLayout(title_hlayout);
    main_vlayout->addWidget(divider_line);
    main_vlayout->addWidget(edit);

    title_hlayout->setSpacing(0);
    main_vlayout->setSpacing(0);
    title_hlayout->setMargin(2);
    main_vlayout->setMargin(0);
    this->setLayout(main_vlayout);

    initStyle();

    // 菜单
    menu = new QMenu("大纲操作", this);
    edit_action = new QAction(thm->icon("menu/modify"), "编辑", this);
    rename_action = new QAction(thm->icon("menu/rename"), "重命名", this);
    delete_action = new QAction(thm->icon("menu/delete"), "删除故事线", this);
    timeline_action = new QAction(thm->icon("menu/time"), "时间轴模式", this);
    tree_model_action = new QAction(thm->icon("menu/tree"), "树状模式", this);
    recycle_action = new QAction(thm->icon("menu/recycle"), "回收站", this);
    help_action = new QAction(thm->icon("menu/help"), "帮助", this);
    setActionsEnabled(false);
    edit_action->setEnabled(false);
    recycle_action->setEnabled(false);
    timeline_action->setEnabled(false);
    tree_model_action->setEnabled(false);
    menu->addAction(edit_action);
    menu->addAction(rename_action);
    menu->addAction(delete_action);
    menu->addAction(timeline_action);
    menu->addAction(tree_model_action);
    menu->addSeparator();
    menu->addAction(recycle_action);
    menu->addAction(help_action);
    menu_button->setMenu(menu);
    //menu_button->setStyleSheet("QPushButton::menu-indicator{image:none;}"); // 不显示下拉小三角

    // 链接信号槽
    connect(outline_switch_combo, SIGNAL(activated(int)), this, SLOT(slotSwitchOutline(int)));
    connect(edit, SIGNAL(textChanged()), this, SLOT(slotEditTextChanged()));
    connect(edit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotEditSlideChanged()));

    connect(edit_action, SIGNAL(triggered()), this, SLOT(actionEdit()));
    connect(rename_action, SIGNAL(triggered()), this, SLOT(actionRename()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(actionDelete()));
    connect(timeline_action, SIGNAL(triggered()), this, SLOT(actionTimeline()));
    connect(tree_model_action, SIGNAL(triggered()), this, SLOT(actionTreeModel()));
    connect(recycle_action, SIGNAL(triggered()), this, SLOT(actionRecycle()));
    connect(help_action, SIGNAL(triggered()), this, SLOT(actionHelp()));

    connect(edit, SIGNAL(signalShowSearchBar(QString, bool)), this, SLOT(slotShowEditorSearchBar(QString, bool)));
    connect(edit, SIGNAL(signalHidePanels()), this, SLOT(slotHidePanels()));
    connect(edit, &ChapterEditor::signalShowCardEditor, [=](CardBean *card) { emit signalShowCardEditor(card); });
    connect(edit, &ChapterEditor::signalAddNewCard, [=] { emit signalAddNewCard(); });

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));

    connect(ac, &SyncThread::signalOutlineDirectoryUpdated, [=](QString novel_name){
        if (novel_name == this->novel_name)
            slotLoadOutlineList(novel_name);
    });
    connect(ac, &SyncThread::signalOutlineUpdated, [=](QString novel_name, QString outline_name){
        if (novel_name == this->novel_name && outline_name == outline_name)
            edit->reloadContent();
    });
}

void OutlineGroup::initStyle()
{
    // setStyleSheet("QWidget{background: transparent;}");

    outline_switch_combo->setFixedHeight(us->widget_size);

    thm->setWidgetStyleSheet(divider_line, "sidebar_dividing_line");

    // edit的margin会导致文字输入跳动，如果开启动画的话必须要关掉
    thm->setWidgetStyleSheet(edit, "outline_editor");
}

void OutlineGroup::initData()
{
    used = true;
    os = nullptr;
    log("大纲：initData : "+novel_name+"/"+outline_name+".txt");
    QString recent_open_novel = us->getStr("recent/novel");
    QString recent_open_outline = us->getStr("recent/outline");

    if (!isFileExist(rt->NOVEL_PATH+recent_open_novel))
        return ; // 文件不存在，全部退出

    loadOutlineList(recent_open_novel);
}

void OutlineGroup::updateUI()
{
    if (menu_button != nullptr)
        menu_button->setIconColor(us->accent_color);

    QString border_radius = "1";
    QString padding_horizone = "";
    if (us->round_view) // 圆角效果
    {
        border_radius = QString("%1").arg(outline_switch_combo->height()/2-5); // 增加圆角
        padding_horizone = "padding-left:10px; padding-right:10px;"; // // 增加文字与左右的边距
    }
    QString combo_style = "QComboBox{ margin:5px; margin-left: 13px; padding: 4px; border:none; border-radius:"+border_radius+"px; min-width:4em; background:rgba(255,255,255,0%);"+padding_horizone+"}\
            QComboBox:hover{background:"+us->getOpacityColorString(us->accent_color, 64)+";}\
            QComboBox::drop-down{subcontrol-origin:padding; subcontrol-position:top right; border-left-width:0px;border-left-color:darkgray; border-left-style:solid; border-top-right-radius:3px; border-bottom-right-radius:3px;}\
            QComboBox::down-arrow{image: url(:/icons/arrow_down_combo); }";
    outline_switch_combo->setStyleSheet(combo_style);

    thm->setWidgetStyleSheet(this->menu, "menu");
}

/**
 * 加载某本书的大纲列表
 * @param novel_name
 * @param outline_name
 */
void OutlineGroup::loadOutlineList(QString novel_name, QString outline_name)
{
    log("大纲：loadOutlineList : "+novel_name+"/"+outline_name);
    this->novel_name = novel_name;
//    this->outline_name = "";
//    edit->setText("");

    outline_switch_combo->clear();
    setActionsEnabled(false);
    if (novel_name.isEmpty())
    {
        this->outline_name = "";
        edit->setText("");
        log("加载空作品名");
        return ;
    }

    QString outlines_path = rt->NOVEL_PATH + novel_name + "/outlines/";
    QDir outlines_dir(outlines_path);
    if (!outlines_dir.exists())
    {
        err("加载大纲错误", "文件夹：" + outlines_path + " 不存在");
        ensureDirExist(outlines_path);
        //QMessageBox::warning(this, "加载大纲错误", "文件夹：" + outlines_path + " 不存在");
        //return ;
    }

    // 列出目录下所有txt文件
    QStringList nameFilters;
    nameFilters << "*.txt";
    QStringList files = outlines_dir.entryList(nameFilters);
    QStringList outlines;
    for (int i = 0; i < files.size(); i++)
        outlines << files.at(i).chopped(4);
    outlines << "创建故事线";

    // 设置大纲设置文件
    if (os != nullptr)
        delete os;
    os = new Settings(rt->NOVEL_PATH+novel_name+"/outline.ini");

    outline_switch_combo->addItems(outlines);
    outline_switch_combo->setCurrentIndex(outlines.size()-1); // 设置为创建
    if (!outline_name.isEmpty()) // 手动加载的大纲名字
    {
        int index = outlines.indexOf(outline_name);
        if (index > -1)
        {
            log("加载大纲 确定", novel_name + "/" + outline_name);
            outline_switch_combo->setCurrentIndex(index);
            if (this->novel_name != novel_name || this->outline_name != outline_name) // 如果一样，可能是自动新建保存，就不再读取了
                loadOutlineText(novel_name, outline_name);
            os->setVal("recent/novel", novel_name);
            os->setVal("recent/outline", outline_name);
            setActionsEnabled(true);
        }
        else
        {
            QMessageBox::warning(this, "大纲不存在", "无法打开大纲 " + novel_name + "/" + outline_name + " ！");
            log("无法打开大纲 " + novel_name + "/" + outline_name + " ！");
        }
    }
    else
    {
        this->outline_name = "";
        edit->setText("");
        // 根据大纲设置文件打开上次使用的大纲，一般是初次打开
        QString recent_open_outline = os->getStr("recent/outline");
        if (!recent_open_outline.isEmpty())
        {
            log("加载大纲 历史", novel_name + "/" + recent_open_outline);
            loadOutlineText(recent_open_outline);
            int index = outlines.indexOf(recent_open_outline);
            if (index > -1)
            {
                outline_switch_combo->setCurrentIndex(index);
                setActionsEnabled(true);
            }
        }
        else
        {
            log("加载空列表（无记录）");
        }
    }
}

/**
 * 加载某本书某个大纲的文字，并且根据大纲设置文件来读取进度
 * @param novel_name
 * @param outline_name
 */
void OutlineGroup::loadOutlineText(QString novel_name, QString outline_name)
{
    log("大纲：loadOutlineText : "+novel_name+"/"+outline_name+".txt");
    this->outline_name = outline_name;
    outline_path = rt->NOVEL_PATH + novel_name + "/outlines/" + outline_name + ".txt";
    /*QString content = readTextFile(outline_path);
    edit->setText(content);*/
    edit->readFromFile(outline_path);
    setActionsEnabled(true);
    QTimer::singleShot(100, [=]{
        edit->setFocus();
    });

    int progress = os->getInt("progress/"+outline_name);
    if (progress)
        edit->verticalScrollBar()->setSliderPosition(progress);
}

void OutlineGroup::loadOutlineText(QString outline_name)
{
    if (novel_name.isEmpty())
        return ;
    loadOutlineText(novel_name, outline_name);
}

QString OutlineGroup::inputName(QString def)
{
    bool isOk;
    QString new_name = def, tip = "请输入故事线名字（例如：主线、感情线）";

    while (1)
    {
        new_name = QInputDialog::getText(this, "创建大纲故事线", tip, QLineEdit::Normal, new_name, &isOk);
        if (!isOk || new_name.isEmpty()) return "";

        // 判断是否包含特殊字符
        QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            if (new_name.contains(cs[i]))
            {
                tip = tr("故事线包含特殊字符【%1】，请重新输入").arg(cs[i]);
                //QMessageBox::information(NULL, tr("创建故事线失败"), tr("包含特殊字符【%1】啦！").arg(cs[i]));
                break;
            }
        }
        if (i == 10) break;
    }

    return new_name.trimmed();
}

void OutlineGroup::setActionsEnabled(bool e)
{
    //edit_action->setEnabled(e);
    rename_action->setEnabled(e);
    delete_action->setEnabled(e);
    //timeline_action->setEnabled(e);
}

/**
 * 修改小说名字
 * @param new_name
 */
void OutlineGroup::slotNovelNameChanged(QString old_name, QString new_name)
{
    log("大纲：切换作品 " + old_name + ">>" + new_name);
    novel_name = new_name;

    if (os != nullptr)
    {
        delete os;
        os = new Settings(rt->NOVEL_PATH+novel_name+"/outline.ini");
    }
}

void OutlineGroup::slotNovelDeleted()
{
    slotLoadOutlineList("");
}

void OutlineGroup::paintEvent(QPaintEvent *event)
{

    return QWidget::paintEvent(event);
}

void OutlineGroup::enterEvent(QEvent *event)
{
    menu_button->showForeground2();

    return QWidget::enterEvent(event);
}

void OutlineGroup::leaveEvent(QEvent *event)
{
    menu_button->hideForeground();

    return QWidget::leaveEvent(event);
}

void OutlineGroup::resizeEvent(QResizeEvent *event)
{

    return QWidget::resizeEvent(event);
}

void OutlineGroup::keyPressEvent(QKeyEvent *event)
{
    auto modifiers = event->modifiers();
    QStringList cmds = gd->shortcuts.getShortutCmds(SCK_GLOBAL, modifiers, static_cast<Qt::Key>(event->key()));
    if (cmds.size() > 0)
    {
        foreach (QString cmd, cmds)
        {
            execCommand(cmd);
        }
    }
    else
    {

    }

    if (cmds.size() > 0)
        return ;
    return QWidget::keyPressEvent(event);
}

void OutlineGroup::showEvent(QShowEvent *event)
{
    if (!used)
    {
        initData();
    }
    return QWidget::showEvent(event);
}

/**
 * 初始化事件，加载某本书的大纲列表
 * @param novel_name
 */
void OutlineGroup::slotLoadOutlineList(QString novel_name)
{
    if (this->novel_name == novel_name)
        return ;

    loadOutlineList(novel_name);
}

/**
 * 切换大纲事件，读取某本书某个大纲的文字
 * @param outline_name
 */
void OutlineGroup::slotLoadOutlineText(QString outline_name)
{
    if (novel_name == "")
        return ;

    loadOutlineText(novel_name, outline_name);
}

/**
 * 点击下拉列表框切换事件
 * @param index
 */
bool OutlineGroup::slotSwitchOutline(int index)
{
    if (novel_name == "")
        return false;

    if (index == outline_switch_combo->count()-1) // 新增大纲
    {
        // 输入书名
        QString new_name = inputName();
        if (new_name.isEmpty())
            return false;

        // 创建完大纲
        // 检验重复大纲
        if (isFileExist(rt->NOVEL_PATH+novel_name+"/outlines/"+new_name+".txt"))
        {
            QMessageBox::information(this, tr("创建大纲失败"), tr("已经有同名大纲《%1》啦！").arg(new_name));
            return false;
        }

        outline_path = rt->NOVEL_PATH+novel_name+"/outlines/"+new_name+".txt";
        ensureFileExist(outline_path);
        os->setVal("recent/outline", new_name); // 保存到设置中
        loadOutlineList(novel_name, new_name); // 全部刷新列表，然后读取最近的那个
    }
    else
    {
        QString name = outline_switch_combo->itemText(index);
        if (name == outline_name) // 选的是同一个
            return false;
        os->setVal("recent/outline", name); // 保存到设置中
        loadOutlineList(novel_name, name); // 全部刷新列表，然后读取最近的那个
    }
    return true;
}

/**
 * 大纲文本改变事件
 */
void OutlineGroup::slotEditTextChanged()
{
    // 没有作品
    if (novel_name.isEmpty() || (outline_name.isEmpty() && edit->toPlainText().trimmed().isEmpty()))
    {
        log("故事线 无处保存");
        return ;
    }

    log("保存故事线：" + novel_name + "/" + outline_name + ".txt    " + QString::number(edit->toPlainText().length()));

    // 没有大纲，自动创建一个
    if (outline_name.isEmpty() && !edit->toPlainText().isEmpty())
    {
        QString outlines_path = rt->NOVEL_PATH + novel_name + "/outlines/";
        QDir outlines_dir(outlines_path);
        if (!outlines_dir.exists())
        {
            err("加载大纲错误", "文件夹：" + outlines_path + " 不存在");
            ensureDirExist(outlines_path);
            //QMessageBox::warning(this, "加载大纲错误", "文件夹：" + outlines_path + " 不存在");
            //return ;
        }

        // 列出目录下所有txt文件
        QStringList nameFilters;
        nameFilters << "*.txt";
        QStringList files = outlines_dir.entryList(nameFilters);
        if (files.size() == 0) // 没有大纲，新建一个
        {
            log("自动创建主线");
            outline_name = "主线";
        }
        else // 已经有了，但是没有选中
        {
            QString new_name = "新故事线";
            if (isFileExist(outlines_path + new_name + ".txt"))
            {
                int index = 1;
                while (isFileExist(outlines_path + new_name + QString::number(index) + ".txt"))
                    index++;
                new_name += QString::number(index);
                log("自动创建 新故事线"+QString::number(index));
            }
            outline_name = new_name;
        }
        outline_path = rt->NOVEL_PATH+novel_name+"/outlines/"+outline_name+".txt";
        writeTextFile(outline_path, edit->toPlainText());
        loadOutlineList(novel_name, outline_name);
        //loadOutlineText(outline_name);
        return ;
    }

    writeTextFile(outline_path, edit->toPlainText());
}

/**
 * 大纲滚动进度改变事件
 */
void OutlineGroup::slotEditSlideChanged()
{
    if (novel_name.isEmpty() || outline_name.isEmpty())
        return ;
    os->setVal("progress/"+outline_name, edit->verticalScrollBar()->sliderPosition());
}

void OutlineGroup::slotLocateOutline(QString novel_name, QString chapter_content)
{
    if (!used) initData();

    int start = -1, end = -1;
    if (us->getBool("us/locate_outline_mode", 0) == 1)
        NovelTools::locateOutline(edit->toPlainText(), chapter_content, start, end);
    else
        LocateMultiple::locateOutlineByChapter(gd->participle, gd->lexicons.getStopWords(), chapter_content, edit->toPlainText(), start, end);

    if (start >= 0 && end >= 0)
    {
        QTextCursor tc = edit->textCursor();
        tc.setPosition(start);
        tc.setPosition(end, QTextCursor::KeepAnchor);
        edit->setTextCursor(tc);
    }
}

bool OutlineGroup::execCommand(QString command)
{
    QStringList args;
    if (command.contains(" "))
    {
        int pos = command.indexOf(" ");
        QString arg = command.right(command.length() - pos - 1).trimmed();
        command = command.left(pos);

        args = arg.split(",", QString::SkipEmptyParts);
        for (int i = 0; i < args.size(); i++)
        {
            if (args.at(i) !=  " " && (args.at(i).startsWith(" ") || args.at(i).endsWith(" ")))
                args[i] = args.at(i).trimmed(); // 去掉首尾空（除非本身就是空的）
        }
    }
    return execCommand(command, args);
}

bool OutlineGroup::execCommand(QString command, QStringList args)
{
    if (command == "新建故事线")
    {
        if (!used) initData(); // 先初始化，否则没有数据
        return slotSwitchOutline(outline_switch_combo->count()-1);
    }
    else if (command == "切换故事线")
    {
        if (!used) initData();
        if (args.size() > 0)
        {
            QString name = args.at(0);
            for (int i = 0; i < outline_switch_combo->count(); i++)
            {
                if (outline_switch_combo->itemText(i) == name)
                {
                    return slotSwitchOutline(i);
                }
            }
            QMessageBox::warning(this, "切换故事线失败", "<参数1>故事线未找到");
        }
        else
        {
            QMessageBox::warning(this, "切换故事线失败", "<参数1>没有指定故事线名字");
        }
        return false;
    }
    else if (command == "编辑故事线")
    {
        if (!used) initData();
        actionEdit();
    }
    else if (command == "重命名故事线")
    {
        if (!used) initData();
        actionRename();
    }
    else if (command == "删除故事线")
    {
        if (!used) initData();
        actionDelete();
    }
    else if (command == "故事线时间轴模式")
    {
        if (!used) initData();
        actionTimeline();
    }
    else if (command == "故事线树状模式")
    {
        if (!used) initData();
        actionTreeModel();
    }
    else if (command == "故事线回收站")
    {
        if (!used) initData();
        actionRecycle();
    }
    else if (command == "故事线帮助")
    {
        if (!used) initData();
        actionHelp();
    }
    else
        return false;
    QTimer::singleShot(100, [=]{
        setFocus();
    });
    return true;
}

void OutlineGroup::slotShowEditorSearchBar(QString def, bool rep)
{
    if (editor_search_bar == nullptr)
    {
        editor_search_bar = new EditorSearchWidget(this);
        //connect(editor_search_bar, SIGNAL(signalSearchPrev(QString)), this, SLOT(slotEditorSearchPrev(QString)));
        //connect(editor_search_bar, SIGNAL(signalSearchNext(QString)), this, SLOT(slotEditorSearchNext(QString)));
        connect(editor_search_bar, SIGNAL(signalClosed()), this, SLOT(slotHideEditorSearchBar()));
    }
    editor_search_bar->setAimEditor(edit);
    editor_search_bar->setText(def);
    editor_search_bar->showReplace(rep);

    if (editor_search_bar->isHidden())
    {
        // 显示动画
        editor_search_bar->show();
        //editor_search_bar->move(0, 0);
        int margin = 10;
        QPoint sp(geometry().width(), margin);
        QPoint ep(geometry().width()-editor_search_bar->width()-margin, margin);
        QPropertyAnimation* ani = new QPropertyAnimation(editor_search_bar, "pos");
        ani->setStartValue(sp);
        ani->setEndValue(ep);
        ani->setDuration(200);
        ani->setEasingCurve(QEasingCurve::OutCubic);
        ani->start();
        connect(ani, &QPropertyAnimation::finished, [=]{
            ani->deleteLater();
        });
        editor_search_bar->setFocus();

//        editor_search_bar->showButtonAnimation();
    }
}

void OutlineGroup::slotHideEditorSearchBar()
{
    edit->setSearchKey("");

    // 隐藏动画
    int margin = 10;
    QPoint sp(editor_search_bar->pos());
    QPoint ep(geometry().width()+10, margin);
    QPropertyAnimation* ani = new QPropertyAnimation(editor_search_bar, "pos");
    ani->setStartValue(sp);
    ani->setEndValue(ep);
    ani->setDuration(200);
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=]{
        editor_search_bar->hide();
        ani->deleteLater();
    });

    QTimer::singleShot(100, [=]{
        edit->setFocus();
    });
}

void OutlineGroup::slotHidePanels()
{
    if (editor_search_bar != nullptr && !editor_search_bar->isHidden())
    {
        slotHideEditorSearchBar();
        return ;
    }
}

void OutlineGroup::actionEdit()
{

}

void OutlineGroup::actionRename()
{
    if (novel_name.isEmpty() || outline_name.isEmpty())
        return ;
    QString new_name = inputName(outline_name);
    if (new_name.isEmpty()) // 取消输入
        return ;
    if (outline_name == new_name) // 没有修改
        return ;
    QString outline_path = rt->NOVEL_PATH+novel_name+"/outlines/"+new_name+".txt";
    if (isFileExist(outline_path))
    {
        QMessageBox::information(this, "提示", "故事线【"+new_name+"】已存在，请换一个名字");
        return ;
    }

    deleteFile(rt->NOVEL_PATH+novel_name+"/outlines/"+outline_name+".txt");
    this->outline_name = new_name;
    this->outline_path = outline_path;
    writeTextFile(this->outline_path, edit->toPlainText());
    loadOutlineList(this->novel_name, new_name);
}

void OutlineGroup::actionDelete()
{
    if (novel_name.isEmpty() || outline_name.isEmpty())
        return ;
    QString outline_path = rt->NOVEL_PATH+novel_name+"/outlines/"+outline_name+".txt";
    if (!isFileExist(outline_path))
        return ;
    if (QMessageBox::information(this, "警告",
                                 "是否要删除故事线：" + novel_name + "/" + outline_name + "\n\n目前不支持回收站功能，删除后将无法找回",
                                 QMessageBox::Ok | QMessageBox::No) == QMessageBox::Ok)
    {
        outline_name = "";
        deleteFile(outline_path);
        edit->removeOpeningFile();
        os->setVal("recent/outline", "");
        loadOutlineList(novel_name);
    }
}

void OutlineGroup::actionTimeline()
{

}

void OutlineGroup::actionTreeModel()
{
    TreeOutlineWindow* tow = new TreeOutlineWindow(edit->toPlainText(), this);
    tow->exec();
    edit->reloadContent();
}

void OutlineGroup::actionRecycle()
{

}

void OutlineGroup::actionHelp()
{
    QString tips;

    tips = "为每部作品创建多条故事线，在写作的同时进行参考\n自动保存修改内容、参考进度\n可以写故事线，也可用来存灵感、笔记、便笺等\n\n";
    tips += "提供小说默认故事线集，分别有：\n主线、辅线、副线、支线、\n事件线、感情线、身世线、事业线、伙伴线、\n明线、暗线。";

    QMessageBox mb("大纲 帮助", tips,
                   QMessageBox::Question,
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape,
                   QMessageBox::NoButton);
    mb.setStyleSheet("QMessageBox{background:" + us->getColorString(us->mainwin_bg_color) + ";}");
    mb.setButtonText(QMessageBox::Yes, "创建默认故事线集");
    mb.setButtonText(QMessageBox::No, "取消");
    if (mb.exec() == QMessageBox::Yes)
    {
        if (novel_name.isEmpty())
            QMessageBox::warning(this, "创建故事线集失败", "请先在目录上选择一本作品");
        else
            createDefaultStories();
    }
}

void OutlineGroup::createDefaultStories()
{
    QStringList names;
    QStringList tips;

    names << "主线"
          << "辅线"
          << "副线"
          << "支线"
          << "事件线"
          << "感情线"
          << "身世线"
          << "事业线"
          << "伙伴线"
          << "明线"
          << "暗线";
    tips << "主要情节发展变化的脉络走向、主要主角的发展方向（即情节的开始、发展、高潮、结尾）\n　　开头？\n　　结局是怎样的？\n　　一句话描述整部小说的内容？\n　　贯穿整部小说的线索是什么？"
         << "各种情节、说明等等的穿插，与主线一起推动情节的发展。"
         << "次要情节的发展脉络、相关人物事情的发展（主线的分支情节）。"
         << "详细的副线（可以理解为支线组合成副线，副线组合成主线）。"
         << "详细的主线副线，通过一件件事件来丰富主线，让小说更丰满，人物的性格、故事的剧情更丰富。"
         << "各种人物相识相知以及态度的变化。"
         << "主角及主要任务的身世及其影响。"
         << "主角擅长的技能，炼药、阵法等。"
         << "主角身边的朋友、敌人，或者有其他各种因果的角色。"
         << "由人物活动或事件发展所直接呈现出来的线索。"
         << "暗不可明说，请自行领悟。";

    for (int i = 0; i < names.size(); i++)
    {
        QString path = rt->NOVEL_PATH+novel_name+"/outlines/"+names.at(i)+".txt";
        if (!isFileExist(path)) // 避免把已经存在的给覆盖了
            writeTextFile(path, tips.at(i));
    }
    loadOutlineList(this->novel_name, this->outline_name/*避免已有的被清空，这个参数不能去掉*/);
}
