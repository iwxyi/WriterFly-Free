#include "syncthread.h"

SyncThread::SyncThread(RuntimeInfo* rt, USettings* us, QObject *parent) : QObject(parent), rt(rt), us(us),
    DP(rt->DATA_PATH), SP(rt->SERVER_PATH), account_state(ACCOUNT_NONE), sync_state(SYNC_NONE),
    prev_sync_finished_timestamp(0), last_download_pull_timestamp(0),latest_download_timstamp(0), syncing(false),
    main_window(nullptr)
{
    userID = username = password = nickname = "";
    rank = 0;

    version = rt->version_number; // 对应码字风云版本
    version_s = QString::number(version);
    _____ = isFileExist(":/documents/MOD") ? readTextFile(":/documents/MOD").toInt() : 1;
    ______ = isFileExist(":/documents/MOD2") ? readTextFile(":/documents/MOD2").toInt() : 2;
    _______ = isFileExist(":/documents/LOW") ? readTextFile(":/documents/LOW").toInt() : 3;

    restoreSequeue(SEQ_BOTH);

    sync_timer = new QTimer(this);
    sync_timer->setInterval(SYNC_MAX_INTERVAL);
    connect(sync_timer, &QTimer::timeout, [=]{
        if (upload_seq.size() || download_seq.size())
        {
            if (getTimestamp() - prev_sync_finished_timestamp > SYNC_MAX_INTERVAL)
            {
                SYNCDBG << "定时云同步";
                syncNext();
            }
        }
        // 定时下载
        if (getTimestamp() > last_download_pull_timestamp + download_interval)
        {
            SYNCDBG << "准备定时下载";
            downloadAll();
        }
    });

    download_interval = DOWNLOAD_PULL_DEFAULT_INTERVAL;
    sync_count_before_download_all = 0;
}

SyncThread::~SyncThread()
{
    //saveSequeue(SEQ_BOTH); // 会导致退出时出错？
}

void SyncThread::startNetwork()
{
    this->CPU_ID = rt->cpu_id;
    // 读取上次使用的账号
    if (!username.isEmpty() && !password.isEmpty())
        login(username, password);
}

NetUtil* SyncThread::login(QString username, QString password)
{
    if (CPU_ID.isEmpty() || CPU_ID == "0") return nullptr;
//    account_state = AccountState::WAITING; // 已经在初次读取的时候就设置了，现在不需要设置
    NetUtil* n = new NetUtil(SP+"account_login.php", QStringList{"version", version_s, "vericode", enVerify(), "imei", CPU_ID, "username", username, "password", password});
    connect(n, &NetUtil::finished, [=](QString s){
        if (deVerify(getXml(s, "verify")) && getXml(s,"state") == "OK") // 成功
        {
            // 账号信息
            this->username = username;
            this->password = password;
            userID = getXml(s, "userID");
            nickname = getXml(s, "nickname");
            rank = getXmlInt(s, "rank");
            us->setVal("account/nickname", nickname);

            // 账号数据
            int net_words = getXmlInt(s, "allwords");
            int net_times = getXmlInt(s, "alltimes");
            int net_useds = getXmlInt(s, "alluseds");
            int net_bonus = getXmlInt(s, "allbonus");
            emit signalUserDataLoaded(net_words, net_times, net_useds, net_bonus);

            // 重新登录，或者从其它账号登录，全部重新上传
            QString prev_userID = us->getStr("account/prev_userID");
            if (prev_userID.isEmpty() || prev_userID != userID)
            {
                us->setVal("sync/last_download_pull_timestamp", 0);
                us->setVal("sync/last_upload_added_timetamp", 0);
                us->setVal("sync/prev_full_upload_timestamp", 0);
                us->setVal("sync/lastest_downloaded_timstamp", 0);

                upload_seq.clear();
                download_seq.clear();
                upload_wating_item = "";
                saveSequeue(SEQ_BOTH);
            }
            us->setVal("account/prev_userID", userID); // 登录其他账号时，如果不一样，则全部清空 [sync] 设置
            if (nickname.isEmpty())
                nickname = username;
            save();
            loginFinished();
            return ;
        }
        if (getXml(s, "state") == "Old") // 版本过旧，无法使用，弹窗之后打开新版
        {
            QMessageBox::information(rt->getMainWindow(), "版本过旧", "版本过旧，无法登录，请手动更新版本");
            QDesktopServices::openUrl(QUrl("http://writerfly.cn/download"));
        }
        else if (!getXml(s, "ERROR").trimmed().isEmpty())
            QMessageBox::information(rt->getMainWindow(), "登录失败", getXml(s, "ERROR"));
        else if (!getXml(s, "REASON").trimmed().isEmpty())
            QMessageBox::information(rt->getMainWindow(), "登录失败", getXml(s, "REASON"));
        else if (s.trimmed().isEmpty())
        {
            int count = us->getInt("sync/login_fail_count", 0);
            if (count < 2) // 连续提示两次以上则不进行提醒
                QMessageBox::information(rt->getMainWindow(), "网络连接失败", "与服务器连接失败，暂时无法登录");
            us->setVal("sync/login_fail_count", ++count);
        }
        else
            QMessageBox::information(rt->getMainWindow(), "登录失败", "为保证云同步安全，请检查时间是否正确？\n错误：" + s);
        if (!username.isEmpty() && !password.isEmpty())
        {
            account_state = AccountState::ACCOUNT_OFFLINE;
            nickname = username;
        }
        else
            account_state = AccountState::ACCOUNT_NONE;
    });
    return n;
}

