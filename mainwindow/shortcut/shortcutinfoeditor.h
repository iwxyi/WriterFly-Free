#ifndef SHORTCUTINFOEDITOR_H
#define SHORTCUTINFOEDITOR_H

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include "mydialog.h"
#include "globalvar.h"
#include "waterfloatbutton.h"

class ShortcutInfoEditor : public MyDialog
{
	Q_OBJECT
public:
    ShortcutInfoEditor(ShortcutEntry& se, QWidget* parent = nullptr);
    ShortcutInfoEditor(ShortcutEntry& se, int focus, QWidget* parent = nullptr);

    void setAddModel();
    void setFocusPlace(int x);

protected:
	void showEvent(QShowEvent* e);

private:
	void initView();
    void initData();

public slots:
    void slotModifyClicked();

private:
    ShortcutEntry& se;

    QLabel *cmd_label;
    QLabel *key_label;
    QLabel *env_label;
    QLabel *when_label;
    QPlainTextEdit *cmd_edit;
    QLineEdit *key_edit;
    QPlainTextEdit *when_edit;
    QComboBox *env_cb;
    InteractiveButtonBase *modify_btn;

    int focus_place;
};

#endif // SHORTCUTINFOEDITOR_H
