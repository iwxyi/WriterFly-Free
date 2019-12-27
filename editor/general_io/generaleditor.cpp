#include "generaleditor.h"

GeneralEditor::GeneralEditor(QWidget* parent)
    : ChapterEditor (parent), single_mode(false), compact_mode(false), normal_mode(false)
{
    initView();
}

GeneralEditor::GeneralEditor(QWidget *parent, EditType type)
    : ChapterEditor (type, parent), single_mode(false), compact_mode(false), normal_mode(false)
{
    initView();
}

GeneralEditor::GeneralEditor(QWidget* parent, QString path)
    : ChapterEditor (parent), single_mode(false), compact_mode(false), normal_mode(false)
{
    initView();
    readFromFile(path);
}

GeneralEditor::GeneralEditor(QWidget* parent, EditType type, QString path)
    : ChapterEditor (type, parent), single_mode(false), compact_mode(false), normal_mode(false)
{
    initView();
    readFromFile(path);
}

GeneralEditor::GeneralEditor(QWidget* parent, EditType type, QString path, bool sm, bool cm, bool nm)
    : ChapterEditor (type, parent), single_mode(false), compact_mode(false), normal_mode(false)
{
    initView();
    readFromFile(path);
    setModels(sm, cm, nm);
}

/**
 * 初始化界面（在父类初始化完成之后）
 */
void GeneralEditor::initView()
{
    blur_effect = false;

    updateUI();
}

void GeneralEditor::setModels(bool sm, bool cm, bool nm)
{
    this->single_mode = sm;
    this->compact_mode = cm;
    this->normal_mode = nm;
}

void GeneralEditor::setType(EditType type)
{
    editing.setType(type);
}

void GeneralEditor::keyPressEvent(QKeyEvent *event)
{

    return ChapterEditor::keyPressEvent(event);
}

void GeneralEditor::saveFontSize()
{
    if (editing.isOutline())
    {
        us->setVal("us/outline_font_size", this->font().pointSize());
    }
    else if (editing.isBrief())
    {
        us->setVal("us/brief_font_size", this->font().pointSize());
    }
    else if (editing.isDetail())
    {
        us->setVal("us/detail_font_size", this->font().pointSize());
    }
}

void GeneralEditor::updateUI()
{
    ChapterEditor::updateUI();

    QFont font(this->font());
    if (editing.isOutline())
        font.setPointSize(us->getInt("us/outline_font_size", font.pointSize()*0.8));
    else if (editing.isBrief())
        font.setPointSize(us->getInt("us/brief_font_size", font.pointSize()*0.5));
    else if (editing.isDetail())
        font.setPointSize(us->getInt("us/detail_font_size", font.pointSize()*0.5));
    this->setFont(font);

    // 因为字体大小不一样，所以重新设置光标动画的大小
    editor_cursor->styleChanged(cursorWidth(), fontMetrics().height(), us->getReverseColor(us->mainwin_bg_color, false)/*us->editor_font_color*/);
}
