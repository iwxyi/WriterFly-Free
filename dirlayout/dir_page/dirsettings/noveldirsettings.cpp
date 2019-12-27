#include "noveldirsettings.h"

NovelDirSettings::NovelDirSettings()
        : start_chapter(1), start_roll(1), char_chapter("章"), char_roll("卷"),
          show_chapter_num(true), show_roll_num(true), show_chapter_words(false), show_change_time(false),
          use_arab(false), recount_num_by_roll(false), no_roll(false), show_word_count(false)
{
    s = nullptr;
}

NovelDirSettings::NovelDirSettings(QString novel_name)
{
    readSettings(novel_name);
}

void NovelDirSettings::readSettings(QString novel_name)
{
    this->novel_name = novel_name;
    if (novel_name.isEmpty()) return ;

    QString stPath = rt->NOVEL_PATH+novel_name+"/settings.ini";
    if (s != nullptr)
        delete s;
    s = new Settings(stPath);
    long long timestamp0 = s->getLongLong("create_timestamp");
    if (timestamp0 <= 0)
    {
        s->setVal("create_timestamp", getTimestamp());
    }

    // 读取目录的各项数据
    start_chapter = s->getInt("start_chapter", 1);
    start_roll = s->getInt("start_roll", 1);
    char_chapter = s->getStr("char_chapter", "章");
    char_roll = s->getStr("char_roll", "卷");
    show_chapter_num = s->getBool("show_chapter_num", true);
    show_roll_num = s->getBool("show_roll_num", true);
    show_chapter_words = s->getBool("show_chapter_words", false);
    show_change_time = s->getBool("show_chapter_time", false);
    use_arab = s->getBool("use_arab", false);
    recount_num_by_roll = s->getBool("recount_num_by_roll", false);
    no_roll = s->getBool("no_roll", false);
    chapter_preview = s->getBool("chapter_preview", false);
    show_word_count = s->getBool("word_count", false);
}

void NovelDirSettings::setVal(QString key, QVariant val)
{
    if (s == nullptr) return ;
    s->setVal(key, val);
}

QString NovelDirSettings::getNovelName()
{
    return novel_name;
}

Settings* NovelDirSettings::getSettings()
{
    return s;
}

int NovelDirSettings::getChapterStartNum() const
{
    return start_chapter;
}
int NovelDirSettings::getRollStartNum() const
{
    return start_roll;
}
QString NovelDirSettings::getChapterChar() const
{
    return char_chapter;
}
QString NovelDirSettings::getRollChar() const
{
    return char_roll;
}

bool NovelDirSettings::isRecountNumByRoll() const
{
    return recount_num_by_roll;
}
bool NovelDirSettings::isUseArab() const
{
    return use_arab;
}
bool NovelDirSettings::isShowChapterNum() const
{
    return show_chapter_num;
}
bool NovelDirSettings::isShowRollNum() const
{
    return show_roll_num;
}
bool NovelDirSettings::isShowChapterWords() const
{
    return show_chapter_words;
}
bool NovelDirSettings::isShowChangeTime() const
{
    return show_change_time;
}

bool NovelDirSettings::isNoRoll() const
{
    return no_roll;
}

bool NovelDirSettings::isChapterPreview() const
{
    return chapter_preview;
}

bool NovelDirSettings::isShowWordCount() const
{
    return show_word_count;
}

void NovelDirSettings::setStartChapterNum(int a)
{
    start_chapter = a;
}
void NovelDirSettings::setStartRollNum(int a)
{
    start_roll = a;
}
void NovelDirSettings::setChapterChar(QString a)
{
    char_chapter = a;
}
void NovelDirSettings::setRollChar(QString a)
{
    char_roll = a;
}
void NovelDirSettings::setRecountNumByRoll(bool a)
{
    recount_num_by_roll = a;
}
void NovelDirSettings::setUseArab(bool a)
{
    use_arab = a;
}
void NovelDirSettings::setShowChapterNum(bool a)
{
    show_chapter_num = a;
}
void NovelDirSettings::setShowRollNum(bool a)
{
    show_roll_num = a;
}
void NovelDirSettings::setShowChapterWords(bool a)
{
    show_chapter_words = a;
}
void NovelDirSettings::setShowChangeTime(bool a)
{
    show_change_time = a;
}

void NovelDirSettings::setChapterPreview(bool a)
{
    chapter_preview = a;
}

void NovelDirSettings::setShowWordCount(bool a)
{
    show_word_count = a;
}
