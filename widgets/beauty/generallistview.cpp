#include "generallistview.h"

GeneralListView::GeneralListView(QWidget *parent)
    : QListView(parent)
{
    updateUI();

    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
//    connect(thm, SIGNAL(editorChanged()), this, SLOT(updateUI()));
}

void GeneralListView::updateUI()
{
    QPalette pa(QApplication::palette());
    // 设置字体颜色
    if (us->editor_font_color.alpha() > 0)
        pa.setColor(QPalette::Text, us->editor_font_color);
    // 设置背景颜色
    pa.setColor(QPalette::Base, us->mainwin_bg_color);
    pa.setColor(QPalette::Background, us->mainwin_bg_color);
    // 设置选中的背景
    if (us->editor_font_selection.alpha() > 0)
        pa.setColor(QPalette::HighlightedText, us->accent_color);
    // 设置选中的文字颜色
    pa.setColor(QPalette::Highlight, us->global_font_color);
    this->setPalette(pa);

    // 上面的对词语弹窗背景无效（下拉列表框有用），所以只能用这个背景了
    QString style = "GeneralListView{background-color:"+us->getFixedOpacityColor(us->mainwin_bg_color, 250, 250)+"; color:"+us->getColorString(us->editor_font_color)+";\
            border:none; padding-left:4px; padding-right:4px;}";
    setStyleSheet(style);
    thm->setWidgetStyleSheet(verticalScrollBar(), "scrollbar");
}
