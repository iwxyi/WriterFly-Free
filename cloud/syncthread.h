#ifndef SYNCTHREAD_H
#define SYNCTHREAD_H

#include <QObject>
#include <QDesktopServices>
#include "runtimeinfo.h"
#include "usettings.h"
#include "stringutil.h"
#include "fileutil.h"
#include "netutil.h"

#define _________ 1000
#define DELTA_MIN_SECOND 30
#define BTYE_MOVE_COUNT 2

#define SYNC_MAX_INTERVAL 60000
#define SYNC_NEXT_INTERVAL 50

#define DOWNLOAD_PULL_DEFAULT_INTERVAL 60000 // 下载的默认间隔：1分钟
#define DOWNLOAD_PULL_MIN_INTERVAL 10000     // 下载的最快间隔：10秒钟
#define DOWNLOAD_PULL_MAX_INTERVAL 600000    // 下载的最长间隔：10分钟

#define toStr(x) QString::number(x)
#define SYNCDBG if(0)qDebug()

class SyncThread : public QObject
{
    Q_OBJECT
    friend class LoginWindow;
    friend class MainMenuWidget;
    friend class SyncStateWindow;
public:
    explicit SyncThread(RuntimeInfo* rt, USettings* us, QObject *parent = nullptr);
    ~SyncThread();

    enum AccountState {
        ACCOUNT_NONE,
        ACCOUNT_LOGIN,
        ACCOUNT_WAITING, // 刚启动程序，等待登录或者正在登录（有记录账号但未登录）
        ACCOUNT_OFFLINE, // 登录失败，或应用程序离线
    };

    enum SyncState {
        SYNC_NONE, // 未进行同步
        SYNC_SYNCING, // 等待云同步（根据自身时间，获取网络下载）
        SYNC_DOWNLOADING, // 下载
        SYNC_UPLOADING,   // 上传
    };

    enum SeqSelector {
        SEQ_NONE = 0,
        SEQ_UPLOAD = 1,
        SEQ_DOWNLOAD = 2,
        SEQ_BOTH = 3,
    };

    // ==== 账号 ====
    void startNetwork();
    NetUtil* login(QString username, QString password);
    void logout();

    QString getUsername();
    QString getPassword();
    QString getNickname();
    QString getUserID();
    int getRank();

    // ==== 云同步 ====
    int getSyncLast(); // 获取上传进度（剩下多少个）

    void downloadAll();    // 下载全部内容

    void uploadAll();      // 上传全部内容
    void addDir(QString novel_name);        // 上传目录
    void addChapter(QString novel_name, QString chapter_name); // 上传章节
    void addChapterDeleted(QString novel_name, QString chapter_name);
    void addNovelDeleted(QString novel_name);

    bool syncNext(bool force = false);   // 云同步的时候，同步下一个

    QStringList getIdentity(QStringList list = QStringList());
    QString enVerify();
    bool deVerify(QString str);

signals:
    void signalLoginFinished();
    void signalUserDataLoaded(int net_words, int net_times, int net_useds,  int net_bonus);
    void signalLogout();
    void signalAllFinishedAndCanExit();

    void signalSyncChanged(int download, int upload);   // 云同步数量变化
    void signalSyncFinished();          // 云同步结束

    void signalNovelAdded(QString novel_name);                          // 添加了小说
    void signalDirectoryUpdated(QString novel_name);                        // 下载了新的目录，更新目录
    void signalChapterUpdated(QString novel_name, QString chapter_name);    // 下载了新的章节，更新正文
    void signalOutlineDirectoryUpdated(QString novel_name); // 大纲进度改变
    void signalOutlineUpdated(QString novel_name, QString outline_name); // 大纲内容改变

public slots:


protected:
    virtual void save() { } // 保存信息（总的）
    virtual void saveAccount() { } // 保存（账号信息）

private:
    void loginFinished();

    void restoreSequeue(SeqSelector ss = SEQ_BOTH);
    void saveSequeue(SeqSelector ss = SEQ_BOTH);
    void addUploadSequeue(QString str);

    void uploadOne(QString text);    // 上传一项
    void downloadOne(QString text);  // 下载一项
    void processUploadError(QString s);
    void uploadOneFinished(bool success = true);
    void downloadOneFinished(bool success = true);
    void pauseSync(); // 因为网络原因，读取到空文本时，暂停云同步（等待下次同步）

    QString directory_MZFY2WriterFly(QString dir); // 码字风云目录转写作天下目录
    QString directory_WriterFly2MZFY(QString dir); // 写作天下目录转码字风云目录

public:
    QString tag(QString str);
    qint64 getTimestamp(int model = 1);

protected:
    // ==== 地址 ====
    RuntimeInfo* rt;
    USettings* us;
    QString DP; // data path 本地数据地址
    QString SP; // server path 服务器地址

    // ==== 秘钥 ====
    int _____, ______, _______;
    int version;
    QString version_s;

    // ==== 账号 ====
    AccountState account_state;
    QString CPU_ID;      // 机器码
    QString userID;      // 数字ID
    QString username;    // 账号
    QString password;    // 密码
    QString nickname;    // 昵称
    QPixmap profile_photo; // 头像
    int rank;
    int sync_times; // 云同步使用次数

    // ==== 云同步 ====
    SyncState sync_state;
    qint64 prev_sync_finished_timestamp; // 上一次同步时间
    qint64 last_download_pull_timestamp;
    qint64 latest_download_timstamp;
    QStringList download_seq;   // 下载序列（优先）
    QStringList upload_seq;     // 上传序列（存的章节名是编码后的文件名）
    QStringList finished_seq;   // 本次运行已完成的上传队列
    QString upload_wating_item; // 上传队列等待项（当前章节正在上传时，等待）
    bool syncing;
    QTimer* sync_timer;
    int download_interval;      // 本次下载的间隔
    int sync_count_before_download_all; // 下载前的已完成数量，和下载后进行对比，判断有没有真正从外部下载
    // ==== 数据 ====

    // ==== 其他 ====
    QWidget* main_window; // 用来弹出信息提示的主窗口
};

#endif // SYNCTHREAD_H
