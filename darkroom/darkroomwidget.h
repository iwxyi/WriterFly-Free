#ifndef DARKROOMWIDGET_H
#define DARKROOMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QSpinBox>
#include <QMenu>
#include <QAction>
#include "globalvar.h"
#include "mydialog.h"
#include "myspinbox.h"
#include "interactivebuttonbase.h"
#include "waterfloatbutton.h"
#include "digitaltimestamppredict.h"

class DarkRoomWidget : public MyDialog
{
    Q_OBJECT
public:
    DarkRoomWidget(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent* e);

signals:

public slots:
    void startDarkRoom();
    void addSuggestWordsList(QList<int> list);

private:
    void initView();
    void initMenu();
    void initData();

private:
    QLabel* desc_label;
    QLabel* word_label;
    MySpinBox* word_spin;
    QLabel* time_label;
    MySpinBox* time_spin;
    QLabel* combo_label;
    QButtonGroup* combo_group;
    QRadioButton *either_radio, *both_radio;
    QLabel* type_label;
    QButtonGroup* force_group;
    QRadioButton *warning_radio, *force_radio;
    InteractiveButtonBase* start_btn;
    QHBoxLayout* suggest_hlayout;

    QAction* suggest_words_action; // 预测字数
    QAction* record_times_action;  // 记录时间
};

#endif // DARKROOMWIDGET_H
