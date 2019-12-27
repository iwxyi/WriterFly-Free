#include "selecteditingcardsthread.h"

SelectEditingCardsThread::SelectEditingCardsThread(const QString &&content) : content(content)
{

}

void SelectEditingCardsThread::run()
{
    static int thread_lock = 0; // 多线程锁
    int current_lock = ++thread_lock;

    bool refresh_all_list = false;
    Cardlib *cardlib = gd->clm.currentLib();
    CardList editing_cards;

    // 检查用到的名片
    foreach (CardBean *card, cardlib->using_cards)
    {
        QRegExp re(card->pattern());
        int pos = re.indexIn(content);
        if (pos > -1)
        {
            int count = 0, len = re.pattern().length();
            while (pos > -1)
            {
                pos = re.indexIn(content, pos + len);
                count++;
            }
            card->sort_data = count;
            editing_cards.append(card);
        }
    }

    // 检查未用到的名片，添加到用到的名片
    foreach (CardBean *card, cardlib->unused_cards)
    {
        QRegExp re(card->pattern());
        int pos = re.indexIn(content);
        if (pos > -1)
        {
            int count = 0, len = re.pattern().length();
            while (pos > -1)
            {
                pos = re.indexIn(content, pos + len);
                count++;
            }
            card->sort_data = count;
            card->used = true;
            editing_cards.append(card);
            cardlib->using_cards.append(card);
            cardlib->unused_cards.removeOne(card);
            refresh_all_list = true;
        }
    }

    // 需要刷新整个的列表
    if (refresh_all_list)
    {
        emit signalRefreshCardsList();
    }

    // 编辑器名片按出现次数排序
    std::sort(editing_cards.begin(), editing_cards.end(), [=](CardBean *a, CardBean *b) {
        return a->sort_data > b->sort_data;
    });

    // 如果是多个线程并发的，那么只调用最后一个（即相等的）
    if (current_lock == thread_lock)
    {
        gd->clm.currentLib()->editing_cards = editing_cards;
        emit signalRefreshEditingCardList();
    }
}
