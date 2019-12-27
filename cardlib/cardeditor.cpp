#include "cardeditor.h"

CardEditor::CardEditor(QWidget *parent) : MyDialog(parent), modify_model(false), card(nullptr), from(nullptr)
{
    if (gd->clm.currentLib() == nullptr)
        return;

    card = new CardBean;
    card->ctype = 0;
    card->color = us->accent_color;

    initView();
    initData();
    initEvent();
}

CardEditor::CardEditor(QString name, QWidget *parent) : MyDialog(parent), modify_model(false), card(nullptr), from(nullptr)
{
    if (gd->clm.currentLib() == nullptr)
        return;
    card = new CardBean;
    card->name = name;
    card->ctype = 0;
    card->color = us->accent_color;

    initView();
    connect(name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(slotNewCardNameChanged(const QString &)));
    initData();
    initEvent();
    name_edit->selectAll();
}

CardEditor::CardEditor(CardBean *card, QWidget *parent) : MyDialog(parent), modify_model(true), card(card), from(nullptr)
{
    initView();
    initData();
    initEvent();

    initMenu();
}

CardEditor::CardEditor(CardBean *card, CardEditor* from, QWidget *parent) : MyDialog(parent), modify_model(true), card(card), from(from)
{
    initView();
    initData();
    initEvent();

    initMenu();

    setTitleTree();
}

