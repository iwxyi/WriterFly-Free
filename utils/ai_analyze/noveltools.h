#ifndef NOVELTOOLS_H
#define NOVELTOOLS_H

#include <QMap>
#include "stringutil.h"
#include "fileutil.h"
#include "settings.h"
#include "globalvar.h"

class NovelTools
{
public:
    /**
     * 通过小说名字获取小说的类型AI
     * @param name 小说名字
     * @return     小说类型，为空表示无法判断
     */
    static QString getTypeFromName(QString name)
    {
        // 处理名字
        name = name.toUpper(); // 转到大写，例如NBA

        // 读取文件
        QString result = "";
        QString dataset = readTextFile(":/database/novel_type");

        // 最高纪录
        int max_type_count = 0;
        QString max_type_name = "";

        // 分析每个类型的值
        QStringList type_str_list = getXmls(dataset, "TYPE");
        for (int i = 0; i < type_str_list.length(); i++)
        {
            QString type_str = type_str_list.at(i);
            QString type_name = getXml(type_str, "NAME").trimmed();
            QString type_dict = getXml(type_str, "DICT").trimmed();
            int integral = 0;
            QStringList keys = type_dict.split(" ", QString::SkipEmptyParts);
            for (int j = 0; j < keys.length(); j++)
            {
                if (name.indexOf(keys.at(j)) > -1)
                    integral++;
            }
            if (integral > max_type_count)
            {
                max_type_count = integral;
                max_type_name = type_name;
            }
            //if (type_name == "二次元") type_name = "次元";
            //qDebug() << i << "\t: " << type_name << "\t- " << integral << "\tmax : " << max_type_name << " - " << max_type_count;
        }

        if (max_type_name != "")
            result = max_type_name;
        return result;
    }


