#include "chaptereditor.h"

ChapterEditor::ChapterEditor(QWidget *parent)
    : QTextEdit(parent)
{
    initEditor();
}

ChapterEditor::ChapterEditor(EditType type, QWidget *parent)
    : QTextEdit(parent)
{
    editing.setType(type);
    initEditor();
}

ChapterEditor::ChapterEditor(QWidget *parent, QString novel_name, QString chapter_name, QString full_chapter_name)
    : QTextEdit(parent)
{
    initEditor();

    openChapter(novel_name, chapter_name, full_chapter_name);
}

ChapterEditor::ChapterEditor(QWidget *parent, EditType type, QString novel_name, QString chapter_name, QString show_name)
    : QTextEdit(parent)
{
    initEditor();

    if (type == EDIT_TYPE_CHAPTER)
        openChapter(novel_name, chapter_name, show_name);
}

ChapterEditor::~ChapterEditor()
{
    save();
}

void ChapterEditor::initEditor()
{
    // ==== 初始化按键和内容的flag ====
    ctrling = false;
    shifting = false;
    alting = false;
    changed_by_redo_undo = false;
    _flag_dont_save = true;
    _flag_is_opening = false;
    last_operator = OPERATOR_NULL;
    completer_case = 0;
    predict_completer = nullptr;
    is_editing_end = false;
    pre_scroll_position = 0;
    pre_scroll_bottom_delta = 0; // 没必要
    pre_scroll_bottom_delta_by_analyze = -1;
    pre_scroll_viewpoer_top_by_analyze = -1;
    document_height = -1;
    editor_height = -2;  // textChange 里面有个 document_height+viewport_margin_bottom <= editor_height，不能一开始成立
    viewport_margin_bottom = 0;
    _flag_is_not_range_changed = false;
    pre_scrollbar_max_range = 0;
    pre_scroll_viewport_top = 0;
    _flag_should_update = false;  // 不需要更新编辑器样式
    _flag_user_change_cursor = false; // 用户修改光标为假
    search_key = "";
    search_regex = false;
    _flag_highlighting = false;
    underline_progress = 0;
    last_pressed_key = -1;
    edit_timestamp = 0;
    blur_title = nullptr;
    p_blur_win_bg = nullptr;
    blur_effect = us->editor_fuzzy_titlebar/* && !editing.isCompact()*/;

    connect(us, &USettings::signalBlurTitlebarChanged, [ = ](bool b)
    {
        if (!getInfo().isChapter())
            return ;

        blur_effect = b;
        int slide_pos = verticalScrollBar()->sliderPosition();
        int min = verticalScrollBar()->minimum();
        verticalScrollBar()->setMinimum(b ? -(us->widget_size) : 0);
        if (!b) // 关闭
        {
            if (!blur_title->isHidden())
            {
                blur_title->hide();
            }
            verticalScrollBar()->setSliderPosition(slide_pos - min);
        }
        else // 开启
        {
            if (blur_effect && verticalScrollBar()->sliderPosition() == 0)
                verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
            if (blur_title != nullptr)
            {
                if (blur_title->isHidden())
                {
                    blur_title->show();
                }
                blurTitleBar();
            }
            verticalScrollBar()->setSliderPosition(slide_pos - us->widget_size);
        }
    });

    // ==== 初始化编辑器 ====
    setUndoRedoEnabled(false);
    setAcceptRichText(false);
    initAnalyze(static_cast<QTextEdit *>(this));

    // ==== 初始化界面 ====
    QString trans("QTextEditor, ChapterEditor{ background-color: transparent; border:none;}"); // 无边框，透明是用来模糊标题栏blur_title的（不然默认淡灰色）
    setStyleSheet(trans);

    if (isFileExist(rt->STYLE_PATH + "edit.qss"))
        setStyleSheet(readTextFile(rt->STYLE_PATH + "edit.qss"));

    if (isFileExist(rt->STYLE_PATH + "scrollbar.qss"))
        verticalScrollBar()->setStyleSheet(readTextFile(rt->STYLE_PATH + "scrollbar.qss"));

    if (blur_effect)
    {
        verticalScrollBar()->setMinimum(-(us->widget_size));
    }

    // ==== 设置信号槽 ====
    connect(this, SIGNAL(textChanged()), this, SLOT(textChange()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorChange()));
    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(copyAva()));
    connect(thm, SIGNAL(editorChanged()), this, SLOT(updateUI()));
    connect(thm, &AppTheme::hlightChanged, [ = ]
    {
        highlighter->rehighlight();
    });

    if (!editing.isBrief() && !editing.isDetail())
    {
        connect(&gd->clm, &CardlibManager::signalRehighlight, [ = ]
        {
            highlighter->rehighlight();
        });
    }

    // ==== 初始化撤销重做 ====
    initUndoRedo();
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollChange()));
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(resetScrollBarRange()));

    // ==== 设置自动提示 ====
    completer = new QCompleter(this);
    completer_listview = new GeneralListView(this);
    completer->setPopup(completer_listview); // 设置自己的ListView，以便可以设置palette
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWidget(this);
    completer->popup()->setMinimumWidth(50);
    completer->popup()->setMaximumWidth(QApplication::desktop()->screenGeometry().width());
    completer->popup()->hide();
    connect(completer, SIGNAL(activated(QString)), this, SLOT(slotOnCompleterActived(QString)));

    completer_timer_timeout = true;
    completer_timer = new QTimer(this);
    completer_timer->setInterval(1000);
    completer_timer->setSingleShot(true);
    connect(completer_timer, SIGNAL(timeout()), this, SLOT(slotCompleterShowed()));

    // ==== 长按左键显示光标所在位置（不含空白符） ====
    mouse_press_timer = new QTimer(this);
    mouse_press_timer->setInterval(500);
    mouse_press_timer->setSingleShot(true);
    connect(mouse_press_timer, &QTimer::timeout, [this]
    {
        mouse_press_timer_timeout = true;
        if (textCursor().hasSelection()) return ;
        int pos = textCursor().position();
        int real_pos = pos;
        for (int i = 0; i < pos; i++)
            if (isBlankChar(_text.mid(i, 1)))
                real_pos--;
        emit signalWordCount(real_pos);
    });

    document_height_timer = new QTimer(this);
    document_height_timer->setInterval(200);
    document_height_timer->setSingleShot(true);
    connect(document_height_timer, &QTimer::timeout, [ = ]
    {
        document_height = static_cast<int>(document()->size().height());
        resetScrollBarRange();
    });

    // ==== 名片 ====
    ctrl_underline_timer = new QTimer(this);
    ctrl_underline_timer->setInterval(300);
    ctrl_underline_timer->setSingleShot(true);
    connect(ctrl_underline_timer, &QTimer::timeout, [ = ]
    {
        slotShowCardUnderline(true);
    });

    // ==== 初始化用户自定义UI ====
    editor_cursor = new EditorCursor(this, this); // 必须要在 updateUI 之前

    // ==== 初始化动画 ====
    smooth_ani = new QPropertyAnimation(this, "smooth_scroll");
    smooth_ani->setEasingCurve(QEasingCurve::OutCirc);

    underline_progress = 0;
    QPropertyAnimation *underline_ani = new QPropertyAnimation(this, "underline_progress");
    underline_ani->setStartValue(1);
    underline_ani->setEndValue(100);
    underline_ani->setDuration(1500);
    underline_ani->start();
    connect(underline_ani, &QPropertyAnimation::finished, [ = ]
    {
        underline_ani->deleteLater();
        underline_progress = 0;
    });

    update_timer = new QTimer(this);
    update_timer->setInterval(20);
    connect(update_timer, SIGNAL(timeout()), this, SLOT(update()));

    // ==== 初始化高亮 ====
    highlighter = new MyHightlighter(document());
    highlighter->setInputManager(input_manager);

    // ==== 初始化右键菜单 ====
    initMenu();

    // ==== 最终界面设置 ====
    updateUI();
}

void ChapterEditor::updateUI()
{
    // 修改之前保存一些数值，修改之后还原
    int old_cursor_height = cursorRect().top(); // 保存光标位置

    // 设置颜色 edit
    QPalette palette = this->palette();
    if (us->editor_font_color.alpha() > 0)
    {
        palette.setColor(QPalette::Text, us->editor_font_color); // 设置字体颜色（开启输入动画时，此项无用）

        /* 内牛满面！！！研究了一整天的 bug，终于好了！！！当我把 QPlainTextEdit 换成 QTextEdit 时……
        int cursor_pos = textCursor().position();
        int select_start = -1, select_end = -1;
        // int scroll_pos = verticalScrollBar()->sliderPosition(); // 暂时用不到
        if (this->textCursor().hasSelection())
        {
            select_start = this->textCursor().selectionStart();
            select_end = this->textCursor().selectionEnd();
        }
        selectAll();
        palette.setColor(QPalette::Text, us->editor_font_color); // 虽然这句话并没有什么用……但是可以设置光标颜色……
        // palette.setColor(QPalette::WindowText, us->editor_font_color);
        QTextCharFormat f;// = textCursor.charFormat();
        f.setForeground(QBrush(us->editor_font_color));
        QTextCursor textCursor = this->textCursor();
        textCursor.setCharFormat(f);
        textCursor.setPosition(cursor_pos);
        if (select_start > -1 && select_end > -1)
        {
            if (cursor_pos == select_start)
                select_start = select_end;
            textCursor.setPosition(select_start, QTextCursor::MoveAnchor);
            textCursor.setPosition(cursor_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(textCursor);
        // verticalScrollBar()->setSliderPosition(scroll_pos); // 不需要也行的*/
    }
    palette.setColor(QPalette::Base, us->editor_bg_color);
    if (us->editor_font_selection.alpha() > 0)
        palette.setColor(QPalette::HighlightedText, us->editor_font_selection);
    palette.setColor(QPalette::Highlight, us->editor_bg_selection);
    this->setPalette(palette);

    thm->setWidgetStyleSheet(this->pop_menu, "menu");


    // 如果开启了输入动画，则要重新设置一下所有的文字颜色
    if (!editing.isCompact() && us->input_animation)
    {
        int ori_position = this->textCursor().position();
        this->selectAll();
        QTextCursor tc = this->textCursor();
        QTextCharFormat fm = tc.charFormat();
        fm.setForeground(QBrush(us->editor_font_color));
        tc.setCharFormat(fm);
        tc.setPosition(ori_position);
        this->setTextCursor(tc);
    }

    // 设置自动提示的颜色 completer
    /*QPalette palette2 = completer_listview->palette();
    if (us->editor_font_color.alpha() > 0)
        palette2.setColor(QPalette::Text, us->editor_font_color);
    palette2.setColor(QPalette::Base, us->editor_bg_color);
    if (us->editor_font_selection.alpha() > 0)
        palette2.setColor(QPalette::HighlightedText, us->editor_font_selection);
    palette2.setColor(QPalette::Highlight, us->editor_bg_selection);
    completer_listview->setPalette(palette2);*/
    completer->setMaxVisibleItems(us->completer_maxnum);

    // 设置字体
    QFont font(this->font());
    font.setPointSize(us->editor_font_size);
    this->setFont(font);

    // 设置光标粗细
    this->setCursorWidth(us->editor_cursor_width);
    editor_cursor->styleChanged(cursorWidth(), fontMetrics().height(), us->getReverseColor(us->mainwin_bg_color, false)/*us->editor_font_color*/);
    input_manager->setColor(us->editor_font_color);

    /* // 设置行间字体（无效 或者 闪退） formt
    QTextCursor cursor = this->textCursor();
    QTextBlockFormat textBlockFormat;
    textBlockFormat.setLineHeight(70, QTextBlockFormat::FixedHeight); // 设置固定行高
    textBlockFormat.setBottomMargin(10); // 设置两行之间的空白高度
    textBlockFormat.setProperty(QTextFormat::OutlinePen, 10);
    cursor.setBlockFormat(textBlockFormat);
    this->setTextCursor(cursor);*/

    // 设置光标所在行的背景颜色 line
    highlightCurrentLine();

    QString scroll_style = thm->getStyleSheet("editor_scrollbar");
    if (!scroll_style.isEmpty())
    {
        this->verticalScrollBar()->setStyleSheet(scroll_style);
    }

    highlightAll();

    // 还原之前记录的数值
    if (cursorRect().top() != old_cursor_height) // 表示修改了字体大小
    {
        document_height = static_cast<int>(document()->size().height());
        resetScrollBarRange();

        pre_scroll_viewport_top = old_cursor_height;
        scrollCursorFix();
    }
}

void ChapterEditor::updateWordCount()
{
    emit signalWordCount(getWordCount(toPlainText()));
}

void ChapterEditor::slotEditorBottomSettingsChanged()
{
    if (us->scroll_bottom_blank <= 0)
        ;
    else if (us->scroll_bottom_blank <= 10)
        viewport_margin_bottom = editor_height / us->scroll_bottom_blank;
    else
        viewport_margin_bottom = us->scroll_bottom_blank;

    resetScrollBarRange();
}

void ChapterEditor::updateEditorMarked()
{
    _flag_should_update = true;
}

/**
 * 更新一些来自其他editor的修改信息，比如字体大小、名片库颜色等
 * @brief ChapterEditor::updateEditor
 */
void ChapterEditor::updateEditor()
{
    if (_flag_should_update == false)
        return ;
    _flag_should_update = false;

    QFont font = this->font();
    font.setPointSize(us->editor_font_size);
    this->setFont(font);
}

bool ChapterEditor::isSaved()
{
    return editing.getSavedText() == this->toPlainText();
}

bool ChapterEditor::save()
{
    if (editing.getPath().isEmpty() || isSaved()) return false;

    writeTextFile(editing.getPath(), this->toPlainText());
    editing.setSavedText(toPlainText()); // 注意：先析构ac，再析构editor（触发强制保存），所以会出现溢出！！！

    ac->addChapter(editing.getNovelName(), editing.getChapterName());
    return true;
}

void ChapterEditor::reloadContent()
{
    QString text = toPlainText();
    QString content = readTextFile(getInfo().getPath());
    if (content != text)
    {
        if (us->save_power) // 有自动保存，那么就是外部更改的
            setPlainText(content);
        else // 未自动保存，询问是否重新加载
        {
            if (edit_timestamp < QFileInfo(getInfo().getPath()).lastModified().toMSecsSinceEpoch() // 文件修改时间
                    && ( us->getBool("ask/reload_chapter") || QMessageBox::information(this, "内容更改", "章节内容已经变更，是否重新加载新的内容？", QMessageBox::Ok, QMessageBox::No) == QMessageBox::Ok) )
                setPlainText(content);
        }
    }
}

void ChapterEditor::openChapter(QString novel_name, QString chapter_name, QString full_chapter_name)
{
    if (!isSaved()) save();

    // 保存章节信息
    editing.setType(EDIT_TYPE_CHAPTER);
    // QString path = NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fnEncode(chapter_name) + ".txt";
    editing.setNames(novel_name, chapter_name);
    //editing.setPath(path);
    editing.setFullChapterName(full_chapter_name);

    // 读取文本内容
    readFromFile(editing.getPath());
}

