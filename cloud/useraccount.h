#ifndef USERACCOUNT_H
#define USERACCOUNT_H

#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include "aesutil.h"
#include "bxorcryptutil.h"
#include "syncthread.h"

#define SAVE_INTERVAL 60000             // 保存的间隔
#define MAX_INPUT_ONCE 16               // 一口气输入的最长字数
#define MAX_INPUT_SPEED_PER_MINUTE 200  // 每分钟最大速度
#define MAX_INPUT_INTERVAL 600000       // 十分钟没输入就不算输入
#define MAX_INPUT_FASTEST_TIME 600000   // 最快速度允许十分钟
#define MAX_SAVE_INTERVAL 30000         // 最大保存间隔
#define MAX_UPLOAD_INTERVAL 20000       // 最大上传间隔
#define MAX_SPEED_INTERVAL 300000       // 最大速度间隔（5分钟）
#define MAX_SYNC_COUNT_SERVICE 10       // 上传下载数量不超过10，则进行最后的云同步

#define UADBG if( 0 ) qDebug()

class UserAccount : public SyncThread
{
    Q_OBJECT
public:
    UserAccount(RuntimeInfo* rt, USettings* us);
    ~UserAccount();
    friend class DarkRoom;

    void initData();
    bool syncBeforeExit();

    bool isLogin();
    void addUserWords(const QString str);
    void addUserWords(int x = 1);
    QPixmap getProfile();

    int getUserWords();
    int getUserTimes();
    int getUserBonus();
    int getUserLevel();

    int getThisRunUserWords();
    int getThisRunUserTimes();
    int getCurrentSpeed();
    int getTodayWords();

private:
    void initDataFromFile(QString path);
    void updateLevel();
    void updateUserTimes();
    bool againstCheatingBeforeSave();

    bool uploadIntergral();

    int quick_sqrt(long X);

signals:
    void signalUserDataModified(int old_words, int old_times, int old_useds,  int old_bonus); // 在第三方影响的情况下进行改变（比如登录后使用云端数据）

public slots:
    void save();
    void saveAccount();

    void loadNetUserData(int net_words, int net_times, int net_useds,  int net_bonus);

private:
    // ==== 统计 ====
    int user_words;  // 用户字数
    int user_times;  // 用户时间(分钟)
    int user_bonus;  // 额外积分
    int user_level;  // 用户等级

    int start_user_times;         // 启动时的时长
    int start_user_words;         // 启动时的字数
    qint64 start_timestamp;       // 启动时的时间戳，与现在时间相减就是使用时间
    qint64 runtime_sum_timestamp; // 本次累计的码字时间和

    int prev_save_words;
    int prev_save_times;
    int prev_save_bonus;
    qint64 pre_saved_timestamp;   // 上次保存字数的时间
    qint64 fatest_latest_time;

    int prev_upload_words;
    int prev_upload_times;
    int prev_upload_bonus;
    qint64 prev_upload_timestamp;

    // 速度
    qint64 prev_speed_timestamp;
    int prev_speed_words;

    // 日期
    QString today_str;
    int start_today_words;
    int start_today_times;

    // ==== 运行 ====
    QString file_path;
    BXORCryptUtil cryption;
    QTimer* save_timer;
};

#endif // USERACCOUNT_H
