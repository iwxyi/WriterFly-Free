#include "mainwindow.h"
MainWindow::MainWindow(QMainWindow *parent) :
    BaseWindow(parent)
{
    log("初始化MainWindow");
    initWindow();
    initFirstOpen();
    initData();

    rt->is_initing = false;

    if (us->auto_detect_update || rand() % 10 == 1) // 默认为true
    {
        log("检查更新");
        rt->setSplash("检查版本更新");
        detectUpdate();
    }

    log("关闭splash");
    rt->finishSplash();
    log("初始化完成");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    log("关闭程序前");
    /*if (isFileExist(rt->DOWNLOAD_PATH + "update.zip")) // 现在已经移动到了 basewindow 这个地方
    {
        log("检测到已下载的安装包，进行更新");
        QProcess process;
        process.startDetached("UpUpTool.exe");
    }*/
    return BaseWindow::closeEvent(event);
}

void MainWindow::detectUpdate()
{
    ApplicationUpdateThread* update_thread = new ApplicationUpdateThread();
    connect(update_thread, SIGNAL(signalHasNewVersion(QString,QString)), this, SLOT(slotHasNewVersion(QString,QString)));
    update_thread->start();
}

void MainWindow::initWindow()
{
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/appicon"));
    setWindowTitle(APPLICATION_NAME);

    // 设置温馨祝福
    if (titlebar_widget != nullptr)
    {
        QTimer::singleShot(500, [=]{
            QString path = rt->DATA_PATH + "database/warm_wish.txt";
            if (!isFileExist(path)) path = ":/database/warm_wish";
            QString dir = rt->DBASE_PATH + "warm_wish_extra/";
            QString warm_wish = WarmWishUtil::getWarmWish(path, dir);
            if (!warm_wish.isEmpty())
            {
                titlebar_widget->setTitleContent(warm_wish);

                // 恢复为章节标题
                QTimer::singleShot(500 * warm_wish.length(), [=]{
                    editors_stack->changeWindowTitle();
                });
            }
        });
    }
}

void MainWindow::initData()
{
    // 初始化目录
    dir_layout->initRecent();

    gd->lexicons.setUS(us);

    QString cpu_id = CPUIDUtil::get_cpuId();
    QString us_cpu_id = us->getStr("runtime/CPU_ID");
    if (cpu_id.isEmpty())
    {
        if (us_cpu_id != "0") // 不是已经获取过的
        {
            QMessageBox::information(this, "设备警告", "无法获取您设备的唯一ID，将禁用部分不安全的功能（例如云同步）。");
            us->setVal("runtime/CPU_ID", "0");
        }
    }
    else if (us_cpu_id != cpu_id)
    {
        if (us_cpu_id.isEmpty()) // 新用户或者升级用户
        {

        }
        else // 换了机器
        {

        }
        us->setVal("runtime/CPU_ID", cpu_id);
    }
    rt->cpu_id = cpu_id;

    ac->initData();
    ac->startNetwork();

    gd->dr.restoreDarkRoom(this);

    // 统计启动次数
    int startup_count = us->getInt("runtime/startup_count", 0);
    startup_count++;
    us->setValue("runtime/startup_count", startup_count);
}

void MainWindow::initFirstOpen()
{
    bool first = us->getBool("runtime/first_open", true);
    if (first) // 第一次打开
    {
#if defined(Q_OS_WIN)
        // 桌面快捷方式
        addLinkToDeskTop(QCoreApplication::applicationFilePath(), "写作天下"); // 添加快捷方式

        // 开始菜单快捷方式
        QString startMenuPath=QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation).append("/").append(QStringLiteral("WriterFly"));
        QDir dir(startMenuPath);
        if(!dir.exists())
            dir.mkdir(startMenuPath);
        QFile::link(QCoreApplication::applicationFilePath(), startMenuPath.append("/").append("写作天下.lnk"));
#endif
        refreshAppPath(); // 刷新软件运行位置

        us->setVal("runtime/first_open", false);
        us->setVal("runtime/CPU_ID", CPUIDUtil::get_cpuId());
        us->setVal("runtime/VERSION_NUMBER", VERSION_NUMBER);
        return ;
    }
    else // 不是第一次打开，但是路径变了
    {
        QString app_path = us->getStr("runtime/APP_PATH");
        if (app_path != rt->APP_PATH) // 文件路径变了
        {
            refreshAppPath();
        }
    }

    // 版本升级
    afterUpdate();
}

