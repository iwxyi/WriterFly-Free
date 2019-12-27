#ifndef DIRSETTINGSWIDGET_H
#define DIRSETTINGSWIDGET_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QShowEvent>
#include <QLineEdit>
#include <QRegExp>
#include "aniswitch.h"
#include "mylabel.h"
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"
#include "noveldirsettings.h"

/**
 * 目录设置界面类
 */
class DirSettingsWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DirSettingsWidget(QWidget *parent = nullptr);
    void setNS(NovelDirSettings* ns);

protected:
	void showEvent(QShowEvent* );

private:
	void initLayout();
	void initData();
    void setVal(QString key, QVariant val);

signals:
    void signalDirSettingsChanged();

public slots:
    void toShow(QString n, QPoint p);
    void toHide();

private:
	NovelDirSettings* ns;
    QString novel_name;
    QPoint callback_point;

    QLabel*    novel_name_edit;
    QLineEdit* start_chapter_edit;
    QLineEdit* start_roll_edit;
    QLineEdit* char_chapter_edit;
    QLineEdit* char_roll_edit;
    AniSwitch* show_chapter_num_switch;
    AniSwitch* show_roll_num_switch;
    AniSwitch* show_chapter_words_switch;
//    AniSwitch* chapter_preview_switch;
    AniSwitch* use_arab_switch;
    AniSwitch* recount_num_by_roll_switch;
    AniSwitch* no_roll_switch;
};

#endif // DIRSETTINGSWIDGET_H
