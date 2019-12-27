#include "novelschedulewidget.h"

NovelScheduleWidget::NovelScheduleWidget(NovelDirListView *listview, QWidget *parent)
    : QWidget(parent)
{
    ns = nullptr;
    dir_listview = listview;

    initLayout();
    initEvent();
    initStyle();

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));

    //this->setObjectName("NovelScheduleWidget");
}

/**
 * @brief NovelScheduleWidget::initLayout
 * 初始化控件(仅布局、相对位置）
 */
void NovelScheduleWidget::initLayout()
{
    // 创建控件
	cover_image     = new QImage(":/icons/book");
	cover_label     = new MyLabel("设置封面", this);
    novel_label     = new InteractiveButtonBase("小说名字", this);
    type_label      = new InteractiveButtonBase("风格", this);
    author_label    = new InteractiveButtonBase("作者", this);
    wc_label        = new InteractiveButtonBase("字数", this);
    desc_edit       = new QTextEdit("简介", this);
	rc_count_label  = new MyLabel("章卷数量", this);
    last_label      = new MyLabel("最近编辑：", this);
    last_c_label[0] = new InteractiveButtonBase("最近一章", this);
    last_c_label[1] = new InteractiveButtonBase("最近二章", this);
    last_c_label[2] = new InteractiveButtonBase("最近三章", this);
	days_label      = new MyLabel("创作天数", this);
    QPixmap* icon_pixmap = new QPixmap(50,50);
    icon_pixmap->load(":/icons/operator");
//    QIcon *icon = new QIcon(*icon_pixmap);
    if (us->round_view)
    {
        operator_button = new WaterCircleButton(QPixmap(":/icons/operator"), this);
        //operator_button->setFixed();
        //operator_button->setFixedSize(32,32);
    }
    else
    {
        //operator_button = new GeneralButtonInterface(*icon, "", this);
        operator_button = new InteractiveButtonBase(QPixmap(":/icons/operator"), this);
    }

    cover_label->setCursor(Qt::PointingHandCursor);
    novel_label->setCursor(Qt::PointingHandCursor);
    type_label->setCursor(Qt::PointingHandCursor);
    author_label->setCursor(Qt::PointingHandCursor);
    last_c_label[0]->setCursor(Qt::PointingHandCursor);
    last_c_label[1]->setCursor(Qt::PointingHandCursor);
    last_c_label[2]->setCursor(Qt::PointingHandCursor);

    // 创建布局
	QVBoxLayout* main_vlayout   = new QVBoxLayout(this);
	QHBoxLayout* name_hlayout   = new QHBoxLayout();
	QVBoxLayout* name_vlayout   = new QVBoxLayout();
	QHBoxLayout* recent_hlayout = new QHBoxLayout();
	QVBoxLayout* recent_vlayout = new QVBoxLayout();
	QHBoxLayout* btn_hlayout    = new QHBoxLayout();

    // 控件添加到布局
	name_hlayout   -> addWidget(cover_label);
	name_vlayout   -> addWidget(novel_label);
	name_vlayout   -> addWidget(type_label);
	name_vlayout   -> addWidget(author_label);
	name_vlayout   -> addWidget(wc_label);
	name_hlayout   -> addLayout(name_vlayout);
    main_vlayout   -> addLayout(name_hlayout);
    main_vlayout   -> addWidget(desc_edit);
    main_vlayout   -> addWidget(rc_count_label);
	recent_hlayout -> addWidget(last_label);
    recent_vlayout -> addWidget(last_c_label[0]);
    recent_vlayout -> addWidget(last_c_label[1]);
    recent_vlayout -> addWidget(last_c_label[2]);
	recent_hlayout -> addLayout(recent_vlayout);
    recent_widget = new QWidget(this);
    recent_widget  -> setLayout(recent_hlayout);
    main_vlayout   -> addWidget(recent_widget);
    btn_hlayout    -> addWidget(days_label);
	btn_hlayout    -> addWidget(operator_button);
    main_vlayout   -> addLayout(btn_hlayout);
    setLayout(main_vlayout);

    // 设置空间大小
    main_vlayout   -> setSpacing(3);
    rc_count_label -> setMargin(10);
    recent_hlayout->setStretch(0, 1);
    recent_hlayout->setStretch(1, 2);
    last_label     -> setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    name_vlayout->setSpacing(0);
    name_hlayout->setContentsMargins(0, 0, 5, 5);
    name_hlayout->setSpacing(10);
    recent_vlayout->setSpacing(0);
    recent_vlayout->setMargin(0);
//    name_vlayout->setSpacing(3);

    // 设置提示
    cover_label->setToolTip("点击修改作品封面");
    novel_label->setToolTip("点击修改作品名字");
    type_label->setToolTip("点击修改作品风格");
    author_label->setToolTip("点击修改作者笔名");
    wc_label->setToolTip("全书字数（不包含空白符）");
    operator_button->setToolTip(tr("作品操作"));

    // Mac控件布局
    cover_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    novel_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    type_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    author_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    wc_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    desc_edit->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    rc_count_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    recent_widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    last_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    last_c_label[0]->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    last_c_label[1]->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    last_c_label[2]->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    days_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    operator_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

/**
 * @brief NovelScheduleWidget::initStyle
 * 美化界面（字体、色彩等）
 */
void NovelScheduleWidget::initStyle()
{
    // 定义一些标准变量
    font_family = wc_label->font().family();
    QFont normal_font(font_family,  font().pointSize() ,  QFont::Normal);
    int small_font_size = font().pointSize() * 0.8;
    int large_font_size = font().pointSize() * 1.4;

    QPalette gray_palette;
    gray_palette.setColor(QPalette::WindowText, Qt::gray);

    // 单独设置每个控件的属性
    cover_label->setScaledContents(true);

    novel_label->setFont(QFont(font_family, font().pointSize(), QFont::Bold));
//    novel_label->setFontSize(8);
    novel_label->setFontSize(large_font_size);
    novel_label->setFixedForeSize(true, 13);

    type_label->setFont(normal_font);
    type_label->setTextColor(Qt::gray);

    author_label->setFont(normal_font);
    author_label->setTextColor(Qt::gray);

    desc_edit->setFont(normal_font);
    desc_edit->setPalette(gray_palette);
    desc_edit->setContentsMargins(5,5,5,5);

    last_label->setFont(normal_font);
    /*last_c_label[0]->setFontSize(small_font_size );
    last_c_label[1]->setFontSize(small_font_size );
    last_c_label[2]->setFontSize(small_font_size );*/
    last_c_label[0]->setFixedForeSize(true, 5);
    last_c_label[1]->setFixedForeSize(true, 5);
    last_c_label[2]->setFixedForeSize(true, 5);

    last_c_label[0]->setAlign(Qt::AlignLeft);
    last_c_label[1]->setAlign(Qt::AlignLeft);
    last_c_label[2]->setAlign(Qt::AlignLeft);
    novel_label->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
    type_label->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
    author_label->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
    wc_label->setAlign(Qt::AlignLeft | Qt::AlignVCenter);

    cover_label->setMaximumHeight(novel_label->height()+type_label->height()+author_label->height()+wc_label->height()+8);
    cover_label->setMaximumWidth(cover_label->maximumHeight()/1.5);

    days_label->setFont(normal_font);
    days_label->setPalette(gray_palette);

    operator_button->setFixedSize(operator_button->height(), operator_button->height());

    /**
     * Font 参数三 常见权重：
     *	QFont::Light - 25 高亮
     *	QFont::Normal - 50 正常
     *	QFont::DemiBold - 63 半粗体
     *	QFont::Bold - 75 粗体
     *	QFont::Black - 87 黑体
     */
}

/**
 * @brief NovelScheduleWidget::initEvent
 * 设置事件
 */
void NovelScheduleWidget::initEvent()
{
    // 修改作品封面
    connect(cover_label, &MyLabel::clicked, [=]{
		QString recent = us->getStr("recent/file_path");
	    if (recent == "" || !isFileExist(recent))
	        recent = ".";
	    QString path = QFileDialog::getOpenFileName(this, "选择作品封面（建议120*150）", recent, tr("Image Files(*.jpg *.png)"));
	    if (path.length() == 0) return ;

	    us->setVal("recent/file_path", path);

        QString new_path;
        if (path.endsWith("png"))
            new_path = rt->NOVEL_PATH+novel_name+"/cover.png";
        else if (path.endsWith("jpg"))
            new_path = rt->NOVEL_PATH+novel_name+"/cover.jpg";
        else
            new_path = rt->NOVEL_PATH+novel_name+"/cover.jpg";
        copyFile2(path, new_path); // 强制覆盖原来的文件
        cover_image->load(new_path);
	    cover_label->setPixmap(QPixmap::fromImage(*cover_image));
    });

    // 修改书名
    connect(novel_label,&InteractiveButtonBase::clicked, [=]{
        emit signalChangeNovelName();
	});

    // 修改类型
    connect(type_label, &InteractiveButtonBase::clicked, [=]{
        bool isOk = false;
        QString type = ns->getStr("type");
        if (type.isEmpty())
            type = NovelTools::getTypeFromName(novel_name);
        QString str = QInputDialog::getText(this, "修改风格", "请输入作品风格", QLineEdit::Normal, type, &isOk);
        if (!isOk) return ;
        ns->setVal("type", str);
        if (str.isEmpty())
            type_label->setText("未设置风格");
        else
            type_label->setText(str);
    });

    // 修改作者
    connect(author_label, &InteractiveButtonBase::clicked, [=]{
        bool isOk = false;
        QString author = ns->getStr("author");
        if (author.isEmpty())
            author = us->getStr("recent/author");
        QString str = QInputDialog::getText(this, "修改作者", "请输入您的笔名", QLineEdit::Normal, author, &isOk);
        if (!isOk) return ;
        ns->setVal("author", str);
        us->setVal("recent/author", str);
        if (str.isEmpty())
            author_label->setText("佚名");
        else
            author_label->setText(str+" 著");
    });

    // 简介自动保存
    connect(desc_edit, &QTextEdit::textChanged, [=]{
        QString text = desc_edit->toPlainText();
        ns->setVal("desc", text);
    });

    // 打开最近编辑的章节
    connect(last_c_label[0], &InteractiveButtonBase::clicked, [=]{
        QString file_name = last_modify_names[0];
        if (file_name.isEmpty())
            return ;
        emit signalToHide();
        dir_listview->slotOpenChapter(fnDecode(file_name));
    });
    connect(last_c_label[1], &InteractiveButtonBase::clicked, [=]{
        QString file_name = last_modify_names[1];
        if (file_name.isEmpty())
            return ;
        emit signalToHide();
        dir_listview->slotOpenChapter(fnDecode(file_name));
    });
    connect(last_c_label[2], &InteractiveButtonBase::clicked, [=]{
        QString file_name = last_modify_names[2];
        if (file_name.isEmpty())
            return ;
        emit signalToHide();
        dir_listview->slotOpenChapter(fnDecode(file_name));
    });

    operator_menu = new QMenu(operator_button);
    undo_action = new QAction(thm->icon("menu/undo"), tr("撤销操作"), operator_button);
    settings_action = new QAction(thm->icon("settings"), tr("目录设置"), operator_button);
    import_action = new QAction(thm->icon("menu/import"), tr("导入"), operator_button);
    export_action = new QAction(thm->icon("menu/export"), tr("导出"), operator_button);
    import_mzfy_action = new QAction(thm->icon("mzfy"), tr("从码字风云导入"), operator_button);
    rename_action = new QAction(thm->icon("menu/modify"), tr("修改书名"), operator_button);
    search_action = new QAction(thm->icon("menu/search_book"), tr("全书查找"), operator_button);
    duplicate_action = new QAction(thm->icon("menu/duplicate"), tr("复制作品"), operator_button);
    delete_action = new QAction(thm->icon("menu/delete"), tr("删除作品"), operator_button);
    recycle_action = new QAction(thm->icon("menu/recycle"), tr("回收站"), operator_button);

    recycle_action->setEnabled(false);
    search_action->setEnabled(false);

    // 修改书名
    connect(rename_action, &QAction::triggered, [=]{
        emit signalChangeNovelName();
    });
    // 删除作品
    connect(delete_action, &QAction::triggered, [=]{
        emit signalDeleteNovel();
        emit signalToHide(); // 通知背景隐藏
        toHide(); // 自己隐藏
    });
    // 导入导出
    connect(import_action, &QAction::triggered, [=]{
        emit signalImExPort(2, novel_name);
    });
    connect(export_action, &QAction::triggered, [=]{
        emit signalImExPort(1, novel_name);
    });
    connect(import_mzfy_action, &QAction::triggered, [=]{
        emit signalImExPort(3, novel_name);
    });
    connect(settings_action, &QAction::triggered, [=]{
        emit signalDirSettings();
    });
    connect(duplicate_action, &QAction::triggered, [=]{
        slotDuplicateBook(novel_name);
        emit signalToHide();
        toHide();
    });


    operator_button->setContextMenuPolicy(Qt::ActionsContextMenu); // 设置为右键菜单样式
    //operator_menu->setContextMenuPolicy(Qt::CustomContextMenu);      // 自定义菜单样式
    operator_menu->addAction(settings_action);
    operator_menu->addAction(export_action);
    operator_menu->addAction(import_action);
    operator_menu->addAction(import_mzfy_action);
    operator_menu->addAction(rename_action);
    operator_menu->addAction(search_action);
    operator_menu->addAction(duplicate_action);
    operator_menu->addAction(delete_action);
    operator_menu->addAction(recycle_action);
    connect(operator_button, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowMenu(QPoint))); // 运行不到的
    connect(operator_button, SIGNAL(clicked(bool)), this, SLOT(slotShowMenu()));
}