void MainWindow::refreshAppPath()
{
    if (!us->getStr("runtime/APP_PATH").isEmpty())
    {
        NotificationEntry* noti = new NotificationEntry("APP_PATH", "已修改运行路径", QString("%1 >> %2").arg(us->getStr("runtime/APP_PATH")).arg(rt->APP_PATH));
        noti->desc = QString("程序运行的路径");
        rt->popNotification(noti);
    }
    qDebug() << "已修改文件路径" << us->getStr("runtime/APP_PATH") << " >> " << rt->APP_PATH;
    us->setVal("runtime/APP_PATH", rt->APP_PATH);

    // 修改 Friso 的属性
    if (isFileExist(rt->APP_PATH+"tools/friso/friso.ini"))
    {
        QString friso_ini = readTextFile(rt->APP_PATH+"tools/friso/friso.ini", "GBK");
        if (!friso_ini.isEmpty())
        {
            int start_pos = friso_ini.indexOf("friso.lex_dir");
            if (start_pos == -1)
                return ;
            start_pos += QString("friso.lex_dir").length();
            while (start_pos < friso_ini.length() && (friso_ini.mid(start_pos, 1) == " " || friso_ini.mid(start_pos, 1) == "="))
                start_pos++;
            int end_pos = friso_ini.indexOf("\n", start_pos);
            if (end_pos == -1)
                end_pos = friso_ini.length();
            QString friso_path = rt->APP_PATH+"tools/friso/dict/UTF-8/";
            friso_ini = friso_ini.left(start_pos) + friso_path + friso_ini.right(friso_ini.length() - end_pos);
            writeTextFile(rt->APP_PATH+"tools/friso/friso.ini", friso_ini, "GBK");
        }
        else
        {
            QMessageBox::information(this, "提示", "无法读取Friso分词配置文件【" + rt->APP_PATH+"tools/friso/frisi.ini" + "】\n可能无法使用分词相关的功能");
        }
    }
}

void MainWindow::gotoUpdate(QString url)
{
#if defined (Q_OS_WIN)
        QProcess process(this);
        QStringList list;
        list << url << "update.zip";
        if (process.startDetached("UpUpTool.exe", list))
            return ;
        // 下载失败，qDebug() 并且打开官网
        qDebug() << log("启动下载程序失败！");
#endif
        rt->has_new_version = 3; // 自动下载失败，等待更新
        QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::slotHasNewVersion(QString version, QString url)
{
    log("有新版，开始下载：v"+version+"    "+url);
    // 倒计时自动下载更新
    if (us->auto_detect_update)
    {
        NotificationEntry* noti = new NotificationEntry("UPDATE", "版本更新", "新版："+version+"\n当前："+VERSION_CODE+"\n即将自动下载更新");
        noti->setBtn("忽略本次");
        rt->popNotification(noti);

        // 自动更新倒计时
        QTimer* update_timer = new QTimer(this);
        update_timer->setInterval(5000);
        update_timer->setSingleShot(true);
        update_timer->start();
        connect(update_timer, &QTimer::timeout, [=]{
            gotoUpdate(url);
            update_timer->deleteLater();
        });

        connect(noti, &NotificationEntry::signalBtnClicked, [=]{
            update_timer->stop();
            update_timer->deleteLater();
        });
    }
    else // 需要手动点击按钮才进行更新
    {
        NotificationEntry* noti = new NotificationEntry("UPDATE", "版本更新", "新版："+version+"\n当前："+VERSION_CODE);
        noti->setBtn(1, "下载安装");
        noti->setBtn(2, "自动更新");
        rt->popNotification(noti);
        connect(noti, &NotificationEntry::signalBtnClicked, [=](int x){
            gotoUpdate(url);
            if (x == 2)
                us->setVal("us/auto_detect_update", us->auto_detect_update = true);
        });
    }
}

void MainWindow::slotPackageDownloadFinished()
{
    if (isFileExist(rt->DOWNLOAD_PATH + "update.zip"))
        rt->has_new_version = 2;
}

void MainWindow::afterUpdate()
{
    // 获取旧的版本号
    int old_version = us->getInt("runtime/VERSION_NUMBER", 1000);
    int cur_version = VERSION_NUMBER;

    while (old_version < cur_version)
    {
        processUpdate(++old_version);
        qDebug() << log("版本升级", old_version);
    }

    us->setVal("runtime/VERSION_NUMBER", cur_version);
}

void MainWindow::processUpdate(int version)
{
    switch (version)
    {
    case 1047:
        // 提示背景模糊
        if (bg_jpg_enabled)
        {
            QTimer::singleShot(1000, [=]{ // 因为之前还没有绑定信号，所以需要延迟一下
                NotificationEntry *noti = new NotificationEntry("update_version", "背景模糊", "已开启背景图片模糊功能");
                noti->setBtn(1, "取消模糊", "");
                tip_box->createTipCard(noti);
                connect(noti, &NotificationEntry::signalBtnClicked, [=](int x) {
                    if (x == 1)
                    {
                        us->setVal("us/blur_picture_radius", 0);
                        us->blur_picture_radius = 0;
                        initBgPictures();
                    }
                });
            });
        }
    case 1052:
        if (isFileExist(rt->DATA_PATH + "darkroom_history.txt"))
        {
            renameFile(rt->DATA_PATH + "darkroom_history.txt", rt->HISTORY_PATH + "darkroom_history.txt");
        }
    }
}
