#ifndef USERSETTINGSWINDOW_H
#define USERSETTINGSWINDOW_H

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

#define GROUPS_HEIGHT 50
#define ITEMS_HEIGHT 50
//#define SWITCH_WIDTH 50

/**
 * 用户设置窗口
 */
class UserSettingsWindow : public QFrame
{
    Q_OBJECT
public:
    UserSettingsWindow(QWidget* parent);

    void initItems();
    void addItems(QString key, QString title, QString desc, QString tip, QString v);
    void addItems(QString key, QString title, QString desc, QString tip, int v);
    void addItems(QString key, QString title, QString desc, QString tip, bool v);
    void addItems(QString key, QString title, QString btn, QString desc, QString tip, bool v);
    void addItems(QString key, QString title, QString btn, QString desc, QString tip, QString val);
    void addItems(QString key, QString title, QString btn, QString desc, QString tip, int val);
    void addItems(QString key, QString title, QString desc, QString tip, QColor c);
    void addItems(QString key, QString title, QString desc, QString tip);
    void addGroup(QString group_key, QString title, QString desc);
    void addSplitter();
    void hideItem();

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent* event);

private:
    void adjustGroupItems(); // 右边滚动时调整对应的分手

signals:
    void signalEditorBottomSettingsChanged();
    void signalDetectUpdate();
    void signalEsc();
    void signalShortcutKey();

public slots:
    void updateUI();
    void slotScrollToGroup(int x); // 分组从0开始
    void slotFocuGroup(int g);
    void slotItemClicked(UserSettingsItem* item, QString key);
    void slotBtnClicked(UserSettingsItem* item, QString key);

private:
    bool inputInt(QString title, QString desc, int def, int min, int max, int& rst);

private:
    bool _flag_group_scroll;
    QListWidget *group_list, *item_list;
    QList<int>group2item_indexs; // 每个分组的开始下标（即标题索引）
    QList<int>item2group_indexs; // 每个设置项对应的分组下标
};

#endif // USERSETTINGSWINDOW_H
