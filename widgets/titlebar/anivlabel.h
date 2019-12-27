#ifndef ANIVLABEL_H
#define ANIVLABEL_H

/*
 * 一个 bug 说明：
 * 初始化时，父类 resizeEvent 中调整时，
 * 若里面的标签控件正在运动动画，
 * 改 QWidget 不会自动伸缩……
 */

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include "globalvar.h"

/**
 * 带动画的上下文字过渡标签控件，用于充当标题栏
 */
class AniVLabel : public QWidget
{
    Q_OBJECT
public:
    AniVLabel(QWidget* parent);

    void setMainText(QString text); // 设置文字带动画
    void setStaticText(QString text); // 设置文字不带动画
    void setMainSize(int size); // 设置字体大小

    void setAlign(Qt::Alignment alignment);
    void setFixedWidgetHeight(int h);

protected:
    void resizeEvent(QResizeEvent*);

public slots:
    void slotAnimationFinished();

private:
    QLabel* lb1, *lb2;
    QString _text;
    bool aniing;
};

#endif // ANIVLABEL_H