void CardEditor::initView()
{
    setMinimumSize(close_btn->height()*8, close_btn->height()*12);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    // setAttribute(Qt::WA_ShowModal, false);
    // setWindowModality(Qt::NonModal);
    // setModal(false);
    setFocusPolicy(Qt::ClickFocus);

    QLabel *name_label = new QLabel("名称", this);
    name_edit = new QLineEdit(this);
    color_btn = new WaterCircleButton(QPixmap(":/icons/color"), this);
    scroll_area = new QScrollArea(this);
    scroll_widget = new QWidget(scroll_area);
    QLabel *pat_label = new QLabel("匹配", scroll_widget);
    pat_edit = new QLineEdit(scroll_widget);
    QLabel *tags_label = new QLabel("标签", scroll_widget);
    tags_edit = new QLineEdit(scroll_widget);
    tags_btn = new WaterCircleButton(QPixmap(":/icons/menu/tag"), this);
    tags_group = new WaterFallButtonGroup(this);
    brief_edit = new GeneralEditor(scroll_widget, EDIT_TYPE_BRIEF);
    detail_edit = new GeneralEditor(scroll_widget, EDIT_TYPE_DETAIL);
    create_btn = new WaterFloatButton("创建名片", this);
    scroll_area->setMinimumHeight(this->height() - close_btn->height()*2);
    scroll_widget->setMinimumSize(this->width() - 20, close_btn->height()*10 - 10);

    int height = name_label->height();
    brief_edit->setFixedHeight(height * 3);
    detail_edit->setMinimumHeight(height * 10);
    color_btn->setIconPaddingProper(0.1);
    color_btn->setFixedSize(height * 0.8, height * 0.8);
    color_btn->setLeaveAfterClick(true);
    tags_btn->setIconPaddingProper(0.1);
    tags_btn->setFixedSize(height * 0.8, height * 0.8);

    QString tooltip = "名片名字(人名/物名/地名等)，列表中显示的文字";
    name_label->setToolTip(tooltip);
    name_edit->setToolTip(tooltip);
    tooltip = "匹配的正则表达式，即编辑框中文字高亮的部分。空则默认使用名字\n注意：符号均为英文（即半角符号）\n\n示例(不包括方括号【】)：\n";
    tooltip += "【王?小明】同时匹配“王小明”和“小明”\n【萧炎|岩枭|药岩】同时匹配三个名字\n";
    tooltip += "【大斗师|斗(者|师|灵)】同时匹配“斗者”、“斗师”、“大斗师”、“斗灵”\n";
    tooltip += "【(半步)?(筑基|金丹|元婴)(初期|中期|后期|大?圆满)?】同时匹配“筑基”、“筑基初期”……“筑基圆满”、“筑基大圆满”、“半步金丹”……“元婴大圆满”等一系列\n";
    pat_label->setToolTip(tooltip);
    pat_edit->setToolTip(tooltip);
    tooltip = "用于筛选和搜索的标签，多个标签空格分隔";
    tags_label->setToolTip(tooltip);
    tags_edit->setToolTip(tooltip);
    tooltip = "简介\n名片列表与搜索结果中显示的简要文字";
    brief_edit->setPlaceholderText(tooltip);
    tooltip = "详细\n可在此记录人物具体事迹、物品细节等";
    detail_edit->setPlaceholderText(tooltip);

    // QVBoxLayout* main_layout = new QVBoxLayout();
    addTitleSpacing();
    {
        QHBoxLayout *name_hlayout = new QHBoxLayout;
        name_hlayout->addWidget(name_label);
        name_hlayout->addWidget(name_edit);
        name_hlayout->addWidget(color_btn);
        main_layout->addLayout(name_hlayout);
    }
    // main_layout->addWidget(scroll_widget);
    main_layout->addWidget(scroll_area);
    scroll_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    {
        QVBoxLayout* scroll_vlayout = new QVBoxLayout;
        scroll_vlayout->setMargin(0);
        // scroll_widget->setFixedHeight(this->height() - close_btn->height() * 2);
        {
            QHBoxLayout *pat_hlayout = new QHBoxLayout;
            pat_hlayout->addWidget(pat_label);
            pat_hlayout->addWidget(pat_edit);
            scroll_vlayout->addLayout(pat_hlayout);
        }
        {
            QHBoxLayout *tags_hlayout = new QHBoxLayout;
            tags_hlayout->addWidget(tags_label);
            tags_hlayout->addWidget(tags_edit);
            tags_hlayout->addWidget(tags_btn);
            scroll_vlayout->addLayout(tags_hlayout);
        }
        {
            scroll_vlayout->addWidget(tags_group);
        }
        scroll_vlayout->addWidget(brief_edit);
        scroll_vlayout->addWidget(detail_edit);
        scroll_area->setWidget(scroll_widget);
        scroll_widget->setLayout(scroll_vlayout);
    }
    main_layout->addWidget(create_btn);
    // setLayout(main_layout);

    // scroll_widget->show();
    pat_label->show();
    pat_edit->show();
    tags_label->show();
    tags_edit->show();
    tags_group->show();
    brief_edit->show();
    detail_edit->show();

    QFont font = name_edit->font();
    font.setBold(true);
    name_edit->setFont(font);
    font = this->font();
    font.setItalic(true);
    tags_edit->setFont(font);
    color_btn->setIconColor(us->accent_color);
    tags_btn->setIconColor(us->accent_color);
    create_btn->setTextColor(us->accent_color);
    create_btn->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);
    setEditColor(name_edit, card->color);

    scroll_area->setObjectName("scroll_area");
    scroll_area->setStyleSheet("QScrollArea#scroll_area{ background: transparent; border: none; margin: 0px; padding: 0px; }");
    scroll_widget->setObjectName("scroll_widget");
    scroll_widget->setStyleSheet("QWidget#scroll_widget { background: transparent; border: none; }");
    thm->setWidgetStyleSheet(scroll_area->verticalScrollBar(), "scrollbar");
    thm->setWidgetStyleSheet(name_label, "card_editor_label");
    thm->setWidgetStyleSheet(name_edit, "card_editor_line_edit");
    thm->setWidgetStyleSheet(pat_label, "card_editor_label");
    thm->setWidgetStyleSheet(pat_edit, "card_editor_line_edit");
    thm->setWidgetStyleSheet(tags_label, "card_editor_label");
    thm->setWidgetStyleSheet(tags_edit, "card_editor_line_edit");
    thm->setWidgetStyleSheet(brief_edit, "card_editor_text_edit");
    thm->setWidgetStyleSheet(detail_edit, "card_editor_text_edit");
    // scroll_area->setScrollBarPolicy(Qt::ScrollBarAsNeeded, Qt::ScrollBarAsNeeded);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 显示在合适的位置(QWidget)
    /* QPoint pos = mapFromGlobal(QCursor::pos());
    if (pos.y() + this->height() > parentWidget()->height())
    {
        if (pos.y() - this->height() >= 0)
            pos.setY(pos.y() - this->height());
        else
            pos.setY(parentWidget()->height() - this->height());
    }
    if (pos.x() + this->width() > parentWidget()->width())
    {
        if (pos.x() - this->width() >= 0)
            pos.setY(pos.x() - this->width());
        else
            pos.setY(parentWidget()->width() - this->width());
    }
    this->move(pos); */
    if (us->card_save_size && us->contains("layout/cardeditor_pos") && us->contains("layout/cardeditor_size"))
    {
        QPoint pos = us->value("layout/cardeditor_pos").toPoint();
        QSize size = us->value("layout/cardeditor_size").toSize();
        setRelatedPos(pos);
        setGeometry(QRect(this->pos(), size));
    }

    // 高度改变
    connect(detail_edit, &GeneralEditor::textChanged, [=]{
//        QTextDocument* doc = detail_edit->document();
//        int height = doc->size().height();
        scroll_widget->setMinimumHeight(detail_edit->geometry().top()+detail_edit->geometry().height());
    });
}

