#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <QObject>
#include "runtimeinfo.h"
#include "usettings.h"
#include "lexicon.h"
#include "boardmanager.h"
#include "segmenttool.h"
#include "darkroom.h"
#include "shortcutkeymanager.h"
#include "cardlibmanager.h"
#include "settingsmanager.h"
#include "predictbooks.h"

class GlobalData : public QObject
{
public:
    GlobalData(RuntimeInfo* rt, USettings* us, UserAccount* ac) :
        sm(rt, us, this),
        boards(rt->DATA_PATH),
        lexicons(rt->DATA_PATH+"lexicon/"),
        participle(nullptr),
        dr(rt->HISTORY_PATH, ac),
        shortcuts(rt->DATA_PATH+"shortcuts.xml"),
        clm(rt, this),
        pb(lexicons.getStopWords(), rt->DBASE_PATH+"books/", this)
    {

    }

    ~GlobalData()
    {
        if (participle != nullptr)
            delete participle;
        /* // 非指针不用析构，否则会出错的
        boards.deleteLater();
        dr.deleteLater();
        1clm.deleteLater();*/
    }

    SettingsManager sm;
    BoardManager boards;
    Lexicons lexicons;
    SegmentTool* participle;
    DarkRoom dr;
    ShortcutKeyManager shortcuts;
    CardlibManager clm;
    PredictBooks pb;
};

#endif // GLOBALDATA_H
