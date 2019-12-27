#ifndef SETTINGSBEAN_H
#define SETTINGSBEAN_H

#include <QString>
#include <QColor>
#include <QVariant>
#include <QList>
#include <QMap>
#include <functional>

enum SettingValueType
{
    SVT_UNKNOW,
    SVT_BOOL,
    SVT_INTEGER,
    SVT_DOUBLE,
    SVT_STRING,
    SVT_COLOR,
    SVT_QINT64,
};

class SettingBean
{
public:
    SettingBean(QString key, QString title, QString desc, QString tooltip, void *point)
        : key(key), title(title), description(desc), tooltip(tooltip), point(point), isChild(false), isHidden(false),
        min(0), max(100), need_restart(false)
    {
    }

    SettingValueType getType()
    {
        if (type == SVT_INTEGER && shows.size()>0)
            return SVT_STRING;
        return type;
    }

    QVariant getValue()
    {
        switch (type)
        {
        case SVT_BOOL:
            return *(static_cast<bool *>(point));
        case SVT_INTEGER:
            if (shows.size() == 0)
                return *(static_cast<int *>(point));
            else
                return shows.at(*(static_cast<int *>(point)));
        case SVT_DOUBLE:
            return *(static_cast<double *>(point));
        case SVT_STRING:
            return *(static_cast<QString *>(point));
        case SVT_COLOR:
            return *(static_cast<QColor *>(point));
        case SVT_QINT64:
            return *(static_cast<qint64 *>(point));
        default:
            return QVariant();
        }
    }

    void addRelated(SettingBean* sb)
    {
        relates << sb;
        sb->isChild = true;
    }

    void setRange(int min, int max)
    {
        this->min = min;
        this->max = max;
    }

    void setRestart()
    {
        this->need_restart = true;
    }

public:
    const QString key;     // 全局唯一key
    QString group;   // 标题
    const QString title;   // 标题
    QString description;   // 描述
    QString tooltip;       // 鼠标悬浮提示
    SettingValueType type; // 值种类
    void *point;           // 指向真实数据的指针

    int min, max;
    bool need_restart;

    bool isChild;
    QList<SettingBean*> relates; // 相关的设置，变成按钮形式
    bool isHidden;

    QStringList shows;
};

#endif // SETTINGSBEAN_H
