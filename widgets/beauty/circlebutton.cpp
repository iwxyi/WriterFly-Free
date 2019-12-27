#include "circlebutton.h"

CircleButton::CircleButton(QWidget *parent) : GeneralButtonInterface(parent), square(false)
{
    initView();
}

CircleButton::CircleButton(QString icon, QWidget *parent) : GeneralButtonInterface (icon, parent), square(false)
{
    this->icon_path = icon;
    initView();
}

CircleButton::CircleButton(QIcon icon, QWidget *parent) : GeneralButtonInterface (icon, parent), square(false)
{
    initView();
}

CircleButton::CircleButton(QIcon icon, QString text, QWidget *parent) : GeneralButtonInterface (icon, text, parent), square(false)
{
    initView();
}

CircleButton::CircleButton(QString icon, QString text, QWidget *parent) : GeneralButtonInterface (text, parent), square(false)
{
    this->icon_path = icon;
    initView();
}

CircleButton::CircleButton(bool squ, QString icon, QWidget *parent) : GeneralButtonInterface (icon, parent), square(squ)
{
    this->icon_path = icon;
    initView();
}

void CircleButton::initView()
{
    fore_alpha = 255;
    back_alpha = 0;
    m_pixmap_setted = false;
    store_direct = DIRECT_NONE;

    int height = this->height(), width = this->width();
    int b = height > width ? width : height;

    QString border_radius = square ? "0" : QString("%1").arg(b / 2);

    QString style = "QPushButton{ padding: 2px; border-radius:"+border_radius+"px; background:rgba(255,255,255,0%);}\
            QPushButton:hover{background:rgba(128,128,128,10%);}\
            QPushButton:pressed{background:rgba(128,128,128,20%);}\
            QPushButton::menu-indicator{image:none;}";
    setStyleSheet(style);

    // 如果不是大宽度，则设置成圆形
    //this->setMaximumSize(b, b); // 换成这个会导致大纲菜单按钮是方形的（莫名）
    this->setFixedSize(b, b);

    ani_label = new QLabel(this);

    QPixmap pix(icon_path);
    QBitmap bitmap = pix.mask();
    pix.fill(us->accent_color);
    pix.setMask(bitmap);
    m_pixmap = pix;

    updateUI();

    // 设置动画
    ani_label->setScaledContents(true);
    ani_label->setStyleSheet("padding: 7px");
    ani_label->stackUnder(this);
    ani_label->hide();

    ani = new QPropertyAnimation(ani_label, "geometry");
    connect(ani, &QPropertyAnimation::finished, [=]{
        if (ani_label->width() > this->geometry().width() / 2)
            this->fore_alpha = 255;
        else
            this->fore_alpha = 0;
        updateUI();
        ani_label->hide();
    });
}

void CircleButton::updateUI()
{
    if (icon_path == "") return ;

    QPixmap pix(icon_path);
    QBitmap bitmap = pix.mask();
    if (fore_alpha != 255)
        pix.fill(us->getOpacityColor(us->accent_color, fore_alpha));
    else
        pix.fill(us->accent_color);
    pix.setMask(bitmap);

    QIcon icon(pix);
    this->setIcon(icon);

    pix.fill(us->accent_color);
    pix.setMask(bitmap);
    m_pixmap = pix;
    ani_label->setPixmap(m_pixmap);
}

void CircleButton::showFore()
{
    if (isFixed()) return ; // 固定，不显示动画

    // 创建一个新的控件来实现动画效果
    QRect er(this->geometry());
    er.moveTo(0,0);
    QRect sr(er.width()/2,er.height()/2,1,1);
    ani_label->show();
    ani->stop();
    ani->setStartValue(sr);
    ani->setEndValue(er);
    ani->setDuration(300);
    ani->start();
}

void CircleButton::hideFore()
{
    if (isFixed()) return ;

    fore_alpha = 0;
    updateUI();

    // 创建一个新的控件来实现动画效果
    QRect sr(this->geometry());
    sr.moveTo(0,0);
    QRect er(sr.width()/2,sr.height()/2,1,1);
    ani_label->show();
    ani->stop();
    ani->setStartValue(sr);
    ani->setEndValue(er);
    ani->setDuration(300);
    ani->start();
}

void CircleButton::showBack()
{

}

void CircleButton::hideBack()
{

}

void CircleButton::disableFixed()
{
    GeneralButtonInterface::disableFixed();
    fore_alpha = 0;
    updateUI();
}

void CircleButton::setFixed()
{
    GeneralButtonInterface::setFixed();

    this->fore_alpha = 255;
    updateUI();
}
