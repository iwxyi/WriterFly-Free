/**
 * 全局运行状态类
 */

#ifndef RUNTIMEINFO_H
#define RUNTIMEINFO_H

#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QMessageBox>
#include <QObject>
#include <QSplashScreen>
#include <QDateTime>
#include <QTime>
#include "fileutil.h"
#include "defines.h"
#include "notificationentry.h"
#include "splashscreen.h"

class RuntimeInfo : public QObject
{
    Q_OBJECT
public:
    RuntimeInfo(int vn);

    void checkApplicationUpdateFile();
    void initPath();
    void initIntegral();
    void initEggs();
    void startSplash();
    void setSplash(QString msg);
    void finishSplash();

    bool promptCreateNovel(QWidget *w);
    void setLineGuideWidget(QWidget *w);
    void finishLineGuide();

    void setMainWindow(QWidget *w);
    QWidget *getMainWindow();
    bool isMainWindowMaxing();

    void popNotification(NotificationEntry *noti);

signals:
    void signalPopNotification(NotificationEntry *noti);
    void signalMainWindowMaxing(bool m);

public:
    // ==== 路径 ====
    QString APP_PATH;      // 运行路径
    QString DATA_PATH;     // 数据总路径
    QString NOVEL_PATH;    // 小说总路径
    QString CARDLIB_PATH;  // 名片库路径
    QString THEME_PATH;    // 主题路径
    QString IMAGE_PATH;    // 图片路径
    QString ICON_PATH;     // 图标路径
    QString STYLE_PATH;    // 样式路径
    QString DBASE_PATH;    // 数据库路径
    QString DOWNLOAD_PATH; // 下载路径
    QString DOMAIN_PATH;   // 域名路径
    QString SERVER_PATH;   // 服务路径
    QString HISTORY_PATH;  // 操作历史路径

    // ==== 运行 ====
    int version_number;
    SplashScreen *splash;
    qint64 opened_timestamp;
    qint64 startup_timestamp; // 启动时间
    bool is_exiting;
    bool is_initing;     // 初始化中
    int has_new_version; // 0 无更新，1 有更新（下载中），2 下载完毕
    QString cpu_id;

    // ==== 视图 ====
    bool full_screen;
    bool maximum_window;
    int search_panel_item_width;

    // ==== 操作 ====
    QString current_novel;
    QString deleted_novel_name;
    qint64 deleted_novel_timestamp;

    // ==== 其他 ====
    int account_dont_match_device;
    QWidget *main_window;

    // ==== 彩蛋 ====
    bool is_line_guide;
    int line_guide_case;
    QWidget *guide_widget;
};

#endif // RUNTIMEINFO_H
