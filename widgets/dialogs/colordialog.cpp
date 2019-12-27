#include "colordialog.h"

ColorDialog::ColorDialog(QColor def_color, QWidget *parent) : QDialog(parent), ctype(nullptr), color(def_color), useForeground(false)
{
	initView();
    initSeries();
    initColorTable();

    slotLoadColors(0);
    slotColorPicked(color, false);
    setHexText(color);
}

ColorDialog::ColorDialog(QColor def_color, bool fore, QWidget *parent) : QDialog(parent), ctype(nullptr), color(def_color), useForeground(fore)
{
    initView();
    initSeries();
    initColorTable();

    slotLoadColors(0);
    slotColorPicked(color, false);
    setHexText(color);
    fore_btn->setBgColor(QColor(0xa0,0xa0,0xa4,0x88));
}

ColorDialog::ColorDialog(int index, QColor def_color, QWidget *parent) : QDialog(parent), ctype(nullptr), color(def_color), useForeground(false)
{
    initView();
    initSeries();
    initColorTable();

    slotLoadColors(index);
    series_combo->setCurrentIndex(index);
    slotColorPicked(color, false);
    setHexText(color);
}

ColorDialog::ColorDialog(int index, QColor def_color, bool fore, QWidget *parent) : QDialog(parent), ctype(nullptr), color(def_color), useForeground(fore)
{
    initView();
    initSeries();
    initColorTable();

    slotLoadColors(index);
    series_combo->setCurrentIndex(index);
    slotColorPicked(color, false);
    setHexText(color);
    fore_btn->setBgColor(QColor(0xa0,0xa0,0xa4,0x88));
}

QColor ColorDialog::pick(QString title, QColor def, QWidget* parent)
{
    QColor result = def;
    ColorDialog* cd = new ColorDialog(def, parent);
    cd->setWindowTitle(title);
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color){
        result = color;
    });
    return result;
}

QColor ColorDialog::pick(QString title, QColor def, bool *ok, QWidget *parent)
{
    QColor result = def;
    *ok = false;
    ColorDialog *cd = new ColorDialog(def, parent);
    cd->setWindowTitle(title);
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

QColor ColorDialog::pickF(QString title, QColor def, QWidget *parent)
{
    QColor result = def;
    ColorDialog *cd = new ColorDialog(def, true, parent);
    cd->setWindowTitle(title);
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
    });
    return result;
}

QColor ColorDialog::pickF(QString title, QColor def, bool *ok, QWidget *parent)
{
    QColor result = def;
    *ok = false;
    ColorDialog *cd = new ColorDialog(def, true, parent);
    cd->setWindowTitle(title);
    connect(cd, &ColorDialog::signalColorPicked, [&](QColor color) {
        result = color;
        *ok = true;
    });
    if (cd->exec() != ColorDialog::Accepted)
        *ok = false;
    return result;
}

void ColorDialog::initView()
{
    series_combo = new QComboBox(this);
    color_list = new QListWidget(this);
    fore_btn = new InteractiveButtonBase(QIcon(":/icons/color"), this);
    hex_line = new QLineEdit(this);
    ok_btn = new InteractiveButtonBase("确定", this);
    cancel_btn = new InteractiveButtonBase("取消", this);

    color_list->setVerticalScrollMode(QListWidget::ScrollPerPixel);
//    color_list->verticalScrollBar()->setSingleStep(10);
    // color_list->setFocusPolicy(Qt::NoFocus);
    fore_btn->setSquareSize();
    QFontMetrics fm(font());
    line_spacing = fm.lineSpacing();
    hex_line->setMinimumWidth(fm.horizontalAdvance("#FFFFFF"));
    hex_line->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ok_btn->setPaddings(40, 0);
    cancel_btn->setPaddings(40, 0);
    ok_btn->setFixedForePos(true);
    cancel_btn->setFixedForePos(true);

    fore_btn->setToolTip("切换前景/背景");

    QVBoxLayout *main_vlayout = new QVBoxLayout;
    {
        body_hlayout = new QHBoxLayout;
        {
            QVBoxLayout* list_vlayout = new QVBoxLayout;
            list_vlayout->addWidget(series_combo);
            list_vlayout->addWidget(color_list);
            list_vlayout->setSpacing(0);
            list_vlayout->setMargin(0);
            body_hlayout->addLayout(list_vlayout);
        }
        main_vlayout->addLayout(body_hlayout);
    }
    {
        QHBoxLayout* color_hlayout = new QHBoxLayout;
        color_hlayout->addWidget(fore_btn);
        color_hlayout->addWidget(hex_line);
        main_vlayout->addLayout(color_hlayout);
        color_hlayout->setAlignment(Qt::AlignLeft);
    }
    {
        QHBoxLayout* btn_hlayout = new QHBoxLayout;
        btn_hlayout->addWidget(ok_btn);
        btn_hlayout->addWidget(cancel_btn);
        main_vlayout->addLayout(btn_hlayout);
        btn_hlayout->setAlignment(Qt::AlignRight);
    }
    setLayout(main_vlayout);

    connect(series_combo, SIGNAL(activated(int)), this, SLOT(slotLoadColors(int)));
    connect(color_list, SIGNAL(currentRowChanged(int)), this, SLOT(slotListWidgetItemChanged(int)));
    connect(color_list, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(slotListWidgetClicked(QListWidgetItem *)));
//    connect(color_list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotListWidgetDoubleClicked(QListWidgetItem *)));
    connect(fore_btn, SIGNAL(clicked()), this, SLOT(slotSwitchForeAndBg()));
    connect(hex_line, SIGNAL(textChanged(const QString &)), this, SLOT(slotHexChanged(const QString&)));
    connect(ok_btn, &QPushButton::clicked, this, [=] {
        emit signalColorPicked(color);
    });
    connect(ok_btn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_btn, SIGNAL(clicked()), this, SLOT(reject()));

    // series_combo->setStyleSheet("border: 1px solid #88888844; background: rgba(255,255,255,40%);");
    hex_line->setStyleSheet("background:transparent; border:1px solid rgba(88,88,88,30);");
    color_list->setStyleSheet("background:transparent; padding-left: 5px;");
}

