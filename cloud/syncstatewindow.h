#ifndef SYNCSTATEWINDOW_H
#define SYNCSTATEWINDOW_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QListView>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "globalvar.h"
#include "syncsequeuemodel.h"
#include "syncfinishedqueuemodel.h"
#include "syncfinishedsequeuedelegate.h"
#include "interactivebuttonbase.h"
#include "anitabwidget.h"
#include "mydialog.h"

class SyncStateWindow : public MyDialog
{
    Q_OBJECT
public:
    explicit SyncStateWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent* event);

private:
	void initView();
	void initData();
    void initMenu();
    void refreshMenu();

signals:

public slots:
    void actionModifyNickname();
    void actionModifyUsername();
    void actionModifyPassword();
    void actionSyncNovels();
    void actionLogout();

private:
    AniTabWidget* tab_widget;
	QListView* download_listview;
	QListView* upload_listview;
    QListView* finished_listview;

    SyncSequeueModel* download_model;
    SyncSequeueModel* upload_model;
    SyncFinishedSequeueModel* finished_model;
    SyncFinishedSequeueDelegate* finished_delegate;

    QMenu* sync_menu;
    QAction* modify_nickname_action;
    QAction* modify_username_action;
    QAction* modify_password_action;
    QAction* sync_novels_action;
    QAction* logout_action;
};

#endif // SYNCSTATEWINDOW_H
