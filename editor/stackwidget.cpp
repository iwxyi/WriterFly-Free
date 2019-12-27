#include "stackwidget.h"

StackWidget::StackWidget(QWidget *parent) : QStackedWidget(parent)
{
    /*QGraphicsDropShadowEffect* e_shadow = new QGraphicsDropShadowEffect(this);
    e_shadow->setOffset(-8, 0);
    this->setGraphicsEffect(e_shadow);*/

    _flag_isNew = false;
    _flag_last = -1;
    _flag_aim = -1;
    _open_pixmap = new QPixmap(this->size());

    editor_search_bar = nullptr;

    this->setMinimumWidth(200);
    this->setContentsMargins(5, 0, 0, 5);

    connect(ac, &SyncThread::signalChapterUpdated, [=](QString novel_name, QString chapter_name){
            for (int i = 0; i < editors.size(); i++)
            {
                EditingInfo info = editors[i]->getInfo();
                if (info.isChapter() && info.getNovelName() == novel_name && info.getChapterName() == chapter_name)
                {
                    editors[i]->reloadContent();
                    return ;
                }
            }
    });

    connect(us, &USettings::signalBlurTitlebarChanged, [=](bool b){
        _open_pixmap = new QPixmap(QSize(width(), height()-(us->editor_fuzzy_titlebar ? us->widget_size : 0)));
        if (b)
        {
            for (int i = 0; i < editors.size(); i++)
                editors.at(i)->setTitleBar(titlebar, p_editor_blur_win_bg_pixmap);
        }
    });
}

void StackWidget::slotOpenChapter(QString novelName, QString chapterName, QString fullChapterName)
{
    us->setVal("recent/chapter", chapterName);
    us->setVal("recent/fullChapterName", fullChapterName);

    // 检查是否已经打开了
	for (int i = 0; i < editors.size(); i++)
    {
        EditingInfo info = editors[i]->getInfo();
        if (info.isChapter() && info.getNovelName() == novelName && info.getChapterName() == chapterName)
        {
            switchEditor(i);
            _flag_isNew = false;
			return ;
        }
	}

    // 关闭超过上限数量的标签
    if (editors.size() > us->tab_maxnum)
	{
        ; // 不管了
	}

	// 打开新的标签页
    int now = _flag_last; // 打开前的标签页索引
    _flag_isNew = false; // 需要这样才能开启动画，不知道为什么之前flag会是true
    createEditor(novelName, chapterName, fullChapterName); // 标签切换动画是在这个时候的

    if (us->open_recent && editors.size() == 1          // 如果是打开第一章（启动后自动打开的那一章节）（可能会有bug？）
            && novelName == us->getVar("recent/novel") && chapterName == us->getVar("recent/chapter"))
    {
        slotResotrePosition(); // 还原光标和滚动的位置
        changeWindowTitle();
        QTimer::singleShot(20, [=]{ // 好像只要比标准的 10 慢一点就行了？（Why?）
            slotDelayFocus();
        });
        _flag_last = 0;
        _flag_aim = -1;
        return ;
    }
    else if (editors.size() == 1) // 只有一个 editor，没办法使用动画
    {
        changeWindowTitle();
        _flag_last = 0;
        _flag_aim = -1;
        return ;
    }

    if (us->open_chapter_animation) // 开启动画：先切换回去，在动画结束后再切换到新的
    {
        _flag_aim = this->currentIndex();

        // 在切换前设置宽度为零，然后再设置为用户设定的宽度，避免光标跳动（不知道行不行，不过看起来没有生效）
        ChapterEditor* editor = editors[currentIndex()];
            editor->setCursorWidth(0);

        // 将widget的内容渲染到pixmap对象中
        QRect rect = us->editor_fuzzy_titlebar ? QRect(geometry().left(), geometry().top()+us->widget_size, width(), height()-us->widget_size) : geometry();
        if (us->integration_window)
            parentWidget()->parentWidget()->render(_open_pixmap, QPoint(0,0), rect/*本身相对于主窗口的位置*/); // 父类的父类，画这种背景……
        else
            parentWidget()->parentWidget()->render(_open_pixmap, QPoint(0,0), QRect(parentWidget()->geometry().topLeft(), this->geometry().size())); // 要计算dock的位置

        editor->setCursorWidth(us->editor_cursor_width);

        if (now != -1)
        {
            // switchEditor(now); // 切换回原来的标签页
            setCurrentIndex(now);
        }
        else if (count() == 1 && !rt->is_initing ) // 打开第一个，并且不是在初始化
        { // 打开后数量还是为一的
            editors[0]->hide();
        }
    }
}

