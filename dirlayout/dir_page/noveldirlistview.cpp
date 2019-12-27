#include <QWidget>
#include "noveldirlistview.h"

NovelDirListView::NovelDirListView(QWidget *parent) : QListView(parent)
{
    pre_index = -1;
    pressed_time1 = pressed_time2 = 0;
    released_time = 0;
    current_item = nullptr;
    _flag_no_rename_by_added = false;

    // 设置 delegate
    novel_dir_delegate = new NovelDirDelegate(this);
    setItemDelegate(novel_dir_delegate);

    // 设置 model
    novel_dir_model = new NovelDirModel(this);
    setModel(novel_dir_model);

    initStyle();
    initMenu();

    connect(novel_dir_model, SIGNAL(signalListDataChange()), this, SLOT(slotInitRowHidden()));
    connect(novel_dir_model, SIGNAL(signalReedit()), this,SLOT(slotReedit()));
    connect(novel_dir_model, SIGNAL(signalOpenCurrentChapter()), this, SLOT(slotOpenCurrentChapter()));
    connect(novel_dir_model, SIGNAL(signalOpenChapter(int)), this, SLOT(slotOpenChapter(int)));
    connect(novel_dir_model, SIGNAL(signalFocuItem(int)), this, SLOT(slotFocuItem(int)));
    connect(novel_dir_model, SIGNAL(signalChangeFullName(QString,QString)), this, SLOT(slotChangeFullName(QString,QString)));
    connect(novel_dir_delegate, SIGNAL(signalUpdateIndex(QModelIndex)), this, SLOT(slotUpdateIndex(QModelIndex)));

    setAcceptDrops(true);

    // 读取最近的一章
    QTimer::singleShot(100, [=]{
        int last_index = us->getInt("recent/index");
        if (last_index >= 0 && last_index < novel_dir_model->getRcCount())
        {
            setCurrentIndex(model()->index(last_index, 0));
            int pos = us->getInt("recent/dir_scroll", -1);
            if (pos >= 0)
                verticalScrollBar()->setSliderPosition(pos);
            else
                scrollTo(model()->index(last_index, 0), QAbstractItemView::PositionAtCenter);

            NovelDirItem* item = novel_dir_model->getItem(last_index);
            if (item->getName() == us->getStr("recent/chapter") || item->getName() == fnDecode(us->getStr("recent/chapter")))
            {
                item->open(); // 这个只是设置为打开状态，和打开章节的操作无关
                item->setSelecting(true);
                item->setPressed(QPoint(0, 0));
                item->setReleased();
            }
        }
    });

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void NovelDirListView::initStyle()
{
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->update();
}

void NovelDirListView::updateUI()
{
    thm->setWidgetStyleSheet(this->verticalScrollBar(), "scrollbar");
    thm->setWidgetStyleSheet(this->menu, "menu");
    thm->setWidgetStyleSheet(this->directory_menu, "menu");
    thm->setWidgetStyleSheet(this->settings_menu, "menu");
    thm->setWidgetStyleSheet(this->im_ex_menu, "menu");
}

void NovelDirListView::readNovel(QString novelName)
{
    this->novel_name = novelName;
    novel_dir_delegate->readSettings(novelName);
    novel_dir_model->readDir(novelName);

    int roll_count = novel_dir_model->getRollCount();
    Q_UNUSED(roll_count);

    // if (novel_dir_model->getRollCount() == 2) // 只有两个，必定是有一个正文
    if (novel_dir_model->getRollCount() >= 2) // 只有两个，必定是有一个正文
    {
        int index = novel_dir_model->getListIndexByRoll(1);
        this->setCurrentIndex(novel_dir_model->index(index, 0));
        current_item = novel_dir_model->getItem(currentIndex().row());
        current_item->setSelecting(true);
        current_item->setPressed(QPoint(0, 0));
    }
    else if (novel_dir_model->getRollCount() == 1) // 只有作品相关
    {
        this->setCurrentIndex(novel_dir_model->index(0, 0));
        current_item = novel_dir_model->getItem(currentIndex().row());
        current_item->setSelecting(true);
        current_item->setPressed(QPoint(0, 0));
    }
    else // 聚焦到最后一章
    {
        // 算了，随缘吧……
    }
}

void NovelDirListView::contextMenuEvent(QContextMenuEvent *event)
{
    if (current_item == nullptr)
        return ;
    // QListView::contextMenuEvent(event);
    extern_point = mapFromGlobal(QCursor::pos()); // 右键菜单的操作，从点击的地方开始
    refreshMenu();
    menu->exec(QCursor::pos());
}

/**
 * 这个函数没啥用了诶
 */
void NovelDirListView::initRecent()
{
    if (us->open_recent) {
        // 打开上次读取的章节（如果存在）
        QString recent_open_novel = us->getStr("recent/novel");
        QString recent_open_chapter = us->getStr("recent/chapter");
        QString recent_open_fullChapterName = us->getStr("recent/fullChapterName");
        int recent_open_roll_index = us->getInt("recent/roll_index", -1);
        int recent_open_chapter_index = us->getInt("recent/chapter_index", -1);

        if (recent_open_novel.isEmpty() || recent_open_chapter.isEmpty()  // 空的
            || recent_open_novel != novel_name )
            return ;

        if (novel_dir_model->isExistChapter(recent_open_roll_index, recent_open_chapter_index))
        // if (novel_dir_model->isExistChapter(recent_open_chapter))
        {
            emit signalOpenChapter(novel_name, recent_open_chapter, recent_open_fullChapterName);
        }
        else
        {
            us->setVal("recent/roll_index", -1);
            us->setVal("recent/chapter_index", -1);
        }
    }

}

/*
 * 打开作品时，恢复目录的上一次滚动位置和选中项目
 */
void NovelDirListView::restoreDirectory()
{
    /*int slide = novel_dir_delegate->getInt("scroll_slide");
    int index = novel_dir_delegate->getInt("current_index");

    if (index >= 0)
        setCurrentIndex(novel_dir_model->index(index));
    if (slide > 0)
        verticalScrollBar()->setSliderPosition(slide);*/
}

void NovelDirListView::mousePressEvent(QMouseEvent *event) // 左键右键的点击都能获取
{
    QListView::mousePressEvent(event);

    pressed_time1 = pressed_time2;
    pressed_time2 = getTimestamp();

    if (current_item != nullptr)
        current_item->setSelecting(false);

    if (!currentIndex().isValid() || !indexAt(event->pos()).isValid()) // 空白处
    {
        current_item = nullptr;
        return ;
    }
    else
    {
        int index = currentIndex().row();
        current_item = novel_dir_model->getItem(index);

        if (current_item != nullptr)
        {
            // current_item->resetActiveProg();
            current_item->setSelecting(true);
            // QPoint pos = event->pos() - this->rectForIndex(currentIndex()).topLeft(); // 相对 index 来说。但是 index 绘制的就是整个 listview 的区域，没必要
            // current_item->setClickPoint(event->pos());
            current_item->setPressed(event->pos());
        }
        us->setVal("recent/index", index);
    }

    if (event->button() == Qt::LeftButton)
    {
        drag_start_pos = event->pos();
    }

}

void NovelDirListView::mouseMoveEvent(QMouseEvent *event)
{
    if (current_item == nullptr || !currentIndex().isValid()) return ;
    /**
     * 不能用 if (event-> button() != Qt::LeftButton) return;
     * button返回“哪个按钮发生了此事件”，buttons返回"发生事件时哪些按钮还处于按下状态"
     * 移动事件不是“按钮产生的”，只检测按下弹起
     */
    if (!(event->buttons() & Qt::LeftButton)) return QListView::mouseMoveEvent(event);

    int distance = (event->pos() - drag_start_pos).manhattanLength();

    if (distance < QApplication::startDragDistance()) // 距离太小，不算移动
        return ;

    // 开始拖拽
    QMimeData * mime_data = new QMimeData;
    mime_data->setText(current_item->getName());

    QList<QUrl>urls;
    urls << getIndexFileTemp(currentIndex()); // 导出文件
    mime_data->setUrls(urls);

    /*QRect rect = rectForIndex(currentIndex());
    QPixmap pixmap(rect.size());
    QRect index_rect(rect.left(), rect.top()-verticalOffset(), rect.width(), rect.height());
    parentWidget()->render(&pixmap, QPoint(0,0), QRect(rect.left()+geometry().left(), rect.top()+geometry().top()-verticalOffset(), rect.width(), rect.height()));
    mime_data->setImageData(pixmap); // 带图片*/

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mime_data);
	// drag->setPixmap(pixmap);
	Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
	if (dropAction == Qt::MoveAction)
	{
        // delete item; // 删除原来的
	}
}

