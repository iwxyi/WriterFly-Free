#include "noveldirmdata.h"

NovelDirMData::NovelDirMData()
{

}

NovelDirMData::NovelDirMData(QString novelName)
{
    readDir(novelName);
}

QString NovelDirMData::getFullText()
{
    return full_text;
}

QString NovelDirMData::getChapterName(int index)
{
    NovelDirItem item = cr_list[index];
    if (item.isRoll())
        return "";
    return item.getName();
}

QString NovelDirMData::getFullNameByName(QString name)
{
    for (int i = 0; i < cr_list.size(); i++)
        if (name == cr_list[i].getName())
            return cr_list[i].getFullChapterName();
    return name;
}

bool NovelDirMData::isExistChapter(int roll_index, int chpt_index)
{
    return (roll_index >= 0 && roll_index < roll_subs.size()
            && chpt_index >= 0 && chpt_index <= roll_subs[roll_index]);
}

bool NovelDirMData::isExistChapter(QString chapter_name)
{
    for (NovelDirItem item : cr_list)
        if (item.getName() == chapter_name)
            return true;
    return false;
}

bool NovelDirMData::isExistRoll(QString roll_name)
{
    return roll_names.contains(fnEncode(roll_name));
//    return full_text.indexOf("<RINF><n:"+fnEncode(roll_name)+">") > -1;
}

int NovelDirMData::getRcCount()
{
    return cr_list.size();
}

int NovelDirMData::getRcWordCount(int index) const
{
    NovelDirItem& item = cr_list[index];
    if (item.isRoll())
    {
        int count = 0;
        while (++index < cr_list.size())
        {
            if (cr_list[index].isRoll())
                break;
            count += getRcWordCount(index); // 递归
        }
        return count;
    }
    else
    {
        QString name = item.getName();
        name = fnEncode(name);
        QString path = rt->NOVEL_PATH+novel_name+"/chapters/" + name + ".txt";
        qint64 count_time = item.getWordCountTime();
        QFileInfo info(path);
        qint64 file_time = info.lastModified().toMSecsSinceEpoch();
        if (count_time == file_time)
            return item.getWordCount();
        QString text = readTextFile(path);
        int count = NovelTools::getArticleWordCount(text);
        item.setWordCountAndTime(count, file_time);
        return count;
    }
}

int NovelDirMData::getRcCountByRoll(int roll_index)
{
    if (roll_index < 0 || roll_index >= roll_subs.size())
        return 0;
    return roll_subs[roll_index];
}

int NovelDirMData::getListIndexByRoll(int roll_index)
{
    if (roll_index < 0)
        return 0;
    if (roll_index >= roll_names.size())
        return cr_list.size()-1;
    int list_index = 0;
    for (int i = 0; i < roll_index; i++)
        list_index += roll_subs[i]+1;
    return list_index;
}

void NovelDirMData::listDataChanged()
{
    ; // 被覆盖的函数，调用派生类（model）的resetModel();
}

void NovelDirMData::changeNameSuccessed(bool is_chapter, QString novel_name, QString old_name, QString new_name)
{
    Q_UNUSED(is_chapter);
    Q_UNUSED(novel_name);
    Q_UNUSED(old_name);
    Q_UNUSED(new_name);
    ; // 被覆盖的函数，调用派生类（model）的signalChangeName();
}

void NovelDirMData::deleteChapterSuccessed(QString novel_name, QString chapter_name)
{
    Q_UNUSED(novel_name);
    Q_UNUSED(chapter_name);
    ; // 被覆盖的函数，调用派生类（model）的signalDeleteChapter();
}

