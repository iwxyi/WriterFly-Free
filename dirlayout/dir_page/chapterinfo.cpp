#include "chapterinfo.h"

QString ArabToCN(int num);

ChapterInfo::ChapterInfo() : opening(false)
{

}

ChapterInfo::ChapterInfo(QString name) : name(name), number(0), opening(false), file_time(-1), word_count(-1)
{

}

ChapterInfo::ChapterInfo(QString name, int number) : name(name), number(number), opening(false)
{

}

void ChapterInfo::setName(QString n)
{
    name = n;
    full_chapter_name = "";
}

void ChapterInfo::setNumber(int s)
{
    number = s;
    number_cn = "";
    number_holder = "";
}

void ChapterInfo::setNumber_cn(QString n)
{
    number_cn = n;
    full_chapter_name = "";
    number_holder = "";
}

void ChapterInfo::setDetail(QString d)
{
    detail = d;
}

void ChapterInfo::open()
{
    opening = true;
}

void ChapterInfo::close()
{
    opening = false;
}

void ChapterInfo::changeNumber(int x)
{
    number += x;
    number_cn = "";
    full_chapter_name = "";
}

void ChapterInfo::setPos(int x, int y)
{
    roll_index = x;
    chpt_index = y;
    full_chapter_name = "";
}

void ChapterInfo::setFullChapterName(QString s)
{
    full_chapter_name = s;
}

void ChapterInfo::changePos(int x, int y)
{
    roll_index += x;
    chpt_index += y;
    full_chapter_name = "";
}

void ChapterInfo::setWordCountAndTime(int count, qint64 time)
{
    word_count = count;
    file_time = time;
}

QString ChapterInfo::getName()
{
    return name;
}

int ChapterInfo::getNumber()
{
    return number;
}

QString ChapterInfo::getNumber_cn()
{
    if (number_cn == nullptr || number_cn == "")
    {
        full_chapter_name = "";
        number_cn = ArabToCN(number);
    }
    return number_cn;
}

QString ChapterInfo::getDetail()
{
    return detail;
}

int ChapterInfo::getRollIndex()
{
    return roll_index;
}

int ChapterInfo::getChptIndex()
{
    return chpt_index;
}

bool ChapterInfo::isOpening()
{
    return opening;
}

QString ChapterInfo::getFullChapterName()
{
    if (full_chapter_name == "")
    {
        if (number == 0)
            full_chapter_name = getName();
        else
            full_chapter_name = QString("%1 %2").arg(number).arg(getName());
    }
    return full_chapter_name;
}

int ChapterInfo::getWordCount()
{
    return word_count;
}

qint64 ChapterInfo::getWordCountTime()
{
    return file_time;
}
