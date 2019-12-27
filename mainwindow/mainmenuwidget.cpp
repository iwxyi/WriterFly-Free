#include "mainmenuwidget.h"

MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent), is_hidding(false)
{
    is_showing = is_hidding = false;

    initView();

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
    // connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(slotFocusChanged(QWidget*, QWidget*)));
}

void MainMenuWidget::initView()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    QVBoxLayout* main_vlayout   = new QVBoxLayout;
    main_vlayout->setMargin(15);
    main_vlayout->setSpacing(0);

    // ==== 初始化分割线 ====
    hsplit1 = new QLabel(this);
    hsplit2 = new QLabel(this);
    vsplit1 = new QLabel(this);
    vsplit2 = new QLabel(this);

    // ==== 初始化状态 ====
    state_panel = new InteractiveButtonBase(this);
    profile_label = new WaterCircleButton(QIcon(ac->getProfile()), state_panel);
    nickname_label = new QLabel(APPLICATION_NAME, state_panel);
    sync_state_label = new QLabel("云同步状态：等待开发", state_panel);
    level_label = new QLabel("Lv.1", state_panel);
    QWidget* level_tmep_widget = new QWidget(state_panel);
    right_arrow = new WaterCircleButton(QPixmap(":/icons/right_arrow"), state_panel);
    right_arrow->setDisabled(true);

    QHBoxLayout* state_hlayout = new QHBoxLayout;
    {
        state_hlayout->setMargin(0);
        state_hlayout->setSpacing(0);
        state_hlayout->addWidget(profile_label);
        QVBoxLayout* name_vlayout   = new QVBoxLayout;
        {
            name_vlayout->setSpacing(0);
            name_vlayout->setMargin(DEFAULT_SPACING);
            QHBoxLayout* name_hlayout = new QHBoxLayout();
            {
                name_hlayout->setSpacing(10);
                name_hlayout->addWidget(nickname_label);
                name_hlayout->addWidget(level_label);
                name_hlayout->addWidget(level_tmep_widget);
                level_tmep_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                level_tmep_widget->setFixedHeight(1);
            }
            name_vlayout->addLayout(name_hlayout);
            name_vlayout->addWidget(sync_state_label);
        }
        state_hlayout->addLayout(name_vlayout);
        state_hlayout->addWidget(right_arrow);
    }
    state_panel->setLayout(state_hlayout);

    // ==== 初始化数据 ====
    word_button = new InteractiveButtonBase(this);
    word_count_label = new QLabel("0", word_button);
    word_label = new QLabel("字数", word_button);
    time_button = new InteractiveButtonBase(this);
    time_count_label = new QLabel("0", time_button);
    time_label = new QLabel("时间", time_button);
    rank_button = new InteractiveButtonBase(this);
    rank_count_label = new QLabel("1", rank_button);
    rank_label = new QLabel("排名", rank_button);
    word_button->setToolTip("码字总字数（本地）\n本地设备与云端分离，不同步");
    time_button->setToolTip("码字总时长（分钟）");
    rank_button->setToolTip("码字榜排名，每日凌晨更新\n等级以云端为主");

    QHBoxLayout* data_hlayout = new QHBoxLayout;
    {
        data_hlayout->setSpacing(0);
        QVBoxLayout* word_vlayout = new QVBoxLayout;
        {
            word_vlayout->setSpacing(DEFAULT_SPACING);
            word_vlayout->setMargin(0);
            word_vlayout->addWidget(word_count_label);
            word_vlayout->addWidget(word_label);
        }
        word_button->setLayout(word_vlayout);
        data_hlayout   -> addWidget(word_button);

        data_hlayout->addWidget(vsplit1);

        QVBoxLayout* time_vlayout = new QVBoxLayout;
        {
            time_vlayout->setSpacing(DEFAULT_SPACING);
            time_vlayout->setMargin(0);
            time_vlayout->addWidget(time_count_label);
            time_vlayout->addWidget(time_label);
        }
        time_button->setLayout(time_vlayout);
        data_hlayout   -> addWidget(time_button);

        data_hlayout->addWidget(vsplit2);

        QVBoxLayout* rank_vlayout = new QVBoxLayout;
        {
            rank_vlayout->setSpacing(DEFAULT_SPACING);
            rank_vlayout->setMargin(0);
            rank_vlayout->addWidget(rank_count_label);
            rank_vlayout->addWidget(rank_label);
        }
        rank_button->setLayout(rank_vlayout);
        data_hlayout   -> addWidget(rank_button);
    }

    // ==== 初始化当前 ====
    speed_today_widget = new QWidget(this);
    speed_today_panel = new InteractiveButtonBase(speed_today_widget);
    speed_label = new QLabel("0 分钟写了 0 字 ( 0 / h )", speed_today_panel);
    today_label = new QLabel("今日共 0 字，快开始吧！", speed_today_panel);

    QVBoxLayout* speed_todat_vlayout = new QVBoxLayout;
    {
        speed_todat_vlayout->addWidget(speed_label);
        speed_todat_vlayout->addWidget(today_label);
    }
    speed_today_panel->setLayout(speed_todat_vlayout);

    QHBoxLayout* speed_today_hlayout = new QHBoxLayout;
    speed_today_hlayout->setMargin(0);
    speed_today_hlayout->setSpacing(0);
    speed_today_hlayout->addWidget(speed_today_panel);
    speed_today_widget->setLayout(speed_today_hlayout);


    // ==== 初始化菜单 ====
    theme_button = new InteractiveButtonBase("主题：默认主题", this);
    theme_label = new WaterCircleButton(QPixmap(":/icons/moon"), this);
    dark_room_button = new InteractiveButtonBase("小黑屋：未开启", this);
    viewport_button = new InteractiveButtonBase("视图", this);
    viewport_button->setPaintAddin(QPixmap(":/icons/right_arrow2"), Qt::AlignRight);
    close_opened_button = new InteractiveButtonBase("关闭打开的章节(0)", this);
    theme_label->setLeaveAfterClick(true);

    QVBoxLayout* menu_vlayout = new QVBoxLayout;
    {
        menu_vlayout->setSpacing(0);
        menu_vlayout->setMargin(0);
        QHBoxLayout* theme_hlayout = new QHBoxLayout;
        {
            theme_hlayout->addWidget(theme_button);
            theme_hlayout->addWidget(theme_label);
        }
        menu_vlayout->addLayout(theme_hlayout);

        menu_vlayout->addWidget(dark_room_button);

        menu_vlayout->addWidget(viewport_button);

        menu_vlayout->addWidget(close_opened_button);
    }


    // ====初始化按钮 ====
    full_screen_button = new InteractiveButtonBase("全屏", this);
    settings_button = new InteractiveButtonBase("设置", this);
    help_button = new InteractiveButtonBase(DEBUG_MODE?"内测":"官网", this);

    QHBoxLayout* button_hlayout = new QHBoxLayout;
    {
        button_hlayout->addWidget(full_screen_button);
        button_hlayout->addWidget(settings_button);
        button_hlayout->addWidget(help_button);
    }

    // ==== 控件布局 ====
    main_vlayout->addWidget(state_panel);
    main_vlayout->addWidget(hsplit1);
    main_vlayout->addLayout(data_hlayout);
    {
        QWidget* temp_widget = new QWidget(this);
        temp_widget->setFixedHeight(DEFAULT_SPACING*2);
        main_vlayout->addWidget(temp_widget);
    }
    main_vlayout->addWidget(speed_today_widget);
    {
        QWidget* temp_widget = new QWidget(this);
        temp_widget->setFixedHeight(DEFAULT_SPACING*2);
        main_vlayout->addWidget(temp_widget);
    }
    main_vlayout->addLayout(menu_vlayout);
    main_vlayout->addWidget(hsplit2);
    main_vlayout->addLayout(button_hlayout);
    setLayout(main_vlayout);

    // Mac按钮控制
    state_panel->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    profile_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    nickname_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    sync_state_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    level_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    right_arrow->setAttribute(Qt::WA_LayoutUsesWidgetRect);;

    word_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    word_count_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    word_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    time_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    time_count_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    time_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    rank_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    rank_count_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    rank_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;

    speed_today_widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    speed_today_panel->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    speed_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    today_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;

    theme_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    dark_room_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    viewport_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    close_opened_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    theme_label->setAttribute(Qt::WA_LayoutUsesWidgetRect);;

    full_screen_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    settings_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;
    help_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);;

    QLabel* hsplit1, *hsplit2, *vsplit1, *vsplit2;

   /*  // ==== 初始化透明度 ====
    effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1); // 默认是全透明的
    this->setGraphicsEffect(effect); */

    initStyle();
    initEvent();
}