/**
 * @brief NovelScheduleWidget::slotShowMenu
 * 这个好像运行不到的
 * @param point
 */
void NovelScheduleWidget::slotShowMenu(QPoint point)
{
    QPoint p = mapFromGlobal(point);
    operator_menu->exec(p);
}

void NovelScheduleWidget::slotShowMenu()
{
    QPoint p = QCursor::pos();
    p.setY(p.y()-operator_menu->height());
    if (p.y() < 0) p.setY(0);
    operator_menu->exec(p);
}

void NovelScheduleWidget::slotChangeNovelNameFinished(QString old_name, QString new_name)
{
    Q_UNUSED(old_name);
    novel_name = new_name;
    novel_label->setText(new_name);
}

void NovelScheduleWidget::slotDuplicateBook(QString novel_name)
{
    int index = 1;
    while (isFileExist(rt->NOVEL_PATH+novel_name+"_"+QString::number(index)))
        index++;
    QString new_name = novel_name+"_"+QString::number(index);

    QString tip = "复制《"+novel_name+"》为另一部新作品《"+new_name+"》，可实现后续一本书拥有多条思路，或者仅作为数据备份。\n\n备份内容包括：目录及目录设置、正文章节、小说封面、大纲、细纲、回收站等。";
    if (QMessageBox::information(this, "复制作品", tip, QMessageBox::Ok, QMessageBox::No) == QMessageBox::No)
        return ;

    copyDir(rt->NOVEL_PATH+novel_name, rt->NOVEL_PATH+new_name);
    QMessageBox::information(this, "复制成功", "复制《"+novel_name+"》为《"+new_name+"》完毕\n\n可在目录顶端的作品列表中看到切换入口。");
    emit signalRefreshBookList("");
}

