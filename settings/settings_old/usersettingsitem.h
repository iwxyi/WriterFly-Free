#ifndef USERSETTINGSITEM_H
#define USERSETTINGSITEM_H

#include <QObject>
#include <QWidget>
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
#include <QMouseEvent>
#include "anicirclelabel.h"
#include "aninumberlabel.h"
#include "globalvar.h"
#include "aniswitch.h"
#include "waterzoombutton.h"

#define GROUPS_HEIGHT 50
#define ITEMS_HEIGHT 50
#define SWITCH_WIDTH 40

// #define DESC_STYLESHEET "color: rgb(100,100,100)"

/**
 * 设置窗口的Item控件
 */
class UserSettingsItem : public QWidget
{
	Q_OBJECT
public:
    UserSettingsItem(QWidget* parent);
    UserSettingsItem(QWidget* parent, QString key);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString desc, QString tip, QString v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString desc, QString tip, int v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString desc, QString tip, bool v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString btn, QString desc, QString tip, bool v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString btn, QString desc, QString tip, QString v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString btn, QString desc, QString tip, int v);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString desc, QString tip, QColor c);
    UserSettingsItem(QWidget* parent, QString key, QString title, QString desc, QString tip);
    UserSettingsItem(QWidget* parent, QString group_key, QString title, QString desc); // 分组
    UserSettingsItem(QWidget* parent, int kind);

    void setVal(QString v);
    void setVal(int     v);
    void setVal(bool    v);
    void setVal(QColor  v);
    void setBtn(QString s);
    void setDesc(QString s);

    void setGroup(int g);
    int getGroup();
    int getHeight();

protected:
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void enterEvent(QEvent* e);

signals:
    void signalMouseEntered(int x);
    void signalClicked(UserSettingsItem* item, QString key);
    void signalBtnClicked(UserSettingsItem* item, QString key);

public slots:
    void updateUI();

private:
    QLabel* lb_title = nullptr;
    AniSwitch* as = nullptr;
    QLabel* lb_desc = nullptr;
    QLabel* lb_val = nullptr;
    AniCircleLabel* rb_color = nullptr;
    AniNumberLabel* nb_val = nullptr;
    QPushButton* ev_btn = nullptr;
	QString key;

    QString DESC_STYLESHEET;

    int group;
    int item_height;

    QPoint press_point;
};

#endif // USERSETTINGSITEM_H