void MainMenuWidget::initStyle()
{
    int size = QApplication::fontMetrics().height() + QApplication::fontMetrics().lineSpacing();

//    profile_label->setFixedSize(nickname_label->height()+sync_state_label->height(), nickname_label->height()+sync_state_label->height());
    profile_label->setFixedSize(size*1.5, size*1.5);
    state_panel->setMinimumHeight(size*1.5);
    theme_label->setFixedSize(size, size);
    right_arrow->setFixedSize(size, size);
    theme_button->setFixedHeight(size);
    dark_room_button->setFixedHeight(size);
    viewport_button->setFixedHeight(size);
    close_opened_button->setFixedHeight(size);
    full_screen_button->setMinimumHeight(size);
    settings_button->setMinimumHeight(size);
    help_button->setMinimumHeight(size);

    QFont small_font;
    QFont large_font;
    small_font.setPointSize(small_font.pointSize() * 8 / 10);
    large_font.setPointSize(large_font.pointSize() * 1.5);
    sync_state_label->setFont(small_font);
    sync_state_label->setMinimumWidth(sync_state_label->fontMetrics().width("零一二三四五六七八九十"));
    level_label->setFont(small_font);
    nickname_label->setFont(large_font);
    nickname_label->setMinimumWidth(nickname_label->fontMetrics().width("一二三四五六七"));

    level_label->setAlignment(Qt::AlignCenter);
    level_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    theme_button->setAlign(Qt::AlignLeft);
    dark_room_button->setAlign(Qt::AlignLeft);
    viewport_button->setAlign(Qt::AlignLeft);
    close_opened_button->setAlign(Qt::AlignLeft);

    word_button->setMinimumHeight(size*1.5);
    time_button->setMinimumHeight(size*1.5);
    rank_button->setMinimumHeight(size*1.5);
    word_count_label->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    word_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    time_count_label->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    time_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    rank_count_label->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    rank_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QString hsplit_style = "background:rgba(128,128,128,50); margin:2px; margin-left:10px; margin-right:10px;";
    hsplit1->setStyleSheet(hsplit_style);
    hsplit1->setFixedHeight(5);
    hsplit2->setStyleSheet(hsplit_style);
    hsplit2->setFixedHeight(5);

    QString vsplit_style = "background:rgba(128,128,128,50); margin-top:10px; margin-bottom:10px; ";
    vsplit1->setStyleSheet(vsplit_style);
    vsplit1->setFixedWidth(1);
    vsplit2->setStyleSheet(vsplit_style);
    vsplit2->setFixedWidth(1);

    size = theme_button->height();
    speed_today_widget->setMinimumHeight(size*2);
    speed_today_panel->setMinimumHeight(size*2);
    speed_today_panel->setObjectName("speed_today_panel");
    speed_label->setStyleSheet("margin:0px; margin-left:10px;");
    today_label->setStyleSheet("margin:0px; margin-left:10px;");
    int shadow = 2;
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(speed_today_widget);
    shadow_effect->setOffset(0, shadow);
    shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
    shadow_effect->setBlurRadius(10);
    speed_today_widget->setGraphicsEffect(shadow_effect);

    int all_height = profile_label->height() + word_count_label->height() + word_label->height()
                    + speed_today_widget->height() + theme_button->height() + dark_room_button->height()
                    + viewport_button->height() + full_screen_button->height()
                    + DEFAULT_SPACING * 14;
    int all_width = profile_label->minimumWidth() + state_panel->minimumWidth() + right_arrow->minimumWidth() + DEFAULT_SPACING*2;
    if (all_width < all_height*0.618) all_width = all_height*0.618;
    int title_size = profile_label->minimumWidth() + nickname_label->fontMetrics().width("一二三四五六七") + level_label->fontMetrics().width("lv.2000") + right_arrow->minimumWidth() + DEFAULT_SPACING*2;
    if (all_width < title_size) all_width = title_size;
    setMinimumSize(all_width, all_height); // 自适应比例：黄金比例
}

