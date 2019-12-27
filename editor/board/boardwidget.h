#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <anitabwidget.h>
#include <QListWidget>
#include <boardpage.h>
#include "globalvar.h"

class BoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoardWidget(QWidget *parent = nullptr);
    void enableAutoDelete();

protected:
    void keyPressEvent(QKeyEvent* event);
    void paintEvent(QPaintEvent *event);

private:
    void initView();
    void toHide();

signals:
    void signalInsertText(QString text);
    void signalHidden(QPixmap pixmap, QRect rect);

public slots:
    void slotTabChanged(int index);
    void slotBoardItemClicked(QListWidgetItem* item);
    void slotFocusChanged(QWidget*, QWidget*);

private:
    AniTabWidget* tab_widget;
};

#endif // BOARDWIDGET_H
