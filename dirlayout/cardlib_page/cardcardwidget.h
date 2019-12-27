#ifndef CARDCARDWIDGET_H
#define CARDCARDWIDGET_H

#include <QGraphicsDropShadowEffect>
#include "globalvar.h"
#include "interactivebuttonbase.h"

class CardCardWidget : public InteractiveButtonBase
{
	Q_OBJECT
public:
    CardCardWidget(CardBean* card, QWidget *parent = nullptr);

    CardBean* getCard() const;

    void showBrief(bool show = true);
    void refreshData();

    bool isExpanded();

private:
	void initView();
    void initStyle();
    void initEvent();

signals:
	void signalExpanded(bool e);

private:
	CardBean * card;
    QLabel* name_label;
    QLabel* brief_label;
};

#endif // CARDCARDWIDGET_H
