#ifndef CARDLIBHIGHLIGHT_H
#define CARDLIBHIGHLIGHT_H

#include <QRegExp>
#include "globalvar.h"

class CardlibHighlight
{
public:
    /**
     * 获取关键词高亮后的HTML
     * 使用正则表达式替换
     * 名片高亮
     */
    static QString getHighlightedText(QString text)
    {
        // 高亮人物语言
        if (us->quote_colorful)
        {
            QRegExp re(QStringLiteral("“([^”]+)”"));
//            QRegExp re(QStringLiteral("“(.+?)”")); // 不知道为什么匹配不到啊
            QString cs = static_cast<QVariant>(us->quote_colorful_color).toString();
            text.replace(re, "“<font color = '"+cs+"'>\\1</font>”");
        }

        // 高亮章节备注
        if (us->chapter_mark)
        {
            QRegExp re("(@.*?)$");
            QString cs = static_cast<QVariant>(us->chapter_mark_color).toString();
            text.replace(re, "<font color = '" + cs + "'>\\1</font>");
        }

        // 高亮自定义正则表达式


        // 高亮名片
        if (gd->clm.currentLib() != nullptr)
        {
            CardList &cards = gd->clm.currentLib()->using_cards;
            foreach (CardBean *card, cards)
            {
                QRegExp re("("+card->pattern()+")");
                QString cs = static_cast<QVariant>(card->color).toString();
                text.replace(re, "<font color = '" + cs + "'>\\1</font>");
            }
        }

        return text;
    }
};

#endif // CARDLIBHIGHLIGHT_H