void CardEditor::initMenu()
{
    if (!modify_model || card == nullptr)
        return;

    // 初始化
    menu = new QMenu(this);
    QAction *search_action = new QAction(thm->icon("menu/search"), "全书搜索", this);
    QAction *unuse_action = new QAction(thm->icon("menu/sensitive"), "封存名片", this);
    QAction *disable_action = new QAction(thm->icon("menu/disable"), "禁用名片", this);
    QAction *delete_action = new QAction(thm->icon("menu/delete"), "删除名片", this);
    QMenu* self_menu = new QMenu("单独设置", this);
    QAction *card_save_size_self_action = new QAction(thm->icon("menu/windows"), "单独保存窗口大小", this);
    QMenu *global_menu = new QMenu("全局设置", this);
    card_fill_tags_action = new QAction(thm->icon("menu/tag"), "自动填充新名片标签", this);
    card_fill_color_action = new QAction(thm->icon("color"), "自动填充新名片颜色", this);
    card_auto_scroll_action = new QAction(thm->icon("menu/brief"), "自动滚动至简介", this);
    card_save_scroll_action = new QAction(thm->icon("menu/scroll"), "保存滚动进度", this);
    card_save_size_action = new QAction(thm->icon("menu/windows"), "保存窗口大小", this);
    card_follow_move_action = new QAction(thm->icon("menu/move"), "跟随主窗口移动", this);
    card_multi_tile_action = new QAction(thm->icon("menu/tile"), "多张名片平铺", this);
    QAction *help_action = new QAction(thm->icon("menu/help"), "帮助", this);

    // 设置属性
    self_menu->setIcon(thm->icon("menu/chrysanthemum"));
    global_menu->setIcon(thm->icon("menu/global"));

    // 设置顺序
    menu->addAction(search_action);
    menu->addAction(unuse_action);
    menu->addAction(disable_action);
    menu->addAction(delete_action);
    menu->addSeparator();
    menu->addMenu(self_menu);
    {
        self_menu->addAction(card_save_size_self_action);
    }
    menu->addMenu(global_menu);
    {
        global_menu->addAction(card_fill_tags_action);
        global_menu->addAction(card_fill_color_action);
        global_menu->addSeparator();
        global_menu->addAction(card_auto_scroll_action);
        global_menu->addAction(card_save_scroll_action);
        global_menu->addAction(card_save_size_action);
        global_menu->addSeparator();
        global_menu->addAction(card_follow_move_action);
        global_menu->addAction(card_multi_tile_action);
    }
    menu->addSeparator();
    menu->addAction(help_action);
    setDialogMenu(menu);

    // 设置可用情况
    search_action->setEnabled(false);
    disable_action->setCheckable(true);
    if (card->disabled || gd->clm.currentLib()->editing_cards.contains(card))
        unuse_action->setEnabled(false);
    else
    {
        if (card->used)
            unuse_action->setText("封存");
        else
            unuse_action->setText("解除封存");
    }
    if (card->disabled)
        disable_action->setChecked(true);
    else
        disable_action->setChecked(false);

    card_save_size_self_action->setCheckable(true);
    card_fill_tags_action->setCheckable(true);
    card_fill_color_action->setCheckable(true);
    card_auto_scroll_action->setCheckable(true);
    card_save_scroll_action->setCheckable(true);
    card_save_size_action->setCheckable(true);
    card_follow_move_action->setCheckable(true);
    card_multi_tile_action->setCheckable(true);
    help_action->setCheckable(true);

    card_save_size_self_action->setEnabled(false);
    card_fill_tags_action->setEnabled(false);
    card_fill_color_action->setEnabled(false);
    // card_auto_scroll_action->setEnabled(false);
    card_save_scroll_action->setEnabled(false);
    // card_save_size_action->setEnabled(false);
    // card_follow_move_action->setEnabled(false);
    card_multi_tile_action->setEnabled(false);

    // 连接信号槽
    connect(search_action, SIGNAL(triggered()), this, SLOT(actionSearchBook()));
    connect(unuse_action, SIGNAL(triggered()), this, SLOT(actionUnuseCard()));
    connect(disable_action, SIGNAL(triggered()), this, SLOT(actionDisableCard()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(actionDeleteCard()));

    connect(card_save_size_self_action, SIGNAL(triggered()), this, SLOT(actionSaveSizeSelf()));
    connect(card_fill_tags_action, SIGNAL(triggered()), this, SLOT(actionFillTags()));
    connect(card_fill_color_action, SIGNAL(triggered()), this, SLOT(actionFillColor()));
    connect(card_auto_scroll_action, SIGNAL(triggered()), this, SLOT(actionAutoScroll()));
    connect(card_save_scroll_action, SIGNAL(triggered()), this, SLOT(actionSaveScroll()));
    connect(card_save_size_action, SIGNAL(triggered()), this, SLOT(actionSaveSize()));
    connect(card_follow_move_action, SIGNAL(triggered()), this, SLOT(actionFollowMove()));
    connect(card_multi_tile_action, SIGNAL(triggered()), this, SLOT(actionMultiTile()));
    connect(help_action, SIGNAL(triggered()), this, SLOT(actionHelp()));
}

void CardEditor::refreshMenu()
{
    card_fill_tags_action->setChecked(us->card_fill_tags);
    card_fill_color_action->setChecked(us->card_fill_color);
    card_auto_scroll_action->setChecked(us->card_auto_scroll);
    card_save_scroll_action->setChecked(us->card_save_scroll);
    card_save_size_action->setChecked(us->card_save_size);
    card_follow_move_action->setChecked(us->card_follow_move);
    card_multi_tile_action->setChecked(us->card_multi_tile);
}

void CardEditor::initData()
{
    rehighlight = false;

    name_edit->setText(card->name);
    pat_edit->setText(card->pat);
    tags_edit->setText(card->tags.join(" "));
    brief_edit->setInitialText(card->brief);
    detail_edit->setInitialText(card->detail);

    QString tooltip = "名片在编辑框中高亮的文字颜色\n";
    if (card->ctype == 0)
        tooltip += "当前颜色：自定义";
    else
        tooltip += "当前颜色：" + QString::number(card->ctype);
    color_btn->setToolTip(tooltip);

    // 去除当前行高亮
    QList<QTextEdit::ExtraSelection> es;
    QTextEdit::ExtraSelection s;
    s.format.setBackground(Qt::transparent);
    s.format.setProperty(QTextFormat::FullWidthSelection, true);
    s.cursor = brief_edit->textCursor();
    s.cursor.clearSelection();
    es.append(s);
    brief_edit->setExtraSelections(es);
    detail_edit->setExtraSelections(es);

    initTags();

    // 设置滚动条默认滚动至内容
    if (us->card_auto_scroll)
    {
        QTimer::singleShot(0, [=] {
            scroll_widget->setMinimumHeight(detail_edit->geometry().top()+detail_edit->geometry().height());
            scroll_widget->setFixedHeight(detail_edit->geometry().top() + detail_edit->geometry().height());
            if (brief_edit->toPlainText().isEmpty() && !detail_edit->toPlainText().isEmpty())
            {
                scroll_area->verticalScrollBar()->setSliderPosition(detail_edit->geometry().top());
            }
            else if (!brief_edit->toPlainText().isEmpty())
            {
                scroll_area->verticalScrollBar()->setSliderPosition(brief_edit->geometry().top());
            }
        });
    }
}

void CardEditor::initEvent()
{
    connect(color_btn, SIGNAL(clicked()), this, SLOT(slotSelectColor()));
    connect(create_btn, SIGNAL(clicked()), this, SLOT(slotCreateCard()));
    connect(brief_edit, &ChapterEditor::signalShowCardEditor, [=](CardBean *card) { if (card != this->card) emit signalShowCardEditor(card, this); });
    connect(detail_edit, &ChapterEditor::signalShowCardEditor, [=](CardBean *card) { if (card != this->card) emit signalShowCardEditor(card, this); });
    connect(this, &MyDialog::signalMoved, this, [=](QPoint){
        us->setVal("layout/cardeditor_pos", getRelatedPos());
    });
    connect(tags_btn, &InteractiveButtonBase::clicked, this, [=]{
        if (tags_group->isHidden())
        {
            tags_group->show();
            scroll_widget->setMinimumHeight(scroll_widget->minimumHeight() + tags_group->height());
        }
        else
        {
            tags_group->hide();
            scroll_widget->setMinimumHeight(scroll_widget->minimumHeight() - tags_group->height());
        }
        scroll_widget->resize(scroll_widget->width(), scroll_widget->minimumHeight());
    });

    // 修改模式，自动保存
    if (modify_model)
        create_btn->hide();

    {
        connect(name_edit, &QLineEdit::textEdited, [=](const QString &) {
            if (!modify_model) return;
            card->name = name_edit->text();
            rehighlight = true;
        });
        connect(pat_edit, &QLineEdit::textEdited, [=](const QString &) {
            if (!modify_model) return;
            card->pat = pat_edit->text();
            rehighlight = true;
        });
        connect(tags_edit, &QLineEdit::textEdited, [=](const QString &) {
            if (!modify_model) return;
            card->tags = tags_edit->text().split(" ", QString::SkipEmptyParts);
        });
        connect(brief_edit, &QTextEdit::textChanged, [=]() {
            if (!modify_model) return;
            card->brief = brief_edit->toPlainText();
        });
        connect(detail_edit, &QTextEdit::textChanged, [=]() {
            if (!modify_model) return;
            card->detail = detail_edit->toPlainText();
        });

        connect(name_edit, SIGNAL(textEdited(const QString &)), this, SLOT(slotSave()));
        connect(pat_edit, SIGNAL(textEdited(const QString &)), this, SLOT(slotSave()));
        connect(tags_edit, SIGNAL(textEdited(const QString &)), this, SLOT(slotSave()));
        connect(brief_edit, SIGNAL(textChanged()), this, SLOT(slotSave()));
        connect(detail_edit, SIGNAL(textChanged()), this, SLOT(slotSave()));
    }

    // 焦点事件（好像会导致切换的时候崩溃）
    /* connect(qApp, &QApplication::focusChanged, [=](QWidget *o, QWidget *c){
        if (o == this||o == name_edit||o == pat_edit||o == tags_edit||o == brief_edit||o == detail_edit)
            return;
        if (c == this || c == name_edit || c == pat_edit || c == tags_edit || c == brief_edit || c == detail_edit)
            this->raise();
    }); */

    /* // 全局卡片变更，自己信息改变了（可能就是本窗口做出的改变？）
    connect(&gd->clm, &CardlibManager::signalCardDeleted, this, [=](CardBean* card){
        if (card == this->card)
        {
            this->close();
        }
    }); */

    // shift键同时移动子对话框
    connect(this, &CardEditor::signalShiftMoved, this, [=](QPoint delta) {
        foreach (CardEditor *editor, toes)
        {
            editor->movePosWithChilds(delta, this);
        }
    });
}

void CardEditor::initTags()
{
    if (gd->clm.currentLib() == nullptr)
        return ;

    // 获取最可能的tag并且排序，添加按钮（文字、颜色、选中等）
    QList<TagInfo> list;
    QStringList default_tags;
    if (modify_model)
    {
        list = CardlibTagsAI::getAllTagsOrderName(card->name, card->tags, false, this);
    }
    else
    {
        list = CardlibTagsAI::getAllTagsOrderName(card->name, true, this);
    }
    foreach (TagInfo info, list)
    {
        tags_group->addButton(info.tag, info.color, info.selected);
        if (info.selected)
            default_tags.append(info.tag);
    }

    // 初始化设置新名片默认的tags
    if (!modify_model)
    {
        tags_edit->setText(default_tags.join(" "));
    }

    // 获取文字可能的颜色


    // 默认排序，临时使用
    /*QHash<QString,int> all_tags = gd->clm.currentLib()->getAllTags();
    QStringList names = all_tags.keys();
    QStringList selected_tags = card->tags;
    tags_group->initStringList(names, selected_tags);*/

    QTimer::singleShot(0, [=] {
        tags_group->updateButtonPositions();
        if (modify_model)
            tags_group->hide();
    });

    // 连接信号槽
    connect(tags_group, &WaterFallButtonGroup::signalSelected, this, [=](QString s) {
        QStringList tags = tags_edit->text().split(" ", QString::SkipEmptyParts);
        if(!tags.contains(s))
        {
            tags.append(s);
            tags_edit->setText(tags.join(" "));
        }
    });
    connect(tags_group, &WaterFallButtonGroup::signalUnselected, this, [=](QString s) {
        QStringList tags = tags_edit->text().split(" ", QString::SkipEmptyParts);
        if (tags.contains(s))
        {
            tags.removeOne(s);
            tags_edit->setText(tags.join(" "));
        }
    });
    connect(tags_edit, &QLineEdit::textChanged, this, [=](const QString &s){
        QStringList tags = tags_edit->text().split(" ", QString::SkipEmptyParts);
        tags_group->setSelects(tags);
    });
}

void CardEditor::setEditColor(QLineEdit *edit, QColor color)
{
    QPalette palette = edit->palette();
    palette.setColor(QPalette::ColorRole::Text, color);
    edit->setPalette(palette);
}

void CardEditor::setTitleTree()
{
    createTitleBar();

    int font_size = this->font().pointSize() * 0.8;

    // 添加返回上一级按钮（其实就是关闭自己）
    InteractiveButtonBase* prev_btn = new InteractiveButtonBase(QIcon(":/icons/sidebar_back"), this);
    prev_btn->setFixedSize(close_btn->size());
    titlebar_hlayout->addWidget(prev_btn);
    connect(prev_btn, SIGNAL(clicked()), this, SLOT(close()));

    QList<QWidget*>widgets;

    // 遍历添加层级（逆序）
    CardEditor* editor = this;
    while (editor->from != nullptr)
    {
        editor = editor->from;
        QString name = editor->card->name;
        InteractiveButtonBase* from_btn = new InteractiveButtonBase(name, this);
        from_btn->setFixedHeight(close_btn->height());
        from_btn->setFontSize(font_size);
        from_btn->setFixedForeSize(true);
        from_btn->setFixedForePos(true);
        widgets.append(from_btn);
        // titlebar_hlayout->addWidget(from_btn);

        connect(from_btn, &InteractiveButtonBase::clicked, [=]{
            emit signalLocateFromEditor(editor);
            close();
        });

        if (editor->from != nullptr)
        {
            QLabel* dot_label = new QLabel("·", this);
            dot_label->setFixedHeight(close_btn->height());
            // titlebar_hlayout->addWidget(dot_label);
            widgets.append(dot_label);
        }
    }

    for (int i = widgets.size() - 1; i >= 0; i--) {
        titlebar_hlayout->addWidget(widgets.at(i));
    }
}

QPoint CardEditor::getRelatedPos()
{
    if (parentWidget() == nullptr)
        return pos();
    return parentWidget()->mapFromGlobal(pos());
}

void CardEditor::setRelatedPos(QPoint pos)
{
    if (parentWidget() != nullptr)
    {
        pos = parentWidget()->mapToGlobal(pos);
    }
    move(pos);
}

void CardEditor::movePos(QPoint delta)
{
    this->move(this->pos() + delta);
}

bool CardEditor::movePosFinished()
{
	/* if (card != nullptr && card->save_size_self)
    {
        card->pos = getRelatedPos(); // 保存自己的位置
		return true;
    } */
    us->setVal("layout/cardeditor_pos", getRelatedPos()); // 保存全局名片框位置
    return false;
}

void CardEditor::movePosWithChilds(QPoint delta, CardEditor *editor)
{
    if (editor == this) // 可能会出错，死循环（虽然逻辑上不会出现这种情况，就怕意外）
        return ;
    this->movePos(delta);
    foreach (CardEditor * editor, toes) {
        movePosWithChilds(delta, editor);
    }
}

void CardEditor::resizeEvent(QResizeEvent* event)
{
    MyDialog::resizeEvent(event);
    scroll_widget->setFixedWidth(scroll_area->width() - scroll_area->verticalScrollBar()->sizeHint().width());
//    scroll_widget->setMinimumHeight(qMax(scroll_area->minimumHeight(), scroll_widget->minimumHeight()));
    scroll_widget->setMinimumHeight(detail_edit->geometry().top()+detail_edit->geometry().height());
    scroll_widget->resize(scroll_widget->width(), scroll_widget->minimumHeight());
//    detail_edit->setMinimumHeight(scroll_area->height()-6);
//    detail_edit->resize(detail_edit->width(), detail_edit->minimumHeight());
    detail_edit->setFixedHeight(scroll_area->height());

    // 保存名片大小位置
    us->setVal("layout/cardeditor_pos", getRelatedPos());
    us->setVal("layout/cardeditor_size", size());
}

void CardEditor::focusInEvent(QFocusEvent* event)
{
    // this->raise();

    MyDialog::focusInEvent(event);
}

void CardEditor::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 设置圆角裁剪
    if (us->integration_window && (us->widget_radius || us->mainwin_border_size))
    {
        QPainterPath clip_path;
        clip_path.addRoundedRect(us->mainwin_border_size, us->mainwin_border_size, width() - us->mainwin_border_size * 2, height() - us->mainwin_border_size * 2, us->widget_radius, us->widget_radius);
        painter.setClipPath(clip_path, Qt::IntersectClip);
    }

    // 绘制背景
    if (dybg)
    {
        dybg->draw(painter);
    }
    else
    {
        if (us->widget_radius)
        {
            QPainterPath path;
            path.addRoundedRect(QRect(0, 0, width(), height()), us->widget_radius, us->widget_radius);
            painter.fillPath(path, us->getNoOpacityColor(us->mainwin_sidebar_color));
        }
        else
            painter.fillRect(QRect(0, 0, width(), height()), us->getNoOpacityColor(us->mainwin_sidebar_color));
    }
}