void NovelDirListView::mouseReleaseEvent(QMouseEvent* event)
{
    if (current_item == nullptr)
        return QListView::mouseReleaseEvent(event);

    int distance = (event->pos() - drag_start_pos).manhattanLength();
    if (distance >= QApplication::startDragDistance()) // 开始拖拽放开
    {
        startDrag(Qt::MoveAction);
    }
    else if (currentIndex().isValid() && indexAt(event->pos()).isValid()/*不是在空地方松开*/)
    {
        int index = currentIndex().row(); // 行数
        current_item = novel_dir_model->getItem(index);

        if (us->one_click && event->button() == Qt::LeftButton) // 点击打开
        {
            emitItemEvent(index);
        }
        current_item->setReleased();

        pre_index = index; // 保留上次的
    }
    released_time = getTimestamp();

    QListView::mouseReleaseEvent(event);
}

void NovelDirListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (us->one_click)
        return ;//QListView::mouseDoubleClickEvent(event); // 取消双击编辑

    int index = currentIndex().row(); // 行数
    emitItemEvent(index);

    QListView::mouseDoubleClickEvent(event);
}

void NovelDirListView::keyPressEvent(QKeyEvent *event)
{
    return QListView::keyPressEvent(event);

    /* 取消 Enter 键激活 Item，因为和重命名确定冲突了
    if (!currentIndex().isValid())
        return ;
    if (event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return)
        return QListView::keyPressEvent(event);

    int index = currentIndex().row(); // 行数
    currentItem = novelDirModel->getItem(index);

    emitItemEvent(index);
    pre_index = index; // 保留上次的*/
}

void NovelDirListView::keyboardSearch(const QString &search)
{
    int count = novel_dir_model->data(model()->index(-1, 0), Qt::UserRole+DRole_CR_COUNT).toInt();

    // 寻找标题
    for (int i = 0; i < count; i++)
    {
        QModelIndex index = novel_dir_model->index(i,0);
        QString name = novel_dir_model->data(index, Qt::UserRole+DRole_CHP_NAME).toString();
        if (name.indexOf(search) > -1)
        {
            setCurrentIndex(index);
            scrollTo(index, ScrollHint::PositionAtCenter);
            return QListView::keyboardSearch(search);
        }
    }

    // 寻找序号


    // 寻找内容（太卡了，还是算了）


    QListView::keyboardSearch(search);
}

/**
 * @brief NovelDirListView::emitItemEvent
 * 激活一个 item（oneclick单击、双击、Enter）
 * @param index
 */
void NovelDirListView::emitItemEvent(int index)
{
    /*if (current_item != nullptr && current_item != novel_dir_model->getItem(index))
    {
        current_item->setSelecting(false);
    }
    setCurrentIndex(novel_dir_model->index(index)); // 不定位到这个位置不能打开啊
    current_item = novel_dir_model->getItem(index);
    current_item->setSelecting(true);*/

    selectIndex(index);
    if (current_item != nullptr && current_item->isRoll())
    {
        bool hide = !current_item->isHide();
        current_item->setHide(hide);

        int i = index;
        while (++i < novel_dir_model->rowCount())
        {
            NovelDirItem* item = novel_dir_model->getItem(i);
            if (item->isRoll()) break;
            item->setHide(hide);
            setRowHidden(i, hide);
        }
        if (hide) // 原来是隐藏状态，展开，确保能见到更多
        {
            scrollTo(novel_dir_model->index(i,0));
            scrollTo(novel_dir_model->index(index,0));
        }

        // 保存Hide情况到文件
        novel_dir_model->setRollHide(index, hide);
    }
    else // 是章节，单击打开章节
    {
        slotOpenCurrentChapter();
    }
}

void NovelDirListView::slotOpenChapter(int index) // 这个是从model的edit完毕事件里面传回来的
{
//    if (current_item != novel_dir_model->getItem(index) || !current_item->isSelecting())
    {
        selectIndex(index);
    }
    if (novel_dir_model->getItem(index)->isRoll())
    {
        us->setVal("recent/index", index);
        return ;
    }

    slotOpenCurrentChapter();
}

void NovelDirListView::slotChapterClosed(QString novel_name, QString chapter_name)
{
    if (novel_name != this->novel_name) // 不是同一本书，算了，不管了
        return ;
    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index == -1) return ;

    NovelDirItem* item = novel_dir_model->getItem(index);
    item->close();
    update();
}

void NovelDirListView::slotOpenCurrentChapter()
{
    if (!currentIndex().isValid()) return ;
    int index = currentIndex().row(); // 行数
    current_item = novel_dir_model->getItem(index);
    //us->setVal("recent/chapter_index", index); // 这是打开章节的index，和recent/index有所区分

    QString chapter_name = current_item->getName();
    QString full_chapter_name = novel_dir_delegate->getItemText(currentIndex());

    emit signalOpenChapter(novel_name, chapter_name, full_chapter_name);

    QRect ir = rectForIndex(currentIndex()); // index中的矩形
    QRect r(ir.left(), ir.top()-verticalOffset()+geometry().top(), ir.width(), ir.height()); // ListView 中的坐标
    QPoint p = mapFromGlobal(QCursor::pos())+this->parentWidget()->pos()+QPoint(0,geometry().top()); // 相对于listview的光标位置
    if (!r.contains(p))
    {
        p.setX(r.left()+r.width()-10);
        p.setY(r.top()+r.height()/2-5);
    }
    else
    {
        int delta_right = ir.width()*0.8-p.x()-10; // 从右侧10px处打开
        if (delta_right<10)
            delta_right = 10;
        p.setX(p.x()+delta_right);
    }

    emit signalOpenAnimationStartPos(p);

    current_item->open();
    us->setVal("recent/index", index);
    us->setVal("recent/roll_index", current_item->getRollIndex());    // 保存最近分组
    us->setVal("recent/chapter_index", current_item->getChptIndex()); // 保存最近子章

    update(); // 打开章节后，立即更新界面，避免无法显示已经打开的标记
}

void NovelDirListView::slotOpenChapter(QString name)
{
    int index = novel_dir_model->getIndexByName(name);
    if (index < 0) return ;
    // 效果等同于 emitItemEvent();
    slotOpenChapter(index);
}

void NovelDirListView::dragMoveEvent(QDragMoveEvent* event)
{
    if (!currentIndex().isValid()) return ;
    NovelDirListView * source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
            //(NovelDirListView*)((void*)(event->source()));
    if (source && source == this)
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else if (source && source != this) // 其他列表拖过来的
    {
        ;
    }
}

void NovelDirListView::dragEnterEvent(QDragEnterEvent* event)
{
    if (!currentIndex().isValid()) return ;
    NovelDirListView * source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
	if (source && source == this)
	{
		event->setDropAction(Qt::MoveAction);
        event->acceptProposedAction();
	}
    else if (source && source != this) // 本软件其他列表拖过来的
    {

    }
    else if (!source) // 无法判断的source，外部软件
    {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.size() == 1)
        {
            QString str = urls.at(0).toString();
            if (str.isEmpty()) return ;

            if (str.startsWith("file:///")) // 是文件（章），或者目录（卷）
            {
                QString path = str.right(str.length()-8);
                if (isFileExist(path) && (QFileInfo(path).isDir() || QFile(path).size() <= NOVEL_FILEINFO_SIZE)) // 10MB 以内
                    event->acceptProposedAction();
            }
            else // 纯文本
                event->acceptProposedAction();
        }
        else if (!(event->mimeData()->text()).isEmpty())
        {
            event->acceptProposedAction();
        }
    }
}

