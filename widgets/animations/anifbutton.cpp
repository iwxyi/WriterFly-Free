#include "anifbutton.h"

AniFButton::AniFButton() : GeneralButtonInterface()
{

}

AniFButton::AniFButton(QString caption) : GeneralButtonInterface(caption, nullptr)
{

}

AniFButton::AniFButton(QString caption, QWidget *parent) : GeneralButtonInterface("", caption, parent)
{
    //this->setStyleSheet(QString("AniButton { padding: 0px; }"));
    QString btnStyle("QPushButton{ background: transparent; border-style: outset; border-width: 0px; border-color: #88888866; padding: 5px; }\
                     QPushButton:hover {background-color: rgba(128, 128, 128, 64);}\
                     QPushButton:pressed {background-color: rgba(128, 128, 128, 128); border-style: inset; } ");
    this->setStyleSheet(btnStyle);

    font_size = this->font().pointSize();
    font_size_l = static_cast<int>(font_size * 1.5);

    QFont font = this->font();
    font.setPointSize(static_cast<int>(font_size * 2.5));
    QFontMetrics fm(font);
    int height = fm.height();
    this->setMinimumHeight(height);
}

void AniFButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "fontsize");
    animation->setDuration(300);
    animation->setStartValue(font_size);
    animation->setEndValue(font_size_l);
    animation->start();
}

void AniFButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "fontsize");
    animation->setDuration(300);
    animation->setStartValue(font_size_l);
    animation->setEndValue(font_size);
    animation->start();
}

void AniFButton::setFontSize(int x)
{
    QFont font = this->font();
    font.setPointSize(x);
    this->setFont(font);
}

int AniFButton::getFontSize()
{
    QFont font = this->font();
    return font.pixelSize();
}

void AniFButton::resetFontSize()
{
    setFontSize(font_size);
}

int AniFButton::getSuitableHeight()
{
    QFont font = this->font();
    font.setPointSize(font_size * 3);
    QFontMetrics fm(font);
    int height = fm.height();
    return height;
}
