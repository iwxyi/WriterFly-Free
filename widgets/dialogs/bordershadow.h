#ifndef BORDERSHADOW_H
#define BORDERSHADOW_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>

enum BorderDirectionFlag {
    BORDER_LEFT = 0x1,
    BORDER_RIGHT = 0x2,
    BORDER_TOP = 0x4,
    BORDER_BOTTOM = 0x8
};

Q_DECLARE_FLAGS(BorderDirection, BorderDirectionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(BorderDirection)

#define MAX_SHADOW 64

class BorderShadow : public QWidget
{
    Q_OBJECT
public:
    BorderShadow(QWidget *parent, BorderDirection direction, int size, int radius);

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent* event) override;

private:
    QColor getOC(int opacity);
    bool isInArea(QPoint pos);

signals:

public slots:
    void adjustGeometry();

private:
    QWidget* widget;
    const BorderDirection direction;
    const int size;
    const int radius;
    const int sr_sum;
    bool is_corner;
    QGradient* gradient;
    QPainterPath path;

    bool moving;
    bool areaing;
    QPoint prev_pos;
    Qt::CursorShape cursor_shape;
};

#endif // BORDERSHADOW_H