/**
 * 登录完成后要做的事情
 */
void SyncThread::loginFinished()
{
    account_state = AccountState::ACCOUNT_LOGIN;
    emit signalLoginFinished();

    if (!us->getBool("sync/login_auto_sync", true))
        return ;

    // 获取上次全部下载后的列表（包括上次打开程序更改的，根据时间戳来判断）
    downloadAll();

    // 登录后开始上传上次关闭后的文件（人为修改）
    uploadAll();

    sync_timer->start();

    rt->account_dont_match_device = 0; // （积分上传失败）账号密码不匹配的提示次数
}

void SyncThread::logout()
{
    account_state = AccountState::ACCOUNT_NONE;
    userID = "";
    // username = ""; // 保留用户名
    // nickname = ""; // 保留昵称
    password = "";
    saveAccount();

    if (download_seq.size())
        us->setVal("sync/last_download_pull_timestamp", rt->opened_timestamp); // 下载下载使用 上次全部下载的时间之后
    if (upload_seq.size())
        us->setVal("sync/last_upload_added_timetamp", 0); // 下次上传使用 上次全部上传的时间之后

    emit signalLogout();
}

QString SyncThread::getUsername()
{
    return username;
}

QString SyncThread::getPassword()
{
    return password;
}

QString SyncThread::getNickname()
{
    return nickname;
}

QString SyncThread::getUserID()
{
    return userID;
}

int SyncThread::getRank()
{
    return rank;
}


void SyncThread::downloadAll()
{
    if (sync_state != SYNC_NONE) return ;
    sync_state = SYNC_SYNCING;
    latest_download_timstamp = last_download_pull_timestamp = us->getLongLong("sync/last_download_pull_timestamp", 0);
    connect(new NetUtil(SP+"download_sync.php", getIdentity(QStringList{"devicetime", toStr(last_download_pull_timestamp/1000)})), &NetUtil::finished, [=](QString s){
        SYNCDBG << "下载的传回文本：" << s;
        // 注意：服务器传回来是小写的！转换为大写的
        s = s.replace("<sync>", "<SYNC>");
        s = s.replace("</sync>", "</SYNC>");
        QStringList list = s.split("\n", QString::SkipEmptyParts);
        if (download_seq.isEmpty())
            download_seq += list;
        else
        {
            foreach (QString line, list)
            {
                if (!download_seq.contains(line))
                    download_seq.append(line);
            }
        }
        saveSequeue(SEQ_DOWNLOAD);
        us->setVal("sync/last_download_pull_timestamp", last_download_pull_timestamp = getTimestamp());
        latest_download_timstamp = getTimestamp();

        sync_state = SYNC_NONE;
        SYNCDBG << "待下载章节获取完成，开始下载";
        if (us->auto_sync)
        {
            sync_count_before_download_all = finished_seq.size();
            syncNext();
        }

        SYNCDBG << "当前下载章节序列数量：" << download_seq.size() << list.size();
    });
}

