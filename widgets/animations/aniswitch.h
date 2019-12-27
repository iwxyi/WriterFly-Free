#ifndef ANISWITCH_H
#define ANISWITCH_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "globalvar.h"

/**
 * 带动画的开关控件
 */
class AniSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int slider READ getSliderPos WRITE setSliderPos RESET resetSliderPos)
public:
    explicit AniSwitch(QWidget *parent = nullptr);
    explicit AniSwitch(bool s, QWidget *parent);
    explicit AniSwitch(bool s);

    void setOnOff(bool b);
    bool isOn();
    void setResponse(bool r);
    void setResponse();

protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);

public:
	int getSliderPos();
	void setSliderPos(int x);
	void resetSliderPos();

signals:
    void signalSwitch(bool b);

public slots:
    void slotSwitch(bool b);
    void slotSwitch();

private:
    bool state;
    int slider_pos; // 百分比来的位置
    bool response;
};

#endif // ANISWITCH_H
