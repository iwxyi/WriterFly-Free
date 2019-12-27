#include "cardlistwidget.h"

CardListWidget::CardListWidget(QWidget *parent) : QListWidget(parent), expand(false)
{
    initView();

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void CardListWidget::initView()
{
    QFontMetrics fm(font());
    card_height = fm.lineSpacing()*4;

    setSpacing(5); // 设置每个item的四周间隔
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // setFocusPolicy(Qt::NoFocus); // 去掉虚线框（不过好像本来就没有啊）
    thm->setWidgetStyleSheet(this, "cardlib_list");
    thm->setWidgetStyleSheet(this->verticalScrollBar(), "scrollbar");

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowItemMenu(QPoint)));

    // 初始化菜单
    card_menu = new QMenu(this);
    modify_action = new QAction(thm->icon("menu/modify"), "查看", this);
    refresh_action = new QAction(thm->icon("menu/conversion"), "刷新", this);
    color_action = new QAction(thm->icon("color"), "颜色", this);
    unuse_action = new QAction(thm->icon("menu/sensitive"), "封存", this);
    disable_action = new QAction(thm->icon("menu/disable"), "禁用", this);
    jump_menu = new QMenu("跳转", this);
    all_appear_action = new QAction(thm->icon("menu/all_appear"), "所有出现", this);
    appear_action = new QAction(thm->icon("menu/appear"), "首次登场", this);
    disappear_action = new QAction(thm->icon("menu/disappear"), "最终退场", this);
    card_menu->addAction(refresh_action);
    card_menu->addSeparator();
    card_menu->addAction(modify_action);
    card_menu->addAction(color_action);
    card_menu->addSeparator();
    card_menu->addMenu(jump_menu);
    jump_menu->setIcon(thm->icon("menu/cardlib"));
    {
        jump_menu->addAction(all_appear_action);
        jump_menu->addAction(appear_action);
        jump_menu->addAction(disappear_action);
    }
    card_menu->addSeparator();
    card_menu->addAction(unuse_action);
    card_menu->addAction(disable_action);

    all_appear_action->setEnabled(false);
    appear_action->setEnabled(false);
    disappear_action->setEnabled(false);

    connect(modify_action, SIGNAL(triggered()), this, SLOT(actionModifyCard()));
    connect(refresh_action, SIGNAL(triggered()), this, SLOT(actionRefreshCard()));
    connect(color_action, SIGNAL(triggered()), this, SLOT(actionColorCard()));
    connect(unuse_action, SIGNAL(triggered()), this, SLOT(actionUnuseCard()));
    connect(disable_action, SIGNAL(triggered()), this, SLOT(actionDisableCard()));
    connect(all_appear_action, SIGNAL(triggered()), this, SLOT(actionJumpAllAppear()));
    connect(appear_action, SIGNAL(triggered()), this, SLOT(actionJumpAppear()));
    connect(disappear_action, SIGNAL(triggered()), this, SLOT(actionJumpDisappear()));
}

void CardListWidget::updateUI()
{
	QColor c = us->mainwin_bg_color;
    if (us->mainwin_sidebar_color.alpha() <= 64)
    {
        c = us->mainwin_sidebar_color;
        c.setAlpha(255);
    }
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem* item = this->item(i);
        QWidget* widget = this->itemWidget(item);
        CardCardWidget* ccw = static_cast<CardCardWidget*>(widget);
        ccw->setBgColor(c);
    }

    thm->setWidgetStyleSheet(this->card_menu, "menu");
}

void CardListWidget::loadCards(CardList cards)
{
    // 先清空列表widget，避免内存泄漏
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem* item = this->item(i);
        QWidget* w = this->itemWidget(item);
        w->deleteLater();
    }
    this->clear();

    foreach (CardBean* card, cards)
    {
        addOneCard(card);
    }
}

void CardListWidget::addOneCard(CardBean* card)
{
    QListWidgetItem* item = new QListWidgetItem(this);
    // item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(0, card_height));
    CardCardWidget* ccw = new CardCardWidget(card, this);
    setItemWidget(item, ccw);

    ccw->showBrief(expand || card->expand);
    connect(ccw, &InteractiveButtonBase::doubleClicked, [=]{
        emit signalShowCardEditor(card);
        /* CardEditor* ce = new CardEditor(card, this);
        connect(ce, &CardEditor::signalCardModified, [=](CardBean * card){
            ccw->refreshData();
        });
        ce->exec(); */
    });

    /*connect(ccw, &CardCardWidget::signalExpanded, [=](bool e){
        // 判断和全局设置是否一致
        if (expand)
            return ;

        // 判断是否全部都是 expanded
        bool ex = e;
        for (int i = 0; i < count(); ++i)
        {
           QListWidgetItem* item = this->item(i);
           CardCardWidget* ccw = static_cast<CardCardWidget*>(itemWidget(item));
           if (ccw->isExpanded() != ex)
               return ;
        }

        // 如果和用户设置的不一样，则修改用户设置
        this->expand = e;
    });*/
}