/**
 * @brief NovelScheduleWidget::initGeometry
 * 设置位置（偏移、大小等）
 */
void NovelScheduleWidget::adjustGeometry(QPoint point)
{
    // 按照百分比手动调整位置，然后就忽视了layout的作用……
    // 因为确实也没什么用

    //cover_label->setFixedSize(width()/2.5, width()/2.5*4/3);

    //QString style = "QWidget#NovelScheduleWidget{ border-radius:5px; background-color:" + us->getOpacityColor(us->mainwin_bg_color, 200) + " ;}";
    //this->setStyleSheet(style);

    int w = parentWidget()->width();
    int h = parentWidget()->height();
    w /= 3;
    h /= 1.5;
    setMaximumSize(w, h);
    setMinimumSize(max(w/2,250), max(h/2,400));

    int l = point.x()-width()/2, t = point.y()-20-height();
    if (l < 30) l = 30;
    if (t < 10) t = 10;

    //setGeometry(l, t, w, h);
    move(l, t);
}

void NovelScheduleWidget::updateUI()
{
    operator_button->setIconColor(us->accent_color);

    desc_edit->setStyleSheet("QTextEdit{border-width:1px;border-style:solid;border-color:"+us->getOpacityColorString(us->accent_color, 80)+"; background-color: transparent;}");
    recent_widget->setStyleSheet("QWidget{border-width:1px;border-style:solid;border-color:"+us->getOpacityColorString(us->accent_color, 80)+"}\
        QLabel,QPushButton{border:none;}");

    last_c_label[0]->setTextColor(us->global_font_color);
    last_c_label[1]->setTextColor(us->global_font_color);
    last_c_label[2]->setTextColor(us->global_font_color);
    novel_label->setTextColor(us->global_font_color);
//    type_label->setTextColor(us->global_font_color);
//    author_label->setTextColor(us->global_font_color);
//    wc_label->setTextColor(us->global_font_color);
}

