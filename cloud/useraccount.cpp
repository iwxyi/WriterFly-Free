#include "useraccount.h"

UserAccount::UserAccount(RuntimeInfo *rt, USettings *us) : SyncThread (rt, us),
    cryption( readTextFile(":/documents/BXOR").isEmpty()?"hhhhhhh~\nhhhhhh~~\nhhhh~~~\nhhh~~~~\nhh~~~~~\nh~~~~~~": readTextFile(":/documents/BXOR"))
{
    this->file_path = rt->DATA_PATH + "account";

    user_words = user_times = user_bonus = user_level = 0;
    prev_save_words = prev_save_times = prev_save_bonus = 0;
    fatest_latest_time = 0;
    profile_photo = QPixmap(":/icons/appicon");

    pre_saved_timestamp = start_timestamp = getTimestamp();
    runtime_sum_timestamp = 0;

    prev_upload_words = prev_upload_times = prev_upload_bonus = 0;
    prev_upload_timestamp = 0;

    start_today_words = start_today_times = 0;
    today_str = QDate::currentDate().toString("yyyy-MM-dd");

    // initDataFromFile(file_path); // 为了分析CPU_ID，留到之后再进行

    save_timer = new QTimer(this);
    save_timer->setInterval(SAVE_INTERVAL); // 一分钟B操一次
    connect(save_timer, SIGNAL(timeout()), this, SLOT(save()));
    save_timer->start();

    connect(this, SIGNAL(signalUserDataLoaded(int, int, int, int)), this, SLOT(loadNetUserData(int, int, int, int)));
}

UserAccount::~UserAccount()
{
    save();
}

bool UserAccount::syncBeforeExit()
{
    if (!isLogin()) return false;

    rt->is_exiting = true; // 完成后发送信号，关闭主窗口，结束整个程序
    bool upload1 = uploadIntergral(); // 上传积分
    bool upload2 = download_seq.size() + upload_seq.size() > 0
            && download_seq.size() + upload_seq.size() <= MAX_SYNC_COUNT_SERVICE
            && syncNext(); // 强制同步

    return upload1 || upload2;
}

void UserAccount::initData()
{
    initDataFromFile(file_path);
}

void UserAccount::initDataFromFile(QString path)
{
    QString text = readTextFileIfExist(path);
    text = cryption.decrypt(text);
    UADBG << "统计数据 读取内容：" << text;

    QString cpu_id = getXml(text, "CPUID");
    if (cpu_id != rt->cpu_id)
    {
        UADBG << "不是同一个设备，无法使用积分" << cpu_id << rt->cpu_id;
        return ;
    }

    // 用户信息
    userID = getXml(text, "USERID");
    username = getXml(text, "USERNAME");
    password = getXml(text, "PASSWORD");
    user_words = getXmlInt(text, "USERWORDS");
    user_times = getXmlInt(text, "USERTIMES");
    user_bonus = getXmlInt(text, "USERBONUS");
    updateLevel();

    if (!username.isEmpty() && !password.isEmpty())
    {
        account_state = AccountState::ACCOUNT_WAITING;
        nickname = us->getStr("account/nickname", "");
    }

    // 统计数据
    prev_upload_words = prev_save_words = user_words;
    prev_upload_times = prev_save_times = user_times;
    prev_upload_bonus = prev_save_bonus = user_bonus;

    start_user_words = user_words;
    start_user_times = user_times;

    prev_speed_timestamp = getTimestamp();
    prev_speed_words = user_words;

    // 每日字数
    QString prev_date_string = getXml(text, "TDDATE");
    int prev_today_words = getXmlInt(text, "TDWORDS");
    int prev_today_times = getXmlInt(text, "TDTIMES");

    if (prev_date_string.isEmpty()) // 第一次使用，没有前一天（或者文件出错）
    {
        start_today_words = start_today_times = 0;
        save();
    }
    else if (prev_date_string != today_str) // 到了第二天
    {
        // 保存昨天的
        QString add_str = QString("%1,%2,%3\n").arg(prev_today_words).arg(prev_today_times).arg(prev_date_string);
        QString content = readTextFileIfExist(rt->DATA_PATH + "daily_history.txt");
        content += add_str;
        writeTextFile(rt->DATA_PATH + "daily_history.txt", content);

        // 今日清空
        start_today_words = start_today_times = 0;
        save();
    }
    else // 是同一天
    {
        start_today_words = prev_today_words;
        start_today_times = prev_today_times;
    }
}

