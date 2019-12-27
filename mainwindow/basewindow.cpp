#include "basewindow.h"

BaseWindow::BaseWindow(QMainWindow *parent)
    : QMainWindow(parent)
{
    initGlobal();

    // FramelessWindowHint属性设置窗口去除边框;
    // WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口;
    if (us->integration_window)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    // this->stackUnder(rt->splash);
    rt->setSplash("正在创建主界面...");
    initLayout();
    initBorderShadow();

    showWindowAnimation();

    rt->startup_timestamp = getTimestamp(); // 打开的时间
    connect(ac, SIGNAL(signalAllFinishedAndCanExit()), this, SLOT(close()));

//   connect(qApp, &QApplication::focusChanged, [=](QWidget* o, QWidget* n){ MAINDBG << "焦点变更" << o << n; });
}

BaseWindow::~BaseWindow()
{
    if (isFileExist(rt->DATA_PATH+"temp"))
        deleteFile(rt->DATA_PATH+"temp"); // 删除临时文件
}

void BaseWindow::initGlobal()
{
#if defined(Q_OS_WIN) && defined(FRISO)
bool use_jieba = us->getBool("us/use_jieba", isFileExist(rt->APP_PATH+"tools/jieba"));
if (!use_jieba)
    gd->participle = new FrisoUtil;
else
#endif
    gd->participle = new JiebaUtil;
    gd->pb.setSegment(gd->participle);
}

void BaseWindow::initLayout()
{
    // 将设置字体放到前面来，以便后面一些控件根据字体来调整大小
    log("设置字体");
    if (isFileExist(rt->DATA_PATH+"style/font.ttf"))
    {
        QString strFont =  loadFontFamilyFromFiles(rt->DATA_PATH+"style/font.ttf");
        QFont font(this->font());
        font.setFamily(strFont);
        QApplication::setFont(font);
    }
    else if (isFileExist(rt->DATA_PATH+"style/font.otf"))
    {
        QString strFont =  loadFontFamilyFromFiles(rt->DATA_PATH+"style/font.otf");
        QFont font(this->font());
        font.setFamily(strFont);
        QApplication::setFont(font);
    }
    else
    {
        QString font_str = us->getStr("us/font");
        if (font_str != "")
        {
            QFont font;
            if (font.fromString(font_str))
            {
                log("加载字体", font_str);
                QApplication::setFont(font);
            }
            else
            {
                log("加载字体失败", font_str);
            }
        }
    }
    if (us->widget_size < QApplication::fontMetrics().height() + QApplication::fontMetrics().lineSpacing()/2)
    {
        us->widget_size = QApplication::fontMetrics().height() + QApplication::fontMetrics().lineSpacing()/2;
    }

    // 设置中心控件
    center_widget = new QWidget();
    this->setCentralWidget(center_widget);

    log("初始化basewindow背景");
    {
        bezier_wave_inited = false;
        bezier_wave_running = false;

        us->emotion_filter = nullptr;
        us->envir_picture = nullptr;
        initBgPictures();
        initEmotionFilter();
        initEnvironmentPictures();
        connect(thm, SIGNAL(signalBgPicturesChanged()), this, SLOT(initBgPictures()));
        connect(us, SIGNAL(signalEmotionFilterChanged()), this, SLOT(initEmotionFilter()));
        connect(us, SIGNAL(signalEnvironmentPicturesChanged()), this, SLOT(initEnvironmentPictures()));

        // 这里本来是rerenderTitlebarPixmap()的，但是会导致崩溃（可能切换背景图的时候会导致错误？），必须要延时一段时间
        // connect(thm, SIGNAL(signalBgPicturesChanged()), this, SLOT(resetTitlebarPixmapSize()));
        connect(thm, &AppTheme::signalBgPicturesChanged,[=]{QTimer::singleShot(0, [=]{ rerenderTitlebarPixmap(); }); });
        connect(us, SIGNAL(signalEmotionFilterChanged()), this, SLOT(rerenderTitlebarPixmap()));
        connect(us, SIGNAL(signalEnvironmentPicturesChanged()), this, SLOT(rerenderTitlebarPixmap()));
    }

    // 菜单栏
    {
        if (us->integration_window)
        {
            int default_height = menuBar()->height(); // = 30
            menubar = new QMenuBar(this);
            menubar->setVisible(true);
            // 这里的 margin 属性，不管方向，且只同时对左上角有效
            // 多余的 1 像素是为了和标题栏对齐……
            menubar->setFixedHeight(us->widget_size);
    //        menubar->move(us->mainwin_border_size + us->widget_radius, us->mainwin_border_size + (us->widget_size-default_height)/2); // move无效
            setMenuBar(menubar);

            menubar->setStyleSheet("background: transparent; margin: "+QString::number(us->mainwin_border_size + (us->widget_size-default_height)+1)+"px;");
            connect(rt, &RuntimeInfo::signalMainWindowMaxing, this, [=](bool m){
               if (m)
                   menubar->setStyleSheet("background: transparent; margin: "+QString::number((us->widget_size-default_height)+1)+"px;");
               else
                   menubar->setStyleSheet("background: transparent; margin: "+QString::number(us->mainwin_border_size + (us->widget_size-default_height)+1)+"px;");
            });
        }
        else
        {
            menuBar()->setStyleSheet("background: transparent; ");
        }


        menu_bar_right = 0;
        if (isFileExist(rt->DATA_PATH + "menu.json"))
            loadMenuFromFile(rt->DATA_PATH + "menu.json");
        else
            loadMenuFromFile(":/default/menu");

        if (!us->menu_bar_showed)
            menuBar()->hide();
        else
            us->editor_fuzzy_titlebar = false; // 强制关闭
    }

    log("初始化标题栏");
    if (us->integration_window)
    {
        titlebar_widget = new MyTitleBar(this);
        connect(titlebar_widget, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonWinMinClicked()));
        connect(titlebar_widget, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonWinRestoreClicked()));
        connect(titlebar_widget, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonWinMaxClicked()));
        connect(titlebar_widget, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonWinCloseClicked()));
        connect(titlebar_widget, SIGNAL(signalButtonSidebarClicked()), this, SLOT(onButtonSidebarClicked()));
        connect(titlebar_widget, SIGNAL(signalButtonSettingsClicked()), this, SLOT(onButtonSettingsClicked()));
        connect(titlebar_widget, SIGNAL(windowPosChanged(QPoint)), this, SLOT(saveWinPos(QPoint)));
        connect(titlebar_widget, SIGNAL(windowPosMoved(QPoint)), this, SLOT(slotWindowPosMoved(QPoint)));
        connect(titlebar_widget, &MyTitleBar::signalMenuBarShowedChanged, [=](bool b){ this->setMenuBarShowed(b); });
        if (!us->win_btn_showed)
            titlebar_widget->hideWinButtons(us->getBool("view/main_menu_also_hided", true));
        titlebar_widget->raise();
#if !defined(Q_OS_MAC)
        if (us->menu_bar_showed)
        {
            titlebar_widget->setFixedWidgetHeight(menuBar()->height()+3);
            titlebar_widget->getContentLabel()->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
            titlebar_widget->setMarginLeftWidgetShowed(true);
        }
        else
        {
            titlebar_widget->setFixedWidgetHeight(us->widget_size);
        }

        if (!rt->isMainWindowMaxing())
        {
            titlebar_widget->setRightCornerBtnRadius(us->widget_radius);
        }
        connect(rt, &RuntimeInfo::signalMainWindowMaxing, this, [=](bool m) {
            titlebar_widget->setRightCornerBtnRadius(m ? 0 : us->widget_radius);
            titlebar_widget->setRightCornerBtnRadius(m || us->menu_bar_showed || us->side_bar_showed ? 0 : us->widget_radius);
        });
#endif
    }
    else
    {
        titlebar_widget = nullptr;
    }

    log("初始化主菜单");
    {
        main_menu = new MainMenuWidget(this);
        // if (us->integration_window)
            // connect(main_menu, SIGNAL(signalHide()), titlebar_widget, SLOT(slotMenuHide()));
        connect(main_menu, SIGNAL(signalHidded()), titlebar_widget, SLOT(slotMenuHide()));
        connect(main_menu, SIGNAL(signalCloudClicked()), this, SLOT(actionCloudState()));
        connect(main_menu, SIGNAL(signalNightClicked()), this, SLOT(actionNightTheme()));
        connect(main_menu, SIGNAL(signalCloseOpenedClicked()), this, SLOT(actionCloseOpenedChapter()));
        connect(main_menu, SIGNAL(signalSettingsClicked()), this, SLOT(slotShowSettings()));
        connect(main_menu, SIGNAL(signalFullScreenClicked()), this, SLOT(actionFullScreen()));
        connect(main_menu, SIGNAL(signalHelpClicked()), this, SLOT(actionHelp()));
        connect(main_menu, SIGNAL(signalThemeClicked()), this, SLOT(actionThemeManage()));
        connect(main_menu, SIGNAL(signalDarkRoomClicked()), this, SLOT(actionDarkRoom()));
        main_menu->hide();
    }

    log("初始化左右布局");
    sidebar_widget = new SideBarWidget(this);
    dir_layout = new NovelDirGroup(this);
    outline_group = new OutlineGroup(this);
    cardlib_group = new CardlibGroup(this);
    editors_stack = new StackWidget(this);
    editor_blur_win_bg_pixmap = nullptr;
    p_editor_blur_win_bg_pixmap = nullptr;
    QVBoxLayout* extra_tab_layout = new QVBoxLayout(); // margin 需要增加一个 layout
    QHBoxLayout *main_layout = new QHBoxLayout();

    extra_tab_layout->addWidget(editors_stack);
    if (us->integration_window)
    {
        // 标题栏间隔
#if defined(Q_OS_MAC)
        int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ ? 0 : us->widget_size;
#else
        int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ || us->menu_bar_showed/*显示菜单栏（状态栏在右边）*/ ? 0 : us->widget_size;
#endif
        extra_tab_layout->setContentsMargins(0, margin_top, 0, us->getInt("us/margin_bottom", 5)); // 和顶部状态栏的间隔、底部窗口大小调整按钮的间隔

        connect(us, &USettings::signalTitlebarHeightChanged, [=]{
    #if defined(Q_OS_MAC)
            int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ ? 0 : us->widget_size;
    #else
            int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ || us->menu_bar_showed/*显示菜单栏（状态栏在右边）*/ ? 0 : us->widget_size;
    #endif
            extra_tab_layout->setContentsMargins(0, margin_top, 0, us->getInt("us/margin_bottom", 5)); // 和顶部状态栏的间隔、底部窗口大小调整按钮的间隔
        });

        // 标题栏模糊
        /*if (us->editor_fuzzy_titlebar)
        {
            editor_blur_win_bg_pixmap = new QPixmap(editors_stack->width(), us->widget_size);
            p_editor_blur_win_bg_pixmap = &editor_blur_win_bg_pixmap;
            editors_stack->setTitleBar(titlebar_widget, p_editor_blur_win_bg_pixmap);
        }
        else
        {
            editor_blur_win_bg_pixmap = nullptr;
            p_editor_blur_win_bg_pixmap = nullptr;
        }*/
        editor_blur_win_bg_pixmap = new QPixmap(editors_stack->width(), us->widget_size);
        p_editor_blur_win_bg_pixmap = &editor_blur_win_bg_pixmap;
        editors_stack->setTitleBar(titlebar_widget, p_editor_blur_win_bg_pixmap);

        connect(us, &USettings::signalBlurTitlebarChanged, [=](bool b){
            if (editor_blur_win_bg_pixmap == nullptr)
            {
                editor_blur_win_bg_pixmap = new QPixmap(editors_stack->width(), us->widget_size);
                p_editor_blur_win_bg_pixmap = &editor_blur_win_bg_pixmap;
                editors_stack->setTitleBar(titlebar_widget, p_editor_blur_win_bg_pixmap);
            }
            if (b)
                resetTitlebarPixmapSize();

#if defined(Q_OS_MAC)
            int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ ? 0 : us->widget_size;
#else
            int margin_top = us->editor_fuzzy_titlebar/*模糊状态栏*/ || us->menu_bar_showed/*显示菜单栏（状态栏在右边）*/ ? 0 : us->widget_size;
#endif
            extra_tab_layout->setContentsMargins(0, margin_top, 0, us->getInt("us/margin_bottom", 5)); // 和顶部状态栏的间隔、底部窗口大小调整按钮的间隔
        });
    }

    // 侧边栏
    if (us->integration_window) // 一体化窗口
    {
        sidebar_widget->addPage(dir_layout, "目录");
        sidebar_widget->addPage(outline_group, "大纲");
        sidebar_widget->addPage(cardlib_group, "名片");
        main_layout->addWidget(sidebar_widget);

        if (!us->getBool("view/side_bar_showed", true))
        {
            setSidebarShowed(false, false);
        }
    }
    else // 原生窗口：dock栏
    {
        sidebar_widget->hide();

        QDockWidget* novel_dir_dock = new QDockWidget("目录", this);
        novel_dir_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        novel_dir_dock->setWidget(dir_layout);
        addDockWidget(Qt::LeftDockWidgetArea, novel_dir_dock);

        QDockWidget* outline_dock = new QDockWidget("大纲", this);
        outline_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        outline_dock->setWidget(outline_group);
        addDockWidget(Qt::LeftDockWidgetArea, outline_dock);

        QDockWidget *cardlib_dock = new QDockWidget("大纲", this);
        cardlib_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        cardlib_dock->setWidget(cardlib_group);
        addDockWidget(Qt::LeftDockWidgetArea, cardlib_dock);

        tabifyDockWidget(novel_dir_dock, outline_dock);
        tabifyDockWidget(novel_dir_dock, cardlib_dock);
        novel_dir_dock->raise();
    }

    main_layout->addLayout(extra_tab_layout);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);
    if (us->integration_window)
    {
        main_layout->setMargin(us->mainwin_border_size);
        connect(rt, &RuntimeInfo::signalMainWindowMaxing, this, [=](bool m){
            main_layout->setMargin(m ? 0 : us->mainwin_border_size);
        });
    }

    log("读取上次的窗口位置");
    QScreen* desktop = QApplication::screenAt(QCursor::pos());
    QRect screen_rect = desktop->availableGeometry();
    QVariant varPos = us->getVar("layout/win_pos", QPoint(screen_rect.width()/10, screen_rect.height()/10));
    QVariant varSize = us->getVar("layout/win_size", QSize(screen_rect.width()*4/5, screen_rect.height()*4/5));
    QPoint window_pos;
    QSize window_size;
    if (us->have("layout/win_pos") && us->have("layout/win_size") /*&& varPos != NULL && varSize != NULL*/)
    {
        // 调整尺寸
        window_pos = varPos.toPoint();
        window_size = varSize.toSize();
        if (window_pos.x() >= screen_rect.right()) // 超过右边
            window_pos.setX(screen_rect.left() + screen_rect.width()*3/4);
        if (window_pos.x()+window_size.width() < screen_rect.left()) // 超过左边
            window_pos.setX(screen_rect.left() + screen_rect.width()/4);
        if (window_pos.y() >= screen_rect.bottom()) // 超过底边
            window_pos.setY(screen_rect.top() + screen_rect.height()*3/4);
        if (window_pos.y() < 0) // 超过顶边
            window_pos.setY(0);
        if (window_size.width() > screen_rect.width()) // 超过屏幕宽度
            window_size.setWidth(screen_rect.width() * 3 / 4);
        if (window_size.height() > screen_rect.height()) // 超过屏幕高度 // 现在不包括任务栏高度了
            window_size.setHeight(screen_rect.height() * 3 / 4);
    }
    else // 没有默认的坐标数据
    {
        // 获取多显示器中，当前显示器的坐标
        /*QDesktopWidget * desktop = QApplication::desktop();
        int curMonitor = desktop->screenNumber ( this ); // desktop->primaryScreen() 主显示器编号
        QRect rect = desktop->availableGeometry(curMonitor);*/ // availableGeometry() 去掉任务栏后的坐标  screenGeometry  包括任务栏的坐标
        QRect rect = QApplication::screenAt(QCursor::pos())->availableGeometry();
        // 根据显示器的坐标调整初始值
        window_pos = QPoint(rect.left() + rect.width()/4, rect.top() + rect.height()/4);
        window_size = QSize(rect.width()/2, rect.height()/2);
    }
    this->setGeometry(QRect(window_pos, window_size));
