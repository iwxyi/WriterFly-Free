#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QPixmap>
#include "splitterwidget.h"
#include "defines.h"
#include "globalvar.h"
#include "anitabwidget.h"

class SideBarWidget : public QWidget
{
    Q_OBJECT
public:
    SideBarWidget(QWidget* parent);

    void initView();
    void addPage(QWidget* widget, QString name);
    void setPage(int index);

    void startShowSidebar(int distance);
    void startHideSidebar(int distance);
    bool isPacking();
    bool isAnimating();

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

signals:
    void signalWidthChanged();

public slots:
    void updateUI();
    void slotFixedWidthChanged(int x);

private:
    // 控件
    AniTabWidget* tab_widget;
    SplitterWidget* splitter_widget;

    // flag
    int fixed_max_width; // 最大值
    bool is_packing; // 目录是否折叠起来
    bool is_animating;
};

#endif // SIDEBARWIDGET_H