void SyncThread::uploadAll()
{
    // 退出程序后修改的文件
    qint64 timestamp = us->getLongLong("sync/last_upload_added_timetamp", 0); // 上一次添加同步文件的时间（重新登录后，添加全部上传、单文件上传的进入等待序列）
    if (timestamp <= 0)
        timestamp = us->getLongLong("sync/prev_full_upload_timestamp", 0);

    // 遍历每部作品
    QDir novels_dir(rt->NOVEL_PATH);
    QFileInfoList novels_info = novels_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QFileInfo novel_info, novels_info)
    {
        QString novel_path = novel_info.filePath() + "/"; // 目录所在文件夹
        QString novel_name = novel_info.baseName();

        QFileInfo dir_file(novel_path + NOVELDIRFILENAME);
        QDir chapters_dir(novel_path + "chapters");
        if (!dir_file.exists() || !chapters_dir.exists()) // 目录文件不存在，不是小说目录，也可能损坏了
        {
            SYNCDBG << "添加上传文件失败：" << novel_name;
            continue;
        }

        if (dir_file.lastModified().toMSecsSinceEpoch() > timestamp)
        {
            addDir(novel_name);
        }

        QFileInfoList chapters_info = chapters_dir.entryInfoList(QDir::Files);
        foreach (QFileInfo chapter_info, chapters_info)
        {
            if (chapter_info.lastModified().toMSecsSinceEpoch() > timestamp && chapter_info.suffix()=="txt")
                addChapter(novel_name, chapter_info.baseName());
        }
    }

    saveSequeue(SEQ_UPLOAD);

    us->setVal("sync/prev_full_upload_timestamp", getTimestamp());
    us->setVal("sync/last_upload_added_timetamp", getTimestamp());

    if (sync_state == SYNC_NONE)
        syncNext();

    SYNCDBG << "当前上传章节序列数量：" << upload_seq.size();
}

void SyncThread::addDir(QString novel_name)
{
    if (account_state == ACCOUNT_NONE) return ;
    if (novel_name.isEmpty()) return ;
    SYNCDBG << " >> 添加目录：" << novel_name;
    addUploadSequeue( tag("目录") + tag(novel_name) );
    syncNext();
}

void SyncThread::addChapter(QString novel_name, QString chapter_name)
{
    if (account_state == ACCOUNT_NONE) return ;
    if (novel_name.isEmpty() || chapter_name.isEmpty()) return;
    chapter_name = fnEncode(chapter_name);
    SYNCDBG << " >> 添加章节：" << novel_name << chapter_name;
    QString sync_str = tag("章节") + tag(novel_name) + tag(chapter_name);

    if (upload_wating_item == sync_str) // 已经在等待位中了
    {
        SYNCDBG << "（章节正在上传队列等待位）";
    }
    else if (sync_state == SYNC_UPLOADING && upload_seq.size() > 0 && upload_seq.at(0) == sync_str)
    {
        if (!upload_wating_item.isEmpty())
            addUploadSequeue( upload_wating_item ); // 之前的等待项入队列

        // 这一项正在上传，等本次上传结束后，再加入队列，后面进行上传
        upload_wating_item = sync_str;
        SYNCDBG << "（章节进入上传队列等待位）";
    }
    else
    {
        addUploadSequeue( sync_str );
        if (getTimestamp() - prev_sync_finished_timestamp >= SYNC_MAX_INTERVAL)
            syncNext();
    }

}

void SyncThread::addChapterDeleted(QString novel_name, QString chapter_name)
{
    if (account_state == ACCOUNT_NONE) return ;
    if (novel_name.isEmpty() || chapter_name.isEmpty()) return ;
    chapter_name = fnEncode(chapter_name);
    SYNCDBG << " >> 删除章节：" << novel_name << chapter_name;
    addUploadSequeue( tag("删章") + tag(novel_name) + tag(chapter_name) );
    syncNext();
}

void SyncThread::addNovelDeleted(QString novel_name)
{
    if (account_state == ACCOUNT_NONE) return ;
    if (novel_name.isEmpty()) return;
    SYNCDBG << " >> 删除作品：" << novel_name;
    addUploadSequeue( tag("删书") + tag(novel_name) );
    syncNext();
}

