#include "mylabel.h"

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{

}

MyLabel::MyLabel(QString text, QWidget *parent) : QLabel(parent)
{
    setText(text);
}

MyLabel::MyLabel(QPixmap *pixmap, QWidget *parent) : QLabel(parent)
{
    setPixmap(*pixmap);
}

MyLabel::MyLabel(QImage *image, QWidget *parent) : QLabel(parent)
{
    setPixmap(QPixmap::fromImage(*image));
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked(event->pos());
    }
}
