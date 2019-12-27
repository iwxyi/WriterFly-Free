#include "noveldirgroup.h"

NovelDirGroup::NovelDirGroup(QWidget * parent)
        : QWidget(parent)
{
    novel_new_name_temp = "";
    novel_had_deleted_temp = false;

    initView();
    initData();

    connect(ac, &SyncThread::signalNovelAdded, [=](QString novel_name){
        readBookList(getListView()->getNovelName());
    });
    connect(ac, &SyncThread::signalDirectoryUpdated, [=](QString novel_name){
        if (novel_name == getListView()->getNovelName())
            getListView()->readNovel(novel_name);
    });
}

void NovelDirGroup::initView()
{
    this->setMinimumSize(10, 50);

    book_switch_combo = new GeneralComboBox(this);
    novel_dir_listview = new NovelDirListView(this);
    if (us->round_view)
    {
        add_roll_button = new WaterCircleButton(QPixmap(":/icons/add_roll"), this);
        add_chapter_button = new WaterFloatButton(tr("添加新章"), this);
        info_button = new WaterCircleButton(QPixmap(":/icons/info"), this);

        add_roll_button->setShowAni(true);
        info_button->setShowAni(true);
        add_roll_button->setFixedSize(us->widget_size, us->widget_size);
        info_button->setFixedSize(us->widget_size, us->widget_size);
        add_chapter_button->setFixedHeight(us->widget_size*3/4);
        info_button->setLeaveAfterClick(true); // 点击失去焦点

        add_chapter_button->delayShowed(1500, QPoint(0, -1));
    }
    else
    {
        add_roll_button = new InteractiveButtonBase(tr("新卷"), this);
        add_chapter_button = new InteractiveButtonBase(tr("新章"), this);
        info_button = new InteractiveButtonBase(tr("一览"), this);

        divider_line = new QWidget(this); // 分割线
        divider_line->setFixedHeight(1);
        divider_line->setAttribute(Qt::WA_LayoutUsesWidgetRect);
        thm->setWidgetStyleSheet(divider_line, "sidebar_dividing_line");
    }

    add_roll_button->setToolTip("添加新的分卷");
    info_button->setToolTip("作品信息一览");

    QVBoxLayout *vlayout = new QVBoxLayout();
    btn_layout = new QHBoxLayout();
    btn_layout->addWidget(add_roll_button);
    btn_layout->addWidget(add_chapter_button);
    btn_layout->addWidget(info_button);
    QRect rect = btn_layout->geometry();
    //rect.setHeight(add_roll_button->getMiniHeight());
    rect.setHeight(add_roll_button->height());
    btn_layout->setGeometry(rect);
    btn_layout->setMargin(4);
    btn_layout->setSpacing(4);

    vlayout->addWidget(book_switch_combo);
    if (!us->round_view)
        vlayout->addWidget(divider_line);
    vlayout->addWidget(novel_dir_listview);
    vlayout->addLayout(btn_layout);
    vlayout->setSpacing(0);
    vlayout->setMargin(0); // 四边边缘的间距

    this->setLayout(vlayout);

    connect(book_switch_combo, SIGNAL(activated(int)), this, SLOT(switchBook(int)));
    connect(add_roll_button, SIGNAL(clicked(bool)), this, SLOT(slotAddRollButtonClicked()));
    connect(add_chapter_button, SIGNAL(clicked(bool)), this, SLOT(slotAddChapterButtonClicked()));
    //connect(add_roll_button, SIGNAL(clicked(bool)), novel_dir_listview, SLOT(slotAddRoll()));
    //connect(add_chapter_button, SIGNAL(clicked(bool)), novel_dir_listview, SLOT(slotAddChapter()));
    connect(novel_dir_listview, SIGNAL(signalCurrentChanged(const QModelIndex, const QModelIndex)), this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect(info_button, SIGNAL(clicked(bool)), this, SLOT(slotInfoClicked()));

    initStyle();

    // 判断光标默认在不在范围内
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    if (!rect.contains(pos))
    {
        /*add_roll_button->hide();
        add_chapter_button->hide();
        info_button->hide();*/
        leaveEvent(nullptr);
    }

    book_switch_combo->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    novel_dir_listview->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    add_roll_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    add_chapter_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    info_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

void NovelDirGroup::initStyle()
{
    // setStyleSheet("background:transparent;");
    QString trans("QPushButton,QListView{background: transparent; border:none;}");
    novel_dir_listview->setStyleSheet(trans);
    add_chapter_button->setBorderWidth(0);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void NovelDirGroup::updateUI()
{
    if (us->round_view) // 圆角
    {
        add_chapter_button->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);
        add_chapter_button->setTextColor(us->accent_color);
        add_chapter_button->setBorderColor(us->accent_color);

        add_roll_button->setIconColor(us->getNoOpacityColor(us->accent_color));
        info_button->setIconColor(us->getNoOpacityColor(us->accent_color));
    }
    else
    {
        add_chapter_button->setIconColor(us->getNoOpacityColor(us->accent_color));
        add_chapter_button->setTextColor(us->global_font_color);

        add_roll_button->setTextColor(us->global_font_color);
        info_button->setTextColor(us->global_font_color);
    }
    thm->setWidgetStyleSheet(add_chapter_button, "add_chapter_button");

    QString combo_style;
    if (us->round_view) // 圆角：一个大圈圈把名字圈起来
    {
        QString border_radius = QString::number(book_switch_combo->height()/2-2);
        thm->setWidgetStyleSheet(book_switch_combo, "book_list_comboBox_rounded", QStringList() << "【round_radius】" << border_radius);
    }
    else
    {
        book_switch_combo->setFixedHeight(us->widget_size+4);
        thm->setWidgetStyleSheet(book_switch_combo, "book_list_comboBox_square");
    }
}

void NovelDirGroup::initRecent()
{
    getListView()->initRecent();
}

void NovelDirGroup::initData()
{
    // 目录文件夹是否存在
    ensureDirExist(rt->DATA_PATH);
    ensureDirExist(rt->NOVEL_PATH);

    //stts->setSection("dir");

    // 打开上次小说
    // 这里只是在列表中显示上次打开的小说，不是在listview中打开章节
    // listview留到mainwindow中加载，然后发出信号改变全局
    QString recent_name = us->getStr("recent/novel");
    if (!novel_new_name_temp.isEmpty())
    {
        recent_name = novel_new_name_temp;
        novel_new_name_temp = "";
    }
    if (recent_name != "")
    {
        QDir bookDir(rt->NOVEL_PATH+recent_name);
        if (!bookDir.exists())
        {
            //QMessageBox::information(this, tr("作品不存在"), QObject::tr("很抱歉，您的作品《%1》文件不存在！\n文件路径：%2").arg(recentName).arg(NovelPath+recentName));
            recent_name = "";
            us->setVal("recent/novel", "");
            novel_dir_listview->readNovel(""); // 打开空的小说，即清空
        }
        else
        {
            novel_dir_listview->readNovel(recent_name); // 打开这个小说
        }
    }

    // 读取所有目录
    readBookList(recent_name);
}

void NovelDirGroup::readBookList(QString recent)
{
    book_switch_combo->clear();
    QDir dirDir(rt->NOVEL_PATH);
    dirDir.setFilter(QDir::Dirs);
    QFileInfoList list = dirDir.entryInfoList();
    int count = list.count();
    for (int i = 0; i < count; i++)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        if (fileName == "." || fileName == "..")
            continue;
        book_switch_combo->addItem(fileName);

        // 是上次打开的小说
        if (!recent.isEmpty() && fileName == recent)
        {
            book_switch_combo->setCurrentIndex(i);
            book_switch_combo->setCurrentText(recent);
            rt->current_novel = fileName; // 创建时就设置当前打开的小说
        }
    }

    // 创建新书的选项
    book_switch_combo->addItem(QWidget::tr("创建新书"));
}

void NovelDirGroup::paintEvent(QPaintEvent *event)
{
    /*QPainter painter(this);
    QPainterPath path_back;
    path_back.setFillRule(Qt::WindingFill);
    path_back.addRect(0, 0, this->width(), this->height());
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(path_back, QBrush(us->mainwin_sidebar_color)); // 不需要了，现在已经移到父类了
//    painter.fillPath(path_back, QBrush(Qt::red));*/
    return QWidget::paintEvent(event);
}

void NovelDirGroup::enterEvent(QEvent *event)
{
    add_roll_button->showForeground2();
    info_button->showForeground2();

    return QWidget::enterEvent(event);
}

void NovelDirGroup::leaveEvent(QEvent *event)
{
    add_roll_button->hideForeground();
    info_button->hideForeground();

    return QWidget::leaveEvent(event);
}

void NovelDirGroup::resizeEvent(QResizeEvent *event)
{
    return QWidget::resizeEvent(event);
}

void NovelDirGroup::keyPressEvent(QKeyEvent *event)
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

NovelDirListView* NovelDirGroup::getListView()
{
    return novel_dir_listview;
}

bool NovelDirGroup::execCommand(QString command)
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

bool NovelDirGroup::execCommand(QString command, QStringList args)
{
    if (command == "新建作品")
    {
        return createBook();
    }
    else if (command == "切换作品")
    {
        if (args.size() > 0)
        {
            QString novel_name = args.at(0);
            for (int i = 0; i < book_switch_combo->count(); i++)
            {
                if (book_switch_combo->itemText(i) == novel_name)
                {
                    return switchBook(i);
                }
            }
            QMessageBox::warning(this, "切换作品失败", "作品《"+novel_name+"》不存在\n若您手动修改了文件，请先执行“刷新作品列表”执行");
        }
        else
        {
            QMessageBox::warning(this, "切换作品失败", "未带有<参数1>作品名字");
        }
        return false;
    }
    else if (command == "刷新作品列表")
    {
        readBookList(getListView()->getNovelName());
    }
    else if (command == "刷新目录列表")
    {
        novel_dir_listview->readNovel(getListView()->getNovelName());
    }
    else if (command == "添加新卷" || command == "添加分卷")
    {
        slotAddRollButtonClicked();
    }
    else if (command == "添加新章" || command == "添加章节")
    {
        slotAddChapterButtonClicked();
    }
    else if (command == "作品信息一览")
    {
        slotInfoClicked();
    }
    else if (command == "目录设置")
    {
        novel_dir_listview->slotDirSettings();
    }
    else if (command == "导出作品")
    {
        novel_dir_listview->actionExport();
    }
    else if (command == "导入作品")
    {
        novel_dir_listview->actionImport();
    }
    else if (command == "从码字风云导入")
    {
        novel_dir_listview->actionImportMzfy();
    }
    else
        return false;
    QTimer::singleShot(100, [=]{
        setFocus();
    });
    return true;
}

bool NovelDirGroup::switchBook(int index)
{
    int count = book_switch_combo->count();

    log("切换作品：" + QString::number(index) + "/" + QString::number(count));

    if (index < 0 || index >= count)
        return false;

    if (index == count-1) // 创建新书
    {
        // 先切换回去（万一取消创建了呢，还是显示“创建新书”呀）
        for (int i = 0; i < book_switch_combo->count(); i++)
            if (book_switch_combo->itemText(i) == rt->current_novel)
            {
                book_switch_combo->setCurrentIndex(i);
                break;
            }
        // 创建作品流程
        createBook();
        return false;
    }

    QString novel_name = book_switch_combo->currentText();
    rt->current_novel = novel_name;

    novel_dir_listview->readNovel(novel_name);

    emit signalOpenNovel(novel_name);

    us->setVal("recent/novel", novel_name);
    return true;
}

bool NovelDirGroup::createBook()
{
    log("创建作品");
    // 输入书名
    bool isOk;
    QString new_name = NovelTools::getNewNameAI(rt->NOVEL_PATH), tip = "请输入新书书名";

    while (1)
    {
        new_name = QInputDialog::getText(this, "创建新书", tip, QLineEdit::Normal, new_name, &isOk);
        if (!isOk || new_name.isEmpty()) return false;

        // 判断是否包含特殊字符
        QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            if (new_name.contains(cs[i]))
            {
                tip = tr("书名包含特殊字符【%1】，请重新输入").arg(cs[i]);
                //QMessageBox::information(NULL, tr("创建新书失败"), tr("包含特殊字符【%1】啦！").arg(cs[i]));
                break;
            }
        }
        if (i == 10) break;
    }

    //qDebug() << "预测类型：" << NovelTypeUtil::getTypeFromName(new_name);

    // 检验重复书籍
    QDir bookDir(rt->NOVEL_PATH+new_name);
    if (bookDir.exists())
    {
        QMessageBox::information(this, tr("创建新书失败"), tr("已经有同名书籍《%1》啦！").arg(new_name));
        return false;
    }

    // 开始创建文件夹
    bookDir.mkdir(rt->NOVEL_PATH+new_name);
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/chapters");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/details");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/outlines");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/recycles");

    // 读取默认目录模板
    QString def_dir = "", lineStr = "";
    QFile def_dir_file(":/default/dir");
    def_dir_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream tsr(&def_dir_file);
        tsr.setCodec("UTF-8");
        while (!tsr.atEnd())
        {
            lineStr = tsr.readLine();
            def_dir += lineStr;
        }
    def_dir_file.close();

    // 创建默认目录
    QFile dir_file(rt->NOVEL_PATH+new_name+"/"+NOVELDIRFILENAME);
    dir_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream tsw(&dir_file);
        tsw.setCodec("UTF-8");
        tsw << def_dir << endl;
    dir_file.close();

    // 重新读取目录(设置+目录）
    novel_dir_listview->readNovel(new_name);
    rt->current_novel = new_name;
    rt->deleted_novel_name = ""; // 创建后无视删除作品的操作
    rt->deleted_novel_timestamp = 0;

    // 添加到目录列表
    int count = book_switch_combo->count();
    book_switch_combo->insertItem(count-1, new_name);
    book_switch_combo->setCurrentIndex(count);
    book_switch_combo->setCurrentText(new_name);

    // 初始化一些设置
    Settings* ns = new Settings(rt->NOVEL_PATH+new_name+"/settings.ini");
    QString author = us->getStr("recent/author");
    if (!author.isEmpty())
        ns->setVal("author", author);
    QString type = NovelTools::getTypeFromName(new_name);
    if (!type.isEmpty())
        ns->setVal("type", type);
    ns->sync();

    emit signalOpenNovel(new_name);

     // 保存到最近作品
    us->setVal("recent/novel", new_name);
    ac->addDir(new_name);
    return true;
}

