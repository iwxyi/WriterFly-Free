#ifndef ANITABWIDGET_H
#define ANITABWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTabBar>
#include "globalvar.h"
#include "defines.h"
#include "mylabel.h"
#include <QTimer>

class AniTabWidget : public QTabWidget
{
	Q_OBJECT
public:
    AniTabWidget(QWidget* parent);

    void setTabInBottom();
    void preventSwitchAnimationOnce();
    void disableSwitchAnimation(bool d);
    void setTransparentBackground(bool transparent = true, bool level = 0);
    void setBaseBacgroundColor(QColor c);
    void setTabEqualWidth(bool e);

private:
    void switchAnimation(DirectType direct);
    int max(int a, int b) { return a >= b ? a : b; }
    int min(int a, int b) { return a < b ? a : b; }

protected:
    void resizeEvent(QResizeEvent* event);

public slots:
	void updateUI();
	void switchTab(int index);
	void moveIndicator();

protected:
    QLabel* indicator;

	// 切换tab动画
    QPixmap pixmap_pre, pixmap_new, pixmap_base;
    int pre_index;
    QColor base_background_color;

    // 切换指示器动画
    bool disable_animation;
    QPropertyAnimation* ani_indicator;
    QRect aim_rect;
    bool tab_in_bottom;
    bool tab_eq_width;
    bool transparent_background;
    int transparent_level;
};

#endif // ANITABWIDGET_H
