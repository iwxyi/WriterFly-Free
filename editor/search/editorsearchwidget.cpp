#include "editorsearchwidget.h"

EditorSearchWidget::EditorSearchWidget(QWidget *parent) : QWidget(parent)
{
    aim_editor = nullptr;
    bg = new QWidget(this);
    key_editor = new QLineEdit(this);
    prev_btn = new WaterCircleButton(QPixmap(":/icons/arrow_up"), this);
    next_btn = new WaterCircleButton(QPixmap(":/icons/arrow_down"), this);
    close_btn = new WaterCircleButton(QPixmap(":/icons/close"), this);
    replace_key_editor = new QLineEdit(this);
    replace_prev_btn = new WaterCircleButton(QPixmap(":/icons/arrow_left"), this);
    replace_next_btn = new WaterCircleButton(QPixmap(":/icons/arrow_right"), this);
    replace_all_btn = new WaterCircleButton(QPixmap(":/icons/arrow_left_right"), this);

    key_editor->setPlaceholderText(" 搜索内容");
    replace_key_editor->setPlaceholderText(" 替换内容");
    key_editor->setFixedWidth(close_btn->width()*6);
    prev_btn->setFixedSize(us->widget_size, us->widget_size);
    next_btn->setFixedSize(us->widget_size, us->widget_size);
    close_btn->setFixedSize(us->widget_size, us->widget_size);
    replace_key_editor->setFixedWidth(close_btn->width()*6);
    replace_prev_btn->setFixedSize(us->widget_size, us->widget_size);
    replace_next_btn->setFixedSize(us->widget_size, us->widget_size);
    replace_all_btn->setFixedSize(us->widget_size, us->widget_size);

    key_editor->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    prev_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    next_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    close_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    replace_key_editor->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    replace_prev_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    replace_next_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    replace_all_btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    QVBoxLayout* main_vlayout = new QVBoxLayout();
    {
        QHBoxLayout *search_hlayout = new QHBoxLayout();
        search_hlayout->addWidget(key_editor);
        search_hlayout->addWidget(prev_btn);
        search_hlayout->addWidget(next_btn);
        search_hlayout->addWidget(close_btn);
        main_vlayout->addLayout(search_hlayout);

        separator = new QWidget(this);
        separator->setFixedHeight(1);
        separator->setStyleSheet("margin-left:10px; margin-right:10px; background-color:rgba(88,88,88,32);");
        main_vlayout->addWidget(separator);

        QHBoxLayout *replace_hlayout = new QHBoxLayout();
        replace_hlayout->addWidget(replace_key_editor);
        replace_hlayout->addWidget(replace_prev_btn);
        replace_hlayout->addWidget(replace_next_btn);
        replace_hlayout->addWidget(replace_all_btn);
        main_vlayout->addLayout(replace_hlayout);
    }
    // layout 默认: spacing=6, margin=9
    main_vlayout->setMargin(5);
    main_vlayout->setSpacing(0);
    this->setLayout(main_vlayout);

    connect(key_editor, SIGNAL(textEdited(QString)), this, SLOT(slotSearchKeyChanged()));
    connect(prev_btn, SIGNAL(clicked()), this, SLOT(slotSearchPrev()));
    connect(next_btn, SIGNAL(clicked()), this, SLOT(slotSearchNext()));
    connect(close_btn, &QPushButton::clicked, [=]{
        emit signalClosed();
    });
    connect(replace_prev_btn, SIGNAL(clicked()), this, SLOT(slotReplacePrev()));
    connect(replace_next_btn, SIGNAL(clicked()), this, SLOT(slotReplaceNext()));
    connect(replace_all_btn, SIGNAL(clicked()), this, SLOT(slotReplaceAll()));

    shadow = 2;
    bg->move(shadow,shadow);
    search_height = close_btn->height()+main_vlayout->margin()*2-5;
    replace_height = close_btn->height()*2+main_vlayout->margin()*2+main_vlayout->spacing()-5;
    QSize widget_size = QSize(close_btn->width()*9+main_vlayout->spacing()*5, replace_height);
    bg->setFixedSize(widget_size);
    this->setFixedSize(bg->size()+QSize(shadow*2,shadow*2));

    // 阴影效果
    shadow--;
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, shadow);
    shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
    shadow_effect->setBlurRadius(10);
    bg->setGraphicsEffect(shadow_effect);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void EditorSearchWidget::showReplace(bool show)
{
    if (show)
    {
        separator->show();
        replace_key_editor->show();
        replace_prev_btn->show();
        replace_next_btn->show();
        replace_all_btn->show();
        setFixedHeight(replace_height+shadow*4);
        bg->setFixedHeight(replace_height);
    }
    else
    {
        separator->hide();
        replace_key_editor->hide();
        replace_prev_btn->hide();
        replace_next_btn->hide();
        replace_all_btn->hide();
        setFixedHeight(search_height+shadow*4);
        bg->setFixedHeight(search_height);
    }
}

