#ifndef DYNAMICBACKGROUNDSHOWWIDGET_H
#define DYNAMICBACKGROUNDSHOWWIDGET_H

#include <QWidget>
#include "dynamicbackgroundpure.h"
#include "dynamicbackgroundgradient.h"

class DynamicBackgroundShowWidget : public QWidget
{
	Q_OBJECT
public:
	DynamicBackgroundShowWidget(QWidget* parent = nullptr)
	{
		dm_bg = new DynamicBackgroundInterface(this);
		dm_bg->setInterval(20);
		dm_bg->showAni();
		connect(dm_bg, SIGNAL(signalRedraw()), this, SLOT(update()));
	}

protected:
	void paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);
		dm_bg->draw(painter);
	}

private:
    DynamicBackgroundInterface* dm_bg;
};

#endif // DYNAMICBACKGROUNDSHOWWIDGET_H
