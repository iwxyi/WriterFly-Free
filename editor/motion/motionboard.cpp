#include "motionboard.h"

MotionBoard::MotionBoard(QTextEdit *edit) : QWidget(edit), edit(edit)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setGeometry(0, 0, edit->width()-edit->verticalScrollBar()->width(), edit->height());
    setFont(edit->font());

    initAvaliableMotion();
    this->show();
    this->setFocus();

    connect(qApp, &QApplication::focusChanged, this, [=](QWidget *, QWidget *c) {
        if (isChildWidget(c))
            ;
        else
            this->close();
    });
}

void MotionBoard::clearMotionNodes()
{
    nodes.clear();
}

void MotionBoard::addMotionNodes(int pos, QString show)
{
    addMotionNodes(pos, show, show);
}

void MotionBoard::addMotionNodes(int pos, QString show, QString text)
{
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(pos);
    QRect rect = edit->cursorRect(cursor);
    QPoint center = rect.center();
    addMotionNodes(center, pos, show, text);
}

void MotionBoard::addMotionNodes(QPoint point, int pos, QString show, QString text)
{
    nodes << MotionNode{point, pos, show, text};
}

/**
 * 初始化可移动的光标节点位置
 * 例如按照句子、词句进行分组
 */
void MotionBoard::initAvaliableMotion()
{
    update();
}

/**
 * 焦点改变时，判断焦点是不是自己
 * 如果不是的话，则关闭本次移动
 */
bool MotionBoard::isChildWidget(QWidget *widget)
{
    return widget == this;
}

/**
 * 确定移动到某一个节点
 */
void MotionBoard::motionTo(MotionNode node)
{
    emit signalMotionTo(node.pos);
    this->close();
}

void MotionBoard::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.fillRect(QRect(0,0,width(),height()), QColor(128,128,128,32));

    // 文字高度
    QFont font(edit->font());
    QFontMetrics fm(font);
    int h = fm.height(); // 行高度

    // 绘制每一个节点的内容
    foreach (MotionNode node, nodes)
    {
        int w = fm.horizontalAdvance(node.show);
        QPainterPath path;
        path.addRoundedRect(node.coor.x()-w/2-1, node.coor.y()-h/2-2, w+3, h+4, 3, 3);
        painter.fillPath(path, BACK_COLOR);

//        painter.drawText(node.coor.x()-w/2, node.coor.y()+h/2, node.show);
        painter.drawText(QRect(node.coor.x()-w/2, node.coor.y()-h/2, w, h), Qt::AlignCenter, node.show);
    }
}

void MotionBoard::closeEvent(QCloseEvent *event)
{
    disconnect(qApp, nullptr, this, nullptr);
    edit->setFocus();
    emit signalClosed();
    this->deleteLater();
//    return QWidget::closeEvent(event);
}

void MotionBoard::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape) // 退出
    {
        this->close();
    }
    else if (key == Qt::Key_Enter || key == Qt::Key_Return) // 确定
    {
        if (nodes.size() > 0)
        {
            motionTo(nodes.first());
        }
        this->close();
    }
    event->accept();
}
