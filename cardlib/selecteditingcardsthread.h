#ifndef SELECTEDITINGCARDSTHREAD_H
#define SELECTEDITINGCARDSTHREAD_H

#include <QThread>
#include "globalvar.h"
#include <QMetaType>

class SelectEditingCardsThread : public QThread
{
    Q_OBJECT
public:
    SelectEditingCardsThread(const QString&& content);

    void run();

signals:
    void signalRefreshEditingCardList();
    void signalRefreshCardsList();
    void signalCardUsed(CardBean* card);

private:
    QString content;
};

#endif // SELECTEDITINGCARDSTHREAD_H
