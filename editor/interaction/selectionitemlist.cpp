#include "selectionitemlist.h"

SelectionItemList::SelectionItemList()
{
    position = -1;
}

void SelectionItemList::clear()
{
    list.clear();
    position = -1;
}

void SelectionItemList::append(SelectionItem si)
{
    clearRedundant();
    // 和上一个进行比较，如果相同则退出
    if (position >= 0)
    {
        SelectionItem s = list[position];
        if (s.start == si.start && s.end == si.end)
            return ;
    }
    list.append(si);
    position++;
}

void SelectionItemList::append(int s, int e, int p)
{
    append(SelectionItem(s, e, p));
}

void SelectionItemList::append(QTextCursor c)
{
    append(c.selectionStart(), c.selectionEnd(), c.position());
}

SelectionItem SelectionItemList::get(int index)
{
    if (index == -1)
        index = position;
    if (position < 0 || position >= list.size())
        return SelectionItem(-1,-1,-1);
    return list.at(index);

}

SelectionItem SelectionItemList::back()
{
    if (position >= 0)
        position--;
    return get(position);
}

SelectionItem SelectionItemList::next()
{
    if (position < list.size()-1)
        position++;
    return get(position);
}

bool SelectionItemList::getCallback(SelectionItem si, int &s, int &e)
{
    if (si.start == -1 || si.end == -1)
        return false;
    if (si.pos == si.start)
    {
        s = si.end;
        e = si.start;
    }
    else
    {
        s = si.start;
        e = si.end;
    }
    return true;
}

bool SelectionItemList::getCallback(int& s, int& e)
{
    return getCallback(get(), s, e);
}

void SelectionItemList::clearRedundant()
{
    while (list.size() > position+1)
        list.removeLast();
}
