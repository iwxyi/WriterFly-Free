#include "runtimeinfo.h"


RuntimeInfo::RuntimeInfo(int vn)
{
    version_number = vn;

    opened_timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    startup_timestamp = 0;
    is_exiting = false;
    is_initing = true;
    has_new_version = 0;
    full_screen = false;
    maximum_window = false;
    main_window = nullptr;
    search_panel_item_width = 0;

    current_novel = "";
    deleted_novel_name = "";
    deleted_novel_timestamp = 0;

    account_dont_match_device = 0;

    initPath();
    initIntegral();
    initEggs();

    startSplash();
}

void RuntimeInfo::checkApplicationUpdateFile()
{
    QString zip_path = APP_PATH + "/download/update.zip";
}

void RuntimeInfo::initPath()
{
#if defined(Q_OS_WIN)
    // 如果没有设置通用目录，则选择安装文件夹
    if (isFileExist(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)))
        APP_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        // C:/Users/Administrator/AppData/Roaming/WriterFly
    else
        APP_PATH = QCoreApplication::applicationDirPath();
#elif defined(Q_OS_ANDROID)
    APP_PATH = "/data/data/com.iwxyi.writerfly/";
#elif defined(Q_OS_MAC) or defined(Q_OS_LINUX) or defined(Q_OS_UNIX) or defined (Q_OS_ANDROID)
    APP_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
    if (!APP_PATH.endsWith("/")) APP_PATH += "/";
    DATA_PATH = APP_PATH+"data/";
    NOVEL_PATH = DATA_PATH + "novels/";
    CARDLIB_PATH = DATA_PATH+"cardlibs/";
    THEME_PATH = DATA_PATH + "themes/";
    IMAGE_PATH = DATA_PATH+"images/";
    ICON_PATH = DATA_PATH+"icons/";
    STYLE_PATH = DATA_PATH+"styles/";
    DBASE_PATH = DATA_PATH+"database/";
    DOWNLOAD_PATH = APP_PATH + "download/";
    DOMAIN_PATH = "http://writerfly.cn";
    SERVER_PATH = DOMAIN_PATH + "/sync3/";
    // SERVER_PATH = "http://127.0.0.1/writerfly/sync5/";
    HISTORY_PATH = DATA_PATH+"history/";

    ensureDirExist(DATA_PATH);
    ensureDirExist(NOVEL_PATH);
    ensureDirExist(HISTORY_PATH);

    checkApplicationUpdateFile();
}

void RuntimeInfo::initIntegral()
{

}

void RuntimeInfo::initEggs()
{
    is_line_guide = false;
    line_guide_case = LINE_GUIDE_NONE;
    guide_widget = nullptr;
}

void RuntimeInfo::startSplash()
{
    splash = new SplashScreen;
    QPixmap pixmap;
    if (isFileExist(IMAGE_PATH + "splash.png"))
        pixmap = QPixmap(IMAGE_PATH + "splash.png");
    else if (isFileExist(IMAGE_PATH + "splash.jpg"))
        pixmap = QPixmap(IMAGE_PATH + "splash.jpg");
    else
        pixmap = QPixmap(":/icons/appicon");
    // splash->setPixmap(pixmap);
    splash->setSplashPixmap(pixmap);
    splash->show();
}

void RuntimeInfo::setSplash(QString msg)
{
    if (splash != nullptr)
        splash->showMessage(msg, Qt::AlignCenter, Qt::blue);
}

void RuntimeInfo::finishSplash()
{
    /* if (splash != nullptr)
        delete splash;
    splash = nullptr; */
    splash->toClose();
}

/**
 * 趣味提示
 * @param w
 * @return 是否需要线条支持
 */
bool RuntimeInfo::promptCreateNovel(QWidget *w)
{
    static int NO_NOVEL_PROMPT_COUNT = 0;  // 提示创建书籍次数，3次则提示

    ++NO_NOVEL_PROMPT_COUNT;
    if (NO_NOVEL_PROMPT_COUNT == 3)
    {
        if (QTime::currentTime().hour() >= 0 && QTime::currentTime().hour()<=3)
            QMessageBox::information(w, QObject::tr("别乱点了"), QObject::tr("大晚上呢，早点睡吧，身体要紧~"));
        else
            QMessageBox::information(w, QObject::tr("没!有!作!品!"), QObject::tr("你这汉子，好不晓事！\n最后再说一遍，要先创建新书！"));
        line_guide_case = LINE_GUIDE_CREATE_BOOK;
        return true;
    }
    else if (NO_NOVEL_PROMPT_COUNT > 3) // 不理会他
        ;
    else
        QMessageBox::information(w, QObject::tr("没有作品"), QObject::tr("请点击上方的“创建新书”来建立一个作品"));
    return false;
}

void RuntimeInfo::setLineGuideWidget(QWidget *w)
{
    return ; // 此处有重绘 bug

    is_line_guide = true;
    guide_widget = w;
}

void RuntimeInfo::finishLineGuide()
{
    is_line_guide = false;
    line_guide_case = LINE_GUIDE_NONE;
    guide_widget = nullptr;
}

void RuntimeInfo::setMainWindow(QWidget* w)
{
    this->main_window = w;
}

QWidget* RuntimeInfo::getMainWindow()
{
    return main_window;
}

bool RuntimeInfo::isMainWindowMaxing()
{
    return maximum_window || full_screen;
}

void RuntimeInfo::popNotification(NotificationEntry* noti)
{
    emit signalPopNotification(noti);
}