    /**
     * 获取可能的小说名AI
     * @param novels_dir_path 小说存储路径（读取已有小说及其创建时间）
     * @return                可能的小说名字
     */
    static QString getNewNameAI(QString novels_dir_path)
    {
        QString name = "新的作品";

        if (!rt->deleted_novel_name.isEmpty()
            && !canRegExp(rt->deleted_novel_name, "^新的作品.*")
            && getTimestamp()-rt->deleted_novel_timestamp<30*1000
            && !isFileExist(novels_dir_path+rt->deleted_novel_name)) // 30秒内刚删除的作品
        {
            name = rt->deleted_novel_name; // 设置为刚删除的那本书
        }
        else if (getDirNumber(novels_dir_path) == 0) // 没有作品，全新的用户
        {
            name = "第一部作品";
        }
        else if (isFileExist(novels_dir_path+name) // 已经存在《新的作品》
            && (getDirNumber(novels_dir_path) == 1 || canRegExp(rt->current_novel, "^新的作品")))
             // 嗯，这是一个懒得起名字的作者，居然懒成这样！得让他勤奋一些，自己输名字吧，后面就不进行判断了！
        {
            int index = 1; // 书名重复的数量
            while (isFileExist(novels_dir_path+name+QString::number(++index))) ;
            name = name + QString::number(index);
        }
        else // 已经有其他作品的用户
        {
            QDir novels_dir(novels_dir_path);
            qint64 latest_timestemp = 0; // 最后创建的作品时间
            QString latest_name = ""; // 最后创建的作品名字
            QString judge_name = "";  // 用来判断的名字

            // 遍历目录下所有的作品，获取时间
            QList<QFileInfo>* novels_info = new QList<QFileInfo>(novels_dir.entryInfoList(QDir::Dirs));

            for (int i = 0; i < novels_info->count(); i++)
            {
                QString novel_name = novels_info->at(i).fileName(); // 小说名字
                QString settings_path = novels_info->at(i).filePath()+"/settings.ini"; // 设置文件的路径
                if (isFileExist(settings_path))
                {
                    Settings sts(settings_path);
                    qint64 time = sts.getLongLong("create_timestamp");
                    if (time > latest_timestemp)
                    {
                        latest_timestemp = time;
                        latest_name = novel_name;
                    }
                }
            }

            if (latest_name.isEmpty() && getDirNumber(novels_dir_path) == 1) // 只有一部作品，那就是这一个了！
            {
                latest_name = novels_info->at(0).fileName();
            }

            log("最近作品", latest_name);

            // 获取判断的书籍
            QString reg = "^.*[\\(（]?第?[一二三四五六七八九十0123456789].?[\\)）]?$";
            if (canRegExp(rt->current_novel, reg))
            {
                judge_name = rt->current_novel;
            }
            else if (!latest_name.isEmpty() && canRegExp(latest_name, reg))
            {
                judge_name = latest_name;
            }
            /*if (!latest_name.isEmpty() && canRegExp(latest_name, reg)) // 首先使用最后创建的一本书（至少不会比现在打开的这本早吧）
            {
                judge_name = latest_name;
            }
            else if (canRegExp(rt->current_novel, reg))
            {
                judge_name = rt->current_novel;
            }*/

            log("判断作品", judge_name);

            /**
              根据正则表达式判断小说名字
              小说名字格式：小说一、小说（二）、小说第三部、小说（第四部）、小说5、小说 6、小说(7)
              只要最后几个名字包含数字
              */
            if (!judge_name.isEmpty())
            {
                QString chinese_numbers = "一二三四五六七八九十";
                QString arab_numbers = "0123456789";
                int char_pos = -1; // 字符所在的位置
                QString char_char = "", next_char = "";
                for (int i = 0; i < chinese_numbers.length()-1/*不包括最后一个序号*/; i++)
                {
                    QString ch = chinese_numbers.mid(i, 1);
                    int find_pos = judge_name.lastIndexOf(ch);
                    if (find_pos > char_pos)
                    {
                        // 判断书名存不存在
                        QString cha = chinese_numbers.mid(i+1, 1);
                        QString new_name = judge_name.left(find_pos) + cha + judge_name.right(judge_name.length()-find_pos-1); // 新的名字
                        QString novel_path = novels_dir_path + new_name; // 新书名名字
                        if (isFileExist(novel_path)) // 如果重复了，就遍历直到最后一个
                        {
                            int index = i+1;
                            while (++index < chinese_numbers.length())
                            {
                                cha = chinese_numbers.mid(index, 1);
                                new_name = judge_name.left(find_pos) + cha + judge_name.right(judge_name.length()-find_pos-1); // 新的名字
                                novel_path = novels_dir_path + new_name; // 新书名名字
                                if (!isFileExist(novel_path))
                                    break;
                            }
                            if (index < chinese_numbers.length())
                            {
                                char_pos = find_pos;
                                next_char = chinese_numbers.mid(index, 1);
                            }
                        }
                        else // 没有重复，直接使用
                        {
                            char_pos = find_pos;
                            next_char = chinese_numbers.mid(i+1, 1);
                        }
                    }
                }
                for (int i = 0; i < arab_numbers.length()-1/*不包括最后一个序号*/; i++)
                {
                    QString ch = arab_numbers.mid(i, 1);
                    int find_pos = judge_name.lastIndexOf(ch);
                    if (find_pos > char_pos)
                    {
                        // 判断书名存不存在
                        QString cha = arab_numbers.mid(i+1, 1);
                        QString new_name = judge_name.left(find_pos) + cha + judge_name.right(judge_name.length()-find_pos-1); // 新的名字
                        QString novel_path = novels_dir_path + new_name; // 新书名名字
                        if (isFileExist(novel_path)) // 如果重复了，就遍历直到最后一个
                        {
                            int index = i+1;
                            while (++index < arab_numbers.length())
                            {
                                cha = arab_numbers.mid(index, 1);
                                new_name = judge_name.left(find_pos) + cha + judge_name.right(judge_name.length()-find_pos-1); // 新的名字
                                novel_path = novels_dir_path + new_name; // 新书名名字
                                if (!isFileExist(novel_path))
                                    break;
                            }
                            if (index < arab_numbers.length())
                            {
                                char_pos = find_pos;
                                next_char = arab_numbers.mid(index, 1);
                            }
                        }
                        else // 没有重复，直接使用
                        {
                            char_pos = find_pos;
                            next_char = arab_numbers.mid(i+1, 1);
                        }
                    }
                }
                if (char_pos >= 0)
                {
                    name = judge_name.left(char_pos) + next_char + judge_name.right(judge_name.length()-char_pos-1);
                }
            }
        }

        if (isFileExist(novels_dir_path+name))
        {
            int index = 1; // 书名重复的数量
            while (isFileExist(novels_dir_path+name+QString::number(++index))) ;
            name = name + QString::number(index);
        }

        return name;
    }

