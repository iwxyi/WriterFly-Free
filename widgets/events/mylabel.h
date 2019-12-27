#ifndef MYLABEL_H
#define MYLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

/**
 * 带有 click 信号的标签
 */
class MyLabel : public QLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget* parent);
    MyLabel(QString text, QWidget* parent);
    MyLabel(QPixmap* pixmap, QWidget* parent);
    MyLabel(QImage* image, QWidget* parent);

protected:
    void mousePressEvent(QMouseEvent* event);

signals:
    void clicked(QPoint point);

};

#endif // MYLABEL_H