//    setMinimumSize(QSize(1,1));
//    setMaximumSize(QApplication::desktop()->size()*2);
//    setFixedSize(window_size);

#if defined(Q_OS_MAC)
    QTimer::singleShot(0, [=]{
        // Mac下会自动适配窗口大小，所以构造函数中的 setGeometry 只能调整位置，大小需要在构造函数后再重新一次设置才行
        setGeometry(QRect(window_pos, window_size));
    });
#endif
    center_widget->setLayout(main_layout);
    // TODO 目前有bug，会挡住一部分作品列表右边的位置，导致无法点击
    slotSideBarWidthChanged(); // 因为本来是titlebar调整大小时发出信号，但是此时没有连接信号槽，只能手动调用这个函数

    /* log("右下角拖动控件");
    if (us->integration_window)
    {
        drag_size_widget = new DragSizeWidget(this);
        drag_size_widget->setFocusPolicy(Qt::FocusPolicy::NoFocus); // 避免抢走焦点
        drag_size_widget->move(this->geometry().width()-drag_size_widget->width(), this->geometry().height()-drag_size_widget->height());
        connect(drag_size_widget, SIGNAL(signalMoveEvent(int, int)), this, SLOT(slotWindowSizeChanged(int,int)));
//        connect(drag_size_widget, SIGNAL(signalReleaseEvent()), editors_stack, SLOT(slotDelayFocus()));
        connect(drag_size_widget, &DragSizeWidget::signalReleased, [=]{
            if (editors_stack->hasFocus())
                editors_stack->slotDelayFocus();
        });
    } */

    log("初始化信号槽事件");
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
    connect(sidebar_widget, SIGNAL(signalWidthChanged()), this, SLOT(slotSideBarWidthChanged()));
    connect(sidebar_widget, SIGNAL(signalWidthChanged()), this, SLOT(resetTitlebarPixmapSize()));

    connect(dir_layout, SIGNAL(signalRenameNovel(QString, QString)), outline_group, SLOT(slotNovelNameChanged(QString, QString)));
    connect(dir_layout, SIGNAL(signalOpenNovel(QString)), outline_group, SLOT(slotLoadOutlineList(QString)));
    connect(dir_layout, SIGNAL(signalDeleteNovel(QString)), outline_group, SLOT(slotNovelDeleted()));
    connect(dir_layout, SIGNAL(signalOpenNovel(QString)), this, SLOT(slotNovelOpened()));
    connect(dir_layout, SIGNAL(signalDeleteNovel(QString)), this, SLOT(slotNovelClosed()));

    connect(dir_layout->getListView(), SIGNAL(signalOpenChapter(QString, QString, QString)), editors_stack, SLOT(slotOpenChapter(QString, QString, QString)));
    connect(dir_layout->getListView()->getModel(), SIGNAL(signalChangeName(bool,QString,QString,QString)), editors_stack, SLOT(slotChangeName(bool,QString,QString,QString)));
    connect(dir_layout->getListView(), SIGNAL(signalChangeFullName(QString,QString,QString)), editors_stack, SLOT(slotChangeFullName(QString,QString,QString)));
    connect(dir_layout->getListView(), SIGNAL(signalOpenAnimationStartPos(QPoint)), editors_stack, SLOT(slotOpenAnimation(QPoint)));
    //connect(novelDirLayout->getListView()->getModel(), SIGNAL(signalDeleteChapter(QString, QString)), tabWidget, SLOT(slotDeleteChapter(QString, QString)));
    connect(dir_layout->getListView(), SIGNAL(signalDeleteChapter(QString,QString)), editors_stack, SLOT(slotDeleteChapter(QString,QString)));
    connect(dir_layout->getListView(), SIGNAL(signalDeleteChapters(QString,QStringList)), editors_stack, SLOT(slotDeleteChapters(QString,QStringList)));
    connect(dir_layout, SIGNAL(signalRenameNovel(QString,QString)), editors_stack, SLOT(slotRenameNovel(QString,QString)));
    connect(dir_layout, SIGNAL(signalDeleteNovel(QString)), editors_stack, SLOT(slotDeleteNovel(QString)));
    connect(dir_layout->getListView(), SIGNAL(signalImExPort(int,QString)), this, SLOT(slotShowImExWindow(int,QString)));

    connect(cardlib_group, SIGNAL(signalAddNewCard()), this, SLOT(slotAddNewCard()));
    connect(editors_stack, SIGNAL(signalEditorSwitched(QTextEdit*)), cardlib_group, SLOT(slotRefreshEditingCards(QTextEdit*)));
    connect(editors_stack, SIGNAL(signalShowCardEditor(CardBean*)), this, SLOT(slotShowCardEditor(CardBean*)));
    connect(editors_stack, SIGNAL(signalAddNewCard()), this, SLOT(slotAddNewCard()));
    connect(outline_group, SIGNAL(signalAddNewCard()), this, SLOT(slotAddNewCard()));
    connect(cardlib_group, SIGNAL(signalNeedRefreshEditingCards()), this, SLOT(slotRefreshEditingCards()));
    connect(cardlib_group, SIGNAL(signalShowCardEditor(CardBean *)), this, SLOT(slotShowCardEditor(CardBean *)));
    connect(outline_group, SIGNAL(signalShowCardEditor(CardBean *)), this, SLOT(slotShowCardEditor(CardBean *)));
    connect(&gd->clm, SIGNAL(signalCardlibClosed()), this, SLOT(slotCardlibClosed()));

    connect(editors_stack, SIGNAL(signalChangeTitleByChapter(QString)), this, SLOT(slotSetWindowTitleBarContent(QString)));
    connect(editors_stack, SIGNAL(signalEditorWordCount(int)), this, SLOT(slotChapterWordCount(int)));
    connect(editors_stack, SIGNAL(signalSetTitleNovel(QString)), this, SLOT(slotSetTitleNovel(QString)));
    connect(editors_stack, SIGNAL(signalSetTitleChapter(QString)), this, SLOT(slotSetTitleChapter(QString)));
    connect(editors_stack, SIGNAL(signalSetTitleFull(QString)), this, SLOT(slotSetTitleFull(QString)));
    // connect(tabWidget, SIGNAL(signalSetTitleWc(int)), this, SLOT(slotSetTitleWc(int)));
    connect(editors_stack, SIGNAL(signalEditorCountChanged(int)), this, SLOT(slotEditorCountChanged(int)));
    connect(editors_stack, SIGNAL(signalShowSearchPanel(QString)), this, SLOT(slotShowSearchPanel(QString)));
    connect(editors_stack, SIGNAL(signalChapterClosed(QString, QString)), dir_layout->getListView(), SLOT(slotChapterClosed(QString, QString)));
    connect(editors_stack, SIGNAL(signalAddNextChapter(QString, QString)), dir_layout->getListView(), SLOT(slotAddNextChapter(QString, QString)));
    connect(editors_stack, SIGNAL(signalAddNextChapterFaster(QString, QString, bool)), dir_layout->getListView(), SLOT(slotAddNextChapterFaster(QString, QString, bool)));
    connect(editors_stack, SIGNAL(signalLocateOutline(QString, QString)), outline_group, SLOT(slotLocateOutline(QString, QString)));
    connect(editors_stack, &StackWidget::signalLocateOutline, [=]{ if (sidebar_widget) { sidebar_widget->setPage(1); setSidebarShowed(true); } });
    connect(editors_stack, SIGNAL(signalLocateChapter(QString, QString)), dir_layout->getListView(), SLOT(slotLocateChapter(QString, QString)));
    connect(editors_stack, &StackWidget::signalLocateChapter, [=]{ if (sidebar_widget) { sidebar_widget->setPage(0); setSidebarShowed(true); } });
    connect(editors_stack, SIGNAL(signalOpenPrevChapter(QString, QString)), dir_layout->getListView(), SLOT(slotOpenPrevChapter(QString, QString)));
    connect(editors_stack, SIGNAL(signalOpenNextChapter(QString, QString)), dir_layout->getListView(), SLOT(slotOpenNextChapter(QString, QString)));
    connect(editors_stack, SIGNAL(signalMoveToPrevChapterEnd(QString, QString, QString)), dir_layout->getListView(), SLOT(slotMoveToPrevChapterEnd(QString, QString, QString)));
    connect(editors_stack, SIGNAL(signalMoveToNextChapterStart(QString, QString, QString)), dir_layout->getListView(), SLOT(slotMoveToNextChapterStart(QString, QString, QString)));

    log("初始化其他窗口");
    user_settings_window = nullptr;  // 设置窗口，等用到时再进行初始化
    user_settings_window2 = nullptr; // 设置窗口，等用到时再进行初始化
    novel_schedule_widget = nullptr; // 一览窗口，同上
    im_ex_window = nullptr;          // 导入导出，同上
    trans_ctn = new TransparentContainer(this);
    search_panel = nullptr;
    connect(dir_layout, SIGNAL(signalShowInfo(QString,QPoint)), this, SLOT(slotShowNovelInfo(QString,QPoint)));
    connect(trans_ctn, SIGNAL(signalToClose(int)), this, SLOT(slotTransContainerToClose(int)));
    connect(trans_ctn, SIGNAL(signalClosed(int)), this, SLOT(slotTransContainerClosed(int)));

    connect(&gd->dr, &DarkRoom::signalOpenDarkRoom, [=]{
        if (!rt->full_screen)
            actionFullScreen();
        if (us->integration_window)
            titlebar_widget->hideWinButtons(false, false);

        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        show(); // 设置完必须要 show 才会显示，否则窗口会消失
    });
    connect(&gd->dr, &DarkRoom::signalCloseDarkRoom, [=]{
        if (rt->full_screen)
            actionFullScreen(); // 退出全屏
#if defined (Q_OS_MAC)
        QTimer::singleShot(2000, [=]{
            if (isFullScreen()) // Mac退出全屏时会出现问题，莫名恢复到全屏
            {
                if (rt->maximum_window)
                    showMaximized();
                else
                    showNormal();
            }
        });
#endif
        if (us->integration_window)
            titlebar_widget->showWinButtons(titlebar_widget->isMaxxing());

        setWindowFlag(Qt::WindowStaysOnTopHint, false);
        show();
    });


    log("判断是否最大化");
    if (us->integration_window)
    {
        titlebar_widget->saveRestoreInfo(window_pos, window_size);
        if (us->contains("layout/win_max") && us->getBool("layout/win_max")) // 最大化
            titlebar_widget->simulateMax(); // TODO 这句话报错

        QTimer::singleShot(500, [=]{
            resetTitlebarPixmapSize();
        });
        QTimer::singleShot(1000, [=]{
            resetTitlebarPixmapSize();
        });
        QTimer::singleShot(2000, [=]{
            resetTitlebarPixmapSize();
        });
    }
    adjustTitlebar();
    log("标题栏调整完毕");

    // 没有打开新书，开启贝塞尔波浪动画
    if (dir_layout->getListView()->getNovelName().isEmpty())
    {
        log("贝塞尔波浪动画");
        initBezierWave();
    }

    initDynamicBackground();
    connect(us, &USettings::signalDynamicBackgroundChanged, [=]{
        initDynamicBackground();
        // us->dynamic_bg->setWidget(this);
        // us->dynamic_bg->redraw();
        rerenderTitlebarPixmap();
    });

    log("初始化通知");
    tip_box = new TipBox(this);
    connect(tip_box, SIGNAL(signalCardClicked(NotificationEntry*)), this, SLOT(slotNotificationCardClicked(NotificationEntry*)));
    connect(tip_box, SIGNAL(signalBtnClicked(NotificationEntry*)), this, SLOT(slotNotificationButtonClicked(NotificationEntry*)));
    connect(rt, SIGNAL(signalPopNotification(NotificationEntry*)), this, SLOT(slotPopNotification(NotificationEntry*)));

    updateUI();

    log("basewindow 初始化完毕");
    // 界面阴影效果（导致平滑滚动特别卡顿）
    /* QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-5, 5);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(8);
    this->setGraphicsEffect(shadow_effect); */
}

