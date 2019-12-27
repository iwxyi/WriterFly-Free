#include "editorcursor.h"

EditorCursor::EditorCursor(QWidget *parent, QTextEdit* edit) : QWidget(parent), _edit(edit)
{
    //connect(_edit, SIGNAL(cursorPositionChanged()), this, SLOT(positionChanged()));

    animation = new QPropertyAnimation(this, "pos");
    cursor_width = _edit->cursorWidth();
    connect(animation, SIGNAL(finished()), this, SLOT(animationFinished()));

    animation_duration = 100;
    max_distance = 15;
    setAutoFillBackground(true);
    this->hide();
}

void EditorCursor::setStartPosition(int position)
{
    if (position < 0) position = _edit->textCursor().position();
    QTextCursor cursor = _edit->textCursor();
    cursor.setPosition(position);
    move(_edit->cursorRect(cursor).topLeft());
}

void EditorCursor::positionChanged(int position)
{
    QPoint aim;
    QPoint pos = this->pos();
    bool no_ani = false;

    if (position == -2)
    {
        no_ani = true;
        position = _edit->textCursor().position();
    }

    if (position != -1 && _edit->textCursor().position() != position) // 比如获取到的位置是0……
    {
        QTextCursor cursor = _edit->textCursor();
        cursor.setPosition(position);
        QRect rect = _edit->cursorRect(cursor);
        aim = rect.topLeft();
    }
    else
    {
         aim = _edit->cursorRect().topLeft();
    }

    int manhattan = (aim - pos).manhattanLength(); // 获取曼哈顿距离
    if (manhattan <= 1) return ;

    // 如果距离大于设置的最大距离，则直接移动到对应位置
    // 根据线段公式来计算
    pos = QPoint(int(aim.x()+(pos.x()-aim.x())*0.6), int(aim.y()+(pos.y()-aim.y())*0.6) );
    if (manhattan > max_distance)
    {
        //pos = QPoint(aim.x()+(pos.x()-aim.x())*max_distance/manhattan, aim.y()+(pos.y()-aim.y())*max_distance-manhattan );
    }

    if (!no_ani)
    {
        int duration = animation_duration;
        if (duration > manhattan * 5)
            duration = manhattan * 5;

        this->show();
        animation->stop();
        animation->setDuration(animation_duration);
        animation->setStartValue(pos);
        animation->setEndValue(aim);
        animation->start();

        _edit->setCursorWidth(0);
    }
    else
    {
        this->move(aim);
    }

}

void EditorCursor::styleChanged(int width, int height, QColor color)
{
    //qDebug() << "style changed : " << width << ", " << height << "  " << color;
    setGeometry(geometry().left(), geometry().top(), width, height);
    cursor_width = width;

    QPalette palette = this->palette();
    palette.setColor(QPalette::ColorRole::Background, color); // 因为莫名的bug，光标只能是黑色的……
//    palette.setColor(QPalette::ColorRole::Background, Qt::black);
    setPalette(palette);

    max_distance = height * 2 + width * 2;
}

void EditorCursor::animationFinished()
{
    _edit->setCursorWidth(cursor_width);
    this->hide();
}