bool SyncThread::syncNext(bool force)
{
    if (account_state != ACCOUNT_LOGIN) return false;
    if (sync_state != SYNC_NONE && !force) return false;
    if (sync_count_before_download_all != 0 && download_seq.size() == 0) // 下载完毕
    {
        if (sync_count_before_download_all == finished_seq.size()) // 没有下载
        {
            download_interval *= 2;
            if (download_interval > DOWNLOAD_PULL_MAX_INTERVAL)
                download_interval = DOWNLOAD_PULL_MAX_INTERVAL;
        }
        else // 有下载，下载间隔减半
        {
            download_interval /= 2;
            if (download_interval < DOWNLOAD_PULL_MIN_INTERVAL)
                download_interval = DOWNLOAD_PULL_MIN_INTERVAL;
        }
        sync_count_before_download_all = 0;
        SYNCDBG << "下载完毕，下载间隔：" << download_interval / 1000;
    }
    if (download_seq.size() != 0) // 下载
    {
        SYNCDBG << "下载下一条，进度：" << download_seq.size() << upload_seq.size() << download_seq.first();
        sync_state = SYNC_DOWNLOADING;
        QTimer::singleShot(SYNC_NEXT_INTERVAL, [=]{
            downloadOne(download_seq.first());
        });
    }
    else if (upload_seq.size() != 0) // 上传
    {
        SYNCDBG << "上传下一条，进度：" << download_seq.size() << upload_seq.size() << upload_seq.first();
        sync_state = SYNC_UPLOADING;
        QTimer::singleShot(SYNC_NEXT_INTERVAL, [=]{
            uploadOne(upload_seq.first());
        });
    }
    else // 云同步结束啦
    {
        // 之前上传的章节正在等待队列中
        if (!upload_wating_item.isEmpty())
        {
            SYNCDBG << "从等待位恢复章节";
            addUploadSequeue(upload_wating_item);
            upload_wating_item = "";
            if (rt->is_exiting) // 退出前进行最后一章的同步，否则继续等待下次的同步
                return syncNext();
        }

        saveSequeue(SEQ_BOTH);
        sync_state = SYNC_NONE;
        prev_sync_finished_timestamp = getTimestamp();

        // 判断能否退出
        if (rt->is_exiting)
        {
            emit signalAllFinishedAndCanExit();
        }
    }
    return true;
}

void SyncThread::uploadOne(QString text)
{
    QStringList list = getXmls(text, "SYNC");
    if (list.size() == 0) // 是个莫名其妙的东西。忽略
    {
        upload_seq.removeFirst();
        uploadOneFinished(false);
        return ;
    }

    QString kind = list.at(0);
    int data_size = list.size();
    QString url = ""; // 上传的文件名
    QStringList post; // 上传的post数据
    if (kind == "目录")
    {
        if (data_size < 2) return uploadOneFinished(false);
        QFileInfo file(rt->NOVEL_PATH + list.at(1) + "/" + NOVELDIRFILENAME);
        if (!file.exists())
            return uploadOneFinished(false);
        url = SP + "novel_content.php";
        post = getIdentity() << "novelname" << list.at(1) << "altertime" << toStr(file.lastModified().toMSecsSinceEpoch()/1000) // 等同于toSecsSinceEpoch，用 /1000 的方法更加显眼
                << "content" << directory_WriterFly2MZFY(readTextFile(rt->NOVEL_PATH + list.at(1) + "/" + NOVELDIRFILENAME));
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            SYNCDBG << "上传目录完成：" << text << s.length() << upload_seq.size()-1;
            if (getXml(s, "state") == "OK")
                return uploadOneFinished(true);
            processUploadError(s);
        });
    }
    else if (kind == "章节")
    {
        if (data_size < 3) return uploadOneFinished(false);
        QFileInfo file(rt->NOVEL_PATH + list.at(1) + "/chapters/" + list.at(2) + ".txt");
        if (!file.exists())
            return uploadOneFinished(false);
        url = SP + "novel_chapter.php";
        post = getIdentity() << "novelname" << list.at(1) << "altertime" << toStr(file.lastModified().toMSecsSinceEpoch()/1000)
             << "kind" << "0" << "title" << list.at(2)
             << "body" << readTextFile(rt->NOVEL_PATH + list.at(1) + "/chapters/" + list.at(2) + ".txt");
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            SYNCDBG << "上传章节完成：" << text << s.length() << upload_seq.size()-1;
            if (getXml(s, "state") == "OK")
                return uploadOneFinished(true);
            processUploadError(s);
        });
    }
    else if (kind == "删书")
    {
        if (data_size < 2) return uploadOneFinished(false);
        url = SP + "novel_delete.php";
        post = getIdentity() << "novelname" << list.at(1);
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            SYNCDBG << "删除作品完成：" << text << s.length();
            if (getXml(s, "state") == "OK" || !s.isEmpty()/*非空就表示删除了*/)
                return uploadOneFinished(true);
            processUploadError(s);
        });
    }
    else if (kind == "删章")
    {
        if (data_size < 3) return uploadOneFinished(false);
        url = SP + "novel_chapter_delete.php";
        post = getIdentity() << "novelname" << list.at(1) << "title" << list.at(2) << "kind" << "0";
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            SYNCDBG << "删除章节完成：" << text << s.length();
            if (getXml(s, "state") == "OK")
                return uploadOneFinished(true);
            processUploadError(s);
        });
    }
    else
    {
        uploadOneFinished(false);
    }
}

