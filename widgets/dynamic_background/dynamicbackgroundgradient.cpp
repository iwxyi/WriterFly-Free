#include "dynamicbackgroundgradient.h"

DynamicBackgroundGradient::DynamicBackgroundGradient(QWidget* parent,
        QColor color1, QColor color2, QColor color3,  QColor color4)
    : DynamicBackgroundInterface (parent), horizone(false), use_mid(false), prop(-1)
{
	setColor(color1, color2, color3, color4);
}

void DynamicBackgroundGradient::setColor(QColor color1, QColor color2, QColor color3, QColor color4)
{
	// 阈值
    colorToArray(c1, color1);
    colorToArray(c2, color2);
    colorToArray(c3, color3);
    colorToArray(c4, color4);

    // 设置各种属性
    for (int i = 1; i <= 4; i++)
    {
        au[i] = randBool(); // 随机变化方向
        cu[i] = randRange(c1[i], c2[i]); // 初始随机颜色
        du[i] = intToUnity(c2[i]-c1[i]); // 每次变化的方向,false向1变化，true向2变化
        if (cu[i] == c1[i]) au[i] = true;
        if (cu[i] == c2[i]) au[i] = false;
    }
    for (int i = 1; i <= 4; i++)
    {
        ad[i] = randBool(); // 随机变化方向
        cd[i] = randRange(c3[i], c4[i]); // 初始随机颜色
        dd[i] = intToUnity(c4[i]-c3[i]); // 每次变化的方向,false向1变化，true向2变化
        if (cd[i] == c3[i]) ad[i] = true;
        if (cd[i] == c4[i]) ad[i] = false;
    }

    draw_coloru = QColor(cu[cR], cu[cG], cu[cB], cu[cA]);
    draw_colord = QColor(cd[cR], cd[cG], cd[cB], cd[cA]);
    redraw();
}

void DynamicBackgroundGradient::setColor2(QColor color1, QColor color2)
{
    // 阈值
    colorToArray(c5, color1);
    colorToArray(c6, color2);

    // 设置各种属性
    for (int i = 1; i <= 4; i++)
    {
        am[i] = randBool(); // 随机变化方向
        cm[i] = randRange(c5[i], c6[i]); // 初始随机颜色
        dm[i] = intToUnity(c6[i]-c5[i]); // 每次变化的方向,false向1变化，true向2变化
        if (cm[i] == c5[i]) am[i] = true;
        if (cm[i] == c6[i]) am[i] = false;
    }

    draw_colorm = QColor(cm[cR], cm[cG], cm[cB], cm[cA]);

    use_mid = true; // 三种颜色默认开启
    prop = 0.5;
    redraw();
}

void DynamicBackgroundGradient::draw(QPainter &painter)
{
    painter.save();
    {
        painter.setRenderHint(QPainter::Antialiasing, true);

        // 获取位置
        QRect rect = getGeometry();
        QPoint pu, pd, pm;
        if (!horizone)
        {
            pu = QPoint(rect.left()+rect.width()/2, rect.top());
            pd = QPoint(QPoint(rect.left()+rect.width()/2, rect.bottom()));
        }
        else
        {
            pu = QPoint(rect.left(), rect.top()+rect.height()/2);
            pd = QPoint(rect.right(), rect.top()+rect.height()/2);
        }

        // 开启渐变
        QLinearGradient linear(pu, pd);
        linear.setColorAt(0, draw_coloru);
        linear.setColorAt(show_ani_progress?show_ani_progress/100.0:1, draw_colord);

        double m_prop = prop + accumulation / 100.0;

        if (use_mid)
        {
            m_prop = show_ani_progress?prop*show_ani_progress/100.0:prop;
            linear.setColorAt(m_prop, draw_colorm);
        }
        else if (accumulation)
        {
            m_prop = 0.5 + accumulation / 100.0;
            linear.setColorAt(m_prop, QColor(
                                    (draw_coloru.red() + draw_colord.red())/2,
                                    (draw_coloru.green() + draw_colord.green())/2,
                                    (draw_coloru.blue() + draw_colord.blue())/2,
                                    (draw_coloru.alpha() + draw_colord.alpha())/2
                                ));
        }

        linear.setSpread(QGradient::PadSpread);

        if (radius)
        {
            QPainterPath path;
            path.addRoundedRect(rect, radius, radius);
            painter.fillPath(path, linear);
        }
        else
        {
            painter.setBrush(linear);
            painter.drawRect(rect);
        }
    }
    painter.restore();
}

void DynamicBackgroundGradient::setHorizone(bool h)
{
    horizone = h;
    redraw();
}

void DynamicBackgroundGradient::accumulate(int x)
{
    accumulation += x;
    if (accumulation < -accumulation_max)
        accumulation = -accumulation_max;
    else if (accumulation > accumulation_max)
        accumulation = accumulation_max;
    redraw();
}

void DynamicBackgroundGradient::timeout()
{
    // 随机变化
	int t = randRange(1, 4);
    if (qrand() % 2)
    {
    	cu[t] += au[t] ? du[t] : -du[t];
    	if (cu[t] == c1[t]) au[t] = true;
    	if (cu[t] == c2[t]) au[t] = false;

        draw_coloru = QColor(cu[cR], cu[cG], cu[cB], cu[cA]);
    }
    else
    {
    	cd[t] += ad[t] ? dd[t] : -dd[t];
    	if (cd[t] == c3[t]) ad[t] = true;
    	if (cd[t] == c4[t]) ad[t] = false;

        draw_colord = QColor(cd[cR], cd[cG], cd[cB], cd[cA]);
    }

    DynamicBackgroundInterface::timeout();
}
