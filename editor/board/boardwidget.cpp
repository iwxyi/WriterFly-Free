#include "boardwidget.h"

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    this->setFocus();

    initView();

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(slotFocusChanged(QWidget*, QWidget*)));
}

void BoardWidget::enableAutoDelete()
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void BoardWidget::initView()
{
    this->setMinimumSize(100, 161);

    tab_widget = new AniTabWidget(this);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(tab_widget);
    this->setLayout(layout);

    layout->setMargin(0);
    layout->setSpacing(0);

//    tab_widget->disableAnimation();

    tab_widget->addTab(new BoardPage(gd->boards.getClip(), this), "剪贴板");
    tab_widget->addTab(new BoardPage(gd->boards.getRecycle(), this), "回收板");
    tab_widget->addTab(new BoardPage(gd->boards.getCollection(), this), "收藏板");

    for (int i = 0; i < tab_widget->count(); i++)
    {
        QListWidget* lw = static_cast<QListWidget*>(tab_widget->widget(i));
        connect(lw, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotBoardItemClicked(QListWidgetItem*)));
    }

    // tab_widget->setTabPosition(QTabWidget::South);
    tab_widget->setTabInBottom();

    int index = us->getInt("recent/board", 0);
    if (index >= 0 && index < tab_widget->count())
    {
        // 第一次是切换到0，第二次需要阻止动画一次（不仅没必要，而且因为要更改大小，导致动画的Label和实际上的不一致）
        if (index>0)
            tab_widget->preventSwitchAnimationOnce();
        tab_widget->setCurrentIndex(index);
    }

    // 要在上面切换完了才接收信号，不然会设置成第一个0，即使原先不是
    connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

    // 默认聚焦当前 index 的第一个 item
    QListWidget* lw = static_cast<QListWidget*>(tab_widget->currentWidget());
    if (lw->count() > 0)
        lw->setCurrentRow(0);
}

void BoardWidget::toHide()
{
    QPixmap pixmap(this->size());
    this->render(&pixmap);
    QRect rect(parentWidget()->mapFromGlobal(this->pos()), this->size());
    emit signalHidden(pixmap, rect);

    QTimer::singleShot(1, [=]{  // 避免隐藏动画前的闪烁问题
        this->hide();
    });

}

void BoardWidget::slotTabChanged(int index)
{
    us->setVal("recent/board", index);
}

void BoardWidget::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape || key == Qt::Key_Close)
        return toHide();

    int page = tab_widget->currentIndex();
    QListWidget* lw = static_cast<QListWidget*>(tab_widget->currentWidget());
    int row = lw->currentRow();

    if (key == Qt::Key_Up)
    {
        if (row <= 0)
            row = lw->count()-1;
        else
            row--;
        if (row >= 0 && row <= lw->count()-1)
            lw->setCurrentRow(row);
    }
    else if (key == Qt::Key_Down)
    {
        if (row >= lw->count()-1)
            row = 0;
        else
            row++;
        if (row >= 0 && row <= lw->count()-1)
            lw->setCurrentRow(row);
    }
    else if (key == Qt::Key_Left || key == Qt::Key_PageUp)
    {
        if (page <= 0)
            page = tab_widget->count()-1;
        else
            page--;
        tab_widget->setCurrentIndex(page);
    }
    else if (key == Qt::Key_Right || key == Qt::Key_PageDown)
    {
        if (page >= tab_widget->count()-1)
            page = 0;
        else
            page++;
        tab_widget->setCurrentIndex(page);
    }
    else if (key == Qt::Key_Home && tab_widget->currentIndex() != 0)
    {
        if (tab_widget->count() > 0)
            tab_widget->setCurrentIndex(0);
    }
    else if (key == Qt::Key_End && tab_widget->currentIndex() != tab_widget->count()-1)
    {
        if (tab_widget->count() > 0)
            tab_widget->setCurrentIndex(tab_widget->count()-1);
    }
    else if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        if (lw->currentRow() >= 0)
        {
            QString str = lw->currentItem()->text();
            if (!str.isEmpty())
            {
                emit signalInsertText(str);
                this->hide();
            }
        }
    }

    return QWidget::keyPressEvent(event);
}

void BoardWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(QRect(0, 0,width(),height()), us->mainwin_sidebar_color);
    return QWidget::paintEvent(event);
}

void BoardWidget::slotBoardItemClicked(QListWidgetItem *item)
{
    if (item->toolTip().isEmpty())
        emit signalInsertText(item->text());
    else
        emit signalInsertText(item->toolTip());
    this->hide();
}

void BoardWidget::slotFocusChanged(QWidget */*old*/, QWidget *now)
{
    if (this->isHidden()) return ;
    if (now != this && now != tab_widget)
    {
        for (int i = 0; i < tab_widget->count(); i++)
        {
            QListWidget* lw = static_cast<QListWidget*>(tab_widget->widget(i));
            if (now == lw)
                return ;
        }
        QPoint p = QCursor::pos();
        p = mapFromGlobal(p);
        if (p.x()>=0 && p.x()<=geometry().width() && p.y()>=0 && p.y()<=geometry().height())
            return ;
        toHide();
    }
}