void NovelDirGroup::slotRenameNovel()
{
    QString novel_name = novel_dir_listview->getNovelName();
    bool b;
    QString new_name = QInputDialog::getText(this, "修改书名", "请输入新的作品名字（不包含特殊符号）", QLineEdit::Normal, novel_name, &b);
    if (!b || new_name.isEmpty())
        return ;

    // 判断是否包含特殊字符
    QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        if (new_name.contains(cs[i]))
        {
            QMessageBox::information(this, tr("修改书名失败"), tr("看起来好像似乎有可能包含了特殊字符【%1】哦~").arg(cs[i]));
            return ;
        }
    }

    if (new_name == novel_name)
        return ;

    // 检验重复书籍
    QDir book_dir(rt->NOVEL_PATH+new_name);
    if (book_dir.exists())
    {
        QMessageBox::information(this, tr("修改书名失败"), tr("已经有同名书籍《%1》啦！").arg(new_name));
        return ;
    }

    // 开始重命名路径
    renameFile(rt->NOVEL_PATH+novel_name, rt->NOVEL_PATH+new_name);
    us->setVal("recent/novel", new_name);
    novel_new_name_temp = new_name;
    initData();
    rt->current_novel = new_name;

    // 发出信号（编辑器修改名字）
    emit signalRenameNovel(novel_name, new_name);
}

