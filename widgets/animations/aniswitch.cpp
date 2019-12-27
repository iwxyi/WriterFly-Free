#include "aniswitch.h"

AniSwitch::AniSwitch(QWidget *parent) : QWidget(parent)
{
    state = false;
    slider_pos = 0;
    response = false;
}

AniSwitch::AniSwitch(bool s, QWidget *parent) : QWidget(parent)
{
    state = s;
	slider_pos = s ? 100 : 0;
    response = false;
}

AniSwitch::AniSwitch(bool s)
{
    state = s;
	slider_pos = s ? 100 : 0;
    response = false;
}

void AniSwitch::setOnOff(bool b)
{
    state = b;
    slider_pos = b ? 100 : 0;
    this->update();
}

bool AniSwitch::isOn()
{
    return state;
}

void AniSwitch::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    int h = height(), w = width()*9/10-4;
    if (w > h*2) w = h * 2;
    if (w < h*2) h = w / 2;
    int r = (h+1)/3;
    int l = (width()-w)-4, t = (height()-h)>>1; // 靠右对齐

    //qDebug() << "h:" << h << "  w:" << w << "  r:" << r;

    // 画背景圆角矩形
    QPainterPath pathBack;
    pathBack.setFillRule(Qt::WindingFill);
    pathBack.addRoundedRect(QRect(l, t+h/6, w, r<<1), r, r);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QColor color(0x88+(us->accent_color.red()-0x88)*slider_pos/100,
                 0x88+(us->accent_color.green()-0x88)*slider_pos/100,
                 0x88+(us->accent_color.blue()-0x88)*slider_pos/100);
    //QColor color2(0x88, 0x88, 0x88+0x66*slider_pos/100);
    painter.fillPath(pathBack, QBrush(color));

    // 画圆形滑块外圈
    QPainterPath pathBack3;
    pathBack3.setFillRule(Qt::WindingFill);
    pathBack3.addEllipse(QRect(l+(w-r-r)*slider_pos/100-2, t+h/6-2, (r<<1)+4, (r<<1)+4));
    painter.fillPath(pathBack3, QBrush(QColor(0xEE, 0xEE, 0xEE)));

    // 画圆形滑块
    QPainterPath pathBack2;
    pathBack2.setFillRule(Qt::WindingFill);
    pathBack2.addEllipse(QRect(l+(w-r-r)*slider_pos/100, t+h/6, r<<1, r<<1));
    painter.fillPath(pathBack2, QBrush(QColor(0xFF, 0xFF, 0xFF)));

    return QWidget::paintEvent(e);
}

void AniSwitch::mousePressEvent(QMouseEvent *e)
{
    if (!response)
        return QWidget::mousePressEvent(e);

    slotSwitch(!state);
    emit signalSwitch(state);
    return QWidget::mousePressEvent(e);
}

void AniSwitch::slotSwitch(bool b)
{
	if (b == state) return ;

	state = b;

	// 切换动画
	QPropertyAnimation *animation = new QPropertyAnimation(this, "slider");
	animation->setDuration(300);
	animation->setStartValue(b?0:100);
	animation->setEndValue(b?100:0);
	animation->setEasingCurve(QEasingCurve::OutBack); // QEasingCurve::InOutQuad 平稳曲线
	animation->start();
}

void AniSwitch::slotSwitch()
{
    slotSwitch(!state);
}

int AniSwitch::getSliderPos()
{
	return slider_pos;
}

void AniSwitch::setSliderPos(int x)
{
	if (x >= -10 && x <= 110)
		slider_pos = x;
	update();
}

void AniSwitch::resetSliderPos()
{
	slider_pos = state ? 100 : 0;
}

void AniSwitch::setResponse(bool r)
{
    response = r;
}

void AniSwitch::setResponse()
{
    response = true;
}

