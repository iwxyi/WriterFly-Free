#include "settingsmanager.h"

SettingsManager::SettingsManager(RuntimeInfo* rt, USettings* us, QObject *parent) : QObject(parent), rt(rt), us(us), _cur_setting(nullptr), _pre_setting(nullptr)
{
    initItems();
}

/**
 * 通过设置项对象来触发设置项修改事件（重载）
 */
bool SettingsManager::trigger(SettingBean *sb, QWidget* widget)
{
    return trigger(sb->key, widget);
}

QList<SettingBean *> SettingsManager::getItems(QString g)
{
    if (g.isEmpty())
        return items;

    initItems(g);
    QList<SettingBean*>gitems;
    foreach (SettingBean* sb, items)
    {
        if (sb->group == g)
            gitems << sb;
    }
    return gitems;
}

void SettingsManager::setGroup(QString name)
{
    group_name = name;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, nullptr);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_UNKNOW;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, bool *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_BOOL;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, int *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_INTEGER;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, int *point, QStringList shows)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_INTEGER;
    _cur_setting->shows = shows;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, double *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_DOUBLE;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, QString *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_STRING;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, QColor *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_COLOR;
    items.append(_cur_setting);
    return _cur_setting;
}

SettingBean* SettingsManager::addItem(QString key, QString title, QString desc, QString tooltip, qint64 *point)
{
	_pre_setting = _cur_setting;
    _cur_setting = new SettingBean(key, title, desc, tooltip, point);
    _cur_setting->group = group_name;
    _cur_setting->type = SVT_QINT64;
    items.append(_cur_setting);
    return _cur_setting;
}

void SettingsManager::addItemChild(SettingBean* sb)
{
	// Q_ASSERT (_pre_setting != nullptr);
    if (_pre_setting == nullptr)
        return;
    _pre_setting->addRelated(sb);
}

void SettingsManager::setItemRange(int min, int max)
{
    if (_cur_setting == nullptr)
        return ;
    _cur_setting->setRange(min, max);
}

void SettingsManager::setItemRestart()
{
    if (_cur_setting == nullptr)
        return ;
    _cur_setting->setRestart();
}

void SettingsManager::setItemHideen()
{
    if (_cur_setting == nullptr)
        return ;
    _cur_setting->isHidden = true;
}
