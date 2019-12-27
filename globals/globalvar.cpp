#include <QString>
#include <QDebug>
#include <QApplication>
#include <QDateTime>
#include <QTime>
#include "settings.h"
#include "usettings.h"
#include "frisoutil.h"
#include "lexicon.h"
#include "runtimeinfo.h"
#include "apptheme.h"
#include "globaldata.h"
#include "useraccount.h"

bool DEBUG_MODE = true;
bool DEBUG_DEEP = false;

QString APPLICATION_NAME = "写作天下";
QString VERSION_CODE = "v1.5.5";
int     VERSION_NUMBER = 1052;

RuntimeInfo* rt;
USettings* us;
AppTheme* thm;
UserAccount* ac;
GlobalData* gd;

bool deb(QVariant str, QString name)
{
    qDebug() << "-------【" << name << "】-------";
    qDebug() << str.toString() << endl;
    return true;
}

bool deb(QVariant str)
{
    qDebug() << "------------------------";
    qDebug() << str.toString() << endl;
    return true;
}

QString log(QVariant str)
{
    static QString _log_text = "";
    static QString _log_path = "";
    if (DEBUG_MODE)
    {
        if (_log_path == "")
        {
            _log_path = rt->DATA_PATH + "runtime.log";
            if (isFileExist(_log_path))
            {
                _log_text = readTextFile(_log_path);
                if (_log_text.length() > 100000)
                    _log_text = _log_text.left(100000);
            }
            else
            {
                DEBUG_MODE = false; // 如果日志文件不存在，则强行关闭（避免忘记删除）
                return "";
            }
        }
        else
        {
            _log_text = readTextFile(_log_path);
            if (_log_text.length() > 100000)
                _log_text = _log_text.left(100000);
        }
        _log_text = str.toString() + QString("\t\t\t\t\t%1").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz MM/dd ddd")) + "\n" + _log_text;
        writeTextFile(_log_path, _log_text);
    }
    return str.toString();
}

QString log(QString title, QVariant str)
{
    log("=="+title+" : "+str.toString());
    return str.toString();
}

QString log2(QVariant str)
{
    if (DEBUG_DEEP)
    {
        return log(str);
    }
    return str.toString();
}

QString log2(QString title, QVariant str)
{
    if (DEBUG_DEEP)
    {
        return log(title, str);
    }
    return str.toString();
}

QString err(QString title, QVariant str)
{
    return log("!!!!" + title + "!!!!    " + str.toString());
}

QString Path(QString str)
{
    if (str.endsWith("/"))
        return rt->DATA_PATH + str;
    else
        return rt->DATA_PATH + str + "/";
}

void initGlobal()
{
    rt = new RuntimeInfo(VERSION_NUMBER);
    rt->setSplash("初始化用户设置");
    us = new USettings(rt->DATA_PATH+"settings.ini");
    thm = new AppTheme(rt, us);
    ac = new UserAccount(rt, us);
    gd = new GlobalData(rt, us, ac);

    log("==== 初始化快要完毕啦，设置全局 ====\n\n\n");
    thm->setUs(us); // 设置主题设置（颜色）

    // 设置全局颜色
    thm->updateGlobal();

    // 设置剪贴板
    if (us->clip_global)
        gd->boards.startConnectClipboard();
    gd->boards.setBoardCount(us->board_count);
    gd->boards.setLimits(us->clip_limit, us->recycle_limit);

    log("==== 初始化完毕，开始运行程序 ====\n\n\n");
}

void deleteGlobal()
{
    log("开始全局析构");
	delete us;
    delete thm;
    delete ac;
    delete gd;
    log("全局析构基本完成");
    rt->deleteLater();
}

/**
 * 毫秒级获取时间戳
 * @return 时间戳
 */
qint64 getTimestamp()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}
