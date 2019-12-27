#include "bordershadow.h"

BorderShadow::BorderShadow(QWidget *parent, BorderDirection direction, int size, int radius) : QWidget(parent),
    widget(parent), direction(direction), size(size), radius(radius), sr_sum(size+radius), moving(false), areaing(false)
{
    if ((direction == BORDER_LEFT) || (direction == BORDER_RIGHT) || (direction == BORDER_TOP) || (direction == BORDER_BOTTOM)) // 线性
    {
        if (direction == BORDER_LEFT)
            gradient = new QLinearGradient(size, 0, 0, 0);
        else if (direction == BORDER_RIGHT)
            gradient = new QLinearGradient(0, 0, size, 0);
        else if (direction == BORDER_TOP)
            gradient = new QLinearGradient(0, size, 0, 0);
        else if (direction == BORDER_BOTTOM)
            gradient = new QLinearGradient(0, 0, 0, size);
        else
            gradient = new QGradient;
        gradient->setColorAt(0, getOC(MAX_SHADOW));
        gradient->setColorAt(1, getOC(0));

        path.clear();
        path.addRect(0,0,width(),height());
    }
    else // 圆角
    {
        QPainterPath outer_path;
        outer_path.addEllipse(-sr_sum, -sr_sum, sr_sum * 2, sr_sum * 2);
        QPainterPath inner_path;
        inner_path.addEllipse(-radius, -radius, radius * 2, radius * 2);
        outer_path -= inner_path;
        path = outer_path;

        gradient = new QRadialGradient(QPoint(0, 0), sr_sum);
        gradient->setColorAt(0, getOC(MAX_SHADOW));
        gradient->setColorAt((double)radius / sr_sum, getOC(MAX_SHADOW));
        gradient->setColorAt(1, getOC(0));
    }

    switch (direction)
    {
    case BORDER_LEFT:
    case BORDER_RIGHT:
        cursor_shape = Qt::SizeHorCursor;
        is_corner = false;
        break;
    case BORDER_TOP:
    case BORDER_BOTTOM:
        cursor_shape = Qt::SizeVerCursor;
        is_corner = false;
        break;
    case BORDER_TOP | BORDER_LEFT:
    case BORDER_BOTTOM | BORDER_RIGHT:
        cursor_shape = Qt::SizeFDiagCursor;
        is_corner = true;
        break;
    case BORDER_TOP | BORDER_RIGHT:
    case BORDER_BOTTOM | BORDER_LEFT:
        cursor_shape = Qt::SizeBDiagCursor;
        is_corner = true;
        break;
    default:
        cursor_shape = Qt::ArrowCursor;
    }

    if (is_corner)
        setMouseTracking(true); // 捕获鼠标事件才能判断是不是在圆角
}

void BorderShadow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    switch (direction)
    {
    case BORDER_LEFT:
        painter.fillPath(path, *gradient);
        break;
    case BORDER_RIGHT:
        painter.fillPath(path, *gradient);
        break;
    case BORDER_TOP:
        painter.fillPath(path, *gradient);
        break;
    case BORDER_BOTTOM:
        painter.fillPath(path, *gradient);
        break;
    case BORDER_TOP | BORDER_LEFT:
        painter.translate(width(), height());
        painter.fillPath(path, *gradient);
        break;
    case BORDER_TOP | BORDER_RIGHT:
        painter.translate(0, height());
        painter.fillPath(path, *gradient);
        break;
    case BORDER_BOTTOM | BORDER_LEFT:
        painter.translate(width(), 0);
        painter.fillPath(path, *gradient);
        break;
    case BORDER_BOTTOM | BORDER_RIGHT:
        painter.fillPath(path, *gradient);
        break;
    }

    return QWidget::paintEvent(event);
}

void BorderShadow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!is_corner || isInArea(event->pos()))
        {
            prev_pos = mapToGlobal(event->pos());
            event->accept();
            moving = true;
            return;
        }
    }
    return QWidget::mousePressEvent(event);
}

