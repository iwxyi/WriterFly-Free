#include "cardcardwidget.h"

CardCardWidget::CardCardWidget(CardBean *card, QWidget *parent) : InteractiveButtonBase(parent), card(card)
{
    initView();
    initStyle();
    initEvent();
    refreshData();
}

void CardCardWidget::initView()
{
    setDoubleClicked(true);          // 双击编辑
    setLeaveAfterClick(true);        // 允许失去焦点事件
    setFocusPolicy(Qt::StrongFocus); // 这样才能监听失去焦点事件

    name_label = new QLabel(this);
    brief_label = new QLabel(this);
    QVBoxLayout *main_vlayout = new QVBoxLayout;
    main_vlayout->addWidget(name_label);
    main_vlayout->addWidget(brief_label);
    setLayout(main_vlayout);

    QFont font = name_label->font();
    font.setBold(true);
    name_label->setFont(font);

    int shadow = 2;
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, shadow);
    shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
    shadow_effect->setBlurRadius(10);
    this->setGraphicsEffect(shadow_effect);
}

void CardCardWidget::initStyle()
{
    setRadius(us->widget_radius);
    setBgColor(us->mainwin_bg_color);
}

void CardCardWidget::initEvent()
{
    connect(this, &InteractiveButtonBase::clicked, [=] {
        bool expand = brief_label->isHidden();
        emit signalExpanded(expand);
        showBrief(expand);
    });
}

CardBean* CardCardWidget::getCard() const
{
    return card;
}

void CardCardWidget::showBrief(bool show)
{
    if (show)
        brief_label->show();
    else
        brief_label->hide();
    card->expand = show;
}

void CardCardWidget::refreshData()
{
    name_label->setStyleSheet("color:" + QVariant(card->color).toString() + ";");
    brief_label->setStyleSheet("color:" + QVariant(QColor(Qt::gray)).toString() + ";");

    name_label->setText(card->name);
    brief_label->setText(card->brief);
}

bool CardCardWidget::isExpanded()
{
    return !brief_label->isHidden();
}