void NovelDirMData::readDir(QString novel_name)
{
    this->novel_name = novel_name;
    dir_path = rt->NOVEL_PATH+novel_name+"/"+NOVELDIRFILENAME;

    cr_list.clear();
    roll_names.clear();
    roll_subs.clear();

    if (novel_name.isEmpty())
    {
        listDataChanged();
        return ;
    }

    // 判断小说文件夹是否存在
    QString book_path = rt->NOVEL_PATH+novel_name;
    QDir novel_dir(book_path);
    if (!novel_dir.exists())
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("读取书籍失败"), QObject::tr("小说文件夹“%1”不存在").arg(book_path));
        return ;
    }
    // 判断小说文件是否存在
    QString dir_file_path = book_path+"/"+NOVELDIRFILENAME;
    QFile dir_file(dir_file_path);
    if (!dir_file.exists())
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("读取目录失败"), QObject::tr("目录文件“%1”不存在").arg(dir_file_path));
        return ;
    }

    // 读取目录文本
    QString dir_text = "", lineStr = "";
    dir_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream tsr(&dir_file);
        tsr.setCodec("UTF-8");
        while (!tsr.atEnd())
        {
            lineStr = tsr.readLine();
            dir_text += lineStr;
        }
    dir_file.close();

    // 这部分全局使用的函数
    full_text = dir_text;

    // 解析目录文本
    dir_text = getStrMid(dir_text, "<BOOK>", "</BOOK>");

    QStringList rolls = getStrMids(dir_text, "<ROLL>", "</ROLL>");

    int c_index = 1; // 总的章序
    for (int ro = 0; ro < rolls.length(); ro++)
    {
        QString roll_text = rolls.at(ro);

        // 获取目录信息信息（如果不存在则是全部）
        QString roll_info = getStrMid(roll_text, "<RINF>", "</RINF>");
        if (roll_info == "") roll_info = roll_text;

        // 目录名字
        QString roll_name = getStrMid(roll_info, "<n:", ">");
        roll_name = fnDecode(roll_name);
        roll_names.append(roll_name);

        // 是否折叠中
        bool hide = false;
        QString roll_state = getStrMid(roll_info, "<c:", ">");
        if (roll_state == "1") hide = true;

        // 添加卷列表
        NovelDirItem r_item(true, hide, roll_name, ro);
        r_item.setPos(ro, 0);
        cr_list.append(r_item);

        // 遍历卷中章节
        roll_text = getStrMid(roll_text, "<LIST>", "</LIST>");
        QStringList chapters = getStrMids(roll_text, "<CHPT>", "</CHPT>");

        for (int ch = 0; ch < chapters.length(); ch++)
        {
            QString chpt = chapters.at(ch);
            QString c_name = getStrMid(chpt, "<n:", ">");
            c_name = fnDecode(c_name);

            NovelDirItem c_item(false, hide, c_name, ro == 0 ? 0 : c_index++);
            c_item.setPos(ro, ch+1);
            cr_list.append(c_item);
        }

        roll_subs.append(chapters.length());
    }


    listDataChanged();
}

int NovelDirMData::addRoll(QString new_name)
{
    // 添加到目录列表
    NovelDirItem r_item(true, new_name, roll_names.length());
    r_item.setPos(roll_names.length(), 0);
    cr_list.append(r_item);

    // 添加到分卷数组
    new_name = fnEncode(new_name);
    roll_names.append(new_name);
    roll_subs.append(0);

    // 寻找书尾标记
    int pos = full_text.lastIndexOf("</BOOK>");
    if (pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("创建新卷失败"), QObject::tr("找不到书籍末尾标记"));
        return 0;
    }

    // 组合成新的卷目录
    QString new_text = QString("<ROLL><RINF><n:%1></RINF><LIST></LIST></ROLL>").arg(fnEncode(new_name));
    QString left_text = full_text.left(pos);
    QString right_text = full_text.right(full_text.length()-pos);
    full_text = QString("%1%2%3").arg(left_text).arg(new_text).arg(right_text);

    listDataChanged();
    saveDir();

    return cr_list.size()-1;
}

int NovelDirMData::addChapter(int list_index, QString new_name)
{
    // 获取所在分卷名字，以便于添加到文件
    NovelDirItem crt_item = cr_list.at(list_index);
    int roll_index = crt_item.getRollIndex();
    QString roll_name = roll_names.at(roll_index); // 这是编码后的名称

    // 判断章节列表序号
    int chpt_num = 0;
    if (roll_index > 0) // 作品相关无序号
    {
        chpt_num = 1;
        for (int i = 1; i <= roll_index; i++)
            chpt_num += roll_subs.at(i);
    }

    // 章节实例
    NovelDirItem c_item(false, new_name, chpt_num);
    c_item.setPos(roll_index, roll_subs.at(roll_index)+1);
    c_item.setNew();

    // 添加到model
    int add_pos = 0;
    if (roll_index == roll_names.length()-1) // 添加到最后（即最后一卷）
    {
        cr_list.append(c_item);
        add_pos = cr_list.size()-1;
    }
    else // 插入到中间（即不是最后一卷）
    {
        // 插入到列表
        if (roll_index == 0)
        {
            add_pos = roll_subs.at(0)+1;
        }
        else
        {
            add_pos = chpt_num+roll_subs.at(0)+roll_index;
        }
        cr_list.insert(add_pos, c_item);

        // 后面的章节序号+1
        // 2019.01.22 更新：已经转换到了 delegate 实时判断序号了
        for (int i = add_pos+1; i < cr_list.size(); i++)
        {
            NovelDirItem* item = &cr_list[i];
            if (!item->isRoll())
            {
                item->changeNumber(1);
            }
        }
    }
    roll_subs[roll_index]++;

    // 修改目录文件
    // 寻找卷尾标记
    int name_pos = full_text.indexOf(QString("<RINF><n:%1>").arg(roll_name));
    if (name_pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("创建新章失败"), QObject::tr("找不到分卷起始标记"));
        return 0;
    }
    int pos = full_text.indexOf("</LIST>", name_pos);
    if (pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("创建新章失败"), QObject::tr("找不到分卷结尾标记"));
        return 0;
    }

    // 组合成新的卷目录
    new_name = fnEncode(new_name);
    QString new_text = QString("<CHPT><n:%1></CHPT>").arg(new_name);
    QString left_text = full_text.left(pos);
    QString right_text = full_text.right(full_text.length()-pos);
    full_text = QString("%1%2%3").arg(left_text).arg(new_text).arg(right_text);

    listDataChanged();
    saveDir();

    return add_pos;
}

