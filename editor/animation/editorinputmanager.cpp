#include "editorinputmanager.h"

EditorInputManager::EditorInputManager()
{

}

void EditorInputManager::setEditor(QTextEdit *edit)
{
    _edit = edit;
}

void EditorInputManager::setFlag(bool *flag)
{
    _flag_is_cursor_change = flag;
}

void EditorInputManager::setColor(QColor color)
{
    this->font_color = color;
}

void EditorInputManager::textChanged(int old_position, int diff)
{
    for (int i = 0; i < ani_list.count(); i++)
    {
        EditorInputCharactor*& charactor = ani_list[i];
        int pos = charactor->getPosition();
        if (diff > 0) // 输入
        {
            if (old_position <= pos) // 在前面输入
            {
                charactor->changePosition(diff);
            }
        }
        else if (diff < 0) // 删除字符
        {
            if (old_position > pos && old_position+diff <= pos) // 删除的部分有这个字；diff是负的，所以用加号
            {
                ani_list.removeAt(i);
                i--;
                //delete charactor; // 不能在这里 delete，不然会导致闪退（虽然应该是这里没错……）
                charactor->hide();
            }
            else if (old_position <= pos)
            {
                charactor->changePosition(diff);
            }
        }
    }
}

void EditorInputManager::updateRect(int range_start, int range_end)
{
    if (range_start == -1 && range_end == -1) return ;
    QTextCursor cursor = _edit->textCursor();
    for (int i = 0; i < ani_list.count(); i++)
    {
        EditorInputCharactor*& charactor = ani_list[i];
        int pos = charactor->getPosition();
        if (pos >= range_start && pos < range_end)
        {
            cursor.setPosition(pos);
            charactor->updateRect(_edit->cursorRect(cursor).topLeft());
        }
    }
}

void EditorInputManager::addInputAnimation(QPoint point, QString str, int position, int delay, int duration)
{
    addInputAnimation(point, str, position, delay, duration, font_color);
}

void EditorInputManager::addInputAnimation(QPoint point, QString str, int position, int delay, int duration, QColor color)
{
    EditorInputCharactor* charactor = new EditorInputCharactor(_edit, point, str, color, position, _edit->font(), delay, duration);
    ani_list.append(charactor);

    QTextCursor cursor = _edit->textCursor();
    QTextCharFormat f = cursor.charFormat();
    QTextCharFormat f2 = f;
    int origin_position = cursor.position();
    f.setForeground(QBrush(color)); // 动画中的文本颜色
    f2.setForeground(QBrush(QColor(128,128,128, 0))); // 实际的文本颜色
    cursor.setCharFormat(f); //
    cursor.setPosition(position);
    cursor.setPosition(position+1, QTextCursor::KeepAnchor);
    cursor.setCharFormat(f2);
    cursor.setPosition(origin_position); // 恢复光标位置
    cursor.mergeCharFormat(f); // 光标后的文字颜色，设置为原来的 // 避免输入后删除时，出现全屏空白的问题

    //*_flag_is_cursor_change = true;
    _edit->setTextCursor(cursor);
//    emit signalRehighlight(); // 不在这里用，添加多个字符的时候统一进行 rehighlight

    QObject::connect(charactor, SIGNAL(aniFinished(int, EditorInputCharactor*)), this, SLOT(aniFinished(int, EditorInputCharactor*)));
}

void EditorInputManager::aniFinished(int position, EditorInputCharactor* charactor)
{
    ani_list.removeOne(charactor); // 稳妥起见应该在最后面的，但是为了高亮的时候还原文字
    if (position > -1 && position < _edit->toPlainText().length() /*&& charactor->text() == _edit->toPlainText().mid(position, 1)*/ /*文字相同*/)
    {
        QTextCursor cursor = _edit->textCursor();
        QTextCharFormat f = cursor.charFormat();
        QTextCharFormat f2 = f;
        int origin_position = cursor.position();
        f2.setForeground(QBrush(font_color));
        cursor.setPosition(position);
        cursor.setPosition(position+1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(f2);
        cursor.setPosition(origin_position);
        //cursor.mergeCharFormat(f); // 光标后的文字颜色，设置为原来的 // 这句话不能加，这里原来的颜色是透明的，加了会出错

        // *_flag_is_cursor_change = true;
        _edit->setTextCursor(cursor); // 自动触发下面的这个信号

        emit signalRehighlight(); // 使用 rehighlight 代替单独修改（因为修改之后还是会调用 highlight 方法） // 备注：这个方法修改了全部的高亮，低性能
    }
}

void EditorInputManager::updateTextColor(int current_position)
{
    // 这个函数好像会影响性能诶？
    //_edit->selectAll();
    QTextCursor cursor = _edit->textCursor();
    QString text = _edit->toPlainText();
    int len = text.length();
    QTextCharFormat f = cursor.charFormat();
    QTextCharFormat f2 = f;
    f2.setForeground(QBrush(QColor(128, 128, 128, 0))); // 透明

    // 设置全局正确颜色
    f.setForeground(QBrush(font_color));
    //cursor.setPosition(0);
    //cursor.setPosition(len, QTextCursor::KeepAnchor);
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(f);

    // 分别设置正在运行的动画
    for (int i = 0; i < ani_list.count(); i++)
    {
        int pos = ani_list.at(i)->getPosition();
        cursor.setPosition(pos);
        cursor.setPosition(pos+1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(f2);
        //cursor.mergeCharFormat(f);
        //qDebug() << "animating pos : " << pos;
    }
    //cursor.mergeCharFormat(f); // 最后一个字会是黑的……
    cursor.setPosition(current_position);
    //cursor.mergeCharFormat(f); // 会导致最后一个字变成白的

    *_flag_is_cursor_change = true;
    _edit->setTextCursor(cursor);
}