void StackWidget::slotOpenAnimation(QPoint point)
{
    if (!_flag_isNew || !us->open_chapter_animation || _flag_aim == -1) return ;

    // 获取当前页面的widget
    //QPixmap* pixmap = new QPixmap(this->size());
    QWidget* w = editors[this->currentIndex()];

    // 将widget的内容渲染到pixmap对象中
    //((QWidget*)w->parent())->render(pixmap); // 不用父类画的话会黑背景*/

    // 绘制当前的widget
    StackWidgetAnimation* animation = new StackWidgetAnimation(this->parentWidget(), _open_pixmap, point, w->pos()+pos()+QPoint( 0, us->editor_fuzzy_titlebar?us->widget_size: 0), w->size() - QSize( 0, us->editor_fuzzy_titlebar?us->widget_size: 0), _flag_aim);
    connect(animation, SIGNAL(signalAni1Finished(int)), this, SLOT(slotOpenAnimationFinished(int)));
}

void StackWidget::slotOpenAnimationFinished(int x)
{
    if (count() >= 1 && x == 0/*1表示下标0*/)
    {
        editors[0]->show();
    }
    // switchEditor(x);
    setCurrentIndex(x);
    _flag_isNew = false;
    if (editor_search_bar != nullptr)
    {
        // 因为为了避免切换三次，在切换的途中就都退出了，并没有设置为新的编辑框，所以需要在动画结束的时候设置
        editor_search_bar->setAimEditor(getCurrentEditor());
    }
}

void StackWidget::slotDeleteChapter(QString novelName, QString chapterName)
{
    for (int i = 0; i < editors.size(); i++)
    {
        if ( editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == chapterName )
        {
            if (currentIndex() == i)
            {
                // 先渲染再切换和删除
                parentWidget()->parentWidget()->render(_open_pixmap, QPoint(0,0), geometry());

                // 切换到其他章节 // TODO 弄一个章节打开的栈
                if (currentIndex() > 0) // 切换到上一个
                {
                    switchEditor(currentIndex()-1);
                    ChapterEditor* edit = editors[currentIndex()];
                    us->setVal("recent/novel", edit->getInfo().getNovelName());
                    us->setVal("recent/chapter", edit->getInfo().getChapterName());
                }
                else if (currentIndex() < editors.size()-1) // 切换到下一个
                {
                    switchEditor(currentIndex()+1);
                    ChapterEditor* edit = editors[currentIndex()];
                    us->setVal("recent/novel", edit->getInfo().getNovelName());
                    us->setVal("recent/chapter", edit->getInfo().getChapterName());
                }
                else // 最后一个章节
                {
                    if (editor_search_bar != nullptr)
                    {
                        editor_search_bar->setAimEditor(nullptr);
                        slotHideEditorSearchBar();
                    }
                }

                // 删除章节的动画
                QRect sr(geometry());
                QRect er(sr.left()+sr.width()/2, sr.top()+sr.height()/2, 1, 1);
                /*ZoomGeometryAnimationWidget* animation = */new ZoomGeometryAnimationWidget(this->parentWidget(), _open_pixmap, sr, er, -1);
            }
            this->removeWidget(editors[i]);
            delete editors.at(i);
            editors.removeAt(i);
        }
    }
}

void StackWidget::slotDeleteChapters(QString novelName, QStringList chapterNames)
{
    for (QString n : chapterNames)
    {
        slotDeleteChapter(novelName, n);
    }
}

void StackWidget::slotChangeName(bool isChapter, QString novelName, QString oldName, QString newName)
{
    if (!isChapter) return ;

    log("重命名章节："+novelName+" / "+oldName+" >> "+newName);
    for (int i = 0; i < editors.size(); i++)
    {
        if ( editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == oldName )
        {
            editors[i]->getInfo().setChapterName(newName);
            log(QString("编辑器[%1].name = %2 >> %3").arg(i).arg(oldName).arg(newName));
            if (i == this->currentIndex()) // 正在编辑的章节，修改程序标题
            {
                log("重命名正在编辑的章节标题");
                //emit signalChangeTitleByChapter(editors[i]->getInfo().getFullName());
                emit signalSetTitleFull(editors[i]->getInfo().getFullChapterName());
            }
            if (us->getStr("recent/novel") == editors[i]->getInfo().getNovelName() && us->getStr("recent/chapter") == oldName)
            {
                us->setVal("recent/chapter", newName);
            }
            break;
        }
    }
}