void BaseWindow::initMenu()
{

}

void BaseWindow::initDynamicBackground()
{
    if (!us->dynamic_bg_model) return ;
    if (us->dynamic_bg->getWidget() == this) return ;
    log("初始化微动背景");
    us->dynamic_bg->setWidget(this);
    if (us->integration_window)
        us->dynamic_bg->setRadius(us->widget_radius);
    connect(us->dynamic_bg, SIGNAL(signalRedraw()), this, SLOT(update()));
    connect(us->dynamic_bg, SIGNAL(signalRedraw()), this, SLOT(rerenderTitlebarPixmap()));
}

void BaseWindow::resizeEvent(QResizeEvent* event)
{
    if (!us->side_bar_showed && sidebar_widget->geometry().left() >= -10) // 折叠状态
    {
        sidebar_widget->move(-sidebar_widget->geometry().width(), sidebar_widget->geometry().top());
        editors_stack->setGeometry(0, editors_stack->geometry().top(), geometry().width(), editors_stack->geometry().height());
    }
    adjustTitlebar();
    editors_stack->setFocus();

    resetTitlebarPixmapSize();

    /* if (us->integration_window)
        drag_size_widget->move(this->geometry().width()-drag_size_widget->width(), this->geometry().height()-drag_size_widget->height()); */

    tip_box->adjustPosition();

    // 水波纹的位置
    if (bezier_wave_inited)
    {
        QRect rect = geometry();
        bw1->set_rect(rect);
        bw2->set_rect(rect);
        bw3->set_rect(rect);
        bw4->set_rect(rect);
    }

    // 边缘阴影效果
    if (us->integration_window)
    {
        bs_left->adjustGeometry();
        bs_right->adjustGeometry();
        bs_top->adjustGeometry();
        bs_bottom->adjustGeometry();
        bs_topLeft->adjustGeometry();
        bs_topRight->adjustGeometry();
        bs_bottomLeft->adjustGeometry();
        bs_bottomRight->adjustGeometry();
    }

    // 透明蒙版的效果
    if (!trans_ctn->isHidden())
    {
        trans_ctn->setFixedSize(width()-1, height() - 1);
    }

    QMainWindow::resizeEvent(event);
}

void BaseWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (rt->is_line_guide)
        paintEvent(nullptr);

    return QMainWindow::mouseMoveEvent(event);
}

void BaseWindow::keyPressEvent(QKeyEvent *event)
{
    auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;

    Q_UNUSED(ctrl)
    Q_UNUSED(shift)
    Q_UNUSED(alt)

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
        switch (event->key())
        {
        case Qt::Key_P:
            if (ctrl)
            {
                slotShowSearchPanel();
                return ;
            }
            break;
        case Qt::Key_Q:
            if (ctrl)
            {
                onButtonSidebarClicked();
                return ;
            }
            break;
        case Qt::Key_F11:
            actionFullScreen();
            return ;
        case Qt::Key_F12:

            break;
        }
    }

    if (cmds.size() > 0)
        return ;

    return QMainWindow::keyPressEvent(event);
}

void BaseWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // 设置圆角裁剪
    if (us->integration_window && !rt->isMainWindowMaxing() && (us->widget_radius || us->mainwin_border_size))
    {
        QPainterPath clip_path;
        clip_path.addRoundedRect(us->mainwin_border_size, us->mainwin_border_size, width() - us->mainwin_border_size * 2, height() - us->mainwin_border_size*2, us->widget_radius, us->widget_radius);
        // painter.setRenderHint(QPainter::SmoothPixmapTransform, true); // 似乎没啥用啊？
        painter.setClipPath(clip_path, Qt::IntersectClip);
    }

    // ==== 设置大背景 ====
    if (bg_jpg_enabled)
        painter.drawPixmap(0, 0, this->width(), this->height(), bg_jpg);

    if (!us->dynamic_bg_model) // 没有使用动态背景
    {
        QPainterPath path_back;
        path_back.setFillRule(Qt::WindingFill);
        path_back.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillPath(path_back, QBrush(us->mainwin_bg_color));
    }
    else // 使用动态背景
    {
        us->dynamic_bg->draw(painter);
    }

    if (bg_png_enabled)
        painter.drawPixmap(0, 0, this->width(), this->height(), bg_png);

    if (us->envir_picture_enabled)
        us->envir_picture->draw(painter);

    if (us->emotion_filter_enabled)
        us->emotion_filter->draw(painter);

    if (bezier_wave_running)
    {
        QPainterPath bezier;
        painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
        QColor color = us->getOpacityColor(us->accent_color, 50);

        bezier = bw1->getPainterPath(painter);
        painter.fillPath(bezier, QBrush(color));

        bezier = bw2->getPainterPath(painter);
        painter.fillPath(bezier, QBrush(color));

        bezier = bw3->getPainterPath(painter);
        painter.fillPath(bezier, QBrush(color));

        bezier = bw4->getPainterPath(painter);
        painter.fillPath(bezier, QBrush(color));
    }

    /* // ==== 引导 ====
    if (rt->is_line_guide)
    {
        QPoint aim_pos = rt->guide_widget->pos();
        if (rt->line_guide_case == LINE_GUIDE_CREATE_BOOK)
            aim_pos += rt->guide_widget->parentWidget()->pos();
        QPoint cur_pos = QCursor::pos();
        cur_pos = mapFromGlobal(cur_pos);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QColor(34, 139, 34));
        painter.drawLine(cur_pos, aim_pos);
    }*/

    // 绘制阴影

    return QWidget::paintEvent(event);
}

void BaseWindow::onButtonWinMinClicked()
{
    if (gd->dr.isInDarkRoom()) return ;
    if (!us->integration_window) return ;
    // showMinimized();
    if (!titlebar_widget->isMaxxing())
        titlebar_widget->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
#if defined(Q_OS_WIN)
    toWinMinAnimation(); // 等动画结束再最小化
#else
    showMinimized(); // Mac最小化无效
#endif
}

void BaseWindow::onButtonWinMaxClicked()
{
    if (rt->full_screen)
    {
        actionFullScreen();
        return ;
    }

    if (us->integration_window)
    {
        titlebar_widget->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
        /* drag_size_widget->hide(); */
    }
    toWinMaxAnimation();
    us->setVal("layout/win_max", true);
}

void BaseWindow::onButtonWinRestoreClicked()
{
    if (rt->full_screen)
    {
        actionFullScreen();
        return ;
    }

    /* if (us->integration_window)
        drag_size_widget->show(); */
    toWinRestoreAnimation();
    us->setVal("layout/win_max", false);
}

void BaseWindow::onButtonWinCloseClicked()
{
    //close();
    if (gd->dr.isInDarkRoom() && !gd->dr.exitDarkRoom(this))
        return ;
    closeWindowAnimation();
}

void BaseWindow::onButtonSidebarClicked()
{
    if (!sidebar_widget) return ;
    if (sidebar_widget->geometry().left() >= -10 || !sidebar_widget->isHidden()) // 开始隐藏（负）
    {
        setSidebarShowed(false);
        editors_stack->setFocus();
    }
    else // 开始显示
    {
        setSidebarShowed(true);
    }
}