void ChapterEditor::readFromFile(QString filePath)
{
    if (filePath == "") return ;

    _flag_is_opening = true;
    editing.setPath(filePath);
    QString text = readExistedTextFile(filePath);

    // 判断是不是全空的，如果是，添加段首空行
    /*if (text.isEmpty()) // 不知道为啥放在这个地方的话，默认缩进会没有效果呢？
    {
        int x = us->indent_blank;
        QString ins = "";
        while (x--)
            ins += "　";
        text = ins;
    }*/
    _flag_dont_save = true;
    editing.setSavedText(text);
    this->setPlainText(text);

    // =============================

    // 初始化撤销重做（会先保存一次记录）
    initUndoRedo();

    highlightAll();

    // 读取完毕后直接初始化 _text，不然第一次修改时会从空文本开始计算，就当做改变一大堆字数，从而判断有误
    _text = toPlainText(); // 第一次先初始化，然后第一次

    // emit signalWordCount(getWordCount(_text)); // 在 stackWidget 中 createEdtor 后，此时还没进行 connect
    completer_timer_timeout = true;

    if (blur_effect && verticalScrollBar()->sliderPosition() == 0)
    {
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
    }

    // 第一次cursorchanged事件：打开章节
    // 第二次cursorchanged事件：读取光标位置（启动后第一次打开）
    // 所以需要这个定时器来关闭触发提示事件
    /*QTimer* timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(true);
    connect(timer, QTimer::timeout, [this]{
        completer_timer_timeout = true;
    });
    timer->start();*/

    // 不满一页的时候把光标聚焦到最后面（全空除外）
    // 还有各种延时操作
    QTimer::singleShot(20/*这个定时必须要比stack打开章节延迟设置焦点的timer晚才行*/, [ = ]
    {
        _flag_is_opening = false;
        if (toPlainText().isEmpty()) // 必须延时一下才有效。不过这样也有个好处，就是可以撤销，回到全空的状态
        {
            int x = us->indent_blank;
            QString ins = "";
            while (x--)
                ins += "　";
            this->insertPlainText(ins);
        }

        // 判断一开始焦点的位置
        QTextDocument *document = this->document();
        if (document)
        {
            document_height = static_cast<int>(document->size().height());
            if (document_height < editor_height - (blur_effect ? us->widget_size : 0)) // 可以聚焦到末尾
            {
                QTextCursor cursor = textCursor();
                if (!cursor.hasSelection())
                {
                    QString text = toPlainText();
                    int len = text.length();
                    int pos = len;
                    while (pos > 0 && isBlankChar(text.mid(pos - 1, 1)))
                        pos--;
                    if (pos <= 0 || text.mid(pos - 1, 1) == "\n" || text.mid(pos - 1, 1) == "\r") // 行首
                    {
                        while (pos < len && isBlankChar(text.mid(pos, 1)))
                            pos++;
                    }
                    cursor.setPosition(pos);
                    setTextCursor(cursor); // 带有当前行高亮
                    updateRecorderPosition(pos);
                    //cursorChangedByUser();
                }
                pre_scroll_bottom_delta_by_analyze = -1;
                pre_scroll_viewpoer_top_by_analyze = -1;
                //resetScrollBarRange(); // 判断一开始需不需要滚动条
                completer_timer_timeout = true;
            }
            else // 内容超过一行了
            {
                // 移动到第一行非空白位置
                QTextCursor cur = textCursor();
                if (cur.position() == 0)
                {
                    QString text = toPlainText();
                    int pos = 0;
                    int len = text.length();
                    while (pos < len && isBlankChar2(text.mid(pos, 1)))
                        pos++;
                    if (pos != 0)
                    {
                        cur.setPosition(pos);
                        setTextCursor(cur);
                    }
                }
                highlightCurrentLine();
            }
            resetScrollBarRange();
            if (blur_effect && verticalScrollBar()->sliderPosition() == 0)
            {
                verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
            }
        }
        else if (!toPlainText().isEmpty())
        {
            highlightCurrentLine();
        }
        cursorChangedByUser();
        cursorRealChanged(textCursor().position());
        updateRecorderPosition(textCursor().position());
    });
}

void ChapterEditor::setInitialText(QString text)
{
    setText(text);

    initUndoRedo();

    // 去除当前行高亮（不知道为何没有效果啊）
    /* if (editing.isCompact())
    {
        QList<QTextEdit::ExtraSelection> es;
        QTextEdit::ExtraSelection s;
        s.format.setBackground(us->editor_line_bg_color);
        s.format.setProperty(QTextFormat::FullWidthSelection, true);
        s.cursor = this->textCursor();
        s.cursor.clearSelection();
        es.append(s);
        this->setExtraSelections(es);
    } */
}

void ChapterEditor::removeOpeningFile()
{
    editing = EditingInfo();
}

void ChapterEditor::beforeClose()
{
    if (!isSaved()) save();
}

EditingInfo &ChapterEditor::getInfo()
{
    return this->editing;
}

void ChapterEditor::setTitleBar(QWidget *widget, QPixmap **p_bg)
{
    if ((us->editor_fuzzy_titlebar == false && !blur_effect) || blur_title) return ;

    blur_title = new BlurWidget(us->getInt("us/titlebar_blur_radius", 20), this);
    blur_title->setGeometry(0, 0, width(), us->widget_size);
    this->stackUnder(widget);
    blur_title->stackUnder(widget);
    this->blur_pixmap = new QPixmap(blur_title->size());
    this->p_blur_win_bg = p_bg;
    blur_title->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    blurTitleBar();
}

void ChapterEditor::blurTitleBar()
{
    if (!blur_effect || !blur_title) return ;

    QTimer::singleShot(0, [ = ] // 延时，不然会报错！！！（或者崩溃）
    {
        if (!blur_effect) return ;

        blur_title->hide(); // 不隐藏的话，再次截取的还是之前显示出来的pixmap（因为位置重叠了），必须要先隐藏
        QPixmap pixmap(blur_title->size());
        if (p_blur_win_bg != nullptr)
            pixmap = **p_blur_win_bg;
        else
            pixmap.fill(Qt::transparent);

        QPixmap fonts_pixmap(blur_title->size());
        fonts_pixmap.fill(Qt::transparent);
        render(&fonts_pixmap, QPoint(0, 0), QRect(QPoint(0, 0), blur_title->size()));

        blur_title->setPixmap(pixmap, fonts_pixmap);
        blur_title->show();
    });
}

void ChapterEditor::blurTitleBarFaster()
{
    if (!blur_effect || !blur_title) return ;
    QTimer::singleShot( 0, [ = ]
    {
        blur_title->hide();
        QPixmap fonts_pixmap(blur_title->size());
        fonts_pixmap.fill(Qt::transparent);
        render(&fonts_pixmap, QPoint(0, 0), QRect(QPoint(0, 0), blur_title->size()));
        blur_title->setForePixmap(fonts_pixmap);
        blur_title->show();
    });
}

bool ChapterEditor::execCommand(QString command)
{
    QStringList args;
    if (command.contains(" "))
    {
        int pos = command.indexOf(" ");
        QString arg = command.right(command.length() - pos - 1).trimmed();
        command = command.left(pos);

        args = arg.split(",", QString::SkipEmptyParts);
        for (int i = 0; i < args.size(); i++)
        {
            if (args.at(i) !=  " " && (args.at(i).startsWith(" ") || args.at(i).endsWith(" ")))
                args[i] = args.at(i).trimmed(); // 去掉首尾空（除非本身就是空的）
        }
    }
    return execCommand(command, args);
}

bool ChapterEditor::execCommand(QString command, QStringList args)
{
    if (command == "综合搜索")
    {
        actionComprehensive();
        return true;
    }
    else if (command == "撤销")
    {
        actionUndo();
    }
    else if (command == "重做")
    {
        actionRedo();
    }
    else if (command == "剪切")
    {
        actionCut();
    }
    else if (command == "剪切全文")
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(cursor.position());
        setTextCursor(cursor);
        actionCut();
    }
    else if (command == "剪切全文带标题")
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(cursor.position());
        setTextCursor(cursor);
        actionCut();
        setPlainText("");
    }
    else if (command == "复制")
    {
        actionCopy();
    }
    else if (command == "复制全文")
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(cursor.position());
        setTextCursor(cursor);
        actionCopy();
    }
    else if (command == "复制全文带标题")
    {
        actionCopy();
    }
    else if (command == "复制并保留标记")
    {
        actionCopyWithChapterMark();
    }
    else if (command == "粘贴")
    {
        actionPaste();
    }
    else if (command == "粘贴纯文本")
    {
        actionPasteWithoutTypeset();
    }
    else if (command == "粘贴并排版")
    {
        actionPasteWithTypeset();
    }
    else if (command == "板板")
    {
        actionBoards();
    }
    else if (command == "剪贴板")
    {
        us->setVal("recent/board", 0);
        actionBoards();
    }
    else if (command == "回收板")
    {
        us->setVal("recent/board", 1);
        actionBoards();
    }
    else if (command == "收藏板")
    {
        us->setVal("recent/board", 2);
        actionBoards();
    }
    else if (command == "搜索")
    {
        actionSearch();
        return true; // 避免焦点还在编辑框
    }
    else if (command == "替换")
    {
        actionReplace();
        return true;
    }
    else if (command == "快速替换")
    {
        if (args.size() >= 2)
        {
            setSearchKey(args.at(0));
            setReplaceKey(args.at(1));
            actionReplaceAll();
        }
        else
            msgLostParam(command, "参数1：被替换的文本\n参数2：替换后的文本");
        return true;
    }
    else if (command == "全选")
    {
        actionSelectAll();
    }
    else if (command == "字数统计")
    {
        actionWordCount();
    }
    else if (command == "一键排版" || command == "排版")
    {
        actionTypeset();
    }
    else if (command == "尝试定位到故事线")
    {
        actionLocateOutline();
    }
    else if (command == "定位到目录")
    {
        actionLocateChapter();
    }
    else if (command == "打开上一章")
    {
        actionOpenPrevChapter();
    }
    else if (command == "打开下一章")
    {
        actionOpenNextChapter();
    }
    else if (command == "后面添加章节")
    {
        actionAddNextChapterFaster();
    }
    else if (command == "重复当前章节")
    {
        emit signalAddNextChapterFaster(toPlainText());
    }
    else if (command == "移动到上一章末尾")
    {
        actionMoveToPrevChapterEnd();
    }
    else if (command == "移动到下一章开头")
    {
        actionMoveToNextChapterStart();
    }
    else if (command == "分割后文到新章")
    {
        actionSplitToNextChapter();
    }
    else if (command == "长文章自动分割")
    {
        actionSplitToMultiChapters();
    }
    else if (command == "错词高亮")
    {
        actionErrorWordHighlight();

    }
    else if (command == "词语纠错")
    {
        actionWordCorrect();
    }
    else if (command == "简单美化")
    {
        actionSimpleBeauty();
    }
    else if (command == "简转繁")
    {
        actionSimplifiedToTraditional();
    }
    else if (command == "繁转简")
    {
        actionTraditionalToSimplified();
    }
    else if (command == "敏感词高亮")
    {
        actionSensitiveHighlight();
    }
    else if (command == "敏感词消除")
    {
        actionSensitiveEliminate();
    }
    else if (command == "敏感词替换")
    {
        actionSensitiveReplace();
    }
    else if (command == "编辑敏感词列表")
    {
        actionSensitiveEditList();
    }
    else if (command == "编辑敏感词替换列表")
    {
        actionSensitiveEditReplace();
    }
    else if (command == "滚动屏幕内容")
    {
        if (args.size() >= 1)
            actionScrollContentByHeight(args.at(0).toInt());
        else
            msgLostParam(command, "参数1：滚动比例");
    }
    else if (command == "滚动屏幕文字")
    {
        if (args.size() >= 1)
            actionScrollContentByLines(args.at(0).toInt());
        else
            msgLostParam(command, "参数1：滚动行数");
    }
    else if (command == "移动到上一段")
    {
        actionMoveTextCursorToPrevPara(args.size() && args.at(0) == "选中");
    }
    else if (command == "移动到下一段")
    {
        actionMoveTextCursorToNextPara(args.size() && args.at(0) == "选中");
    }
    else
        return false;
    QTimer::singleShot(100, [ = ]
    {
        setFocus();
    });
    return true;
}

void ChapterEditor::surroundWordSearch()
{
    if (!focusing) return ;
    int pos = textCursor().position();
    QString text = toPlainText();
    if (pos > 2 && (us->auto_similar_sentence || us->auto_predict_sentence))
    {
        QString l1 = text.mid(pos - 1, 1);
        if (isChinese(l1))
        {
            if (us->auto_similar_sentence)
            {
                actionSimilarSentence();
                return ;
            }
        }
        else if (isSentSplit(l1))
        {
            if (us->auto_predict_sentence)
            {
                actionPredictSentence();
                return ;
            }
        }
    }
    int len = text.length();
    int left = pos, right = pos;
    while (left > 0 && (isChinese(text.mid(left - 1, 1)) || isEnglish(text.mid(left - 1, 1)) ))
        left--;
    while (right < len && (isChinese(text.mid(right, 1)) || isEnglish(text.mid(right, 1))) )
        right++;
    if (left == right) // 如果是空的
    {
        completer->popup()->hide();
        return ;
    }
    QString sent = text.mid(left, right - left);

    if (gd->lexicons.surroundSearch(sent, pos - left))
    {
        QStringList list = gd->lexicons.getResult();
        QString key = gd->lexicons.getMatched();

        while (list.removeOne(key)); // 移除当前已经输入的
        QStringListModel *model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width + 10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        completer_case = COMPLETER_CONTENT_WORD;
    }
    else
    {
        completer->popup()->hide();
    }
}

void ChapterEditor::closeCompleteTimerOnce()
{
    completer_timer_timeout = true;
}

/**
 * 设置搜索关键词高亮
 * @param str    要搜索的关键词
 * @param regexp 是否使用正则表达式
 */
void ChapterEditor::setSearchKey(QString key, bool /*regexp*/)
{
    highlighter->setSearchKey(search_key = key);
}

void ChapterEditor::setReplaceKey(QString rep)
{
    replace_key = rep;
}

void ChapterEditor::slotCompleterShowed()
{
    if (!completer_timer_timeout)
    {
        completer_timer_timeout = true;
        if (textCursor().hasSelection())
        {
            completer->popup()->hide();
            return ;
        }
        surroundWordSearch();
    }
}

void ChapterEditor::slotOnCompleterActived(const QString &completion)
{
    if ( gd->lexicons.getMatchedCase() == COMPLETER_CONTENT_SENT || gd->lexicons.getMatchedCase() == COMPLETER_PREDICT_SIMILAR )//completer_case == COMPLETER_CONTENT_SENT)
    {
        if (!us->insert_sentence)
        {
            QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
            clipboard->setText(completion);                  //设置剪贴板内容</span>
            return ;
        }
    }

    if ( completion == "-->" ) // 找到句子
    {
        //completer_case = COMPLETER_CONTENT_SENT;
        QStringList list = gd->lexicons.getSentence();
        QStringListModel *model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width + 10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        return ;
    }
    if (gd->lexicons.getMatchedCase() == COMPLETER_PREDICT_NEXT || gd->lexicons.getMatchedCase() == COMPLETER_PREDICT_SIMILAR)
    {
        this->insertPlainText(completion);
        return ;
    }

    QString key = gd->lexicons.getMatched(); // 匹配到的字符，把这个字符替换成completion
    int pos = textCursor().position();
    QString text = toPlainText();
    int len = text.length();
    int left = pos, right = pos;
    while (left > 0 && (isChinese(text.mid(left - 1, 1)) || isEnglish(text.mid(left - 1, 1)) ))
        left--;
    while (right < len && (isChinese(text.mid(right, 1)) || isEnglish(text.mid(right, 1))) )
        right++;
    if (left == right) return ;
    int start_pos = pos - key.length();
    if (start_pos < left) start_pos = left;
    int find_pos = text.indexOf(key, start_pos);
    if (find_pos == -1) return ;
    operatorWordReplace(find_pos, find_pos + key.length(), completion);

    if ( gd->lexicons.getMatchedCase() != COMPLETER_CONTENT_NAME
            && gd->lexicons.getMatchedCase() != COMPLETER_CONTENT_SURN ) // 随机的话，不再进行随机搜索
        surroundWordSearch();

    // 改变文本后光标会再次改变……所以需要抵消下一次的显示提示框
    completer_timer_timeout = true;
}

void ChapterEditor::slotShowCardUnderline(bool show)
{
    highlighter->setCardUnderlineVisible(is_ctrl_underline = show);
}

void ChapterEditor::slotSubWidgetHidden(QPixmap pixmap, QRect rect)
{
    QPixmap *pm = new QPixmap(pixmap);
    ZoomGeometryAnimationWidget *ani = new ZoomGeometryAnimationWidget(this, pm, rect, cursorRect(), true);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [ = ]
    {
        delete pm;
    });
}

void ChapterEditor::slotShowMotion()
{
    MotionBoard *mcn;
    // mcn = new MotionBoard(this);
    mcn = new MotionContentNumber(this);

    connect(mcn, &MotionBoard::signalMotionTo, this, [ = ](int pos)
    {
        if (pos >= 0 && pos < this->toPlainText().length())
        {
            QTextCursor cursor = textCursor();
            cursor.setPosition(pos);
            setTextCursor(cursor);
            cursorRealChanged(pos);
        }
    });
}

