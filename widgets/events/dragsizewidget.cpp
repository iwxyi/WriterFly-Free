#include "dragsizewidget.h"

DragSizeWidget::DragSizeWidget(QWidget *parent) : QWidget(parent)
{
    mx = my = 0;
    is_pressing = false;
    this->setMaximumSize(10, 10);
    this->setMouseTracking(true);
    this->setGraphicsEffect(&effect);
    effect.setOpacity(0.5);
    timer.setInterval(3000);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, [this]{
        effect.setOpacity(0.5);
    });
}

void DragSizeWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        is_pressing = true;
        this->setCursor(Qt::SizeFDiagCursor);
    }

    mx = static_cast<int>(e->windowPos().x());
    my = static_cast<int>(e->windowPos().y());
}

void DragSizeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        is_pressing = false;

        emit signalReleased();
    }
}

void DragSizeWidget::mouseMoveEvent(QMouseEvent *e)
{
    int x = static_cast<int>(e->windowPos().x());
    int y = static_cast<int>(e->windowPos().y());

    if (is_pressing)
    {
        emit signalMoveEvent(x-mx, y-my);
    }
    mx = x;
    my = y;

    //if (x >= this->geometry().left() && x <= this->geometry().right() && y >= this->geometry().top() && y <= this->geometry().bottom())
    if (geometry().contains(e->windowPos().toPoint()))
    {
        this->setCursor(Qt::SizeFDiagCursor); // 左上-右下 改变大小
        effect.setOpacity(1);
        timer.stop();
        timer.start();
    }
    else if (!is_pressing) // 这个好像是不生效的……因为出去后就不监听了
    {
        effect.setOpacity(0.5);
        this->setCursor(Qt::ArrowCursor); // 变回默认的箭头
    }
}

void DragSizeWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(":/icons/rbc"));
    return QWidget::paintEvent(e);
}
