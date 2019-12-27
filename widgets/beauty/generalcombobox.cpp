#include "generalcombobox.h"

GeneralComboBox::GeneralComboBox(QWidget *parent)
    : QComboBox (parent)
{
    list_view = new GeneralListView(this);
    this->setView(list_view);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void GeneralComboBox::updateUI()
{

}