void BaseWindow::setSidebarShowed(bool show, bool animation)
{
    if (!sidebar_widget) return ; // 如果压根就没初始化侧边栏呢？
    if (show != sidebar_widget->isHidden()) return ; // 如果已经显示了
    us->setVal("view/side_bar_showed", us->side_bar_showed = show);
    if (show) // 开始显示
    {
        if (sidebar_widget->isHidden())
        {
            if (animation)
            {
                sidebar_widget->move(-sidebar_widget->width(), sidebar_widget->geometry().top());
                // editors_stack->setGeometry(0, editors_stack->geometry().top(), width(), editors_stack->height()); // 这行代码开启编辑框动画
            }
            sidebar_widget->show();
        }

        if (animation)
        {
            int distance = (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size) - sidebar_widget->geometry().left();
            sidebar_widget->startShowSidebar(distance);

            distance = sidebar_widget->geometry().width()-editors_stack->geometry().left();

            /*QPropertyAnimation *animation = new QPropertyAnimation(editors_stack, "geometry");
            animation->setDuration(200);
            animation->setStartValue(editors_stack->geometry());
            animation->setEndValue(QRect(editors_stack->geometry().left()+distance, editors_stack->geometry().top(), editors_stack->geometry().width()-distance, editors_stack->geometry().height()));
            animation->setEasingCurve(QEasingCurve::InOutQuad);
            animation->start();*/

            if (!us->menu_bar_showed && titlebar_widget!=nullptr)
            {
                /*bool fuzzy = us->editor_fuzzy_titlebar;
                if (fuzzy)
                    emit us->signalBlurTitlebarChanged(false);*/

                titlebar_widget->setMinimumWidth(width() - sidebar_widget->width() - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size)); // 不设置最小值无法进行缩小动画啊
                if (!(us->editor_fuzzy_titlebar && (bg_jpg_enabled || bg_png_enabled)))
                {
                    QPropertyAnimation* titlebar_animation = new QPropertyAnimation(titlebar_widget, "geometry");
                    titlebar_animation->setDuration(200);
                    titlebar_animation->setStartValue(titlebar_widget->geometry());
                    titlebar_animation->setEndValue(QRect(sidebar_widget->width(), titlebar_widget->geometry().top(), width() - sidebar_widget->width()-(rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), us->widget_size));
                    titlebar_animation->start();
                    connect(titlebar_animation, SIGNAL(finished()), titlebar_animation, SLOT(deleteLater()));
                }
                else
                {
                    titlebar_widget->setGeometry(sidebar_widget->width(), titlebar_widget->geometry().top(), width() - sidebar_widget->width() - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), us->widget_size);
                    resetTitlebarPixmapSizeImediately();
                }

                /*if (fuzzy)
                    connect(titlebar_animation, &QPropertyAnimation::finished, [=]{
                        emit us->signalBlurTitlebarChanged(true);
                    });*/
            }
            else
            {
                adjustTitlebar();
            }
        }
        else
        {
            sidebar_widget->move( (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), sidebar_widget->geometry().top());
            adjustTitlebar();
        }
        titlebar_widget->setLeftCornerBtnRadius(0);
    }
    else // 开始隐藏
    {
        int distance = sidebar_widget->geometry().left() - sidebar_widget->geometry().width(); // 负的
        sidebar_widget->startHideSidebar(distance);

        distance = editors_stack->geometry().left(); // 正的
        if (animation)
        {
            if (!(us->editor_fuzzy_titlebar && (bg_jpg_enabled || bg_png_enabled)))
            {
                QPropertyAnimation *animation = new QPropertyAnimation(editors_stack, "geometry");
                animation->setDuration(200);
                animation->setStartValue(editors_stack->geometry());
                animation->setEndValue(QRect(editors_stack->geometry().left() - distance + (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), editors_stack->geometry().top(), editors_stack->geometry().width() + distance, editors_stack->geometry().height()));
                animation->setEasingCurve(QEasingCurve::InOutQuad);
                animation->start();
                connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
            }
            else
            {
                editors_stack->setGeometry(editors_stack->geometry().left()-distance+(rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), editors_stack->geometry().top(), editors_stack->geometry().width()+distance, editors_stack->geometry().height());
                resetTitlebarPixmapSizeImediately();
            }

            // 不知道为什么不需要状态栏动画诶？
            // 开启后，状态栏宽度会莫名减少侧边栏宽度，导致不能全宽度
            if (!us->menu_bar_showed && titlebar_widget!=nullptr)
            {
                /*bool fuzzy = us->editor_fuzzy_titlebar;
                if (fuzzy)
                    emit us->signalBlurTitlebarChanged(false);*/

                if (!(us->editor_fuzzy_titlebar && (bg_jpg_enabled || bg_png_enabled)))
                {
                    QPropertyAnimation* titlebar_animation = new QPropertyAnimation(titlebar_widget, "geometry");
                    titlebar_widget->setMaximumWidth(width() - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size*2));
                    titlebar_animation->setDuration(200);
                    titlebar_animation->setStartValue(titlebar_widget->geometry());
                    titlebar_animation->setEndValue(QRect((rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), titlebar_widget->geometry().top(), width() - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size*2), us->widget_size));
                    titlebar_animation->start();
                    connect(titlebar_animation, SIGNAL(finished()), titlebar_animation, SLOT(deleteLater()));
                }
                else
                {
                    titlebar_widget->setGeometry((rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), titlebar_widget->geometry().top(), width() - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size*2), us->widget_size);
                    resetTitlebarPixmapSizeImediately();
                }

                /*if (fuzzy)
                    connect(titlebar_animation, &QPropertyAnimation::finished, [=]{
                        emit us->signalBlurTitlebarChanged(true);
                    });*/
            }
            else
            {
                adjustTitlebar();
            }

            // 动画结束后，隐藏侧边栏（免得不知道什么时候就跳出来显示了）
            QTimer::singleShot(200, [=]{
                sidebar_widget->hide();
            });
        }
        else
        {
            sidebar_widget->move(-sidebar_widget->width(), sidebar_widget->geometry().top());
            sidebar_widget->hide();
            adjustTitlebar();
        }
        if (!us->menu_bar_showed)
        {
            titlebar_widget->setLeftCornerBtnRadius(rt->isMainWindowMaxing() ? 0 : us->widget_radius);
        }
        if (editors_stack->getCurrentEditor() != nullptr)
        {
            QTimer::singleShot(0, [=]{
                if (editors_stack->getCurrentEditor() != nullptr) // 重新判断一次，万一以后会有什么操作关掉编辑框了呢
                    editors_stack->getCurrentEditor()->setFocus();
            });
        }
    }
    if (animation)
    {
        QTimer::singleShot(200, [=]{
            resetTitlebarPixmapSize();
        });
    }

    if (titlebar_widget)
    {
        titlebar_widget->setSidebarButtonState(!show);
    }
}

void BaseWindow::setMenuBarShowed(bool show)
{
    if (show) // 显示菜单
    {
        menuBar()->show();
#if !defined(Q_OS_MAC)
        if (titlebar_widget != nullptr)
        {
            titlebar_widget->setFixedWidgetHeight(menuBar()->height()+3);
            titlebar_widget->getContentLabel()->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
            titlebar_widget->setMarginLeftWidgetShowed(true);
            titlebar_widget->setLeftCornerBtnRadius(0);
        }
        if (us->editor_fuzzy_titlebar)
        {
            emit us->signalBlurTitlebarChanged(us->editor_fuzzy_titlebar = false);
        }
#endif
    }
    else // 隐藏菜单
    {
        menuBar()->hide();
#if !defined(Q_OS_MAC)
        if (titlebar_widget != nullptr)
        {
            titlebar_widget->setFixedWidgetHeight(us->widget_size);
            titlebar_widget->getContentLabel()->setAlign(Qt::AlignCenter);
            titlebar_widget->setMarginLeftWidgetShowed(false);
            titlebar_widget->setLeftCornerBtnRadius(rt->isMainWindowMaxing() || us->side_bar_showed ? 0 : us->widget_radius);
        }
        if (!us->editor_fuzzy_titlebar && us->getBool("us/editor_fuzzy_titlebar", true))
        {
            emit us->signalBlurTitlebarChanged(us->editor_fuzzy_titlebar = true);
        }
#endif
    }

    us->menu_bar_showed = show;
    us->setVal("view/menu_bar_showed", show);

    adjustTitlebar(); // 菜单栏导致标题栏位置改变
    emit us->signalTitlebarHeightChanged(titlebar_widget == nullptr ? 0 : titlebar_widget->height());
}

void BaseWindow::onButtonSettingsClicked()
{
    if (main_menu->isHidden())
        slotShowMainMenu();
    else
        slotHideMainMenu();
}

void BaseWindow::adjustTitlebar()
{
    if (titlebar_widget == nullptr) return ;

#if defined(Q_OS_MAC)
    int margin_left = us->side_bar_showed ? sidebar_widget->geometry().right() : 0;
#else
    int margin_left = us->menu_bar_showed
            ? (menu_bar_right)
            : (us->side_bar_showed ? sidebar_widget->geometry().right()
                : (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size));
#endif
    int margin_top = (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size); //us->menu_bar_showed?menuBar()->height():0;
    titlebar_widget->setMinimumWidth(width()-margin_left - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size));
    titlebar_widget->setGeometry(margin_left, margin_top, width() - margin_left - (rt->isMainWindowMaxing() ? 0 : us->mainwin_border_size), titlebar_widget->height());
}

void BaseWindow::slotShowMainMenu()
{
    QPoint cs_pos = mapFromGlobal(QCursor::pos());
    QPoint sh_pos(cs_pos);
    if (sh_pos.x() + main_menu->width() >= size().width())
        sh_pos.setX(sh_pos.x() - main_menu->width());
    if (sh_pos.x()<0)
        sh_pos.setX(cs_pos.x());
    if (sh_pos.y() < us->widget_size)
        sh_pos.setY(us->widget_size);
    main_menu->move(mapToGlobal(sh_pos));

    main_menu->setOpenedChapterCount(editors_stack->count());

    main_menu->toShow();
}

void BaseWindow::slotHideMainMenu()
{
    // main_menu->toHide();
}

void BaseWindow::slotShowSettings()
{
    return slotShowSettings2();

    if (user_settings_window == nullptr)
    {
        initUserSettingsWindow();
    }

    int left = this->geometry().left(), top = this->geometry().top();
    int width = this->width(), height = this->height();
    if (us->settings_in_mainwin)
    {
        user_settings_window->setGeometry(width/10,height/10, width*4/5, height*4/5);
    }
    else
    {
        user_settings_window->setGeometry(left+width/10,top+height/10, width*4/5, height*4/5);
        user_settings_window->show();
        return ;
    }
    callback_point = mapFromGlobal(QCursor::pos());
    user_settings_window->show();
    QPoint po(callback_point);
    QRect er(user_settings_window->geometry());
    QPixmap* pixmap = new QPixmap(er.size());
    user_settings_window->render(pixmap);
    ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, QRect(po, QSize(1,1)), er, QEasingCurve::Linear, 200, 1);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
        user_settings_window->show();
        user_settings_window->setFocus();
        delete pixmap;
    });
    user_settings_window->hide();
    editors_stack->stackUnder(trans_ctn);
    trans_ctn->toShow();

    /*QPropertyAnimation *animation = new QPropertyAnimation(userSettingsWindow, "geometry");
    animation->setDuration(150);
    QRect startR = titlebar_widget->getBtnRect();
    animation->setStartValue(QRect(QCursor::pos(), QSize(userSettingsWindow->width()>>1, userSettingsWindow->height()>>1)));
    animation->setEndValue(userSettingsWindow->geometry());
    animation->start();*/
}

void BaseWindow::slotShowSettings2()
{
    if (user_settings_window2 == nullptr)
    {
        initUserSettingsWindow2();
    }

    int left = this->geometry().left(), top = this->geometry().top();
    int width = this->width(), height = this->height();
    if (us->settings_in_mainwin)
    {
        user_settings_window2->setGeometry(width / 10, height / 10, width * 4 / 5, height * 4 / 5);
    }
    else
    {
        user_settings_window2->setGeometry(left + width / 10, top + height / 10, width * 4 / 5, height * 4 / 5);
        user_settings_window2->show();
        return;
    }
    callback_point = mapFromGlobal(QCursor::pos());
    user_settings_window2->show();
    QPoint po(callback_point);
    QRect er(user_settings_window2->geometry());
    QPixmap *pixmap = new QPixmap(er.size());
    user_settings_window2->render(pixmap);
    ZoomGeometryAnimationWidget *ani = new ZoomGeometryAnimationWidget(this, pixmap, QRect(po, QSize(1, 1)), er, QEasingCurve::Linear, 255, 1);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=] {
        user_settings_window2->show();
        user_settings_window2->setFocus();
        delete pixmap;
    });
    user_settings_window2->hide();
    editors_stack->stackUnder(trans_ctn);
    trans_ctn->toShow();
}

void BaseWindow::slotShowShortcutWindow()
{
    static ShortcutWindow* scw = new ShortcutWindow(this);
    scw->show();
}

