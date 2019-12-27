#ifndef CARDLIBLOADTHREAD_H
#define CARDLIBLOADTHREAD_H

#include <QObject>
#include <QThread>
#include "cardlib.h"

class CardlibLoadThread : public QThread
{
    Q_OBJECT
public:
    CardlibLoadThread(QString name, QString dir): name(name), dir(dir)
    {

    }

    void run()
    {
        Cardlib *cardlib = new Cardlib(name, dir, nullptr);
        emit signalLoadFinished(cardlib);
    }

signals:
    void signalLoadFinished(Cardlib* cardlib);

private:
    QString name;
    QString dir;
};

#endif // CARDLIBLOADTHREAD_H
