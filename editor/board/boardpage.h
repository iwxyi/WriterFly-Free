#ifndef BOARDPAGE_H
#define BOARDPAGE_H

#include <QObject>
#include <QListWidget>
#include "boardbase.h"
#include "globalvar.h"

class BoardPage : public QListWidget
{
public:
    BoardPage(BoardBase* data, QWidget* parent = nullptr);

private:
    void initView();
    void initData();

    QString getShowedText(QString text, bool* isFull);

private:
	BoardBase* data;
};

#endif // BOARDPAGE_H
