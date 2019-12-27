#ifndef IMPORTPAGE_H
#define IMPORTPAGE_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QShowEvent>
#include <QFileInfo>
#include <QLineEdit>
#include "noveldirlistview.h"
#include "aniswitch.h"
#include "mylabel.h"
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"

/**
 * 导入界面
 */
class ImportPage : public QWidget
{
    Q_OBJECT
public:
    explicit ImportPage(QWidget *parent = 0);
    void initLayout();
    void initData();
    void setNovelName(QString name);
    QString getTargetName();
    void setNovelDirListView(NovelDirListView* listview);

protected:
	void showEvent(QShowEvent*);
	void resetTargetCombo();
	void resetRollCombo();
    void resetCodecCombo();
	QString simplifyChapter(QString chpt);

private:
	bool isBlankChar(QString c);

signals:
	void signalImport(QString file_path, bool split_roll, int roll_index);
	void signalImportFinished(QString novel_name);

public slots:
	void slotImportFinished(QString name);
	void slotStartImport();

private:
	QString novel_name;
	NovelDirListView* listview;

	QString file_path, file_name;
	QString target_path, target_name;

    bool add_new_novel;
	bool auto_split_roll;
    QString codec;
	QString roll_reg;
	QString chpt_reg;
	QString meaningless_reg;

	QPushButton* path_button;
	QComboBox* target_combo;
	QComboBox* roll_combo;
    QComboBox* codec_combo;
	QPushButton* submit_button;
};

#endif // IMPORTPAGE_H
