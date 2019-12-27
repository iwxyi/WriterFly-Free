#ifndef BOARDITEM_H
#define BOARDITEM_H

#include "stringutil.h"

/**
 * 板板的一个item对象
 */
class BoardItem
{
public:
    BoardItem(QString te, qint64 ti) : timestamp(ti), text(te)
    {
    }

    BoardItem(QString str)
    {
        timestamp = getXml(str, "__TIME__").toLongLong();
        text = getXml(str, "__TEXT__");
    }

    QString toString() const
    {
        return makeXml(timestamp, "__TIME__")+makeXml(text, "__TEXT__");
    }

    qint64 timestamp;
    QString text;
};

#endif // BOARDITEM_H
