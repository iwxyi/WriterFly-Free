#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QStatusBar>
#include <QDockWidget>
#include <QMenuBar>
#ifdef Q_OS_WIN
#include <QFontDataBase>
#endif
#include <QIODevice>
#include <QtConcurrent/QtConcurrent>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "mytitlebar.h"
#include "stackwidget.h"
#include "noveldirgroup.h"
#include "chaptereditor.h"
#include "splitterwidget.h"
#include "defines.h"
#include "globalvar.h"
#include "settings.h"
#include "dragsizewidget.h"
#include "usersettingswindow.h"
#include "usersettingswindow2.h"
#include "transparentcontainer.h"
#include "novelschedulewidget.h"
#include "im_ex_window.h"
#include "sidebarwidget.h"
#include "outlinegroup.h"
#include "globalvar.h"
#include "bezierwavebean.h"
#include "searchpanel.h"
#include "mainmenuwidget.h"
#include "loginwindow.h"
#include "syncstatewindow.h"
#include "darkroomwidget.h"
#include "thememanagewindow.h"
#include "tipbox.h"
#include "shortcutwindow.h"
#include "jiebautil.h"
#include "frisoutil.h"
#include "cardlibgroup.h"
#include "cardeditor.h"

#define MAINDBG if(1) qDebug()

QT_BEGIN_NAMESPACE
    extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

/**
 * 基础窗口，用来完善程序的布局
 */
class BaseWindow : public QMainWindow
{
    Q_OBJECT
public:
    BaseWindow(QMainWindow *parent = nullptr);
    ~BaseWindow();

    struct MenuOrAction {
        MenuOrAction() : menu(nullptr), action(nullptr), seperator(false) {}
        MenuOrAction(QMenu* m) : menu(m), action(nullptr), seperator(false) {}
        MenuOrAction(QAction* a) : menu(nullptr), action(a), seperator(false) {}
        MenuOrAction(int i) : menu(nullptr), action(nullptr), seperator(false) {
            if (i == 1)
                seperator = true;
        }
        bool isMenu() { return menu != nullptr; }
        bool isAction() { return action != nullptr; }
        bool isSeparator() { return seperator; }
        QMenu* menu;
        QAction* action;
        bool seperator;
    };

    void setSidebarShowed(bool show, bool animation = true);
    void setMenuBarShowed(bool show);

protected:
    void initGlobal();
    void initLayout();
    void initDynamicBackground();

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent* event);

    void showWindowAnimation();
    void closeWindowAnimation();
    void toWinMaxAnimation();
    void toWinMinAnimation();
    void toWinRestoreAnimation();

    virtual void detectUpdate();
    void initUserSettingsWindow();
    void initUserSettingsWindow2();
    void initNovelScheduleWindow();
    void initSearchWidget();

    bool execCommand(QString command);
    bool execCommand(QString command, QStringList args);
private:
    QString loadFontFamilyFromFiles(const QString &fontFilePath);
    void initMenu();
    void initBezierWave();
    void initBorderShadow();

    // 动态加载菜单
    void loadMenuFromFile(const QString &filePath);
    MenuOrAction getMenuFromJson(QJsonValue value);
    MenuOrAction addActionsToMenu(QJsonValue value, QMenu* parent_menu);
    void processActionCondition(QAction* action, QString condition);

    QString getProperSelectedText();

