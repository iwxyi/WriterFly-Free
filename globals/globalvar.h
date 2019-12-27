/**
 * 全局变量
 */

#ifndef GLOBALVAR_H
#define GLOBALVAR_H

#include <QCoreApplication>
#include <QApplication>
#include "settings.h"
#include "usettings.h"
#include "frisoutil.h"
#include "lexicon.h"
#include "runtimeinfo.h"
#include "apptheme.h"
#include "globaldata.h"
#include "useraccount.h"

// ==== 全局宏定义 ====
extern bool DEBUG_MODE;
extern bool DEGBU_DEEP;

// ==== 应用 ====
extern QString APPLICATION_NAME;
extern QString VERSION_CODE;
extern int     VERSION_NUMBER;

extern RuntimeInfo* rt;
extern USettings* us;
extern AppTheme* thm;
extern UserAccount* ac;
extern GlobalData* gd;

extern bool deb(QVariant str, QString name);
extern bool deb(QVariant str);
extern QString log(QVariant str);
extern QString log(QString title, QVariant str);
extern QString log2(QVariant str);
extern QString log2(QString title, QVariant str);
extern QString err(QString title, QVariant str);
extern QString Path(QString str);
extern qint64 getTimestamp();

void initGlobal();
void deleteGlobal();

#endif // GLOBALVAR_H