void NovelDirListView::dropEvent(QDropEvent* event)
{
    NovelDirListView* source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
	if (source && source == this)
	{
        drag_end_pos = event->pos();

        int org_index = indexAt(drag_start_pos).row();
        int aim_index = indexAt(drag_end_pos).row();
        novel_dir_model->exchangeItem(org_index, aim_index);

		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
    else if (source && source != this) // 其他列表拖过来的
    {

    }
    else if (!source) // 无法判断的source，外部软件
    {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.size() == 1)
        {
            QString str = urls.at(0).toString();
            if (str.isEmpty()) return ;
#if defined(Q_OS_WIN)
            //设置窗口置顶
            ::SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            ::SetWindowPos(HWND(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            this->show();
            this->activateWindow();
#endif
            this->setFocus(); // 获取焦点，后面重命名的时候可以直接重命名
            if (processDropUrl(str))
            {
                event->accept();
            }
        }
        else if (!(event->mimeData()->text()).isEmpty())
        {
#if defined(Q_OS_WIN)
            //设置窗口置顶
            ::SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            ::SetWindowPos(HWND(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            this->show();
            this->activateWindow();
#endif
            if (processDropText(event->mimeData()->text()))
                event->accept();
        }
    }
}

void NovelDirListView::wheelEvent(QWheelEvent *event)
{
    /*auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;*/

    if (/*!ctrl && !shift && !alt &&*/ us->smooth_scroll)
    {
        if (event->delta() > 0) // 上滚
            addSmoothScrollThread(-us->smooth_scroll_speed, us->smooth_scroll_duration);
        else if (event->delta() < 0)
            addSmoothScrollThread(us->smooth_scroll_speed, us->smooth_scroll_duration);
    }
    else
    {
        QListView::wheelEvent(event);
    }
}

void NovelDirListView::currentChange(const QModelIndex &current, const QModelIndex &previous)
{
    // 好像并没有触发？
    emit signalCurrentChanged(current, previous);
}

void NovelDirListView::initRowHidden()
{
    NovelDirModel *model = getModel();
    for (int i = 0; i < model->rowCount(); i++)
    {
        NovelDirItem* item = model->getItem(i);
        if (!item->isRoll() && item->isHide())
        {
            setRowHidden(i, true);
        }
    }

}

void NovelDirListView::slotInsertRoll()
{
    slotInsertRoll("");
}

void NovelDirListView::slotInsertRoll(QString new_name)
{
    if (current_item == nullptr) return ;
    if (!current_item->isRoll()) return ;
    int list_index = currentIndex().row();
    if (list_index <= 0) return ;
    if (list_index == 0)
    {
        QMessageBox::information(this, QObject::tr("你在干嘛"), QObject::tr("难道【作品相关】上面还能再加个卷？"));
        return ;
    }

    QString full_text = getModel()->getFullText();
    if (new_name.isEmpty())
    {
        int i = 1;
        while (novel_dir_model->isExistRoll("新卷"+QString::number(i)))
            i++;
        new_name = "新卷"+QString::number(i);
    }
    else if (novel_dir_model->isExistRoll(new_name))
    {
        int i = 1;
        while (novel_dir_model->isExistRoll(new_name+" ("+QString::number(i)+")"))
            i++;
        new_name = new_name+" ("+QString::number(i)+")";
    }

    int add_pos = novel_dir_model->insertRoll(list_index, new_name);
    QModelIndex modelIndex = model()->index(add_pos, 0);
    if (add_pos > 0)
    {
        setCurrentIndex(modelIndex);
        editItem(add_pos);
    }
}

void NovelDirListView::slotDeleteRoll()
{
    if (current_item == nullptr)
    {
        log("删除分卷失败：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();

    if (list_index > 0)
    {
        QStringList names = novel_dir_model->deleteRoll(list_index);
        emit signalDeleteChapters(novel_name, names);

        for (QString s : names)
        {
            if (!s.isEmpty() && isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(s)+".txt"))
            {
                deleteFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(s)+".txt");
            }
        }
        if (list_index < novel_dir_model->rowCount())
            setCurrentIndex(model()->index(list_index,0));
        else if (list_index > 1)
            setCurrentIndex(model()->index(list_index-1,0));
    }
    else
        QMessageBox::information(this, QObject::tr("删除分卷失败"), QObject::tr("【作品相关】是开发者罩着的，不允许动它！"));
}

void NovelDirListView::slotRenameRoll()
{
    if (currentIndex().row() > 0)
        editItem(currentIndex().row(), false); // 手动重命名函数
    else
        QMessageBox::information(this, QObject::tr("嘻嘻嘻"), QObject::tr("【作品相关】受到开发者庇佑，不允许更改名字"));
}

void NovelDirListView::slotRollWordCount()
{
    if (current_item == nullptr || !currentIndex().isValid())
    {
        log("字数统计：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();
    int count = currentIndex().data(Qt::UserRole+DRole_CR_COUNT).toInt();
    QString roll_name = novel_dir_delegate->getItemText(currentIndex());
    QString roll_full = "";
    int chapter_count = 0;
    while (++list_index < count)
    {
        QModelIndex ind = model()->index(list_index, 0);
        if (ind.data(Qt::UserRole+DRole_CHP_ISROLL).toBool())
            break;
        QString name = ind.data(Qt::UserRole+DRole_CHP_NAME).toString();
        if (isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt"))
        {
            roll_full += readTextFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt");
        }
        chapter_count++;
    }
    NovelTools::showWordCountDetail(
                QString(roll_full),
                roll_name+" ("+QString::number(chapter_count)+")", this);

}

void NovelDirListView::slotInsertChapter()
{
    slotInsertChapter("", "");
}

void NovelDirListView::slotInsertChapter(QString new_name, QString content)
{
    if (current_item == nullptr) return ;
    if (current_item->isRoll()) return ;
    int list_index = currentIndex().row();
    if (list_index <= 0) return ;
    if (list_index == 0)
    {
        QMessageBox::information(this, QObject::tr("你在干嘛"), QObject::tr("难道【作品相关】上面还能再加个章节？"));
        return ;
    }

    // 判断标题
    QString full_text = getModel()->getFullText();
    if (new_name.isEmpty()) // 自动创建标题
    {
        int i = 1;
        while (novel_dir_model->isExistChapter("新章"+QString::number(i)))
            i++;
        new_name = QObject::tr("新章%1").arg(i);
    }
    else if (novel_dir_model->isExistChapter(new_name))
    {
        int i = 1;
        while (novel_dir_model->isExistChapter(new_name+" ("+QString::number(i)+")"))
            i++;
        new_name = new_name+"("+QString::number(i)+")";
    }

    // 开始插入
    int add_pos = novel_dir_model->insertChapter(list_index, new_name);
    if (add_pos > 0) // 添加成功
    {
        editItem(add_pos);

        if (!content.isEmpty()) // 自动保存内容
        {
            QString chapter_path = rt->NOVEL_PATH + novel_name + "/chapters/" + fnEncode(new_name) + ".txt";
            writeTextFile(chapter_path, content);
        }
    }
}

int NovelDirListView::slotAddNextChapter(QString new_name, QString content)
{
    if (current_item == nullptr) return -1;
    if (current_item->isRoll()) return -1;
    int list_index = currentIndex().row();

    // 判断标题
    QString full_text = getModel()->getFullText();
    if (new_name.isEmpty()) // 自动创建标题
    {
        int i = 1;
        while (novel_dir_model->isExistChapter("新章"+QString::number(i)))
            i++;
        new_name = QObject::tr("新章%1").arg(i);
    }
    else if (novel_dir_model->isExistChapter(new_name))
    {
        int i = 1;
        while (novel_dir_model->isExistChapter(new_name+" ("+QString::number(i)+")"))
            i++;
        new_name = new_name+"("+QString::number(i)+")";
    }

    // 开始插入
    int add_pos = novel_dir_model->addNextChapter(list_index, new_name);
    if (add_pos > 0) // 添加成功
    {
        // 此处是编辑器直接信号传递过来的，需要重新修改一下动画方向
        extern_point = mapFromGlobal(QCursor::pos());

        editItem(add_pos);

        if (!content.isEmpty()) // 自动保存内容
        {
            QString chapter_path = rt->NOVEL_PATH + novel_name + "/chapters/" + fnEncode(new_name) + ".txt";
            writeTextFile(chapter_path, content);
        }
    }
    return add_pos;
}

void NovelDirListView::slotAddNextChapterFaster(QString new_name, QString content, bool open)
{
    _flag_no_rename_by_added = true;
    int index = slotAddNextChapter(new_name, content);
    _flag_no_rename_by_added = false;
    if (open && index >= 0)
    {
        emitItemEvent(index);
    }
}

void NovelDirListView::slotLocateChapter(QString novel_name, QString chapter_name)
{
    if (novel_name != this->novel_name) return ;

    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index > 0)
    {
        // 设置并滚动目标位置
        QModelIndex model_index = model()->index(index, 0);
        // setCurrentIndex(model_index);
        selectIndex(index);
        scrollTo(model_index, QAbstractItemView::PositionAtCenter);
    }

}

void NovelDirListView::slotOpenPrevChapter(QString novel_name, QString chapter_name)
{
    if (novel_name != this->novel_name) return ;

    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index < 0) return ;

    while (--index > 0 && novel_dir_model->getItem(index)->isRoll());
    if (index > 0)
        emitItemEvent(index);
}

void NovelDirListView::slotOpenNextChapter(QString novel_name, QString chapter_name)
{
    if (novel_name != this->novel_name) return ;

    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index < 0) return;

    while (++index < novel_dir_model->rowCount() && novel_dir_model->getItem(index)->isRoll()) ;
    if (index < novel_dir_model->rowCount())
        emitItemEvent(index);
}

void NovelDirListView::slotMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text)
{
    if (novel_name != this->novel_name) return ;

    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index < 0) return ;

    while (--index > 0 && novel_dir_model->getItem(index)->isRoll());
    if (index <= 0)// 没有上一章，创建一个
    {
        setCurrentIndex(novel_dir_model->index(novel_dir_model->getIndexByName(chapter_name)));
        slotInsertChapter("", text);
    }
    else // 读取下一章内容，并格式化
    {
        // 读取章节内容
        NovelDirItem* item = novel_dir_model->getItem(index);
        QString title = item->getName();
        QString content = readTextFile(getChapterPath(title));

        // 开始拼接
        content = (isAllBlank(content)
                    ? ""
                    : removeBlank(content, false, true) + repeatString("\n", us->indent_line) )
                + repeatString("　", us->indent_blank)
                + removeBlank(text, true, true);

        // 写回去
        writeTextFile(getChapterPath(title), content);
    }
}

void NovelDirListView::slotMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text)
{
    if (novel_name != this->novel_name) return ;

    int index = novel_dir_model->getIndexByName(chapter_name);
    if (index < 0) return;

    while (++index < novel_dir_model->rowCount() && novel_dir_model->getItem(index)->isRoll()) ;
    if (index >= novel_dir_model->rowCount())// 没有上一章，创建一个
    {
        setCurrentIndex(novel_dir_model->index(novel_dir_model->getIndexByName(chapter_name)));
        slotAddNextChapter("", text);
    }
    else // 读取下一章内容，并格式化
    {
        // 读取章节内容
        NovelDirItem* item = novel_dir_model->getItem(index);
        QString title = item->getName();
        QString content = readTextFile(getChapterPath(title));

        // 开始拼接
        content = repeatString("　", us->indent_blank)
                + removeBlank(text)
                + (isAllBlank(content)
                    ? ""
                    : repeatString("\n", us->indent_line) + content );

        // 写回去
        writeTextFile(getChapterPath(title), content);
    }
}

void NovelDirListView::slotDeleteChapter()
{
    if (current_item == nullptr)
    {
        log("删除章节失败：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();
    QString name = novel_dir_model->deleteChapter(list_index);

    if (list_index < novel_dir_model->rowCount() && !novel_dir_model->getItem(list_index)->isRoll()) // 聚焦到下一个章节
        setCurrentIndex(model()->index(list_index,0));
    else if (list_index > 1)
        setCurrentIndex(model()->index(list_index-1,0));

    emit signalDeleteChapter(novel_name, name); // 通过信号槽删除编辑器中的章节
    if (isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt"))
    {
        deleteFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt");
    }
}

void NovelDirListView::slotRenameChapter()
{
    if (currentIndex().isValid())
    {
        editItem(currentIndex().row(), false); // 手动重命名函数
    }
}

void NovelDirListView::slotChapterWordCount()
{
    if (current_item == nullptr)
    {
        log("字数统计：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();
    QString name = novel_dir_model->getChapterName(list_index);

    if (isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt"))
    {
        NovelTools::showWordCountDetail(
                    readTextFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt"),
                    novel_dir_delegate->getItemText(currentIndex(), false),
                    this);
    }
}

void NovelDirListView::slotChangeFullName(QString novel_name, QString chapter_name)
{
    int i = getModel()->getIndexByName(chapter_name);
    QModelIndex index = model()->index(i, 0);
    QString full_chapter_name = novel_dir_delegate->getItemText(index);
    emit signalChangeFullName(novel_name, chapter_name, full_chapter_name);
}

void NovelDirListView::slotAddRoll()
{
    slotAddRoll("");
}

void NovelDirListView::slotAddRoll(QString new_name)
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(parentWidget()))
            emit signalNeedLineGuide();
        return ;
    }
    //if (!(currentIndex().isValid())) return ;

    QString full_text = getModel()->getFullText();
    if (new_name.isEmpty())
    {
        int i = 1;
        while (novel_dir_model->isExistRoll("新卷"+QString::number(i)))
            i++;
        new_name = "新卷"+QString::number(i);
    }
    else if (novel_dir_model->isExistRoll(new_name))
    {
        int i = 1;
        while (novel_dir_model->isExistRoll(new_name+" ("+QString::number(i)+")"))
            i++;
        new_name = new_name+" ("+QString::number(i)+")";
    }

    addRoll(new_name);
}

void NovelDirListView::slotAddChapter()
{
    slotAddChapter("", "");
}

void NovelDirListView::slotAddChapter(QString new_name, QString content)
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(parentWidget()))
            emit signalNeedLineGuide();
        return ;
    }
    if (!(currentIndex().isValid()))
    {
        QMessageBox::information(this, QObject::tr("请先选中某一卷"), QObject::tr("请选择或者创建一个分卷后，再新建章节"));
        return ;
    }

    // 判断标题
    QString full_text = getModel()->getFullText();
    if (new_name.isEmpty()) // 自动创建标题
    {
        new_name = getSuitableChapterName(currentIndex().row(), true);
    }
    else if (novel_dir_model->isExistChapter(new_name))
    {
        int i = 1;
        while (novel_dir_model->isExistChapter(new_name+"("+QString::number(i)+")"))
            i++;
        new_name = new_name+"("+QString::number(i)+")";
    }

    if (addChapter(currentIndex().row(), new_name))
    {
        if (!content.isEmpty())
        {
            writeTextFile(getChapterPath(new_name), content);
        }
    }

}

void NovelDirListView::slotInitRowHidden()
{
    initRowHidden();
}

void NovelDirListView::slotReedit()
{
    //this->closeEditor(novelDirDelegate->getEditor(), QAbstractItemDelegate::NoHint);

    // 因为还在编辑时不能重新编辑，所以增加了延迟
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotRenameChapter()));
    timer->start(100);
}

