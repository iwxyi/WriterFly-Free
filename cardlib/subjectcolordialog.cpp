#include "subjectcolordialog.h"

SubjectColorDialog::SubjectColorDialog(QColor color, QWidget *parent) : ColorDialog(color, parent)
{
    initView();
    loadSubjectColors();
}

SubjectColorDialog::SubjectColorDialog(QColor color, bool fore, QWidget *parent) : ColorDialog(color, fore, parent)
{
    initView();
    loadSubjectColors();
}

SubjectColorDialog::SubjectColorDialog(QString key, QColor color, QWidget *parent) : ColorDialog(us->getInt("recent/color_series_"+key, 0), color, parent)
{
    initView();
//    connect(series_combo, &QComboBox::activated, [=](int x){ us->setVal("recent/color_series_"+key, x); });
    connect(this, &ColorDialog::signalSeriesLoaded, [=](int x){ us->setVal("recent/color_series_"+key, x); });
    loadSubjectColors();
}

SubjectColorDialog::SubjectColorDialog(QString key, QColor color, bool fore, QWidget *parent) : ColorDialog(us->getInt("recent/color_series_"+key, 0), color, fore, parent)
{
    initView();
//    connect(series_combo, &QComboBox::activated, [=](int x){ us->setVal("recent/color_series_"+key, x); });
    connect(this, &ColorDialog::signalSeriesLoaded, [=](int x){ us->setVal("recent/color_series_"+key, x); });
    loadSubjectColors();
}

QColor SubjectColorDialog::pick(QString title, int *ctype, QColor color, QWidget *parent)
{
    QColor result = color;
    SubjectColorDialog* cd = new SubjectColorDialog(color, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color){
        result = color;
    });
    return result;
}

