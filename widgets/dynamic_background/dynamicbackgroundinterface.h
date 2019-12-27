#ifndef DYNAMICBACKGROUNDINTERFACE_H
#define DYNAMICBACKGROUNDINTERFACE_H

#include <QObject>
#include <QWidget>
#include <QColor>
#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QDebug>

#define PI 3.1415926

#define REFRESH_INTERVAL 2000
#define cR 1
#define cG 2
#define cB 3
#define cA 4

typedef int ColorArray[5];
typedef bool ColorDirects[5];

class DynamicBackgroundInterface : public QObject
{
    Q_OBJECT
public:
    DynamicBackgroundInterface(QWidget *parent);

    void setWidget(QWidget* widget);
    QWidget* getWidget();
    virtual void setInterval(int iv = 1000);
    virtual void setHorizone(bool) {}
    virtual void setAngle(double a = 0);
    void setRadius(int r = 0);

    virtual void setColor(QColor) {}
    virtual void setColor(QColor, QColor) {}
    virtual void setColor(QColor, QColor, QColor) {}
    virtual void setColor(QColor, QColor, QColor, QColor) {}
    virtual void setColor2(QColor) {}
    virtual void setColor2(QColor, QColor) {}
    virtual void draw(QPainter&) {}

    void showAni();
    void setAccumulate(int x);
    virtual void accumulate(int x = 1);


protected:
    virtual QRect getGeometry();

    int randRange(int min, int max);
    bool randBool();
    int intToUnity(int v);
    void colorToArray(int* arr, QColor c);

signals:
    void signalRedraw();

public slots:
    virtual void timeout();
    virtual void redraw();

protected:
    QTimer* timer;
    int interval;
    QWidget* widget;

    double angle; // PI 的角度
    int radius;

    int show_ani_progress; // 出现动画进度
    int accumulation;      // 某一项累计的数值
    int accumulation_max;  // 累计最大值
};

#endif // DYNAMICBACKGROUNDINTERFACE_H
