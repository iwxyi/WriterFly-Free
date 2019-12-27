#include "cardlibgroup.h"

CardlibGroup::CardlibGroup(QWidget *parent) : QWidget(parent), clm(gd->clm)
{
    initView();
    initStyle();
    initData();
}

void CardlibGroup::initView()
{
    this->setMinimumSize(10, 50);

    cardlibs_list_combo = new GeneralComboBox(this);
    tab_widget = new AniTabWidget(this);
    editing_list_widget = new CardListWidget(this);
    using_list_widget = new CardListWidget(this);
    total_list_widget = new CardListWidget(this);
    tab_widget->addTab(editing_list_widget, "章节");
    tab_widget->addTab(using_list_widget, "近期");
    tab_widget->addTab(total_list_widget, "封存");
    if (us->round_view)
    {
        filter_btn = new WaterCircleButton(QPixmap(":/icons/filter"), this);
        add_card_btn = new WaterFloatButton(tr("添加名片"), this);
        expand_btn = new WaterCircleButton(QPixmap(":/icons/expand"), this);

        filter_btn->setShowAni(true);
        expand_btn->setShowAni(true);
        filter_btn->setFixedSize(us->widget_size, us->widget_size);
        expand_btn->setFixedSize(us->widget_size, us->widget_size);
        add_card_btn->setFixedHeight(us->widget_size * 3 / 4);
    }
    else
    {
        filter_btn = new InteractiveButtonBase(tr("过滤"), this);
        add_card_btn = new InteractiveButtonBase(tr("新名片"), this);
        expand_btn = new InteractiveButtonBase(tr("展开"), this);

        divider_line = new QWidget(this); // 分割线
        divider_line->setFixedHeight(1);
        divider_line->setAttribute(Qt::WA_LayoutUsesWidgetRect);
        thm->setWidgetStyleSheet(divider_line, "sidebar_dividing_line");
    }

    tab_widget->setTransparentBackground(true, 1);
    filter_btn->setToolTip("关键词/标签 过滤");
    expand_btn->setToolTip("展开/折叠 所有名片");

    QVBoxLayout *main_vlayout = new QVBoxLayout(this);
    main_vlayout->addWidget(cardlibs_list_combo);
    if (!us->round_view)
        main_vlayout->addWidget(divider_line);
    main_vlayout->addWidget(tab_widget);
    {
        QHBoxLayout *btn_hlayout = new QHBoxLayout;
        btn_hlayout->addWidget(filter_btn);
        btn_hlayout->addWidget(add_card_btn);
        btn_hlayout->addWidget(expand_btn);
        btn_hlayout->setMargin(4);
        btn_hlayout->setSpacing(4);
        main_vlayout->addLayout(btn_hlayout);
    }
    main_vlayout->setSpacing(0);
    main_vlayout->setMargin(0);
    setLayout(main_vlayout);

    connect(cardlibs_list_combo, SIGNAL(activated(int)), this, SLOT(switchCardlib(int)));
    connect(add_card_btn, SIGNAL(clicked()), this, SLOT(slotAddCardClicked()));
    connect(filter_btn, SIGNAL(clicked()), this, SLOT(slotFilterClicked()));
    connect(expand_btn, SIGNAL(clicked()), this, SLOT(slotExpandClicked()));

    connect(editing_list_widget, &CardListWidget::signalShowCardEditor, [=](CardBean* card){ emit signalShowCardEditor(card); });
    connect(using_list_widget, &CardListWidget::signalShowCardEditor, [=](CardBean* card){ emit signalShowCardEditor(card); });
    connect(total_list_widget, &CardListWidget::signalShowCardEditor, [=](CardBean* card){ emit signalShowCardEditor(card); });
    connect(editing_list_widget, &CardListWidget::signalRefreshAll, [=]{ refreshCardLists(); emit signalNeedRefreshEditingCards(); });
    connect(using_list_widget, &CardListWidget::signalRefreshAll, [=]{ refreshCardLists(); emit signalNeedRefreshEditingCards(); });
    connect(total_list_widget, &CardListWidget::signalRefreshAll, [=]{ refreshCardLists(); emit signalNeedRefreshEditingCards(); });

    cardlibs_list_combo->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    tab_widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    filter_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    add_card_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    expand_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    // 保存上次的使用记录
    connect(tab_widget, &QTabWidget::currentChanged, [=](int index) {
        us->setVal("recent/cardlib_page", index);
    });
    // 不知道为什么不能直接设置啊，不延时就报错
    // QWidget::setMinimumSize: (/MyTitleBar) Negative sizes (-34,35) are not possible
    QTimer::singleShot(200, [=] {
        int index = us->getInt("recent/cardlib_page", 0);
        if (index > 0 && index < tab_widget->count())
            tab_widget->setCurrentIndex(index);
    });

    // 初始化菜单
    expand_btn->setLeaveAfterClick(true);
    QMenu* menu = new QMenu(this);
    QAction* refresh_action = new QAction(tr("刷新"), this);
    QAction* total_action = new QAction(tr("所有名片"), this);
    QAction* expand_action = new QAction(tr("展开/折叠"), this);
    QAction* rename_action = new QAction(tr("重命名"), this);
    QAction* delete_action = new QAction(tr("删除名片库"), this);
    menu->addAction(refresh_action);
    menu->addAction(total_action);
    menu->addAction(expand_action);
    menu->addSeparator();
    menu->addAction(rename_action);
    menu->addAction(delete_action);
    expand_btn->setMenu(menu);

    total_action->setEnabled(false);

    connect(refresh_action, SIGNAL(triggered()), this, SLOT(actionRefresh()));
    connect(total_action, SIGNAL(triggered()), this, SLOT(actionTotal()));
    connect(expand_action, SIGNAL(triggered()), this, SLOT(actionExpand()));
    connect(rename_action, SIGNAL(triggered()), this, SLOT(actionRename()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(actionDelete()));

    // 全局信号槽
    connect(&gd->clm, &CardlibManager::signalReloaded, this, [=] {
        refreshCardLists();
    });
}

void CardlibGroup::initStyle()
{
    // setStyleSheet("background:transparent;");
    QString trans("AniTabWidget{background: transparent; border:none;}");
    // tab_widget->setStyleSheet(trans);
    add_card_btn->setBorderWidth(0);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void CardlibGroup::initData()
{
    // refreshCardlibList();

    // 名片库加载完成
    connect(&gd->clm, &CardlibManager::signalReloaded, [=] {
        log("名片库加载完成");
        emit signalNeedRefreshEditingCards();
    });

    // 名片库正在使用中的名片变更，刷新全部
    connect(&gd->clm, &CardlibManager::signalUsingCardsModified, [=] {
        refreshCardLists();
        emit signalNeedRefreshEditingCards();
    });

    // 名片库正在使用中的名片变更，刷新单个
    connect(&gd->clm, &CardlibManager::signalCardUsed, [=](CardBean *card) {
        total_list_widget->removeCard(card);
        using_list_widget->addOneCard(card);
        emit signalNeedRefreshEditingCards();
    });
    connect(&gd->clm, &CardlibManager::signalCardUnused, [=](CardBean *card) {
        using_list_widget->removeCard(card);
        total_list_widget->addOneCard(card);
        emit signalNeedRefreshEditingCards();
    });

    // 名片库添加名片时，添加到卡片列表末尾，并且聚焦此卡片
    connect(&gd->clm, &CardlibManager::signalCardAppened, [=](CardBean *card) {
        if (card->disabled)
            return;
        if (card->used)
        {
            using_list_widget->addOneCard(card);
            using_list_widget->scrollToBottom();
        }
        else
        {
            total_list_widget->addOneCard(card);
            total_list_widget->scrollToBottom();
        }
        emit signalNeedRefreshEditingCards();
    });

    // 名片库删除名片时
    connect(&gd->clm, &CardlibManager::signalCardDeleted, [=](CardBean *card) {
        if (card->disabled)
            return;
        if (card->used)
        {
            using_list_widget->removeCard(card);
            editing_list_widget->removeCard(card);
        }
        else
        {
            total_list_widget->removeCard(card);
        }
        emit signalNeedRefreshEditingCards();
    });

    // 启用名片
    connect(&gd->clm, &CardlibManager::signalCardEnabled, [=](CardBean *card) {
        if (card->used)
        {
            using_list_widget->addOneCard(card);
            using_list_widget->scrollToBottom();
        }
        else
        {
            total_list_widget->addOneCard(card);
            total_list_widget->scrollToBottom();
        }
        emit signalNeedRefreshEditingCards();
    });

    // 禁用名片
    connect(&gd->clm, &CardlibManager::signalCardDisabled, [=](CardBean *card) {
        if (card->used)
        {
            using_list_widget->removeCard(card);
            editing_list_widget->removeCard(card);
        }
        else
        {
            total_list_widget->removeCard(card);
        }
        emit signalNeedRefreshEditingCards();
    });

    // 修改颜色
    connect(&gd->clm, &CardlibManager::signalCardColorChanged, [=](CardBean *card) {
        using_list_widget->reloadCard(card);
        editing_list_widget->reloadCard(card);
        total_list_widget->reloadCard(card);
    });
    connect(&gd->clm, &CardlibManager::signalRefreshCardColors, [=] {
        refreshCardLists();
    });

    // 延时加载名片库
    QTimer::singleShot(300, [=] {
        QString name = us->getStr("recent/cardlib");
        refreshCardlibList(name); // 刷新名片库列表
        if (!name.isEmpty())
        {
            clm.loadCardlib(name);
            cardlibs_list_combo->setCurrentText(name);
        }
    });
}

void CardlibGroup::updateUI()
{
    if (us->round_view) // 圆角
    {
        add_card_btn->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);
        add_card_btn->setTextColor(us->accent_color);
        add_card_btn->setBorderColor(us->accent_color);

        filter_btn->setIconColor(us->getNoOpacityColor(us->accent_color));
        expand_btn->setIconColor(us->getNoOpacityColor(us->accent_color));
    }
    else
    {
        add_card_btn->setIconColor(us->getNoOpacityColor(us->accent_color));
        add_card_btn->setTextColor(us->global_font_color);

        filter_btn->setTextColor(us->global_font_color);
        expand_btn->setTextColor(us->global_font_color);
    }
    thm->setWidgetStyleSheet(add_card_btn, "add_chapter_button");

    QString combo_style;
    if (us->round_view)
    {
        QString border_radius = QString::number(cardlibs_list_combo->height() / 2 - 2);
        thm->setWidgetStyleSheet(cardlibs_list_combo, "book_list_comboBox_rounded", QStringList() << "【round_radius】" << border_radius);
    }
    else
    {
        cardlibs_list_combo->setFixedHeight(us->widget_size + 4);
        thm->setWidgetStyleSheet(cardlibs_list_combo, "book_list_comboBox_square");
    }

    // tab_widget->setTransparentBackground(us->mainwin_sidebar_color.alpha() != 255);
    tab_widget->setTransparentBackground(true, 1);
    tab_widget->setBaseBacgroundColor(us->mainwin_sidebar_color.alpha() == 255 ? us->mainwin_sidebar_color : Qt::transparent);
    tab_widget->disableSwitchAnimation(us->mainwin_sidebar_color.alpha() != 255 && us->mainwin_sidebar_color.alpha() != 0);
}

void CardlibGroup::refreshCardlibList(QString recent)
{
    cardlibs_list_combo->clear();
    QDir dir(rt->CARDLIB_PATH);
    QList<QFileInfo> files = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    int count = files.count();
    for (int i = 0; i < count; ++i)
    {
        QFileInfo info = files.at(i);
        QString file_name = info.fileName();
        if (file_name == "." || file_name == "..")
            continue;
        cardlibs_list_combo->addItem(file_name);

        // 是上次打开的名片库
        if (!recent.isEmpty() && file_name == recent)
        {
            cardlibs_list_combo->setCurrentIndex(i);
            cardlibs_list_combo->setCurrentText(file_name);
            // clm.loadCardlib(file_name); // 初始化的时候再进行读取
        }
    }

    cardlibs_list_combo->addItem("新建名片库");
}

void CardlibGroup::refreshCardLists()
{
    using_list_widget->loadCards(clm.currentLib()->using_cards);
    total_list_widget->loadCards(clm.currentLib()->unused_cards);
}

void CardlibGroup::enterEvent(QEvent *event)
{
    filter_btn->showForeground2();
    expand_btn->showForeground2();

    return QWidget::enterEvent(event);
}

void CardlibGroup::leaveEvent(QEvent *event)
{
    filter_btn->hideForeground();
    expand_btn->hideForeground();

    return QWidget::leaveEvent(event);
}

bool CardlibGroup::switchCardlib(int index)
{
    int count = cardlibs_list_combo->count();

    if (index < 0 || index >= count)
        return false;

    if (index == count - 1) // 创建新书
    {
        // 先切换回去（万一取消创建了呢，还是显示“创建新书”呀）
        for (int i = 0; i < cardlibs_list_combo->count(); i++)
            if (cardlibs_list_combo->itemText(i) == rt->current_novel)
            {
                cardlibs_list_combo->setCurrentIndex(i);
                break;
            }
        // 创建作品流程
        createCardlib();
        return false;
    }

    QString cardlib_name = cardlibs_list_combo->currentText();
    clm.loadCardlib(cardlib_name);

    emit signalOpenCardlib(cardlib_name);

    us->setVal("recent/cardlib", cardlib_name);
    return true;
}

bool CardlibGroup::createCardlib()
{
    // 输入书名
    bool isOk;
    QString new_name = clm.getSuitableCardlibName(), tip = "请输入新名片库名字";

    while (1)
    {
        new_name = QInputDialog::getText(this, "创建新名片库", tip, QLineEdit::Normal, new_name, &isOk);
        if (!isOk || new_name.isEmpty())
            return false;

        // 判断是否包含特殊字符
        QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            if (new_name.contains(cs[i]))
            {
                tip = tr("库名包含特殊字符【%1】，请重新输入").arg(cs[i]);
                //QMessageBox::information(NULL, tr("创建新书失败"), tr("包含特殊字符【%1】啦！").arg(cs[i]));
                break;
            }
        }
        if (i == 10)
            break;
    }

    // 检验重复名片库
    QDir bookDir(rt->CARDLIB_PATH + new_name);
    if (bookDir.exists())
    {
        QMessageBox::critical(this, tr("创建新名片库失败"), tr("已经有同名名片库《%1》啦！").arg(new_name));
        return false;
    }

    // 开始创建文件夹
    clm.addCardlib(new_name);

    // 重新读取目录(设置+目录）
    clm.loadCardlib(new_name);

    // 添加到目录列表
    int count = cardlibs_list_combo->count();
    cardlibs_list_combo->insertItem(count - 1, new_name);
    cardlibs_list_combo->setCurrentIndex(count);
    cardlibs_list_combo->setCurrentText(new_name);

    emit signalOpenCardlib(new_name);

    // 保存到最近作品
    us->setVal("recent/cardlib", new_name);
    return true;
}