void CardEditor::slotSelectColor()
{
    // 全选的话，取消全选，以便于看清楚颜色
    name_edit->deselect();

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

    setEditColor(name_edit, color);

    // 保存修改
    if (modify_model)
    {
        rehighlight = true;
        slotSave();
    }
}

void CardEditor::slotSave()
{
    if (gd->clm.currentLib() == nullptr)
        return;
    if (!modify_model) return;
    gd->clm.currentLib()->saveToFile(card);
    if (rehighlight)
    {
        rehighlight = false;
        emit gd->clm.signalRehighlight();
    }
    emit signalCardModified(card);
}

void CardEditor::slotCreateCard()
{
    if (gd->clm.currentLib() == nullptr)
        return;

    QString name = name_edit->text();

    foreach (CardBean* card, gd->clm.currentLib()->using_cards)
    {
        if (card->name == name)
        {
            create_btn->setText("已存在");
            create_btn->setEnabled(false);
            return ;
        }
    }

    foreach (CardBean* card, gd->clm.currentLib()->unused_cards)
    {
        if (card->name == name)
        {
            create_btn->setText("已存在");
            create_btn->setEnabled(false);
            return ;
        }
    }

    // 恢复禁用的名片
    foreach (CardBean* card, gd->clm.currentLib()->disabled_cards)
    {
        if (card->name == name)
        {
            create_btn->setText("重新禁用");
            create_btn->setEnabled(true);
            this->card = card;
            modify_model = true;
            initData();
            actionDisableCard();
            return ;
        }
    }

    if (!modify_model)
    {
        card->name = name_edit->text();
        card->pat = pat_edit->text();
        card->tags = tags_edit->text().split(" ", QString::SkipEmptyParts);
        card->disabled = false;
        card->used = true;
        card->brief = brief_edit->toPlainText();
        card->detail = detail_edit->toPlainText();

        if (card->name.isEmpty())
        {
            QMessageBox::warning(this, "空名", "请输入名字");
            return;
        }
        // 添加到全局名片中
        gd->clm.currentLib()->addCard(card);
        emit signalCardCreated(card);

        // 避免关闭时删除
        modify_model = true;
    }
    create_btn->hide();
    close();
}