    static QString getNewChapterAI(QString full_text, QString pre1_chapter, QString pre2_chapter)
    {

        QStringList exist_names = getStrMids(full_text, "<CHPT><n:", ">");

        // 获取默认名字
        int i = 1;
        while (full_text.indexOf(QObject::tr("<CHPT><n:新章%1>").arg(i)) > 0)
            i++;
        QString default_name = QObject::tr("新章%1").arg(i);

        // 如果是空的
        if (pre1_chapter.isEmpty()) return default_name;

        // 假定是数字变量，保存结果和数字末尾位置（最后一个字符后面）
        int chinese_number_end_pos = -1, arab_number_end_pos = -1;
        QString chinese_added = findAndAddChineseNumber(pre1_chapter, chinese_number_end_pos, exist_names); // 找到中文数字并加一
        QString arab_added = findAndAddArabNumber(pre1_chapter, arab_number_end_pos, exist_names); // 找到阿拉伯数字并加一

        // 连续的文字
        QStringList sequences;
        sequences << "一二三四五六七八九" << "0123456789"    // 这几个其实是在另一个函数判断的
                  << "壹贰叁肆伍陆柒捌玖"
                  << "上中下" << "前中后" << "初中末"
                  << "①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳"
                  << "ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫ"
                  << "ⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹ";
        QString find_char = "", next_char = ""; // 找到的字符、需要替换的字符
        int find_max_pos = -1; // 最后找到的文字位置
        for (int si = 0; si < sequences.size(); si++)
        {
            QString sequence = sequences.at(si);

            for (int i = 0; i < sequence.length()-1; i++) // 遍历每一个字符
            {
                QString ch = sequence.mid(i, 1);
                int find_pos = pre1_chapter.lastIndexOf(ch);
                if (find_pos == -1) continue;
                if (find_pos < pre1_chapter.length()-2) // 不是最后一个字符，例如：《第一次尝试》、《第二次尝试》
                {
                    if (i == 0)
                        continue;
                    if (pre1_chapter.length() != pre2_chapter.length())
                        continue;
                    QString ch_in_pre2 = pre2_chapter.mid(find_pos, 1);
                    if (sequence.indexOf(ch_in_pre2) == -1) // pre2 没有序号
                        continue;
                    if (pre1_chapter.left(find_pos) != pre2_chapter.left(find_pos)) // 左边不一样
                        continue;
                    if (pre1_chapter.right(pre1_chapter.length()-find_pos-1)
                            != pre2_chapter.right(pre2_chapter.length()-find_pos-1))
                        continue;
                }
                if (find_pos <= find_max_pos) // 先前已经有足够靠后的序号了
                    continue;
                if (i > 0 && pre1_chapter.length() == pre2_chapter.length()) // 说明这个可能是第二个
                {
                    QString ch_in_pre2 = pre2_chapter.mid(find_pos, 1);
                    if (ch_in_pre2 != sequence.mid(i-1, 1))
                        continue;
                    if (pre1_chapter.right(pre1_chapter.length()-find_pos-1)
                            != pre2_chapter.right(pre2_chapter.length()-find_pos-1))
                        continue;
                }
                find_max_pos = find_pos;
                find_char = ch;
                next_char = sequence.mid(i+1, 1); // 下一个字符
            }
        }

        // 先比较数字结果
        chinese_number_end_pos--;  arab_number_end_pos--; // 一个是结尾位置，这里判断的是最后一个字符位置
        if (chinese_number_end_pos > find_max_pos && chinese_number_end_pos > arab_number_end_pos)
            return chinese_added;
        if (arab_number_end_pos > find_max_pos && arab_number_end_pos > chinese_number_end_pos)
            return arab_added;

        if (find_max_pos >= 0)
        {
            // 最后一位不是9的话，直接加一
            QString name = pre1_chapter.left(find_max_pos) + next_char + pre1_chapter.right(pre1_chapter.length()-find_max_pos-1);
            return name;
        }

        return default_name;
    }

