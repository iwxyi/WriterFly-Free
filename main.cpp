/**
 * 主函数，所有程序的入口
 */

#include <QApplication>
#include <QStyleFactory>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include "mainwindow.h"
#include "globalvar.h"

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(true); // 据说是避免不同分辨率导致显示的字体大小不一致

    QApplication a(argc, argv);

    // 全局初始化
    initGlobal();

    MainWindow w;
    w.show();

    int result_code = a.exec();
    deleteGlobal();
    return result_code;
};