void CardEditor::slotLocateFromEditor(CardEditor* editor)
{

    if (editor == this)
    {
        this->raise();
        this->setFocus();
        return;
    }

    QTimer::singleShot(100, [=]{
        emit signalLocateFromEditor(editor);
        QTimer::singleShot(100, [=]{
            this->close();
        });
    });
}

CardBean* CardEditor::getCard()
{
    return card;
}

CardEditor* CardEditor::getFrom()
{
    return from;
}

void CardEditor::addToEditor(CardEditor* editor)
{
    toes.append(editor);
}

void CardEditor::delToEditor(CardEditor* editor)
{
    toes.removeOne(editor);
}

void CardEditor::closeEvent(QCloseEvent *event)
{
    // 如果输入了一定量的内容但是一不小心直接关闭了，询问是否要进行保存
    /* if (!modify_model && !name_edit->text().isEmpty() && (brief_edit->toPlainText().length() >= 6 || detail_edit->toPlainText().length() >= 6))
    {
        if (QMessageBox::information(this, tr("提示"), tr("删除未保存的名片？"), "删除", "继续编辑", 0, 0) == 1)
        {
            event->accept();
            return;
        }
    } */

    // 删除未保存的卡片
    if (!modify_model)
        delete card;

    // 删除生成的
    foreach (CardEditor* editor, toes) {
        editor->close(); // 关闭会触发列表中删除的信号
        editor->deleteLater();
    }
    emit signalClosed();

    MyDialog::closeEvent(event);
}

