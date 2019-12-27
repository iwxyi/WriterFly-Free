#ifndef GENERALLISTVIEW_H
#define GENERALLISTVIEW_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QScrollBar>
#include "globalvar.h"

class GeneralListView : public QListView
{
    Q_OBJECT
public:
    GeneralListView(QWidget* parent = nullptr);

public slots:
    virtual void updateUI();
};

#endif // GENERALLISTVIEW_H