int NovelDirMData::getRollCount()
{
    return roll_names.size();
}

bool NovelDirMData::tryChangeName(int index, QString new_name)
{
    NovelDirItem* item = &cr_list[index];
    QString old_name = item->getName();
    if (new_name == "") return false;
    if (new_name == old_name) return true;

    QString fn_new_name = fnEncode(new_name);
    QString fn_old_name = fnEncode(old_name);

    if (item->isRoll())
    {
        QString flag_text = QString("<RINF><n:%1>").arg(fn_new_name);
        int pos = full_text.indexOf(flag_text);
        if (pos != -1)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改卷名失败"), QObject::tr("分卷【%1】已存在！").arg(fn_new_name));
            gotoReedit();
            return false;
        }

        flag_text = QString("<RINF><n:%1>").arg(fn_old_name);
        pos = full_text.indexOf(flag_text);
        if (pos == -1)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改卷名失败"), QObject::tr("未找到分卷【%1】标记！").arg(old_name));
            return false;
        }

        int start_pos = pos+6;
        int end_pos = full_text.indexOf(QString(">"), start_pos+3)+1;
        if (end_pos <= 0)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改卷名失败"), QObject::tr("未找到分卷名字结束标记！"));
            return false;
        }

        QString new_name_text = QString("<n:%1>").arg(fn_new_name);
        full_text = full_text.left(start_pos) + new_name_text + full_text.right(full_text.length()-end_pos);

        roll_names[item->getRollIndex()] = fn_new_name;

        item->setName(new_name);

        changeNameSuccessed(false, novel_name, old_name, new_name);

        listDataChanged();
        saveDir();
    }
    else // 修改章节名字，需要改变文件名以及正在编辑的章节名字
    {
        QString flag_text = QString("<CHPT><n:%1>").arg(fn_new_name);
        int pos = full_text.indexOf(flag_text);
        if (pos != -1)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改章名失败"), QObject::tr("章节【%1】已存在！").arg(fn_new_name));
            gotoReedit();
            return false;
        }

        flag_text = QString("<CHPT><n:%1>").arg(fn_old_name);
        pos = full_text.indexOf(flag_text);
        if (pos == -1)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改章名失败"), QObject::tr("未找到章节【%1】标记！").arg(old_name));
            return false;
        }

        int start_pos = pos+6;
        int end_pos = full_text.indexOf(QString(">"), start_pos)+1;
        if (end_pos <= 0)
        {
            QMessageBox::information(rt->getMainWindow(), QObject::tr("修改章名失败"), QObject::tr("未找到章节名字结束标记！"));
            return false;
        }

        QString new_name_text = QString("<n:%1>").arg(fn_new_name);
        full_text = full_text.left(start_pos) + new_name_text + full_text.right(full_text.length()-end_pos);

        item->setName(new_name);

        QString old_path = rt->NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fn_old_name + ".txt";
        QString new_path = rt->NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fn_new_name + ".txt";
        ensureFileNotExist(new_path);
        QFile::rename(old_path, new_path);

        changeNameSuccessed(true, novel_name, old_name, new_name);

        listDataChanged();
        saveDir();
    }

    return true;
}

