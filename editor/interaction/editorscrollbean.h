#ifndef EDITORSCROLLBEAN_H
#define EDITORSCROLLBEAN_H

#include <QTextEdit>
#include <QScrollBar>

/**
 * 记录了编辑框滚动位置的类
 */
struct EditorScrollBean {
    int pos;
    int sel;
    int slide;

    EditorScrollBean(int p, int s)
    {
        pos = p;
        slide = s;
        sel = 0;
    }

    void fromEditor(QTextEdit* te)
    {
        this->pos = te->textCursor().position();
        this->slide = te->verticalScrollBar()->sliderPosition();
    }

    void setEditor(QTextEdit* te)
    {
        QTextCursor tc = te->textCursor();
        tc.setPosition(this->pos);
        te->setTextCursor(tc);

        QScrollBar* sb = te->verticalScrollBar();
        if (sb != nullptr)
        {
            sb->setSliderPosition(this->slide);
        }
    }
};

#endif // EDITORSCROLLBEAN_H