/**
 * @brief NovelScheduleWidget::initDate
 * 初始化数据：设置具体的显示内容
 */
void NovelScheduleWidget::initData()
{
    if (ns != nullptr)
        delete ns;
    ns = new Settings(rt->NOVEL_PATH+novel_name+"/settings.ini");

    if (isFileExist(rt->NOVEL_PATH+novel_name+"/cover.png"))
    {
        cover_image->load(rt->NOVEL_PATH+novel_name+"/cover.png");
    }
    else if (isFileExist(rt->NOVEL_PATH+novel_name+"/cover.jpg"))
    {
        cover_image->load(rt->NOVEL_PATH+novel_name+"/cover.jpg");
    }
    else if (isFileExist(rt->NOVEL_PATH+novel_name+"/cover.jpeg"))
    {
        cover_image->load(rt->NOVEL_PATH+novel_name+"/cover.jpeg");
    }
    else
    {
        cover_image->load(":/icons/book");
    }
    cover_label->setPixmap(QPixmap::fromImage(*cover_image));
    QSize size = cover_image->size();
    if (size.width() <= size.height() && size.width()*2 >=  size.height())
    {
        int height = novel_label->height() + type_label->height() + author_label->height() + wc_label->height() + 8;
        cover_label->setMaximumSize(height * size.width() / size.height(), height);
        cover_label->resize(height * size.width() / size.height(), height);
    }

    novel_label->setText(novel_name);

    type_label->setText(ns->getStr("type", ">> 风格"));

    QString author = ns->getStr("author");
    if (author.isEmpty())
        author = ">> 作者";
    else
        author += " 著";
    author_label->setText(author);

    wc_label->setText(QString("共 %1 字").arg(getWordCount(novel_name)));

    desc_edit->setText(ns->getStr("desc", "简介："));

    rc_count_label->setText(getRcCount());

    getRecentChapter(novel_name);

    days_label->setText(getCreateDays(novel_name));
}

