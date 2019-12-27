#ifndef DYNAMICBACKGROUNDPREVIEWWIDGET_H
#define DYNAMICBACKGROUNDPREVIEWWIDGET_H

#include <QWidget>
#include "dynamicbackgroundpure.h"
#include "dynamicbackgroundgradient.h"

class DynamicBackgroundPreviewWidget : public QWidget
{
	Q_OBJECT
public:
    DynamicBackgroundPreviewWidget(DynamicBackgroundInterface* d, QWidget* parent = nullptr) : QWidget(parent), dm_bg(d)
	{
		dm_bg->setInterval(20);
		dm_bg->showAni();
		connect(dm_bg, SIGNAL(signalRedraw()), this, SLOT(update()));
	}

protected:
    void paintEvent(QPaintEvent*)
	{
		QPainter painter(this);
		dm_bg->draw(painter);
	}

public:
    DynamicBackgroundInterface* dm_bg;
};

#endif // DYNAMICBACKGROUNDPREVIEWWIDGET_H