void BorderShadow::mouseMoveEvent(QMouseEvent *event)
{
    // 尺寸调整
    if (is_corner && !moving)
    {
        if (!areaing && isInArea(event->pos()))
        {
            areaing = true;
            prev_pos = mapToGlobal(event->pos());
            setCursor(cursor_shape);
        }
        else if (areaing && !isInArea(event->pos()))
        {
            areaing = false;
            setCursor(Qt::ArrowCursor);
        }
    }
    if (moving)
    {
        event->accept();

        QPoint delta = mapToGlobal(event->pos()) - prev_pos;
        if (delta == QPoint(0, 0)) // 坐标没有变
            return ;
        prev_pos = mapToGlobal(event->pos());
        QRect rect = widget->geometry();

        // 移动坐标
        if (direction & BORDER_LEFT)
        {
            if (widget->minimumWidth() > 0 && rect.width() - delta.x() < widget->minimumWidth()) // 判断最小值
                delta.setX(widget->minimumWidth() - rect.width());
            if (widget->maximumWidth() > 0 && rect.width() - delta.x() > widget->maximumWidth()) // 判断最大值
                delta.setX(widget->maximumWidth() - rect.width());
            rect.setLeft(rect.left() + delta.x());
        }
        if (direction & BORDER_RIGHT)
        {
            rect.setRight(rect.right() + delta.x());
        }
        if (direction & BORDER_TOP)
        {
            if (widget->minimumHeight() > 0 && rect.height() - delta.y() < widget->minimumHeight())
                delta.setY(widget->minimumHeight() - rect.height());
            if (widget->maximumHeight() > 0 && rect.height() - delta.y() > widget->maximumHeight())
                delta.setY(widget->maximumHeight() - rect.height());
            rect.setTop(rect.top() + delta.y());
        }
        if (direction & BORDER_BOTTOM)
        {
            rect.setBottom(rect.bottom() + delta.y());
        }

        widget->setGeometry(rect);
        return ;
    }

    return QWidget::mouseMoveEvent(event);
}

void BorderShadow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        moving = false;
        areaing = false;
    }

    return QWidget::mouseReleaseEvent(event);
}

void BorderShadow::enterEvent(QEvent *event)
{
    if (!is_corner || isInArea(mapFromGlobal(QCursor::pos())))
    {
        setCursor(cursor_shape);
        areaing = true;
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        areaing = false;
    }
    event->accept();
    return QWidget::enterEvent(event);
}

QColor BorderShadow::getOC(int opacity)
{
    return QColor(128, 128, 128, opacity);
}

/**
 * 判断四个角落的点在不在阴影区域
 * 圆心为角落的圆心
 */
bool BorderShadow::isInArea(QPoint pos)
{
    static int srsr = sr_sum * sr_sum / 2;
    bool in_area = true;
    if (!QRect(0,0,sr_sum,sr_sum).contains(pos)) // 必定成立，否则不会触发事件（按下除外）
        return false;
    if (direction == (BORDER_TOP | BORDER_LEFT))
    {
        if ((sr_sum - pos.x()) * (sr_sum - pos.x()) + (sr_sum - pos.y()) * (sr_sum - pos.y()) < srsr)
            in_area = false;
    }
    else if (direction == (BORDER_TOP | BORDER_RIGHT))
    {
        if (pos.x() * pos.x() + (sr_sum - pos.y()) * (sr_sum - pos.y()) < srsr)
            in_area = false;
    }
    else if (direction == (BORDER_BOTTOM | BORDER_LEFT))
    {
        if ((sr_sum - pos.x()) * (sr_sum - pos.x()) + pos.y() * pos.y() < srsr)
            in_area = false;
    }
    else if (direction == (BORDER_BOTTOM | BORDER_RIGHT))
    {
        if (pos.x() * pos.x() + pos.y() * pos.y() < srsr)
            in_area = false;
    }
    return in_area;
}

void BorderShadow::adjustGeometry()
{
    switch (direction)
    {
    case BORDER_LEFT:
    {
        setGeometry(0, sr_sum, size, widget->height() - sr_sum*2);
        path.clear();
        path.addRect(0,0,width(),height());
        break;
    }
    case BORDER_RIGHT:
    {
        setGeometry(widget->width()-size, sr_sum, size, widget->height() - sr_sum * 2);
        path.clear();
        path.addRect(0,0,width(),height());
        break;
    }
    case BORDER_TOP:
    {
        setGeometry(sr_sum, 0, widget->width() - sr_sum*2, size);
        path.clear();
        path.addRect(0,0,width(),height());
        break;
    }
    case BORDER_BOTTOM:
    {
        setGeometry(sr_sum, widget->height()-size, widget->width() - sr_sum * 2, size);
        path.clear();
        path.addRect(0,0,width(),height());
        break;
    }
    case BORDER_TOP | BORDER_LEFT:
    {
        setGeometry(0, 0, sr_sum, sr_sum);
        break;
    }
    case BORDER_TOP | BORDER_RIGHT:
    {
        setGeometry(widget->width() - sr_sum, 0, sr_sum, sr_sum);
        break;
    }
    case BORDER_BOTTOM | BORDER_LEFT:
    {
        setGeometry(0, widget->height() - sr_sum, sr_sum, sr_sum);
        break;
    }
    case BORDER_BOTTOM | BORDER_RIGHT:
    {
        setGeometry(widget->width() - sr_sum, widget->height() - sr_sum, sr_sum, sr_sum);
        break;
    }
    }
}

