#ifndef USERSETTINGSWINDOW2_H
#define USERSETTINGSWINDOW2_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QColor>
#include <QPalette>
#include <QList>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QDesktopServices>
#include "globalvar.h"
#include "aniswitch.h"
#include "usersettingsitem.h"
#include "fileutil.h"
#include "stringutil.h"
#include "homonymcoverwindow.h"
#include "dynamicbackgroundsettings.h"

class UserSettingsWindow2 : public QWidget
{
    Q_OBJECT
public:
    UserSettingsWindow2(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent* event);

private:
	void initGroups();
    void initItems(QString g = "常用");
    void addOneItem(SettingBean* sb, bool hide = false);
    void addSeperator(int count);
    void setItemLayout(WaterZoomButton *btn, SettingBean* sb);
    void refreshItemValue(SettingBean *sb);
    void startGroupSwitchAnimation(QPixmap pixmap_old, QPixmap pixmap_new, int index);

    bool inputInt(QString title, QString desc, int def, int min, int max, int& rst);

signals:
    void signalEditorBottomSettingsChanged();
    void signalDetectUpdate();
    void signalEsc();
    void signalShortcutKey();

public slots:
    void updateUI();
    void slotTriggerSettingItem(QString key);
    void slotTriggerSettingItem(SettingBean* sb);

private:
    QWidget* bg_widget;
    QListWidget *group_list, *item_list;
    QList<InteractiveButtonBase*> group_btns, item_btns, item_child_btns;
    QList<SettingBean*> item_beans;
    QList<void*> item_value_widgets;
    int current_group_index;
};

#endif // USERSETTINGSWINDOW2_H