void CardEditor::slotNewCardNameChanged(const QString & s)
{
    if (modify_model)
        return ;

    if (s.trimmed().isEmpty())
    {
        create_btn->setText(tr("创建名片"));
        create_btn->setEnabled(false);
    }
    else
    {
        foreach (CardBean* card, gd->clm.currentLib()->using_cards)
        {
            if (card->name == s)
            {
                create_btn->setText("已存在");
                create_btn->setEnabled(false);
                return ;
            }
        }

        foreach (CardBean* card, gd->clm.currentLib()->unused_cards)
        {
            if (card->name == s)
            {
                create_btn->setText("已存在");
                create_btn->setEnabled(false);
                return ;
            }
        }

        foreach (CardBean* card, gd->clm.currentLib()->disabled_cards)
        {
            if (card->name == s)
            {
                create_btn->setText("解除禁用");
                create_btn->setEnabled(true);
                return ;
            }
        }

        create_btn->setText("创建名片");
        create_btn->setEnabled(true);
    }
}

void CardEditor::actionSearchBook()
{
}

void CardEditor::actionUnuseCard()
{
    if (card == nullptr)
        return;

    card->used = !card->used;
    rehighlight = true;
    slotSave();
    if (card->used)
        gd->clm.currentLib()->useCard(card);
    else
        gd->clm.currentLib()->unuseCard(card);
}

