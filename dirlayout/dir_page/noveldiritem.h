#ifndef NOVELDIRITEM_H
#define NOVELDIRITEM_H

#include <QPoint>
#include <QDateTime>
#include "chapterinfo.h"

/**
 * ListView的一个Item实体类，包括章节信息、分组信息、展开信息、打开信息等
 */
class NovelDirItem : public ChapterInfo
{
public:
    NovelDirItem();
    NovelDirItem(bool roll, QString name);
    NovelDirItem(bool roll, QString name, int number);
    NovelDirItem(bool roll, bool hide, QString name, int number);

public:
    bool isRoll(); // 是否是卷
    bool isHide(); // 是否收起/隐藏中
    bool isAnimating();
    bool isSelecting();
    bool isNew();
    bool isInOpenAnimating();
    QPoint getClickPoint();
    QString getS_temp();

    void setHide(bool state);
    void setAnimating(bool a);
    void setS_temp(QString s);
    void setNew(bool n = true);

    void setSelecting(bool s);
    void setClickPoint(QPoint p);

    int getHoverLeaveProg100();
    int getHoverProg100();
    int getLeaveProg100();
    void setPressed(QPoint point);
    void setReleased();
    int getSelectProg100();
    int getUnselectProg100();

private:
    void initData();
    qint64 getTimestamp();

private:
    bool roll;
    bool hide;
    bool animating; // 加入动画中
    bool selecting; // 是否选中
    bool activing;  // 是否激活：选中 & 获取焦点
    QPoint click_point; // 鼠标按下的位置
    QString s_temp;
    bool is_new;

    int hover_ani_duration;
    int press_ani_duration;
    int click_ani_duration;
    int unselect_ani_duration;
    qint64 hover_timestamp;
    qint64 leave_timestamp;
    qint64 press_timestamp;
    qint64 release_timestamp;
    qint64 unselect_timestamp;
};



#endif // NOVELDIRITEM_H
