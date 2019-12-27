#include "boardpage.h"

BoardPage::BoardPage(BoardBase* data, QWidget* parent) : QListWidget(parent), data(data)
{
    initView();
    initData();
}

void BoardPage::initView()
{
    this->setWordWrap(true); // 默认为 true
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // this->setSpacing(us->mainwin_sidebar_spacing);
    setTextElideMode(Qt :: ElideNone);

    QString style = "QListWidget{background:transparent; border:none; alternate-background-color:"+us->getColorString(us->mainwin_bg_color)+";}";
    this->setStyleSheet(style);
    this->setAlternatingRowColors(true);


    QPalette pa = this->palette();
    // 设置字体颜色
    if (us->editor_font_color.alpha() > 0)
        pa.setColor(QPalette::Text, us->editor_font_color);
    // 设置背景颜色
    pa.setColor(QPalette::Base, us->editor_bg_color);
    pa.setColor(QPalette::Background, us->editor_bg_color);
    // 设置选中的背景
    if (us->editor_font_selection.alpha() > 0)
        pa.setColor(QPalette::HighlightedText, us->editor_font_selection);
    // 设置选中的文字颜色
    pa.setColor(QPalette::Highlight, us->editor_bg_selection);
    this->setPalette(pa);
}
void BoardPage::initData()
{
	QStringList ss = data->getRecentSting();

	for (int i = ss.count()-1; i >= 0; i--)
	{
        bool isFull = false;
        QString s = getShowedText(ss.at(i), &isFull);
        QListWidgetItem *item = new QListWidgetItem(s);
        if (!isFull)
            item->setToolTip(ss.at(i));
		this->addItem(item);
    }
}

QString BoardPage::getShowedText(QString text, bool* isFull)
{
    if (text.length() > 100)
    {
        *isFull = false;
        return text.left(100) + QString(" (共%1字)").arg(text.length());
    }
    else
    {
        *isFull = true;
        return text;
    }
}