void ColorDialog::initSeries()
{
    series_combo->addItems(color_series);
}

void ColorDialog::initColorTable()
{
    color_table_string = "";
    QFile file(":/database/color_series");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text) || !file.isReadable())
        return;
    QTextStream text_stream(&file);
    text_stream.setCodec("UTF-8");
    while (!text_stream.atEnd())
        color_table_string = text_stream.readAll();
    file.close();
}

void ColorDialog::slotLoadColors(int x)
{
    // 先清空列表widget，避免内存泄漏
    for (int i = 0; i < color_list->count(); i++)
    {
        QListWidgetItem* item = color_list->item(i);
        QWidget* w = color_list->itemWidget(item);
        w->deleteLater();
    }

    colors.clear();
	color_list->clear();
    if (x < 0 || x >= color_series.size())
        x = 0;
    loadColorSeries(color_series.at(x));
    emit signalSeriesLoaded(x);
}

void ColorDialog::slotSwitchForeAndBg()
{
    useForeground = !useForeground;
    slotLoadColors(series_combo->currentIndex());
    if (useForeground)
        fore_btn->setBgColor(QColor(0xa0,0xa0,0xa4,0x88));
    else
        fore_btn->setBgColor(Qt::transparent);
}

void ColorDialog::loadColorSeries(QString series)
{
	// 设获取标题位置
    QString anchor = "【" + series + "】";
    int left_pos = color_table_string.indexOf(anchor);
    if (left_pos == -1)
        left_pos = 0;
    left_pos += anchor.length();
    int right_pos = color_table_string.indexOf("【", left_pos);
    if (right_pos == -1)
        right_pos = color_table_string.length();
    // 对应颜色的正文
    QString table = color_table_string.mid(left_pos, right_pos - left_pos).trimmed();
    QStringList table_list = table.split("\n");
    // 遍历每一个颜色
    foreach (const QString &color_table, table_list)
    {
        QStringList list = color_table.split("\t");
        if (list.size() < 4)
            continue;
        addOneColor(list.at(0), QColor(list.at(1).toInt(), list.at(2).toInt(), list.at(3).toInt()));
    }

    // 选中最相近的颜色
    _flag_ignore_row_changed_once = true;
    if (!selectSimilarColor(color))
    {
        if (color_list->count() > 0)
            color_list->setCurrentRow(0);
    }
    _flag_ignore_row_changed_once = false;
}

void ColorDialog::addOneColor(QString name, QColor color)
{
    colors.append(color);
    QListWidgetItem* item = new QListWidgetItem(color_list);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(10, line_spacing*2));
    WaterZoomButton* btn = new WaterZoomButton(name, color_list);
    color_list->setItemWidget(item, btn);
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
    int x = color_list->count()-1;
    connect(btn, &InteractiveButtonBase::clicked, [=]{
        color_list->setCurrentRow(x);
    });
    connect(btn, &InteractiveButtonBase::doubleClicked, [=]{
        slotListWidgetDoubleClicked(color_list->item(x));
    });

    /*if (useForeground) // 前景色
    {
        item->setBackground(Qt::transparent);
        item->setForeground(color);
    }
    else
    {
        item->setBackground(QBrush(color));
        item->setForeground(QBrush(getReverseColor(color)));
    }*/
}

bool ColorDialog::selectSimilarColor(QColor color)
{
    int index = -1;
    int variance = 64*64*3; // 方差

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
        color_list->setCurrentRow(index);
        color_list->scrollToItem(color_list->currentItem(), QAbstractItemView::PositionAtCenter);
        showSelectAnimation(index);
        return true;
    }
    else
        return false;
}

void ColorDialog::slotListWidgetItemChanged(int row)
{
    if (_flag_ignore_row_changed_once)
    {
        _flag_ignore_row_changed_once = false;
        return ;
    }
    if (row < 0 || row >= colors.size())
        return;
    _flag_ignore_hex_changed = true;
    slotColorPicked(colors.at(row));
    _flag_ignore_hex_changed = false;

    if (ctype != nullptr)
        *ctype = 0;
}