/**
 * 聚焦item；解决重命名之后焦点丢失的问题
 * @param row  行数
 */
void NovelDirListView::slotFocuItem(int row)
{
    setCurrentIndex(model()->index(row, 0));
}

void NovelDirListView::slotUpdateIndex(QModelIndex index)
{
    if (index.isValid())
    {
        update(index);
    }
}

NovelDirModel *NovelDirListView::getModel()
{
    return novel_dir_model;
}

NovelDirDelegate *NovelDirListView::getDelegate()
{
    return novel_dir_delegate;
}

QString NovelDirListView::getNovelName()
{
    return novel_name;
}

int NovelDirListView::addRoll(QString newName)
{
    int add_pos = novel_dir_model->addRoll(newName);
    editItem(add_pos);
    return add_pos;
}

int NovelDirListView::addChapter(int index, QString newName)
{
    int add_pos = novel_dir_model->addChapter(index, newName); // 添加章节后返回的index
    editItem(add_pos);
    return add_pos;
}

void NovelDirListView::editItem(int index, bool use_ani)
{
    if (index <= 0) return ; // 0 是作品相关不能编辑
    if (_flag_no_rename_by_added) return ;

    QModelIndex model_index = model()->index(index, 0);

    // 设置当前位置
    setCurrentIndex(model_index);
    // selectIndex(index);
    scrollTo(model_index);
    current_item = novel_dir_model->getItem(index);
    this->update(model_index);

    // 编辑章节的动画
    if (us->open_chapter_animation && use_ani) // 开启动画
    {
        // 添加章节的动画
        QRect rect = rectForIndex(model_index); // item 的范围（相对于整个列表最顶端）
        QPixmap *pixmap = new QPixmap(rect.size());
        QRect index_rect(rect.left(), rect.top()-verticalOffset(), rect.width(), rect.height()); // item 相对于可视控件的几何
        parentWidget()->render(pixmap, QPoint(0,0), QRect(rect.left()+geometry().left(), rect.top()+geometry().top()-verticalOffset(), rect.width(), rect.height()));
        StackWidgetAnimation* ani = new StackWidgetAnimation(this, pixmap, extern_point, index_rect.topLeft(), index_rect.size(), index);
        connect(ani, SIGNAL(signalAni1Finished(int)), this, SLOT(slotAddAnimationFinished(int)));

        novel_dir_model->getItem(index)->setAnimating(true);
        novel_dir_model->listDataChanged();
    }
    else
    {
        edit(model_index); // 直接编辑章节名
    }
}

