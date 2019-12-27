#include "aninlabel.h"

AniNLabel::AniNLabel(QWidget *parent) : QWidget(parent)
{

}

void AniNLabel::setShowNum(int x)
{
    Q_UNUSED(x);
}

void AniNLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    return QWidget::paintEvent(e);
}
