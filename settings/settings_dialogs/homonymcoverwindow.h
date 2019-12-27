#ifndef HOMONYMCOVERWIDGET_H
#define HOMONYMCOVERWIDGET_H

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
#include "mylabel.h"
#include "globalvar.h"
#include "stringutil.h"

class HomonymCoverWindow : public QDialog
{
    Q_OBJECT
public:
    explicit HomonymCoverWindow(QWidget *parent = nullptr);

    void initView();
    void initData();

public slots:
    void slotTip();
    void slotSave();

private:
    MyLabel* tip_label;
    QPlainTextEdit* edit;
    QPushButton* save_button;
};

#endif // HOMONYMCOVERWIDGET_H