void ChapterEditor::keyPressEvent(QKeyEvent *event)
{
    log2("按下按键", event->key());

    last_operator = OPERATOR_KEY;
    // deb(event->key(), "editor key press event"); // 判断按下了什么键
    auto key = event->key();
    auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;
    ctrling = ctrl;
    shifting = shift;
    alting = alt;
    last_pressed_key = key;

    // 名片提示
    if (gd->clm.currentLib() != nullptr)
    {
        if (key == Qt::Key_Control)
            ctrl_underline_timer->start();
        else
            ctrl_underline_timer->stop();
    }

    // 隐藏下拉提示框
    if (/*completer &&*/ completer->popup()->isVisible())
    {
        if (modifiers == 0) // 没有修饰键
        {
            switch (key)
            {
            case Qt::Key_Escape:
                event->ignore();
                completer->popup()->hide();
                return ;
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
                if (completer->currentIndex().isValid() && completer->popup()->currentIndex().row() > -1)
                {
                    event->ignore();
                    return ;
                }
                completer->popup()->hide();
                break;
            case Qt::Key_Home :
            case Qt::Key_End :
                completer->popup()->hide();
                break;
            default:
                break;
            }
        }
        else
        {
            completer->popup()->hide();
        }
    }

    // 非输入前处理输入内容
    // 此处专门处理被删除的内容
    if (key == Qt::Key_Backspace || key == Qt::Key_Delete)
        beforeInputted("");

    // 执行手动配置的快捷键命令
    QStringList cmds = gd->shortcuts.getShortutCmds(SCK_EDITOR, modifiers, static_cast<Qt::Key>(key));
    if (cmds.size() > 0)
    {
        foreach (QString cmd, cmds)
        {
            execCommand(cmd);
        }
    }
    else
    {
        // 默认快捷键列表，如果没有覆盖的快捷键的话
        switch (key)
        {
        // ========== 方向键 ==========
        case Qt::Key_Up : // 上方向键
            if (ctrl && !shift && !alt) // 屏幕向上滚1行
            {
                actionScrollContentByHeight(-4);
                return ;
            }
            else if (!ctrl && !shift && alt) // 屏幕向上滚5行
            {
                actionScrollContentByHeight(-2);
                return ;
            }
            else if (ctrl && shift && !alt) // 扩选
            {
                activeExpandSelection();
                return ;
            }
            else if (ctrl && alt) // 上一段
            {
                actionMoveTextCursorToPrevPara(shift);
                return ;
            }
            else if (!ctrl && !shift && !alt)
            {
                key_pressed_pos = textCursor().position(); // 保存位置。如果松开按键时位置没变，则添加新段
            }
            _flag_user_change_cursor = true;

            break;
        case Qt::Key_Down : // 下方向键
            if (ctrl && !shift && !alt) // 屏幕向下滚3行
            {
                actionScrollContentByHeight(4);
                return ;
            }
            else if (!ctrl && !shift && alt) // 屏幕向下滚5行
            {
                actionScrollContentByHeight(2);
                return ;
            }
            else if (ctrl && shift && !alt) // 收缩选择
            {
                activeShrinkSelection();
                return ;
            }
            else if (ctrl && alt) // 下一段
            {
                actionMoveTextCursorToNextPara(shift);
                return ;
            }
            else if (!ctrl && !shift && !alt)
            {
                key_pressed_pos = textCursor().position(); // 保存位置。如果松开按键时位置没变，则添加新段
            }
            _flag_user_change_cursor = true;
            break;
        case Qt::Key_Left : // 左方向键
        {
            if (ctrl || alt) // 向左移动一个单位
            {
                actionMoveTextCursorToLeft(ctrl, alt, shift);
                return ;
            }
            _flag_user_change_cursor = true;
            break;
        }
        case Qt::Key_Right : // 右方向键
            if (ctrl || alt) // 向右移动一个单位
            {
                actionMoveTextCursorToRight(ctrl, alt, shift);
                return ;
            }
            _flag_user_change_cursor = true;
            break;
        case Qt::Key_Home :
            _flag_user_change_cursor = true;
            if (!ctrl && !alt)
            {
                operatorHomeKey(shift);
                return ;
            }
            _flag_user_change_cursor = true;
            if (blur_effect && verticalScrollBar()->sliderPosition() == 0)
                verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
            break;
        case Qt::Key_End :
            _flag_user_change_cursor = true;
            break;
        case Qt::Key_PageUp :
            _flag_user_change_cursor = true;
            break;
        case Qt::Key_PageDown :
            _flag_user_change_cursor = true;
            break;
        // ========== 智能标点增强键 =========
        case Qt::Key_Space : // 32 空格键
            if (!textCursor().hasSelection() && us->smart_space)
            {
                activeSmartSpace();
                return ;
            }
            break;
        case Qt::Key_Enter :
        case Qt::Key_Return : // 16777220 回车键
            if (ctrl && shift)   // 添加到上一段
            {
                if (alt)   // 强行添加标点并添加上一段
                {
                    if (textCursor().position() > 0 && isChinese(toPlainText().mid(textCursor().position() - 1, 1)))
                        activeSentFinish();
                }
                operatorAddPrevPara();
                return ;
            }
            else if (ctrl)   // 行末添加到下一段
            {
                if (alt)
                    activeSentFinish();
                operatorAddNextPara();
                return ;
            }
            else if (shift)   // 单纯换行
            {

            }
            else if (us->smart_enter)
            {
                activeSmartEnter();
                activeParaSplit();
                return ;
            }
            break;
        case Qt::Key_Backspace : // 16777219
            if (textCursor().hasSelection()) break;
            if (ctrl && !alt)
            {
                return backspaceWord();
            }
            else if (!ctrl && alt) // 删除句子
            {
                return backspaceSent();
            }
            else if (ctrl && alt) // 删除段落
            {
                return backspacePara();
            }
            else if (!ctrl && !alt && !shift)
            {
                if (us->smart_backspace)
                    return activeSmartBackspace();
            }
            break;
        case Qt::Key_Delete : // 向右删除
            if (textCursor().hasSelection() || shift) break;
            if (ctrl && !alt)
            {
                return deleteWord();
            }
            else if (!ctrl && alt) // 删除句子
            {
                return deleteSent();
            }
            else if (ctrl && alt) // 删除段落
            {
                return deletePara();
            }
            else if (!ctrl && !shift && !alt)
            {
                //                if (us->smart_backspace) // TODO
                //                    return operatorSmartDelete();
            }
            break;
        case Qt::Key_Tab : // tab补全、tab跳过
            if (!shift)
            {
                if (us->tab_complete && us->tab_skip)
                {
                    return operatorTabSkip(operatorTabComplete());
                }
                else if (us->tab_skip)
                {
                    return operatorTabSkip(false);
                }
            }
            else // shift
            {
                if (us->tab_skip)
                {
                    return operatorReverseTabSkip();
                }
            }
            break;
        case Qt::Key_Backtab : // shift + tab
            if (us->tab_skip)
            {
                return operatorReverseTabSkip();
            }
            break;
        // ========== 快捷键 ===========
        case Qt::Key_T :
            if (ctrl)   // ctrl+T 排版 typography
            {
                operatorTypeset();
                return ;
            }
            break;
        case Qt::Key_D :
            if (ctrl)   // ctrl+D 强制覆盖 homonym
            {
                activeHomonymCoverForce();
                return ;
            }
            break;
        case Qt::Key_J :
            if (ctrl)
            {
                slotShowMotion();
                return;
            }
            break;
        case Qt::Key_Less :
            if (ctrl)
            {
                actionSimilarSentence();
                return;
            }
            break;
        case Qt::Key_Greater :
            if (ctrl)
            {
                actionPredictSentence();
                return;
            }
            break;
        case Qt::Key_Escape : // ESC
        case Qt::Key_Close :
            emit signalHidePanels();
            break;
        case Qt::Key_F3:
            if (shift)
                if (ctrl)
                    actionSearchFirst();
                else
                    actionSearchPrev();
            else if (ctrl)
                actionSearchLast();
            else
                actionSearchNext();
            break;
        case Qt::Key_New :
            break;
        case Qt::Key_Open :
            break;
        case Qt::Key_S :
        case Qt::Key_Save :
            save();
            break;
        case Qt::Key_Undo : // 已经在本控件的构造函数中被禁用了
            undoOperator();
            return ;
        case Qt::Key_Redo :
            redoOperator();
            return ;
        case Qt::Key_Z :
            if (ctrl)
            {
                undoOperator();
                return ;
            }
            break;
        case Qt::Key_Y :
            if (ctrl)
            {
                redoOperator();
                return ;
            }
            break;
        case Qt::Key_Search:
            actionSearch();
            return ;
        case Qt::Key_F :
            if (ctrl)
            {
                actionSearch();
                return ;
            }
            break;
        case Qt::Key_H :
            if (ctrl)
            {
                actionReplace();
                return ;
            }
            break;
        case Qt::Key_B :
            if (ctrl)
            {
                actionBoards();
                return ;
            }
            break;
        case Qt::Key_Copy:
            actionCopy();
            return ;
        case Qt::Key_C:
            if (ctrl)
            {
                actionCopy();
                return ;
            }
            break;
        case Qt::Key_Cut:
            actionCut();
            return ;
        case Qt::Key_X:
            if (ctrl)
            {
                actionCut();
                return ;
            }
            break;
        case 16777248 : // shift
            shifting = true;
            break;
        case 16777249 : // ctrl
            ctrling = true;
            break;
        case 16777251 : // alt
            alting = true;
            break;
        }

        // 此处专门处理选中文本被其他可输入文本覆盖的情况
        if (!ctrl && !shift && !alt)
        {
            if (key >= 0x20 && key <= 0xff) // 可输入的字符（即 InputMethod 之外可输入的内容）
                beforeInputted("");
        }
    }

    // 执行完毕后记录光标的手动变化情况
    if (key == Qt::Key_Up || key == Qt::Key_Down)
    {
        // 保存位置。如果松开按键时位置没变，则添加新段
        if (!ctrl && !shift && !alt)
        {
            key_pressed_pos = textCursor().position();
        }
        else
        {
            key_pressed_pos = -1;
        }
        _flag_user_change_cursor = true;
    }
    else if (key == Qt::Key_Left || key == Qt::Key_Right)
    {
        if (completer->popup()->isVisible())
        {
            if (completer->popup()->currentIndex().row() == -1)
                completer->popup()->hide();
            surroundWordSearch();
            last_operator = OPERATOR_IMD_COMPLETER;
        }
        _flag_user_change_cursor = true;
    }
    else if (key == Qt::Key_Home || key == Qt::Key_End)
    {

        _flag_user_change_cursor = true;

        if (blur_effect && verticalScrollBar()->sliderPosition() == 0)
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->minimum());
    }
    else if ( key == Qt::Key_PageUp || key == Qt::Key_PageDown)
    {

        _flag_user_change_cursor = true;
    }

    if (cmds.size() > 0)
        return ; // 已经覆盖，不执行TextEdit的方法了

    QTextEdit::keyPressEvent(event);
}

void ChapterEditor::keyReleaseEvent(QKeyEvent *event)
{
    log2("松开按键", event->key());

    last_operator = OPERATOR_KEY;
    last_pressed_key = -1;

    auto key = event->key();
    auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;

    // 取消名片下划线
    if (is_ctrl_underline)
    {
        slotShowCardUnderline(false);
    }
    else if (ctrl_underline_timer->isActive())
    {
        ctrl_underline_timer->stop();
    }

    switch (key)
    {
    case Qt::Key_Up :
        if (!ctrl && !shift && !alt
                && key_pressed_pos == textCursor().position() // 如果光标没有变化，则手动判断
                && !isAtBlankLine2() // 第一段空行不添加段落，但是会判断为光标没有变，从而再次添加空行
           )
        {
            operatorSmartKeyUp();
            break;
        }
    case Qt::Key_Down :
        if (!ctrl && !shift && !alt && key_pressed_pos == textCursor().position()) // 如果光标没有变化，则手动判断
        {
            operatorSmartKeyDown();
            break;
        }
    case Qt::Key_Left :
    case Qt::Key_Right :
    case Qt::Key_Home :
    case Qt::Key_End :
    case Qt::Key_PageUp :
    case Qt::Key_PageDown :
        updateRecorderPosition(textCursor().position());
        cursorChangedByUser();
        _flag_user_change_cursor = false;
        break;
    case Qt::Key_Return :
    case Qt::Key_Enter :
        is_editing_end = isAtEditEnd();
        //if (!editing.isCompact() && us->cursor_animation)
        //cursorRealChanged(); // 在这里用，会导致撤销时光标位置出问题……
        break;
    case 16777248 : // shift
        shifting = false;
        break;
    case 16777249 : // ctrl
        ctrling = false;
        break;
    case 16777251 : // alt
        alting = false;
        break;
    }

    QTextEdit::keyReleaseEvent(event);
}

/**
 * 输入中文事件
 * 也只能检测到中文或者中文标点，数字、字母、半角标点、换行等全都检测不到
 */
void ChapterEditor::inputMethodEvent(QInputMethodEvent *event)
{
    log2("输入中文", event->commitString());
    ac->addUserWords(event->commitString());
    last_operator = OPERATOR_CN;
    QString in = event->commitString();

    // 禁止微软拼音的打字字母，避免字母bug
    /*if (us->ban_microsoft_im && canRegExp(in, "\\w('\\w+)+"))
    {
        return ;
    }*/

    if (in == "“" || in == "”")
    {
        if (us->smart_quotes)
        {
            if (this->textCursor().hasSelection())
                activeSmartQuotes2();
            else
                activeSmartQuotes();
            return ;
        }
    }
    else if (in == "‘" || in == "’")
    {
        if (us->change_apos)
        {
            if (this->textCursor().hasSelection())
                activeSmartQuotes2();
            else
                activeSmartQuotes();
            return ;
        }
    }

    beforeInputted(in);

    if (isChinese(in))
    {
        _input_chinese_text = in;
        _input_chinese_pos = _edit->textCursor().position() + in.length();
        _input_chinese_buffer = true;
    }
    else
    {
        _input_chinese_text = "";
        _input_chinese_pos = -1;
        _input_chinese_buffer = false;
    }

    // 判断未被用到的名片
    if (gd->clm.currentLib() != nullptr)
    {
        QtConcurrent::run([ = ]()
        {
            foreach (CardBean *card, gd->clm.currentLib()->unused_cards)
            {
                QRegExp re(card->pattern());
                if (re.indexIn(in) > -1) // 输入了这个本来没用到的名片
                {
                    // gd->clm.currentLib()->useCard(card); // 多线程中不能直接操作UI，只能通过信号槽
                    emit gd->clm.signalUseCardByEdit(card);
                }
            }
            foreach (CardBean *card, gd->clm.currentLib()->using_cards)
            {
                if (gd->clm.currentLib()->editing_cards.contains(card))
                    continue;
                QRegExp re(card->pattern());
                if (re.indexIn(in) > -1) // 输入了这个名片
                {
                    // gd->clm.currentLib()->addEditingCard(card);
                    emit gd->clm.signalEditCardByEdit(card);
                }
            }
        });
    }


    // 输入文字提示
    delayCompleter(us->completer_input_delay);
    QTextEdit::inputMethodEvent(event);
}

void ChapterEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;

    QTextEdit::mouseDoubleClickEvent(e);
}

void ChapterEditor::mouseMoveEvent(QMouseEvent *e)
{
    if (waters.size() > 0 && waters.last().release_timestamp == 0)
    {
        // 此项开启移动关闭
        if ( (e->pos() - waters.last().point).manhattanLength() >= QApplication::startDragDistance() )
            waters.last().release_timestamp = getTimestamp();

        // 此项开启移动
        // waters.last().point = e->pos();
    }

    QTextEdit::mouseMoveEvent(e);
}

void ChapterEditor::mousePressEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;
    _flag_user_change_cursor = true;

    // 长按显示光标位置
    mouse_press_timer_timeout = false;
    if (e->button() == Qt::LeftButton)
        mouse_press_timer->start();

    // 鼠标点击水波纹动画
    if (us->editor_water_effect)
    {
        waters << Water(e->pos(), getTimestamp());
#if not defined(Q_OS_WIN)
        if (e->button() == Qt::RightButton)
            waters.last().release_timestamp = getTimestamp();
#endif
        update_timer->start();
    }

    QTextEdit::mousePressEvent(e);

    // 显示鼠标名片或者其他内容
    if (ctrling || QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        showCardlibEditor();
    }
}

void ChapterEditor::mouseReleaseEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;
    _flag_user_change_cursor = false;

    QTextCursor textCursor = this->textCursor();
    int position = textCursor.position();
    updateRecorderPosition(position);
    cursorChangedByUser();

    if (is_ctrl_underline)
    {
        slotShowCardUnderline(false);
    }
    else if (ctrl_underline_timer->isActive())
    {
        ctrl_underline_timer->stop();
    }

    if (textCursor.hasSelection())
        cursorRealChanged(-2);

    if (mouse_press_timer_timeout)
    {
        mouse_press_timer_timeout = false;
        if (!textCursor.hasSelection())
        {
            emit signalWordCount(getWordCount(_text));
        }
    }
    else
    {
        mouse_press_timer->stop();
    }

    if (us->editor_water_effect && waters.size())
    {
        if (waters.last().release_timestamp == 0) // 双击不会识别 press 事件
            waters.last().release_timestamp = getTimestamp();
    }

    QTextEdit::mouseReleaseEvent(e);
}

