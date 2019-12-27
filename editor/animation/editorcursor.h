#ifndef EDITORCURSOR_H
#define EDITORCURSOR_H

#include <QObject>
#include <QWidget>
#include <QColor>
#include <QPalette>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <QDebug>

/**
 * 光标动画类，记录和更新光标位置
 */
class EditorCursor : public QWidget
{
    Q_OBJECT
public:
    explicit EditorCursor(QWidget *parent, QTextEdit *edit);

private:

signals:

public slots:
    void setStartPosition(int position);
    void positionChanged(int position);
    void styleChanged(int width, int height, QColor color);
    void animationFinished();

private:
    int animation_duration;
    int max_distance;
    int cursor_width;
    QPropertyAnimation* animation;
    QTextEdit* _edit;
};

#endif // EDITORCURSOR_H
