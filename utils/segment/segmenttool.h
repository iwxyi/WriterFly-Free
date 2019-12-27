#ifndef SEGMENTTOOL_H
#define SEGMENTTOOL_H

#include <QStringList>

class SegmentTool
{
public:
    virtual ~SegmentTool(){}
    virtual bool init() = 0;
    virtual QStringList WordSegment(QString) = 0;
};

#endif // SEGMENTTOOL_H