    static QString findAndAddChineseNumber(QString text, int& find_pos, QStringList& names)
    {
        QString pat = "[零一二三四五六七八九十百千万]+";
        QRegularExpression re(pat);
        QRegularExpressionMatch match;
        find_pos = text.lastIndexOf(re, -1, &match);
        if (find_pos == -1) return "";
        QString capture = match.captured();

        int num = CNToArab(capture);
        QString name = text.left(find_pos) + ArabToCN(++num) + text.right(text.length()-find_pos-capture.length());
        while (names.contains(name)) {
            name = text.left(find_pos) + ArabToCN(++num) + text.right(text.length()-find_pos-capture.length());
        }
        find_pos += capture.length();
        return name;
    }

    static QString findAndAddArabNumber(QString text, int& find_pos, QStringList& names)
    {
        QString pat = "[0123456789]+";
        QRegularExpression re(pat);
        QRegularExpressionMatch match;
        find_pos = text.lastIndexOf(re, -1, &match);
        if (find_pos == -1) return "";
        QString capture = match.captured();
        int num = capture.toInt();
        QString name = text.left(find_pos) + QString::number(++num) + text.right(text.length()-find_pos-capture.length());
        while (names.contains(name)) {
            name = text.left(find_pos) + QString::number(++num) + text.right(text.length()-find_pos-capture.length());
        }
        find_pos += capture.length();
        return name;
    }

    /**
     * 获取正文内容，过滤文章标注（@开头的）
     * @param text 正文
     * @return     去掉标注后的正文
     */
    static QString getMainBodyWithoutMark(QString text)
    {
        if (!us->chapter_mark) return text;

        QRegExp re;
        QString pat;
        QString rep;
        re.setMinimal(true);

        // ==== 替换标注单独行 ====
        // 与后缀不同的时候，包括删除前面空白

        // 替换中间：单独一行的，去掉这一整行，直到后一段开头（当做全部不存在）
        pat = "\\n[　 \\t]*@[^\\n]*\\s*(?=\\n[　 \\t]*[\\S])";
        rep = "";
        re.setPattern(pat);
        text = text.replace(re, rep);

        // 替换开头
        pat = "^\\s*@[^\\n]*\\s*\\n(?=[　 \\t]*[\\S])";
        rep = "";
        re.setPattern(pat);
        text = text.replace(re, rep);

        // 替换结尾
        pat = "\\s*@[^\\n]*\\s*$";
        rep = "";
        re.setPattern(pat);
        text = text.replace(re, rep);

        // ==== 替换标注后缀 ====
        pat = "@[^\\n]*";
        rep = "";
        re.setPattern(pat);
        re.setMinimal(false);
        text = text.replace(re, rep);

        // ==== 啥都没有？？？ ====
        // 不进行替换

        return text;
    }


    /**
     * 获取文章有效字数统计，排除空白符、计算同一个单词数、连续标点
     * @param text 正文
     * @return     字数
     */
    static int getArticleWordCount(QString& text)
    {
        int count = 0;

        // 非空白符的字数
        int len = text.length();
        QString ch;
        for (int i = 0; i < len; i++)
        {
            ch = text.mid(i, 1);
            if (ch != " " && ch != "　" && ch != "\n" && ch != "\t" && ch != "\r")
                count++;
        }

        // 去掉单词/数字(- 字符数 - 连续单词数)


        // 去掉 破折号、省略号（-数量）


        return count;
    }

