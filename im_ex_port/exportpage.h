#ifndef EXPORTPAGE_H
#define EXPORTPAGE_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QShowEvent>
#include "aniswitch.h"
#include "mylabel.h"
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"

/**
 * 导出界面
 */
class ExportPage : public QWidget
{
    Q_OBJECT
public:
    explicit ExportPage(QWidget *parent = 0);

private:
	void initLayout();
	void initData();

signals:
    void signalExport();

public slots:
	void slotExportFinished();

protected:
	void showEvent(QShowEvent*);
	
private:
    QPushButton* submit_button;
};

#endif // EXPORTPAGE_H