void SyncThread::processUploadError(QString s)
{
    if (s.isEmpty()) // 网络欠佳
        return pauseSync();
    else if (!getXml(s,"ERROR").isEmpty()) // 有错误
        QMessageBox::information(rt->getMainWindow(), "上传错误", getXml(s,"ERROR"));
    else
        QMessageBox::information(rt->getMainWindow(), "上传错误", "未知返回错误："+s);
    pauseSync();
}

void SyncThread::downloadOne(QString text)
{
    QStringList list = getXmls(text, "SYNC"); // 注意，服务器传回来的是小写的！先转换成大写的
    if (list.size() == 0) // 是个莫名其妙的东西。忽略
        return downloadOneFinished(false);

    QString kind = list.at(0);
    int data_size = list.size();
    QString url = ""; // 上传的文件名
    QStringList post; // 上传的post数据
    if (kind == "目录")
    {
        if (data_size < 3) return downloadOneFinished(false);
        qint64 timestamp = list.at(2).toLongLong()*1000;
        QFileInfo file(rt->NOVEL_PATH + list.at(1) + "/" + NOVELDIRFILENAME);
        if (file.exists() && file.lastModified().toMSecsSinceEpoch() >= timestamp) // 本地的时间更晚，或者是同一个
            return downloadOneFinished(false);
        url = SP + "download_content.php";
        post = getIdentity() << "novelname" << list.at(1);
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            if (!s.isEmpty())
            {
                // 如果作品并不存在，则新建（只需要新建文件夹，然后，目录会自动创建）
                if (!isFileExist(rt->NOVEL_PATH + list.at(1)))
                {
                    ensureDirExist(rt->NOVEL_PATH + list.at(1));
                    ensureDirExist(rt->NOVEL_PATH + list.at(1) + "/chapters");
                    QTimer::singleShot(10, [=]{
                        emit signalNovelAdded(list.at(1));
                    });
                }

                QString path = rt->NOVEL_PATH + list.at(1) + "/" + NOVELDIRFILENAME;
                s = urlDecode(s.replace("+", "%2B"));
                s = directory_MZFY2WriterFly(s);
                writeTextFile(path, s);
                emit signalDirectoryUpdated(list.at(1));

                SYNCDBG << " >> 下载目录完成：" << text << s.length() << download_seq.size()-1;
                downloadOneFinished(true);
            }
            else // 目录不可能是空的，所以要暂停
            {
                SYNCDBG << "下载目录失败，居然是空的";
                if (QMessageBox::information(rt->getMainWindow(), "下载失败", "作品《" + list.at(1) + "》是空的，暂停云同步（暂时离线）还是略过此项？", "暂停", "略过", 0, 1) == 0)
                    pauseSync();
                else
                    downloadOneFinished(false);
            }
        });
    }
    else if (kind == "章节")
    {
        if (data_size < 4) return downloadOneFinished(false);
        qint64 timestamp = list.at(3).toLongLong()*1000;
        QString path = rt->NOVEL_PATH + list.at(1) + "/chapters/" + list.at(2) + ".txt";
        QFileInfo file(path);
        if (file.exists() && file.lastModified().toMSecsSinceEpoch() >= timestamp) // 本地的时间更晚，或者是同一个
            return downloadOneFinished(false);
        url = SP + "download_chapter.php";
        post = getIdentity() << "novelname" << list.at(1) << "title" << list.at(2) << "kind" << "0";
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            // 如果作品或者章节文件夹不存在，则创建
            ensureDirExist(rt->NOVEL_PATH + list.at(1) + "/chapters");

            if (!s.isEmpty())
            {
                s = urlDecode(s.replace("+", "%20"));
                writeTextFile(path, s);
                emit signalChapterUpdated(list.at(1), list.at(2));
            }
            SYNCDBG << " >> 下载章节完成：" << text << s.length() << download_seq.size()-1;
            downloadOneFinished(true);
        });
    }
    else if (kind == "大纲目录")
    {
        if (data_size < 3) return downloadOneFinished(false);
        qint64 timestamp = list.at(2).toLongLong()*1000;
        QString path = rt->NOVEL_PATH + list.at(1) + "/" + OUTLINE_DIR_FILENAME;
        QFileInfo file(path);
        if (file.exists() && file.lastModified().toMSecsSinceEpoch() >= timestamp) // 本地的时间更晚，或者是同一个
            return downloadOneFinished(false);
        url = SP + "download_outline_content.php";
        post = getIdentity() << "novelname" << list.at(1);
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            if (!s.isEmpty())
            {
                // 如果作品并不存在，则新建（只需要新建文件夹，然后，目录会自动创建）
                if (!isFileExist(rt->NOVEL_PATH + list.at(1)))
                {
                    ensureDirExist(rt->NOVEL_PATH + list.at(1));
                    ensureDirExist(rt->NOVEL_PATH + list.at(1) + "/chapters");
                    QTimer::singleShot(10, [=]{
                        emit signalNovelAdded(list.at(1));
                    });
                }

                s = urlDecode(s.replace("+", "%20"));
                writeTextFile(path, s);
                emit signalOutlineDirectoryUpdated(list.at(1));

                SYNCDBG << " >> 下载故事线目录完成：" << text << s.length() << download_seq.size()-1;
                downloadOneFinished(true);
            }
            else // 目录不可能是空的，所以要暂停
            {
                SYNCDBG << "下载故事线目录失败，居然是空的";
                downloadOneFinished(false);
            }
        });
    }
    else if (kind == "大纲")
    {
        if (data_size < 4) return downloadOneFinished(false);
        qint64 timestamp = list.at(3).toLongLong()*1000;
        QString outline_name = list.at(2);
        { // 通过目录修改大纲名字
            bool ok = false;
            int id = outline_name.toInt(&ok);
            if (ok)
            {
                QString outline_directory = readTextFile(rt->NOVEL_PATH + list.at(1) + "/" + OUTLINE_DIR_FILENAME);
                QStringList outlines = outline_directory.split("\n", QString::SkipEmptyParts);
                if (outlines.length() > id+1)
                {
                    QStringList ss = outlines.at(id+1).split(" ", QString::SkipEmptyParts);
                    if (ss.length() >= 2)
                    {
                        outline_name = ss.first();
                    }
                }
            }
        }

        QString path = rt->NOVEL_PATH + list.at(1) + "/outlines/" + outline_name + ".txt";
        QFileInfo file(path);
        if (file.exists() && file.lastModified().toMSecsSinceEpoch() >= timestamp) // 本地的时间更晚，或者是同一个
            return downloadOneFinished(false);
        url = SP + "download_chapter.php";
        post = getIdentity() << "novelname" << list.at(1) << "title" << list.at(2) << "kind" << "1";
        connect(new NetUtil(url, post), &NetUtil::finished, [=](QString s){
            // 如果作品或者章节文件夹不存在，则创建
            ensureDirExist(rt->NOVEL_PATH + list.at(1) + "/outlines");

            if (!s.isEmpty())
            {
                s = urlDecode(s.replace("+", "%20"));
                writeTextFile(path, s);
                emit signalOutlineUpdated(list.at(1), outline_name);
            }
            SYNCDBG << " >> 下载故事线完成：" << text << s.length() << download_seq.size()-1;
            downloadOneFinished(true);
        });
    }
    else
    {
        downloadOneFinished(false);
    }
}