void ChapterEditor::wheelEvent(QWheelEvent *e)
{
    // 隐藏自动提示
    if (completer->popup()->isVisible() && completer->popup()->currentIndex().row() < 0)
    {
        completer->popup()->hide();
    }

    if (ctrling || QApplication::keyboardModifiers() == Qt::ControlModifier) // 修改字体大小
    {
        int old_cursor_top = cursorRect().top(); // 保存光标位置

        // 修改字体
        int size = this->font().pointSize();
        if (e->delta() > 0) // 滚轮往前转
        {
            if (size > 1)
            {
                if (size > 30)
                    size *= 1.1;
                else
                    size++;
            }
        }
        else if (e->delta() < 0)
        {
            if (size < 200)
            {
                if (size > 20)
                    size *= 0.8;
                else if (size > 3)
                    size--;
            }
        }
        QFont font = this->font();
        font.setPointSize(size);
        this->setFont(font);
        saveFontSize();

        // 还原光标进度（按比例）
        pre_scroll_viewport_top = old_cursor_top;
        scrollCursorFix();

        e->accept();
    }
    else if (shifting || QApplication::keyboardModifiers() == Qt::ShiftModifier) // 扩大选择、收缩选择
    {
        if (e->delta() > 0) // 滚轮往前转
        {
            activeExpandSelection();
        }
        else if (e->delta() < 0)
        {
            activeShrinkSelection();
        }

        e->accept();
    }
    else if (!ctrling) // 普通的上下滚动
    {
        if (us->smooth_scroll)
        {
            if (e->delta() > 0) // 上滚
            {
                if (verticalScrollBar()->sliderPosition() > verticalScrollBar()->minimum())
                {
                    addSmoothScrollThread(-us->smooth_scroll_speed, us->smooth_scroll_duration);
                    e->accept();
                }
            }
            else if (e->delta() < 0)
            {
                if (verticalScrollBar()->sliderPosition() < verticalScrollBar()->maximum())
                {
                    addSmoothScrollThread(us->smooth_scroll_speed, us->smooth_scroll_duration);
                    e->accept();
                }
            }

            e->accept();
        }
        else
        {
            QTextEdit::wheelEvent(e);

            cursorRealChanged();
        }

        if (!editing.isCompact() && us->cursor_animation)
            editor_cursor->setStartPosition(-1); // 直接跳转，避免光标移动动画
    }

}

void ChapterEditor::showEvent(QShowEvent *e)
{
    updateEditor();

    return QTextEdit::showEvent(e);
}

void ChapterEditor::resizeEvent(QResizeEvent *e)
{
    int old_cursor_top = cursorRect().top(); // 保存光标位置
    int old_editor_height = e->oldSize().height(); // 保存旧版的窗口大小
    if (old_editor_height < 0)
        old_editor_height = height();

    QTextEdit::resizeEvent(e); // 进行真正的大小改变

    editor_height = e->size().height();
    document_height = static_cast<int>(document()->size().height());

    // 调整底部行空白的数值
    //document_height_timer->stop();
    //document_height_timer->start();
    if (us->scroll_bottom_blank <= 0)
        ;
    else if (us->scroll_bottom_blank == 1) // 滚动超过屏幕高度时，会出现滚动的问题（比如删除行）
        viewport_margin_bottom = static_cast<int>(editor_height * 0.8); // - verticalScrollBar()->singleStep();
    else if (us->scroll_bottom_blank <= 10)
    {
        viewport_margin_bottom = editor_height / us->scroll_bottom_blank;
    }
    else
    {
        viewport_margin_bottom = us->scroll_bottom_blank;
        if (viewport_margin_bottom > editor_height * 0.8)
            viewport_margin_bottom = static_cast<int>(editor_height * 0.8);
    }

    // 保存滚动位置
    //    pre_scroll_viewport_top = old_cursor_pos; // 设置为和之前光标距离顶部一样高的
    pre_scroll_viewport_top = editor_height * old_cursor_top / old_editor_height; // 按照之前光标所在的比例设置位置

    if (!_flag_is_opening) // 打开章节时不进行修改
        scrollCursorFix(); // 固定光标位置
    if (cursorRect().top() > editor_height && (old_cursor_top < old_editor_height)) // bug：重复定位一次（侧边栏显示时会出现这个问题）
    {
        QTimer::singleShot(0, [ = ]
        {
            pre_scroll_viewport_top = editor_height *old_cursor_top / old_editor_height;
            scrollCursorFix();
        });
    }
    resetScrollBarRange(); // 保持底部空白的高度

    if (blur_effect && blur_title != nullptr)
    {
        blur_title->setGeometry(0, 0, width() - verticalScrollBar()->width(), us->widget_size);
        blurTitleBar();
    }

    cursorRealChanged(); // 修改光标动画位置
    // scrollCursorFix(); // <无效> 不知道要怎么才能保持滚动的位置？

    //resetScrollBarRange(); // 应该会触发范围改变事件，就不用多此一举了呢
    // return QTextEdit::resizeEvent(e);
}

void ChapterEditor::focusInEvent(QFocusEvent *e)
{
    focusing = true;
    QTextEdit::focusInEvent(e);
}

void ChapterEditor::focusOutEvent(QFocusEvent *e)
{
    focusing = false;
    ctrling = shifting = alting = false;
    if (is_ctrl_underline)
    {
        slotShowCardUnderline(false);
    }
    else if (ctrl_underline_timer->isActive())
    {
        ctrl_underline_timer->stop();
    }
    QTextEdit::focusOutEvent(e);
}

void ChapterEditor::insertFromMimeData(const QMimeData *source)
{
    beforeInputted("");

    if (shifting && ctrling); // ctrl+shift+v 手动格式化粘贴
    else if (!us->typeset_paste)
        return QTextEdit::insertFromMimeData(source);

    /**
     * 智能粘贴：
     * - 文字排版
     * - 文件读取内容
     */

    if (source->hasText())
    {
        QString context = source->text();
        if (this->textCursor().hasSelection()) // 已经选择一部分，直接覆盖，不进行任何其他操作
            return QTextEdit::insertFromMimeData(source);

        // 文件路径格式
        if ( context.startsWith("file:///") && context.length() < 256 && (context.endsWith(".txt") || context.endsWith(".xml")))
        {
            QString file_path = context.right(context.length() - 8);
            if (isFileExist(file_path))
            {
                QString file_content = readTextFile(file_path);
                if (!file_content.isEmpty())
                {
                    int start = textCursor().position();
                    int end = start + file_content.length();
                    QTextEdit::insertPlainText(file_content);
                    if (file_content.indexOf("\n") > -1 && !isBlankString(file_content) && file_content.length() < us->no_operator_mime_length)
                        operatorTypesetPart(start, end);
                    return ;
                }
            }
        }

        // 粘贴
        int start = textCursor().position();
        int end = start + context.length();
        QTextEdit::insertFromMimeData(source);
        if (context.contains("\n") && !isBlankString(context) && context.length() < us->no_operator_mime_length)
        {
            operatorTypesetPart(start, end);

            // 粘贴排版是真的
            if (us->getBool("ask/paste_typeset", true))
            {
                us->setVal("ask/paste_typeset", false);
                NotificationEntry *noti = new NotificationEntry("paste_typeset", "粘贴自动排版", "粘贴多段文本进行格式化");
                noti->setBtns("撤销", "关闭排版");
                rt->popNotification(noti);
                connect(noti, &NotificationEntry::signalBtnClicked, [ = ](int x)
                {
                    undoOperator();
                    if (x == 2)
                        us->setVal("us/typeset_paste", us->typeset_paste = false);
                });
            }
        }
        // typesetPaste(this->textCursor().position(), context);
    }

}

void ChapterEditor::contextMenuEvent(QContextMenuEvent *event)
{
    refreshMenu();
    pop_menu->exec(QCursor::pos());
    event->accept();
}

bool ChapterEditor::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        if (gd->clm.currentLib() != nullptr)
        {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            showCardlibToolTip(helpEvent->pos());
        }
        else
        {
            event->ignore();
        }
        return true;
    }
    return QTextEdit::event(event);
}

void ChapterEditor::cutRecorderPosition()
{
    if (recorders_pos > 1 && recorders_pos == recorders.size())
    {
        recorders_pos--;
        recorders.removeLast();
    }
}

void ChapterEditor::setTextInputColor()
{
    if (editing.isCompact() || !us->input_animation)
        return;
    // isNotInput(); // 已经转到 input_manager 对象里面的* _flag_is_not_input 了
    _flag_is_not_scrolled = true; // 因为里面有 setTextCursor，导致滚动条混乱
    input_manager->updateTextColor(_pos);
}

void ChapterEditor::textChange()
{
    if (_flag_is_analyzing) return ; // 避免死循环
    if (_flag_highlighting) return ; // 文本高亮中

    // ==== 滚动条高度调整 ====
    // 这一段之所以放到这里而不是 rangeChanged 信号槽里，是因为 rangeChanged 会经常蜜汁触发
    if (document_height + viewport_margin_bottom <= editor_height
            && (document_height = static_cast<int>(document()->size().height())) + viewport_margin_bottom > editor_height)
    {
        resetScrollBarRange();
        verticalScrollBar()->show();
    }
    document_height = static_cast<int>(document()->size().height()); // 要是用两层 if else ，避免重复获取高度，不知道能不能提高效率？

    //resetScrollBarRange();
    if (_flag_dont_save)   // 无需理会：打开新章时
    {
        _flag_dont_save = false;
        return ;
    }

    if (toPlainText() == _text && !_change_text) // 文本没有改变（不是用户主动输入的，finishAnalyze也不算改变，还有字符格式修改等）
    {
        return ;
    }

    /*if (_flag_is_cursor_change)
    {
        _flag_is_cursor_change = false;
        return ;
    }*/

    // 撤销重做引起的改变。TextAnalyze中，已经加入了flag_isNotInput
    if (changed_by_redo_undo == false)
    {
        // 保存撤销重做
        // 注意，这个的光标位置是 0 的
        recorderOperator();
    }

    // 修改中文输入
    if (_input_chinese_buffer) // 这个是用来缓冲的，抵消这一次中文输入的文本改变事件
    {
        _input_chinese_buffer = false;
    }
    else // 不是紧接在输入中文之后的文本改变，直接设置为没有输入中文
    {
        _input_chinese_pos = -1;
    }

    // ==== AI判断 ====
    // 如果不是 finishAnalyze 的话，则进行文本分析
    if (!_change_text)
    {
        _flag_is_analyzing = true;
        textAnalyze();

#ifdef TEXT_ANALYZE_MODE2
        // 如果有文字改变，则进行
        if (_change_text)
            recorderOperator();
        else if (_change_pos)
            updateRecorderPosition(_pos);
#endif
        _flag_is_analyzing = false;
    }

    // 自动保存
    if (us->save_power)
    {
        save();
    }

    // 文字高亮
    highlightAll();
    // 使用适配的 MyHighLighter 进行高亮，不在这里面了

    // 发出字数改变的信号
    emit signalWordCount(getWordCount(_text));

    // 自动提示搜索
    //surroundWordSearch();
    if (/*completer &&*/ completer->popup()->isVisible())
    {
        surroundWordSearch();
        completer_timer_timeout = true;
        //completer->popup()->hide();
    }
    if (changed_by_redo_undo == false && !_change_text)
    {
        cursorRealChanged(_pos);
    }

    edit_timestamp = getTimestamp();
}

void ChapterEditor::cursorChange()
{
    if (_flag_highlighting) return ;

    if (_flag_user_change_cursor) // 用户鼠标或者按键按下导致的
    {
        _flag_user_change_cursor = false;
        if (last_pressed_key == Qt::Key_Up/* && !textCursor().hasSelection()*/)
        {
            // 如果按住 shift，在下一步之前还是选中状态
            operatorSmartKeyUp(shifting);
        }
        else if (last_pressed_key == Qt::Key_Down/* && !textCursor().hasSelection()*/)
        {
            // 如果按住 shift，在下一步之前还是选中状态
            operatorSmartKeyDown(shifting);
        }
        cursorRealChanged();
    }
    // ==== 显示选中数量的字数 ====
    if (this->textCursor().hasSelection())
    {
        QString text = textCursor().selectedText();
        int count = getWordCount(text);
        emit signalWordCount(count);
    }

    // ==== 当前行高亮 ====
    if (editor_height > -1) // 初始化结束后再显示行背景（不然打开动画很难受）
    {
        highlightCurrentLine();
    }


    /* // ==== 保存光标位置便于撤销 ====
    // 避免撤销重做引起的重新修改光标事件
    if (changedByRedoUndo == true) {
        return ;
    }
    updateRecorderPosition(textCursor.position();); // 光标改变 比 文本改变 还要早*/

    // ==== 自动提示 ====
    //if (!(completer_timer_timeout == false && completer_timer->interval() != us->completer_input_delay)) // 不是中文输入的延迟
    if (last_operator != OPERATOR_CN && last_operator != OPERATOR_NOT_COMPLETER)
    {
        if (last_operator == OPERATOR_IMD_COMPLETER) // 必须是可视的
            surroundWordSearch();
        else
            delayCompleter(us->completer_cursor_delay);
    }
}

void ChapterEditor::highlightCurrentLine()
{
    if (toPlainText().isEmpty() ||  editing.isCompact() || !us->editor_line_bg)
        return ;

    QList<QTextEdit::ExtraSelection> es;
    QTextEdit::ExtraSelection s;
    s.format.setBackground(us->editor_line_bg_color);
    s.format.setProperty(QTextFormat::FullWidthSelection, true);
    s.cursor = this->textCursor();
    s.cursor.clearSelection();
    es.append(s);
    this->setExtraSelections(es);
}

void ChapterEditor::highlightAll()
{
    // 先判断有没有需要开启的
    if (!us->editor_line_spacing)
        return ;

    // 判断类型
    if (editing.isCompact())
        return ;

    // 备份当前的位置
    int current_pos = textCursor().position();
    int current_slider = verticalScrollBar()->sliderPosition();
    _flag_highlighting = true;

    // 修改行间距（5000字以上快速操作卡顿，20000字以上略微卡顿）
    if (us->editor_line_spacing)
    {
        selectAll();
        QTextCursor tc = textCursor();
        QTextBlockFormat tbf = tc.blockFormat();
        tbf.setLineHeight(us->editor_line_spacing, QTextBlockFormat::LineDistanceHeight);
        tc.mergeBlockFormat(tbf);
        tc.setPosition(current_pos); // 加上这句避免末尾打字时会全选文字
        setTextCursor(tc);
    }

    // 文字下划线
    /*{
        QTextCharFormat tcf;
        tcf.setFontUnderline(QTextCharFormat::SingleUnderline);
        selectAll(); // copyAvaliable 槽中已经针对 _flag_highlight 参数进行屏蔽了
        mergeCurrentCharFormat(tcf);
    }*/

    // 高亮人物语言


    // 高亮章节备注


    // 高亮自定义正则表达式


    // 高亮名片


    // 高亮词语纠正


    // 高亮关键词搜索


    // 高亮敏感词


    // 还原正确的位置
    if (textCursor().position() != current_pos)
    {
        QTextCursor tc = this->textCursor();
        tc.setPosition(current_pos);
        setTextCursor(tc);
    }

    document_height = static_cast<int>(document()->size().height()); // 重新设置 document 的高度，必须这样才不会跳
    resetScrollBarRange(); // 重新调整滚动条范围
    if (verticalScrollBar()->sliderPosition() != current_slider)
        verticalScrollBar()->setSliderPosition(current_slider);
    _flag_highlighting = false;
    //scrollChange();
}

bool ChapterEditor::showCardlibToolTip(QPoint point)
{
    if (gd->clm.currentLib() == nullptr)
        return false;
    int pos = getPositionByMouseHover(point);
    if (pos < 0)
        return false;

    int start, end;
    QString sent = getSent3(toPlainText(), pos, start, end);
    if (start == end)
        return false;

    CardBean *card = getCardByTextCursor(pos);
    if (card == nullptr)
    {
        QToolTip::hideText();
        return false;
    }

    QString tooltip = card->brief;
    tooltip = CardlibHighlight::getHighlightedText(tooltip);
    QToolTip::showText(mapToGlobal(point), tooltip, this);
    return true;
}

/**
 * 获取鼠标悬浮下的光标位置
 * 方法：二分法
 * 注意：可能设置了行间距，y 不在任一行间距中，此时返回 -1
 * @param point 鼠标的坐标
 * @return      字符文本的位置（文本框外为-1）
 */
int ChapterEditor::getPositionByMouseHover(QPoint point)
{
    if (point.x() < 0 || point.x() > width() || point.y() < 0 || point.y() > height())
        return -1;

    QTextCursor text_cursor = textCursor();
    int len = toPlainText().length();
    int line_pos = -1, line_top = -1;
    int topest_pos = 0, botest_pos = toPlainText().length();
    text_cursor.setPosition(topest_pos);
    QRect topest_rect = cursorRect(text_cursor);
    text_cursor.setPosition(botest_pos);
    QRect botest_rect = cursorRect(text_cursor);

    // 二分找到当前行
    while (topest_pos < botest_pos)
    {
        int mid = (topest_pos + botest_pos) / 2;
        text_cursor.setPosition(mid);
        QRect mid_rect = cursorRect(text_cursor);

        if (topest_rect.top() > point.y() || botest_rect.bottom() < point.y())
            return -1;
        else if (mid_rect.top() > point.y()) // 上移
        {
            botest_pos = mid - 1;
            text_cursor.setPosition(botest_pos);
            botest_rect = cursorRect(text_cursor);
        }
        else if (mid_rect.bottom() < point.y()) // 下移
        {
            topest_pos = mid + 1;
            text_cursor.setPosition(topest_pos);
            topest_rect = cursorRect(text_cursor);
        }
        else if (mid_rect.top() <= point.y() && mid_rect.bottom() >= point.y()) // 找到这一行了
        {
            line_pos = mid;
            line_top = mid_rect.top();
            break;
        }
    }
    if (line_pos == -1)
        return -1;

    // 鼠标移动获取列位置（和光标最近的）
    int nearest_pos = line_pos, nearest_dis = width();
    while (1)
    {
        text_cursor.setPosition(line_pos);
        QRect rect = cursorRect(text_cursor);
        if (rect.top() != line_top) // 已经不在同一行了
            break;

        int dis = qMin( abs(rect.left() - point.x()), abs(rect.right() - point.x()) );
        if (dis < nearest_dis) // 最小的
        {
            nearest_dis = dis;
            nearest_pos = line_pos;
        }
        else if (dis == nearest_dis) // 又回来了
        {
            break;
        }

        // 左右移动
        if (line_pos >= len || line_pos <= 0) // 超过了位置
        {
            return -1;
        }
        else if (rect.right() < point.x()) // 右移
        {
            line_pos++;
        }
        else if (rect.left() > point.x()) // 左移
        {
            line_pos--;
        }
        else // 在光标矩形内，太妙了
        {
            break;
        }
    }
    QFontMetrics fm(this->fontMetrics());
    if (nearest_dis >= fm.horizontalAdvance("啊"))
        return -1;

    return line_pos;
}

