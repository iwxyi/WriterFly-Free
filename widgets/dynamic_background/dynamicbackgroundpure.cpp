#include "dynamicbackgroundpure.h"

DynamicBackgroundPure::DynamicBackgroundPure(QWidget* parent, QColor color1, QColor color2)
    : DynamicBackgroundInterface (parent)
{
    setColor(color1, color2);
}

void DynamicBackgroundPure::setColor(QColor color1, QColor color2)
{
    // 阈值
    colorToArray(c1, color1);
    colorToArray(c2, color2);

    // 设置各种属性
    for (int i = 1; i <= 4; i++)
    {
        a[i] = randBool(); // 随机变化方向
        c[i] = randRange(c1[i], c2[i]); // 初始随机颜色
        d[i] = intToUnity(c2[i]-c1[i]); // 每次变化的方向,false向1变化，true向2变化
        if (c[i] == c1[i]) a[i] = true;
        if (c[i] == c2[i]) a[i] = false;
    }

    draw_color = QColor(c[cR], c[cG], c[cB], c[cA]);
    redraw();
}

void DynamicBackgroundPure::draw(QPainter &painter)
{
    painter.save();
    {
        // 绘制矩形
        if (radius)
        {
            QPainterPath path;
            path.addRoundedRect(getGeometry(), radius, radius);
            painter.fillPath(path, draw_color);
        }
        else
        {
            painter.setBrush(draw_color);
            painter.drawRect(getGeometry());
        }
    }
    painter.restore();
}

void DynamicBackgroundPure::timeout()
{
    // 随机变化
    int t = randRange(1, 4);
    c[t] += a[t] ? d[t] : -d[t];
    if (c[t] == c1[t]) a[t] = true;
    if (c[t] == c2[t]) a[t] = false;

    draw_color = QColor(c[cR], c[cG], c[cB], c[cA]);

    DynamicBackgroundInterface::timeout();
}
