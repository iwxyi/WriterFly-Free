#include "darkroom.h"

DarkRoom::DarkRoom(QString data_dir, UserAccount* ac) :
	file_path(data_dir+"darkroom_history.txt"), ac(ac),
    ct(ComboType_None), ft(ForceType_None), start_timstamp(0),
    delta_word(0), finish_word(0), delta_time(0), finish_time(0)
{
    // 用户数据变化
    connect(ac, &UserAccount::signalUserDataModified, [=](int old_words, int old_times, int old_useds, int old_bonus){
        if (ct == ComboType_None) return ;

        int dw = ac->user_words - old_words; // 改变的字数
        int dt = ac->user_times - old_times; // 改变的时间

        if (delta_word && ct != ComboType_Time && dw != 0)
        {
            ac->us->setVal("dr/finish_words", finish_word);
            finish_word += dw;
        }

        if (delta_time && ct != ComboType_Word && dt != 0)
        {
            finish_word += dt;
        }
    });
}

bool DarkRoom::isInDarkRoom()
{
    return ft != ForceType_None;
}

void DarkRoom::startDarkRoom(int word, int time, ComboType combo, ForceType force)
{
	if (isInDarkRoom()) return ;

	delta_word = word;
	delta_time = time;
	finish_word = ac->getUserWords() + delta_word;
	finish_time = ac->getUserTimes() + delta_time;
    ct = combo;
    ft = force;

	start_timstamp = ac->getTimestamp();

    if (word > 0 && time == 0)
        ct = ComboType_Word;
    else if (word == 0 && time > 0)
        ct = ComboType_Time;

	/*QString add_str = "";
    if (combo !=  ComboType_Time)
		add_str += makeXml(word, "WORD");
    if (combo != ComboType_Word)
		add_str += makeXml(time, "TIME");
    add_str += makeXml(combo, "COMBO");
    add_str += makeXml(force, "FORCE");*/
    QString add_str = QString("%1,%2,%3,%4,%5\n").arg(word).arg(time).arg(combo).arg(force).arg(ac->getTimestamp());

	QString content = readTextFileIfExist(file_path);
	content = content + add_str; // 插入到最后面
    // content += add_str; //makeXml(add_str, "DARK");
    writeTextFile(file_path, content);

    emit signalOpenDarkRoom();

    ac->us->setVal("dr/delta_words", delta_word);
    ac->us->setVal("dr/finish_words", finish_word);
    setAutoStart(true);
}

bool DarkRoom::exitDarkRoom(QWidget* w)
{
    if (ct == ComboType_None)
        return true;

    bool word_complete = false, time_complete = false;
    if (ct != ComboType_Time)
    {
        int delta = finish_word - ac->getUserWords();
        word_complete = delta <= 0;
    }

    if (ct != ComboType_Word)
    {
        int delta = finish_time - ac->getUserTimes();
        time_complete = delta <= 0;
    }

    if ( (ct == ComboType_Either && (word_complete || time_complete) )
    	|| (ct == ComboType_Both && word_complete && time_complete)
    	|| (ct == ComboType_Word && word_complete)
    	|| (ct == ComboType_Time && time_complete)
    	|| (warningExit(w))
    	)
    {
    	// 添加奖励
    	// ac->addAddin(delta_word + delta_time);

    	// 清零小黑屋
    	delta_word = 0;
    	delta_time = 0;
    	ft = ForceType_None;
    	ct = ComboType_None;

    	QTimer::singleShot( 10, [=]{
	        emit signalCloseDarkRoom();
    	});


	    ac->us->setVal("dr/delta_words", 0);
        ac->us->setVal("dr/finish_words", 0);
    	setAutoStart(false);

    	return true;
    }

    return false;
}

void DarkRoom::restoreDarkRoom(QWidget* w)
{
	int user_words = ac->getUserWords();
    int finish_words = ac->us->getInt("dr/finish_words", 0);
    if (user_words >= finish_words || ac->us->getInt("dr/force", ForceType_Warning) != ForceType_Force)
		return ;

    int full_delta_words = ac->us->getInt("dr/delta_words", 0);
    int rest_delta_words = finish_words - user_words;
    startDarkRoom(rest_delta_words, 0, ComboType_Word, ForceType_Force);

    QString tip = "";
    if (full_delta_words >= 80000 && rest_delta_words >= full_delta_words*7/10)
        tip = "人生可真是一段漫长的路途！\n若是走不下去了，请加入交流群获取逃离方法：705849222";
    else if (full_delta_words >= 50000 && rest_delta_words >= full_delta_words*9/10)
        tip = "让暴风雨来得更猛烈些吧！\n如若后悔，可加入交流群获取逃离方法：705849222";
    else if (full_delta_words >= 30000 && rest_delta_words >= full_delta_words*7/10) // 3万字以上，剩下七成
        tip = "认定了的路，再痛也不要皱一下眉头，再怎么难走都是你自己选的，你没有资格喊疼。\n若打算自暴自弃，请加入交流群获取逃离方法：705849222";
    else if (full_delta_words >= 10000 && rest_delta_words >= full_delta_words*2/5 && rest_delta_words <= full_delta_words/2)
        tip = "您已完成一半，再累也要坚持住！";
    else if (full_delta_words > 1000 && rest_delta_words >= full_delta_words*9/10)
        tip = "再进一次小黑屋的感觉怎样？\n[魔鬼的邪恶笑容]";
    else if (full_delta_words > 1000 && rest_delta_words <= full_delta_words / 100)
        tip = "最后几个字了，冲呀——！！！";
    else if (full_delta_words > 1000 && rest_delta_words <= full_delta_words / 10)
        tip = "快结束了，加油！";
    else if (full_delta_words < 10)
    {
        QString content = readTextFileIfExist(file_path);
        QStringList history = content.split("\n", QString::SkipEmptyParts);
        bool has_100 = false;
        int count = 0;
        foreach (QString his, history)
        {
            QStringList items = his.split(",", QString::SkipEmptyParts);
            if (items.size() >= 5)
            {
                if (items.at(0).size() > 2)
                {
                    has_100 = true;
                    break;
                }
                else
                    count++;
            }
        }
        if (!has_100 && count >= 2 && count <= 5)
            tip = "不用试了，小黑屋很稳定的~";
    }

    if (!tip.isEmpty())
        tip = "\n\n" + tip;
    QMessageBox::information(w, "小黑屋", QString("已恢复上次的小黑屋（仅字数）：\n进度：%1 / %2 字%3")
                             .arg(full_delta_words - rest_delta_words).arg(full_delta_words).arg(tip));
}