void CardEditor::actionDisableCard()
{
    if (card == nullptr)
        return ;

    card->disabled = !card->disabled;
    rehighlight = true;
    slotSave();
    gd->clm.currentLib()->disableCard(card);

    // if (card->disabled)
    this->close();
}

void CardEditor::actionDeleteCard()
{
    log("删除名片");
    if (us->getBool("ask/delete_card", true))
    {
        us->setVal("ask/delete_card", false);
        if (QMessageBox::information(this, "删除名片", "建议禁用名片，而非永久删除\n禁用后不会高亮，也不会显示在列表中，但在综合搜索中可查看。\n\n是否继续删除？", QMessageBox::Ok, QMessageBox::No) != QMessageBox::Ok)
            return ;
    }
    gd->clm.currentLib()->deleteCard(card);
    log("删除名片结束");
    this->close();
}

void CardEditor::actionSaveSizeSelf()
{

}

void CardEditor::actionFillTags()
{
    us->card_fill_tags = !us->card_fill_tags;
    us->setVal("us/card_fill_tags", us->card_fill_tags);
    card_fill_tags_action->setChecked(us->card_fill_tags);
}

void CardEditor::actionFillColor()
{
    us->card_fill_color = !us->card_fill_color;
    us->setVal("us/card_fill_color", us->card_fill_color);
    card_fill_color_action->setChecked(us->card_fill_color);
}