void MainMenuWidget::initEvent()
{
    connect(state_panel, &InteractiveButtonBase::clicked, [=]{
        emit signalCloudClicked();
    });

    connect(theme_button, &InteractiveButtonBase::clicked, [=]{
        emit signalThemeClicked();
        emitHide();
    });

    connect(theme_label, &InteractiveButtonBase::clicked, [=]{
        emit signalNightClicked();
        emitHide();
    });

    connect(close_opened_button, &InteractiveButtonBase::clicked, [=]{
        emitHide();
        emit signalCloseOpenedClicked();
    });


    connect(settings_button, &InteractiveButtonBase::clicked, [=]{
        emitHide(); // 后来修改了，所以需要隐藏下面的hide，否则两项相同，负负得正了
        // 后注：其实重复关系不大，并不会负负得正
        // hide(); // 先行一步隐藏，虽然多余，但是避免进入设置的模糊化时还显示菜单
        QTimer::singleShot( 0, [=]{
            emit signalSettingsClicked();
        });
    });

    connect(full_screen_button, &InteractiveButtonBase::clicked, [=]{
        emitHide();
        emit signalFullScreenClicked();
    });

    connect(help_button, &InteractiveButtonBase::clicked, [=]{
        emitHide();
        emit signalHelpClicked();
    });
    connect(dark_room_button, &InteractiveButtonBase::clicked, [=]{
        emitHide();
        emit signalDarkRoomClicked();
    });

    connect(ac, &SyncThread::signalSyncChanged, [=](int download, int upload){
       if (!isHidden())
       {
           QString state;
           if (download && upload)
               state = QString("云同步：下载 %1, 上传 %2").arg(download).arg(upload);
           else if (download)
               state = QString("云同步：下载 %1 项").arg(download);
           else if (upload)
               state = QString("云同步：上传 %1 项").arg(upload);
           else
               state = "云同步完毕";
           sync_state_label->setText(state);
       }
    });
}

