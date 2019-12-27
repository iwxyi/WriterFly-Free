#include "circlefloatbutton.h"

CircleFloatButton::CircleFloatButton(QWidget *parent) : GeneralButtonInterface (parent)
{
    initView();
}

CircleFloatButton::CircleFloatButton(QString text, QWidget *parent) : GeneralButtonInterface("", text, parent)
{
    initView();
}

CircleFloatButton::CircleFloatButton(QIcon icon, QString text, QWidget *parent) : GeneralButtonInterface (icon, text, parent)
{
    initView();
}

void CircleFloatButton::initView()
{
    fore_alpha = back_alpha = 0;

    int height = this->height(), width = this->width();
    int b = height > width ? width : height;

    // 限定高度，两边设置成圆形
    this->setFixedHeight(b);

    updateUI();
}

void CircleFloatButton::updateUI()
{
    int height = this->height(), width = this->width();
    int b = height > width ? width : height;
    QString border_radius = us->round_view ? QString("%1").arg(b / 2) : "0";
    QString style = "QPushButton{ padding:-2px; border:1px solid "+us->getOpacityColorString(us->accent_color, 180)+"; border-radius:"+border_radius+"px;}\
            QPushButton:hover{color:white;background:"+us->getOpacityColorString(us->accent_color, 180)+";}\
            QPushButton:pressed{border:1px solid "+us->getColorString(us->accent_color)+"; background:"+us->getColorString(us->accent_color)+";}";
    this->setStyleSheet(style);
}

void CircleFloatButton::showFore()
{
    fore_alpha = 200;
    updateUI();
}

void CircleFloatButton::hideFore()
{
    fore_alpha = 0;
    updateUI();
}

void CircleFloatButton::showBack()
{

}

void CircleFloatButton::hideBack()
{

}
