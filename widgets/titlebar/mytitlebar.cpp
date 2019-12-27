#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QMouseEvent>
#include "mytitlebar.h"
#include "defines.h"
#include "globalvar.h"

MyTitleBar::MyTitleBar(QWidget *parent)
    : QWidget(parent), color_R(255), color_G(250), color_B(250), margin_left(0), is_pressed(false), is_transparent(true)
{
    initControl();
    initConnections();
    initStyle();
    initMenu();

    this->setTitleContent(APPLICATION_NAME);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

MyTitleBar::~MyTitleBar()
{

}

void MyTitleBar::initControl()
{
    button_rect = QRect(0, 0, 1, 1);

    title_content_widget = new AniVLabel(this);
    title_content_widget->setFixedHeight(us->widget_size);

    settings_btn = new WinMenuButton(this);
    min_btn = new WinMinButton(this);
    max_btn = new WinMaxButton(this);
    restore_btn = new WinRestoreButton(this);
    close_btn = new WinCloseButton(this);
    sidebar_btn = new WinSidebarButton(this);
    close_btn->setBgColor(QColor(240, 77, 64, 192), QColor(255, 47, 34, 192));

    settings_btn->delayShowed(1400, QPoint(0, 1));
    min_btn->delayShowed(1200, QPoint(0, 1));
    max_btn->delayShowed(1000, QPoint(0, 1));
    restore_btn->delayShowed(1000, QPoint(0, 1));
    close_btn->delayShowed(800, QPoint(0, 1));

    int border = us->widget_size;
    QSize size = QSize(border, border);
    min_btn->setFixedSize(size);
    restore_btn->setFixedSize(size);
    max_btn->setFixedSize(size);
    close_btn->setFixedSize(size);
    sidebar_btn->setFixedSize(size);
    settings_btn->setFixedSize(size);

    title_content_widget->setObjectName("Title");
    min_btn->setObjectName("Min");
    restore_btn->setObjectName("Restore");
    max_btn->setObjectName("Max");
    close_btn->setObjectName("Close");
    sidebar_btn->setObjectName("Sidebar");
    settings_btn->setObjectName("Settings");

    min_btn->setToolTip(QStringLiteral("最小化"));
    restore_btn->setToolTip(QStringLiteral("还原"));
    max_btn->setToolTip(QStringLiteral("最大化"));
    close_btn->setToolTip(QStringLiteral("关闭"));
    sidebar_btn->setToolTip(QStringLiteral("展开侧边栏"));
    settings_btn->setToolTip(QStringLiteral("菜单"));

    titlebar_left_margin = new QLabel(this);
    titlebar_left_margin->setFixedSize(us->widget_size, 1);
    titlebar_left_margin->hide();

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addWidget(sidebar_btn);
    hlayout->addWidget(titlebar_left_margin);
//    hlayout->addWidget(left_margin_widget);
    hlayout->addWidget(title_content_widget);
    hlayout->addWidget(settings_btn);
    hlayout->addWidget(min_btn);
    hlayout->addWidget(restore_btn);
    hlayout->addWidget(max_btn);
    hlayout->addWidget(close_btn);

    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(0);

    // 设置拉伸
    title_content_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    this->setFixedHeight(us->widget_size);
    this->setWindowFlags(Qt::FramelessWindowHint);

    restore_btn->setVisible(false);
    sidebar_btn->setVisible(us->getBool("view/side_bar_button", false));
    sidebar_btn->setState(!us->getBool("view/side_bar_showed", true));
    if (sidebar_btn->isVisible())
        sidebar_btn->delayShowed(1600, QPoint(1, 0));

    // Mac 布局
    min_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    restore_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    max_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    close_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    sidebar_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    settings_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

void MyTitleBar::initConnections()
{
    connect(min_btn, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
    connect(restore_btn, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
    connect(max_btn, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
    connect(close_btn, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
    connect(sidebar_btn, SIGNAL(clicked()), this, SLOT(onButtonSidebarClicked()));
    connect(settings_btn, SIGNAL(clicked()), this, SLOT(onButtonSettingsClicked()));
}

void MyTitleBar::initStyle()
{
    QString styleSheet = QString("QPushButton {background-color: transparent; border: none; }\
                                QPushButton:hover {background-color: rgb(220, 220, 220); }\
                                QPushButton:pressed {background-color: rgb(192, 192, 192); }");
//    min_btn->setStyleSheet(styleSheet);
//    restore_btn->setStyleSheet(styleSheet);
//    max_btn->setStyleSheet(styleSheet);
    // sidebar_btn->setStyleSheet(styleSheet);
    // settings_btn->setStyleSheet(styleSheet);
    QString styleSheet_redbg = QString("QPushButton {background-color: transparent; border: none; }\
                                       QPushButton:hover {background-color: rgb(240, 77, 64); }\
                                       QPushButton:pressed {background-color: rgb(255, 47, 34); }");
//    close_btn->setStyleSheet(styleSheet_redbg);

    if (isFileExist(rt->STYLE_PATH + "titlebar.qss"))
        setStyleSheet(readTextFile(rt->STYLE_PATH + "titlebar.qss"));
    if (isFileExist(rt->STYLE_PATH + "titlebar_button.qss"))
    {
        styleSheet = readTextFile(rt->STYLE_PATH + "titlebar_button.qss");
//        min_btn->setStyleSheet(styleSheet);
//        restore_btn->setStyleSheet(styleSheet);
//        max_btn->setStyleSheet(styleSheet);
//        close_btn->setStyleSheet(styleSheet);
        // sidebar_btn->setStyleSheet(styleSheet);
        // settings_btn->setStyleSheet(styleSheet);
}
}

void MyTitleBar::initMenu()
{
    menu = new QMenu(this);
    sidebar_showed_action = new QAction(thm->icon("menu/show_sidebar"), "显示侧边栏", this);
    sidebar_button_action = new QAction(thm->icon("menu/show_sidebar_btn"), "显示侧边栏按钮", this);
    menubar_showed_action = new QAction(thm->icon("menu/show_menubar"), "显示菜单栏", this);
    menu->addAction(sidebar_showed_action);
    menu->addAction(sidebar_button_action);
#if !defined(Q_OS_MAC)
    menu->addAction(menubar_showed_action); // Mac菜单栏统一的，不需要隐藏
#endif

    sidebar_showed_action->setCheckable(true);
    sidebar_button_action->setCheckable(true);
    menubar_showed_action->setCheckable(true);

    sidebar_showed_action->setChecked(us->side_bar_showed);
    sidebar_button_action->setChecked(us->getBool("view/side_bar_button", false));
    menubar_showed_action->setChecked(us->menu_bar_showed);

    connect(sidebar_showed_action, &QAction::triggered, [=]{
        sidebar_showed_action->setChecked(!us->side_bar_showed);
        us->setVal("view/side_bar_showed", !us->side_bar_showed);
//        emit signalButtonSidebarClicked();
        sidebar_btn->simulateStatePress(us->side_bar_showed);
    });
    connect(sidebar_button_action, &QAction::triggered, [=]{
        bool b = !us->getBool("view/side_bar_button", false);
        sidebar_button_action->setChecked(b);
        sidebar_btn->setVisible(b);
        us->setVal("view/side_bar_button", b);
    });
    connect(menubar_showed_action, &QAction::triggered, [=]{
        bool b = !us->menu_bar_showed;
        menubar_showed_action->setChecked(b);
        emit signalMenuBarShowedChanged(b);
    });
}

void MyTitleBar::updateUI()
{
    min_btn->setIconColor(us->accent_color);
    max_btn->setIconColor(us->accent_color);
    restore_btn->setIconColor(us->accent_color);
    close_btn->setIconColor(us->accent_color);
    settings_btn->setIconColor(us->accent_color);
    sidebar_btn->setIconColor(us->accent_color);

    thm->setWidgetStyleSheet(this->menu, "menu");

    // avoid titlebar's content's font color
    title_content_widget->setStyleSheet("QLabel { color:"+us->getColorString(us->global_font_color) + "; }");
}

void MyTitleBar::setBackgroundColor(int r, int g, int b, bool isTransparent)
{
    color_R = r;
    color_G = g;
    color_B = b;
    is_transparent = isTransparent;
    update(); // 重新绘制（调用paintEvent事件）
}

void MyTitleBar::setTitleIcon(QString filePath, QSize IconSize)
{
    QPixmap titleIcon(filePath);
}

void MyTitleBar::setTitleContent(QString titleContent, int titleFontSize)
{
    Q_UNUSED(titleFontSize);
    // 设置标题字体大小;
    /*QFont font = m_pTitleContent->font();
    font.setPointSize(titleFontSize);
    m_pTitleContent->setFont(font);*/
    // 设置标题内容;
    title_content_widget->setMainText(titleContent);
    title_content = titleContent;
}

void MyTitleBar::setTitleContent2(QString titleContent, int titleFontSize)
{
    Q_UNUSED(titleFontSize);
    title_content_widget->setStaticText(titleContent);
    title_content = titleContent;
}

void MyTitleBar::setTitleWidth(int width)
{
    this->setFixedWidth(width);
}

void MyTitleBar::saveRestoreInfo(const QPoint point, const QSize size)
{
    restore_pos = point;
    restore_size = size;
}

void MyTitleBar::getRestoreInfo(QPoint &point, QSize &size)
{
    point = restore_pos;
    size = restore_size;
}

void MyTitleBar::setRestoreInfo(QPoint point, QSize size)
{
    restore_pos = point;
    restore_size = size;
}

void MyTitleBar::paintEvent(QPaintEvent *event)
{
    // 是否设置标题透明;
    if (!is_transparent)
    {
        //设置背景色;
        QPainter painter(this);
        QPainterPath pathBack;
        pathBack.setFillRule(Qt::WindingFill);
        pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(pathBack, QBrush(QColor(color_R, color_G, color_B)));
    }

    // 当窗口最大化或者还原后，窗口长度变了，标题栏的长度应当一起改变;
    // 这里减去m_windowBorderWidth ，是因为窗口可能设置了不同宽度的边框;
    // 如果窗口有边框则需要设置m_windowBorderWidth的值，否则m_windowBorderWidth默认为0;
    /*if (margin_left + this->width() != (this->parentWidget()->width())) // 怎么想到这种方法的，也太聪明了吧！
    {
        this->setFixedWidth(this->parentWidget()->width()-margin_left);
    }

    if (this->geometry().left() != margin_left) // 保证位置正确
    {
        this->move(margin_left, this->geometry().top());
    }*/
    QWidget::paintEvent(event);
}

void MyTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 通过最大化按钮的状态判断当前窗口是处于最大化还是原始大小状态;
    // 或者通过单独设置变量来表示当前窗口状态;
    if (max_btn->isVisible())
    {
        onButtonMaxClicked();
    }
    else
    {
        onButtonRestoreClicked();
    }


    return QWidget::mouseDoubleClickEvent(event);
}

void MyTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (isMaxxing())
    {
        // 在窗口最大化时禁止拖动窗口;
    }
    else
    {
        is_pressed = true;
        start_move_pos = event->globalPos();
    }

    return QWidget::mousePressEvent(event);
}

void MyTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (is_pressed && !rt->full_screen) // 按住并且不是全屏状态
    {
        QPoint movePoint = event->globalPos() - start_move_pos;
        QPoint widgetPos = this->parentWidget()->pos();
        this->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
        start_move_pos = event->globalPos();
        //QMessageBox::information(NULL, "save", QString("%1, %2").arg(this->parentWidget()->pos().x()).arg(this->parentWidget()->pos().y()));
        emit windowPosMoved(movePoint);
        is_moved = true;
    }
    return QWidget::mouseMoveEvent(event);
}

void MyTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    is_pressed = false;
    if (is_moved) {
        emit windowPosChanged(this->parentWidget()->pos());
        // emit changeFinished(this->parentWidget()->pos(), QSize(this->parentWidget()->width(), this->parentWidget()->height()));
        is_moved = false;
    }
    return QWidget::mouseReleaseEvent(event);
}

void MyTitleBar::contextMenuEvent(QContextMenuEvent *event)
{
    sidebar_showed_action->setChecked(us->side_bar_showed);
    sidebar_button_action->setChecked(sidebar_btn->isVisible());

    menu->exec(QCursor::pos()); // 得用 QCursor::pos()，而非 event->pos()
    event->accept();
}

bool MyTitleBar::isMaxxing()
{
    return rt->maximum_window;
    /*if (restore_btn->isVisible()) return true;
    if (max_btn->isVisible()) return false;
    return us->getBool("layout/win_max", false);*/
}

bool MyTitleBar::simulateMax()
{
    rt->maximum_window = true;
    button_rect = max_btn->geometry();
    max_btn->setVisible(false);
    restore_btn->setVisible(true);
    emit signalButtonMaxClicked();
    return true;
}

AniVLabel* MyTitleBar::getContentLabel()
{
    return title_content_widget;
}

void MyTitleBar::setMarginLeftWidgetShowed(bool b)
{
    titlebar_left_margin->setVisible(b);
}

QRect MyTitleBar::getBtnRect()
{
    return button_rect;
}

void MyTitleBar::setFixedWidgetHeight(int h)
{
    this->setFixedHeight(h);
    sidebar_btn->setFixedSize(h, h);
    settings_btn->setFixedSize(h, h);
    max_btn->setFixedSize(h, h);
    restore_btn->setFixedSize(h, h);
    min_btn->setFixedSize(h, h);
    close_btn->setFixedSize(h, h);
    title_content_widget->setFixedWidgetHeight(h);
}

void MyTitleBar::onButtonMinClicked()
{
    button_rect = min_btn->geometry();
    emit signalButtonMinClicked();
}

void MyTitleBar::onButtonRestoreClicked()
{
    if (rt->full_screen)
    {
        emit signalButtonRestoreClicked();
        return ;
    }
    rt->maximum_window = false;
    button_rect = restore_btn->geometry();
    restore_btn->setVisible(false);
    if (min_btn->isVisible())
        max_btn->setVisible(true);
    emit signalButtonRestoreClicked();
}

void MyTitleBar::onButtonMaxClicked()
{
    if (rt->full_screen)
    {
        emit signalButtonMaxClicked();
        return ;
    }
    rt->maximum_window = true;
    button_rect = max_btn->geometry();
    max_btn->setVisible(false);
    if (min_btn->isVisible())
        restore_btn->setVisible(true);
    emit signalButtonMaxClicked();
}

void MyTitleBar::onButtonCloseClicked()
{
    button_rect = close_btn->geometry();
    emit signalButtonCloseClicked();
}

void MyTitleBar::onButtonSidebarClicked()
{
    button_rect = sidebar_btn->geometry();
    emit signalButtonSidebarClicked();
}

void MyTitleBar::onButtonSettingsClicked()
{
    button_rect = settings_btn->geometry();
    if (settings_btn->getState())
        emit signalButtonSettingsClicked();
}

void MyTitleBar::onRollTitle()
{
    // 已废除
}

/*
 * 模拟鼠标点击出现
 */
void MyTitleBar::slotMenuShow()
{
    settings_btn->simulateStatePress(true);
}

/*
 * 模拟鼠标点击消失
 */
void MyTitleBar::slotMenuHide()
{
#if defined (Q_OS_WIN)
    settings_btn->simulateStatePress(false, true);
#else
    settings_btn->simulateStatePress(false);
#endif
}

void MyTitleBar::showSidebarButton(bool show)
{
    sidebar_btn->setVisible(show);
}

void MyTitleBar::setSidebarButtonState(bool s)
{
    sidebar_btn->setState(s);
}

bool MyTitleBar::isWinButtonHidden()
{
    return min_btn->isHidden();
}

void MyTitleBar::showWinButtons(bool maxxing)
{
    settings_btn->show();
    min_btn->show();
    close_btn->show();

    if (maxxing)
        restore_btn->show();
    else
        max_btn->show();
    us->win_btn_showed = true;
    if (us->getStr("view/win_btn_showed", true) != true)
        us->setVal("view/win_btn_showed", true);
}

void MyTitleBar::hideWinButtons(bool menu, bool save)
{
    if (menu)
        settings_btn->hide();
    min_btn->hide();
    max_btn->hide();
    restore_btn->hide();
    close_btn->hide();
    us->win_btn_showed = false;
    if (save)
        us->setVal("view/win_btn_showed", false);
}

void MyTitleBar::setLeftCornerBtnRadius(int r)
{
    sidebar_btn->setTopLeftRadius(r);
}

void MyTitleBar::setRightCornerBtnRadius(int r)
{
    close_btn->setTopRightRadius(r);
}