void StackWidget::slotChangeFullName(QString novelName, QString chapterName, QString newFullName)
{
    for (int i = 0; i < editors.size(); i++)
    {
        if ( editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == chapterName )
        {
            editors[i]->getInfo().setFullChapterName(newFullName);
            log(QString("编辑器[%1](%2) = %3").arg(i).arg(chapterName).arg(newFullName));
            if (i == this->currentIndex()) // 正在编辑的章节，修改程序标题
            {
                emit signalSetTitleFull(newFullName);
            }
            if (us->getStr("recent/novel") == novelName && us->getStr("recent/chapter") == chapterName)
            {
                us->setVal("recent/fullChapterName", newFullName);
            }
            break;
        }
    }
}

void StackWidget::slotAddNextChapter(QString text)
{
    emit signalAddNextChapter("", text);
}

void StackWidget::slotAddNextChapterFaster(QString text)
{
    emit signalAddNextChapterFaster("", text, true);
}

void StackWidget::slotLocateOutline(QString novel_name, QString chapter_content)
{
    emit signalLocateOutline(novel_name, chapter_content);
}

void StackWidget::slotLocateChapter(QString novel_name, QString chapter_name)
{
    emit signalLocateChapter(novel_name, chapter_name);
}

void StackWidget::slotOpenPrevChapter(QString novel_name, QString chapter_name)
{
    emit signalOpenPrevChapter(novel_name, chapter_name);
}

void StackWidget::slotOpenNextChapter(QString novel_name, QString chapter_name)
{
    emit signalOpenNextChapter(novel_name, chapter_name);
}

void StackWidget::slotMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text)
{
    emit signalMoveToPrevChapterEnd(novel_name, chapter_name, text);
}

void StackWidget::slotMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text)
{
    emit signalMoveToNextChapterStart(novel_name, chapter_name, text);
}

void StackWidget::slotCloseEditor(int index)
{
	if (index >= editors.size()) return ;

    ChapterEditor* edit = editors[index];
    EditingInfo info = edit->getInfo();
    edit->beforeClose(); // 删除前保存

    emit signalChapterClosed(info.getNovelName(), info.getChapterName());

    this->removeWidget(edit);
    editors.removeAt(index);
    edit->deleteLater();
}

void StackWidget::slotCloseAllEditors()
{
    while (editors.size() > 0)
        slotCloseEditor( 0 );

    _flag_last = -1;
    _flag_aim = -1;
    _flag_isNew = -1;

    us->setVal("recent/chapter", "");
    us->setVal("recent/fullChapterName", "");
}

void StackWidget::slotTextChanged()
{

}

void StackWidget::updateUI()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->updateUI();
    }
}

void StackWidget::slotUpdateAllEditor()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->updateEditorMarked();
    }
}

void StackWidget::slotEditorBottomSettingsChanged()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->slotEditorBottomSettingsChanged();
    }
}

void StackWidget::slotEditorWordCount(int x)
{
    emit signalEditorWordCount(x);
    // emit signalSetTitleWc(x);
}

void StackWidget::slotSavePosition()
{
    if (editors.size() == 0)
    {
        us->setVal("recent/cursor", -1);
        us->setVal("recent/scroll", -1);
        return ;
    }
    ChapterEditor* edit = editors[currentIndex()];
    int pos = edit->textCursor().position();
    int scr = edit->verticalScrollBar()->sliderPosition();
    us->setVal("recent/cursor", pos);
    us->setVal("recent/scroll", scr);
    if (edit->textCursor().hasSelection())
    {
        us->setVal("recent/selection_start", edit->textCursor().selectionStart());
        us->setVal("recent/selection_end", edit->textCursor().selectionEnd());
    }
    else
    {
        us->setVal("recent/selection_start", -1);
        us->setVal("recent/selection_end", -1);
    }
    us->sync();
}

