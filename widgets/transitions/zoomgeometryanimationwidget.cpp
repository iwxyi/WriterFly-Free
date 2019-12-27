#include "zoomgeometryanimationwidget.h"

ZoomGeometryAnimationWidget::ZoomGeometryAnimationWidget(QWidget *parent, QPixmap *pixmap, QRect sr, QRect er, int x)
                    : QWidget(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose); // 父窗口不销毁子窗口也不会销毁，这个属性避免close后一直占用内存

    animation = new QPropertyAnimation(this, "geometry");
    animation->setEasingCurve(QEasingCurve::OutQuad); // 默认
    this->pixmap = pixmap;
    this->show();
    connect(animation, SIGNAL(finished()), this, SLOT(slotFinished1()));
    start(pixmap, sr, er);
    this->x = x;

    this->setStyleSheet("background-color:transparent; border:none; border-radius:3px;");
}

ZoomGeometryAnimationWidget::ZoomGeometryAnimationWidget(QWidget *parent, QPixmap *pixmap, QRect sr, QRect er, bool faster)
    : ZoomGeometryAnimationWidget(parent, pixmap, sr, er, x)
{
    if (faster)
        animation->setDuration(ANI_LATEST / 2);
}

ZoomGeometryAnimationWidget::ZoomGeometryAnimationWidget(QWidget *parent, QPixmap *pixmap, QRect sr, QRect er, QEasingCurve easing, int x)
    : ZoomGeometryAnimationWidget(parent, pixmap, sr, er, x)
{
    animation->setEasingCurve(easing);
}

ZoomGeometryAnimationWidget::ZoomGeometryAnimationWidget(QWidget *parent, QPixmap *pixmap, QRect sr, QRect er, QEasingCurve easing, int opa, int x)
    : ZoomGeometryAnimationWidget(parent, pixmap, sr, er, x)
{
    if (easing != QEasingCurve::Linear)
        animation->setEasingCurve(easing);

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(opa / 255.0);
    this->setGraphicsEffect(effect);
}

void ZoomGeometryAnimationWidget::start(QPixmap *pixmap, QRect sr, QRect er)
{
    Q_UNUSED(pixmap);
    animation->setDuration(ANI_LATEST);
    animation->setStartValue(sr);
    animation->setEndValue(er);
    animation->start();
}

void ZoomGeometryAnimationWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), *pixmap);
    return QWidget::paintEvent(event);
}

void ZoomGeometryAnimationWidget::slotFinished1()
{
    emit signalAni1Finished(x);

    this->close();

    return ;

    /*QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0);
    this->setGraphicsEffect(effect);

    QPropertyAnimation* ani = new QPropertyAnimation(effect, "opacity");
    ani->setDuration(ANI_LATEST);
    ani->setStartValue(1.0);
    ani->setEndValue(0);
    ani->start();

    connect(ani, SIGNAL(finished()), this, SLOT(slotFinished2()));*/
}

void ZoomGeometryAnimationWidget::slotFinished2()
{
    this->close();
}