QColor SubjectColorDialog::pick(QString title, int *ctype, QColor color, bool *ok, QWidget *parent)
{
    QColor result = color;
    *ok = false;
    SubjectColorDialog *cd = new SubjectColorDialog(color, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

QColor SubjectColorDialog::pickF(QString title, int *ctype, QColor color, QWidget *parent)
{
    QColor result = color;
    SubjectColorDialog* cd = new SubjectColorDialog(color, true, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color){
        result = color;
    });
    return result;
}

QColor SubjectColorDialog::pickF(QString title, int *ctype, QColor color, bool *ok, QWidget *parent)
{
    QColor result = color;
    *ok = false;
    SubjectColorDialog *cd = new SubjectColorDialog(color, true, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

QColor SubjectColorDialog::pick(QString key, QString title, int *ctype, QColor color, QWidget *parent)
{
    QColor result = color;
    SubjectColorDialog* cd = new SubjectColorDialog(key, color, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color){
        result = color;
    });
    return result;
}

QColor SubjectColorDialog::pick(QString key, QString title, int *ctype, QColor color, bool *ok, QWidget *parent)
{
    QColor result = color;
    *ok = false;
    SubjectColorDialog *cd = new SubjectColorDialog(key, color, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

QColor SubjectColorDialog::pickF(QString key, QString title, int *ctype, QColor color, QWidget *parent)
{
    QColor result = color;
    SubjectColorDialog* cd = new SubjectColorDialog(key, color, true, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color){
        result = color;
    });
    return result;
}

QColor SubjectColorDialog::pickF(QString key, QString title, int *ctype, QColor color, bool *ok, QWidget *parent)
{
    QColor result = color;
    *ok = false;
    SubjectColorDialog *cd = new SubjectColorDialog(key, color, true, parent);
    cd->setWindowTitle(title);
    cd->ctype = ctype;
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

void SubjectColorDialog::initView()
{
    MyLabel* subject_label = new MyLabel("主题色系 (?)", this);
    subject_list = new QListWidget(this);

    QString tooltip = "名片专用的跟随主题而变化的色彩系列（需要主题支持）\n拖动左边列表至色系，可修改色系颜色";
    subject_label->setToolTip(tooltip);
    connect(subject_label, &MyLabel::clicked, [=]{
        QMessageBox::information(this, "主题色系", tooltip);
    });

    QVBoxLayout* list_vlayout = new QVBoxLayout;
    list_vlayout->addWidget(subject_label);
    list_vlayout->addWidget(subject_list);
    body_hlayout->addLayout(list_vlayout);

    subject_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(subject_list, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowItemMenu(QPoint)));
    connect(subject_list, SIGNAL(currentRowChanged(int)), this, SLOT(slotListWidgetItemChanged(int)));
    connect(fore_btn, SIGNAL(clicked()), this, SLOT(loadSubjectColors()));
    connect(hex_line, SIGNAL(textChanged(const QString &)), this, SLOT(slotHexChanged(const QString&)));

    menu = new QMenu(this);
    QAction* refreshAction = new QAction("刷新", this);
    QAction* insertAction = new QAction("插入", this);
    QAction* appendAction = new QAction("添加", this);
    QAction* renameAction = new QAction("重命名", this);
    QAction* modifyAction = new QAction("修改颜色", this);
    QAction* deleteAction = new QAction("删除", this);
    menu->addAction(refreshAction);
    menu->addSeparator();
    menu->addAction(insertAction);
    menu->addAction(appendAction);
    menu->addAction(renameAction);
    menu->addAction(modifyAction);
    menu->addAction(deleteAction);
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(actionRefreshAllSeriesItems()));
    connect(insertAction, SIGNAL(triggered()), this, SLOT(actionInsertSeriesItem()));
    connect(appendAction, SIGNAL(triggered()), this, SLOT(actionAppendSeriesItem()));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(actionRenameSeriesItem()));
    connect(modifyAction, SIGNAL(triggered()), this, SLOT(actionModifySeriesItem()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(actionDeleteSeriesItem()));

    // 设置颜色对话框为本程序的对应颜色
    series_combo->setStyleSheet("QComboBox { background:" + us->getColorString(us->getNoOpacityColor(us->mainwin_bg_color)) + "; border: none; padding: 1px; }\
        QComboBox::down-arrow { background: transparent; image: url(:/icons/arrow_down_combo); }");
}

void SubjectColorDialog::loadSubjectColors()
{
    int slide = subject_list->verticalScrollBar()->sliderPosition(); // 保存滚动位置
    // 先清空列表widget，避免内存泄漏
    for (int i = 0; i < subject_list->count(); i++)
    {
        QListWidgetItem* item = subject_list->item(i);
        QWidget* w = subject_list->itemWidget(item);
        w->deleteLater();
    }

    subject_list->clear();
    QStringList names = gd->clm.colorSeries().getNames();
    QList<QColor>colors = gd->clm.colorSeries().getColors();
    for (int i = 0;  i < colors.size(); i++)
    {
        addOneColor(names.at(i), colors.at(i));
    }
    subject_list->verticalScrollBar()->setSliderPosition(slide); // 滚回到上次加载的地方
}

void SubjectColorDialog::slotListWidgetItemChanged(int row)
{
    if (_flag_ignore_row_changed_once)
    {
        _flag_ignore_row_changed_once = false;
        return ;
    }
    if (row < 0 || row >= gd->clm.colorSeries().getColors().size())
        return;
    _flag_ignore_hex_changed = true;
    slotColorPicked(gd->clm.colorSeries().getColors().at(row));
    _flag_ignore_hex_changed = false;
    if (ctype != nullptr)
        *ctype = subject_list->currentRow() + 1;
}

void SubjectColorDialog::slotListWidgetDoubleClicked(QListWidgetItem *item)
{
    subject_list->setCurrentItem(item);

    color = gd->clm.colorSeries().getColors().at(subject_list->currentRow());
    emit signalColorPicked(color);
    this->accept();
}

void SubjectColorDialog::addOneColor(QString name, QColor color)
{
    QListWidgetItem* item = new QListWidgetItem(subject_list);

    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(10, line_spacing*2));
    WaterZoomButton* btn = new WaterZoomButton(name, subject_list);
    subject_list->setItemWidget(item, btn);
    btn->setAlign(Qt::AlignLeft | Qt::AlignVCenter);
    btn->setRadius(5, 3);
    btn->setChoking(3);
    btn->setPaddings(10, 0, 0, 0);
    btn->setDoubleClicked(true);
    if (useForeground) // 前景色
    {
        btn->setBgColor(getReverseColor(color), QColor(0x88, 0x88, 0x88, 0x44));
        btn->setTextColor(color);
    }
    else
    {
        btn->setBgColor(color);
        btn->setTextColor(getReverseColor(color));
    }
    int x = subject_list->count()-1;
    connect(btn, &InteractiveButtonBase::clicked, [=]{
        subject_list->setCurrentRow(x);
    });
    connect(btn, &InteractiveButtonBase::doubleClicked, [=]{
        slotListWidgetDoubleClicked(subject_list->item(x));
    });
}

void SubjectColorDialog::slotShowItemMenu(QPoint p)
{
    if (subject_list->currentRow() == -1)
        return ;
    menu->exec(QCursor::pos());
}

void SubjectColorDialog::slotHexChanged(const QString &text)
{
    if (_flag_ignore_hex_changed)
    {
        return ;
    }



    /*_flag_ignore_row_changed_once = true;
    selectSimilarColor(color);
    _flag_ignore_row_changed_once = false;*/
}

bool SubjectColorDialog::selectSimilarColor(QColor color)
{
    ColorDialog::selectSimilarColor(color);

    int index = -1;
    int variance = 64*64*3; // 方差

    QList<QColor>colors = gd->clm.colorSeries().getColors();
    for (int i = 0; i < colors.size(); i++)
    {
        const QColor c = colors.at(i);
        int dr = color.red() - c.red();
        int dg = color.green() - c.green();
        int db = color.blue() - c.blue();
        int v = dr * dr + dg * dg + db * db;
        if (v < variance)
        {
            index = i;
            variance = v;
        }
    }

    if (index >= 0)
    {
        subject_list->setCurrentRow(index);
        subject_list->scrollToItem(color_list->currentItem(), QAbstractItemView::PositionAtCenter);
        showSelectAnimation(index);
        return true;
    }
    else
        return false;
}

void SubjectColorDialog::actionRefreshAllSeriesItems()
{
    gd->clm.refreshAllSeriesItems();
}

void SubjectColorDialog::actionInsertSeriesItem()
{
    int index = subject_list->currentRow();
    if (index == -1)
        return ;

    gd->clm.colorSeries().insertItem(index);

    loadSubjectColors();

    showSelectAnimation(index);
}

void SubjectColorDialog::actionAppendSeriesItem()
{
    int index = subject_list->currentRow();
    if (index == -1)
        return ;

    gd->clm.colorSeries().appendItem(index);

    loadSubjectColors();

    showSelectAnimation(index+1);
}

void SubjectColorDialog::actionRenameSeriesItem()
{
    int index = subject_list->currentRow();
    if (index == -1)
        return ;

    QString name = gd->clm.colorSeries().getColorName(index);
    bool ok;
    name = QInputDialog::getText(this, "修改主题色系项名字", "空则延续上一项名字", QLineEdit::Normal, name, &ok);
    if (!ok)
        return ;
    gd->clm.colorSeries().renameItem(index, name);

    loadSubjectColors();

    showSelectAnimation(index);
}

void SubjectColorDialog::actionModifySeriesItem()
{
    int index = subject_list->currentRow();
    if (index == -1)
        return ;

    QColor color = gd->clm.colorSeries().getColor(index);
    bool ok;
    int type = 0;
    if (useForeground)
        color = SubjectColorDialog::pickF("series_color", "修改主题色系项颜色", &type, color, &ok, this);
    else
        color = SubjectColorDialog::pick("series_color", "修改主题色系项颜色", &type, color, &ok, this);
    if (!ok)
        return ;
    gd->clm.colorSeries().modifyItem(index, color);

    loadSubjectColors();

    showSelectAnimation(index);
}

void SubjectColorDialog::actionDeleteSeriesItem()
{
    if (subject_list->currentRow() == -1)
        return ;

    gd->clm.colorSeries().deleteItem(subject_list->currentRow());

    loadSubjectColors();
}

void SubjectColorDialog::showSelectAnimation(int x)
{
    QWidget* w = subject_list->itemWidget(subject_list->item(x));
    WaterZoomButton* btn = static_cast<WaterZoomButton*>(w);
    btn->simulateStatePress();
}