void NovelDirGroup::slotDeleteNovel()
{
    QString novel_name = novel_dir_listview->getNovelName();
    QString recycle_path;
    int exist_index = 0;
    if (isFileExist(rt->DATA_PATH+"recycles/"+novel_name))
        exist_index++;
    while (isFileExist(rt->DATA_PATH+"recycles/"+novel_name+"("+QString("%1").arg(exist_index)+")"))
        exist_index++;
    if (exist_index > 0)
        recycle_path = rt->DATA_PATH+"recycles/"+novel_name+"("+QString("%1").arg(exist_index)+")";
    else
        recycle_path = rt->DATA_PATH+"recycles/"+novel_name;

    if (QMessageBox::question(this, QString("是否删除《%1》？").arg(novel_name), QObject::tr("删除后作品将会放到回收站，路径：%1/").arg(recycle_path), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return ;

    ensureDirExist(rt->DATA_PATH+"recycles");

    // 先发送删除的信号，其他地方（比如编辑器）接收到后，可以先进行保存操作，然后再删除
    emit signalDeleteNovel(novel_name);

    // 设置删除记录。作用：在接下来马上创建作品时可以设置默认作品名为刚删除的这个
    rt->deleted_novel_name = novel_name;
    rt->deleted_novel_timestamp = getTimestamp();

    renameFile(rt->NOVEL_PATH+novel_name, recycle_path); // 重命名文件到回收站
    novel_dir_listview->readNovel(""); // 打开空的小说，即清空列表
    book_switch_combo->setCurrentText(tr("切换作品"));
    rt->current_novel = "";

    //novel_had_deleted_temp = true;
    ac->addNovelDeleted(novel_name);
    us->setVal("recent/novel", "");
    initData();
}

void NovelDirGroup::slotNeedLineGuide()
{
    rt->setLineGuideWidget(book_switch_combo);
}

void NovelDirGroup::slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    // 这个方法并没有用到
    Q_UNUSED(previous);
    add_chapter_button->setEnabled(current.isValid());
}

void NovelDirGroup::slotInfoClicked()
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    pos.setY(pos.y()); // 相对于整个窗口来说的
    emit signalShowInfo(novel_dir_listview->getNovelName(), pos);
}

void NovelDirGroup::slotAddRollButtonClicked()
{
    QRect r = add_roll_button->rect();
    QRect rect(r.left(), r.top()+btn_layout->geometry().top()-novel_dir_listview->geometry().top(), r.width(), r.height());
    novel_dir_listview->slotExternRect(rect);

    QPoint p = QCursor::pos();
    p = this->mapFromGlobal(p);
    QPoint point(p.x(), p.y());
    novel_dir_listview->slotExternPoint(point);

    novel_dir_listview->slotAddRoll();
}

void NovelDirGroup::slotAddChapterButtonClicked()
{
    QRect r = add_chapter_button->rect();
    QRect rect(r.left(), r.top()+btn_layout->geometry().top()-novel_dir_listview->geometry().top(), r.width(), r.height());
    novel_dir_listview->slotExternRect(rect); // 按钮的位置

    QPoint p = QCursor::pos();
    p = this->mapFromGlobal(p);
    QPoint point(p.x(), p.y());
    novel_dir_listview->slotExternPoint(point); // 点击的位置，从下而上的动画

    novel_dir_listview->slotAddChapter();
}

void NovelDirGroup::slotImportFinished(QString name)
{
    Q_UNUSED(name);
    initData();
}