void MainMenuWidget::refreshData()
{
    ac->save();

    if (ac->isLogin())
    {
        nickname_label->setText(ac->getNickname());
        if (ac->sync_state == SyncThread::SYNC_SYNCING)
        {
            sync_state_label->setText("云同步：正在比对中");
        }
        else
        {
            int count = ac->getSyncLast();
            sync_state_label->setText("云同步：" + (count ? QString("剩下 %1 项").arg(count) : "已同步"));
        }
    }
    else
    {
        if (ac->account_state == SyncThread::AccountState::ACCOUNT_WAITING)
        {
            nickname_label->setText("正在登录...");
            sync_state_label->setText("即将进行云同步");
        }
        else if (ac->account_state == SyncThread::AccountState::ACCOUNT_OFFLINE)
        {
            nickname_label->setText(ac->getNickname());
            sync_state_label->setText("离线使用中");
        }
        else // NONE
        {
            nickname_label->setText(APPLICATION_NAME);
            sync_state_label->setText("云同步状态：未登录");
        }
    }

    level_label->setText("Lv."+QString::number(ac->getUserLevel()));
    word_count_label->setText(QString::number(ac->getUserWords()));
    time_count_label->setText(QString::number(ac->getUserTimes()));
    rank_count_label->setText(QString::number(ac->getRank()));

    speed_label->setText(QString::number(ac->getThisRunUserTimes()) + " 分钟，" + QString::number(ac->getThisRunUserWords()) + " 字 ("+QString::number(ac->getCurrentSpeed())+"/h)");
    today_label->setText("今日共 "+ QString::number(ac->getTodayWords()) +" 字，加油！");

    QString theme_name = thm->getThemeName();
    if (theme_name.isEmpty())
        theme_name = "默认主题";
    theme_button->setText("主题：" + theme_name);

    if (gd->dr.isInDarkRoom())
    {
        dark_room_button->setText("小黑屋：" + gd->dr.getRestString());
    }
    else
    {
        dark_room_button->setText("小黑屋：未开启");
    }

    if (rt->has_new_version)
    {
        if (rt->has_new_version == 1 && isFileExist(rt->DOWNLOAD_PATH + "update.zip"))
        {
            rt->has_new_version = 2;
        }
        if (rt->has_new_version == 1)
            help_button->setText("新版下载中");
        else if (rt->has_new_version == 2)
            help_button->setText("立即更新");
        else if (rt->has_new_version == 3)
            help_button->setText("等待更新");
        else
            help_button->setText("无更新");
        help_button->setTextColor(us->accent_color);
    }
}

