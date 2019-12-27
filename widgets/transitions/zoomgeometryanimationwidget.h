#ifndef ZOOMOUTANIMATIONWIDGET_H
#define ZOOMOUTANIMATIONWIDGET_H


#include <QWidget>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QPoint>
#include <QSize>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include "globalvar.h"

#define ANI_LATEST 300

/**
 * 打开内部控件（子窗口）显示缩放动画的临时控件
 */
class ZoomGeometryAnimationWidget : public QWidget
{
    Q_OBJECT
public:
    ZoomGeometryAnimationWidget(QWidget* parent, QPixmap* pixmap, QRect sr, QRect er, int x = 0);
    ZoomGeometryAnimationWidget(QWidget* parent, QPixmap* pixmap, QRect sr, QRect er, bool faster);
    ZoomGeometryAnimationWidget(QWidget* parent, QPixmap* pixmap, QRect sr, QRect er, QEasingCurve easing, int x = 0);
    ZoomGeometryAnimationWidget(QWidget* parent, QPixmap* pixmap, QRect sr, QRect er, QEasingCurve easing, int opa, int x);

    void start(QPixmap* pixmap, QRect sr, QRect er);

protected:
    void paintEvent(QPaintEvent* event);

public slots:
    void slotFinished1();
    void slotFinished2();

signals:
    void signalAni1Finished(int x);

private:
    QPropertyAnimation* animation;
    QPixmap* pixmap;
    int duration;
    bool is_animating;
    QVariant currentValue;
    int x;
};

#endif // ZOOMOUTANIMATIONWIDGET_H
