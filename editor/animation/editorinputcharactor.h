#ifndef EDITORINPUTCHARATOR_H
#define EDITORINPUTCHARATOR_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QPropertyAnimation>
#include <QPixmap>

/**
 * 输入动画的某一个字符实体，存储动画信息、光标位置、光标坐标、光标文字等
 */
class EditorInputCharactor : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int fontsize READ getFontSize WRITE setFontSize RESET resetFontSize)
public:
    EditorInputCharactor(QWidget* parent, QPoint point, QString str, int position, QFont font, int delay = 0, int duration = 200);
    EditorInputCharactor(QWidget* parent, QPoint point, QString str, QColor color, int position, QFont font, int delay = 0, int duration = 200);

    void setPosition(int x);    // （已废弃）
    int getPosition();          // 获取动画的文字位置，更新位置、动画结束文字变回原色时要用到
    void changePosition(int x); // 修改动画的位置

    bool isColorSetted();
    void setColor(QColor c);
    void updateRect(QPoint point); // 更新控件坐标

    // 字体动画接口
    void setFontSize(int x);
    int getFontSize();
    void resetFontSize();

signals:
    void aniFinished(int position, EditorInputCharactor* charactor); // 动画结束

private:
    QPoint point; // 坐标
    QString str;
    int position; // 文字位置
    int duration; // 动画时长，同时输入的不同位置的文字时长不一样
    QPropertyAnimation *ani; // 动画对象
    int origin_position;     // 一开始的位置

    bool color_setted;

    int font_size; // 文字大小动画属性
    int font_size_l;
};

#endif // EDITORINPUTCHARATOR_H