void DarkRoom::finishDarkRoomForce()
{
    if (delta_word && ct != ComboType_Time && finish_word > ac->getUserWords())
	{
        delta_word -= finish_word - ac->getUserWords();
		if (delta_word < 0) delta_word = 0;
	    finish_word = ac->getUserWords();
	}

    if (delta_time && ct != ComboType_Word && finish_time > ac->getUserTimes())
	{
        delta_time -= finish_time - ac->getUserTimes();
		if (delta_time < 0) delta_time = 0;
	    finish_time = ac->getUserTimes();
	}
}

bool DarkRoom::warningExit(QWidget* w)
{
	if (ft != ForceType_Warning)
    {
        EscapeDialog ed("小黑屋信息", getStateString(), "强制退出", "继续码字", w);
        if (ed.exec()) // 点到了退出按钮
        {
            // 不进行任何操作，只提示
            QMessageBox::information(w, "逃离小黑屋", "QQ交流群：705849222\n加入后咨询管理员");
        }
        return false;
    }
	return QMessageBox::information(w, "小黑屋警告", getStateString(), "强制退出", "返回", 0, 1) == 0;
}

QString DarkRoom::getRestString()
{
    if (ct == ComboType_None)
        return "未开启";

	QString s = "";
	bool word_complete = false, time_complete = false;
	if (ct != ComboType_Time)
	{
		int delta = finish_word - ac->getUserWords();
        if (delta > 0) // 未完成
            s = QString("差%1字").arg(delta);
		else
			word_complete = true;
	}
	else
		word_complete = true;

	if (ct != ComboType_Word)
	{
		int delta = finish_time - ac->getUserTimes();
        if (delta > 0) // 未完成
		{
			if (!s.isEmpty()) s += "，";
            s += QString("差%1分钟").arg(delta);
		}
		else
			time_complete = true;
	}
	else
		time_complete = true;

	if (word_complete && time_complete)
	{
		s = QString("已完成，点击退出");
	}
	return s;
}

QString DarkRoom::getStateString()
{
	if (ct == ComboType_None)
        return "未开启";

	QString word_str = "", time_str = "", combo_str = "", force_str = "";

	if (ct != ComboType_Time)
        word_str = QString("字数：%1 / %2").arg(ac->getUserWords()-finish_word+delta_word).arg(delta_word);
	else
		word_str = "字数：未开启";

	if (ct != ComboType_Word)
        time_str = QString("时间：%1 / %2").arg(ac->getUserTimes()-finish_time+delta_time).arg(delta_time);
	else
		time_str = "时间：未开启";

	if (ct == ComboType_Word)
		combo_str = "类型：字数";
	else if (ct == ComboType_Time)
		combo_str = "类型：时间";
	else if (ct == ComboType_Either)
		combo_str = "类型：字数 或 时间";
	else if (ct == ComboType_Both)
		combo_str = "类型：字数 与 时间";

	if (ft == ForceType_Warning)
		force_str = "强制：允许退出";
	else
		force_str = "强制：无法退出";

	QString s =word_str+"\n"+time_str+"\n"+combo_str+"\n"+force_str;

    return s;
}

QString DarkRoom::getHistories()
{
    QString histories = readTextFileIfExist(file_path);
    return histories;
}

void DarkRoom::setAutoStart(bool bAutoStart)
{
#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString strAppName = QFileInfo(strAppPath).baseName();

    reg.setValue(strAppName, bAutoStart ? strAppPath: "");
#endif

#ifdef Q_OS_LINUX
    //写.desktop文件, 到/etc/xdg/autostart目录下
#endif

#ifdef Q_OS_MACOS
    /*if (bAutoStart){
        LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
        CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:QStringToNSString(appPath)];
        LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemLast, NULL, NULL, url, NULL, NULL);
        CFRelease(item);
        CFRelease(loginItems);
    }else{
        UInt32 seedValue;
        CFURLRef thePath;
        LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
        CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);
        for (id item in (NSArray *)loginItemsArray) {
            LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
            if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr){
                if ([[(NSURL *)thePath path] hasPrefix:QStringToNSString(appPath)]){
                    LSSharedFileListItemRemove(loginItems, itemRef);
                }
                CFRelease(thePath);
            }
        }
        CFRelease(loginItemsArray);
        CFRelease(loginItems);
    }*/
#endif
}
