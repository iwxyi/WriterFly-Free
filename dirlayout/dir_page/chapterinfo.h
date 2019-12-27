#ifndef CHAPTERINFO_H
#define CHAPTERINFO_H

#include "stringutil.h"
#include "fileutil.h"

/**
 * 章节信息实体类
 */
class ChapterInfo
{
public:
    ChapterInfo();
    ChapterInfo(QString name);
    ChapterInfo(QString name, int number);

public:
    void setName(QString n);
    void setNumber(int n);
    void setNumber_cn(QString n);
    void setDetail(QString d);
    void open();  // 设置为打开状态
    void close(); // 设置为关闭状态
    void changeNumber(int x); // 调整序号的大小，并清空中文
    void setPos(int x, int y);
    void changePos(int x, int y); // 修改章节所在的位置
    void setFullChapterName(QString s);
    void setWordCountAndTime(int count, qint64 time);

    QString getName();
    int getNumber();
    QString getNumber_cn();
    QString getDetail();
    int getRollIndex(); // 下标从0开始（0是作品相关）
    int getChptIndex(); // 下标从1开始（0是卷名）
    bool isOpening();
    QString getFullChapterName();
    int getWordCount();
    qint64 getWordCountTime();

protected:
    QString name;       // 名字（未加密）
    int number;         // 章序（从1开始）;卷序（从0开始）
    QString number_cn;  // 中文章序，避免重复计算
    QString number_holder; // 显示的缓存
    int list_index;     // * 在列表的第几个（所有列表的真实索引,从0开始）
    int roll_index;     // * 第几卷（索引，从0开始）
    int chpt_index;     // * 卷内第几章（索引+1，从1开始）
    QString full_chapter_name; // 带章序的名字，这个已经没用了，被动态的 delegated 彻底代替了

    QString content;    // 内容
    QString detail;     // 细纲
    QString c_path;     // 内容路径
    QString d_path;     // 细纲路径
    int cn, eff_cn;     // 字符数、有效字数
    bool opening;       // 是否打开（可能有编辑过）
    qint64 file_time;   // 文件时间
    int word_count;     // 章节字数
};

#endif // CHAPTERINFO_H