int NovelDirMData::insertChapter(int index, QString new_name)
{
    if (cr_list[index].isRoll()) return 0;
    QString pre_name = cr_list[index].getName();
    int r_index = cr_list[index].getRollIndex();
    int c_index = cr_list[index].getChptIndex();
    NovelDirItem item(false, new_name, cr_list[index].getNumber());
    item.setPos(r_index, c_index);
    item.setNew(true);
    QString fn_name = fnEncode(new_name);
    if (full_text.indexOf(QString("<CHPT><n:%1>").arg(fn_name)) != -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("创建新章失败"), QObject::tr("名称重复"));
        return 0;
    }

    // 调整列表数据
    cr_list.insert(index, item);
    roll_subs[r_index]++;

    if (r_index > 0) // 非作品相关才改动后面的序号
    {
        for (int i = index+1; i < cr_list.size(); i++)
        {
            if (!cr_list[i].isRoll())
            {
                cr_list[i].changeNumber(1); // 改变章节序号
                if (cr_list[i].getRollIndex() == r_index)
                    cr_list[i].changePos(0, 1); // 改变同一卷的章节序号
            }
        }
    }

    // 添加到文件
    QString new_text = QString("<CHPT><n:%1></CHPT>").arg(fnEncode(new_name));
    QString pre_text = QString("<CHPT><n:%1>").arg(fnEncode(pre_name));
    int pos = full_text.indexOf(pre_text);
    if (pos == -1) return -1;
    pos = full_text.lastIndexOf("<CHPT>", pos);
    if (pos == -1) return -1;
    full_text = full_text.left(pos) + new_text + full_text.right(full_text.length()-pos);

    listDataChanged();
    saveDir();
    return index;
}

int NovelDirMData::addNextChapter(int index, QString new_name)
{
    if (index == cr_list.size()-1 || cr_list[index+1].isRoll()) // 分卷的最后一章
    {
        while (index && !cr_list[index].isRoll())
            index--; // 遍历移动到这一卷上
        return addChapter(index, new_name);
    }
    else // 等效于从下一个索引出插入
    {
        return insertChapter(++index, new_name);
    }
}

int NovelDirMData::insertRoll(int index, QString newName)
{
    if (!cr_list[index].isRoll()) return 0;
    QString pre_name = cr_list[index].getName();
    int r_index = cr_list[index].getRollIndex();
    NovelDirItem item(true, newName, r_index);
    item.setPos(r_index, 0);
    QString fn_name = fnEncode(newName);
    if (full_text.indexOf(QString("<RINF><n:%1>").arg(fn_name)) != -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("创建新卷失败"), QObject::tr("名称重复"));
        return 0;
    }

    // 调整列表数据
    cr_list.insert(index, item);
    roll_names.insert(r_index, newName);
    roll_subs.insert(r_index, 0);

    for (int i = index+1; i < cr_list.size(); i++)
    {
        cr_list[i].changePos(1, 0); // 改变卷的位置
        if (cr_list[i].isRoll())
            cr_list[i].changeNumber(1);
    }

    // 添加到文件
    QString new_text = QString("<ROLL><RINF><n:%1></RINF><LIST></LIST></ROLL>").arg(fnEncode(newName));
    QString pre_text = QString("<RINF><n:%1>").arg(fnEncode(pre_name));
    int pos = full_text.indexOf(pre_text);
    if (pos == -1) return -1;
    pos = full_text.lastIndexOf("<ROLL>", pos);
    if (pos == -1) return -1;
    full_text = full_text.left(pos) + new_text + full_text.right(full_text.length()-pos);

    listDataChanged();
    saveDir();
    return index;
}

QString NovelDirMData::deleteChapter(int list_index)
{
    // 获取章节信息（名字、卷序）
    NovelDirItem item = cr_list[list_index];
    if (item.isRoll()) {
        log("删除章节失败：这不是章节");
        return "";
    }
    QString c_name_return = item.getName();
    QString c_name = fnEncode(c_name_return);
    int rIndex = item.getRollIndex();

    // 从列表中删除
    cr_list.removeAt(list_index);
    roll_subs[rIndex]--;

    if (rIndex > 0) // 删除的是正文章节
    {
        int i = list_index;
        // 修改同卷后面的章节序号-1
        while (i < cr_list.size())
        {
            NovelDirItem * item = &cr_list[i];
            if (item->isRoll()) break;
            item->changeNumber(-1);
            item->changePos(0, -1);
            i++;
        }
        // 修改后面卷的章序-1
        while (i < cr_list.size())
        {
            NovelDirItem * item = &cr_list[i];
            if (!item->isRoll())
                item->changeNumber(-1);
            i++;
        }
    }
    else // 删除的是作品相关章节
    {
        int i = list_index;
        // 仅修改同卷后面的章节位置
        while (i < cr_list.size())
        {
            NovelDirItem * item = &cr_list[i];
            if (item->isRoll()) break;
            item->changePos(0, -1);
            i++;
        }
    }

    // 从目录中删除
    int left_pos = full_text.indexOf(QString("<CHPT><n:%1>").arg(c_name));
    if (left_pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("删除章节失败"), QObject::tr("找不到章节起始标记"));
        return "";
    }
    int right_pos = full_text.indexOf(QString("</CHPT>"), left_pos);
    if (right_pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("删除章节失败"), QObject::tr("找不到章节结尾标记"));
        return "";
    }
    right_pos += 7;
    full_text = full_text.left(left_pos)+full_text.right(full_text.length()-right_pos);

    deleteChapterSuccessed(novel_name, c_name);

    listDataChanged();
    saveDir();
    ac->addChapterDeleted(novel_name, c_name);

    return c_name_return;
}

