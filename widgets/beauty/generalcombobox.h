#ifndef GENERALCOMBOBOX_H
#define GENERALCOMBOBOX_H

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include "globalvar.h"
#include "generallistview.h"

class GeneralComboBox : public QComboBox
{
    Q_OBJECT
public:
    GeneralComboBox(QWidget* parent);

public slots:
    virtual void updateUI();

private:
    GeneralListView* list_view;
};

#endif // GENERALCOMBOBOX_H
