#ifndef MOTIONBOARD_H
#define MOTIONBOARD_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QApplication>
#include <QList>
#include <QPainter>
#include <QScrollBar>
#include <QDebug>

#define BACK_COLOR QColor(255, 255, 77)
#define FORE_COLOR QColor(0, 0, 0)

class MotionBoard : public QWidget
{
    Q_OBJECT
public:
    MotionBoard(QTextEdit *edit);

    struct MotionNode {
        QPoint coor;  // 坐标（所在光标的中间点）
        int pos;      // 字符位置
        QString show; // 显示的字符
        QString text; // 内容
    };

protected:
    void clearMotionNodes();
    void addMotionNodes(int pos, QString show);
    void addMotionNodes(int pos, QString show, QString text);
    void addMotionNodes(QPoint point, int pos, QString show, QString text);

    virtual void initAvaliableMotion();
    virtual bool isChildWidget(QWidget *widget);
    virtual void motionTo(MotionNode node);

    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void signalMotionTo(int pos);
    void signalClosed();

public slots:

protected:
    QTextEdit* edit;
    QList<MotionNode>nodes;
};

#endif // MOTIONBOARD_H
