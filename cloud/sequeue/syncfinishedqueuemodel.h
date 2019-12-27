#ifndef SYNCFINISHEDQUEUEMODEL_H
#define SYNCFINISHEDQUEUEMODEL_H

#include "syncsequeuemodel.h"

#define SyncTimestampRole 1

class SyncFinishedSequeueModel : public SyncSequeueModel
{
    Q_OBJECT
public:
    SyncFinishedSequeueModel(QStringList* plist, QObject* parent = nullptr) : SyncSequeueModel (plist, parent)
    {
        this->plist = plist;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        // 不需要 index 的 data
        if (role == Qt::UserRole + SyncTimestampRole) // 时间
        {
            if (index.row() >= plist->size()) return -1;
            QString sync = plist->at(index.row());
            QStringList list = getXmls(sync, "SYNC");
            int list_size = list.size();
            if (list_size <= 1) return -1;

            return list.last();
        }
        else if (role == Qt::ToolTipRole)
        {
            QString sync = plist->at(index.row());
            QStringList list = getXmls(sync, "SYNC");
            int list_size = list.size();
            if (list_size <= 2) return plist->at(index.row());
            QString tip = QString("[%1]").arg(list.at(0));
            if (list_size <= 3) tip += " " + list.at(1);
            if (list_size <= 4) tip += " / " + list.at(2);
            return tip;
        }

        return SyncSequeueModel::data(index, role);
    }

};

#endif // SYNCFINISHEDQUEUEMODEL_H