void NovelScheduleWidget::toShow(QString name, QPoint point)
{
    novel_name = name;
    if (!isHidden())
    {
        initData();  // 设置novel对应的数据
        return ;
    }
    bg_color = us->mainwin_bg_color;

    adjustGeometry(point); // 重新调整界面
    initData();  // 设置novel对应的数据

    callback_point = point;
    this->show();
}

void NovelScheduleWidget::toHide()
{

    hide();
}

void NovelScheduleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 画背景
    QPainterPath pathBack;
    pathBack.setFillRule(Qt::WindingFill);
    pathBack.addRoundedRect(QRect(1, 1, this->width()-2, this->height()-2), 5, 5);
    painter.fillPath(pathBack, QBrush(bg_color));

    // 画边框
    /*QPainterPath pathBack1;
    pathBack1.setFillRule(Qt::WindingFill);
    pathBack1.addRoundedRect(QRect(0, 0, this->width(), this->height()), 5, 5);
    painter.fillPath(pathBack1, QBrush(QColor(0,0,0,50)));*/
    QPen pen(painter.pen());
    pen.setColor(QColor(0,0,0,50));
    painter.setPen(pen);
    painter.drawRoundRect(QRect(0, 0, this->width()-1, this->height()-1), 5, 5);

    return QWidget::paintEvent(event);
}

void NovelScheduleWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        emit signalEsc();
    }

    return QWidget::keyPressEvent(event);
}

int NovelScheduleWidget::getWordCount(QString name)
{
	int wc = 0;
    r_count = c_count = 0;
	QString dir_path = rt->NOVEL_PATH+name+"/"+NOVELDIRFILENAME; // 目录路径
	QString chpt_path = rt->NOVEL_PATH+name+"/chapters/"; // 章节路径

    // 读取目录文件
    QString content = readTextFile(dir_path);

	// 获取所有章节数量
	QStringList rolls = getStrMids(content,  "<ROLL>",  "</ROLL>");
    r_count = rolls.size()-1;
	for (int ro = 1; ro < rolls.length(); ro++) // 跳过作品相关
    {
        QString roll_text = getStrMid(rolls[ro],  "<LIST>",  "</LIST>");
        QStringList chpts = getStrMids(roll_text,  "<CHPT>",  "</CHPT>");
        for (QString chpt : chpts)
        {
            QString chapter_path = chpt_path+getStrMid(chpt, "<n:", ">")+".txt";
            if (!isFileExist(chapter_path))
                continue ;
            wc += getTextWordCount(readTextFile(chapter_path));
        }
        c_count += chpts.size();
	}

    return wc;
}

int NovelScheduleWidget::getTextWordCount(QString text)
{
	int blank_count = 0;
	int len = text.length();
	for (int i = 0; i < len; i++)
	{
        QString c(text.at(i));
        if (c == " " || c == "\n" || c == "　" || c == "\t")
			blank_count++;
    }
    return len-blank_count;
}

void NovelScheduleWidget::getRecentChapter(QString name)
{
    QList<QFileInfo> file_list;

    QString dir_path = rt->NOVEL_PATH+name+"/"+NOVELDIRFILENAME; // 目录路径
    QString chpt_path = rt->NOVEL_PATH+name+"/chapters/"; // 章节路径
    QString content = readTextFile(dir_path);

    // ==== 遍历每一章 ====
    QStringList rolls = getStrMids(content,  "<ROLL>",  "</ROLL>");
    for (int ro = 1; ro < rolls.length(); ro++) // 跳过作品相关
    {
        QString roll_text = getStrMid(rolls[ro],  "<LIST>",  "</LIST>");
        QStringList chpts = getStrMids(roll_text,  "<CHPT>",  "</CHPT>");
        for (QString chpt : chpts)
        {
            QString c_name = getStrMid(chpt, "<n:", ">");
            if (!isFileExist(chpt_path+c_name+".txt"))
                continue ;
            QFileInfo info(chpt_path+c_name+".txt");
            file_list.append(info);
        }
    }

    // 获取前三的
    for (int i = 0; i < 3; i++)
    {
    	int max_index = -1;
    	QDateTime max_time;
        for (int fi = 0; fi < file_list.size(); fi++)
    	{
    	    if (max_index == -1 || file_list[fi].lastModified() > max_time)
    	    {
    	        max_index = fi;
    	        max_time = file_list[fi].lastModified();
    	    }
    	}
    	if (max_index > -1)
    	{
            QString file_name = file_list[max_index].fileName();
            QDateTime time = file_list[max_index].lastModified();
            long long timestamp0 = time.toMSecsSinceEpoch();
            if (file_name.endsWith(".txt"))
                file_name.chop(4); // 去掉后缀
            last_modify_names[i] = file_name; // 添加到全局列表
            file_list.removeAt(max_index);    // 移除这一项
            QString name = fnDecode(last_modify_names[i]);
            QModelIndex model_index = dir_listview->getModel()->getModelIndexByName(name);
            name = dir_listview->getDelegate()->getItemText(model_index);
            //name = dir_listview->getModel()->getFullNameByName(name);
            last_c_label[i]->setText(name);

            QString content = readTextFile(chpt_path+file_name+".txt");
            QString time_delta = getTimeDelta(timestamp0);
            QString tip = QString("%1 字，%2 前\n%3").arg(getTextWordCount(content)).arg(time_delta).arg(getFirstPara(content));
            last_c_label[i]->setToolTip(tip);
    	}
        else
        {
            last_c_label[i]->setText("");
            last_c_label[i]->setToolTip("本作品的章节数量不够啊，还需努力呢！");
        }
    }
}

