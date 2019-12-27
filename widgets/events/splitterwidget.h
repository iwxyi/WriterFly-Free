#ifndef SPLITTERWIDGET_H
#define SPLITTERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QCursor>
#include <QPaintEvent>
#include <QPainter>
#include "fileutil.h"
#include "globalvar.h"

/**
 * 左右调整目录宽度的控件
 */
class SplitterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SplitterWidget(QWidget *parent = nullptr);

    void setDirection(int d);

protected:
	void enterEvent(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

signals:
    void signalMoveEvent(int x);

public slots:

private:
    int direction; // 分割方向，0横向，1纵向(2双向)
    int mx, my;
    bool is_pressing;
};

#endif // SPLITTERWIDGET_H