void SyncThread::uploadOneFinished(bool success)
{
    if (upload_seq.size() > 0)
    {
        if (success)
            finished_seq.insert(0, upload_seq.first() + "<SYNC>" + QString::number(getTimestamp())+"</SYNC>");
        upload_seq.removeFirst();
    }
    saveSequeue(SEQ_UPLOAD);

    sync_state = SYNC_NONE;
    syncNext();

    emit signalSyncChanged(download_seq.size(), upload_seq.size());
}

void SyncThread::downloadOneFinished(bool success)
{
    if (download_seq.size() > 0)
    {
        if (success)
            finished_seq.insert(0, download_seq.first() + "<SYNC>"+QString::number(getTimestamp())+"</SYNC>");
        download_seq.removeFirst();
    }
    saveSequeue(SEQ_DOWNLOAD);
    emit signalSyncChanged(download_seq.size(), upload_seq.size());

    sync_state = SYNC_NONE;
    syncNext();

    latest_download_timstamp = getTimestamp();
    us->setVal("sync/lastest_downloaded_timstamp", latest_download_timstamp);
}

QStringList SyncThread::getIdentity(QStringList list)
{
    QStringList post;
    post << "version" << version_s << "imei" << CPU_ID << "vericode" << enVerify() << "userID" << userID << "password" << password;
    if (list.size() > 0)
        post += list;
    return post;
}