bool ChapterEditor::showCardlibEditor()
{
    if (gd->clm.currentLib() == nullptr)
        return false;

    CardBean *suitable_card = getCardByTextCursor();

    // 显示名片
    if (suitable_card != nullptr)
    {
        emit signalShowCardEditor(suitable_card);
        return true;
    }
    else
        return false;
}

/**
 * 获取光标附近的名片
 */
CardBean *ChapterEditor::getCardByTextCursor(int pos)
{
    if (gd->clm.currentLib() == nullptr)
        return nullptr;
    if (pos == -1)
        pos = textCursor().position();
    int start, end;
    QString text = toPlainText();
    QString sent = getSent3(text, pos, start, end);
    if (start >= end || pos < start || pos > end)
    {
        return nullptr;
    }
    pos -= start;
    end -= start;
    start = 0;

    // 遍历名片，选择最合适的那一张
    int find_pos = -1;
    int shortest = -1;                 // 使用离光标最近、最短的那个名片
    CardBean *suitable_card = nullptr; // 要显示的名片
    foreach (CardBean *card, gd->clm.currentLib()->using_cards)
    {
        QRegExp re(card->pattern());
        if ((find_pos = re.lastIndexIn(sent, pos)) != -1)
        {
            QString ma = re.cap();
            if (find_pos + ma.length() < pos)
                continue;
            if (shortest == -1 || shortest > ma.length())
            {
                shortest = ma.length();
                suitable_card = card;
            }
        }
    }
    return suitable_card;
}

void ChapterEditor::copyAva()
{
    if (_flag_highlighting) return ;

    // ==== 显示选中数量的字数 ====
    if (!textCursor().hasSelection())
    {
        emit signalWordCount(getWordCount(_text));
    }
    else
    {
        // 只在开始选中的时候保存
        // 取消选中的话，可能是输入文字，和这个有冲突
        // 一般取消文字也是需要用户手动点的
        // 所以就放到了用户修改光标的块里面
        updateRecorderSelection();
    }
}

void ChapterEditor::cursorChangedByUser()
{
    // ==== 更新是否是光标最后 ====
    is_editing_end = isAtEditEnd();
    pre_scroll_position = verticalScrollBar()->sliderPosition();
    pre_scroll_bottom_delta = verticalScrollBar()->maximum() - pre_scroll_position;
    pre_scroll_viewport_top = cursorRect().top();
    updateRecorderSelection();
    if (!_edit->textCursor().hasSelection())
        selection_list.clear();
}

void ChapterEditor::scrollChange()
{
    if (_flag_highlighting) return ; // 通过修改格式引起的变化

    updateRecorderScroll();

    if (_flag_is_not_scrolled)
    {
        _flag_is_not_scrolled = false;
        return ;
    }

    /**
     * 滚轮一次滚动触发一次
     * 编辑内容触发多次
     * 例如回车：内容改变、修改范围、调整位置
     */

    if (is_editing_end)
    {
        pre_scroll_bottom_delta = verticalScrollBar()->maximum() - verticalScrollBar()->sliderPosition();
    }

    pre_scroll_viewport_top = cursorRect().top();

    /*if (!editing.isCompact() && us->cursor_animation)
    {
        editor_cursor->positionChanged(-1);
    }*/
    QTimer::singleShot( 0, [ = ]
    {
        blurTitleBarFaster();
    } );
}

void ChapterEditor::resetScrollBarRange() // scrollbar range changed event
{
    if (editing.isCompact())
        return ;
    if (_flag_is_not_range_changed) // 一个变量避免死循环（貌似可以设置成静态？）
    {
        _flag_is_not_range_changed = false;
        return ;
    }

    if (document_height + viewport_margin_bottom - (blur_effect ? us->widget_size : 0) <= editor_height) // 不足一页的，不显示滚动条
    {
        if (blur_effect)
        {
            verticalScrollBar()->setMinimum(-us->widget_size);
            // verticalScrollBar()->setSliderPosition(-us->widget_size);
        }
        return ;
    }

    if (us->scroll_bottom_blank) // 设置底部空白距离
    {
        _flag_is_not_range_changed = true; // 避免下面这句引发问题
        //        int minimum = verticalScrollBar()->minimum();//; // 状态栏模糊不一样
        int minimum = -(blur_effect ? us->widget_size : verticalScrollBar()->minimum());
        verticalScrollBar()->setRange(minimum, -minimum + document_height - verticalScrollBar()->pageStep() + viewport_margin_bottom);
        _flag_is_not_range_changed = false;
    }

    // 玄学！！！！！！！！使用下面的语句，会导致窗口拉伸时滚动到回到 0~2的位置，注释掉后就不会有问题了……
    // 而且全文光标行固定和末尾光标行固定照样可以生效，不会出问题……
    /*if (verticalScrollBar()->maximum() != pre_scrollbar_max_range) // 是文本改变或者宽高改变引起的范围改变（哇，我真是个天才！！！（琢磨了一上午+一中午才想出来的！！！））
    {
        pre_scrollbar_max_range = verticalScrollBar()->maximum();  // 同上，通过高度的改变来判断是不是文本的变化
        if (us->scroll_bottom_fixed) // 固定尾部光标行位置
        {
            if (is_editing_end)
            {
                if (pre_scroll_bottom_delta_by_analyze == -1) // openChapter 里面设置的
                    pre_scroll_bottom_delta = verticalScrollBar()->maximum();
                else
                    pre_scroll_bottom_delta = pre_scroll_bottom_delta_by_analyze; // 恢复到 文本改变 之前
                scrollToEditEnd();

                return ;
            }
        }

        if (us->scroll_cursor_fixed) // 固定全文光标行位置
        {
            if (pre_scroll_viewpoer_top_by_analyze == -1) // openChanter 刚打开章节时是 -1
                pre_scroll_viewport_top = cursorRect().top();
            else
                pre_scroll_viewport_top = pre_scroll_viewpoer_top_by_analyze;
            scrollCursorFix();
        }
    }*/

}

void ChapterEditor::beforeInputted(QString in)
{
    QTextCursor tc = textCursor();
    if (tc.hasSelection())
    {
        QString str = tc.selectedText();
        gd->boards.tryAddRecycle(str);
    }

    if (us->climax_threshold)
    {
        if (in == "！")
            us->addClimaxValue(true);
        else if (in == "。" || in == "？")
            us->addClimaxValue(false);
    }

    if (us->emotion_filter_enabled && !in.isEmpty())
        us->emotion_filter->input(in);
    if (us->envir_picture_enabled && !in.isEmpty())
        us->envir_picture->input(in);
}

/**
 * 光标真正变化（而不是setText后光标变成0这种附加的）时调用
 * 用来配合光标移动动画
 * @param pos 光标新位置
 */
void ChapterEditor::cursorRealChanged(int pos)
{
    if (!editing.isCompact() && us->cursor_animation)
        editor_cursor->positionChanged(pos);
}

/**
 * 底部光标所在行位置固定（ assert(isAtEditEnd()) ）
 */
void ChapterEditor::scrollToEditEnd()
{
    /*if (!isAtEditEnd())
    {
        is_editing_end = false;
        return ;
    }*/
    int pos = document_height + viewport_margin_bottom - editor_height - pre_scroll_bottom_delta
              - verticalScrollBar()->minimum(); //+(blur_effect ? us->widget_size : 0); // 这两个暂时等价

    //if (pos < 0) return ;
    verticalScrollBar()->setSliderPosition(pos);
}

/**
 * 和之前位置相比，固定光标所在行在窗口视图中的位置
 * 改变字体大小、窗口大小等时候会用到
 */
void ChapterEditor::scrollCursorFix()
{
    int scroll_slide = verticalScrollBar()->sliderPosition();
    int viewport_slide = cursorRect().top();
    int delta = pre_scroll_viewport_top - viewport_slide;
    scroll_slide -= delta;
    verticalScrollBar()->setSliderPosition(scroll_slide);
}


int quick_sqrt(long X)
{
    unsigned long M = static_cast<unsigned long>(X);
    unsigned int N, i;
    unsigned long tmp, ttp; // 结果、循环计数
    if (M == 0) // 被开方数，开方结果也为0
        return 0;
    N = 0;
    tmp = (M >> 30); // 获取最高位：B[m-1]
    M <<= 2;
    if (tmp > 1) // 最高位为1
    {
        N ++; // 结果当前位为1，否则为默认的0
        tmp -= N;
    }
    for (i = 15; i > 0; i--) // 求剩余的15位
    {
        N <<= 1; // 左移一位
        tmp <<= 2;
        tmp += (M >> 30); // 假设
        ttp = N;
        ttp = (ttp << 1) + 1;
        M <<= 2;
        if (tmp >= ttp) // 假设成立
        {
            tmp -= ttp;
            N ++;
        }
    }
    return static_cast<int>(N);
}

/**
 * 平滑滚动
 * @param position 光标位置
 * @param sel      选中位置
 * @param slider   界面滚动位置，-1为只使用光标位置
 */
void ChapterEditor::smoothScrollTo(int pos, int sel, int slider)
{
    // 保存当前的滚动位置
    int start_slider = verticalScrollBar()->sliderPosition();
    QPoint start_point = cursorRect().topLeft();

    // 滚动到目标位置
    QTextCursor tc = textCursor();
    tc.setPosition(pos);
    if (sel > 0)
        tc.setPosition(pos + sel, QTextCursor::KeepAnchor);
    setTextCursor(tc);
    int end_slider = slider >= 0 ? slider : verticalScrollBar()->sliderPosition();

    // 如果没有设置强制的slider，并且为-2，则滚动到中间三分之一处
    if (slider == -2)
    {
        if (abs(end_slider - start_slider) < 5) // 进度条几乎没有移动
        {
            if (cursorRect().top() < us->widget_size + (us->editor_fuzzy_titlebar ? us->widget_size : 0))
                end_slider -= (us->widget_size + (us->editor_fuzzy_titlebar ? us->widget_size : 0) - cursorRect().top());
            else if (cursorRect().top() > height() - us->widget_size)
                end_slider += (cursorRect().top() - height() + us->widget_size);
        }
        else
        {
            if (cursorRect().top() < height() / 3)
                end_slider -= (height() / 3 - cursorRect().top());
            else if (cursorRect().top() > height() * 2 / 3)
                end_slider += (cursorRect().top() - height() * 2 / 3);
        }

        if (!editing.isCompact() && us->cursor_animation)
            verticalScrollBar()->setSliderPosition(end_slider); // 光标移动动画要用到
    }

    //    QPoint end_point = cursorRect().topLeft();

    if (!editing.isCompact() && us->cursor_animation)
    {
        editor_cursor->move(start_point);
        editor_cursor->positionChanged(-1);
    }

    // 暂时滚回去，光标就留到最新的地方
    verticalScrollBar()->setSliderPosition(start_slider);

    // 判断滚动时间
    int duration = qMin(400, quick_sqrt(abs(end_slider - start_slider)) << 3);

    // 开始动画
    smooth_ani->setStartValue(start_slider);
    smooth_ani->setEndValue(end_slider);
    smooth_ani->setDuration(duration);
    smooth_ani->start();

    // 修改光标动画
    /*if (!editing.isCompact() && us->cursor_animation)
    {
        editor_cursor->move(start_pos);
        editor_cursor->positionChanged(-1);

        // us->cursor_animation = false; // 临时屏蔽其他的光标动画

        // editor_cursor->show();
        // QPropertyAnimation* cur_ani = new QPropertyAnimation(editor_cursor, "pos");
        // cur_ani->setDuration(duration);
        // cur_ani->setStartValue(start_point);
        // cur_ani->setEndValue(end_point);
        // cur_ani->start();
        // connect(cur_ani, &QPropertyAnimation::finished, [=]{
        //     editor_cursor->hide();
        //     us->cursor_animation = true;
        //     cur_ani->deleteLater();
        // });
    }*/
}

void ChapterEditor::smoothScrollTo(EditorScrollBean ecb)
{
    smoothScrollTo(ecb.pos, ecb.sel, ecb.slide);
}

void ChapterEditor::paintEvent(QPaintEvent *event)
{
    if (us->editor_underline)
    {
        drawUnderLine();
    }

    if (us->editor_water_effect && waters.size())
    {
        QPainter painter(this->viewport());
        paintWaterRipple(painter);
    }

    if (!blur_effect)
        return QTextEdit::paintEvent(event); // 画文字

    if (update_timer->isActive() && waters.size() == 0)
        update_timer->stop();

    // 标题栏模糊效果
    QTextEdit::paintEvent(event); // 画文字
}

void ChapterEditor::drawUnderLine()
{
    if (editing.isCompact())
        return ;

    QPainter painter(this->viewport());

    int slide = verticalScrollBar()->sliderPosition(); // 滚动位置
    //int page = verticalScrollBar()->pageStep();

    QFontMetricsF fm = fontMetrics();
    int line_spacing = fm.lineSpacing() + us->editor_line_spacing; // 行间距
    int offset_x = 2, offset_y = us->editor_underline_offset - us->editor_line_spacing; // 针对视图的偏移
    // << fm.lineSpacing() << fm.height() << fm.leading() << fm.capHeight();
    if (us->editor_underline_middle)
        offset_y += us->editor_line_spacing / 2;

    int width = geometry().width() - offset_x - offset_x;
    int start_y = line_spacing - slide % line_spacing + offset_y; // 开始的纵坐标
    //    double start_y = line_spacing - (slide - (int)(slide / line_spacing) * line_spacing) + offset_y; // 开始的纵坐标
    int end_y = start_y + geometry().height() + line_spacing * 2 /*多两行*/; // 末尾的纵坐标

    painter.setPen(us->editor_underline_color);

    if (!underline_progress)
    {
        while (start_y <= end_y)
        {
            painter.drawLine(offset_x, start_y, offset_x + width, start_y);
            start_y += line_spacing;
        }
    }
    else
    {
        int line_count = (end_y - start_y) / line_spacing;
        int count = 0;
        int x, w, p;

        while (start_y <= end_y)
        {
            ++count;
            int min = 50 * count / line_count; // 越早的越在前面
            //            int max = min + 50;
            if (underline_progress < min)
                p = 0;
            else
                p = 100 * (underline_progress - min) / 50;

            x = offset_x + width / 2 * (100 - p) / 100;
            w = width * p / 100;

            painter.drawLine(x, start_y, x + w, start_y);
            start_y += line_spacing;
        }
    }

}

void ChapterEditor::setScrollSlider(int x)
{
    _smooth_scroll_slider = x;
    verticalScrollBar()->setSliderPosition(x);
}

int ChapterEditor::getScrollSlider()
{
    return _smooth_scroll_slider;
}

void ChapterEditor::resetScrollSlider()
{
    if (smooth_ani->state() & QPropertyAnimation::Running)
        smooth_ani->stop();
}

void ChapterEditor::addSmoothScrollThread(int distance, int duration)
{
    SmoothScrollBean *bean = new SmoothScrollBean(distance, duration);
    smooth_scrolls.append(bean);
    connect(bean, SIGNAL(signalSmoothScrollDistance(SmoothScrollBean *, int)), this, SLOT(slotSmoothScrollDistance(SmoothScrollBean *, int)));
    connect(bean, &SmoothScrollBean::signalSmoothScrollFinished, [ = ]
    {
        smooth_scrolls.removeOne(bean);
    });

    // 删掉原来反方向的
    bool positive = (distance > 0);
    for (int i = 0; i < smooth_scrolls.size(); i++)
    {
        if (smooth_scrolls.at(i)->isPositive() != positive)
            smooth_scrolls.removeAt(i--);
    }
}

void ChapterEditor::setUnderlineProgress(int x)
{
    underline_progress = x;
    update();
}

int ChapterEditor::getUnderlineProgress()
{
    return underline_progress;
}

void ChapterEditor::resetUnderlineProgress()
{
    underline_progress = 1;
}