/**
 * 选中项目，并且添加点击效果
 */
void NovelDirListView::selectIndex(int index)
{
    if (current_item != nullptr && current_item != novel_dir_model->getItem(index)) // 如果不是当前的
        current_item->setSelecting(false);
    setCurrentIndex(novel_dir_model->index(index));
    current_item = novel_dir_model->getItem(index);
    if (current_item->isInOpenAnimating()) // 还在打开动画中
        return ;
    current_item->setSelecting(true);
    current_item->setPressed(QPoint(0,0));
    current_item->setReleased();
}

void NovelDirListView::slotAddAnimationFinished(int index)
{
    //setRowHidden(index, false);
    novel_dir_model->getItem(index)->setAnimating(false);
    novel_dir_model->listDataChanged();
    QModelIndex model_index = model()->index(index, 0);
    setCurrentIndex(model_index);
    scrollTo(model_index);
    edit(model_index);
}

void NovelDirListView::slotExternRect(QRect r)
{
    extern_rect = r;
}

void NovelDirListView::slotExternPoint(QPoint p)
{
    extern_point = p;
}

void NovelDirListView::slotExport()
{
    // 选取一个路径
    QString recent_export_path = us->getStr("recent/export_path");
    QString recent;
    if (!recent_export_path.isEmpty())
        recent = recent_export_path;
    else
        recent = us->getStr("recent/file_path");
    if (!isFileExist(recent))
        recent = "/home";
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择导出路径"), recent,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return ;
    us->setVal("recent/export_path", dir);

    QString file_path = getPathWithIndex(dir, novel_name, ".txt");

    if (exportNovel(file_path))
    {
        emit signalExportFinished();
    }
    else
    {
        QMessageBox::information(this, tr("导出失败"), QString("导出作品失败，路径：%1").arg(file_path));
    }
}

bool NovelDirListView::exportNovel(QString export_path)
{
    // 各种变量配置
    bool contains_about = us->getBool("export/about", false);
    bool contains_roll_split = us->getBool("export/split", true);
    bool contains_chpt_roll = us->getBool("export/roll", false);
    QString newline;
    int newline_index = us->getInt("export/newline", 0);
    if (newline_index == 1) newline = "\r\n"; // Window
    else if (newline_index == 2) newline = "\r"; // Mac
    else if (newline_index == 3) newline = "\n"; // Unix
    else newline = "\n";
    QString export_text = "";
    QString chapters_path = rt->NOVEL_PATH+novel_name+"/chapters/";
    NovelDirModel* model = novel_dir_model;
    NovelDirDelegate* delegate = novel_dir_delegate;

    // 导出的内容（后期应该可以手动调整）
    QString roll_prefix("　　　　　　");
    QString roll_suffix(newline + newline + newline);
    QString chapter_prefix("　　　　");
    QString chapter_suffix(newline + newline + newline);
    QString content_prefix("");
    QString content_suffix(newline + newline + newline);
    QString current_roll_name; // 现在正在遍历的卷

    // 遍历作品相关
    int traverse_start;
    if (contains_about)
        traverse_start = 0;
    else
        traverse_start = model->roll_subs[0]+1;

    // 遍历各卷
    int cr_len = model->getRcCount();
    for (int i = traverse_start; i < cr_len; i++) // 遍历每一个项目
    {
        NovelDirItem item = model->cr_list[i];
        if (item.isRoll()) // 是卷
        {
            if (contains_roll_split)
                export_text += roll_prefix + delegate->getItemText(model->index(i, 0)) + roll_suffix;
            current_roll_name = delegate->getItemText(model->index(i, 0));
        }
        else // 是章节
        {
            if (contains_chpt_roll)
                export_text += chapter_prefix + current_roll_name + " " +delegate->getItemText(model->index(i, 0)) + chapter_suffix;
            else
                export_text += chapter_prefix + delegate->getItemText(model->index(i, 0)) + chapter_suffix;

            // 读取内容并且导出
            QString content = readTextFile(chapters_path+fnEncode(item.getName())+".txt");
            if (us->chapter_mark)
                content = NovelTools::getMainBodyWithoutMark(content);
            export_text += content_prefix + content + content_suffix;
        }
    }

    writeTextFile(export_path, export_text);
    return true;
}

/**
 * 导入文件到当前小说（新书在导入窗口导入）
 * @param file_path  文件路径
 * @param split_roll 是否分卷
 * @param index      如果分卷，index==0时表示从最后一卷开始，否则新建一卷
 *                   如果不分卷，index<size时表示添加的卷下标，index>=size时表示新建一卷；
 */