int SyncThread::getSyncLast()
{
    return upload_seq.size() + download_seq.size();
}

void SyncThread::restoreSequeue(SeqSelector ss)
{
    if (ss & SEQ_UPLOAD)
        upload_seq = readTextFileIfExist(DP+"sync_upload.xml").split("\n", QString::SkipEmptyParts);
    if (ss & SEQ_DOWNLOAD)
        download_seq = readTextFileIfExist(DP+"sync_download.xml").split("\n", QString::SkipEmptyParts);
}

void SyncThread::saveSequeue(SeqSelector ss)
{
    if (ss & SEQ_UPLOAD)
    {
        writeTextFile(DP+"sync_upload.xml", upload_seq.join("\n"));
        us->setVal("sync/last_upload_added_timetamp", getTimestamp()); // 最近一次添加上传文件的时间
    }
    if (ss & SEQ_DOWNLOAD)
        writeTextFile(DP+"sync_download.xml", download_seq.join("\n"));
}

void SyncThread::addUploadSequeue(QString str)
{
    // 第一个可能正在上传，暂时不进行替换，大不了重复上传
    /*if ((upload_seq.size() > 0 && upload_seq.at(0) == str) // 正在上传
            || !upload_seq.contains(str)) // 没有在这里面
        upload_seq.append(str);*/

    int first = 0;
    if (sync_state != SYNC_NONE) // 第一个可能正在同步
        first++;
    for (int i = upload_seq.size()-1; i >= first;  --i)
    {
        if (upload_seq.at(i) == str) // 有重复的了
            return ;
    }
    upload_seq.append(str);
    saveSequeue(SEQ_UPLOAD);
}

void SyncThread::pauseSync()
{
    sync_state = SYNC_NONE;
}

QString SyncThread::directory_MZFY2WriterFly(QString content)
{
    if (!content.startsWith("<zopb")) // 不是码字风云的目录，直接返回
        return content;
    QString ans = "<BOOK>";
    QString book = getStrMid(content, "<zopb", "/zopb>");
    QStringList rolls = getStrMids(book, "<jrjr", "/jrjr>");
    foreach (QString roll, rolls)
    {
        ans += "<ROLL><RINF><n:" + getStrMid(roll, "<jrmk", "/jrmk>") + "></RINF><LIST>";
        QStringList chapters = getStrMids(roll, "<vhmk", "/vhmk>");
        foreach (QString chapter, chapters)
        {
            ans += "<CHPT><n:" + fnEncode(chapter) + "></CHPT>";
        }
        ans += "</LIST></ROLL>";
    }
    ans += "</BOOK>";
    return ans;
}

