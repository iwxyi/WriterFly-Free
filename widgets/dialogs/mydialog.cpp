#include "mydialog.h"

MyDialog::MyDialog(QWidget *parent) : QDialog(parent), dybg(nullptr), titlebar_widget(nullptr), pressing(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    initBorderShadow();
    initView();

    /* QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(15, 15);
    shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
    shadow_effect->setBlurRadius(10);
    this->setGraphicsEffect(shadow_effect); */
}

void MyDialog::showEvent(QShowEvent *event)
{
    // 相当于 updateUI()
    initDybg();
    close_btn->setIconColor(us->global_font_color);
    if (menu_btn != nullptr && !menu_btn->isHidden())
        menu_btn->setIconColor(us->global_font_color);

    QDialog::showEvent(event);
}

void MyDialog::hideEvent(QHideEvent *event)
{
    if (dybg != nullptr)
        dybg->deleteLater();

    QDialog::hideEvent(event);
}

void MyDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 设置圆角裁剪
    if (us->integration_window && (us->widget_radius || us->mainwin_border_size))
    {
        QPainterPath clip_path;
        clip_path.addRoundedRect(us->mainwin_border_size, us->mainwin_border_size, width() - us->mainwin_border_size * 2, height() - us->mainwin_border_size * 2, us->widget_radius, us->widget_radius);
        painter.setClipPath(clip_path, Qt::IntersectClip);
    }

    // 绘制背景
    if (dybg)
    {
        dybg->draw(painter);

        /* if (!us->widget_radius)
        {
            // 画边框
            painter.drawRect(0, 0, size().width()-1, size().height()-1);
        } */
    }
    else
    {
        if (us->widget_radius)
        {
            QPainterPath path;
            path.addRoundedRect(QRect(0, 0, width(), height()), us->widget_radius, us->widget_radius);
            painter.fillPath(path, us->getNoOpacityColor(us->mainwin_bg_color));
        }
        else
            painter.fillRect(QRect(0, 0, width(), height()), us->getNoOpacityColor(us->mainwin_bg_color));
    }
}

void MyDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    close_btn->move(width() - close_btn->width() - us->mainwin_border_size, us->mainwin_border_size);
    menu_btn->move(close_btn->geometry().left() - menu_btn->width(), close_btn->geometry().top());
    if (titlebar_widget != nullptr)
    {
        titlebar_widget->setFixedWidth(this->width() - close_btn->width() - (menu_btn->isHidden() ? 0 : close_btn->width()) - us->mainwin_border_size*2);
    }

    bs_left->adjustGeometry();
    bs_right->adjustGeometry();
    bs_top->adjustGeometry();
    bs_bottom->adjustGeometry();
    bs_topLeft->adjustGeometry();
    bs_topRight->adjustGeometry();
    bs_bottomLeft->adjustGeometry();
    bs_bottomRight->adjustGeometry();
}

void MyDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressing = true;
        press_pos = event->globalPos();
    }

    QDialog::mousePressEvent(event);
}

void MyDialog::mouseMoveEvent(QMouseEvent *event)
{
    QDialog::mouseMoveEvent(event);

	if (pressing)
    {
        event->accept();
        QPoint delta = event->globalPos() - press_pos; // 和一开始的坐标差
        press_pos = event->globalPos();
        this->move(this->pos() + delta);
        emit signalMoved(delta);
        if (QApplication::keyboardModifiers() == Qt::ControlModifier)
            emit signalCtrlMoved(delta);
        else if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
            emit signalShiftMoved(delta);
    }
}

void MyDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        pressing = false;

    QDialog::mouseReleaseEvent(event);
}

void MyDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        emit signalConfirm();
    return QDialog::keyPressEvent(event);
}

void MyDialog::initView()
{
    close_btn = new WinCloseButton(this);
    close_btn->setFixedSize(close_btn->height(), close_btn->height());
    close_btn->setTopRightRadius(us->widget_radius);
    close_btn->move(width() - close_btn->width(), us->mainwin_border_size);
    connect(close_btn, &InteractiveButtonBase::clicked, [=]{
        this->close();
    });

    menu_btn = new PointMenuButton(this);
    menu_btn->setFixedSize(close_btn->size());
    menu_btn->move(close_btn->geometry().left() - menu_btn->width(), us->mainwin_border_size);
    menu_btn->hide();

    main_layout = new QVBoxLayout(this);
    // main_layout->setMargin(us->mainwin_border_size);
    // main_layout->setContentsMargins(5, 5, 5, 5);
    setLayout(main_layout);
}

void MyDialog::initDybg()
{
    if (us->dynamic_bg_model != DynamicBgModel::Dynamic_None)
    {
        dybg = us->createDynamicBackground();
        dybg->setInterval(50);
        dybg->setRadius(us->widget_radius);
        dybg->setWidget(this);
        connect(dybg, SIGNAL(signalRedraw()), this, SLOT(update()));
    }
    else
    {
        if (dybg != nullptr)
            dybg->deleteLater();
        dybg = nullptr;
    }
    update();
}

void MyDialog::setDialogMenu(QMenu* menu)
{
    menu_btn->show();
    menu_btn->setMenu(menu);
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(refreshMenu()));
}

void MyDialog::refreshMenu()
{

}

void MyDialog::addTitleSpacing()
{
    main_layout->addSpacing(close_btn->height());
}

void MyDialog::addTitleBar(QString s)
{
    createTitleBar();

    InteractiveButtonBase* btn = new InteractiveButtonBase(s, this);
    btn->setFixedWidth(close_btn->width());
    btn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    titlebar_hlayout->addWidget(btn);
}

void MyDialog::addTitleBar(QIcon icon, QString s)
{
    createTitleBar();

    InteractiveButtonBase *btn1 = new InteractiveButtonBase(icon, this);
    btn1->setFixedSize(close_btn->size());
    btn1->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    titlebar_hlayout->addWidget(btn1);

    InteractiveButtonBase *btn2 = new InteractiveButtonBase(s, this);
    btn2->setFixedHeight(close_btn->height());
    btn2->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    titlebar_hlayout->addWidget(btn2);
}

void MyDialog::createTitleBar()
{
    titlebar_widget = new QWidget(this);
    titlebar_widget->move(us->mainwin_border_size, us->mainwin_border_size);
    titlebar_widget->setFixedHeight(close_btn->height());
    titlebar_widget->setFixedWidth(this->width() - close_btn->width() - (menu_btn->isHidden() ? 0 : close_btn->width()) - us->mainwin_border_size*2);

    titlebar_hlayout = new QHBoxLayout(titlebar_widget);
    titlebar_hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titlebar_hlayout->setMargin(0);
    titlebar_hlayout->setSpacing(0);
    titlebar_widget->setLayout(titlebar_hlayout);
}

void MyDialog::initBorderShadow()
{
    bs_left = new BorderShadow(this, BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_right = new BorderShadow(this, BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);
    bs_top = new BorderShadow(this, BORDER_TOP, us->mainwin_border_size, us->widget_radius);
    bs_bottom = new BorderShadow(this, BORDER_BOTTOM, us->mainwin_border_size, us->widget_radius);
    bs_topLeft = new BorderShadow(this, BORDER_TOP | BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_topRight = new BorderShadow(this, BORDER_TOP | BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);
    bs_bottomLeft = new BorderShadow(this, BORDER_BOTTOM | BORDER_LEFT, us->mainwin_border_size, us->widget_radius);
    bs_bottomRight = new BorderShadow(this, BORDER_BOTTOM | BORDER_RIGHT, us->mainwin_border_size, us->widget_radius);
}
