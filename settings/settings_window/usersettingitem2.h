#ifndef USERSETTINGITEM2_H
#define USERSETTINGITEM2_H

#include <QObject>
#include "interactivebuttonbase.h"
#include "aniswitch.h"
#include "aninumberlabel.h"
#include "anicirclelabel.h"


class UserSettingItem2 : public QObject
{
    Q_OBJECT
public:
    UserSettingItem2(SettingBean* sb, QWidget* parent = nullptr) : QObject(parent)
    {

    }


public:
    SettingBean* sb;
};

#endif // USERSETTINGITEM2_H