QString SyncThread::directory_WriterFly2MZFY(QString content)
{
    QString ans = "<zopb";
    QString book = getStrMid(content, "<BOOK>", "</BOOK>");
    QStringList rolls = getStrMids(book, "<ROLL>", "</ROLL>");
    foreach (QString roll, rolls)
    {
        ans += "<jrjr<jrmk" + getStrMid(getStrMid(roll, "<RINF>", "</RINF>"), "<n:", ">") + "/jrmk>";
        QStringList chapters = getStrMids(getStrMid(roll, "<LIST>", "</LIST>"), "<CHPT>", "</CHPT>");
        foreach (QString chapter, chapters)
        {
            ans += "<vhmk" + fnDecode(getStrMid(chapter, "<n:", ">")) + "/vhmk>";
        }
        ans += "/jrjr>";
    }
    ans += "/zopb>";
    return ans;
}

QString SyncThread::enVerify()
{
    qint64 __________ = getTimestamp();
    int ___________ = qrand() % 89999 + 10000;
    int ____________ = __________ / _________ % ______;
    int _____________ = __________ % _________;
    _____________ = _____________ * _____________ * version % _____ + _______;
    int ______________ = _____________ * _____________ % _____ * _____________ * version % _____ + _______; // 突然发现好像多了一步，啧啧
    ____________ = ____________ * (______________ >> BTYE_MOVE_COUNT);
    return QString("%1%2%3%4").arg(___________).arg(____________).arg(______________).arg(_____________);
}

bool SyncThread::deVerify(QString ________)
{
    int __________ = ________.length();
    if (__________ < 16) return false;
    int ___________ = ________.mid(5, __________-11).toInt();
    int ____________ = ________.mid(__________-6, 3).toInt();
    int _____________ = ________.mid(__________-3, 3).toInt();
    if (_____________ * _____________ % _____ * _____________ * version % _____ + _______ != ____________)
        return false;

    int ______________ = ___________ / (____________ >> BTYE_MOVE_COUNT);
    int _______________ = getTimestamp() / _________ % ______;
    bool ________________ = _______________ - ______________ < DELTA_MIN_SECOND && _______________ - ______________ > -DELTA_MIN_SECOND;
    return ________________;
}

/**
    <sync>"目录"</sync><sync>书名</sync>
    <sync>"章节"</sync><sync>书名</sync><sync>章名</sync>
    <sync>"新书"</sync><sync>书名</sync> ' 不是很必要
    <sync>"删书"</sync><sync>书名</sync>
    <sync>"书名"</sync><sync>书名</sync>
    <sync>"删章"</sync><sync>书名</sync><sync>章名</sync> ' 删卷为遍历删章
    <sync>"章名"</sync><sync>书名</sync><sync>章名</sync>

    <sync>"名片库"</sync><sync>名片名</sync>
    <sync>"名片"</sync><sync>名片名</sync><sync>章名</sync>
    <sync>"新名片"</sync><sync>名片名</sync> ' 不是很必要
    <sync>"删名片"</sync><sync>名片名</sync>
    <sync>"名片库名"</sync><sync>名片名</sync>
    <sync>"删注释"</sync><sync>名片名</sync><sync>注释名</sync>
    <sync>"注释名"</sync><sync>名片名</sync><sync>注释名</sync>

    <sync>"大纲目录"</sync><sync>书名</sync> ' 不是很必要
    <sync>"大纲"</sync><sync>书名</sync>

    <sync>"细纲"</sync><sync>书名</sync><sync>章名</sync> ' 删卷为遍历删章
    <sync>"删细纲"</sync><sync>书名</sync><sync>章名</sync>
 */
QString SyncThread::tag(QString str)
{
    return "<SYNC>" + str + "</SYNC>";
}

qint64 SyncThread::getTimestamp(int mode)
{
    if (mode == 1)
        return QDateTime::currentDateTime().toMSecsSinceEpoch();
    else
        return QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000;
}