int NovelScheduleWidget::compByTime(QFileInfo a, QFileInfo b)
{
    return a.lastModified() > b.lastModified();
}

QString NovelScheduleWidget::getFirstPara(QString text)
{
    int len = text.length();
    int first_pos = 0;
    while (first_pos < len)
    {
        QString c = text.mid(first_pos, 1);
        if (c == " " || c == "\n" || c == "　" || c == "\t")
            first_pos++;
        else
            break;
    }
    int end_pos = text.indexOf("\n", first_pos);
    if (end_pos == -1)
        end_pos = len;
    text = text.mid(first_pos, end_pos-first_pos);
    if (text.length() > 500)
        return text.left(500);
    return text;
}

QString NovelScheduleWidget::getRcCount()
{
    return QString("共 %1 卷，%2 章").arg(r_count).arg(c_count);
}

QString NovelScheduleWidget::getCreateDays(QString name)
{
    Q_UNUSED(name);
    long long timestamp0 = ns->getLongLong("create_timestamp", 0);
    long long timestamp = getTimestamp();
    long long delta = 0;
    delta = timestamp - timestamp0;
    long long ten_year = 315360000000; // 10*365*24*3600*1000
    if (timestamp0 == 0 || delta > ten_year || delta < 0)
    {
        timestamp0 = timestamp;
        ns->setVal("create_timestamp", timestamp);
        delta = 0;
    }

    // 统一计算时间
    int vals[10] = {};
    QString quans[] = {"毫秒", "秒", "分钟", "小时", "天", "年", "世纪"};
    int mutiples[] = {1000, 60, 60, 24, 12, 100};

    int index = 0;
    while (delta > 0 && index < 6)
    {
        vals[index] = delta % mutiples[index];
        delta /= mutiples[index];
        index++;
    }

    QString time_str = "";
    QString tip = "";
    int i = index;
    while (--i >= 0)
    {
        if (i >= index-2)
            time_str += QString(" %1%2").arg(vals[i]).arg(quans[i]);
        tip += QString(" %1%2").arg(vals[i]).arg(quans[i]);
    }

    days_label->setToolTip("本书创作时长：" + tip.trimmed());

    return time_str.trimmed();
}

QString NovelScheduleWidget::getTimeDelta(long long timestamp0)
{
    long long timestamp = getTimestamp();
    long long delta = timestamp - timestamp0;
    long long ten_year = 315360000000; // 10*365*24*3600*1000
    if (timestamp0 == 0 || delta > ten_year || delta < 0)
    {
        return "一段时间";
    }

    // 统一计算时间
    int vals[10] = {};
    QString quans[] = {"毫秒", "秒", "分钟", "小时", "天", "年", "世纪"};
    int mutiples[] = {1000, 60, 60, 24, 12, 100};

    int index = 0;
    while (delta > 0 && index < 6)
    {
        vals[index] = delta % mutiples[index];
        delta /= mutiples[index];
        index++;
    }

    QString time_str = "";
    QString tip = "";
    int i = index;
    while (--i >= 0)
    {
        if (i >= index-2)
            time_str += QString(" %1%2").arg(vals[i]).arg(quans[i]);
        tip += QString(" %1%2").arg(vals[i]).arg(quans[i]);
    }

    days_label->setToolTip(tip.trimmed());

    return time_str.trimmed();
}

int NovelScheduleWidget::max(int a, int b)
{
    return a > b ? a : b;
}
