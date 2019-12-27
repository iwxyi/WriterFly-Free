#include "sidebarwidget.h"

SideBarWidget::SideBarWidget(QWidget *parent)
    : QWidget(parent), fixed_max_width(200), is_packing(false)
{
    initView();
}

void SideBarWidget::initView()
{
    this->setMinimumWidth(10);
    // setStyleSheet("QWidget{background: transparent;}");

    tab_widget = new AniTabWidget(this);
    splitter_widget = new SplitterWidget(this);

    QHBoxLayout* main_hlayout = new QHBoxLayout(this);
    main_hlayout->addWidget(tab_widget);
    main_hlayout->addWidget(splitter_widget);
    this->setLayout(main_hlayout);

    main_hlayout->setSpacing(0);
    //main_hlayout->setContentsMargins(0, us->mainwin_titlebar_height, 0, 0);
    main_hlayout->setContentsMargins(0, 0, 0, 0);

    connect(splitter_widget, SIGNAL(signalMoveEvent(int)), this, SLOT(slotFixedWidthChanged(int)));

    // 读取上次的宽度设置
    int x = us->getInt("layout/sidebar_width", 200);
    if (x >= 50 && x <= parentWidget()->width()*4/5)
        fixed_max_width = x;
    else if (x < 50)
        x = 50;
    else if (x > parentWidget()->width()*4/5)
        fixed_max_width = parentWidget()->width()*4/5;
    else
        fixed_max_width = 200;
    setMaximumWidth(fixed_max_width);
    setFixedWidth(fixed_max_width);
//    emit signalWidthChanged(); // 这时候还没有connect，无法使用

    /*// 设置指示器
    indicator->stackUnder(tab_widget->tabBar());
    indicator->setFixedHeight(2);
    indicator->setAutoFillBackground(true);
    indicator->show();
    connect(ani_indicator, SIGNAL(finished()), this, SLOT(moveIndicator()));*/

    updateUI();
    tab_widget->setTabBarAutoHide(true); // 只有一行的时候隐藏
    tab_widget->setTabInBottom();
    tab_widget->setStyleSheet("QTabWidget{border: 1px solid transparent; border-radius:5px;}");

    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));

    // 设置再次打开时重新读取索引
    connect(tab_widget, &AniTabWidget::currentChanged, [=](int x){
        if (tab_widget->count() == 1)
            return;
        us->setVal("recent/sidebar_tab_index", x);
    });
    QTimer::singleShot(10, [=]{
        int index = us->getInt("recent/sidebar_tab_index", 0);
        if (index)
            tab_widget->setCurrentIndex(index);
    });
}

void SideBarWidget::updateUI()
{
    // 侧边栏背景透明的时候，可以设置为透明动画
    // 侧边栏背景完全不透明的时候，设置为不透明动画
    tab_widget->setTransparentBackground(us->mainwin_sidebar_color.alpha() != 255);
    tab_widget->setBaseBacgroundColor(us->mainwin_sidebar_color.alpha() == 255 ? us->mainwin_sidebar_color : Qt::transparent);
    tab_widget->disableSwitchAnimation(us->mainwin_sidebar_color.alpha() != 255 && us->mainwin_sidebar_color.alpha() != 0);
}

void SideBarWidget::addPage(QWidget *widget, QString label)
{
    tab_widget->addTab(widget, label);
}

void SideBarWidget::setPage(int index)
{
    if (tab_widget->currentIndex() != index)
        tab_widget->setCurrentIndex(index);
}

void SideBarWidget::startShowSidebar(int distance)
{
    is_packing = false;
    is_animating = true;

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().left()+distance, geometry().top(), geometry().width(), geometry().height()));
    //animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start();

    /*QGraphicsOpacityEffect *opacity_effect = new QGraphicsOpacityEffect(this);
    opacity_effect->setOpacity(0);
    this->setGraphicsEffect(opacity_effect);

    QPropertyAnimation *animation2 = new QPropertyAnimation(opacity_effect, "opacity", this);
    animation2->setDuration(250);
    animation2->setStartValue(0.2);
    animation2->setEndValue(1);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->start();*/

    connect(animation, &QPropertyAnimation::finished, [=]{
       is_animating = false;
    });
}

/**
 * @brief SideBarWidget::startHideSidebar
 * @param distance  向右移动距离（参数中为负，即左移）
 */
void SideBarWidget::startHideSidebar(int distance)
{
    is_packing = true;
    is_animating = true;

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().left()+distance, geometry().top(), geometry().width(), geometry().height()));
    //animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start();

    /*QGraphicsOpacityEffect *m_pGraphicsOpacityEffect = new QGraphicsOpacityEffect(this);
    m_pGraphicsOpacityEffect->setOpacity(1);
    this->setGraphicsEffect(m_pGraphicsOpacityEffect);

    QPropertyAnimation *animation2 = new QPropertyAnimation(m_pGraphicsOpacityEffect, "opacity", this);
    animation2->setDuration(200);
    animation2->setStartValue(1);
    animation2->setEndValue(0);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->start();*/

    connect(animation, &QPropertyAnimation::finished, [=]{
       is_animating = false;
    });
}

bool SideBarWidget::isPacking()
{
    return is_packing;
}

bool SideBarWidget::isAnimating()
{
    return is_animating;
}

void SideBarWidget::paintEvent(QPaintEvent *event)
{
    //设置背景色;
    QPainter painter(this);
    QPainterPath path_back;
    path_back.addRect(0, 0, this->width()-splitter_widget->width(), this->height());
    if (!this->isHidden() && !rt->isMainWindowMaxing() && us->widget_radius)
    {
        // 空出左边圆角
        QPainterPath lt_path;
        lt_path.addRect(0, 0, us->widget_radius, us->widget_radius);
        QPainterPath lt_round_path;
        lt_round_path.addEllipse( 0, 0, us->widget_radius*2, us->widget_radius*2);
        lt_path -= lt_round_path;
        path_back -= lt_path;

        QPainterPath lb_path;
        lb_path.addRect(0, height()-us->widget_radius, us->widget_radius, us->widget_radius);
        QPainterPath lb_round_path;
        lb_round_path.addEllipse(0, height() - us->widget_radius*2, us->widget_radius * 2, us->widget_radius * 2);
        lb_path -= lb_round_path;
        path_back -= lb_path;

        painter.setClipPath(path_back, Qt::IntersectClip);
    }

    if (0 && isFileExist(rt->IMAGE_PATH+"sidebar_bg.png"))
    {
        painter.drawPixmap(0, 0, this->width()-splitter_widget->width(), this->height(), QPixmap(rt->IMAGE_PATH+"win_bg.jpg"));
    }
    else
    {
        path_back.setFillRule(Qt::WindingFill);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillPath(path_back, QBrush(us->mainwin_sidebar_color));
    }

//    return QWidget::paintEvent(event);
}

void SideBarWidget::resizeEvent(QResizeEvent *event)
{
    return QWidget::resizeEvent(event);
}

void SideBarWidget::slotFixedWidthChanged(int x)
{
    int max_width = parentWidget()->width()/2;
    int min_width = 50;

    if ( (fixed_max_width+x >= min_width && fixed_max_width+x <= max_width) // 标准大小
         || (x<0 && fixed_max_width >= max_width) // 超过右边界后左移
         || (x>0 && fixed_max_width <= min_width) )  // 超过左边界后右移
    {
        fixed_max_width += x;
        //this->setMaximumWidth(fixed_max_width);
        this->setFixedWidth(fixed_max_width);
        us->setVal("layout/sidebar_width", fixed_max_width);

        emit signalWidthChanged();
    }
}