    /**
     * 显示字数统计的详情
     * @param text    要统计的文本（右值引用）
     * @param widget  要显示的对象
     */
    static void showWordCountDetail(QString&& text, QString title = "字数统计", QWidget* widget = nullptr)
    {
        Q_UNUSED(widget);

        QString result = "";        // 给用户看的数据
        int ascii_count = 0;        // 所有字数
        int valid_count = 0;        // 有效字数
        int blank_count = 0;        // 空白符数量（不包括换行）
        int return_count = 0;       // 换行符数量
        int para_count = 0;         // 段落数量
        int chinese_count = 0;      // 汉字字数
        int english_count = 0;      // 英语字母数
        int number_count = 0;       // 阿拉伯数字数
        int chinese_punc_count = 0; // 标点数量（计算破折号和省略号为1）
        int english_punc_count = 0; // 英语标点数量
        int unknow_punc_count = 0;  // 其余未知标点
        int dashes_ellipsis_count = 0;  // 破折号和省略号
        int word_count = 0;         // 单词数（\\w+）
        int said_count = 0;         // 人物语言数量
        int rhetorical_quest = 0;   // 反问句数量

        bool short_text = false;

        ascii_count = text.length();
        short_text = ascii_count < 10000;

        // 遍历每个字符是什么
        for (int i = 0; i < ascii_count; i++)
        {
            QString ch = text.mid(i, 1);
            if (ch == "\n")
                return_count++;
            else if (isBlank(ch))
                blank_count++;
            else if (isChinese(ch))
                chinese_count++;
            else if (isEnglish(ch))
                english_count++;
            else if (isNumber(ch))
                number_count++;
            else if (isChinesePunc(ch))
                chinese_punc_count++;
            else if (isEnglishPunc(ch))
                english_punc_count++;
            else
                unknow_punc_count++;
        }

        // 统计两个的破折号和省略号
        dashes_ellipsis_count = getAppearTimesInString(text, "——")
                + getAppearTimesInString(text, "……");
        chinese_punc_count -= dashes_ellipsis_count;

        // 统计单词数量
        word_count = getAppearTimesInString(text, "\\w+");

        // 统计人物语言数量
        if (short_text)
        {
            said_count = getAppearTimesInString(text, "“[^”]+”");
        }

        // 有效字数
        valid_count = chinese_count + chinese_punc_count + english_punc_count + word_count;

        // 段落数量
        if (short_text) // 一万字以内的
        {
            QStringList sl = text.split("\n");
            foreach (QString s, sl)
            {
                if (!isAllBlank(s))
                    para_count++;
            }
        }

        // 反问句彩蛋
        if (short_text)
        {

        }

        // 设置显示的文字
        result.append("　字符数：" + QString::number(ascii_count));
        result.append("\n有效字数：" + QString::number(valid_count));
        result.append("\n--------------");
        result.append("\n中文字数：" + QString::number(chinese_count));
        if (number_count)
            result.append("\n数字数量：" + QString::number(number_count));
        if (english_count)
            result.append("\n字母数量：" + QString::number(english_count));
        if (word_count)
            result.append("\n单词数量：" + QString::number(word_count));
        if (chinese_punc_count)
            result.append("\n中文标点：" + QString::number(chinese_punc_count));
        if (english_punc_count)
            result.append("\n英文标点：" + QString::number(english_punc_count));
        if (para_count)
            result.append("\n段落数量：" + QString::number(para_count));
        if (return_count)
            result.append("\n换行数量：" + QString::number(return_count));
        if (blank_count)
            result.append("\n空白数量：" + QString::number(blank_count));
        if (unknow_punc_count)
            result.append("\n未知标点：" + QString::number(unknow_punc_count));
        if (said_count)
            result.append("\n人物语言：" + QString::number(said_count));
        if (rhetorical_quest)
            result.append("\n　反问句：" + QString::number(rhetorical_quest));
        if (dashes_ellipsis_count)
            result.append("\n\n破折号/省略号：" + QString::number(dashes_ellipsis_count) + "对");

        QMessageBox msg(rt->getMainWindow());
        msg.setPalette(QApplication::palette());
        msg.setWindowTitle(title);
        msg.setText(result);
        msg.setIconPixmap(QPixmap(":/icons/menu/word_count"));
        msg.addButton(QObject::tr("确定"), QMessageBox::ButtonRole::AcceptRole);
        msg.addButton(QObject::tr("复制"), QMessageBox::ButtonRole::ActionRole);
        int ret = msg.exec();
        if (ret == 1/*复制的索引*/)
        {
            QClipboard* cb = QApplication::clipboard();
            cb->setText(result);
//            QMessageBox::information(widget, "字数统计", "已复制到剪贴板")
        }
    }

