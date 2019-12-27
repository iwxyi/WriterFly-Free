#ifndef CARDLISTWIDGET_H
#define CARDLISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include "globalvar.h"
#include "cardcardwidget.h"
#include "cardeditor.h"

class CardListWidget : public QListWidget
{
	Q_OBJECT
public:
    CardListWidget(QWidget *parent = nullptr);

    void loadCards(CardList cards);
    void addOneCard(CardBean *card);
    void removeCard(CardBean *card);
    void reloadCard(CardBean *card);

    CardCardWidget* getCardWidget(CardBean *card);
    CardList getCards();
    void expandOrShrink();

private:
	void initView();

    CardBean* currentCard();

signals:
	void signalAllExpanded(bool e);
    void signalShowCardEditor(CardBean *card);
    void signalRefreshAll();

public slots:
	void updateUI();
    void slotShowItemMenu(QPoint p);

    void actionModifyCard();
    void actionRefreshCard();
    void actionColorCard();
    void actionUnuseCard();
    void actionDisableCard();
    void actionJumpAllAppear();
    void actionJumpAppear();
    void actionJumpDisappear();

private:
	int card_height;
    bool expand;

    QMenu* card_menu;
    QAction* modify_action;
    QAction* refresh_action;
    QAction* color_action;
    QAction* unuse_action;
    QAction* disable_action;
    QMenu* jump_menu;
    QAction* all_appear_action;
    QAction* appear_action;
    QAction* disappear_action;
};

#endif // CARDLISTWIDGET_H