void StackWidget::slotResotrePosition()
{
    if (editors.size() == 0) return ;
    ChapterEditor* edit = editors[currentIndex()];
    int pos = us->getInt("recent/cursor");
    int scr = us->getInt("recent/scroll");
    if (pos > 0 && pos <= edit->toPlainText().length())
    {
        QTextCursor text_cursor = edit->textCursor();
        text_cursor.setPosition(pos);
        int selection_start = us->getInt("recent/selection_start", -1),
            selection_end = us->getInt("recent/selection_end", -1);
        if (selection_start > -1 && selection_end > -1)
        {
            if (pos == selection_start)
            {
                int temp = selection_start;
                selection_start = selection_end;
                selection_end = temp;
            }
            text_cursor.setPosition(selection_start, QTextCursor::MoveAnchor);
            text_cursor.setPosition(selection_end, QTextCursor::KeepAnchor);
        }
        edit->setTextCursor(text_cursor);
        edit->closeCompleteTimerOnce();
    }
    if (scr > 0/* && scr < edit->verticalScrollBar()->maximum()*/)
    {
        edit->verticalScrollBar()->setSliderPosition(scr); // 后面有延迟再重新设置一遍
    }
    QTimer::singleShot(10, [=]{
        slotDelayFocus();
        slotDelayScroll();
    });
}

void StackWidget::slotDelayFocus()
{
    // 不知道为什么，一定要延迟一段时间才行……
    // 猜测可能是，点击列表项打开章节，当时鼠标还按在那个列表项上，焦点强行聚在列表项，编辑框获取不到
    // 也可能是因为初始化时，其他控件自动抢走了焦点
    if (editors.size() == 0) return ;
    this->setFocus(Qt::ActiveWindowFocusReason);
    if (currentIndex()<0) return ;
    editors[currentIndex()]->setFocus(Qt::ActiveWindowFocusReason);
}

void StackWidget::slotDelayScroll()
{
    // editor 触发 scrollbar 的 rangeChanged 事件需要一时间
    // 触发后调整下方的空白
    // 但是直接加载的话，下面的空白可能没有调整好，高度不够，此时只能滚动到document的位置
    // 延迟一段时间，就能在 resetScrollRange 后再进行滚动
    if (editors.size() == 0) return ;
    if (currentIndex()<0) return ;
    int scr = us->getInt("recent/scroll");
    ChapterEditor* edit = editors[currentIndex()];
    edit->verticalScrollBar()->setSliderPosition(scr);
}

void StackWidget::slotShowEditorSearchBar(QString def, bool rep)
{
    if (editor_search_bar == nullptr)
    {
        editor_search_bar = new EditorSearchWidget(this);
        //connect(editor_search_bar, SIGNAL(signalSearchPrev(QString)), this, SLOT(slotEditorSearchPrev(QString)));
        //connect(editor_search_bar, SIGNAL(signalSearchNext(QString)), this, SLOT(slotEditorSearchNext(QString)));
        connect(editor_search_bar, SIGNAL(signalClosed()), this, SLOT(slotHideEditorSearchBar()));
    }
    editor_search_bar->setAimEditor(getCurrentEditor());
    editor_search_bar->setText(def);
    editor_search_bar->showReplace(rep);

    if (editor_search_bar->isHidden())
    {
        // 显示动画
        editor_search_bar->show();
        //editor_search_bar->move(0, 0);
        int margin = 10 + (us->editor_fuzzy_titlebar ? us->widget_size : 0);
        QPoint sp(geometry().width(), margin);
        QPoint ep(geometry().width()-editor_search_bar->width()-margin, margin);
        QPropertyAnimation* ani = new QPropertyAnimation(editor_search_bar, "pos");
        ani->setStartValue(sp);
        ani->setEndValue(ep);
        ani->setDuration(300);
        ani->setEasingCurve(QEasingCurve::OutQuart);
        ani->start();
        connect(ani, &QPropertyAnimation::finished, [=]{
            ani->deleteLater();
        });
        editor_search_bar->setFocus();

//        editor_search_bar->showButtonAnimation();
    }
}

