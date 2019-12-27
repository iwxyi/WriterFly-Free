#include "splitterwidget.h"

SplitterWidget::SplitterWidget(QWidget *parent) : QWidget(parent)
{
    direction = 0;
    mx = my = 0;
    is_pressing = false;

    this->setMinimumWidth(5);
    this->setMaximumWidth(5);
    this->setMouseTracking(true);
}

void SplitterWidget::setDirection(int d)
{
    if (d == 0 || d == 1)
        direction = d;
}

void SplitterWidget::enterEvent(QEvent* e)
{
    setCursor(Qt::SizeHorCursor);
    return QWidget::enterEvent(e);
}

void SplitterWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        is_pressing = true;
        this->setCursor(Qt::SizeHorCursor);
    }

    mx = static_cast<int>(e->windowPos().x());
    my = static_cast<int>(e->windowPos().y());
}

void SplitterWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        is_pressing = false;
    }
}

void SplitterWidget::mouseMoveEvent(QMouseEvent *e)
{
    int x = static_cast<int>(e->windowPos().x());
    int y = static_cast<int>(e->windowPos().y());
    if (is_pressing)
    {
        emit signalMoveEvent(x-mx);
    }
    mx = x;
    my = y;

    if (x >= this->geometry().left() && x <= this->geometry().right() && y >= this->geometry().top() && y <= this->geometry().bottom())
    {
        this->setCursor(Qt::SizeHorCursor); // 左右改变大小
    }
    else if (!is_pressing)
    {
        // this->setCursor(Qt::ArrowCursor); // 变回默认的箭头（不需要）
    }
}

void SplitterWidget::paintEvent(QPaintEvent *e)
{
    if (!us->split_shade)
        return QWidget::paintEvent(e);

    QPainterPath path;
    // path.setFillRule(Qt::WindingFill);
    path.addRect(0, 0, width(), height());

    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);

    // QColor color(0, 0, 0, 50);
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0, QColor(128, 128, 128, 128));
    gradient.setColorAt(1, QColor(128, 128, 128, 0));

    painter.fillPath(path, gradient);

    /* for (int i = 0; i < this->width(); i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(i, 0, 1, this->height());
        color.setAlpha((i+2)*(i+2)>>2);
        painter.setPen(color);
        painter.drawPath(path);
    } */

    return QWidget::paintEvent(e);
}
