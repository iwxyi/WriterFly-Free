#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QStatusBar>
#include <QProcess>
#include <QUrl>
#include "basewindow.h"
#include "applicationupdatethread.h"
#include "fileutil.h"
#include "warmwishutil.h"
#include "cpu_id_util.h"

namespace Ui {
class MainWindow;
}

/**
 * 主窗口，用来完善额外的非必要选项，例如检测更新、多线程读取词典等
 */
class MainWindow : public BaseWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QMainWindow *parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event);

    void detectUpdate();

private:
    void initWindow();
    void initData();
    void initFirstOpen();
    void refreshAppPath();
    void toastPlatform();
    void gotoUpdate(QString url);
    void afterUpdate();
    void processUpdate(int version);

public slots:
    void slotHasNewVersion(QString version, QString url);
    void slotPackageDownloadFinished();

private:
    QString getCpuId2();
};

#endif // MAINWINDOW_H
