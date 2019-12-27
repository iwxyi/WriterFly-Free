#include "editorrecorderitem.h"

EditorRecorderItem::EditorRecorderItem()
{

}

EditorRecorderItem::EditorRecorderItem(QString text, int pos, int scroll)
{
    this->text = text;
    this->pos = pos;
    this->scroll = scroll;
    this->time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    this->start = this->end = -1;
    this->cas = 0;
}

QString EditorRecorderItem::getText()
{
    return text;
}

int EditorRecorderItem::getPos()
{
    return pos;
}

int EditorRecorderItem::getScroll()
{
    return scroll;
}

qint64 EditorRecorderItem::getTime()
{
    return time;
}

void EditorRecorderItem::getSelection(int &start, int &end)
{
    start = this->start;
    end = this->end;
}

int EditorRecorderItem::getCase()
{
    return cas;
}

void EditorRecorderItem::setText(QString text)
{
    this->text = text;
}

void EditorRecorderItem::setPos(int pos)
{
    this->pos = pos;
}

void EditorRecorderItem::setScroll(int scroll)
{
    this->scroll = scroll;
}

void EditorRecorderItem::updateTime()
{
    this->time = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void EditorRecorderItem::setSelection(int start, int end)
{
    this->start = start;
    this->end = end;
}

void EditorRecorderItem::setCase(int x)
{
    this->cas = x;
}