void CardlibGroup::slotFilterClicked()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return;
    }
}

void CardlibGroup::slotAddCardClicked()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return;
    }
    emit signalAddNewCard();
}

void CardlibGroup::slotExpandClicked()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return;
    }
}

void CardlibGroup::slotRefreshEditingCards(QTextEdit *edit)
{
    if (gd->clm.currentLib() == nullptr || edit == nullptr)
        return;
    log2("刷新名片", gd->clm.currentLib()->using_cards.size());

    // 使用多线程加载名片
    SelectEditingCardsThread* thread = new SelectEditingCardsThread(edit->toPlainText());
    connect(thread, &SelectEditingCardsThread::signalRefreshEditingCardList, this, [=]{
        editing_list_widget->loadCards(gd->clm.currentLib()->editing_cards);
    });
    connect(thread, &SelectEditingCardsThread::signalRefreshCardsList, this, [=]{
        refreshCardLists();
        emit gd->clm.signalRehighlight();
    });
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

QList<CardCardWidget*> CardlibGroup::getCardWidgets(CardBean *card)
{
    QList<CardCardWidget*> list;

    CardCardWidget* w = editing_list_widget->getCardWidget(card);
    if (w != nullptr)
        list.append(w);

    w = using_list_widget->getCardWidget(card);
    if (w != nullptr)
        list.append(w);

    w = total_list_widget->getCardWidget(card);
    if (w != nullptr)
        list.append(w);

    return list;
}

void CardlibGroup::actionRefresh()
{
    refreshCardLists();
    emit signalNeedRefreshEditingCards();
}

void CardlibGroup::actionExpand()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return;
    }

    CardListWidget* clw = static_cast<CardListWidget*>(tab_widget->currentWidget());
    clw->expandOrShrink();
}