void NovelDirListView::slotImport(QString file_path, bool split_roll, int index)
{
    Q_UNUSED(split_roll);

    // ==== 读取导入参数 ====
    bool auto_split_roll = us->getBool("import/auto_split_roll", true);
    bool keep_cr_number = us->getBool("import/keep_cr_number", false);
    bool add_new_roll = true;

    QString roll_reg = us->getStr("import/roll_regexp", "(第.{1,5}卷) +([^\\n]+)\\n");
    QString chpt_reg = us->getStr("import/chpt_regexp", "(第.{1,5}章) +([^\\n]+)\\n");

    if (novel_name.isEmpty() || file_path.isEmpty() || roll_reg.isEmpty() || chpt_reg.isEmpty())
        return ;
    if (!isFileExist(file_path))
        return ;

    // ==== 判断文件变量 ====
    QString novel_path = rt->NOVEL_PATH + novel_name;   // 小说文件夹
    QString dir_path = novel_path+"/" + NOVELDIRFILENAME;      // 目录文件
    QString chapter_path = novel_path+"/chapters/"; // 章节文件夹

    // ==== 判断小说目录 ====
    ensureDirExist(novel_path);
    ensureDirExist(novel_path+"/chapters");
    /*ensureDirExist(novel_path+"/details");
    ensureDirExist(novel_path+"/outlines");
    ensureDirExist(novel_path+"/recycles");*/

    QString total_content = readTextFile(file_path, us->getStr("import/codec", "GBK"));

    QRegExp roll_rx(roll_reg);
    QRegExp chpt_rx(chpt_reg);

    if (auto_split_roll) // 自动分卷
    {
        int add_roll_index = novel_dir_model->getRcCount()-1; // 暂定为最后一卷
        if (index == 0)
        {
            add_new_roll = false; // 从最后一卷开始添加
        }
        else
        {
            add_new_roll = true; // 新建一卷开始添加
        }
        int roll_count = 0, chpt_count = 0;
        int roll_pos = 0;
        while (1)
        {
            // 当前卷的起始位置
            roll_pos = roll_rx.indexIn(total_content, roll_pos);
            if (roll_pos == -1) break;
            roll_pos += roll_rx.matchedLength();

            // 目录内容加上卷名
            QString roll_numb_str = roll_rx.cap(1); // 卷序号
            QString roll_name_str = roll_rx.cap(2); // 卷名字
            if (roll_count > 0 || add_new_roll == true)
            {
                if (keep_cr_number)
                {
                    novel_dir_model->addRoll(QString("%1 %2").arg(roll_numb_str).arg(roll_name_str));
                }
                else
                {
                    novel_dir_model->addRoll(QString("%1").arg(roll_name_str));
                }
                //add_roll_index++; // 添加章节的目标卷的索引+1
                add_roll_index = novel_dir_model->getRcCount()-1;
            }

            // 下一卷的位置
            int next_roll_pos = roll_rx.indexIn(total_content, roll_pos);
            if (next_roll_pos == -1)
                next_roll_pos = total_content.length();

            // 解析章节
            // 从 roll_pos ~ nextroll_pos 中间的都是章节
            QString roll_content = total_content.mid(roll_pos, next_roll_pos-roll_pos);
            int chpt_pos = 0;
            while ( 1 )
            {
                // 这一章名的位置
                chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
                if (chpt_pos == -1) break;
                chpt_pos += chpt_rx.matchedLength();

                // 目录加上章节的名字
                QString chpt_numb_str = chpt_rx.cap(1);
                QString chpt_name_str = chpt_rx.cap(2);
                // 判断章节同名情况，加上后缀
                if (novel_dir_model->isExistChapter(chpt_name_str))
                {
                    int index = 1;
                    while (novel_dir_model->isExistChapter(QString("%1(%2)").arg(chpt_name_str).arg(index)))
                        index++;
                    chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
                }
                if (keep_cr_number) // addChapter 参数一：crList的列表索引，从索引中获取章节
                {
                    novel_dir_model->addChapter(add_roll_index, QString("%1 %2").arg(chpt_numb_str).arg(fnEncode(chpt_name_str)));
                }
                else
                {
                    novel_dir_model->addChapter(add_roll_index, QString("%1").arg(fnEncode(chpt_name_str)));
                }

                // 下一章名的位置
                int next_chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
                if (next_chpt_pos == -1)
                    next_chpt_pos = roll_content.length();

                // 获取章节正文
                QString chpt_content = roll_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
                chpt_content = simplifyChapter(chpt_content);
                QString chpt_file_path = chapter_path + fnEncode(chpt_name_str) + ".txt";
                writeTextFile(chpt_file_path, chpt_content);

                chpt_count++;
            }

            roll_count++;
        }

        QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n卷数：%2\n章数：%3").arg(novel_name).arg(roll_count).arg(chpt_count));
    }
    else // 导入到当前作品某一卷
    {
        // ==== 清除卷名行 ====
        int roll_pos = 0;
        while ( (roll_pos=roll_rx.indexIn(total_content, roll_pos)) != -1 )
            total_content = total_content.left(roll_pos) + total_content.right(total_content.length()-roll_pos-roll_rx.matchedLength());

        int add_roll_index = index;
        int chpt_count = 0;
        if (index < novel_dir_model->getRcCount())
        {
            add_new_roll = false; // 加到现有的卷
            add_roll_index = novel_dir_model->getListIndexByRoll(index);
        }
        else
        {
            add_new_roll = true; // 新建一卷再添加
            QFileInfo info(file_path);
            QString file_name = info.baseName();
            novel_dir_model->addRoll(file_name);
            add_roll_index = novel_dir_model->getRcCount()-1;
        }

        // ==== 开始导入 ====
        int chpt_pos = 0;
        while ( 1 )
        {
            // 章名位置
            chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
            if (chpt_pos == -1) break;
            chpt_pos += chpt_rx.matchedLength();

            QString chpt_numb_str = chpt_rx.cap(1);
            QString chpt_name_str = chpt_rx.cap(2);

            // 判断章节同名情况，加上后缀
            if (novel_dir_model->isExistChapter(chpt_name_str))
            {
                int index = 1;
                while (novel_dir_model->isExistChapter(QString("%1(%2)").arg(chpt_name_str).arg(index)))
                    index++;
                chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
            }
            if (keep_cr_number)
            {
                novel_dir_model->addChapter(add_roll_index, QString("%1 %2").arg(chpt_numb_str).arg(fnEncode(chpt_name_str)));
            }
            else
            {
                novel_dir_model->addChapter(add_roll_index, QString("%1").arg(fnEncode(chpt_name_str)));
            }

            // 下一章名的位置
            int next_chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
            if (next_chpt_pos == -1)
                next_chpt_pos = total_content.length();

            // 获取章节正文
            QString chpt_content = total_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
            chpt_content = simplifyChapter(chpt_content);
            QString chpt_file_path = chapter_path + fnEncode(chpt_name_str) + ".txt";
            writeTextFile(chpt_file_path, chpt_content);

            chpt_count++;
        }

        QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n章数：%2").arg(novel_name).arg(chpt_count));
    }

    emit signalImportFinished(novel_name);
}

QUrl NovelDirListView::getIndexFileTemp(QModelIndex index)
{
    NovelDirItem* item = novel_dir_model->getItem(index.row());
    QString file_name = novel_dir_delegate->getItemText(index);
    file_name = fnEncode(file_name);
    ensureDirExist(rt->DATA_PATH+"temp");
    if (item->isRoll()) // 是分卷，遍历里面的章节
    {
        QString dir_path = rt->DATA_PATH+"temp/" + file_name;
        if (isFileExist(dir_path))
            deleteFile(dir_path); // 同时删除里面的文件
        ensureDirExist(dir_path);

        // 获取这一分卷的信息
        QString chapter_dir = rt->NOVEL_PATH+novel_name+"/chapters"; // 章节所在的文件夹
        int row = index.row();
        int count = index.data(Qt::UserRole+DRole_ROLL_SUBS).toInt();

        // 遍历导出后面的章节
        for (int i = 1; i <= count; i++)
        {
            QModelIndex chapter_index = novel_dir_model->index(row+i);
            QString full_name = novel_dir_delegate->getItemText(chapter_index);
            QString chapter_name = chapter_index.data(Qt::UserRole+DRole_CHP_NAME).toString();
            QString chapter_path = rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(chapter_name)+".txt";
            QString chapter_content = readTextFile(chapter_path);
            QString file_path = dir_path+"/"+fnEncode(full_name)+".txt";
            chapter_content = NovelTools::getMainBodyWithoutMark(chapter_content);
            writeTextFile(file_path, chapter_content);
        }
        drag_file_path = dir_path;
    }
    else // 是章节，直接导出成一个txt
    {
        QString file_path = rt->DATA_PATH+"temp/" + file_name + ".txt";
        QString chapter_path = rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(item->getName())+".txt";
        QString chapter_content = readTextFile(chapter_path);
        chapter_content = NovelTools::getMainBodyWithoutMark(chapter_content);
        writeTextFile(file_path, chapter_content);
        drag_file_path = file_path;
    }

    return QUrl("file:///"+drag_file_path);
}

QString NovelDirListView::getSuitableChapterName(int index, bool move_to_end/*是否移动到分卷的最后*/)
{
    QString full_Text = getModel()->getFullText();
    QString pre1_chapter = "", pre2_chapter = "";

    if (move_to_end)
    {
        int count = novel_dir_model->getRcCount();
        while (++index < count)
        {
            NovelDirItem* item = novel_dir_model->getItem(index);
            if (item->isRoll())
                break;
        }
        if (index >= 1) // 前一章标题
        {
            NovelDirItem* pre1 = novel_dir_model->getItem(index-1);
            if (!pre1->isRoll())
            {
                pre1_chapter = pre1->getName();

                if (index >= 2) // 前两章标题
                {
                    NovelDirItem* pre2 = novel_dir_model->getItem(index-2);
                    if (!pre2->isRoll())
                        pre2_chapter = pre2->getName();
                }
            }
        }
    }

    QString suitable_name = NovelTools::getNewChapterAI(full_Text, pre1_chapter, pre2_chapter);

    return suitable_name;
}