    static void locateOutline(const QString full_outline, const QString chapter_content, int& start, int& end)
    {
        QStringList paras = full_outline.split("\n", QString::SkipEmptyParts);
        for (int i = 0; i < paras.size(); i++)
            paras[i] = paras[i].trimmed(); // 去掉空白符

        // 停用词
        QStringList& stop_words = gd->lexicons.getStopWords();

        // 最大值
        int max_prop = -1;
        QString max_para = "";

        // 遍历每一段大纲
        foreach (QString para, paras)
        {
            if (para.length() == 0) continue; // 如果长度为零（不太可能）

            QStringList words = gd->participle->WordSegment(para); // 分词

            // 去掉停用词
            for (int i = 0; i < words.size(); i++)
                if (isBlankChar(words.at(i)) || stop_words.contains(words.at(i)))
                    words.removeAt(i--);

            // 判断在段落中出现的比例
            int para_appear = 0, chapter_appear_length = 0;
            for (int i = 0; i < words.size();  i++)
                if (chapter_content.contains(words.at(i))) // 正文中包含大纲中的这个词语
                {
                    para_appear++;

                    // 获取这个词语出现的次数，成为另一个调整参数
                    int count = getAppearTimesInString(chapter_content, words.at(i));
                    chapter_appear_length += count * words.at(i).length();
                }

            // 比例算法：100 * 大纲段落中词语出现次数^2 * 正文中出现大纲词语的长度
            int prop = 100 * para_appear * para_appear * chapter_appear_length / words.size() / words.size() / chapter_content.length();
//            qDebug() << "prop:" << prop << 100*para_appear/words.size() << 100 * chapter_appear_length/chapter_content.length();
            if (prop > max_prop)
            {
                max_prop = prop;
                max_para = para;
            }
        }

        if (max_prop > -1)
        {
            start = full_outline.indexOf(max_para);
            if (start == -1) return ;
            end = full_outline.indexOf("\n", start+1);
            if (end == -1) // 段落末尾
                end = full_outline.length();
        }
        else
        {
            start = end = -1;
        }
    }

public:
    /**
     * 获取文件路径下文件夹的数量
     * 用于获取小说总路径中有多少本小说
     * @param dirPath 文件夹路径
     * @return 文件夹数量
     */
    static int getDirNumber(const QString &dirPath)
    {
        QDir dir(dirPath);
        QStringList filter;
        QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(QDir::Dirs));
        int count = fileInfo->count();
        for (int i = 0; i < fileInfo->count(); i++)
            if (fileInfo->at(i).fileName() == "." || fileInfo->at(i).fileName() == "..")
                count--;
        return count;
    }

    static bool isBlank(QString str)
    {
        if (str == "") return false;
        return str == " " || str == "　" || str == "\t" || str == "\r";
    }

    static bool isAllBlank(QString str)
    {
        int len = str.length();
        for (int i = 0; i < len; i++)
            if (!isBlank(str.mid(i,1)))
                return false;
        return true;
    }

    static bool isChinese(QString str)
    {
        if (str == "") return false;
        QChar ch = str.at(0);
        ushort uni = ch.unicode();
        return (uni >= 0x4E00 && uni <= 0x9FA5);
    }

    static bool isEnglish(QString str)
    {
        if (str == "") return false;
        QByteArray ba = str.toLatin1();
        const char *s = ba.data();
        return ( (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') );
    }

    static bool isNumber(QString str)
    {
        if (str == "") return false;
        QByteArray ba = str.toLatin1();
        const char *s = ba.data();
        return ( *s >= '0' && *s <= '9' );
    }

    static bool isChinesePunc(QString str)
    {
        if (str == "") return false;
        return QString("。？！，、；：“”‘’（）【】—…．《》·").indexOf(str) > -1;
    }

    static bool isEnglishPunc(QString str)
    {
        if (str == "") return false;
        QByteArray ba = str.toLatin1();
        const char *s = ba.data();
        return ( *s > 0 /*&& *s <= 127*/ ); // *s 永远 <= 127，不需要判断
    }

    static int getAppearTimesInString(const QString& text, QString pat)
    {
        int count = 0;
        QRegularExpression expression(pat);
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            i.next();
            count++;
        }
        return count;
    }

private:

};

#endif // NOVELTOOLS_H