bool UserAccount::isLogin()
{
    return account_state == ACCOUNT_LOGIN;
}

void UserAccount::addUserWords(const QString str)
{
    addUserWords(str.length());

    if (getTimestamp() - pre_saved_timestamp >= MAX_SAVE_INTERVAL)
    {
        save();
    }
}

void UserAccount::addUserWords(int x)
{
    if (x <= 0 || x > MAX_INPUT_ONCE) return;
    user_words += x;
}

void UserAccount::updateLevel()
{
    user_level = quick_sqrt((user_words + user_times + user_bonus)/100);
}

QPixmap UserAccount::getProfile()
{
    return profile_photo;
}

int UserAccount::getUserWords()
{
    return user_words;
}

int UserAccount::getUserTimes()
{
    return user_times;
}

int UserAccount::getUserBonus()
{
    return user_bonus;
}

int UserAccount::getUserLevel()
{
    return user_level;
}

int UserAccount::getThisRunUserWords()
{
    return user_words - start_user_words;
}

int UserAccount::getThisRunUserTimes()
{
    return user_times - start_user_times;
}

int UserAccount::getCurrentSpeed()
{
    if (getThisRunUserTimes() == 0)
        return 0;
    return getThisRunUserWords() * 60 / getThisRunUserTimes(); // 每小时的速度
}

int UserAccount::getTodayWords()
{
    return start_today_words + user_words - start_user_words;
}

void UserAccount::save()
{
    updateUserTimes();

    if (user_words == prev_save_words && user_times == prev_save_times && user_bonus == prev_save_bonus)
        return ; // 一模一样的，不进行保存

    againstCheatingBeforeSave();

    if (file_path.isEmpty())
    {
        UADBG << "保存统计数据的配置文件路径不存在:" << file_path;
        return ;
    }

    saveAccount();

    updateLevel();

    if (getTimestamp() - prev_upload_timestamp >= MAX_UPLOAD_INTERVAL)
    {
        uploadIntergral();
    }
}

void UserAccount::saveAccount()
{
    QString text = makeXml(rt->cpu_id, "CPUID")
            +makeXml(userID, "USERID")
            +makeXml(username, "USERNAME")
            +makeXml(password, "PASSWORD")
            +makeXml(user_words, "USERWORDS")
            +makeXml(user_times, "USERTIMES")
            +makeXml(user_bonus, "USERBONUS")
            +makeXml(today_str, "TDDATE")
            +makeXml(start_today_words+user_words-start_user_words, "TDWORDS")
            +makeXml(start_today_times+user_times-start_user_times, "TDTIMES");
    UADBG << "统计数据 存储内容：" << text;
    text = cryption.encrypt(text);
    writeTextFile(file_path, text);
    pre_saved_timestamp = getTimestamp();

    prev_save_words = user_words;
    prev_save_times = user_times;
    prev_save_bonus = user_bonus;
}

void UserAccount::loadNetUserData(int net_words, int net_times, int net_useds,  int net_bonus)
{
    if (user_words + user_times/10 >= net_words + net_times/10) // 字数正确，不需要使用
        return ;
    UADBG << "使用网络数据" << net_words << net_times << net_useds << net_bonus;
    int old_words = user_words;
    int old_times = user_times;
    int old_useds = 0;
    int old_bonus = user_bonus;

    user_words = net_words;
    user_times = net_times;
    user_bonus = net_bonus;

    prev_upload_words = prev_save_words = user_words;
    prev_upload_times = prev_save_times = user_times;
    prev_upload_bonus = prev_save_bonus = user_bonus;

    start_user_words = user_words;
    start_user_times = user_times;

    prev_upload_timestamp = prev_speed_timestamp = getTimestamp();
    prev_speed_words = user_words;

    saveAccount();
    updateLevel();

    // 发出信号提示改变（小黑屋的delta变化）
    emit signalUserDataModified(old_words, old_times, old_useds, old_bonus);
}