void BaseWindow::toWinMin()
{
    showMinimized();

    if (titlebar_widget != nullptr && titlebar_widget->isMaxxing())
    {
        toWinMax();
    }
    else
    {
        toWinRestore();
    }
}

void BaseWindow::toWinMax()
{
    QScreen* desktop = QApplication::screenAt(QCursor::pos());
    QRect desktop_rect = desktop->availableGeometry();
    QRect fact_rect = QRect(desktop_rect.x() - 3, desktop_rect.y() - 3, desktop_rect.width() + 6, desktop_rect.height() + 6);
    setGeometry(fact_rect);
    adjustTitlebar();
    us->setVal("layout/win_max", true);
    emit rt->signalMainWindowMaxing(true);
}

void BaseWindow::toWinRestore()
{
    QPoint window_pos;
    QSize window_size;
    titlebar_widget->getRestoreInfo(window_pos, window_size);
    this->setGeometry(QRect(window_pos, window_size));
    us->setVal("layout/win_max", false);
}

void BaseWindow::rerenderTitlebarPixmap()
{
    if (!us->editor_fuzzy_titlebar) return ;

     // QTimer::singleShot(0, [=]{ // 必须要延迟，不然会报错，无法 render 或者 paint // BUG: 如果绘制有问题，可以尝试注释掉延迟（但可能会崩溃）
        editor_blur_win_bg_pixmap->fill(QColor(0,0,0,0));
        QRect rect(editors_stack->geometry().left(), editors_stack->geometry().top(), editors_stack->width(), us->widget_size);
        render(editor_blur_win_bg_pixmap, QPoint(0, 0), rect, QWidget::DrawWindowBackground);

        ChapterEditor* e = editors_stack->getCurrentEditor();
        if (e)
            e->blurTitleBar();
     // });
}

void BaseWindow::rerenderTitlebarPixmapLater()
{
    if (!us->editor_fuzzy_titlebar) return ;

     QTimer::singleShot(0, [=]{ // 必须要延迟，不然会报错，无法 render 或者 paint // BUG: 如果绘制有问题，可以尝试注释掉延迟（但可能会崩溃）
        editor_blur_win_bg_pixmap->fill(QColor(0,0,0,0));
        QRect rect(editors_stack->geometry().left(), editors_stack->geometry().top(), editors_stack->width(), us->widget_size);
        render(editor_blur_win_bg_pixmap, QPoint(0, 0), rect, QWidget::DrawWindowBackground);

        ChapterEditor* e = editors_stack->getCurrentEditor();
        if (e)
            e->blurTitleBar();
     });
}

void BaseWindow::resetTitlebarPixmapSize()
{
    if (!us->editor_fuzzy_titlebar || !us->integration_window) return ;

    QTimer::singleShot(10, [=]{
        *editor_blur_win_bg_pixmap = QPixmap(editors_stack->width(), us->widget_size);
        rerenderTitlebarPixmap();
    });
}

void BaseWindow::resetTitlebarPixmapSizeImediately()
{
    if (!us->editor_fuzzy_titlebar || !us->integration_window) return ;
    *editor_blur_win_bg_pixmap = QPixmap(titlebar_widget->size());
//    rerenderTitlebarPixmap();
    {
        editor_blur_win_bg_pixmap->fill(QColor(0,0,0,0));
        render(editor_blur_win_bg_pixmap, QPoint(0, 0), QRect(editors_stack->geometry().left(), editors_stack->geometry().top(), editors_stack->width(), us->widget_size), QWidget::DrawWindowBackground);

        ChapterEditor* e = editors_stack->getCurrentEditor();
        if (e)
            e->blurTitleBar();
    }
}

void BaseWindow::initBezierWave()
{
    bezier_wave_inited = true;
    bezier_wave_running = true;

    bw1 = new BezierWaveBean(this);
    bw1->set_offsety(geometry().height()/10);
    bw1->set_speedx(7);
    bw1->start();

    bw2 = new BezierWaveBean(this);
    bw2->set_offsety(geometry().height()/10);
    bw2->set_speedx(6);
    bw2->start();

    bw3 = new BezierWaveBean(this);
    bw3->set_offsety(geometry().height()*3/20);
    bw3->set_speedx(8);
    bw3->start();

    bw4 = new BezierWaveBean(this);
    bw4->set_offsety(geometry().height()/5);
    bw4->set_speedx(5);
    bw4->start();
}

void BaseWindow::initBorderShadow()
{
    if (!us->integration_window)
        return ;
    bs_left = new BorderShadow(this, BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_right = new BorderShadow(this, BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);
    bs_top = new BorderShadow(this, BORDER_TOP, us->mainwin_border_size, us->widget_radius);
    bs_bottom = new BorderShadow(this, BORDER_BOTTOM, us->mainwin_border_size, us->widget_radius);
    bs_topLeft = new BorderShadow(this, BORDER_TOP | BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_topRight = new BorderShadow(this, BORDER_TOP | BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);
    bs_bottomLeft = new BorderShadow(this, BORDER_BOTTOM | BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_bottomRight = new BorderShadow(this, BORDER_BOTTOM | BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);

    connect(rt, &RuntimeInfo::signalMainWindowMaxing, this, [=](bool m){
        slotSwitchWindowShadow(!m);
    });
}

void BaseWindow::slotSwitchWindowShadow(bool s)
{
    if (!us->integration_window)
        return;
    if (s)
    {
        bs_left->show();
        bs_right->show();
        bs_top->show();
        bs_bottom->show();
        bs_topLeft->show();
        bs_topRight->show();
        bs_bottomLeft->show();
        bs_bottomRight->show();
    }
    else
    {
        bs_left->hide();
        bs_right->hide();
        bs_top->hide();
        bs_bottom->hide();
        bs_topLeft->hide();
        bs_topRight->hide();
        bs_bottomLeft->hide();
        bs_bottomRight->hide();
    }
}

void BaseWindow::slotNovelOpened()
{
    if (bezier_wave_running)
    {
        //bezier_wave_running = false;
        bw1->pause();
        bw2->pause();
        bw3->pause();
        bw4->pause();
        QTimer::singleShot(3000, [=]{
            if (!dir_layout->getListView()->getNovelName().isEmpty())
                bezier_wave_running = false;
        });
    }
}

void BaseWindow::slotNovelClosed()
{
    if (!bezier_wave_inited)
    {
        initBezierWave();
    }
    else
    {
        bezier_wave_running = true;
        bw1->resume();
        bw2->resume();
        bw3->resume();
        bw4->resume();
    }
}

void BaseWindow::slotEditorCountChanged(int x)
{

}

void BaseWindow::showWindowAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(300);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
    connect(animation, &QPropertyAnimation::finished, [=]{
        animation->deleteLater();
    });
}

void BaseWindow::closeWindowAnimation()
{
   QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
   animation->setDuration(300);
   animation->setStartValue(1);
   animation->setEndValue(0);
   animation->start();
   connect(animation, SIGNAL(finished()), this, SLOT(close()));
    connect(animation, &QPropertyAnimation::finished, [=]{
        animation->deleteLater();
    });
}

void BaseWindow::toWinMinAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().x(),geometry().y()+height()/2,width(),0));
    animation->start();
    connect(animation, SIGNAL(finished()), this, SLOT(toWinMin()));
    connect(animation, &QPropertyAnimation::finished, [=]{
        animation->deleteLater();
    });
}

void BaseWindow::toWinMaxAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(QRect(geometry().left(), geometry().top(), geometry().width(), geometry().height()));
    QRect desktopRect = QApplication::screenAt(QCursor::pos())->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    animation->setEndValue(FactRect);
    animation->start();
    connect(animation, SIGNAL(finished()), this, SLOT(toWinMax()));
    connect(animation, &QPropertyAnimation::finished, [=]{
        animation->deleteLater();
    });
}

void BaseWindow::toWinRestoreAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(QRect(geometry().left(), geometry().top(), geometry().width(), geometry().height()));
    QPoint point;
    QSize size;
    titlebar_widget->getRestoreInfo(point, size);
    animation->setEndValue(QRect(point, size));
    animation->start();
    connect(animation, &QPropertyAnimation::finished, [=]{
        adjustTitlebar();
        animation->deleteLater();
        emit rt->signalMainWindowMaxing(false);
    });
}

void BaseWindow::detectUpdate()
{

}

void BaseWindow::initUserSettingsWindow()
{
    if (us->settings_in_mainwin) // 界面嵌入到主窗口
    {
        user_settings_window = new UserSettingsWindow(this);
    }
    else
    {
        user_settings_window = new UserSettingsWindow(this);
        user_settings_window->setWindowFlags(Qt::Dialog);
        user_settings_window->setWindowModality(Qt::WindowModal);
    }

    connect(user_settings_window, SIGNAL(signalEditorBottomSettingsChanged()), editors_stack, SLOT(slotEditorBottomSettingsChanged()));
    connect(user_settings_window, SIGNAL(signalEsc()), trans_ctn, SLOT(slotEsc()));
    connect(user_settings_window, &UserSettingsWindow::signalDetectUpdate, [=]{
        detectUpdate(); // 检查更新
    });
    connect(trans_ctn, &TransparentContainer::signalClosed, [=]{
        rerenderTitlebarPixmap();
    });
    connect(user_settings_window, SIGNAL(signalShortcutKey()), this, SLOT(slotShowShortcutWindow()));
}

void BaseWindow::initUserSettingsWindow2()
{
    if (us->settings_in_mainwin) // 界面嵌入到主窗口
    {
        user_settings_window2 = new UserSettingsWindow2(this);
    }
    else
    {
        user_settings_window2 = new UserSettingsWindow2(this);
        user_settings_window2->setWindowFlags(Qt::Dialog);
        user_settings_window2->setWindowModality(Qt::WindowModal);
    }

    connect(user_settings_window2, SIGNAL(signalEditorBottomSettingsChanged()), editors_stack, SLOT(slotEditorBottomSettingsChanged()));
    connect(user_settings_window2, SIGNAL(signalEsc()), trans_ctn, SLOT(slotEsc()));
    connect(user_settings_window2, &UserSettingsWindow2::signalDetectUpdate, [=] {
        detectUpdate(); // 检查更新
    });
    connect(trans_ctn, &TransparentContainer::signalClosed, [=] {
        rerenderTitlebarPixmap();
    });
    connect(user_settings_window2, SIGNAL(signalShortcutKey()), this, SLOT(slotShowShortcutWindow()));
}

void BaseWindow::initNovelScheduleWindow()
{
    novel_schedule_widget = new NovelScheduleWidget(dir_layout->getListView(), this);

    // 初始化信号槽
    connect(novel_schedule_widget, &NovelScheduleWidget::signalToHide, [=]{
        novel_schedule_widget->hide();
        trans_ctn->hide();
    });
    connect(novel_schedule_widget, SIGNAL(signalChangeNovelName()), dir_layout, SLOT(slotRenameNovel()));
    connect(dir_layout, SIGNAL(signalRenameNovel(QString,QString)), novel_schedule_widget, SLOT(slotChangeNovelNameFinished(QString,QString)));
    connect(novel_schedule_widget, SIGNAL(signalDeleteNovel()), dir_layout, SLOT(slotDeleteNovel()));
    connect(novel_schedule_widget, SIGNAL(signalImExPort(int,QString)), this, SLOT(slotShowImExWindow(int,QString)));
    connect(novel_schedule_widget, SIGNAL(signalDirSettings()), dir_layout->getListView(), SLOT(slotDirSettings()));
    connect(novel_schedule_widget, SIGNAL(signalEsc()), trans_ctn, SLOT(slotEsc()));
    connect(novel_schedule_widget, SIGNAL(signalRefreshBookList(QString)), dir_layout, SLOT(readBookList(QString)));
}

