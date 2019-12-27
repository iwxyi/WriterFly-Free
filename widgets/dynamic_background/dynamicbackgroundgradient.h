#ifndef DYNAMICBACKGROUNDGRADIENT_H
#define DYNAMICBACKGROUNDGRADIENT_H

#include <QLinearGradient>
#include "dynamicbackgroundinterface.h"

class DynamicBackgroundGradient : public DynamicBackgroundInterface
{
public:
    DynamicBackgroundGradient(QWidget* parent,
        QColor color1 = QColor(230, 230, 250)/*薰衣草紫*/,
        QColor color2 = QColor(204, 204, 255)/*长春花色*/,
        QColor color3 = QColor(240, 248, 255)/*爱丽丝蓝*/,
        QColor color4 = QColor(248, 248, 253)/*幽灵白*/
        );

    void setColor(QColor color1, QColor color2, QColor color3, QColor color4);
    void setColor2(QColor color1, QColor color2);
    void draw(QPainter& painter);
    void setHorizone(bool h);
    void accumulate(int x = 1);

protected:
	void timeout();

private:
    // 颜色属性
    ColorArray c1, c2, c3, c4, c5, c6, cu, cd, cm;
    ColorArray du, dd, dm;
    ColorDirects au, ad, am;

    // 方向
    bool horizone;

    // 中间颜色
    bool use_mid;
    double prop;
	
    QColor draw_coloru, draw_colord, draw_colorm;
};

#endif // DYNAMICBACKGROUNDGRADIENT_H
