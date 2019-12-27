#ifndef THEMEMANAGEWINDOW_H
#define THEMEMANAGEWINDOW_H

#include <QObject>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "globalvar.h"
#include "mydialog.h"
#include "interactivebuttonbase.h"
#include "waterfloatbutton.h"

class ThemeManageWindow : public MyDialog
{
    Q_OBJECT
public:
    explicit ThemeManageWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent* e);

private:
	void initView();
	void initList();
	void initMenu();

    QString getRandomThemeName();
signals:

public slots:
    void slotItemClicked(QListWidgetItem* item);
    void OnListContextMenu(const QPoint& point);
    void saveCurrentTheme();

    void actionOpenExplore();
    void actionRename();
    void actionDelete();

private:
    InteractiveButtonBase* save_btn;
	QListWidget* theme_list;

	QMenu* menu;
	QAction* locate_action;
	QAction* rename_action;
	QAction* delete_action;
};

#endif // THEMEMANAGEWINDOW_H