void BaseWindow::initSearchWidget()
{
    search_panel = new SearchPanel(this);

    //search_panel->setWindowFlags(Qt::Dialog);
    //search_panel->setWindowModality(Qt::WindowModal);

    connect(search_panel, SIGNAL(signalInsertText(QString)), this, SLOT(slotInsertText(QString)));
    connect(search_panel, SIGNAL(signalInsertTextWithSuffix(QString,QString)), this, SLOT(slotInsertTextWithSuffix(QString,QString)));
    connect(search_panel, SIGNAL(signalLocateEdit(int, int, int)), this, SLOT(slotLocateEdit(int, int, int)));
    connect(search_panel, SIGNAL(signalHide()), this, SLOT(slotHideSearchPanel()));
    connect(search_panel, SIGNAL(signalEmitCommand(QString)), this, SLOT(slotEmitCommand(QString)));
    connect(search_panel, SIGNAL(signalOpenCard(CardBean*)), this, SLOT(slotShowCardEditor(CardBean*)));
}

bool BaseWindow::execCommand(QString command)
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

bool BaseWindow::execCommand(QString command, QStringList args)
{
    log("执行命令", command);

    if (command == "设置")
    {
        slotShowSettings();
    }
    else if (command == "显示/隐藏侧边栏")
    {
        /*if (us->integration_window)
            titlebar_widget->onButtonSidebarClicked();
        else
            onButtonSidebarClicked();*/
        onButtonSidebarClicked();
    }
    else if (command == "显示目录" || command == "显示大纲" || (command == "侧边栏" && args.size() >= 1))
    {
        /*if (sidebar_widget->isHidden())
        {
            if (us->integration_window)
                titlebar_widget->onButtonSidebarClicked();
            else
                onButtonSidebarClicked();
        }*/
        setSidebarShowed(true);
        if (command == "显示目录" || (command == "侧边栏" && args.at(0) == "目录"))
        {
            sidebar_widget->setPage(0);
        }
        else if (command == "显示大纲" || (command == "侧边栏" && args.at(0) == "大纲"))
        {
            sidebar_widget->setPage(1);
        }
        else
            QMessageBox::warning(this, tr("侧边栏页面出错"), "没有页面<参数1>(" + args.at(0) + ")");
    }
    else if (command == "显示/隐藏菜单栏")
    {
        setMenuBarShowed(!us->menu_bar_showed);
    }
    else if (command == "显示/隐藏窗口按钮")
    {
        if (us->integration_window)
            if (!titlebar_widget->isWinButtonHidden())
                titlebar_widget->hideWinButtons(true);
            else
                titlebar_widget->showWinButtons(titlebar_widget->isMaxxing());
        else
            ; // 非一体化界面，无法隐藏按钮
    }
    else if (command == "最大化")
    {
        if (us->integration_window)
        {
            if (titlebar_widget->isMaxxing())
                titlebar_widget->onButtonRestoreClicked();
            else
                titlebar_widget->onButtonMaxClicked();
        }
        else
            toWinMax();
    }
    else if (command == "最小化")
    {
        showMinimized();
    }
    else if (command == "关闭程序" || command == "退出程序")
    {
        close();
    }
    else if (command == "日/夜间模式")
    {
        actionNightTheme();
    }
    else if (command == "小黑屋")
    {
        actionDarkRoom();
    }
    else if (command == "全屏")
    {
        actionFullScreen();
    }
    else if (command == "云同步")
    {
        actionCloudState();
    }
    else if (command == "关闭已打开的章节")
    {
        actionCloseOpenedChapter();
    }
    else if (command == "官网")
    {
        actionHelp();
    }
    else if (command == "码字风云榜")
    {
        QDesktopServices::openUrl(QUrl(QLatin1String("http://writerfly.cn/")));
    }
    else if (command == "安装路径")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath(), QUrl::TolerantMode)))
#endif
#if defined (Q_OS_LINUX)
//        qDebug() << "dde-file-manager \"" + QApplication::applicationDirPath() + "\"";
//        if (!QProcess::execute("dde-file-manager \"" + QApplication::applicationDirPath() + "\""))
#endif
            if (QMessageBox::information(this, "安装路径", "路径：" + QApplication::applicationDirPath(), "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(QApplication::applicationDirPath());

    }
    else if (command == "数据路径")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(rt->DATA_PATH , QUrl::TolerantMode)))
#endif
            if (QMessageBox::information(this, "数据路径", "路径：" + rt->DATA_PATH, "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(rt->DATA_PATH);
    }
    else if (command == "小说路径")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(rt->NOVEL_PATH, QUrl::TolerantMode)))
#endif
            if (QMessageBox::information(this, "小说路径", "路径：" + rt->NOVEL_PATH, "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(rt->NOVEL_PATH);
    }
    else if (command == "主题路径")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(rt->THEME_PATH, QUrl::TolerantMode)))
#endif
            if (QMessageBox::information(this, "主题路径", "路径：" + rt->THEME_PATH, "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(rt->THEME_PATH);
    }
    else if (command == "工具路径")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(rt->APP_PATH+"/tools", QUrl::TolerantMode)))
#endif
            if (QMessageBox::information(this, "工具路径", "路径：" + rt->APP_PATH+"/tools", "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(rt->APP_PATH+"/tools");
    }
    else if (command == "自定义样式表")
    {
#if defined(Q_OS_WIN)
        if (!QDesktopServices::openUrl(QUrl(rt->STYLE_PATH, QUrl::TolerantMode)))
#endif
            if (QMessageBox::information(this, "样式表路径", "路径：" + rt->STYLE_PATH, "复制", "确定", 0, 1) == 0)
                QApplication::clipboard()->setText(rt->STYLE_PATH);
    }
    else if (command == "刷新主题")
    {
        thm->updateGlobal();
        thm->updateWindow();
        thm->updateEditor();
        thm->updateHighlight();
    }
    else if (command == "一体化界面")
    {
        us->setVal("us/integration_window", !us->integration_window);
        QMessageBox::information(this, tr("一体化界面"), tr("重启生效，享受彻底不同的界面！"));
    }
    else if (command == "逃离小黑屋")
    {
        qint64 timestamp = getTimestamp();
        long long mid5 = timestamp / 100000 % 100000;
        if (args.size() < 1) return false;
        if (QString::number(mid5*mid5/100%100000) == args.at(0) || QString::number((mid5-1)*(mid5-1)/100%100000) == args.at(0))
        {
            if (gd->dr.isInDarkRoom())
            {
                gd->dr.finishDarkRoomForce();
                gd->dr.exitDarkRoom(this);
            }
        }
    }
    else if (command == "离线升级")
    {
        QString file_path = QFileDialog::getOpenFileName(this, "选择下载的安装包", us->getStr("recent/offline_update_path"), tr("Zip files (*.zip)"));
        if (file_path.isEmpty())
            return true;
        us->setVal("recent/offline_update_path", QFileInfo(file_path).dir().absolutePath());

        ensureDirExist(rt->DOWNLOAD_PATH);
        copyFile2(file_path, rt->DOWNLOAD_PATH+"update.zip");

        NotificationEntry *noti = new NotificationEntry("update", "升级", "已选择升级包，关闭程序后自动升级。");
        noti->setBtn(1, "立即关闭并升级", "");
        tip_box->createTipCard(noti);
        connect(noti, &NotificationEntry::signalBtnClicked, [=](int x) {
            if (x == 1)
            {
                this->close();
            }
        });
    }
    else if (command == "时间戳")
    {
        QMessageBox::information(this, tr("时间戳"), QString::number(getTimestamp()));
    }
    else
        return false;

    return true;
}

void BaseWindow::slotShowSearchPanel(QString str)
{
    if (search_panel == nullptr)
        initSearchWidget();
    search_panel->setEditors(editors_stack->getCurrentEditor(), editors_stack->getEditors());
    search_panel->show();

    QTextEdit* edit = editors_stack->getCurrentEditor();
    if (str.isEmpty() && edit != nullptr)
    {
        QTextCursor tc = edit->textCursor();
        if (tc.hasSelection())
        {
            QString text = tc.selectedText();
            if (text.length() < 20)
                search_panel->setText(text); // 设置默认搜索的关键词
        }
    }
    else
        search_panel->setText(str);
}

void BaseWindow::slotHideSearchPanel()
{

}

