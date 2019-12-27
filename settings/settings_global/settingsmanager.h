#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QDebug>
#include "settingbean.h"
#include "usettings.h"
#include "runtimeinfo.h"

class QWidget;

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    SettingsManager(RuntimeInfo* rt, USettings* us, QObject *parent = nullptr);

    bool trigger(QString key, QWidget* widget = nullptr);      // 激活某一个设置项
    bool trigger(SettingBean* sb, QWidget* widget = nullptr);  // 激活某一个设置项

    QList<SettingBean*> getItems(QString g = "");

private:
    void initItems(QString g = "");

    void setGroup(QString name);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, bool* point);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, int* point);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, int* point, QStringList shows);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, double* point);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, QString* point);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, QColor* point);
    SettingBean* addItem(QString key, QString title, QString desc, QString tooltip, qint64* point);
    void addItemChild(SettingBean* sb);
    void setItemRange(int min, int max);
    void setItemRestart();
    void setItemHideen();

signals:
    void signalTriggered(QString key);

public slots:

private:
    RuntimeInfo* rt;
    USettings* us;
    QList<SettingBean*>items;
    QString group_name;

    SettingBean* _cur_setting;
    SettingBean* _pre_setting;
};

#endif // SETTINGSMANAGER_H
