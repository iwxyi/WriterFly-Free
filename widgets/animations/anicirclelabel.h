#ifndef ROUNDLABEL_H
#define ROUNDLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QBrush>
#include <QPropertyAnimation>
#include "globalvar.h"

#define ZOOM 1.25

/**
 * 显示颜色的带动画的圆形控件
 */
class AniCircleLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int animation_progress READ getAnimationProgress WRITE setAnimationProgress RESET resetAnimationProgress)

public:
    AniCircleLabel();
    AniCircleLabel(QWidget* parent);

    void setMainColor(QColor c);

protected:
    void paintEvent(QPaintEvent* e);

    void startChangeAnimation();

private:
    int getAnimationProgress();
	void setAnimationProgress(int x);
    void resetAnimationProgress();

private:
    QColor color;
    int dr, dg, db, da;
    int animation_progress;
};

#endif // ROUNDLABEL_H
