#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include "globalvar.h"
#include "anivlabel.h"

#include "winminbutton.h"
#include "winmaxbutton.h"
#include "winrestorebutton.h"
#include "winclosebutton.h"
#include "winmenubutton.h"
#include "winsidebarbutton.h"

/**
 * 自定义窗口标题栏控件
 */
class MyTitleBar : public QWidget
{
        Q_OBJECT
public:
    MyTitleBar(QWidget* parent);
    ~MyTitleBar();

    void setBackgroundColor(int r, int g, int b, bool isTransparent = false); // 设置标题栏背景色及是否设置标题栏背景色透明
    void setTitleIcon(QString filePath, QSize IconSize = QSize(25, 25));      // 设置标题栏图标
    void setTitleContent(QString titleContent, int titleFontSize = 9);        // 设置标题内容
    void setTitleContent2(QString titleContent, int titleFontSize = 9);       // 设置标题内容（不带动画）
    void setTitleWidth(int width);                                            // 设置标题栏宽度

    void saveRestoreInfo(const QPoint point, const QSize size); // 保存/获取 最大化前窗口的位置和大小
    void getRestoreInfo(QPoint& point, QSize& size);
    void setRestoreInfo(QPoint point, QSize size);
    bool isMaxxing();
    bool simulateMax();

    void showSidebarButton(bool show);
    void setSidebarButtonState(bool s);
    bool isWinButtonHidden();
    void showWinButtons(bool maxxing = false);
    void hideWinButtons(bool menu = false, bool save = true);

    QRect getBtnRect(); // 获取按钮点击位置
    void setFixedWidgetHeight(int h);
    AniVLabel* getContentLabel();
    void setMarginLeftWidgetShowed(bool b);
    void setRightCornerBtnRadius(int r);
    void setLeftCornerBtnRadius(int r);

private:
    void paintEvent(QPaintEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent *event);

    void initControl();                            // 初始化控件
    void initConnections();                        // 信号槽的绑定
    void initStyle();                              // 编辑样式
    void initMenu();
    void loadStyleSheet(const QString& sheetName); // 加载样式文件

signals:
    // 按钮触发的信号
    void signalButtonMinClicked();
    void signalButtonRestoreClicked();
    void signalButtonMaxClicked();
    void signalButtonCloseClicked();
    void signalButtonSidebarClicked();
    void signalButtonSettingsClicked();
    void signalMenuBarShowedChanged(bool b);
    // 位置变化的信号
    void windowPosMoved(QPoint delta);   // 实时
    void windowPosChanged(QPoint point); // 变化结束，建议保存

public slots:
    void updateUI();

public slots:
    // 按钮触发的槽
    void onButtonMinClicked();
    void onButtonRestoreClicked();
    void onButtonMaxClicked();
    void onButtonCloseClicked();
    void onButtonSidebarClicked();
    void onButtonSettingsClicked();
    void onRollTitle();

    void slotMenuShow();
    void slotMenuHide();

private:
    QLabel* titlebar_left_margin;                  // 标题栏图标
    AniVLabel* title_content_widget;         // 标题栏内容
    WinMinButton* min_btn;        // 最小化按钮
    WinRestoreButton* restore_btn;    // 最大化还原按钮
    WinMaxButton* max_btn;        // 最大化按钮
    WinCloseButton* close_btn;      // 关闭按钮
    WinSidebarButton* sidebar_btn;    // 侧边栏按钮
    WinMenuButton* settings_btn;   // 设置按钮

    int color_R, color_G, color_B; // 标题栏背景色

    int margin_left;                // 相对窗口最左边偏移的位置
    QPoint restore_pos;              // 最大化，最小化变量
    QSize restore_size;

    bool is_moved;                     // 是否被移动
    bool is_pressed;                 // 移动窗口的变量
    QPoint start_move_pos;

    QString title_content;           // 标题栏内容
    bool is_transparent;             // 标题栏是否透明

    QRect button_rect;                  // 按钮坐标

    // ==== 菜单 ====
    QMenu* menu;
    QAction* sidebar_showed_action;
    QAction* sidebar_button_action;
    QAction* menubar_showed_action;
};

#endif // CUSTOMTITLEBAR_H
