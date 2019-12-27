#include "noveldiritem.h"

NovelDirItem::NovelDirItem() : hide(false)
{
    initData();
}

NovelDirItem::NovelDirItem(bool roll, QString name)
        : ChapterInfo(name), roll(roll), hide(false)
{
    initData();
}

NovelDirItem::NovelDirItem(bool roll, QString name, int number)
        : ChapterInfo(name, number), roll(roll), hide(false)
{
    initData();
}

NovelDirItem::NovelDirItem(bool roll, bool hide, QString name, int number)
        : ChapterInfo(name, number), roll(roll), hide(hide)
{
    initData();
}

void NovelDirItem::initData()
{
    animating = false;
    selecting = false;
    click_point = QPoint(-1, -1);
    is_new = false;

    hover_ani_duration = 200;
    press_ani_duration = 500;
    click_ani_duration = 300;
    unselect_ani_duration = 500;
    hover_timestamp = leave_timestamp = press_timestamp = release_timestamp = unselect_timestamp = 0;
}

bool NovelDirItem::isRoll()
{
    return roll;
}

bool NovelDirItem::isHide()
{
    return hide;
}

bool NovelDirItem::isAnimating()
{
    return animating;
}

bool NovelDirItem::isSelecting()
{
    return selecting;
}

bool NovelDirItem::isNew()
{
    if (is_new)
    {
        is_new = false;
        return true;
    }
    return false;
}

bool NovelDirItem::isInOpenAnimating()
{
    if (press_timestamp == 0) return false;

    if (release_timestamp == 0) // 一直按住，或许动画完了，但至少应该聚焦
    {
        return true;
    }
    else // 已经松手了
    {
        int prog = 100 * (release_timestamp - press_timestamp) / press_ani_duration
             + 100 * (getTimestamp() - release_timestamp) / click_ani_duration;
        return prog <= 100;
    }
}

QPoint NovelDirItem::getClickPoint()
{
    return click_point;
}

void NovelDirItem::setHide(bool state)
{
    hide = state;
}

void NovelDirItem::setAnimating(bool a)
{
    animating = a;
}

void NovelDirItem::setS_temp(QString s)
{
    s_temp = s;
}

void NovelDirItem::setSelecting(bool s)
{
    if (selecting && s) // 已经选中了
        return ;
    selecting = s;
    if (s) // 选中
    {
        unselect_timestamp = 0;
    }
    else // 取消选中
    {
        if (getTimestamp() > unselect_timestamp + unselect_ani_duration)
            unselect_timestamp = getTimestamp();
    }
}

void NovelDirItem::setClickPoint(QPoint p)
{
    click_point = p;
}

void NovelDirItem::setNew(bool n)
{
    is_new = n;
}

QString NovelDirItem::getS_temp()
{
    return s_temp;
}

qint64 NovelDirItem::getTimestamp()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

int NovelDirItem::getHoverLeaveProg100()
{
    if (hover_timestamp == 0)
        return 0;
    if (leave_timestamp == 0) // 一直悬浮，没有离开
    {
        int prog = 100 * (getTimestamp() - hover_timestamp) / hover_ani_duration;
        if (prog > 100) prog = 100;
        return prog;
    }
    else // 已经离开了
    {
        int max_prog = 100 * (leave_timestamp - hover_timestamp) / hover_ani_duration;
        if (max_prog > 100) max_prog = 100;

        int prog = max_prog * (getTimestamp() - leave_timestamp) / hover_ani_duration;
        if (prog > 100) prog = 100;
        return 100 - prog;
    }
}

int NovelDirItem::getHoverProg100()
{
    if (hover_timestamp == 0 || leave_timestamp != 0) // 如果没有 hover
    {
        hover_timestamp = getTimestamp();
        leave_timestamp = 0;
    }

    if (getTimestamp() - hover_timestamp >= hover_ani_duration) // 动画结束
        return 100;

    int prog = 100 * (getTimestamp() - hover_timestamp) / hover_ani_duration;
    if (prog > 100) prog = 100;
    return prog;
}

int NovelDirItem::getLeaveProg100()
{
    if (hover_timestamp == 0)
        return 0;
    if (leave_timestamp == 0)
        leave_timestamp = getTimestamp();

    if (getTimestamp() - leave_timestamp >= hover_ani_duration) // 动画结束
        return leave_timestamp = hover_timestamp = 0;

    int max_prog = 100 * (leave_timestamp - hover_timestamp) / hover_ani_duration;
    if (max_prog > 100) max_prog = 100;

    int prog = max_prog * (getTimestamp() - leave_timestamp) / hover_ani_duration;
    if (prog >= max_prog)
    {
        prog = max_prog;
        hover_timestamp = leave_timestamp = 0;
    }
    return max_prog - prog;
}

void NovelDirItem::setPressed(QPoint point)
{
    press_timestamp = getTimestamp();
    release_timestamp = 0;
    unselect_timestamp = 0;
    click_point = point;
}

void NovelDirItem::setReleased()
{
    if (press_timestamp == 0) return ;
    release_timestamp = getTimestamp();
}

int NovelDirItem::getSelectProg100()
{
    if (press_timestamp == 0) return 0;

    qint64 timestamp = (unselect_timestamp == 0 ? getTimestamp() : unselect_timestamp);
    int prog = 0;
    if (( press_timestamp && timestamp >= press_timestamp + press_ani_duration )
            || ( release_timestamp && timestamp >= release_timestamp + click_ani_duration))
    {
        return 100;
    }
    else if (release_timestamp == 0) // 一直按住
    {
        prog = 100 * (timestamp - press_timestamp) / press_ani_duration;
    }
    else // 已经松手了
    {
        prog = 100 * (release_timestamp - press_timestamp) / press_ani_duration
             + 100 * (timestamp - release_timestamp) / click_ani_duration;
    }
    if (prog > 100) prog = 100;
    return prog;
}

int NovelDirItem::getUnselectProg100()
{
    if (press_timestamp == 0 || release_timestamp == 0 || unselect_timestamp == 0)
        return 0;

    qint64 timestamp = getTimestamp();
    int prog = 100 * (timestamp - unselect_timestamp) / unselect_ani_duration;
    if (prog == 0) // 开始动画，避免中间的闪跳
        prog = 1;
    else if (prog >= 100) // 动画结束
    {
        prog = 100;
        press_timestamp = release_timestamp = unselect_timestamp = 0;
    }
    return prog;
}