void EditorSearchWidget::setReplaceKey(QString str)
{
    replace_key_editor->setText(str);
}

void EditorSearchWidget::setAimEditor(ChapterEditor *editor)
{
    this->aim_editor = editor;
    if (aim_editor != nullptr)
    {
        aim_editor->stackUnder(this);

        slotSearchKeyChanged();

        aim_editor->setSearchKey(key_editor->text());
    }
}

void EditorSearchWidget::setText(QString def)
{
    if (!def.isEmpty() && def != key_editor->text())
    {
        key_editor->setText(def);
        slotSearchKeyChanged();
    }
    key_editor->setFocus();
    QTimer::singleShot(10, [=]{
        key_editor->selectAll();
    });
}

void EditorSearchWidget::updateUI()
{
    thm->setWidgetStyleSheet(bg, "editor_search_bar");
    thm->setWidgetStyleSheet(key_editor, "editor_search_editor");
    thm->setWidgetStyleSheet(replace_key_editor, "editor_search_editor");

    prev_btn->setIconColor(us->accent_color);
    next_btn->setIconColor(us->accent_color);
    close_btn->setIconColor(us->accent_color);
    replace_prev_btn->setIconColor(us->accent_color);
    replace_next_btn->setIconColor(us->accent_color);
    replace_all_btn->setIconColor(us->accent_color);
}

void EditorSearchWidget::showEvent(QShowEvent *event)
{
    if (aim_editor != nullptr)
    {
        connect(aim_editor, SIGNAL(cursorPositionChanged()), this, SLOT(slotSearchKeyChanged()));
    }

    key_editor->selectAll();

    prev_btn->delayShowed(100, QPoint(-1, 0));
    next_btn->delayShowed(200, QPoint(-1, 0));
    close_btn->delayShowed(300, QPoint(-1, 0));

    if (!replace_key_editor->isHidden())
    {
        replace_prev_btn->delayShowed(400, QPoint(-1, 0));
        replace_next_btn->delayShowed(500, QPoint(-1, 0));
        replace_all_btn->delayShowed(600, QPoint(-1, 0));
    }

    return QWidget::showEvent(event);
}

void EditorSearchWidget::hideEvent(QHideEvent *event)
{
    if (aim_editor != nullptr)
    {
       disconnect(aim_editor, SIGNAL(cursorPositionChanged()), this, SLOT(slotSearchKeyChanged())); // 关闭位置改变信号
    }
    return QWidget::hideEvent(event);
}

void EditorSearchWidget::focusInEvent(QFocusEvent *event)
{
    key_editor->setFocus();

    return QWidget::focusInEvent(event);
}

void EditorSearchWidget::keyPressEvent(QKeyEvent *event)
{
    auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;
    switch(event->key())
    {
    case Qt::Key_Escape: // 16777216
        emit signalClosed();
        return ;
    /**
     * ctrl:  是否聚焦编辑框
     * shift: 是否向上方搜索
     * alt:   是否不选中文字
     */
    case Qt::Key_Enter:
    case Qt::Key_Return: // 16777220
        if (shift) // 定位到上一个
        {
            slotSearchPrev(!alt);
        }
        else // 定位到下一个
        {
            slotSearchThis(!alt);
        }
        adjustFocus(ctrl);
        return ;
    case Qt::Key_PageUp:
    case Qt::Key_Up:
        slotSearchPrev(!alt);
        adjustFocus(ctrl || shift);
        return ;
    case Qt::Key_PageDown:
    case Qt::Key_Down:
        slotSearchNext(!alt);
        adjustFocus(ctrl || shift);
        return ;
    case Qt::Key_Home:
        slotSearchFirst(!alt);
        adjustFocus(ctrl || shift);
        return ;
    case Qt::Key_End:
        slotSearchLast(!alt);
        adjustFocus(ctrl || shift);
        return ;
    case Qt::Key_F3:
        if (shift)
            if (ctrl)
                slotSearchFirst(true);
            else
                slotSearchPrev(true);
        else
            if (ctrl)
                slotSearchLast(true);
            else
                slotSearchNext(true);
        adjustFocus(false);
        return ;
    }

    return QWidget::keyPressEvent(event);
}