QStringList NovelDirMData::deleteRoll(int list_index)
{
    QStringList deleted_chapters;

    // 获取分卷信息（名字、索引、章节数）
    NovelDirItem item = cr_list[list_index];
    if (!item.isRoll()) {
        log("删除分卷失败：这不是分卷");
        return deleted_chapters;
    }

    QString r_name = item.getName();
    int r_index = item.getRollIndex();
    int subs = roll_subs.at(r_index);


    // 确认删除
    if (subs > 0 && QMessageBox::question(rt->getMainWindow(), QObject::tr("是否确认删除分卷【%1】？").arg(r_name), QObject::tr("包含章节数量：%1\n目前没有回收站功能，删除后将无法找回！").arg(subs), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No )
        return deleted_chapters;

    r_name = fnEncode(r_name);

    // 从列表中删除
    for (int i = 0; i <= subs; i++)
    {
        QString name = getChapterName(list_index);
        deleted_chapters.append(name);
        deleteChapterSuccessed(novel_name, name);
        cr_list.removeAt(list_index);
    }
    roll_names.removeAt(r_index);
    roll_subs.removeAt(r_index);

    // 修改后面的卷序
    for (int i = list_index; i < cr_list.size(); i++)
    {
        NovelDirItem* item = &cr_list[i];
        item->changePos(-1, 0);
        if (item->isRoll())
            item->changeNumber(-1);
        else
            item->changeNumber(-subs);
    }

    // 从目录中删除
    int pos = full_text.indexOf(QString("<RINF><n:%1>").arg(r_name));
    if (pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("删除分卷失败"), QObject::tr("找不到分卷名字标记"));
        return deleted_chapters;
    }
    int left_pos = full_text.lastIndexOf(QString("<ROLL>"), pos);
    if (left_pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("删除分卷失败"), QObject::tr("找不到分卷起始标记"));
        return deleted_chapters;
    }
    int right_pos = full_text.indexOf(QString("</ROLL>"), left_pos);
    if (right_pos == -1)
    {
        QMessageBox::information(rt->getMainWindow(), QObject::tr("删除分卷失败"), QObject::tr("找不到分卷结尾标记"));
        return deleted_chapters;
    }
    right_pos += 7;
    full_text = full_text.left(left_pos)+full_text.right(full_text.length()-right_pos);

    listDataChanged();
    saveDir();

    return deleted_chapters;
}

/**
 * 添加多段章节（仅名字）（带内容的章节只能用遍历单章插入，因为可能会有重复的名字，后面会加上序号）
 */
void NovelDirMData::addMutiChapter()
{

}

/**
 * 添加仅用来分割的卷，而不影响章节的序号
 * @param index 添加卷的位置
 */
int NovelDirMData::addSplitRoll(int index, QString new_name)
{
    NovelDirItem item = cr_list[index];
    if (item.isRoll() || index <= 0) return -1;

    QString name = fnEncode(item.getName());
    QString pre_text = QString("<CHPT><n:%1>").arg(fnEncode(name));
    int pos = full_text.indexOf(pre_text);
    if (pos == 100) return -1;
    QString new_text = QString("</LIST></ROLL>\n<ROLL><RINF><n:%1></RINF><LIST>").arg(fnEncode(new_name));
    full_text = full_text.left(pos)+ new_text + full_text.right(full_text.length()-pos);

    saveDir();
    readDir(novel_name);
    listDataChanged();
    return index;
}

/**
 * 只取消卷，而不影响章节的位置
 * @param index 取消卷的位置
 */
bool NovelDirMData::cancelRoll(int index)
{
    NovelDirItem item = cr_list[index];
    if (!item.isRoll() || index <= 0) return false;

    QString name = fnEncode(item.getName());
    QString pre_text = QString("<RINF><n:%1>").arg(fnEncode(name));
    int pos = full_text.indexOf(pre_text);
    if (pos == -1) return false;
    int left_pos = full_text.lastIndexOf(QString("</LIST>"), pos); // 上一卷的list末尾
    int right_pos = full_text.indexOf(QString("<LIST>"), pos); // 当前卷的list开头
    if (left_pos == -1 || right_pos == -1) return false;
    right_pos += 6;
    full_text = full_text.left(left_pos) + full_text.right(full_text.length()-right_pos);

    saveDir();
    readDir(novel_name);
    listDataChanged();
    return true;
}

