#ifndef OUTLINESETTINGS_H
#define OUTLINESETTINGS_H

#include "settings.h"
#include "globalvar.h"

/**
 * 大纲设置读取与存储类
 */
class OutlineSettings : Settings
{
public:
    OutlineSettings() : Settings(""){}

private:
    QString novel_name;       // 书籍名称
    Settings* s;
};

#endif // OUTLINESETTINGS_H
