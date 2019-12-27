#ifndef EDITORINPUTMANAGER_H
#define EDITORINPUTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QList>
#include <QTextCharFormat>
#include <QDebug>
#include "editorinputcharactor.h"

/**
 * 输入动画管理器类
 */
class EditorInputManager : public QObject
{
    Q_OBJECT
public:
    EditorInputManager();
    friend class MyHightlighter;

    void setEditor(QTextEdit* edit); // 设置开启动画对应的编辑器
    void setFlag(bool* flag);        // 动画结束后会出现QTextCursor被改变事件。已通过另一方式解决，此项已废弃

    void setColor(QColor color);     // 设置全文颜色

    void textChanged(int old_position, int diff);    // 文字改变时调整动画控件
    void updateRect(int range_start, int rande_end); // 更新动画控件的位置

    void addInputAnimation(QPoint point, QString str, int position, int delay, int duration); // 添加一个文字动画
    void addInputAnimation(QPoint point, QString str, int position, int delay, int duration, QColor color); // 添加一个带有不同颜色的文字动画

    void updateTextColor(int current_position); // 修改全文颜色，同时修改正在动画的颜色

public slots:
    void aniFinished(int position, EditorInputCharactor *); // 动画结束，传参文字位置

signals:
    void signalRehighlight();

private:
    QTextEdit* _edit;  // 编辑器
    bool* _flag_is_cursor_change; // （已废弃）
    QColor font_color; // 全文颜色
    QList<EditorInputCharactor*> ani_list; // 动画控件列表
};

#endif // EDITORINPUTMANAGER_H