void MainMenuWidget::updateUI()
{
    right_arrow->setIconColor(us->accent_color);
    theme_label->setIconColor(us->accent_color);
    viewport_button->setIconColor(us->accent_color);

    int radius = (level_label->fontMetrics().height() + level_label->fontMetrics().lineSpacing()/2) / 2;
    level_label->setFixedHeight(radius*2);
    QString level_style = "color:"+us->getColorString(us->mainwin_bg_color)+"; background:"+us->getColorString(us->accent_color)+"; border-radius:"+QString::number(radius)+"px; padding-left:"+QString::number(radius)+"px; padding-right:"+QString::number(radius)+"px;";
    level_label->setStyleSheet(level_style);

    QString global_font_style = "color:"+us->getColorString(us->global_font_color)+";";
    QString gray_style = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";
    QString gray_style2 = "color:"+us->getOpacityColorString(us->global_font_color, 192)+";";
    sync_state_label->setStyleSheet(gray_style);
    word_count_label->setStyleSheet(gray_style2);
    word_label->setStyleSheet(gray_style2);
    time_count_label->setStyleSheet(gray_style2);
    time_label->setStyleSheet(gray_style2);
    rank_count_label->setStyleSheet(gray_style2);
    rank_label->setStyleSheet(gray_style2);

    speed_today_widget->setStyleSheet("QWidget { background:"+us->getColorString(us->getNoOpacityColor(us->mainwin_bg_color))+";}"); // 阴影效果避免文字，必须设置背景
    speed_today_panel->setStyleSheet("QPushButton,QWidget { background:transparent;}");

    QPalette pa(QApplication::palette());
    nickname_label->setStyleSheet(global_font_style);
    speed_label->setStyleSheet(global_font_style);
    today_label->setStyleSheet(global_font_style);
    theme_button->setTextColor(us->global_font_color);
    dark_room_button->setTextColor(us->global_font_color);
    viewport_button->setTextColor(Qt::gray/*us->global_font_color*/);
    close_opened_button->setTextColor(us->global_font_color);
    full_screen_button->setTextColor(us->global_font_color);
    settings_button->setTextColor(us->global_font_color);
    help_button->setTextColor(us->global_font_color);
}

void MainMenuWidget::slotFocusChanged(QWidget *, QWidget *cur)
{
    if (isHidden() || is_hidding) return ;

    if (children().contains(cur)) // 是自己的子控件
        return ;
    if (cur == profile_label || cur == speed_today_panel)
        return ;

    emitHide();
}

void MainMenuWidget::toShow()
{
    /* if (is_showing) return ;
    is_showing = true;
    is_hidding = false; */

    refreshData();
    show();

    /* QPropertyAnimation* opa_ani = new QPropertyAnimation(effect, "opacity");
    opa_ani->setDuration(300);
    opa_ani->setStartValue(0);
    opa_ani->setEndValue(1);
    opa_ani->start();
    connect(opa_ani, &QPropertyAnimation::finished, [=]{
        opa_ani->deleteLater();
        if (is_showing)
        {
            is_showing = false;
            is_hidding = false;
        }
    }); */

    profile_label->delayShowed(10, QPoint(0,1));
}

void MainMenuWidget::toHide()
{
    /* if (is_hidding) return ;
    is_showing = false;
    is_hidding = true; */

    /* QPropertyAnimation* opa_ani = new QPropertyAnimation(effect, "opacity");
    opa_ani->setDuration(300);
    opa_ani->setStartValue(1);
    opa_ani->setEndValue(0);
    opa_ani->start();
    connect(opa_ani, &QPropertyAnimation::finished, [=]{
        opa_ani->deleteLater();
        if (is_hidding)
        {
            is_hidding = false;
            is_showing = false;
            hide();
        }
    }); */
    hide();
}

/**
 * 因为按钮分两级动画的，所以需要通过那个按钮来开关主菜单
 * 这是发送隐藏的信号，mainwindow控制信号到菜单按钮
 * 模拟点击菜单按钮来开关菜单
 */
void MainMenuWidget::emitHide()
{
	hide();
    // emit signalHide();
}

void MainMenuWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    QTimer::singleShot(0, [=]{
        setFocus();
    });
}

void MainMenuWidget::hideEvent(QHideEvent *event)
{
    emit signalHidded();

    QWidget::hideEvent(event);
}

void MainMenuWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);

}

void MainMenuWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);

}

void MainMenuWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);

}

void MainMenuWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int shadow = 0;
    QRect rect(shadow, shadow, size().width()-shadow*2, size().height()-shadow*2);
    // QRect rect(-1, -1, width()+2, height()+2);

    QPainter painter(this);
    painter.setBrush(us->getNoOpacityColor(us->mainwin_bg_color));
    painter.drawRect(rect);
}

void MainMenuWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() & Qt::Key_Escape)
    {
        emitHide();
        return ;
    }
    return QWidget::keyPressEvent(event);
}

void MainMenuWidget::setOpenedChapterCount(int x)
{
    close_opened_button->setText("关闭已打开的章节(" + QString::number(x) + ")");
}
