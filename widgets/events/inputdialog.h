#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScrollBar>
#include "mylabel.h"
#include "stringutil.h"
#include "fileutil.h"

class InputDialog : public QDialog
{
    Q_OBJECT
public:
    InputDialog(QWidget *parent, QString title, QString filepath = "", QString tip = "", QString long_tip = "");
    InputDialog(QWidget* parent, QString title, QString content, QString* result, QString tip = "", QString long_tip = "");

    void initView();
    void initData();

public slots:
    void slotTip();
    void slotSave();

private:
    MyLabel* tip_label;
    QPlainTextEdit* edit;
    QPushButton* save_button;

    QString title;
    QString filepath;
    QString content;
    QString tip;
    QString long_tip;
    QString* result;
};

#endif // INPUTDIALOG_H
