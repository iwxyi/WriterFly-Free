#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include "globalvar.h"
#include "winclosebutton.h"
#include "pointmenubutton.h"
#include "dynamicbackgroundgradient.h"
#include "interactivebuttonbase.h"
#include "bordershadow.h"

class MyDialog : public QDialog
{
    Q_OBJECT
public:
    MyDialog(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void setDialogMenu(QMenu* menu);
    void addTitleSpacing();
    void addTitleBar(QString s = "");
    void addTitleBar(QIcon icon, QString s);
    void createTitleBar();

private:
    void initView();
    void initDybg();
    void initBorderShadow();

public slots:
    virtual void refreshMenu();

signals:
    void signalConfirm();
    void signalMoved(QPoint point);
    void signalCtrlMoved(QPoint point);
    void signalShiftMoved(QPoint point);
    void signalAltMoved(QPoint point);

protected:
    DynamicBackgroundInterface* dybg;
    WinCloseButton* close_btn;
    PointMenuButton* menu_btn;
    QVBoxLayout* main_layout;

    QWidget* titlebar_widget;
    QHBoxLayout* titlebar_hlayout;
    bool pressing;
    QPoint press_pos;

    BorderShadow *bs_left, *bs_right, *bs_top, *bs_bottom,
        *bs_topLeft, *bs_topRight, *bs_bottomLeft, *bs_bottomRight;
};

#endif // MYDIALOG_H