void EditorSearchWidget::adjustFocus(bool aim_focus)
{
    if (!aim_focus)
        key_editor->setFocus();
    else
        // 选中关键词并且高亮
        aim_editor->setFocus();
}

void EditorSearchWidget::slotSearchKeyChanged()
{
    if (aim_editor != nullptr && key_editor->text().isEmpty())
        aim_editor->setSearchKey("");

    if (aim_editor == nullptr || key_editor->text().isEmpty())
    {
        prev_btn->setEnabled(false);
        next_btn->setEnabled(false);
        replace_prev_btn->setEnabled(false);
        replace_next_btn->setEnabled(false);
        replace_all_btn->setEnabled(false);
        return ;
    }

    QString key = key_editor->text();
    QString text = aim_editor->toPlainText();
    int pos = aim_editor->textCursor().position();

    // 判断前面有没有
    int find = text.lastIndexOf(key, (pos>0&&pos==text.length())?pos-1:pos);
    if (find > -1 && find >= pos-key.length()) // 光标正在当前的位置
        find = text.lastIndexOf(key, find-1);
    prev_btn->setEnabled(find != -1);
    replace_prev_btn->setEnabled(find != -1);

    // 判断后面有没有
    find = text.indexOf(key, pos);
    if (find == pos) // 光标正在当前的位置
        find = text.indexOf(key, find+key.length());
    next_btn->setEnabled(find != -1);
    replace_next_btn->setEnabled(find != -1);

    replace_all_btn->setEnabled(replace_prev_btn->isEnabled() || replace_next_btn->isEnabled());

    aim_editor->setSearchKey(key); // 设置文字高亮和支持快捷键搜索
}

void EditorSearchWidget::setCursorPosition(int position, int sel)
{
    aim_editor->smoothScrollTo(position, sel, -2);
}

void EditorSearchWidget::slotSearchPrev(bool sel)
{
    if (key_editor->text().isEmpty())
        return ;

    QString key = key_editor->text();
    if (aim_editor == nullptr)
    {
        emit signalSearchPrev(key);
        return ;
    }
    int find = aim_editor->actionSearchPrev(sel);
    if (find == -1)
        prev_btn->setEnabled(false);
    prev_btn->setHover();
}

void EditorSearchWidget::slotSearchNext(bool sel)
{
    if (key_editor->text().isEmpty())
        return ;

    QString key = key_editor->text();
    if (aim_editor == nullptr)
    {
        emit signalSearchPrev(key);
        return ;
    }
    int find = aim_editor->actionSearchNext(sel);
    if (find == -1)
        next_btn->setEnabled(false);
    next_btn->setHover();
}

void EditorSearchWidget::slotSearchFirst(bool sel)
{
    if (!aim_editor || key_editor->text().isEmpty())
        return ;

    aim_editor->actionSearchFirst();
}

void EditorSearchWidget::slotSearchLast(bool sel)
{
    if (!aim_editor || key_editor->text().isEmpty())
        return ;

    aim_editor->actionSearchLast();
}

void EditorSearchWidget::slotSearchThis(bool sel)
{
    if (key_editor->text().isEmpty())
        return ;

    QString key = key_editor->text();
    if (aim_editor == nullptr)
    {
        emit signalSearchPrev(key);
        return ;
    }
    QString text = aim_editor->toPlainText();
    int pos = aim_editor->textCursor().position();
    int find = text.indexOf(key, pos);
    if (find == -1)
        next_btn->setEnabled(false);
    else
    {
        setCursorPosition(find, sel?key.length():0);
    }
}

void EditorSearchWidget::slotReplacePrev()
{
    if (key_editor->text().isEmpty())
        return ;
    if (aim_editor == nullptr)
        return ;

    aim_editor->setReplaceKey(replace_key_editor->text());
    aim_editor->actionReplacePrev();
}

void EditorSearchWidget::slotReplaceNext()
{
    if (key_editor->text().isEmpty())
        return ;
    if (aim_editor == nullptr)
        return ;

    aim_editor->setReplaceKey(replace_key_editor->text());
    aim_editor->actionReplaceNext();
}

void EditorSearchWidget::slotReplaceAll()
{
    if (key_editor->text().isEmpty())
        return ;
    if (aim_editor == nullptr)
        return ;

    aim_editor->setReplaceKey(replace_key_editor->text());
    aim_editor->actionReplaceAll();

    adjustFocus(true);
}