void StackWidget::slotHideEditorSearchBar()
{
    for (int i = 0; i < editors.count(); i++)
        editors[i]->setSearchKey("");

    // 隐藏动画
    int margin = 10 + (us->editor_fuzzy_titlebar ? us->widget_size : 0);
    QPoint sp(editor_search_bar->pos());
    QPoint ep(geometry().width()+10, margin);
    QPropertyAnimation* ani = new QPropertyAnimation(editor_search_bar, "pos");
    ani->setStartValue(sp);
    ani->setEndValue(ep);
    ani->setDuration(300);
    ani->setEasingCurve(QEasingCurve::OutQuart);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=]{
        editor_search_bar->hide();
        ani->deleteLater();
    });

    this->slotDelayFocus();
}

void StackWidget::switchEditor(int index)
{
    if (index >= this->count())
        return ;
    if (currentIndex() == index) // 无需切换
    {
        getCurrentEditor()->setFocus();

        // 名片库
        // 只有一个，可能就是打开的第一个？
        // 不过名片库延迟加载了，启动时打开的话可能还没有初始化完成呢
        if (index == 0 && this->count() == 1)
            emit signalEditorSwitched(getCurrentEditor());
        return ;
    }

    this->setCurrentIndex(index);
    _flag_last = index;
//    if (!us->one_click) // 不知道为什么要放在这里？
//        editors[index]->setFocus();

    if (_flag_isNew == true) // 这个会切换三次，故延迟动画
        return ;

    changeWindowTitle();
    editors[index]->setFocus();
    editors[index]->reloadContent();
    editors[index]->blurTitleBar();

    if (editor_search_bar != nullptr)
        editor_search_bar->setAimEditor(editors[index]);

    emit signalEditorSwitched(editors[index]);
}

void StackWidget::createEditor(QString novelName, QString chapterName, QString fullChapterName)
{
    ChapterEditor* editor = new ChapterEditor(this, novelName, chapterName, fullChapterName);
	editors.append(editor);

    editor->setTitleBar(titlebar, p_editor_blur_win_bg_pixmap);

    connect(editor, SIGNAL(signalWordCount(int)), this, SLOT(slotEditorWordCount(int)));
    connect(editor, SIGNAL(signalUpdateAllEditor()), this, SLOT(slotUpdateAllEditor()));

    connect(editor, SIGNAL(signalShowSearchBar(QString, bool)), this, SLOT(slotShowEditorSearchBar(QString, bool)));
    connect(editor, SIGNAL(signalHidePanels()), this, SLOT(slotHidePanels()));
    connect(editor, SIGNAL(signalShowSearchPanel()), this, SLOT(slotShowSearchPanel()));

    connect(editor, SIGNAL(signalAddNextChapter(QString)), this, SLOT(slotAddNextChapter(QString)));
    connect(editor, SIGNAL(signalAddNextChapterFaster(QString)), this, SLOT(slotAddNextChapterFaster(QString)));
    connect(editor, SIGNAL(signalLocateOutline(QString, QString)), this, SLOT(slotLocateOutline(QString, QString)));
    connect(editor, SIGNAL(signalLocateChapter(QString, QString)), this, SLOT(slotLocateChapter(QString, QString)));
    connect(editor, SIGNAL(signalOpenPrevChapter(QString, QString)), this, SLOT(slotOpenPrevChapter(QString, QString)));
    connect(editor, SIGNAL(signalOpenNextChapter(QString, QString)), this, SLOT(slotOpenNextChapter(QString, QString)));
    connect(editor, SIGNAL(signalMoveToPrevChapterEnd(QString, QString, QString)), this, SLOT(slotMoveToPrevChapterEnd(QString, QString, QString)));
    connect(editor, SIGNAL(signalMoveToNextChapterStart(QString, QString, QString)), this, SLOT(slotMoveToNextChapterStart(QString, QString, QString)));

    connect(editor, &ChapterEditor::signalShowCardEditor, [=](CardBean *card){ emit signalShowCardEditor(card); });
    connect(editor, &ChapterEditor::signalAddNewCard, [=]{ emit signalAddNewCard(); });

    this->addWidget(editor);

    log("创建编辑器", novelName+"/"+chapterName);
    switchEditor(editors.size()-1);

    _flag_isNew = true;

    emit signalEditorCountChanged(editors.size());
}

ChapterEditor *StackWidget::getCurrentEditor()
{
    if (currentIndex()<0) return nullptr;
    return editors[currentIndex()];
}

