#include "stackwidgetanimation.h"

StackWidgetAnimation::StackWidgetAnimation(QWidget *parent, QPixmap *pixmap, QPoint sp, QPoint ep, QSize size, int x)
                    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose); // 父窗口不销毁子窗口也不会销毁，这个属性避免close后一直占用内存

    animation = new QPropertyAnimation(this, "geometry");
    this->pixmap = pixmap;
    this->show();
    connect(animation, SIGNAL(finished()), this, SLOT(slotFinished1()));
    start(pixmap, sp, ep, size);
    this->x = x;
}

void StackWidgetAnimation::start(QPixmap *pixmap, QPoint sp, QPoint ep, QSize size)
{
    Q_UNUSED(pixmap);

    animation->setDuration(ANI_LATEST);
    animation->setStartValue(QRect(sp, QSize(5,5)));
    animation->setEndValue(QRect(ep, size));
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start();
}

void StackWidgetAnimation::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), *pixmap);
    return QWidget::paintEvent(event);
}

void StackWidgetAnimation::slotFinished1()
{
    emit signalAni1Finished(x);

    // 不进行下一阶段的动画
    this->close();
    return ;

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0);
    this->setGraphicsEffect(effect);

    QPropertyAnimation* ani = new QPropertyAnimation(effect, "opacity");
    ani->setDuration(ANI_LATEST);
    ani->setStartValue(1.0);
    ani->setEndValue(0);
    //ani->setEasingCurve(QEasingCurve::InCirc);
    ani->start();

    connect(ani, SIGNAL(finished()), this, SLOT(slotFinished2()));
}

void StackWidgetAnimation::slotFinished2()
{
    this->close();
}