QString BaseWindow::loadFontFamilyFromFiles(const QString &fontFilePath)
{
    QString font = "";

    QFile fontFile(fontFilePath);
    if(!fontFile.open(QIODevice::ReadOnly))
    {
        MAINDBG << log("!!!打开字体文件出错");
        return font;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if(!loadedFontFamilies.empty())
    {
        font = loadedFontFamilies.at(0);
    }
    fontFile.close();
    return font;
}

void BaseWindow::initEmotionFilter()
{
    // 情景滤镜
    if (us->emotion_filter_enabled)
    {
        QString path = rt->DATA_PATH+"database/emotion_filter.txt";
        if (!isFileExist(path))
            path = ":/database/emotion_filter";
        QString extra_dir = rt->DATA_PATH+"database/emotion_filter_extra";
        us->emotion_filter = new EmotionFilter(path, extra_dir, this);
        connect(us->emotion_filter, SIGNAL(signalRedraw()), this, SLOT(update()));
        connect(us->emotion_filter, SIGNAL(signalRedraw()), this, SLOT(rerenderTitlebarPixmap()));
    }
    else
    {
        if (us->emotion_filter != nullptr)
            us->emotion_filter = nullptr;
    }
}

void BaseWindow::initEnvironmentPictures()
{
    // 场景图片
    if (us->envir_picture_enabled)
    {
        QString path = rt->DATA_PATH+"database/environment_pictures.txt";
        if (!isFileExist(path))
            path = ":/database/environment_pictures";
        QString dir = rt->DATA_PATH+"database/environment_pictures/";
        QString extra_dir = rt->DATA_PATH+"database/environment_pictures_extra/";
        us->envir_picture = new EnvironmentPictures(path, dir, extra_dir, this);
        connect(us->envir_picture, SIGNAL(signalRedraw()), this, SLOT(update()));
        connect(us->envir_picture, SIGNAL(signalRedraw()), this, SLOT(rerenderTitlebarPixmap()));
    }
    else
    {
        if (us->envir_picture != nullptr)
            us->envir_picture = nullptr;
    }
}

void BaseWindow::closeEvent(QCloseEvent *event)
{
    // 小黑屋禁止退出
    if (gd->dr.isInDarkRoom())
    {
        if (!gd->dr.exitDarkRoom(this))
        {
            event->ignore();
            this->show();
            return ;
        }
    }

    // 保存各项滚动位置
    editors_stack->slotSavePosition();
    int dir_scroll = dir_layout->getListView()->verticalScrollBar()->sliderPosition();
    if (dir_scroll >= 0)
        us->setVal("recent/dir_scroll", dir_scroll);

    // 保存窗口位置大小
    if (!rt->maximum_window && !rt->full_screen)
    {
        us->setVal("layout/win_pos", this->pos());
        us->setVal("layout/win_size", this->size());
    }

    // 保存所有设置
    us->sync();

    // 清除临时文件夹
    if (isFileExist(rt->DATA_PATH + "temp"))
        deleteFile(rt->DATA_PATH+"temp");

    // 最后的云同步
    if (!rt->is_exiting && ac->syncBeforeExit())
    {
        this->hide(); // 隐藏起来避免操作
        event->ignore();
        return ;
    }

    // 更新历史版本
    if (isFileExist(rt->DOWNLOAD_PATH + "update.zip")) // 现在已经移动到了 basewindow 这个地方
    {
        log("检测到已下载的安装包，进行更新");
        QProcess process;
        process.startDetached("UpUpTool.exe");
    }
}

void BaseWindow::changeEvent(QEvent *event)
{
    if(event->type()!=QEvent::WindowStateChange)
    {
        if (rt->full_screen && !this->isFullScreen()) // 手动关闭全屏按钮，而不是应用自带的全屏
        {
            if (gd->dr.isInDarkRoom())
            {
                showFullScreen(); // 强制恢复全屏
                return ;
            }
            actionFullScreen(); // 接着关闭全屏
        }
    }

    return QMainWindow::changeEvent(event);
}

void BaseWindow::saveWinPos(QPoint point)
{
    us->setVal("layout/win_pos", point);
}

void BaseWindow::saveWinLayout(QPoint point, QSize size)
{
    us->setVal("layout/win_pos", point);
    us->setVal("layout/win_size", size);
    us->setVal("layout/win_max", false);

    // 判断名片窗口的移动
    if (us->card_follow_move && card_editors.size() > 0)
    {
        // 以第一个保存全局位置的名片框为准
        int i = 0;
        while (i < card_editors.size() && !card_editors[i]->movePosFinished())
            i++;
    }
}

/*void BaseWindow::slotSetTitleBarContent(QString title)
{
    m_titleBar->setTitleContent(title);
}*/

/**
 * 窗口位置变化信号
 */
void BaseWindow::slotWindowPosMoved(QPoint delta)
{
    // 判断名片窗口的移动
    if (us->card_follow_move)
    {
        foreach (CardEditor * editor, card_editors)
        {
            editor->movePos(delta);
        }
    }
}

/**
 * 窗口大小变化信号
 */
void BaseWindow::slotWindowSizeChanged(int dw, int dh)
{
    if (titlebar_widget != nullptr && titlebar_widget->isMaxxing())
        return ;
    if (this->isMaximized() || this->isMinimized())
        return ;
    int w = this->width();
    int h = this->height();
    if (w+dw > 100)
        w += dw;
    if (h+dh > 80)
        h += dh;
    this->setGeometry(this->geometry().left(), this->geometry().top(), w, h);
//    setFixedSize(w, h);
//    if (us->integration_window)
     /* drag_size_widget->move(this->geometry().width()-drag_size_widget->width(), this->geometry().height()-drag_size_widget->height()); */
    us->setVal("layout/win_size", this->size());
}

void BaseWindow::updateUI()
{
    log("basewindow.updateUI()");
    rerenderTitlebarPixmapLater();

    thm->setWidgetStyleSheet(this, "menu");

    tip_box->setBgColor(us->getNoOpacityColor(us->mainwin_sidebar_color));
    tip_box->setBtnColor(us->accent_color);
    tip_box->setFontColor(us->global_font_color);

    thm->setWidgetStyleSheet(qApp, "global");
}

void BaseWindow::slotSideBarWidthChanged()
{
    if (titlebar_widget != nullptr)
    {
        adjustTitlebar();
    }
}

void BaseWindow::initBgPictures()
{
    bg_jpg_enabled = isFileExist(rt->IMAGE_PATH+"win_bg.jpg");
    bg_png_enabled = isFileExist(rt->IMAGE_PATH+"win_bg.png");
    if (bg_jpg_enabled)
    {
        bg_jpg = QPixmap(rt->IMAGE_PATH+"win_bg.jpg");

        QT_BEGIN_NAMESPACE
          extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
        QT_END_NAMESPACE

        // 设置模糊
        QTimer::singleShot(0, [=]{
            int radius = us->blur_picture_radius;
            if (radius)
            {
                // 开始模糊
                QPixmap pixmap = bg_jpg;
                QImage img = pixmap.toImage(); // img -blur-> painter(pixmap)
                QPainter painter( &pixmap );
                qt_blurImage( &painter, img, radius, true, false );
                // 裁剪掉边缘（模糊后会有黑边）
//                bg_jpg.copy(radius, radius, bg_jpg.width()-radius*2, bg_jpg.height()-radius*2);
                int c = qMin(bg_jpg.width(), bg_jpg.height());
                c = qMin(c/2, radius);
                QPixmap clip = pixmap.copy(c/2, c/2, pixmap.width()-c, pixmap.height()-c);
                bg_jpg = clip;
                update();
            }
        });
    }

    if (bg_png_enabled)
        bg_png = QPixmap(rt->IMAGE_PATH+"win_bg.png");
    update();
}

void BaseWindow::slotSetWindowTitleBarContent(QString title)
{
    if (us->integration_window)
        titlebar_widget->setTitleContent(title);
    setWindowTitle(title);
}

void BaseWindow::slotSetTitleNovel(QString t)
{
    title_novel = t;
    if (us->integration_window)
    {
        if (title_novel == rt->current_novel)
        {
            titlebar_widget->setTitleContent(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
        }
        else
        {
            titlebar_widget->setTitleContent(QString("%1 / %2  [ %3字 ]").arg(title_novel).arg(title_full).arg(title_wc));
        }
        setWindowTitle(QString("%1").arg(title_full));
    }
    else
    {
        setWindowTitle(QString(" %1  [ %2字 ]").arg(title_full).arg(title_wc));
    }
}

void BaseWindow::slotSetTitleChapter(QString t)
{
    title_chapter = t;
    if (us->integration_window)
    {
        if (title_novel == rt->current_novel)
        {
            titlebar_widget->setTitleContent(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
        }
        else
        {
            titlebar_widget->setTitleContent(QString("%1 / %2  [ %3字 ]").arg(title_novel).arg(title_full).arg(title_wc));
        }
        setWindowTitle(QString("%1").arg(title_full));
    }
    else
    {
        setWindowTitle(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    }
}

void BaseWindow::slotSetTitleFull(QString t)
{
    title_full = t;
    if (us->integration_window)
    {
        if (title_novel == rt->current_novel)
        {
            titlebar_widget->setTitleContent(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
        }
        else
        {
            titlebar_widget->setTitleContent(QString("%1 / %2  [ %3字 ]").arg(title_novel).arg(title_full).arg(title_wc));
        }
        setWindowTitle(QString("%1").arg(title_full));
    }
    else
    {
        setWindowTitle(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    }
}

void BaseWindow::slotChapterWordCount(int x)
{
    title_wc = x;
    if (us->integration_window)
    {
        if (title_novel == rt->current_novel)
        {
            titlebar_widget->setTitleContent2(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
        }
        else
        {
            titlebar_widget->setTitleContent2(QString("%1 / %2  [ %3字 ]").arg(title_novel).arg(title_full).arg(title_wc));
        }
        setWindowTitle(QString("%1").arg(title_full));
    }
    else
    {
        setWindowTitle(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    }
}

void BaseWindow::slotTransContainerToClose(int kind)
{
    Q_UNUSED(kind)
    if (user_settings_window != nullptr && !user_settings_window->isHidden())
    {
        QRect sr = user_settings_window->geometry();
        QRect er(callback_point, QSize(1,1));
        QPixmap* pixmap = new QPixmap(sr.size());
        // pixmap->fill(Qt::transparent);
        user_settings_window->render(pixmap);
        ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, QEasingCurve::Linear, 200, 1);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
            delete pixmap;
        });
        user_settings_window->hide();
    }
    if (user_settings_window2 != nullptr && !user_settings_window2->isHidden())
    {
        QRect sr = user_settings_window2->geometry();
        QRect er(callback_point, QSize(1, 1));
        QPixmap *pixmap = new QPixmap(sr.size());
        // pixmap->fill(Qt::transparent);
        user_settings_window2->render(pixmap);
        ZoomGeometryAnimationWidget *ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, QEasingCurve::Linear, 200, 1);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=] {
            delete pixmap;
        });
        user_settings_window2->hide();
    }
    if (novel_schedule_widget != nullptr && !novel_schedule_widget->isHidden())
    {
        QRect sr = novel_schedule_widget->geometry();
        QRect er(callback_point, QSize(1,1));
        QPixmap *pixmap = new QPixmap(sr.size());
        pixmap->fill(Qt::transparent);
        // 因为未设置透明，是用painter画的透明背景，所以不要绘制QWidget::DrawWindowBackground
        novel_schedule_widget->render(pixmap, QPoint(0,0), QRect(0,0,sr.width(),sr.height()), QWidget::DrawChildren);
        ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, 1);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
            delete pixmap;
        });
        novel_schedule_widget->hide();
    }
}

void BaseWindow::slotTransContainerClosed(int kind)
{
    Q_UNUSED(kind)
    editors_stack->slotDelayFocus();
}

void BaseWindow::slotShowNovelInfo(QString novel_name, QPoint point)
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(this))
            dir_layout->slotNeedLineGuide();
        return ;
    }
    if (novel_schedule_widget == nullptr)
    {
        initNovelScheduleWindow();
    }
    callback_point = point;

    // 动画
    novel_schedule_widget->toShow(novel_name, point); // 先显示，再隐藏
    QPixmap* pixmap = new QPixmap(novel_schedule_widget->geometry().size());
    pixmap->fill(Qt::transparent);
    novel_schedule_widget->render(pixmap);
    QRect sr(point, QSize(1,1));
    QRect er(novel_schedule_widget->geometry());
    novel_schedule_widget->hide(); // 隐藏掉
    editors_stack->stackUnder(trans_ctn);
    trans_ctn->toShow(); // 先显示蒙版，避免把小窗口图标也显示出来
    ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, QEasingCurve::OutBack, us->mainwin_bg_color.alpha(), -1);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
        novel_schedule_widget->show();
        novel_schedule_widget->setFocus();
        delete pixmap;
    });

}

void BaseWindow::slotShowImExWindow(int kind, QString def_name)
{
    if (im_ex_window == nullptr)
    {
        // 初始化导入导出窗口
        im_ex_window = new ImExWindow(this);
        // if (!us->settings_in_mainwin)
        {
            im_ex_window->setWindowFlags(Qt::Dialog);
            im_ex_window->setWindowModality(Qt::WindowModal);
        }

        // 初始化信号连接
        // 导出
        connect(im_ex_window->export_page, SIGNAL(signalExport()), dir_layout->getListView(), SLOT(slotExport()));
        connect(dir_layout->getListView(), SIGNAL(signalExportFinished()), im_ex_window->export_page, SLOT(slotExportFinished()));
        // 导入
        connect(im_ex_window->import_page, SIGNAL(signalImport(QString,bool,int)), dir_layout->getListView(), SLOT(slotImport(QString,bool,int)));
        connect(dir_layout->getListView(), SIGNAL(signalImportFinished(QString)), im_ex_window->import_page, SLOT(slotImportFinished(QString)));
        connect(im_ex_window->import_page, SIGNAL(signalImportFinished(QString)), dir_layout, SLOT(slotImportFinished(QString)));

        connect(im_ex_window->import_page, &ImportPage::signalImportFinished, [=]{
            if (novel_schedule_widget != nullptr && !novel_schedule_widget->isHidden())
                novel_schedule_widget->toShow(im_ex_window->import_page->getTargetName(), QPoint(0, 0));
        });
        connect(im_ex_window->import_mzfy_page, SIGNAL(signalImportMzfyFinished(QString)), dir_layout, SLOT(slotImportFinished(QString)));
    }
    im_ex_window->import_page->setNovelDirListView(dir_layout->getListView());
    //im_ex_window->setNovelName(def_name);
    im_ex_window->toShow(kind, def_name);
}

void BaseWindow::slotInsertText(QString text)
{
    QTextEdit* edit = editors_stack->getCurrentEditor();
    if (edit != nullptr)
    {
        edit->insertPlainText(text);
        edit->setFocus();
    }
}

/**
 * 插入一个带前缀的文本，不一定插入前缀
 * @param suffix 如果光标左边或者选中的是这个前缀，则不插入前缀
 * @param text   必定插入的文本
 */
void BaseWindow::slotInsertTextWithSuffix(QString suffix, QString str)
{
    QTextEdit* edit = editors_stack->getCurrentEditor();
    if (edit != nullptr)
    {
        QString text = edit->toPlainText();
        QTextCursor tc = edit->textCursor();
        int position = tc.position();
        if (tc.hasSelection() && tc.selectedText() == suffix) // 如果选中了这个前缀（其实没必要，一起插入前缀就好了）（可能是为了撤销重做的规范性？）
        {
            tc.setPosition(tc.selectionEnd()); // 取消选中，文本直接放后面来
            edit->setTextCursor(tc);
            edit->insertPlainText(str);
        }
        else if (position >= suffix.length() && text.mid(position-suffix.length(), suffix.length()) == suffix) // 光标左边刚好是前缀
        {
            edit->insertPlainText(str);
        }
        else // 插入前缀+文本
        {
            edit->insertPlainText(suffix+str);
        }

        edit->setFocus();
    }
}

