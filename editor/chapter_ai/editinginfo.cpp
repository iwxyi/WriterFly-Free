#include "editinginfo.h"

EditingInfo::EditingInfo()
{
    words_count = 0;
}

EditingInfo::EditingInfo(EditType type)
{
	this->edit_type = type;
    words_count = 0;
}

void EditingInfo::setType(EditType type)
{
    this->edit_type = type;
}

void EditingInfo::setNames(QString novelName, QString chapterName)
{
    this->novel_name = novelName;
    this->chapter_name = chapterName;

    this->full_name = novelName + " / " + chapterName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novelName) + "/chapters/" + fnEncode(chapterName) + ".txt";
}

void EditingInfo::setChapterName(QString chapterName)
{
    if (novel_name == "") return ;
    this->chapter_name = chapterName;
    this->full_name = novel_name + " / " + chapterName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fnEncode(chapterName) + ".txt";
}

void EditingInfo::setNovelName(QString novelName)
{
    this->novel_name = novelName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novelName) + "/chapters/" + fnEncode(chapter_name) + ".txt";
}

void EditingInfo::setPath(QString path)
{
    this->file_path = path;
}

void EditingInfo::setFullChapterName(QString fullChapterName)
{
    this ->full_chapter_name = fullChapterName;
    if (fullChapterName != "")
        this->full_name = QString("%1 / %2").arg(novel_name).arg(fullChapterName);
    else
        this->full_name = QString("%1 / %2").arg(novel_name).arg(chapter_name);
}

void EditingInfo::setSavedText(QString text)
{
    this->saved_text = text;
}

EditType EditingInfo::getType()
{
    return this->edit_type;
}

QString EditingInfo::getFullChapterName()
{
    return full_chapter_name;
}

QString EditingInfo::getFullName()
{
    return full_name; // 返回章节全内容
}

QString EditingInfo::getNovelName()
{
    return novel_name;
}

QString EditingInfo::getChapterName()
{
    return chapter_name;
}

void EditingInfo::setPos(int x)
{
    this->position = x;
}

int EditingInfo::getPos()
{
    return position;
}

void EditingInfo::setWc(int x)
{
    this->words_count = x;
}

int EditingInfo::getWc()
{
    return words_count;
}

QString EditingInfo::getPath()
{
    return file_path;
}

QString EditingInfo::getSavedText()
{
    return saved_text;
}

bool EditingInfo::isChapter()
{
    return edit_type == EDIT_TYPE_CHAPTER;
}

bool EditingInfo::isOutline()
{
    return edit_type == EDIT_TYPE_OUTLINE;
}

bool EditingInfo::isFineOl()
{
    return edit_type == EDIT_TYPE_FINE_OL;
}

bool EditingInfo::isBrief()
{
    return edit_type == EDIT_TYPE_BRIEF;
}

bool EditingInfo::isDetail()
{
    return edit_type == EDIT_TYPE_DETAIL;
}

bool EditingInfo::isCompact()
{
    return edit_type != EDIT_TYPE_CHAPTER && edit_type != EDIT_TYPE_OUTLINE;
}

