#ifndef SYNCSEQUEUEMOEDL_H
#define SYNCSEQUEUEMOEDL_H

#include <QAbstractItemModel>
#include "stringutil.h"

class SyncSequeueModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SyncSequeueModel(QStringList* plist, QObject* parent = nullptr) : QAbstractListModel(parent)
	{
        this->plist = plist;
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return plist->count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        // 不需要 index 的 data
        if (role == Qt::DisplayRole)
        {
        	if (index.row() >= plist->size()) return "";
            QString sync = plist->at(index.row());
            QStringList list = getXmls(sync, "SYNC");
        	int list_size = list.size();
        	if (list_size <= 1) return "";
        	QString ans = "";
            QString type = list.at(0);

            if (type == "目录" || type == "删书" || type == "大纲目录")
        	{
                if (list_size < 2) return "";
                return QString("[%1] %2").arg(list.at(0)).arg(list.at(1));
        	}
            else if (type == "章节" || type == "删章" || type == "大纲")
        	{
        		if (list_size < 3) return "";
                return QString("[%1] %2 / %3").arg(list.at(0)).arg(list.at(1)).arg(list.at(2));
        	}

            return ans;
        }

        return QVariant();
    }

    void listDataChanged()
    {
        beginResetModel();
        endResetModel();
    }

protected:
	QStringList* plist;
};

#endif // SYNCSEQUEUEMOEDL_H
