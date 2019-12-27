#ifndef EDITORRECORDERITEM_H
#define EDITORRECORDERITEM_H

#include <QDateTime>

/**
 * 撤销重做的实体类
 */
class EditorRecorderItem
{
public:
    EditorRecorderItem();
    EditorRecorderItem(QString text, int pos, int scroll);

    QString getText();
    int getPos();
    int getScroll();
    qint64 getTime();
    void getSelection(int& start, int& end);
    int getCase();

    void setText(QString text);
    void setPos(int pos);
    void setScroll(int scroll);
    void updateTime();
    void setSelection(int start, int end);
    void setCase(int x);

private:
	QString text;
	int pos;
	int scroll;
    qint64 time; // 设置时间戳
    int start;
    int end;
    int cas;
};

#endif // EDITORRECORDERITEM_H
