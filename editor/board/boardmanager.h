#ifndef BOARDMANAGER_H
#define BOARDMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMimeData>
#include "boardbase.h"

class BoardManager : public QObject
{
    Q_OBJECT
public:
    explicit BoardManager(QString data_dir, QObject *parent = nullptr);

    BoardBase* getClip();
    BoardBase* getRecycle();
    BoardBase* getCollection();

    void tryAddClip(const QString &str);
    void tryAddRecycle(const QString &str);

    void addClip(QString str);
    void addRecycle(QString str);
    void addCollection(QString str);

    void setBoardCount(int x);

    void startConnectClipboard();
    void stopConnectClipboard();
    void setLimits(int cl, int rl);

private:
    qint64 getTimestamp();

signals:

public slots:
    void slotClipboardDataChanged();

private:
    BoardBase clip_board;
    BoardBase recycle_board;
    BoardBase collection_board;

    int clip_limit;
    int recycle_limit;

    QClipboard* system_clipboard;
};

#endif // BOARDMANAGER_H
