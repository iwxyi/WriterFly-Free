#ifndef STACKWIDGETANIMATION_H
#define STACKWIDGETANIMATION_H

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
 * 打开章节缩放动画的临时控件
 */
class StackWidgetAnimation : public QWidget
{
    Q_OBJECT
public:
    StackWidgetAnimation(QWidget* parent, QPixmap* pixmap, QPoint sp, QPoint ep, QSize size, int x);

    void start(QPixmap* pixmap, QPoint sp, QPoint ep, QSize size);

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

#endif // STACKWIDGETANIMATION_H
