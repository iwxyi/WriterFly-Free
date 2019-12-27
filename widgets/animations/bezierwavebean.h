#ifndef BEZIERWAVEBEAN_H
#define BEZIERWAVEBEAN_H

#include <QObject>
#include <QWidget>
#include <QPaintEvent>
#include <QList>
#include <QPainter>
#include <random>
#include <QTimer>
#include <QDebug>
#include "time.h"

class BezierWaveBean : public QObject
{
    Q_OBJECT
public:
    BezierWaveBean(QWidget *parent);

    void start();
    void resume();
    void pause();

    void set_count(int x);
    void set_offsety(int x);
    void set_speedx(int x);
    void set_rect(QRect rect);

    QPainterPath getPainterPath(QPainter &painter);

protected:
    int getRandomHeight();

signals:

public slots:
    void slotUpdateAims();
    void slotMovePoints();
    void slotSetOffset();

protected:
    QWidget* target;
    int _offsety; // 多层波浪y轴偏移
    int _max_offsety; // y轴上下偏移的最大值
    QRect _rect;

    QTimer* update_timer;
    QTimer* move_timer;
    QTimer* offset_timer;
    QTimer* pause_timer;

    bool running;

    int count;
    int inter;
    int appear_speedy;

    int speedy, offsety;
    int offsety_direct;
    int speedx, offsetx;

    QList<QPoint>keys;
    QList<QPoint>aim_keys;
};

#endif // BEZIERWAVEBEAN_H