void NovelDirMData::saveDir()
{
    QFile dir_file(dir_path);
    dir_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream tsw(&dir_file);
        tsw.setCodec("UTF-8");
        tsw << full_text << endl;
    dir_file.close();
    ac->addDir(novel_name);
}

bool NovelDirMData::exchangeItem(int org, int aim)
{
    if (org == -1) return false; // 拖拽的是无效位置
    if (org == 0) return false;  // 作品相关卷不允许拖动
    if (org == aim) return true; // 没有拖动

    NovelDirItem *org_item = &cr_list[org];
    if (aim == -1) // 拖到底部
    {
        if (org_item->isRoll()) // 整卷移到底部
        {
            int org_roll_index = org_item->getRollIndex();
            int org_subs = roll_subs[org_roll_index];
            QString org_name = roll_names[org_roll_index];

            // 删除旧的数据
            QList<NovelDirItem> temp_list;
            for (int i = org; i <= org+org_subs; i++)
            {
                temp_list.append(cr_list[org]);
                cr_list.removeAt(org);
            }

            // 添加到卷末尾
            for (int i = 0; i < temp_list.size(); i++)
            {
                cr_list.append(temp_list[i]);
            }

            // 调整分卷数据
            roll_subs.removeAt(org_roll_index);
            roll_names.removeAt(org_roll_index);
            roll_subs.append(org_roll_index);
            roll_names.append(org_name);

             // 提取原来的章节列表
            int pos = full_text.indexOf(QString("<RINF><n:%1>").arg(org_name));
            if (pos == -1) return false;
            int left_pos = full_text.lastIndexOf(QString("<ROLL>"), pos);
            int right_pos = full_text.indexOf(QString("</ROLL>"), pos);
            if (left_pos == -1 || right_pos == -1) return false;
            QString move_roll = full_text.mid(left_pos, right_pos+7-left_pos);

            // 删除分卷内容
            full_text = full_text.left(left_pos) + full_text.right(full_text.length()-right_pos-7);

            // 插入到新的位置
            int insert_pos = full_text.indexOf(QString("</BOOK>"));
            if (insert_pos == -1) return false;
            full_text = full_text.left(insert_pos) + move_roll + full_text.right(full_text.length()-insert_pos);

            adjustAllIndex();
            listDataChanged();
            saveDir();

            return true;
        }
        else // 章节移到最后一章
        {
            aim = cr_list.size()-1;
        }
    }

    NovelDirItem* aim_item = &cr_list[aim];

    int org_roll_index = org_item->getRollIndex(); // 来源分卷
    int aim_roll_index = aim_item->getRollIndex(); // 目标分卷

    if (org_item->isRoll() && aim_item->isRoll())        // 卷 -> 卷 ： 【合并】
    {
        if (QMessageBox::information(rt->getMainWindow(), "合并卷", "是否合并两卷？\n目前该操作无法撤销，请谨慎。", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
            return false; // 取消继续
        int org_subs = roll_subs[org_roll_index], aimSubs = roll_subs[aim_roll_index];
        QString org_name = roll_names[org_roll_index], aimName = roll_names[aim_roll_index];
        bool isHide = aim_item->isHide();

        // 删除旧的数据
        QList<NovelDirItem> temp_list;
        for (int i = org; i <= org+org_subs; i++)
        {
            if (i > org) // 不包括卷名
                temp_list.append(cr_list[org]); // 备份到一个临时数组中
            cr_list.removeAt(org);
        }

        // 分析插入位置，并插入
        int insert_index = aim+aimSubs+1;
        if (org_roll_index < aim_roll_index)
            insert_index -= (org_subs+1);

        // 插入到卷末尾
        for (int i = 0; i < temp_list.size(); i++)
        {
            temp_list[i].setHide(isHide); // 展开或者收起
            temp_list[i].changePos(aim_roll_index-org_roll_index, 0); // 修改分卷
            cr_list.insert(insert_index+i, temp_list[i]);
        }

        // 调整分卷数据
        roll_subs[aim_roll_index] += org_subs;
        roll_names.removeAt(org_roll_index);
        roll_subs.removeAt(org_roll_index);

        // 提取原来的章节列表
        int pos = full_text.indexOf(QString("<RINF><n:%1>").arg(org_name));
        if (pos == -1) return false;
        int left_pos = full_text.indexOf(QString("<LIST>"), pos);
        int right_pos = full_text.indexOf(QString("</LIST>"), pos);
        if (left_pos == -1 || right_pos == -1) return false;
        QString move_chapters = full_text.mid(left_pos+6, right_pos-left_pos-6);

        // 删除分卷内容
        left_pos = full_text.lastIndexOf(QString("<ROLL>"), pos);
        right_pos = full_text.indexOf(QString("</ROLL>"), pos);
        if (left_pos == -1 || right_pos == -1) return false;
        full_text = full_text.left(left_pos) + full_text.right(full_text.length()-right_pos-7);

        // 插入到新的位置
        pos = full_text.indexOf(QString("<RINF><n:%1>").arg(aimName));
        if (pos == -1) return false;
        int insert_pos = full_text.indexOf(QString("</LIST>"), pos);
        if (insert_pos == -1) return false;
        full_text = full_text.left(insert_pos) + move_chapters + full_text.right(full_text.length()-insert_pos);
    }
    else if (!org_item->isRoll() && aim_item->isRoll())  // 章 -> 卷 ： 【移动】
    {
        int aim_roll_index = aim_item->getRollIndex();
        int aim_subs = roll_subs[aim_roll_index];
        bool is_hide = aim_item->isHide();
        int insert_index = aim + aim_subs + 1; // 卷尾

        NovelDirItem item = cr_list[org];
        cr_list.removeAt(org); // 上面的 orgItem 是指针，对象已经被删除了，所以重新定了个变量

        if (insert_index >= org)
            insert_index--;
        item.setHide(is_hide);
        if (aim_roll_index == 0) // 作品相关
            item.setNumber(0);
        item.changePos(aim_roll_index-org_roll_index, 0);
        cr_list.insert(insert_index, item);

        roll_subs[org_roll_index]--;
        roll_subs[aim_roll_index]++;

        QString c_name = org_item->getName();
        QString r_name = roll_names[aim_roll_index];
        c_name = fnEncode(c_name);

        // 提取章节
        int pos = full_text.indexOf(QString("<CHPT><n:%1>").arg(c_name));
        if (pos == -1) return false;
        int left_pos = full_text.lastIndexOf(QString("<CHPT>"), pos);
        int right_pos = full_text.indexOf(QString("</CHPT>"), pos);
        if (left_pos == -1 || right_pos == -1) return false;
        QString move_chapter = full_text.mid(left_pos, right_pos+7-left_pos);
        full_text = full_text.left(left_pos) + full_text.right(full_text.length() - right_pos-7);

        // 找到插入位置
        pos = full_text.indexOf(QString("<RINF><n:%1>").arg(r_name));
        if (pos == -1) return false;
        int insert_pos = full_text.indexOf(QString("</LIST>"), pos);
        if (insert_pos == -1) return false;
        full_text = full_text.left(insert_pos) + move_chapter + full_text.right(full_text.length()-insert_pos);
    }
    else if (org_item->isRoll() && !aim_item->isRoll())  // 卷 -> 章 ： 【移动/分割 ？】
    {
        if (org_roll_index == aim_roll_index) // 同一卷拖拽
        {
            ;
        }
        else
        {
            ;
        }
    }
    else if (!org_item->isRoll() && !aim_item->isRoll()) // 章 -> 章 ： 【移动】
    {
        QString org_name = org_item->getName(), aim_name = aim_item->getName();
        org_name = fnEncode(org_name);
        aim_name = fnEncode(aim_name);

        if (aim_roll_index == 0) // 拖到作品相关
            cr_list[org].setNumber(0);
        cr_list[org].setHide(false/*cr_list[0].isHide()*/);
        cr_list[org].changePos(aim_roll_index-org_roll_index, 0); // 修改卷序

        cr_list.move(org, aim); // 移动过来

        roll_subs[org_roll_index]--;
        roll_subs[aim_roll_index]++;

        // 提取章节
        int pos = full_text.indexOf(QString("<CHPT><n:%1>").arg(org_name));
        if (pos == -1) return false;
        int left_pos = full_text.lastIndexOf(QString("<CHPT>"), pos);
        int right_pos = full_text.indexOf(QString("</CHPT>"), pos);
        if (left_pos == -1 || right_pos == -1) return false;
        QString move_chapter = full_text.mid(left_pos, right_pos+7-left_pos);
        full_text = full_text.left(left_pos) + full_text.right(full_text.length() - right_pos-7);
        // 找到插入位置
        pos = full_text.indexOf(QString("<CHPT><n:%1>").arg(aim_name));
        if (pos == -1) return false;
        int insert_pos;
        if (org > aim) // 从下往上，插入当aim的上一章
        {
            insert_pos = full_text.lastIndexOf(QString("<CHPT>"), pos);
            if (insert_pos == -1) return false;
        }
        else
        {
            insert_pos = full_text.indexOf(QString("</CHPT>"), pos);
            if (insert_pos == -1) return false;
            insert_pos += 7;
        }
        full_text = full_text.left(insert_pos) + move_chapter + full_text.right(full_text.length()-insert_pos);
    }

    // 调整所有位置
    adjustAllIndex();
    listDataChanged();
    saveDir();

    return true;
}

void NovelDirMData::adjustAllIndex()
{
    int r_index = 0, c_index = 0, i_index = 0;
    for (int i = 1; i < cr_list.size(); i++)
    {
        /*if (rIndex == 0 && !crList[i].isRoll()) // 作品相关
            crList[i].setNumber(0);
        else*/ if (cr_list[i].isRoll()) // 分卷
        {
            r_index++;
            i_index = 0;
            cr_list[i].setNumber(r_index);
            cr_list[i].setPos(r_index, i_index++);
        }
        else if (r_index /* && !crList[i].isRoll()*/) // 章节
        {
            c_index++;
            cr_list[i].setNumber(c_index);
            cr_list[i].setPos(r_index, i_index++);
        }
    }
}

void NovelDirMData::setChapterProperty(QString name, QString key, QString val)
{
    Q_UNUSED(name);
    Q_UNUSED(key);
    Q_UNUSED(val);
}

void NovelDirMData::setRollProperty(QString name, QString key, QString val)
{
    Q_UNUSED(name);
    Q_UNUSED(key);
    Q_UNUSED(val);
}

void NovelDirMData::gotoReedit()
{
    ;
}

void NovelDirMData::setRollHide(int index, bool hide)
{
    QString value;
    if (hide) value = "1";
    else value = "0";

    setItemProperty(index, "c", value);

    /*// 找到所在区域
    QString name = crList[index].getName();
    QString fnName = fnEncode(name);
    int leftPos = fullText.indexOf(QString("<RINF><n:%1>").arg(fnName));
    if (leftPos == -1) return ;
    int rightPos = fullText.indexOf(QString("</RINF>"), leftPos);
    if (rightPos == -1) return ;

    // 修改属性内容
    int pos = fullText.lastIndexOf(QString("<c:"), rightPos);
    if (pos == -1 || pos <= leftPos) // 不存在这个属性
    {
        fullText = fullText.left(rightPos) + str + fullText.right(fullText.length()-rightPos);
    }
    else // 找到这个属性
    {
        int rPos = fullText.indexOf(QString(">"), pos);
        if (rPos >= rightPos) return ;
        fullText = fullText.left(pos) + str + fullText.right(fullText.length()-rPos-1);
    }

    saveDir();*/
}

void NovelDirMData::setChapterWc(int index, int wc)
{
    setItemProperty(index, "w", QString("%1").arg(wc));
}

void NovelDirMData::setItemProperty(int index, QString key, QString value)
{
    //key = fnEncode(key);
    //value = fnEncode(key);
    QString str = QString("<%1:%2>").arg(key).arg(value);
    QString label = cr_list[index].isRoll() ? QString("RINF") : QString("CHPT");

    // 找到所在区域
    QString name = cr_list[index].getName();
    QString fn_name = fnEncode(name);
    int left_pos = full_text.indexOf(QString("<%1><n:%2>").arg(label).arg(fn_name));
    if (left_pos == -1) return ;
    int right_pos = full_text.indexOf(QString("</%1>").arg(label), left_pos);
    if (right_pos == -1) return ;

    // 修改属性内容
    int pos = full_text.lastIndexOf(QString("<%1:").arg(key), right_pos);
    if (pos == -1 || pos <= left_pos) // 不存在这个属性
    {
        full_text = full_text.left(right_pos) + str + full_text.right(full_text.length()-right_pos);
    }
    else // 找到这个属性
    {
        int r_pos = full_text.indexOf(QString(">"), pos);
        if (r_pos >= right_pos) return ;
        full_text = full_text.left(pos) + str + full_text.right(full_text.length()-r_pos-1);
    }

    saveDir();
}

QString NovelDirMData::getItemProperty(int index, QString key)
{
    //key = fnEncode(key);
    //value = fnEncode(key);
    QString label = cr_list[index].isRoll() ? QString("RINF") : QString("CHPT");

    // 找到所在区域
    QString name = cr_list[index].getName();
    QString fn_name = fnEncode(name);
    int left_pos = full_text.indexOf(QString("<%1><n:%2>").arg(label).arg(fn_name));
    if (left_pos == -1) return nullptr;
    int right_pos = full_text.indexOf(QString("</%1>").arg(label), left_pos);
    if (right_pos == -1) return nullptr;

    // 获取属性位置
    int pos = full_text.lastIndexOf(QString("<%1:").arg(key), right_pos);
    if (pos == -1 || pos <= left_pos) // 不存在这个属性
    {
        return nullptr;
    }
    else // 找到这个属性
    {
        int r_pos = full_text.indexOf(QString(">"), pos);
        if (r_pos >= right_pos) return nullptr;
        pos += 2+key.length();
        return full_text.mid(pos, r_pos-pos);
    }
}