public slots:
	// 主窗口标题栏
    //void slotMainSplitterMoved(int x);
    void onButtonWinMinClicked();
    void onButtonWinRestoreClicked();
    void onButtonWinMaxClicked();
    void onButtonWinCloseClicked();
    void onButtonSidebarClicked();
    void onButtonSettingsClicked();
    void adjustTitlebar();
    void toWinMax();
    void toWinMin();
    void toWinRestore();
    void slotSwitchWindowShadow(bool s);
    void rerenderTitlebarPixmap();
    void rerenderTitlebarPixmapLater();
    void resetTitlebarPixmapSize();
    void resetTitlebarPixmapSizeImediately();

    // 窗口布局、样式、背景
    void saveWinPos(QPoint point);
    void saveWinLayout(QPoint point, QSize size);
    void slotWindowPosMoved(QPoint delta);
    void slotWindowSizeChanged(int dw, int dh);
    void updateUI();
    void slotSideBarWidthChanged(); // 拖动侧边栏调整宽度，修改标题栏位置变化
    void initBgPictures();
    void initEmotionFilter();
    void initEnvironmentPictures();

    // 窗口标题栏文字
    void slotSetWindowTitleBarContent(QString title);
    void slotSetTitleNovel(QString t);
    void slotSetTitleChapter(QString t);
    void slotSetTitleFull(QString t);
    // void slotSetTitleWc(int x);
    void slotChapterWordCount(int x);

    // 窗口控件
    void slotTransContainerToClose(int kind);
    void slotTransContainerClosed(int kind);
    void slotShowNovelInfo(QString novel_name, QPoint point);
    void slotShowImExWindow(int kind, QString def_name);
    void slotShowSettings();
    void slotShowSettings2();
    void slotShowMainMenu();
    void slotHideMainMenu();
    void slotShowShortcutWindow();

    // 综合搜索栏
    void slotShowSearchPanel(QString str = "");
    void slotHideSearchPanel();
    void slotInsertText(QString text);
    void slotInsertTextWithSuffix(QString suffix, QString str);
    void slotLocateEdit(int index, int pos, int len);
    void slotEmitCommand(QString command);

    // 用户操作事件
    void slotNovelOpened();
    void slotNovelClosed();
    void slotEditorCountChanged(int x);

    // 通知
    void slotPopNotification(NotificationEntry* noti);
    void slotNotificationCardClicked(NotificationEntry* noti);
    void slotNotificationButtonClicked(NotificationEntry* noti);

    // 菜单控件
    void actionCloudState();
    void actionThemeManage();
    void actionNightTheme();
    void actionDarkRoom();
    void actionCloseOpenedChapter();
    void actionFullScreen();
    void actionHelp();

    // 名片
    void slotAddNewCard();
    void slotRefreshEditingCards();
    void slotShowCardEditor(CardBean *card);
    void slotShowCardEditor(CardBean *card, CardEditor *from);
    void slotCardlibClosed();
    void slotMoveAllCardEditor(QPoint delta, CardEditor* ce);

protected:
    QWidget* center_widget;
    MyTitleBar* titlebar_widget;   // 标题栏
    MainMenuWidget* main_menu;     // 自定义主菜单
    TipBox* tip_box;               // 右下角通知控件

    SideBarWidget *sidebar_widget; // 侧边栏标签页
    NovelDirGroup *dir_layout;     // 目录
    OutlineGroup *outline_group;   // 大纲
    CardlibGroup *cardlib_group;   // 名片

    StackWidget *editors_stack;    // 章节编辑器

    QMenuBar* menubar;                          // 自定义菜单栏
    // DragSizeWidget* drag_size_widget;           // 右下角调整窗口大小的控件
    UserSettingsWindow* user_settings_window;   // 用户设置窗口
    UserSettingsWindow2* user_settings_window2; // 用户设置窗口
    TransparentContainer* trans_ctn;            // 模糊渐变控件背景
    NovelScheduleWidget* novel_schedule_widget; // 作品一览控件
    ImExWindow* im_ex_window;                   // 导入导出窗口
    SearchPanel* search_panel;                  // 搜索栏

    QRect callback_rect;
    QPoint callback_point;

    // 名片
    QList<CardEditor*> card_editors;

    // 背景动画
    bool bezier_wave_inited, bezier_wave_running;
    BezierWaveBean *bw1, *bw2, *bw3, *bw4;
    bool bg_jpg_enabled, bg_png_enabled;
    QPixmap bg_jpg, bg_png;
    QPixmap* editor_blur_win_bg_pixmap, **p_editor_blur_win_bg_pixmap;

    // 四边
    BorderShadow *bs_left, *bs_right, *bs_top, *bs_bottom,
        *bs_topLeft, *bs_topRight, *bs_bottomLeft, *bs_bottomRight;

private:
    QString title_novel, title_chapter, title_full;
    int title_wc;
    int menu_bar_right;
};

#endif // BASEWINDOW_H