bool NovelDirListView::processDropUrl(QString str)
{
    if (novel_name.isEmpty()) return false;
    int aim_index = indexAt(mapFromGlobal(QCursor::pos())).row(); // 如果是超过内容的最末尾，则是 -1
    QModelIndex model_index = model()->index(aim_index, 0);
    NovelDirItem* item = model_index.isValid() ? novel_dir_model->getItem(aim_index) : nullptr;
    extern_point = mapFromGlobal(QCursor::pos());

    if (!str.startsWith("file:///"))
        return false;

    QString path = str.right(str.length() - 8);
    if (!isFileExist(path))
        return false;

    if (QFileInfo(path).isDir()) // 目录，当做分卷遍历插入
    {
        QDir dir(path);
        QFileInfoList info_list = dir.entryInfoList(QDir::Files | QDir::NoDot | QDir::NoDotDot | QDir::NoSymLinks);
        QString roll_name = dir.dirName();

        _flag_no_rename_by_added = true;
        setCurrentIndex(model_index);
        QModelIndex roll_index = currentIndex();
        if (model_index.isValid() && item->isRoll() && aim_index > 0) // 插入到上一个目录，并且不是《作品相关》
        {
            if (item->getName() != dir.dirName()) // 如果名字相同，则认为是同一卷，只导入章节
                slotInsertRoll(roll_name);
            roll_index = currentIndex();
        }
        else
        {
            slotAddRoll(roll_name);
        }

        // 如果包含 txt 文件，则只导入 txt 文件
        // 否则导入所有不管类型的文件
        bool contains_txt = false;
        foreach (QFileInfo info, info_list)
        {
            if (info.suffix() == "txt")
            {
                contains_txt = true;
                break;
            }
        }

        // 遍历添加章节到新目录末尾
        foreach (QFileInfo info, info_list)
        {
            if ((!contains_txt || info.suffix() == "txt") && info.size() <= CHAPTER_FILEINFO_SIZE) // 小于 30KB 的txt文件
                slotAddChapter(info.baseName(), readTextFile(info.filePath()));
        }
        _flag_no_rename_by_added = false;

        setCurrentIndex(roll_index); // 聚焦到目录

        return true;
    }
    else if (QFile(path).size() <= CHAPTER_FILEINFO_SIZE) // 30KB以内的文件（假定为 txt）
    {
        QString title = QFileInfo(path).baseName(); // 去掉后缀名的
        QString content = readTextFile(path);
        if (model_index.isValid()) // 插入到中间
        {
            if (item->isRoll()) // 插入目录到全文最后（添加新章）
            {
                setCurrentIndex(novel_dir_model->index(novel_dir_model->getRcCount() - 1)); // 设置到最后一章/卷
                slotAddChapter(title, content);
            }
            else // 添加到目标位置（插入新章）
            {
                setCurrentIndex(model_index);
                slotInsertChapter(title, content);
            }
        }
        else // 插入到最后
        {
            setCurrentIndex(novel_dir_model->index(novel_dir_model->getRcCount() - 1)); // 设置到最后一章/卷
            slotAddChapter(title, content);
        }

        return true;
    }
    else if (QFile(path).size() <= NOVEL_FILEINFO_SIZE) // 10MB 以内 全文
    {
        us->setVal("import/target_path", path);
        actionImport();
    }
    return false;
}

bool NovelDirListView::processDropText(QString str)
{
    if (novel_name.isEmpty()) return false;
    int aim_index = indexAt(mapFromGlobal(QCursor::pos())).row(); // 如果是超过内容的最末尾，则是 -1
    QModelIndex model_index = model()->index(aim_index, 0);
    NovelDirItem* item = model_index.isValid() ? novel_dir_model->getItem(aim_index) : nullptr;
    extern_point = mapFromGlobal(QCursor::pos());

    QString title = "", content = "";
    if (str.length() > 10 || str.contains("\n") || str.contains("\r")) // 当做正文
    {
        content = str;
    }
    else // 当做标题
    {
        title = str;
    }

    if (model_index.isValid())
    {
        if (item->isRoll()) // 添加到卷末
        {
            setCurrentIndex(model_index);
            slotAddChapter(title, str);
        }
        else // 插入到章节
        {
            setCurrentIndex(model_index);
            slotInsertChapter(title, str);
        }
    }
    else // 插入到最后
    {
        setCurrentIndex(novel_dir_model->index(novel_dir_model->getRcCount()-1)); // 设置到最后一章/卷
        slotAddChapter(title, content);
    }

    return true;
}

bool NovelDirListView::processDropUrls(QStringList /*list*/)
{
    return false;
}

QString NovelDirListView::getChapterPath(QString name)
{
    if (name.isEmpty()) return "";
    return rt->NOVEL_PATH + novel_name + "/chapters/" + fnEncode(name) + ".txt";
}

void NovelDirListView::addSmoothScrollThread(int distance, int duration)
{
    SmoothScrollBean* bean = new SmoothScrollBean(distance, duration);
    smooth_scrolls.append(bean);
    connect(bean, SIGNAL(signalSmoothScrollDistance(SmoothScrollBean*, int)), this, SLOT(slotSmoothScrollDistance(SmoothScrollBean*, int)));
    connect(bean, &SmoothScrollBean::signalSmoothScrollFinished, [=]{
        smooth_scrolls.removeOne(bean);
    });
}

void NovelDirListView::slotSmoothScrollDistance(SmoothScrollBean *bean, int dis)
{
    int slide = verticalScrollBar()->sliderPosition();
    slide += dis;
    if (slide < 0)
    {
        slide = 0;
        smooth_scrolls.removeOne(bean);
    }
    else if (slide > verticalScrollBar()->maximum())
    {
        slide = verticalScrollBar()->maximum();
        smooth_scrolls.removeOne(bean);
    }
    verticalScrollBar()->setSliderPosition(slide);
}

void NovelDirListView::setCurrentIndex(const QModelIndex &index)
{
    /*  设置 current_item  */
    if (current_item != nullptr)
        ;//current_item->setSelecting(false);
    current_item = novel_dir_model->getItem(index.row());
    return QListView::setCurrentIndex(index);
}

void NovelDirListView::slotDirSettings()
{
    DirSettingsWidget* dir_settings_widget = new DirSettingsWidget(this);
    dir_settings_widget->setNS(novel_dir_delegate);
    dir_settings_widget->setModal(true);
    dir_settings_widget->exec(); // 模态，运行结束后再进行读取
    // dir_settings_widget->show();
    novel_dir_delegate->readSettings(novel_name);
    update();
}

