#ifndef SYNCFINISHEDSEQUEUEDELEGATE_H
#define SYNCFINISHEDSEQUEUEDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#define SyncTimestampRole 1

class SyncFinishedSequeueDelegate: public QItemDelegate
{
    Q_OBJECT
public:
	SyncFinishedSequeueDelegate(QObject* parent = nullptr) : QItemDelegate(parent)
	{
	}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
        QItemDelegate::paint(painter, option, index); // 如果放到最后，会绘制背景色挡住前面所有内容

		// 绘制时间
        qint64 timestamp = index.data(Qt::UserRole + SyncTimestampRole).toLongLong();

        QString str = timestamp > 0 ? getTimeDeltaString(timestamp) : QString::number(timestamp);
        painter->save();
        painter->setPen(Qt::gray);
        painter->drawText(option.rect, Qt::AlignRight, str);
        painter->restore();
	}

private:
    QString getTimeDeltaString(qint64 timestamp) const
	{
        qint64 delta = getTimestamp() - timestamp;
        delta /= 1000; // 转换为秒数

        if (delta <= 120)
            return QString::number(delta) + "秒前";
        else if (delta <= 120 * 60)
            return QString::number(delta / 60) + "分钟前";
        else if (delta <= 24 * 60 * 60)
            return QString::number(delta / 60 / 60) + "小时前";
        else
            return QString::number(delta / 24 / 60 / 60) + "天前";
	}

    qint64 getTimestamp(int mode = 1) const
	{
	    if (mode == 1)
	        return QDateTime::currentDateTime().toMSecsSinceEpoch();
	    else
	        return QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000;
	}
};

#endif // SYNCFINISHEDSEQUEUEDELEGATE_H