void UserAccount::updateUserTimes()
{
    qint64 delta_times = getTimestamp() - pre_saved_timestamp;
    if (delta_times <= MAX_INPUT_INTERVAL) // 如果超过了码字间隔，则这中间的时间不算
    {
        runtime_sum_timestamp += delta_times;
        user_times = start_user_times + static_cast<int>(runtime_sum_timestamp / 60000);
    }
    // pre_saved_timestamp = getTimestamp(); // 放到保存结束再更新
}

bool UserAccount::againstCheatingBeforeSave()
{
    int delta_timestamp = getTimestamp() - pre_saved_timestamp;
    int delta_words = user_words - prev_save_words;
    int delta_times = user_times - prev_save_times;
    int delta_addin = user_bonus - prev_save_bonus;

    if (delta_words < 0) // 字数负值，不正常
    {
        if (delta_words < - MAX_INPUT_SPEED_PER_MINUTE)
            user_words = prev_save_words; // 字数设置为0
        else // if ( -MAX_SPEED <= delta_words < 0)
            user_words = prev_save_words;
    }
    else if (delta_words > MAX_INPUT_SPEED_PER_MINUTE * SAVE_INTERVAL) // 增加字数 > 保存间隔 * 最大速度，不可能正常
    {
        user_words = prev_save_words;
    }
    if (delta_times < 0) // 时间异常
        user_times = prev_save_times;
    else if (delta_times > MAX_INPUT_SPEED_PER_MINUTE * 2) // 时间过快异常
        user_times = prev_save_times * MAX_INPUT_SPEED_PER_MINUTE * 2;
    if (delta_addin < 0)
        delta_addin = prev_save_bonus;

    if (delta_timestamp < 0) // 时间怎么会是负的？
    {
        UADBG << "保存间隔有点问题：" << delta_timestamp;
        return false;
    }

    if (delta_timestamp == 0) // 同一时刻保存的
        return false;

    if (delta_timestamp > SAVE_INTERVAL * 2) // 保存间隔超过两倍标准保存间隔
    {
        UADBG << "和上次保存的间隔太久" << delta_timestamp;
        user_words = prev_save_words;
        return false;
    }

    int speed_minute = delta_words * 60000 / delta_timestamp; // 每分钟字数
    if (speed_minute <= MAX_INPUT_SPEED_PER_MINUTE) // 正常
    {
        if (fatest_latest_time > 0) // 减少异常时长
        {
            fatest_latest_time -= delta_timestamp;
            if (fatest_latest_time < 0)
                fatest_latest_time = 0;
        }
        return true;
    }
    else if (speed_minute <= MAX_INPUT_SPEED_PER_MINUTE * 2) // 可能只是偶尔速度快
    {
        fatest_latest_time += delta_timestamp; // 增加异常时长
        if (delta_timestamp > MAX_INPUT_FASTEST_TIME || delta_timestamp > MAX_INPUT_FASTEST_TIME) // 长时间速度快？作弊了作弊了
        {
            UADBG << "长期速度快，判定为作弊" << speed_minute << delta_timestamp << fatest_latest_time;
            user_words = prev_save_words; // 回到初始速度
        }
        else // 真的是偶尔速度快
        {
            user_words = prev_save_words + MAX_INPUT_SPEED_PER_MINUTE * delta_timestamp / 60000; // 设置为最快速度
        }
        UADBG << "速度太快（偶尔）" << speed_minute << " / " << fatest_latest_time;
        return true;
    }
    else // 速度真的太快了，很可能就是作弊的
    {
        UADBG << "速度太快" << speed_minute << delta_words << delta_timestamp;
        user_words = prev_save_words; // 回到初始速度
        return false;
    }
}

