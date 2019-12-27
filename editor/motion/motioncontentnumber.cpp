#include "motioncontentnumber.h"

MotionContentNumber::MotionContentNumber(QTextEdit *edit) : MotionBoard(edit)
{
    input_edit = new QLineEdit(this);
    input_edit->setStyleSheet("background: transparent; border: none;");

    // 固定在右上角
    input_edit->move(width()-input_edit->width(), height()-input_edit->height());
    input_edit->show();

    // 输入内容信号槽
    input_edit->setFocus();
    connect(input_edit, SIGNAL(textChanged(const QString &)), this,  SLOT(slotInputted(const QString &)));
}

void MotionContentNumber::initAvaliableMotion()
{

}

void MotionContentNumber::slotInputted(const QString &text)
{
    if (text.trimmed().isEmpty())
    {
        clearMotionNodes();
        return ;
    }

    // 获取内容
    int pos = edit->textCursor().position();
    QString content = edit->toPlainText();
    QTextCursor cursor = edit->textCursor();

    // 找到对应的位置了
    if (QString("123456789").contains(text.right(1)) && !content.contains(text))
    {
        int index = QString("123456789").indexOf(text.right(1));
        if (index >= 0 && index < nodes.size())
        {
            emit signalMotionTo(nodes.at(index).pos);
        }
        this->close();
        return ;
    }

    clearMotionNodes();

    // 检索上下文，按距离排序
    int len = text.length(), full_len = content.length();            // 文本长度
    int pos_left = pos-len, pos_right = pos+len;                     // 找到的左右
    bool ava_left = pos_left >= 0, ava_right = pos_right < full_len; // 左右是否有效
    int index = 0;
    while (ava_left || ava_right)
    {
        if (ava_left) // 搜索左边的位置
        {
            pos_left = content.lastIndexOf(text, pos_left);
            if (pos_left < 0)
                ava_left = false;
            else
            {
                cursor.setPosition(pos_left);
                QRect rect = edit->cursorRect(cursor);
                if (rect.bottom() <= 0)
                    ava_left = false;
            }
        }

        if (ava_right) // 搜索右边的位置
        {
            pos_right = content.indexOf(text, pos_right);
            if (pos_right < 0)
                ava_right = false;
            else
            {
                cursor.setPosition(pos_right);
                QRect rect = edit->cursorRect(cursor);
                if (rect.top() >= edit->height())
                    ava_right = false;
            }
        }

        if (ava_left && ava_right) // 左右都有效，只添加一个
        {
            if (pos-pos_left <= pos_right-pos)
            {
                addMotionNodes(pos_left, getNumber(text, index, content));
                pos_left -= len;
            }
            else
            {
                addMotionNodes(pos_right, getNumber(text, index, content));
                pos_right += len;
            }
        }
        else if (ava_left)
        {
            addMotionNodes(pos_left, getNumber(text, index, content));
            pos_left -= len;
        }
        else if (ava_right)
        {
            addMotionNodes(pos_right, getNumber(text, index, content));
            pos_right += len;
        }

        if (index >= 9)
            break;
    }

    update();
}

bool MotionContentNumber::isChildWidget(QWidget *widget)
{
    return widget == input_edit || MotionBoard::isChildWidget(widget);
}

QString MotionContentNumber::getNumber(const QString &key, int &number, const QString &text)
{
    while (text.contains(key + QString::number(++number)));
    return QString::number(number);
}
