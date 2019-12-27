#ifndef CARDLIBGROUP_H
#define CARDLIBGROUP_H

#include <QListWidget>
#include <QInputDialog>
#include <QTimer>
#include "generalcombobox.h"
#include "watercirclebutton.h"
#include "waterfloatbutton.h"
#include "anitabwidget.h"
#include "cardlistwidget.h"
#include "selecteditingcardsthread.h"

class CardlibGroup : public QWidget
{
    Q_OBJECT
public:
    CardlibGroup(QWidget *parent = nullptr);

    void refreshCardlibList(QString recent = "");
    void refreshCardLists();

    QList<CardCardWidget*> getCardWidgets(CardBean* card);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    void initView();
    void initStyle();
    void initData();

signals:
    void signalOpenCardlib(QString name);
    void signalNeedRefreshEditingCards();
    void signalAddNewCard();
    void signalShowCardEditor(CardBean *card);

public slots:
    void updateUI();
    bool switchCardlib(int index);
    bool createCardlib();

    void slotFilterClicked();
    void slotAddCardClicked();
    void slotExpandClicked();

    void slotRefreshEditingCards(QTextEdit *editor);

    void actionRefresh();
    void actionTotal();
    void actionExpand();
    bool actionRename();
    bool actionDelete();

private:
    GeneralComboBox *cardlibs_list_combo;
    InteractiveButtonBase *filter_btn;
    InteractiveButtonBase *add_card_btn;
    InteractiveButtonBase* expand_btn;
    QWidget *divider_line;

    AniTabWidget* tab_widget;
    CardListWidget* editing_list_widget;
    CardListWidget* using_list_widget;
    CardListWidget* total_list_widget;

    CardlibManager &clm;
};

#endif // CARDLIBGROUP_H