void ChapterEditor::paintWaterRipple(QPainter &painter)
{
    const int press_speed = 50;     // 速度： 50/100 像素 / 每秒
    const int release_speed = 100;
    const int water_release_duration = 500;

    qint64 timestamp = getTimestamp();
    for (int i = 0; i < waters.size(); i++)
    {
        Water water = waters.at(i);

        int radius = 0;
        QColor color = us->editor_water_color;

        if (water.release_timestamp) // 已经松开了
        {
            qint64 press_length = water.release_timestamp - water.press_timestamp;
            qint64 release_length = timestamp - water.release_timestamp;
            radius = press_length * press_speed / 1000 + release_length * release_speed / 1000;
            int prog = 100 * release_length / water_release_duration;
            if (prog > 100) prog = 100;
            color.setAlpha(color.alpha() * (100 - prog) / 100);
            if (prog == 100)
            {
                waters.removeAt(i--);
            }
        }
        else // 一直按住
        {
            qint64 press_length = timestamp - water.press_timestamp;
            radius = press_length * press_speed / 1000;
        }

        QPainterPath path;
        path.addEllipse(QRect(water.point.x() - radius, water.point.y() - radius, radius << 1, radius << 1));
        painter.fillPath(path, color);
    }
}

void ChapterEditor::slotSmoothScrollDistance(SmoothScrollBean *bean, int dis)
{
    int slide = verticalScrollBar()->sliderPosition();
    slide += dis;
    if (slide < verticalScrollBar()->minimum())
    {
        slide = verticalScrollBar()->minimum();
        smooth_scrolls.removeOne(bean);
    }
    else if (slide > verticalScrollBar()->maximum())
    {
        slide = verticalScrollBar()->maximum();
        smooth_scrolls.removeOne(bean);
    }
    verticalScrollBar()->setSliderPosition(slide);
    cursorRealChanged(); // 光标位置改变
}

void ChapterEditor::initUndoRedo()
{
    _pos = textCursor().position();
    recorders_pos = 0;
    recorders.clear();
    recorderOperator(); // 先保存一次记录
}

void ChapterEditor::recorderOperator()
{
    /*recorders_pos : 当前项目的下标，上次的坐标*/
    // 删除后面的
    while (recorders.size() > recorders_pos)
    {
        recorders.removeLast();
    }
    // 要是超过了上限，则删除多余的
    if (recorders.size() > us->editor_recorder_max)
    {
        recorders.removeFirst();
        recorders_pos--;
    }

    QString text = this->toPlainText();
    int pos = this->textCursor().position();
    if (pos == 0) pos = _pos;
    int scroll = this->verticalScrollBar()->sliderPosition();

    EditorRecorderItem item(text, pos, scroll);

    // 和上一个变化作比较，如果是同一个就合并
    // 可能会引发崩溃问题
    if (recorders_pos >= 1) // 前面有2条记录
    {
        // if (text == _text) return ; // 文字相同，当做没有改变 // 加上这句话会出bug……
        EditorRecorderItem last_item = recorders.at(recorders_pos - 1);
        if (text == last_item.getText()) return ; // 文本相同，当做没有改变
        if (1/*last_item.getTime()+1000 >= item.getTime()*/)
        {
            //int last_pos = last_item.getPos();
            QString last_text = last_item.getText();
            int start, end;
            last_item.getSelection(start, end);
            if (start == end) // 没有选中内容
            {
                if (/*pos == last_pos+1 && */text.length() == last_text.length() + 1) // 连续输入一个字
                {
                    int back = text.length() - pos;
                    if (text.left(pos - 1) == last_text.left(pos - 1) && text.right(back - 1) == last_text.right(back - 1))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_INPUTONE && last_item.getTime() + 1000 >= item.getTime())
                        {
                            recorders.removeLast();
                            // 有第一个，则说明上一个可能也是这个
                            // 继续删除上一个
                            if ( recorders_pos >= 2
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getCase() == UNDOREDO_CASE_INPUTONE
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getTime() + 1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_INPUTONE);
                    }
                }
                else if (/*pos == last_pos-1 && */text.length() == last_text.length() - 1) // 连续 backspace 一个字
                {
                    int back = text.length() - pos;
                    if (text.left(pos) == last_text.left(pos) && text.right(back) == last_text.right(back))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_BACKSPACEONE && last_item.getTime() + 1000 >= item.getTime())
                        {
                            recorders.removeLast();
                            if (recorders_pos >= 2
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getCase() == UNDOREDO_CASE_BACKSPACEONE
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getTime() + 1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_BACKSPACEONE);
                    }
                }
                else if (/*pos == last_pos && */text.length() == last_text.length() - 1) // 连续 delete 一个字
                {
                    int back = text.length() - pos;
                    if (text.left(pos) == last_text.left(pos) && text.right(back) == last_text.right(back))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_DELETEONE && last_item.getTime() + 1000 >= item.getTime())
                        {
                            recorders.removeLast();
                            if (recorders_pos >= 2
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getCase() == UNDOREDO_CASE_DELETEONE
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos - 2))).getTime() + 1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_DELETEONE);
                    }
                }
            }
        }
    }

    recorders.append(item);
    recorders_pos = recorders.size();
}

void ChapterEditor::undoOperator()
{
    if (recorders_pos <= 1 || recorders.size() <= 1)
        return ;

    readRecorder(--recorders_pos - 1);
}

void ChapterEditor::redoOperator()
{
    if (recorders_pos >= recorders.size())
        return ;
    recorders_pos++;

    readRecorder(recorders_pos - 1);
    cursorRealChanged(); // 显示光标改变动画
}