void CardListWidget::removeCard(CardBean* card)
{
    // 先清空列表widget，避免内存泄漏
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem* item = this->item(i);
        QWidget* w = this->itemWidget(item);
        CardCardWidget* ccw = static_cast<CardCardWidget*>(w);
        if (ccw->getCard() == card)
        {
            this->takeItem(i);
            delete item;
            ccw->deleteLater();
            break;
        }
    }
}

void CardListWidget::reloadCard(CardBean *card)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem *item = this->item(i);
        QWidget *w = this->itemWidget(item);
        CardCardWidget *ccw = static_cast<CardCardWidget *>(w);
        if (!card || ccw->getCard() == card)
        {
            ccw->refreshData();
        }
    }
}

CardCardWidget* CardListWidget::getCardWidget(CardBean* card)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem *item = this->item(i);
        QWidget *w = this->itemWidget(item);
        CardCardWidget *ccw = static_cast<CardCardWidget *>(w);
        if (ccw->getCard() == card)
        {
            return ccw;
        }
    }
    return nullptr;
}

CardList CardListWidget::getCards()
{

}

void CardListWidget::expandOrShrink()
{
	bool all_expanded = true;
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem *item = this->item(i);
        QWidget *w = this->itemWidget(item);
        CardCardWidget *ccw = static_cast<CardCardWidget *>(w);
        if (!ccw->getCard()->expand)
        {
            all_expanded = false;
            break;
        }
    }

    bool expand = !all_expanded;
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem *item = this->item(i);
        QWidget *w = this->itemWidget(item);
        CardCardWidget *ccw = static_cast<CardCardWidget *>(w);
        ccw->showBrief(expand);
    }
}

CardBean*  CardListWidget::currentCard()
{
    if (this->currentRow() == -1)
        return nullptr;
    QListWidgetItem* item = this->item(currentRow());
    QWidget* w = this->itemWidget(item);
    CardCardWidget* ccw = static_cast<CardCardWidget*>(w);
    CardBean* card = ccw->getCard();
    return card;
}

void CardListWidget::slotShowItemMenu(QPoint p)
{
    CardBean* card = currentCard();
    if (card == nullptr)
        return;

    if (card->used)
    {
        unuse_action->setText("封存");
        if (gd->clm.currentLib()->editing_cards.contains(card))
            unuse_action->setEnabled(false);
        else
            unuse_action->setEnabled(true);
    }
    else
    {
        unuse_action->setText("解除封存");
        unuse_action->setEnabled(true);
    }

    if (!card->disabled)
        disable_action->setEnabled(false);
    else
        disable_action->setEnabled(true);

    card_menu->exec(QCursor::pos());
}

void CardListWidget::actionModifyCard()
{
    CardBean* card = currentCard();
    if (card == nullptr)
        return;

    emit signalShowCardEditor(card);
}

void CardListWidget::actionRefreshCard()
{
    emit signalRefreshAll();
    emit gd->clm.signalRehighlight();
}

void CardListWidget::actionColorCard()
{
    CardBean *card = currentCard();
    if (card == nullptr)
        return;

    // 先默认原来的数值
    int ctype = card->ctype;
    QColor color = card->color;

    // 选择颜色
    bool ok;
    color = SubjectColorDialog::pickF("card_name", "选择名片文字颜色", &ctype, color, &ok, this);
    if (!ok)
        return;

    // 设置名片新的颜色
    card->ctype = ctype;
    card->color = color;

    emit gd->clm.signalCardColorChanged(card);
}

void CardListWidget::actionUnuseCard()
{
    CardBean *card = currentCard();
    if (card == nullptr)
        return;

    card->used = !card->used;
    gd->clm.currentLib()->saveToFile(card);
    if (card->used)
        gd->clm.currentLib()->useCard(card);
    else
        gd->clm.currentLib()->unuseCard(card);
    emit gd->clm.signalRehighlight();
}

void CardListWidget::actionDisableCard()
{
    CardBean* card = currentCard();
    if (card == nullptr)
        return;

    card->disabled = !card->disabled;
    gd->clm.currentLib()->saveToFile(card);
    gd->clm.currentLib()->disableCard(card);
    emit gd->clm.signalRehighlight();
}

void CardListWidget::actionJumpAllAppear()
{

}

void CardListWidget::actionJumpAppear()
{

}

void CardListWidget::actionJumpDisappear()
{

}