void NovelDirListView::initMenu()
{
    menu = new QMenu(this);
    insert_action = new QAction(thm->icon("menu/insert"), "插入", this);
    delete_action = new QAction(thm->icon("menu/delete"), "删除", this);
    rename_action = new QAction(thm->icon("menu/rename"), "重命名", this);
    word_count_action = new QAction(thm->icon("menu/word_count"), "字数统计", this);
    book_search_action = new QAction(thm->icon("menu/search"), "全书查找", this);
    book_replace_action = new QAction(thm->icon("menu/replace"), "全书替换", this);
    recycle_action = new QAction(thm->icon("menu/recycle"), "回收站", this);

	directory_menu = new QMenu("目录", this);
	directory_menu->setIcon(thm->icon("menu/directory"));
	roll_split_action = new QAction(thm->icon("menu/roll_split"), "从此处分割到新卷", this);
	roll_cancel_action = new QAction(thm->icon("menu/roll_cancel"), "整卷合并到上一卷", this);

    im_ex_menu = new QMenu("导入导出", this);
    im_ex_menu->setIcon(thm->icon("menu/import_export"));
    immediate_export_action = new QAction(thm->icon("menu/export2"), "单章导出", this);
    export_action = new QAction(thm->icon("menu/export"), "全书导出", this);
    import_action = new QAction(thm->icon("menu/import"), "全书导入", this);
    import_mzfy_action = new QAction(thm->icon("mzfy"), "从码字风云导入", this);

    settings_menu = new QMenu("设置", this);
    settings_menu->setIcon(thm->icon("settings"));
    dir_settings_action = new QAction(thm->icon("settings"), tr("目录设置"), this);
    show_word_count_action = new QAction(thm->icon("menu/word_count"), "目录右侧字数", this);
    chapter_preview_action = new QAction(thm->icon("menu/preview"), "鼠标悬浮预览", this);

    show_word_count_action->setCheckable(true);
    chapter_preview_action->setCheckable(true);

    book_search_action->setEnabled(false);
    book_replace_action->setEnabled(false);
    recycle_action->setEnabled(false);

    menu->addAction(insert_action);
    menu->addAction(delete_action);
    menu->addAction(rename_action);
	menu->addMenu(directory_menu);
	{
        directory_menu->addAction(roll_split_action);
        directory_menu->addAction(roll_cancel_action);
	}
    menu->addSeparator();
    menu->addAction(word_count_action);
    menu->addAction(book_search_action);
    menu->addAction(book_replace_action);
    menu->addSeparator();
    menu->addMenu(settings_menu);
    {
        settings_menu->addAction(dir_settings_action);
        settings_menu->addSeparator();
        settings_menu->addAction(show_word_count_action);
        settings_menu->addAction(chapter_preview_action);
    }
    menu->addMenu(im_ex_menu);
    {
        im_ex_menu->addAction(immediate_export_action);
        im_ex_menu->addSeparator();
        im_ex_menu->addAction(export_action);
        im_ex_menu->addAction(import_action);
        im_ex_menu->addAction(import_mzfy_action);
    }
    menu->addSeparator();
    menu->addAction(recycle_action);

    connect(insert_action, SIGNAL(triggered()), this, SLOT(actionInsert()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(actionDelete()));
    connect(rename_action, SIGNAL(triggered()), this, SLOT(actionRename()));
    connect(word_count_action, SIGNAL(triggered()), this, SLOT(actionWordCount()));
    connect(roll_split_action, SIGNAL(triggered()), this, SLOT(actionRollSplit()));
    connect(roll_cancel_action, SIGNAL(triggered()), this, SLOT(actionRollCancel()));
    connect(dir_settings_action, SIGNAL(triggered()), this, SLOT(slotDirSettings()));
    connect(show_word_count_action, SIGNAL(triggered()), this, SLOT(actionShowWordCount()));
    connect(chapter_preview_action, SIGNAL(triggered()), this, SLOT(actionChapterPreview()));
    connect(immediate_export_action, SIGNAL(triggered()), this, SLOT(actionImmediateExport()));
    connect(export_action, SIGNAL(triggered()), this, SLOT(actionExport()));
    connect(import_action, SIGNAL(triggered()), this, SLOT(actionImport()));
    connect(import_mzfy_action, SIGNAL(triggered()), this, SLOT(actionImportMzfy()));
}

void NovelDirListView::refreshMenu()
{
    QModelIndex index = currentIndex();
    // ==== 分开判断 ====
    if (!index.isValid()) // 空白
    {
        insert_action->setEnabled(false);
        delete_action->setEnabled(false);
        rename_action->setEnabled(false);
        word_count_action->setEnabled(false);

        insert_action->setText("添加");
        delete_action->setText("删除");
    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        insert_action->setEnabled(true);
        delete_action->setEnabled(true);
        rename_action->setEnabled(true);
        word_count_action->setEnabled(true);

        insert_action->setText("插入分卷");
        delete_action->setText("删除分卷");
        word_count_action->setText("本卷字数");
        immediate_export_action->setText("单卷立即导出");
    }
    else // 章节
    {
        insert_action->setEnabled(true);
        delete_action->setEnabled(true);
        rename_action->setEnabled(true);
        word_count_action->setEnabled(true);

        insert_action->setText("插入章节");
        delete_action->setText("删除章节");
        word_count_action->setText("本章字数");
        immediate_export_action->setText("单章立即导出");
    }

    // ==== 章节特有 ====
	if (index.isValid() && !index.data(Qt::UserRole + DRole_CHP_ISROLL).toBool()) // 是章节
	{
		roll_split_action->setEnabled(true);
	}
	else // 不是章节
	{
		roll_split_action->setEnabled(false);
	}

    // ==== 分卷特有 ====
    if (index.isValid() && index.data(Qt::UserRole + DRole_CHP_ISROLL).toBool() && index.row() > 0) // 是普通的分卷
    {
        roll_cancel_action->setEnabled(true);
    }
    else // 不是分卷
    {
        roll_cancel_action->setEnabled(false);
    }

    chapter_preview_action->setChecked(us->chapter_preview);
    show_word_count_action->setChecked(novel_dir_delegate->isShowWordCount());
    show_word_count_action->setText(QString(novel_dir_delegate->isShowWordCount()?"隐藏每章字数":"显示每章字数(临时)"));
    chapter_preview_action->setText(QString(us->chapter_preview?"关闭":"开启")+"章节悬浮预览");
}

void NovelDirListView::actionInsert()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) // 空白
    {

    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        slotInsertRoll();
    }
    else // 章节
    {
        slotInsertChapter();
    }
}

void NovelDirListView::actionDelete()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) // 空白
    {

    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        slotDeleteRoll();
    }
    else // 章节
    {
        slotDeleteChapter();
    }
}

void NovelDirListView::actionRename()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) // 空白
    {

    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        slotRenameRoll();
    }
    else // 章节
    {
        slotRenameChapter();
    }
}

void NovelDirListView::actionWordCount()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) // 空白
    {

    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        slotRollWordCount();
    }
    else // 章节
    {
        slotChapterWordCount();
    }
}

void NovelDirListView::actionBookSearch()
{

}

void NovelDirListView::actionBookReplace()
{

}

void NovelDirListView::actionReycle()
{

}

void NovelDirListView::actionRollSplit()
{
	if (novel_name.isEmpty()) return;
	if (!currentIndex().isValid()) return;
    int index = currentIndex().row();
    NovelDirItem* item = novel_dir_model->getItem(index);
    if (item->isRoll()) return;

    QString new_name = "新卷";
    if (novel_dir_model->isExistRoll(new_name))
    {
       int i = 1;
       while (novel_dir_model->isExistRoll(new_name+QString::number(i)))
           i++;
       new_name = new_name+QString::number(i);
    }

    int add_pos = novel_dir_model->addSplitRoll(index, new_name);
    editItem(add_pos);
}

void NovelDirListView::actionRollCancel()
{
	if (novel_name.isEmpty()) return;
    if (!currentIndex().isValid()) return;
    int index = currentIndex().row();
    novel_dir_model->cancelRoll(currentIndex().row());
    if (index < novel_dir_model->getRcCount())
        setCurrentIndex(novel_dir_model->index(index));
}

void NovelDirListView::actionImmediateExport()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) // 空白
    {
		slotExport();
    }
    else if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 分卷
    {
        QUrl url = getIndexFileTemp(index); // 文件夹路径
        QDesktopServices::openUrl(url);
    }
    else // 章节
    {
        QUrl url = getIndexFileTemp(index); // 文件夹路径
        QProcess::execute("explorer /select, \""+url.toString()+"\"");
    }
}

void NovelDirListView::actionExport()
{
    emit signalImExPort(1, novel_name);
}

void NovelDirListView::actionImport()
{
    emit signalImExPort(2, novel_name);
}

void NovelDirListView::actionImportMzfy()
{
    emit signalImExPort(3, novel_name);
}

void NovelDirListView::actionShowWordCount()
{
    novel_dir_delegate->setShowWordCount(!novel_dir_delegate->isShowWordCount());
    update();
}

void NovelDirListView::actionChapterPreview()
{
//    novel_dir_delegate->setChapterPreview(!novel_dir_delegate->isChapterPreview());
//    novel_dir_delegate->setVal("chapter_preview", novel_dir_delegate->isChapterPreview());
    us->chapter_preview = !us->chapter_preview;
    us->setVal("us/chapter_preview", us->chapter_preview);
}