void ChapterEditor::readRecorder(int x) // x 从零开始计算（原本是从1开始）
{
    isNotInput();
    changed_by_redo_undo = true;
    EditorRecorderItem item = recorders.at(x);
    this->setPlainText(item.getText());

    QTextCursor cursor = this->textCursor();
    int position = item.getPos();
    cursor.setPosition(position);
    this->setTextCursor(cursor);
    _pos = position;

    int start, end;
    item.getSelection(start, end);

    if (start > -1 && end > -1)
    {
        if (start == _pos)
            start = end;
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(_pos, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }

    // 如果设置了行间距，则必须要调用彩色化方法，并在里面自动调整滚动进度，不然显示不对、以及超过原来文档高度的位置都不能显示到
    highlightAll();

    QScrollBar *scrollbar = this->verticalScrollBar();
    scrollbar->setSliderPosition(item.getScroll());
    changed_by_redo_undo = false;
    //this->ensureCursorVisible();
    cursorRealChanged(); // 显示光标改变动画
}

void ChapterEditor::updateRecorderPosition(int x)
{
    if (recorders_pos - 1 < 0) return ;
    // if (recorders_pos == recorders.size()) // 是最前面的位置，避免撤销后重做再撤销这种怪操作
    {
        recorders[recorders_pos - 1].setPos(x);
        recorders[recorders_pos - 1].setScroll(this->verticalScrollBar()->sliderPosition());
    }
    /*EditorRecorderItem item = &recorders.at(recorders.size()-1);
    if (x != item.getPos()) {
        item.setPos(x);

        QScrollBar* scrollbar = this->verticalScrollBar();
        int s = scrollbar->sliderPosition();
        if (s != item.getScroll())
            item.setScroll(s);
    }*/
}

void ChapterEditor::updateRecorderScroll()
{
    if (_flag_is_not_scrolled)
    {
        _flag_is_not_scrolled = false;
        return ;
    }

    recorders[recorders_pos - 1].setScroll(this->verticalScrollBar()->sliderPosition());
}

void ChapterEditor::updateRecorderSelection()
{
    if (textCursor().hasSelection())
    {
        recorders[recorders_pos - 1].setSelection(textCursor().selectionStart(), textCursor().selectionEnd());
    }
    else
    {
        recorders[recorders_pos - 1].setSelection(-1, -1);
    }
}

void ChapterEditor::delayCompleter(int ms)
{
    if (!completer_timer_timeout)
    {
        completer_timer->stop();
    }
    if (us->auto_suggestion)
    {
        completer_timer_timeout = false;
        completer_timer->start(ms);
    }
}

void ChapterEditor::saveFontSize()
{
    us->editor_font_size = this->font().pointSize();
    us->setVal("editor_font_size", us->editor_font_size);
    this->updateUI();
    emit signalUpdateAllEditor();
}

void ChapterEditor::msgLostParam(QString cmd, QString s)
{
    QString tip = "命令参数不完整：\n" + s;
    QMessageBox::information(this, cmd, tip);
}

void ChapterEditor::actionScrollContentByLines(int line)
{
    int slider_position = verticalScrollBar()->sliderPosition() + verticalScrollBar()->singleStep() * line;
    if (slider_position < verticalScrollBar()->minimum())
        slider_position = verticalScrollBar()->minimum();
    else if (slider_position > verticalScrollBar()->maximum())
        slider_position = verticalScrollBar()->maximum();
    verticalScrollBar()->setSliderPosition(slider_position);
}

void ChapterEditor::actionScrollContentByHeight(int part)
{
    addSmoothScrollThread(editor_height / part, 100);
    if (!editing.isCompact() && us->cursor_animation)
        editor_cursor->setStartPosition(-1);
}

void ChapterEditor::actionMoveTextCursorToPrevPara(bool select)
{
    int select_start = -1, select_end = -1;
    select_start = select_end = textCursor().position();
    if (select_start == -1)
        select_start = select_end = _pos;
    if (textCursor().hasSelection())
    {
        select_start = textCursor().selectionStart();
        select_end = textCursor().selectionEnd();
    }
    if (select && select_end == -1)
        select_end = textCursor().position();
    moveToParaPrevStart(this->textCursor().position());
    if (select)
    {
        QTextCursor tc = textCursor();
        if (_pos < select_start) // 选区左边移动
        {
            tc.setPosition(select_end, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        else // 选区右边移动
        {
            tc.setPosition(select_start, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(tc);
    }
}

void ChapterEditor::actionMoveTextCursorToNextPara(bool select)
{
    int select_start = -1, select_end = -1;
    select_start = select_end = textCursor().position();
    if (select_start == -1)
        select_start = select_end = _pos;
    if (textCursor().hasSelection())
    {
        select_start = textCursor().selectionStart();
        select_end = textCursor().selectionEnd();
        if (_pos == select_start) // 如果需要从右边选到左边
        {
            select_start = select_end;
        }
    }
    if (select && select_start == -1)
        select_start = textCursor().position();
    moveToParaNextStart(this->textCursor().position());
    if (select)
    {
        QTextCursor tc = textCursor();
        if (_pos < select_start) // 选区左边移动
        {
            tc.setPosition(select_end, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        else // 选区右边移动
        {
            tc.setPosition(select_start, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(tc);
    }
}

void ChapterEditor::actionMoveTextCursorToLeft(bool word, bool sent, bool select)
{
    _flag_user_change_cursor = true;
    int select_start = -1, select_end = -1;
    select_start = select_end = textCursor().position();
    if (select_start == -1)
        select_start = select_end = _pos;
    if (textCursor().hasSelection())
    {
        select_start = textCursor().selectionStart();
        select_end = textCursor().selectionEnd();
    }
    if (select && select_end == -1)
        select_end = textCursor().position();

    if (word && !sent) // 左移一个单词
        moveToPreWord(this->textCursor().position());
    else if (!word && sent) // 左移一个句子
        moveToSentStart(this->textCursor().position());
    else // 移动到段首
        moveToParaStart(this->textCursor().position());

    if (select)
    {
        QTextCursor tc = textCursor();
        if (_pos < select_start) // 选区左边移动
        {
            tc.setPosition(select_end, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        else // 选区右边移动
        {
            tc.setPosition(select_start, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(tc);
    }
}

void ChapterEditor::actionMoveTextCursorToRight(bool word, bool sent, bool select)
{
    _flag_user_change_cursor = true;
    int select_start = -1, select_end = -1;
    select_start = select_end = textCursor().position();
    if (select_start == -1)
        select_start = select_end = _pos;
    if (textCursor().hasSelection())
    {
        select_start = textCursor().selectionStart();
        select_end = textCursor().selectionEnd();
        if (_pos == select_start) // 从结尾开始
        {
            select_start = select_end;
        }
    }
    if (select && select_end == -1)
        select_end = textCursor().position();

    if (word && !sent) // 右移一个单词
        moveToNextWord(this->textCursor().position());
    else if (!word && sent) // 右移一个句子
        moveToSentEnd(this->textCursor().position());
    else // 移动到段尾
        moveToParaEnd(this->textCursor().position());

    if (select)
    {
        QTextCursor tc = textCursor();

        if (_pos < select_start) // 选区左边移动
        {
            tc.setPosition(select_end, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        else // 选区右边移动
        {
            tc.setPosition(select_start, QTextCursor::MoveAnchor);
            tc.setPosition(_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(tc);
    }
}

int ChapterEditor::actionSearchPrev(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.lastIndexOf(search_key, (pos > 0 && pos == text.length()) ? pos - 1 : pos);
    if (find > -1 && find >= pos - search_key.length()) // 光标正在当前的位置
        find = text.lastIndexOf(search_key, find - 1);
    if (find >= 0)
        smoothScrollTo(find, select ? search_key.length() : 0, -2);
    return find;
}

int ChapterEditor::actionSearchNext(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.indexOf(search_key, pos);
    if (find == pos) // 光标正在当前的位置
        find = text.indexOf(search_key, find + search_key.length());
    if (find >= 0)
        smoothScrollTo(find, select ? search_key.length() : 0, -2);
    return find;
}

int ChapterEditor::actionSearchFirst(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.indexOf(search_key);
    if (find == -1 || find >= pos)
        return -1;
    smoothScrollTo(find, select ? search_key.length() : 0, -2);
    return find;
}

int ChapterEditor::actionSearchLast(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.lastIndexOf(search_key);
    if (find <= pos)
        return -1;
    smoothScrollTo(find, select ? search_key.length() : 0, -2);
    return find;
}

int ChapterEditor::actionReplacePrev(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.lastIndexOf(search_key, (pos > 0 && pos == text.length()) ? pos - 1 : pos);
    if (find > -1 && find >= pos - search_key.length()) // 光标正在当前的位置
        find = text.lastIndexOf(search_key, find - 1);
    if (find >= 0)
    {
        smoothScrollTo(find, select ? search_key.length() : 0, -2);
        int start = find, end = find + replace_key.length();
        operatorWordReplace(find, find + search_key.length(), replace_key);
        setSelection(start, end);
    }
    return find + replace_key.length();
}

int ChapterEditor::actionReplaceNext(bool select)
{
    if (search_key.isEmpty()) return -1;
    QString text = toPlainText();
    int pos = textCursor().position();
    int find = text.indexOf(search_key, pos);
    if (find == pos) // 光标正在当前的位置
        find = text.indexOf(search_key, find + search_key.length());
    if (find >= 0)
    {
        smoothScrollTo(find, select ? search_key.length() : 0, -2);
        int start = find, end = find + replace_key.length();
        operatorWordReplace(find, find + search_key.length(), replace_key);
        setSelection(start, end);
    }
    return find;
}

int ChapterEditor::actionReplaceAll(bool select)
{
    if (search_key.isEmpty()) return -1;

    prepareAnalyze();
    int find = 0;
    int search_length = search_key.length(), replace_length = replace_key.length();
    while ((find = _text.indexOf(search_key, find)) != -1)
    {
        deleteText(find, find + search_length);
        insertText(find, replace_key);
        find += replace_length;
    }
    finishAnalyze();
}

void ChapterEditor::initMenu()
{
    // 创建menu
    pop_menu = new QMenu(this);

    // 创建action
    comprehansive_action = new QAction(thm->icon("menu/net"), "综合搜索 (&P)", this);
    cut_action = new QAction(thm->icon("menu/cut"), "剪切 (&X)", this);
    copy_action = new QAction(thm->icon("menu/copy"), "复制 (&C)", this);
    paste_action = new QAction(thm->icon("menu/paste"), "粘贴 (&V)", this);
    boards_action = new QAction(thm->icon("menu/boards"), "板板 (&B)", this);
    undo_action = new QAction(thm->icon("menu/undo"), "撤销 (&Z)", this);
    redo_action = new QAction(thm->icon("menu/redo"), "重做 (&Y)", this);
    search_action = new QAction(thm->icon("menu/search"), "查找 (&F)", this);
    replace_action = new QAction(thm->icon("menu/replace"), "替换 (&H)", this);
    select_all_action = new QAction(thm->icon("menu/select_all"), "全选 (&A)", this);
    word_count_action = new QAction(thm->icon("menu/word_count"), "字数统计 (&W)", this);
    typeset_action = new QAction(thm->icon("menu/typeset"), "排版 (&T)", this);

    /*boards_menu = new QMenu("板板");
    clip_board_action = new QAction(thm->icon(""剪贴板", this);
    recycle_board_action = new QAction(thm->icon(""回收板", this);
    collection_board_action = new QAction(thm->icon(""收藏板", this);*/

    chapter_menu = new QMenu(tr("章节"), this);
    chapter_menu->setIcon(thm->icon("menu/chapter"));
    locate_outline_action = new QAction(thm->icon("menu/test_locate"), "尝试定位到故事线", this);
    locate_chapter_action = new QAction(thm->icon("menu/locate"), "定位到目录", this);
    open_prev_chapter_action = new QAction(thm->icon("arrow_up"), "上一章", this);
    open_next_chapter_action = new QAction(thm->icon("arrow_down"), "下一章", this);
    add_next_chapter_faster_action = new QAction(thm->icon("add"), "后面添加章节", this);
    move_to_prev_chapter_end_action = new QAction(thm->icon("menu/move_up"), "移动到上一章末尾", this);
    move_to_next_chapter_start_action = new QAction(thm->icon("menu/move_down"), "移动到下一章开头", this);
    split_to_next_chapter_action = new QAction(thm->icon("menu/split_one"), "分割后文到新章", this);
    split_to_multi_chapters_action = new QAction(thm->icon("menu/split_multi"), "长文章自动分割（未开发）", this);

    add_card_action = new QAction(thm->icon("menu/cardlib_add"), "添加名片", this);
    see_card_action = new QAction(thm->icon("menu/cardlib_card"), "查看名片", this);
    cardlib_menu = new QMenu("名片操作", this);
    cardlib_menu->setIcon(thm->icon("menu/cardlib"));
    card_color_action = new QAction(thm->icon("color"), "修改颜色", this);
    card_all_appear_action = new QAction(thm->icon("menu/all_appear"), "所有出现", this);
    jump_appear_action = new QAction(thm->icon("menu/appear"), "跳转至登场", this);
    jump_disappear_action = new QAction(thm->icon("menu/disappear"), "跳转至退场", this);

    conversion_menu = new QMenu(tr("转换"), this);
    conversion_menu->setIcon(thm->icon("menu/conversion"));
    error_word_highlight_action = new QAction(thm->icon("menu/error_word_highlight"), "错词高亮", this);
    word_correct_action = new QAction(thm->icon("menu/word_correct"), "词语纠错", this);
    simple_beauty_action = new QAction(thm->icon("menu/beauty"), "简单美化（未开发）", this);
    similar_sentence_action = new QAction(thm->icon("menu/similar"), "相似内容", this);
    predict_sentence_action = new QAction(thm->icon("menu/predict"), "预测内容", this);
    simplified_to_traditional_action = new QAction(thm->icon("menu/chinese_traditional"), "简转繁", this);
    traditional_to_simplified_action = new QAction(thm->icon("menu/chinese_simplified"), "繁转简", this);

    sensitive_menu = new QMenu(tr("敏感词"), this);
    sensitive_menu->setIcon(thm->icon("menu/sensitive"));
    sensitive_action = new QAction(thm->icon("menu/warning_disabled"), "敏感词高亮 (&S)", this);
    sensitive_eliminate_action = new QAction(thm->icon("menu/eliminate"), "敏感词消除", this);
    sensitive_replace_action = new QAction(thm->icon("menu/replace"), "敏感词替换", this);
    sensitive_replace_pinyin_action = new QAction(thm->icon("menu/pinyin"), "替换为拼音", this);
    sensitive_replace_first_action = new QAction(thm->icon("menu/pinyina"), "替换为首字母", this);
    sensitive_edit_list_action = new QAction(thm->icon("menu/sensitive_edit_list"), "编辑敏感词列表", this);
    sensitive_edit_replace_action = new QAction(thm->icon("menu/sensitive_edit_replace"), "编辑敏感词替换列表", this);

    function_menu = new QMenu(tr("功能"), this);
    //    function_menu->setIcon(thm->icon("menu/magic"));

    // 添加主菜单
    pop_menu->addAction(comprehansive_action);
    pop_menu->addSeparator();
    pop_menu->addAction(cut_action);
    pop_menu->addAction(copy_action);
    pop_menu->addAction(paste_action);
    pop_menu->addAction(select_all_action);
    pop_menu->addAction(boards_action);
    {
        /*boards_action->setMenu(boards_menu);
        boards_menu->addAction(clip_board_action);
        boards_menu->addAction(recycle_board_action);
        boards_menu->addAction(collection_board_action);*/
    }
    pop_menu->addSeparator();
    pop_menu->addAction(undo_action);
    pop_menu->addAction(redo_action);
    pop_menu->addSeparator();
    pop_menu->addAction(search_action);
    pop_menu->addAction(replace_action);

    pop_menu->addMenu(chapter_menu);
    {
        chapter_menu->addAction(locate_outline_action);
        chapter_menu->addSeparator();
        chapter_menu->addAction(locate_chapter_action);
        chapter_menu->addAction(open_prev_chapter_action);
        chapter_menu->addAction(open_next_chapter_action);
        chapter_menu->addAction(add_next_chapter_faster_action);
        chapter_menu->addSeparator();
        chapter_menu->addAction(split_to_next_chapter_action);
        chapter_menu->addAction(move_to_prev_chapter_end_action);
        chapter_menu->addAction(move_to_next_chapter_start_action);
        chapter_menu->addSeparator();
        chapter_menu->addAction(split_to_multi_chapters_action);
    }

    pop_menu->addSeparator();
    {
        pop_menu->addAction(add_card_action);
        pop_menu->addAction(see_card_action);
        cardlib_menu->addAction(card_color_action);
        cardlib_menu->addSeparator();
        cardlib_menu->addAction(card_all_appear_action);
        cardlib_menu->addAction(jump_appear_action);
        cardlib_menu->addAction(jump_disappear_action);
    }
    pop_menu->addMenu(cardlib_menu);
    pop_menu->addSeparator();

    pop_menu->addMenu(sensitive_menu);
    {
        sensitive_menu->addAction(sensitive_action);
        sensitive_menu->addSeparator();
        sensitive_menu->addAction(sensitive_eliminate_action);
        sensitive_menu->addAction(sensitive_replace_action);
        sensitive_menu->addAction(sensitive_replace_pinyin_action);
        sensitive_menu->addAction(sensitive_replace_first_action);
        sensitive_menu->addSeparator();
        sensitive_menu->addAction(sensitive_edit_list_action);
        sensitive_menu->addAction(sensitive_edit_replace_action);
    }

    pop_menu->addMenu(conversion_menu);
    {
        conversion_menu->addAction(error_word_highlight_action);
        conversion_menu->addAction(word_correct_action);
        conversion_menu->addSeparator();
        conversion_menu->addAction(simple_beauty_action);
        conversion_menu->addSeparator();
        conversion_menu->addAction(similar_sentence_action);
        conversion_menu->addAction(predict_sentence_action);
        conversion_menu->addSeparator();
        conversion_menu->addAction(simplified_to_traditional_action);
        conversion_menu->addAction(traditional_to_simplified_action);
    }
    pop_menu->addSeparator();
    //    pop_menu->addMenu(function_menu);
    {

    }
    pop_menu->addAction(word_count_action);
    pop_menu->addAction(typeset_action);

    // 设置menu和action属性
    sensitive_action->setCheckable(true);
    sensitive_action->setChecked(false);
    error_word_highlight_action->setCheckable(true);
    error_word_highlight_action->setChecked(false);
    if (!isFileExist(rt->DBASE_PATH + "books"))
    {
        predict_sentence_action->setEnabled(false);
        similar_sentence_action->setEnabled(false);
        predict_sentence_action->setText(predict_sentence_action->text() + "(不给用)");
        similar_sentence_action->setText(similar_sentence_action->text() + "(不给用)");
    }

    // 链接信号槽
    connect(comprehansive_action, SIGNAL(triggered()), this, SLOT(actionComprehensive()));
    connect(undo_action, SIGNAL(triggered()), this, SLOT(actionUndo()));
    connect(redo_action, SIGNAL(triggered()), this, SLOT(actionRedo()));
    connect(cut_action, SIGNAL(triggered()), this, SLOT(actionCut()));
    connect(copy_action, SIGNAL(triggered()), this, SLOT(actionCopy()));
    connect(paste_action, SIGNAL(triggered()), this, SLOT(actionPaste()));
    connect(boards_action, SIGNAL(triggered()), this, SLOT(actionBoards()));
    connect(search_action, SIGNAL(triggered()), this, SLOT(actionSearch()));
    connect(replace_action, SIGNAL(triggered()), this, SLOT(actionReplace()));
    connect(select_all_action, SIGNAL(triggered()), this, SLOT(actionSelectAll()));
    connect(typeset_action, SIGNAL(triggered()), this, SLOT(actionTypeset()));
    connect(locate_outline_action, SIGNAL(triggered()), this, SLOT(actionLocateOutline()));
    connect(locate_chapter_action, SIGNAL(triggered()), this, SLOT(actionLocateChapter()));
    connect(open_prev_chapter_action, SIGNAL(triggered()), this, SLOT(actionOpenPrevChapter()));
    connect(open_next_chapter_action, SIGNAL(triggered()), this, SLOT(actionOpenNextChapter()));
    connect(add_next_chapter_faster_action, SIGNAL(triggered()), this, SLOT(actionAddNextChapterFaster()));
    connect(move_to_prev_chapter_end_action, SIGNAL(triggered()), this, SLOT(actionMoveToPrevChapterEnd()));
    connect(move_to_next_chapter_start_action, SIGNAL(triggered()), this, SLOT(actionMoveToNextChapterStart()));
    connect(split_to_next_chapter_action, SIGNAL(triggered()), this, SLOT(actionSplitToNextChapter()));
    connect(split_to_multi_chapters_action, SIGNAL(triggered()), this, SLOT(actionSplitToMultiChapters()));
    connect(word_count_action, SIGNAL(triggered()), this, SLOT(actionWordCount()));
    connect(simplified_to_traditional_action, SIGNAL(triggered()), this, SLOT(actionSimplifiedToTraditional()));
    connect(traditional_to_simplified_action, SIGNAL(triggered()), this, SLOT(actionTraditionalToSimplified()));
    connect(simple_beauty_action, SIGNAL(triggered()), this, SLOT(actionSimpleBeauty()));
    connect(similar_sentence_action, SIGNAL(triggered()), this, SLOT(actionSimilarSentence()));
    connect(predict_sentence_action, SIGNAL(triggered()), this, SLOT(actionPredictSentence()));
    connect(error_word_highlight_action, SIGNAL(triggered()), this, SLOT(actionErrorWordHighlight()));
    connect(word_correct_action, SIGNAL(triggered()), this, SLOT(actionWordCorrect()));
    connect(sensitive_action, SIGNAL(triggered()), this, SLOT(actionSensitiveHighlight()));
    connect(sensitive_eliminate_action, SIGNAL(triggered()), this, SLOT(actionSensitiveEliminate()));
    connect(sensitive_replace_action, SIGNAL(triggered()), this, SLOT(actionSensitiveReplace()));
    connect(sensitive_replace_pinyin_action, SIGNAL(triggered()), this, SLOT(actionSensitiveReplacePinyin()));
    connect(sensitive_replace_first_action, SIGNAL(triggered()), this, SLOT(actionSensitiveReplaceFirst()));
    connect(sensitive_edit_list_action, SIGNAL(triggered()), this, SLOT(actionSensitiveEditList()));
    connect(sensitive_edit_replace_action, SIGNAL(triggered()), this, SLOT(actionSensitiveEditReplace()));
    connect(add_card_action, SIGNAL(triggered()), this, SLOT(actionAddCard()));
    connect(see_card_action, SIGNAL(triggered()), this, SLOT(actionSeeCard()));
    connect(card_color_action, SIGNAL(triggered()), this, SLOT(actionChangeCardColor()));
    connect(card_all_appear_action, SIGNAL(triggered()), this, SLOT(actionCardAllAppear()));
    connect(jump_appear_action, SIGNAL(triggered()), this, SLOT(actionJumpCardAppear()));
    connect(jump_disappear_action, SIGNAL(triggered()), this, SLOT(actionJumpCardDisappear()));
}

void ChapterEditor::refreshMenu()
{
    // 获取选中的内容
    QString text = toPlainText();
    int pos = textCursor().position();
    QString sel_str = "";
    if (textCursor().hasSelection())
        sel_str = textCursor().selectedText();
    bool seled = !(sel_str.isEmpty()); // 是否有选中内容
    bool is_dir_novel = (getInfo().getType() == EditType::EDIT_TYPE_CHAPTER && getInfo().getNovelName() == rt->current_novel);
    QClipboard *cb = QApplication::clipboard();
    QString cb_str = cb->text();
    bool edit_blank = isBlankString(toPlainText());

    undo_action->setEnabled(recorders_pos > 1);
    redo_action->setEnabled(recorders_pos < recorders.size());
    paste_action->setEnabled(!cb_str.isEmpty());
    select_all_action->setEnabled(sel_str.length() != toPlainText().length());

    if (seled) //有选区
    {
        cut_action->setText("剪切 (&X)");
        copy_action->setText("复制 (&C)");
        cut_action->setEnabled(true);
        cut_action->setEnabled(true);

        move_to_prev_chapter_end_action->setText("移动选区到上一章末尾");
        move_to_next_chapter_start_action->setText("移动选区到下一章开头");
        move_to_prev_chapter_end_action->setEnabled(true);
        move_to_next_chapter_start_action->setEnabled(true);
    }
    else // 没有选区
    {
        cut_action->setText("剪切全文 (&X)");
        copy_action->setText("复制全文 (&C)");
        if (!edit_blank)
        {
            cut_action->setEnabled(true);
            copy_action->setEnabled(true);
        }
        else
        {
            cut_action->setEnabled(false);
            copy_action->setEnabled(false);
        }

        // 是否是全文开头
        move_to_prev_chapter_end_action->setText("移动前文到上一章末尾");
        if (isAtEditStart())
        {
            move_to_prev_chapter_end_action->setEnabled(false);
        }
        else
        {
            move_to_prev_chapter_end_action->setEnabled(true);
        }

        // 是否是全文结尾
        move_to_next_chapter_start_action->setText("移动后文到下一章开头");
        if (isAtEditEnd())
        {
            move_to_next_chapter_start_action->setEnabled(false);
            split_to_next_chapter_action->setEnabled(false);
        }
        else
        {
            move_to_next_chapter_start_action->setEnabled(true);
            if (text.mid(pos - 1, 1) == "\n" || text.mid(pos, 1) == "\n") // 换行边缘方行
                split_to_next_chapter_action->setEnabled(true);
            else
                split_to_next_chapter_action->setEnabled(false);
        }
    }

    if (!cb_str.isEmpty() && us->typeset_paste && cb_str.indexOf("\n") > -1)
        paste_action->setText("粘贴并排版 (&V)");
    else
        paste_action->setText("粘贴 (&V)");

    int board_index = us->getInt("recent/board", 0);
    if (board_index == 0)
        boards_action->setText("剪贴板");
    else if (board_index == 1)
        boards_action->setText("回收板");
    else
        boards_action->setText("收藏板");
    if (editing.isCompact())
        boards_action->setEnabled(false);

    if (!edit_blank)
        typeset_action->setEnabled(true);
    else
        typeset_action->setEnabled(false);

    sensitive_action->setChecked(us->sensitive_highlight);
    if (us->sensitive_highlight)
        sensitive_action->setIcon(QIcon(":icons/menu/warning_enabled"));
    else
        sensitive_action->setIcon(QIcon(":icons/menu/warning_disabled"));
    error_word_highlight_action->setChecked(us->error_word_highlight);
    if (us->error_word_highlight)
        error_word_highlight_action->setIcon(QIcon(":icons/menu/error_word_highlight_enabled"));
    else
        error_word_highlight_action->setIcon(QIcon(":icons/menu/error_word_highlight"));


    // 是否是当前作品，如果不是打开的小说，则禁止所有章节选项
    chapter_menu->setEnabled(is_dir_novel);

    // 未开发
    split_to_multi_chapters_action->setEnabled(false);
    simple_beauty_action->setEnabled(false);
    card_all_appear_action->setEnabled(false);
    jump_appear_action->setEnabled(false);
    jump_disappear_action->setEnabled(false);

    // 多线程检查名片选项是否可用
    if (gd->clm.currentLib() != nullptr && (editing.isChapter() || editing.isOutline()))
    {
        add_card_action->setEnabled(true);
        QtConcurrent::run([ = ]
        {
            CardBean *card = getCardByTextCursor(-1);
            see_card_action->setEnabled(card != nullptr);
            cardlib_menu->setEnabled(card != nullptr);
        });
    }
    else
    {
        add_card_action->setEnabled(gd->clm.currentLib() != nullptr);
        see_card_action->setEnabled(false);
        cardlib_menu->setEnabled(false);
    }
}

void ChapterEditor::actionComprehensive()
{
    emit signalShowSearchPanel();
}

void ChapterEditor::actionUndo()
{
    undoOperator();
}

void ChapterEditor::actionRedo()
{
    redoOperator();
}

void ChapterEditor::actionCut()
{
    beforeInputted("");

    if (textCursor().hasSelection())
        this->cut();
    else
    {
        QString str = toPlainText();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(str);
        setPlainText("");
    }

    if (!us->clip_global)
    {
        gd->boards.tryAddClip(QApplication::clipboard()->text());
    }
}

void ChapterEditor::actionCopy()
{
    if (textCursor().hasSelection())
        this->copy();
    else
    {
        QString str = toPlainText();
        if (us->chapter_mark)
            str = NovelTools::getMainBodyWithoutMark(str);
        QClipboard *cb = QApplication::clipboard();
        cb->setText(str);
    }

    if (!us->clip_global)
    {
        gd->boards.addClip(QApplication::clipboard()->text());
    }
}

void ChapterEditor::actionCopyWithTitle()
{
    QString str = toPlainText();
    if (us->chapter_mark)
        str = NovelTools::getMainBodyWithoutMark(str);

    str = "        " + getInfo().getFullChapterName();

    QClipboard *cb = QApplication::clipboard();
    cb->setText(str);

    if (!us->clip_global)
    {
        gd->boards.addClip(QApplication::clipboard()->text());
    }
}

void ChapterEditor::actionCopyWithChapterMark()
{
    if (textCursor().hasSelection())
        this->copy();
    else
    {
        QString str = toPlainText();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(str);
    }

    if (!us->clip_global)
    {
        gd->boards.addClip(QApplication::clipboard()->text());
    }
}

void ChapterEditor::actionPaste()
{
    this->paste();
}

void ChapterEditor::actionPasteWithTypeset()
{
    // 临时开启排版，粘贴后还原
    bool typeset = us->typeset_paste;
    us->typeset_paste = true;
    actionPaste();
    us->typeset_paste = typeset;
}

void ChapterEditor::actionPasteWithoutTypeset()
{
    // 临时关闭排版，粘贴后还原
    bool typeset = us->typeset_paste;
    us->typeset_paste = false;
    actionPaste();
    us->typeset_paste = typeset;
}

void ChapterEditor::actionBoards()
{
    if (editing.isCompact())
        return ;
    BoardWidget *bw = new BoardWidget(this);
    connect(bw, SIGNAL(signalInsertText(QString)), this, SLOT(insertPlainText(QString)));

    bw->show();

    QTimer::singleShot(0, [ = ] // 不知道为什么 BoardWidget 的 resize比较慢，所以需要一个时钟
    {
        QRect rect = cursorRect();
        QRect geo = this->geometry();
        rect.moveTopLeft(mapToGlobal(rect.topLeft())); // 因为 BoardWidget 设置了 QDialog 属性，那么参照的坐标系就是全局的了，需要将编辑框位置转换为全局坐标
        geo.moveTopLeft(mapToGlobal(geo.topLeft()));
        bw->move(rect.bottomRight());
        if (bw->geometry().bottom() > geo.top() + geometry().height()) // 超过高度
            bw->move(bw->geometry().left(), rect.top() - bw->geometry().height());
        if (bw->geometry().right() > geo.left() + geometry().width()) // 超过宽度
            bw->move(rect.left() - bw->geometry().width(), bw->geometry().top());

        QPixmap *pixmap = new QPixmap(bw->size());
        bw->render(pixmap);
        bw->hide();
        ZoomGeometryAnimationWidget *ani = new ZoomGeometryAnimationWidget(this, pixmap, cursorRect(), QRect(mapFromGlobal(bw->pos()), bw->size()), true);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [ = ]{
            bw->show();
            bw->setFocus();
            bw->enableAutoDelete();
            connect(bw, SIGNAL(signalHidden(QPixmap, QRect)), this, SLOT(slotSubWidgetHidden(QPixmap, QRect)));
            delete pixmap;
        });
    });
}

void ChapterEditor::actionSearch()
{
    if (textCursor().hasSelection() && textCursor().selectedText().indexOf("\n") == -1/*没有换行*/)
        emit signalShowSearchBar(textCursor().selectedText(), false);
    else
        emit signalShowSearchBar("", false);
}

void ChapterEditor::actionReplace()
{
    if (textCursor().hasSelection() && textCursor().selectedText().indexOf("\n") == -1/*没有换行*/)
        emit signalShowSearchBar(textCursor().selectedText(), true);
    else
        emit signalShowSearchBar("", true);
}

void ChapterEditor::actionSelectAll()
{
    this->selectAll();
}

void ChapterEditor::actionWordCount()
{
    NovelTools::showWordCountDetail(toPlainText(), getInfo().getFullChapterName(), this);
}

void ChapterEditor::actionTypeset()
{
    this->operatorTypeset();
}

void ChapterEditor::actionSensitiveHighlight()
{
    us->sensitive_highlight = !us->sensitive_highlight;
    thm->updateHighlight();
}

void ChapterEditor::actionLocateOutline()
{
    emit signalLocateOutline(getInfo().getNovelName(), toPlainText());
}

void ChapterEditor::actionLocateChapter()
{
    emit signalLocateChapter(getInfo().getNovelName(), getInfo().getChapterName());
}

void ChapterEditor::actionOpenPrevChapter()
{
    emit signalOpenPrevChapter(getInfo().getNovelName(), getInfo().getChapterName());
}

void ChapterEditor::actionOpenNextChapter()
{
    emit signalOpenNextChapter(getInfo().getNovelName(), getInfo().getChapterName());
}

void ChapterEditor::actionAddNextChapterFaster()
{
    if (textCursor().hasSelection())
    {
        QString sel = textCursor().selectedText();
        int start = textCursor().selectionStart();
        int end = textCursor().selectionEnd();
        emit signalAddNextChapterFaster(sel);

        onlyDeleteText(start, end);
    }
    else
    {
        emit signalAddNextChapterFaster("");
    }
}

void ChapterEditor::actionMoveToPrevChapterEnd()
{
    if (isAtEditStart()) return ;

    if (textCursor().hasSelection())
    {
        QString sel = textCursor().selectedText();
        int start = textCursor().selectionStart();
        int end = textCursor().selectionEnd();
        emit signalMoveToPrevChapterEnd(getInfo().getNovelName(), getInfo().getChapterName(), sel);

        onlyDeleteText(start, end);
    }
    else
    {
        QString text = toPlainText();
        int pos = textCursor().position();
        QString sel = text.left(pos);
        emit signalMoveToPrevChapterEnd(getInfo().getNovelName(), getInfo().getChapterName(), sel);

        // 剩下的开头开头进行排版
        int p = pos, len = text.length(), cur = -1;
        //        bool is_all_blank = true;
        while (p < len && isBlankChar(text.mid(p, 1)))
            p++;
        if (p != len) // 如果后面不是全部空白
        {
            // 删除p后面的所有空内容，只剩下排版后的正文
            p = pos;
            while (p < len && isBlankChar(text.mid(p, 1))) // 删除空字符
                p++;
            cur = p;
            while (p > pos && isBlankChar2(text.mid(p - 1, 1))) // 恢复空格缩进
                p--;
            pos = p;
            if (p == cur)
                cur = -1;
        }

        onlyDeleteText( 0, pos );
        if (cur > pos)
            moveCursorFinished(cur - pos);
    }
}

void ChapterEditor::actionMoveToNextChapterStart()
{
    if (isAtEditEnd()) return ;

    if (textCursor().hasSelection())
    {
        QString sel = textCursor().selectedText();
        int start = textCursor().selectionStart();
        int end = textCursor().selectionEnd();
        emit signalMoveToNextChapterStart(getInfo().getNovelName(), getInfo().getChapterName(), sel);

        onlyDeleteText(start, end);
    }
    else
    {
        QString text = toPlainText();
        int pos = textCursor().position();
        QString sel = text.right(text.length() - pos);
        emit signalMoveToNextChapterStart(getInfo().getNovelName(), getInfo().getChapterName(), sel);

        onlyDeleteText(pos, text.length());
    }
}

void ChapterEditor::actionSplitToNextChapter()
{
    if (getInfo().getNovelName() != rt->current_novel) return ;
    int pos = textCursor().position();
    QString text = toPlainText();
    QString next_chapter_text = text.right(text.length() - pos);
    onlyDeleteText(pos, text.length());
    emit signalAddNextChapter(next_chapter_text);
}

void ChapterEditor::actionSplitToMultiChapters()
{

}

void ChapterEditor::actionSimplifiedToTraditional()
{
    QString content = readTextFile(":/lexicons/simplified_traditional");

    QStringList lines = content.split("\n", QString::SkipEmptyParts);
    QStringList simplifieds, traditionals;
    foreach (QString line, lines)
    {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        if (list.length() < 2) continue;
        simplifieds.append(list.at(0));
        traditionals.append(list.at(1));
    }

    operatorMultiReplace(simplifieds, traditionals);
}

void ChapterEditor::actionTraditionalToSimplified()
{
    QString content = readTextFile(":/lexicons/simplified_traditional");

    QStringList lines = content.split("\n", QString::SkipEmptyParts);
    QStringList simplifieds, traditionals;
    foreach (QString line, lines)
    {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        if (list.length() < 2) continue;
        simplifieds.append(list.at(1));
        traditionals.append(list.at(0));
    }

    operatorMultiReplace(simplifieds, traditionals);
}

void ChapterEditor::actionErrorWordHighlight()
{
    if (us->error_word_highlight)
    {
        us->error_word_highlight = false;
    }
    else
    {
        us->error_word_highlight = true;
        gd->lexicons.loadErrorWords();
    }
    thm->updateHighlight();
}

void ChapterEditor::actionWordCorrect()
{
    QString path = rt->DATA_PATH + "lexicon/translation/词语纠错.txt";
    QString content = readTextFile(path);

    QStringList lines = content.split("\n", QString::SkipEmptyParts);
    QStringList error_words, correct_words;
    foreach (QString line, lines)
    {
        QStringList list = line.split("\t", QString::SkipEmptyParts);
        if (list.length() < 2) continue;
        error_words.append(list.at(0));
        correct_words.append(list.at(1));
    }

    operatorMultiReplace(error_words, correct_words);
}

void ChapterEditor::actionSensitiveEliminate()
{
    QStringList sens = gd->lexicons.getSensitiveWords();
    operatorMultiEliminate(sens);
}

void ChapterEditor::actionSensitiveReplace()
{
    QStringList old_words, new_words;
    QString text = readTextFile(rt->DATA_PATH + "lexicon/sensitive/敏感词替换.txt");
    if (text.trimmed().isEmpty())
    {
        if (QMessageBox::information(this, "空内容", "当前替换资源没有词语，是否前往添加？", "添加替换词", "取消", 0, 1) == 0)
            actionSensitiveEditReplace();
        return ;
    }
    QStringList lines = text.split("\n");
    foreach (QString line, lines)
    {
        QStringList words = line.split("\t", QString::SkipEmptyParts);
        if (words.size() != 2)
            words = line.split(" ", QString::SkipEmptyParts);
        if (words.size() != 2)
            continue;
        old_words.append(words.at(0));
        new_words.append(words.at(1));
    }
    if (old_words.size() > 0)
        operatorMultiReplace(old_words, new_words);
    else if (QMessageBox::information(this, "无法读取内容", "当前替换资源不是正确的格式，是否前往修改？", "修改替换词", "取消", 0, 1) == 0)
        actionSensitiveEditReplace();
}

void ChapterEditor::actionSensitiveReplacePinyin()
{
    QStringList sens_words, sens_pinyins;
    sens_words = readTextFile(rt->DATA_PATH + "lexicon/sensitive/敏感词.txt").split(" ", QString::SkipEmptyParts);
    foreach (QString word, sens_words)
    {
        QString pinyin = PinyinUtil::ChineseConvertPinyin_str(word);
        if (pinyin.isEmpty())
            pinyin = word;
        sens_pinyins.append(pinyin);
    }
    if (sens_words.size() > 0)
        operatorMultiReplace(sens_words, sens_pinyins);
    else if (QMessageBox::information(this, "无法读取内容", "无敏感词资源，是否前往添加？", "添加敏感词", "取消", 0, 1) == 0)
        actionSensitiveEditList();
}

void ChapterEditor::actionSensitiveReplaceFirst()
{
    QStringList sens_words, sens_pinyins;
    sens_words = readTextFile(rt->DATA_PATH + "lexicon/sensitive/敏感词.txt").split(" ", QString::SkipEmptyParts);
    foreach (QString word, sens_words)
    {
        QString pinyin = "";
        for (int i = 0; i < word.length(); i++)
        {
            QString pf = PinyinUtil::ChineseConvertPinyin_str(word.at(i)).left(1);
            pinyin += pf.toUpper(); // 转换成大写
        }
        if (pinyin.isEmpty())
            pinyin = word;
        sens_pinyins.append(pinyin);
    }
    if (sens_words.size() > 0)
        operatorMultiReplace(sens_words, sens_pinyins);
    else if (QMessageBox::information(this, "无法读取内容", "无敏感词资源，是否前往添加？", "添加敏感词", "取消", 0, 1) == 0)
        actionSensitiveEditList();
}

void ChapterEditor::actionSensitiveEditList()
{
    QString path = rt->DATA_PATH + "lexicon/sensitive/敏感词.txt";
    ensureDirExist(rt->DATA_PATH + "lexicon/sensitive");
    InputDialog id(this, "敏感词列表", path, "自定义小说违禁词，用于高亮/替换\n以空格分隔多个敏感词");
    id.exec();

    gd->lexicons.initSensitive();
}

void ChapterEditor::actionSensitiveEditReplace()
{
    QString path = rt->DATA_PATH + "lexicon/sensitive/敏感词替换.txt";
    ensureDirExist(rt->DATA_PATH + "lexicon/sensitive");
    InputDialog id(this, "敏感词列表", path, "多行，每行两列，用空格或者Tab分隔\n每行的第一列文本替换为第二列文本");
    id.exec();
}

void ChapterEditor::actionSimpleBeauty()
{

}

void ChapterEditor::actionAddCard()
{
    emit signalAddNewCard();
}

void ChapterEditor::actionSeeCard()
{
    showCardlibEditor();
}

void ChapterEditor::actionChangeCardColor()
{
    CardBean *card = getCardByTextCursor();
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
    gd->clm.currentLib()->saveToFile(card);

    emit gd->clm.signalCardColorChanged(card);
    emit gd->clm.signalRehighlight();
}

void ChapterEditor::actionCardAllAppear()
{

}

void ChapterEditor::actionJumpCardAppear()
{

}

void ChapterEditor::actionJumpCardDisappear()
{

}

QString ChapterEditor::getPrecedingParaText()
{
    int pos = textCursor().position();
    QString text = toPlainText();

    // 去掉末尾空
    while (pos > 0 && isBlankChar(text.mid(pos - 1, 1)))
        pos--;
    if (pos <= 0)
        return "";
    QString left = text.left(pos);

    // 获取左边
    int left_pos = left.lastIndexOf("\n");
    if (left_pos == -1)
        left_pos = 0;

    // 去掉开头空（去除包括全角空格，不确定 trimmed 行不行）
    while (left_pos <= pos && isBlankChar(text.mid(left_pos, 1)))
        left_pos++;

    QString judge_content = left.mid(left_pos, pos - left_pos);
    return judge_content;
}

QString ChapterEditor::getPrecedingSentText()
{
    QString judge_content = getPrecedingParaText();
    if (judge_content.isEmpty())
        return "";
    QRegExp re("[？，。！“”—…（）]");
    QStringList list = judge_content.trimmed().split(re, QString::SkipEmptyParts);
    int index = list.size();
    int char_len = 0;
    QString sents; // 避免一个字一个字的实在是太短了
    while (--index >= 0)
    {
        char_len += list.at(index).trimmed().length();
        sents = list.at(index).trimmed() + sents;
        if (char_len >= 3)
            break;
    }
    return sents.trimmed();
}

/**
 * 根据上文，在小说资源中获取相似的
 */
void ChapterEditor::actionSimilarSentence()
{
    QString text = getPrecedingSentText();
    if (text.isEmpty())
        return ;

    if (predict_completer != nullptr)
        predict_completer->disable(); // 禁用之前的，避免出现问题
    predict_completer = new PredictCompleter(textCursor().position(), this);
    connect(predict_completer, &PredictCompleter::signalPredictResult, this, [=](QStringList list) {
        if (list.size() == 0 || textCursor().position() != predict_completer->getPosition()) // 没有找到或位置已经改变
            return ;
        QStringListModel *model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width + 10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        gd->lexicons.setMatchedCase(completer_case = COMPLETER_PREDICT_SIMILAR);
        predict_completer = nullptr; // 预测类中自动删除，此处不应该留存
    });
    gd->pb.startSimilar(text, predict_completer);
}

/**
 * 根据上文，在小说资源中获取可能的下一句
 */
void ChapterEditor::actionPredictSentence()
{
    QString text = getPrecedingSentText();
    if (text.isEmpty())
        return;

    if (predict_completer != nullptr)
        predict_completer->disable(); // 禁用之前的，避免出现问题
    predict_completer = new PredictCompleter(textCursor().position(), this);
    connect(predict_completer, &PredictCompleter::signalPredictResult, this, [=](QStringList list) {
        if (list.size() == 0 || textCursor().position() != predict_completer->getPosition()) // 没有找到或位置已经改变
            return ;
        _PDTTEST << "11111111111";
        QStringListModel *model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width + 10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        gd->lexicons.setMatchedCase(completer_case = COMPLETER_PREDICT_NEXT);
        predict_completer = nullptr;
        _PDTTEST << "22222222222";
    });
    gd->pb.startPredict(text, predict_completer);
}
