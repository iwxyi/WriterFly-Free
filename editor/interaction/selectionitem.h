#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H

/**
 * 文字选择情况实体类
 */
struct SelectionItem
{
    SelectionItem()
    {
        pos = start = end = -1;
    }

    SelectionItem(int s, int e) : start(s), end(e), pos(-1) {}

    SelectionItem(int s, int e, int p) : start(s), end(e), pos(p) {}

    int start;
    int end;
    int pos;
};

#endif // SELECTIONITEM_H