bool UserAccount::uploadIntergral()
{
    static bool is_uploading_integral = false;

    if (account_state != ACCOUNT_LOGIN || is_uploading_integral)
        return false;
    // 保存上传时的状态。否则上传之际可能数值会变，
    // 上次上传的状态保存的是上传结束后的，上传时的差值将会丢失
    int upload_words = user_words;
    int upload_times = user_times;
    int upload_bonus = user_bonus;

    // 获取两次上传之间的差值
    int words = upload_words - prev_upload_words;
    int times = upload_times - prev_upload_times;
    int bonus = upload_bonus - prev_upload_bonus;
    int useds = 0;

    if (words == 0 && times == 0 && bonus == 0 && useds == 0)
        return false;

    // 计算速度
    // 码字风云是按照从启动到现在的总体码字速度来统计
    // 写作天下使用最近几分钟的速度，更加精确
    int speed = 0;
    qint64 delta_timestamp = getTimestamp() - prev_speed_timestamp;
//    UADBG << "速度差值：" << delta_timestamp << "/" << MAX_SPEED_INTERVAL;
    if (delta_timestamp >= MAX_SPEED_INTERVAL) // 最快5分钟统计一次
    {
        speed = (upload_words-prev_speed_words) * 60 / (delta_timestamp/1000);
//        UADBG << "计算速度：" << speed << (upload_words-prev_speed_words)*60 << (delta_timestamp/1000);
        prev_speed_timestamp = getTimestamp();
        prev_speed_words = upload_words;
    }

    is_uploading_integral = true; // 避免重复上传
    QString url = SP + "account_val.php";
    QStringList post = getIdentity() << "allwords" << toStr(words) << "alltimes" << toStr(times) << "allbonus" << toStr(bonus) << "alluseds" << toStr(useds) << "speed" << toStr(speed);
    connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
        is_uploading_integral = false;

        if (getXml(s, "state") == "OK")
        {
            prev_upload_words = user_words;
            prev_upload_times = user_times;
            prev_upload_bonus = user_bonus;
//            UADBG << "上传积分成功：" << words << times << bonus << speed;
        }
        else if (s.isEmpty()) // 稍后连接
        {
            UADBG << "上传积分连接失败" << words << times << bonus << speed;
        }
        else // 数据出错，无法处理
        {
            UADBG << "上传积分失败" << s << words << times << bonus << speed;
            if (!(rt->account_dont_match_device++)) // 最多只提示一次
                QMessageBox::information(rt->getMainWindow(), "上传积分失败", s);
            if (s.contains("出错") || s.contains("不"))
            {
                prev_upload_words = user_words;
                prev_upload_times = user_times;
                prev_upload_bonus = user_bonus;
            }
        }
        prev_upload_timestamp = getTimestamp();

        // 判断是不是上传结束了
        if (rt->is_exiting && sync_state == SYNC_NONE)
        {
            emit signalAllFinishedAndCanExit();
        }
    });
    return true;
}

int UserAccount::quick_sqrt(long X)
{
    unsigned long M = static_cast<unsigned long>(X);
    unsigned int N, i;
    unsigned long tmp, ttp; // 结果、循环计数
    if (M == 0) // 被开方数，开方结果也为0
        return 0;
    N = 0;
    tmp = (M >> 30); // 获取最高位：B[m-1]
    M <<= 2;
    if (tmp > 1) // 最高位为1
    {
        N ++; // 结果当前位为1，否则为默认的0
        tmp -= N;
    }
    for (i = 15; i > 0; i--) // 求剩余的15位
    {
        N <<= 1; // 左移一位
        tmp <<= 2;
        tmp += (M >> 30); // 假设
        ttp = N;
        ttp = (ttp << 1) + 1;
        M <<= 2;
        if (tmp >= ttp) // 假设成立
        {
            tmp -= ttp;
            N ++;
        }
    }
    return static_cast<int>(N);
}