void CardEditor::actionAutoScroll()
{
    us->card_auto_scroll = !us->card_auto_scroll;
    us->setVal("us/card_auto_scroll", us->card_auto_scroll);
    card_auto_scroll_action->setChecked(us->card_auto_scroll);
}

void CardEditor::actionSaveScroll()
{
    us->card_save_scroll = !us->card_save_scroll;
    us->setVal("us/card_save_scroll", us->card_save_scroll);
    card_save_scroll_action->setChecked(us->card_save_scroll);
}

void CardEditor::actionSaveSize()
{
    us->card_save_size = !us->card_save_size;
    us->setVal("us/card_save_size", us->card_save_size);
    card_save_size_action->setChecked(us->card_save_size);
}

void CardEditor::actionFollowMove()
{
    us->card_follow_move = !us->card_follow_move;
    us->setVal("us/card_follow_move", us->card_follow_move);
    card_follow_move_action->setChecked(us->card_follow_move);
}

void CardEditor::actionMultiTile()
{
    us->card_multi_tile = !us->card_multi_tile;
    us->setVal("us/card_multi_tile", us->card_multi_tile);
    card_multi_tile_action->setChecked(us->card_multi_tile);
}

void CardEditor::actionHelp()
{
    QString tip = "名片介绍：\n";
    tip += "提取文章中所有的名字（人物、地点、物品等），作为一张张卡片实时显示在侧边栏中，并且提供名字色彩高亮、鼠标悬浮提示、名片搜索等功能。\n\n";
    tip += "名片状态：\n";
    tip += "章节：正在编辑的章节中包含这些名片，全自动化检测，不可封存；\n";
    tip += "近期：最近使用的名片，创建时默认为此状态；\n";
    tip += "封存：暂时用不到、但还是有可能出现的名片。正文输入后自动变成“近期”；\n";
    tip += "禁用：长期不使用的名片，只能在搜索结果、所有名片中看到。可减少卡顿情况。";

    QMessageBox::information(this, "名片帮助", tip);
}