void BaseWindow::slotLocateEdit(int index, int pos, int len)
{
    if (index < -1 || index >= editors_stack->getTextEditors().count())
        return ;
    if (index > -1 && editors_stack->currentIndex() != index) // -1表示不进行切换，为当前章节
    {
        //editors_stack->switchEditor(index); // 打开章节的时候会进行切换的…… // 切换两次也无妨啊
        ChapterEditor* ce = editors_stack->getEditors().at(index);
        EditingInfo info = ce->getInfo();
        if (dir_layout->getListView()->getNovelName() == info.getNovelName()) // 是同一本书，如果不是就不进行目录操作
            dir_layout->getListView()->slotOpenChapter(info.getChapterName());
    }
    ChapterEditor* edit = editors_stack->getCurrentEditor();
    if (edit == nullptr)
        return ;
    /*QTextCursor tc = edit->textCursor();
    tc.setPosition(pos);
    tc.setPosition(pos+len, QTextCursor::KeepAnchor);
    edit->setTextCursor(tc);*/
    edit->smoothScrollTo(pos, len);
}

void BaseWindow::slotEmitCommand(QString command)
{
    command = command.trimmed();
    // 前缀
    ShortcutEnvironment env = SCK_GLOBAL;
    if (command.startsWith("全局·"))
    {
        env = SCK_GLOBAL;
        command = command.right(command.length()-3);
    }
    else if (command.startsWith("编辑·"))
    {
        env = SCK_EDITOR;
        command = command.right(command.length()-3);
    }
    else if (command.startsWith("目录·"))
    {
        env = SCK_DIR;
        command = command.right(command.length()-3);
    }
    else if (command.startsWith("大纲·"))
    {
        env = SCK_OUTLINE;
        command = command.right(command.length()-3);
    }

    // 字符串转换成命令和参数
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

    // 目录命令
    if ((env == SCK_DIR || env == SCK_GLOBAL) && dir_layout != nullptr && dir_layout->execCommand(command, args))
    {
        if (us->integration_window)
        {
            /*if (sidebar_widget->isHidden()) // 如果隐藏的话，恢复显示
                titlebar_widget->onButtonSidebarClicked();*/
            setSidebarShowed(true);
            sidebar_widget->setPage(0);
        }
        else
        {
            // TODO : if (hidden) noveldir_dock->show();
        }
        return ;
    }
    // 故事线命令
    if ((env == SCK_OUTLINE || env == SCK_GLOBAL) && outline_group != nullptr && outline_group->execCommand(command, args))
    {
        if (us->integration_window)
        {
            /*if (sidebar_widget->isHidden()) // 如果隐藏的话，恢复显示
                titlebar_widget->onButtonSidebarClicked();*/
            setSidebarShowed(true);
            sidebar_widget->setPage(1);
        }
        else
        {
            // TODO : if (hidden) outline_dock->show();
        }
        return ;
    }
    // 所有编辑器指令
    if ((env == SCK_EDITORS || env == SCK_GLOBAL) && editors_stack != nullptr && editors_stack->execCommand(command, args))
        return ;
    // 单个编辑器指令
    if ((env == SCK_EDITOR || env == SCK_GLOBAL) && editors_stack->getCurrentEditor() != nullptr && editors_stack->getCurrentEditor()->execCommand(command, args))
        return ;
    // 全局指令
    execCommand(command, args);
}

void BaseWindow::slotPopNotification(NotificationEntry* noti)
{
    tip_box->createTipCard(noti);
}

void BaseWindow::slotNotificationCardClicked(NotificationEntry *noti)
{
    qDebug() << noti->toString();
}

void BaseWindow::slotNotificationButtonClicked(NotificationEntry *noti)
{
    qDebug() << noti->toString();
}

void BaseWindow::actionCloudState()
{
    if (!ac->isLogin())
    {
        static LoginWindow* lw = new LoginWindow(this);
        lw->exec();
    }
    else
    {
        static SyncStateWindow* ssw = new SyncStateWindow(this);
        ssw->show();

        ac->syncNext(); // 点开这项开始同步
    }
}

void BaseWindow::actionThemeManage()
{
    static ThemeManageWindow* tmw = new ThemeManageWindow(this);
    tmw->show();
}

void BaseWindow::actionNightTheme()
{
    if (thm->getThemeName().contains(NIGHT_THEME)) // 切换到原来的
    {
        thm->loadTheme(USER_THEME);
    }
    else // 切换到夜间
    {
        thm->loadTheme(NIGHT_THEME);
    }

    if (us->dynamic_bg != nullptr)
    {
        initDynamicBackground();
    }
}

void BaseWindow::actionDarkRoom()
{
    static DarkRoomWidget* drw = new DarkRoomWidget(this);

    DarkRoom& dr = gd->dr;
    if (!dr.isInDarkRoom()) // 显示小黑屋
    {
        drw->exec();
    }
    else // 正在小黑屋里面
    {
        dr.exitDarkRoom(this); // 尝试退出
    }
}

void BaseWindow::actionCloseOpenedChapter()
{
    editors_stack->slotCloseAllEditors();

    if (titlebar_widget != nullptr)
    {
        titlebar_widget->setTitleContent("写作天下");
    }
}

void BaseWindow::actionFullScreen()
{
    bool& b = rt->full_screen;
    if (!b) // 进入全屏
    {
        showFullScreen();
        /* if (us->integration_window)
            drag_size_widget->hide(); */
        emit rt->signalMainWindowMaxing(true);
    }
    else // 退出全屏
    {
        if (gd->dr.isInDarkRoom()) // 正在小黑屋中，不允许退出全屏
            return ;
        if (rt->maximum_window)
            showMaximized();
        else
        {
            showNormal();
            emit rt->signalMainWindowMaxing(false);
        }
        /* if (!this->isMaximized() && us->integration_window)
            drag_size_widget->show(); */
    }
    b = !b;
}

void BaseWindow::actionHelp()
{
    if (rt->has_new_version == 2)
        this->close(); // 重启更新
    else
        if (DEBUG_MODE)
            QDesktopServices::openUrl(QUrl(QLatin1String("http://writerfly.cn/downloadbeta/")));
        else
            QDesktopServices::openUrl(QUrl(QLatin1String("http://writerfly.cn/download/")));
}

/*
 * 获取可能的用户需要选中的词语
 */
QString BaseWindow::getProperSelectedText()
{
    QString predict_name;
    ChapterEditor* editor = editors_stack->getCurrentEditor();
    if (editor == nullptr)
        return "";
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection()) // 判断选中文本
    {
        QString text = cursor.selectedText();
        if (text.length() > 10 || text.contains("\n") || text.contains(" ")
            || text.contains("\t") || text.contains("　"))
            return "";
        else // 直接返回选中文本
            return text;
    }

    // 分析上下文，获取可能的文本
    QString text = editor->toPlainText();
    int pos = cursor.position();
    QString left = text.left(pos);
    int line_pos = left.lastIndexOf("\n");
    if (line_pos>-1) // 去掉换行
        left = left.right(left.length() - line_pos - 1);

    // 选中左边的单词
    int word_left = pos = left.length();
    while (word_left > 0)
    {
        QString c = left.mid(word_left - 1, 1);
        if ((NovelTools::isChinese(c) || NovelTools::isEnglish(c) || NovelTools::isNumber(c))
            && !gd->lexicons.isSplitWord(c))
            word_left--;
        else
            break;
    }
    if (word_left>=pos || word_left+10<pos)
        return "";
    else
        return left.mid(word_left, pos - word_left);

    /* return [](QString s, int pos){
        return QString("");
    }(editor->toPlainText(), cursor.position()); */
}

void BaseWindow::slotAddNewCard()
{
	if (gd->clm.currentLib() ==  nullptr)
    	return ;
    QString proper_name = getProperSelectedText();
    ChapterEditor* chapter_editor = editors_stack->getCurrentEditor();
    CardEditor* ce = new CardEditor(proper_name, this);
    // 如果刚好把选中文字添加到了名片，那么就取消文字的选中
    if (!proper_name.isEmpty() && chapter_editor != nullptr &&
        chapter_editor->textCursor().hasSelection() && chapter_editor->textCursor().selectedText() == proper_name)
    {
        connect(ce, &CardEditor::signalCardCreated, [=](CardBean* card){
            // 要先进行判断，因为是异步的，可能光标位置、选中位置、甚至编辑框的内容都已经变了
            if (card->name ==  proper_name && chapter_editor == editors_stack->getCurrentEditor()
                && chapter_editor->textCursor().hasSelection() && chapter_editor->textCursor().selectedText() == proper_name)
            {
                // 把光标聚焦到最右边，因为可能是添加在最左边的
                QTextCursor cursor = chapter_editor->textCursor();
                int start = cursor.selectionStart();
                int end = cursor.selectionEnd();
                int pos = qMax(start, end);
                cursor.setPosition(pos);
                chapter_editor->setTextCursor(cursor);
            }
        });
    }
    card_editors.append(ce);
    connect(ce, &CardEditor::signalClosed, this, [=] { card_editors.removeOne(ce); });
    connect(ce, &CardEditor::signalCtrlMoved, this, [=](QPoint delta) { slotMoveAllCardEditor(delta, ce); });
    connect(ce, SIGNAL(signalShowCardEditor(CardBean *, CardEditor *)), this, SLOT(slotShowCardEditor(CardBean *, CardEditor *)));
    ce->show();
    ce->show();
}

void BaseWindow::slotRefreshEditingCards()
{
    QTextEdit* edit = editors_stack->getCurrentEditor();
    if (edit == nullptr)
        return;
    cardlib_group->slotRefreshEditingCards(edit);
}

void BaseWindow::slotShowCardEditor(CardBean* card)
{
    foreach (CardEditor* editor, card_editors)
    {
        if (editor->getCard() == card)
        {
            editor->raise();
            editor->setFocus();
            return;
        }
    }

    CardEditor *ce = new CardEditor(card, this);
    card_editors.append(ce);
    connect(ce, &CardEditor::signalClosed, this, [=]{ card_editors.removeOne(ce); });
    connect(ce, &CardEditor::signalCtrlMoved, this, [=](QPoint delta){ slotMoveAllCardEditor(delta, ce); });

    // 绑定名片显示的卡片和编辑窗口
    QList<CardCardWidget*> ws = cardlib_group->getCardWidgets(card);
    foreach (CardCardWidget* w, ws)
    {
        connect(ce, &CardEditor::signalCardModified, [=](CardBean *card) {
            w->refreshData();
        });
    }
    connect(ce, SIGNAL(signalShowCardEditor(CardBean *, CardEditor*)), this, SLOT(slotShowCardEditor(CardBean *, CardEditor*)));
    ce->show();
    ce->setFocus();
}

void BaseWindow::slotShowCardEditor(CardBean *card, CardEditor* from)
{
    foreach (CardEditor *editor, card_editors)
    {
        if (editor->getCard() == card)
        {
            editor->raise();
            editor->setFocus();
            return;
        }
    }

    CardEditor *ce = new CardEditor(card, from, this);
    card_editors.append(ce);
    connect(ce, &CardEditor::signalClosed, this, [=] { card_editors.removeOne(ce); });
    connect(ce, &CardEditor::signalCtrlMoved, this, [=](QPoint delta) { slotMoveAllCardEditor(delta, ce); });

    // 绑定名片显示的卡片和编辑窗口
    QList<CardCardWidget *> ws = cardlib_group->getCardWidgets(card);
    foreach (CardCardWidget *w, ws)
    {
        connect(ce, &CardEditor::signalCardModified, [=](CardBean *card) {
            w->refreshData();
        });
    }

    connect(ce, SIGNAL(signalShowCardEditor(CardBean *, CardEditor*)), this, SLOT(slotShowCardEditor(CardBean *, CardEditor*)));
    connect(ce, SIGNAL(signalLocateFromEditor(CardEditor*)),  from,  SLOT(slotLocateFromEditor(CardEditor*)));
    from->addToEditor(ce);
    connect(ce, &CardEditor::signalClosed, this, [=]{
        if (ce->getFrom() == from)
        {
            from->delToEditor(ce);
        }
    });
    ce->show();
    ce->setFocus();
}

void BaseWindow::slotCardlibClosed()
{
    // 关闭所有打开的编辑窗口
    foreach (CardEditor *editor, card_editors)
    {
        editor->close();
    }
}

void BaseWindow::slotMoveAllCardEditor(QPoint delta, CardEditor *ce)
{
    foreach (CardEditor *editor, card_editors)
    {
        if (editor != ce) // 不是触发事件的那个
        {
            editor->movePos(delta);
        }
    }
}
