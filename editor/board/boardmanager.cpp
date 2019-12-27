#include "boardmanager.h"

BoardManager::BoardManager(QString data_dir, QObject *parent) : QObject(parent),
    clip_board(data_dir+"board_clip.txt"),
    recycle_board(data_dir+"board_recycle.txt"),
    collection_board(data_dir+"board_collection.txt"),
    clip_limit(0), recycle_limit(30)
{
}

BoardBase* BoardManager::getClip()
{
    return &clip_board;
}

BoardBase* BoardManager::getRecycle()
{
    return &recycle_board;
}

BoardBase* BoardManager::getCollection()
{
    return &collection_board;
}

void BoardManager::tryAddClip(const QString& str)
{
    if (str.length() >= clip_limit)
        addClip(str);
}

void BoardManager::tryAddRecycle(const QString& str)
{
    if (str.length() >= recycle_limit)
        addRecycle(str);
}

void BoardManager::addClip(QString str)
{
	clip_board.addItem(str, getTimestamp());
}

void BoardManager::addRecycle(QString str)
{
	recycle_board.addItem(str, getTimestamp());
}

void BoardManager::addCollection(QString str)
{
    collection_board.addItem(str, getTimestamp());
}

void BoardManager::setBoardCount(int x)
{
    clip_board.setCount(x);
    recycle_board.setCount(x);
    collection_board.setCount(x);
}

void BoardManager::startConnectClipboard()
{
    QClipboard* cb = QApplication::clipboard();
    connect(cb, SIGNAL(dataChanged()), this, SLOT(slotClipboardDataChanged()));
}

void BoardManager::stopConnectClipboard()
{
    QClipboard* cb = QApplication::clipboard();
    disconnect(cb, SIGNAL(dataChanged()), this, SLOT(slotClipboardDataChanged()));
}

qint64 BoardManager::getTimestamp()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void BoardManager::slotClipboardDataChanged()
{
    QString str = QApplication::clipboard()->text();

    if (str.isEmpty())
    {
        const QMimeData* data = QApplication::clipboard()->mimeData();
        if (data->hasText())
        {
            str = data->text();
        }
    }


    if (!str.isEmpty())
    {
        tryAddClip(str);
    }

}

void BoardManager::setLimits(int cl, int rl)
{
	this->clip_limit = cl;
	this->recycle_limit = rl;
}