void ColorDialog::slotListWidgetClicked(QListWidgetItem *item)
{
	// 已转移至 item changed 事件
}

void ColorDialog::slotListWidgetDoubleClicked(QListWidgetItem *item)
{
    color_list->setCurrentItem(item);

    color = colors.at(color_list->currentRow());
    emit signalColorPicked(color);
    this->accept();
}

void ColorDialog::slotColorPicked(QColor color, bool modifyHex)
{
	this->color = color;
    if (useForeground) // 前景色
    {
        ok_btn->setTextColor(color);
        ok_btn->setBgColor(Qt::transparent);
    }
    else
    {
        ok_btn->setTextColor(getReverseColor(color));
        ok_btn->setBgColor(color);
    }

    if (modifyHex)
        setHexText(color);
}

void ColorDialog::setHexText(QColor color)
{
    _flag_ignore_hex_changed_once = true;

    hex_line->setText(QString("#%1%2%3")
        .arg((color.red()), 2, 16, QLatin1Char('0'))
        .arg((color.green()), 2, 16, QLatin1Char('0'))
        .arg((color.blue()), 2, 16, QLatin1Char('0')).toUpper()
    );

    _flag_ignore_hex_changed_once = false;
}

void ColorDialog::slotHexChanged(const QString& _text)
{
    if (_flag_ignore_hex_changed_once)
    {
        _flag_ignore_hex_changed_once = false;
        return ;
    }
    if (_flag_ignore_hex_changed)
        return ;

    // 十六进制转颜色，并选中对应(最近)的颜色
    QColor new_color(color);
    QString text = _text.toLower();
    // #ABABAB
    if (QRegExp("^\\s*#?[0-9a-f]{6}\\s*$").exactMatch(text))
    {
        QRegExp rx("^\\s*#?([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})\\s*$");
        int pos = rx.indexIn(text);
        if (pos == -1) return ;
        QStringList hexs = rx.capturedTexts();
        bool ok;
        new_color.setRed(hexs.at(1).toInt(&ok, 16));
        new_color.setGreen(hexs.at(2).toInt(&ok, 16));
        new_color.setBlue(hexs.at(3).toInt(&ok, 16));
        color = new_color;
    }
    // #FFF
    else if (QRegExp("^\\s*#?[0-9a-z]{3}\\s*$").exactMatch(text))
    {
        QRegExp rx("^\\s*#?([0-9a-f])([0-9a-f])([0-9a-f])\\s*$");
        int pos = rx.indexIn(text);
        if (pos == -1) return;
        QStringList hexs = rx.capturedTexts();
        bool ok;
        new_color.setRed((hexs.at(1)+hexs.at(1)).toInt(&ok, 16));
        new_color.setGreen((hexs.at(2)+hexs.at(2)).toInt(&ok, 16));
        new_color.setBlue((hexs.at(3)+hexs.at(3)).toInt(&ok, 16));
        color = new_color;
    }
    // 12 23 34
    else if (QRegExp("^\\D*\\d{1,3}\\D+\\d{1,3}\\D+\\d{1,3}\\D*$").exactMatch(text))
    {
        QRegExp rx("^\\D*(\\d{1,3})\\D+(\\d{1,3})\\D+(\\d{1,3})\\D*$");
        int pos = rx.indexIn(text);
        if (pos == -1) return;
        QStringList hexs = rx.capturedTexts();
        bool ok;
        new_color.setRed(hexs.at(1).toInt(&ok));
        new_color.setGreen(hexs.at(2).toInt(&ok));
        new_color.setBlue(hexs.at(3).toInt(&ok));
        color = new_color;
    }
    else // 不支持的格式
        return;

    slotColorPicked(color, false);
    _flag_ignore_row_changed_once = true;
    selectSimilarColor(color);
    _flag_ignore_row_changed_once = false;

    if (ctype != nullptr)
        *ctype = 0;
}

/**
 * 由于隐藏了选中项的列表，所以需要通过动画高亮显示选中项
 * @param x 下标
 */
void ColorDialog::showSelectAnimation(int x)
{
    QWidget* w = color_list->itemWidget(color_list->item(x));
    WaterZoomButton* btn = static_cast<WaterZoomButton*>(w);
    btn->simulateStatePress();
}

/**
 * 背景颜色和前景颜色混在一起
 * 获取一个比较显眼的颜色
 */
QColor ColorDialog::getReverseColor(QColor color)
{
    return QColor(
            getReverseChannel(color.red()),
            getReverseChannel(color.green()),
            getReverseChannel(color.blue())
        );
}

int ColorDialog::getReverseChannel(int x)
{
    if (x < 92 || x > 159)
        return 255 - x;
    else if (x < 128)
        return 255;
    else // if (x > 128)
        return 0;
}

void ColorDialog::resizeEvent(QResizeEvent * event)
{
    // 自适应多列(然而无效)
    QFontMetrics fm(font());
    int w = fm.horizontalAdvance("國際奇連藍 International Klein Blue");
    int col = width() / w;
    if (col <= 0)
        col = 1;
    color_list->setModelColumn(col);

    return QDialog::resizeEvent(event);
}