QList<ChapterEditor *> StackWidget::getEditors()
{
    return editors;
}

QList<QTextEdit *> StackWidget::getTextEditors()
{
    QList<QTextEdit *>list;
    for (ChapterEditor* ce : editors)
        list.append(static_cast<QTextEdit*>(ce));
    return list;
}

void StackWidget::resizeEvent(QResizeEvent *e)
{
    if (_open_pixmap != nullptr) {
        ;
    }//delete open_pixmap; // 出bug就注释掉这一行
    _open_pixmap = new QPixmap(QSize(width(), height()-(us->editor_fuzzy_titlebar ? us->widget_size : 0)));

    return QStackedWidget::resizeEvent(e);
}

void StackWidget::changeWindowTitle()
{
    int index = currentIndex();
    if (index < 0 || index >= count())
        return ;
    EditingInfo info = editors[index]->getInfo();

    // 其实不用也没事的，因为除了章节之外，没有连接信号槽，到不了标题栏（但是就怕万一啊）
    if (info.getType() == EditType::EDIT_TYPE_CHAPTER)
    {
        emit signalSetTitleNovel(editors[index]->getInfo().getNovelName());
        emit signalSetTitleChapter(editors[index]->getInfo().getChapterName());
        emit signalSetTitleFull(editors[index]->getInfo().getFullChapterName());
        editors[index]->updateWordCount();
    }
}

void StackWidget::setTitleBar(QWidget* widget, QPixmap** p_bg)
{
    this->titlebar = widget;
    this->p_editor_blur_win_bg_pixmap = p_bg;
}

void StackWidget::slotRenameNovel(QString oldName, QString newName)
{
    // 是否修改了当前打开的编辑器名字
    int index = currentIndex();
    if (index > -1 && editors[index]->getInfo().getType()==EditType::EDIT_TYPE_CHAPTER && editors[index]->getInfo().getNovelName() == oldName)
    {
        emit signalSetTitleNovel(newName);
    }

    for (int i = 0; i < editors.size(); i++)
    {
        if (editors[index]->getInfo().getType()==EditType::EDIT_TYPE_CHAPTER && editors[i]->getInfo().getNovelName() == oldName)
        {
            editors[i]->getInfo().setNovelName(newName);
        }
    }
}

void StackWidget::slotDeleteNovel(QString novelName)
{
    // 是否关闭了当前打开的编辑器
    bool isSwitch = false;
    int index = currentIndex();
    if (index > -1 && editors[index]->getInfo().getType()==EditType::EDIT_TYPE_CHAPTER && editors[index]->getInfo().getNovelName() == novelName)
    {
        isSwitch = true;
    }

    for (int i = editors.size()-1; i >= 0; i--)
    {
        if (editors[index]->getInfo().getType()==EditType::EDIT_TYPE_CHAPTER && editors[i]->getInfo().getNovelName() == novelName)
        {
            slotCloseEditor(i);
        }
    }

    // 更换标题
    if (isSwitch) // 是否删除当前作品
    {
        if (editors.size() > 0)
        {
            switchEditor(editors.size()-1);
        }
        else
        {
            emit signalChangeTitleByChapter(APPLICATION_NAME);
        }
    }
}

void StackWidget::slotShowSearchPanel()
{
    emit signalShowSearchPanel("");
}

void StackWidget::slotHidePanels()
{
    if (editor_search_bar != nullptr && !editor_search_bar->isHidden())
    {
        slotHideEditorSearchBar();
        return ;
    }
}

bool StackWidget::execCommand(QString command, QStringList args)
{
    if (command == "关闭所有章节")
    {
        slotCloseAllEditors();
    }
    else if (command == "关闭当前章节")
    {
        if (currentIndex() != -1)
            slotCloseEditor(currentIndex());
    }
    else if (command == "搜索" && args.size())
    {
        slotShowEditorSearchBar(args.at(0), false);
    }
    else if (command == "替换" && args.size() >= 1)
    {
        slotShowEditorSearchBar(args.at(0), true);
        if (args.size() >= 2)
            editor_search_bar->setReplaceKey(args.at(1));
    }
    else if (command == "综合搜索" && args.size())
    {
        emit signalShowSearchPanel(args.at(0));
    }
    else
        return false;
    return true;
}