void CardlibGroup::actionTotal()
{

}

bool CardlibGroup::actionRename()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return false;
    }

    // 输入书名
    bool isOk;
    QString new_name = clm.currentLib()->getName(), tip = "请输入名片库新名字";

    while (1)
    {
        new_name = QInputDialog::getText(this, "重命名名片库", tip, QLineEdit::Normal, new_name, &isOk);
        if (!isOk || new_name.isEmpty())
            return false;

        // 判断是否包含特殊字符
        QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            if (new_name.contains(cs[i]))
            {
                tip = tr("库名包含特殊字符【%1】，请重新输入").arg(cs[i]);
                break;
            }
        }
        if (i == 10)
            break;
    }

    // 检验重复名片库
    QDir bookDir(rt->CARDLIB_PATH + new_name);
    if (bookDir.exists())
    {
        QMessageBox::critical(this, tr("重命名名片库失败"), tr("已经有同名名片库《%1》啦！").arg(new_name));
        return false;
    }

    // 重命名文件
    if (!clm.renameCardlib(new_name))
    {
        QMessageBox::warning(this, tr("提示"), tr("重命名名片库失败"));
        return false;
    }

    // 刷新列表
    refreshCardlibList("");
    cardlibs_list_combo->setCurrentText(new_name);

    us->setVal("recent/cardlib", new_name);
    return true;
}

bool CardlibGroup::actionDelete()
{
    if (gd->clm.currentLib() == nullptr)
    {
        QMessageBox::warning(this, "空名片库", "请点击上方“新建名片库”，创建作品专属名片库");
        return false;
    }
    if (QMessageBox::question(this, "提示", "删除名片库后将无法恢复，是否继续？") != QMessageBox::Yes)
        return false;

    if (!clm.deleteCardlib(clm.currentLib()->getName()))
    {
        QMessageBox::warning(this, tr("提示"), tr("删除名片库失败"));
        return false;
    }

    us->setVal("recent/cardlib", "");

    refreshCardlibList();
    editing_list_widget->loadCards(CardList());
    using_list_widget->loadCards(CardList());
    total_list_widget->loadCards(CardList());
    return true;
}
