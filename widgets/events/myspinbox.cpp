#include "myspinbox.h"

MySpinBox::MySpinBox(QWidget *parent) : QSpinBox (parent)
{
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
    initStyle();
}

MySpinBox::MySpinBox(int min, int max, QWidget *parent) : MySpinBox(parent)
{
    setRange(min, max);
}

void MySpinBox::initStyle(QColor main_color, QColor font_color)
{
    int radius = 0; //height() / 2-2;
    setStyleSheet("QSpinBox { background: transparent; border:1px solid rgba(128,128,128,128); border-radius: " + QString::number(radius) + "px; padding-left: 8px; padding-right: 8px; }");
}

void MySpinBox::focusInEvent(QFocusEvent *e)
{
    QTimer::singleShot(10, [=]{
        selectAll();
    });
    return QSpinBox::focusInEvent(e);
}

void MySpinBox::slotValueChanged(int val)
{
    if (val < minimum() || val > maximum()) // 虽然不需要，但是不排除万一有其他出错的地方呢？
            return ;

    // 根据 spin 的值，智能调整不同的单步间隔
    int step = 1;
    if (val < 0) val = -val;
    while (val >= 100)
    {
        if (val % 10 != 0) // 如果最后一位不是 0，退出
            break;
        val /= 10;
        step *= 10;
    }
    this->setSingleStep(step);
}
