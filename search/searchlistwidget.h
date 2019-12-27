#ifndef SEARCHLISTWIDGET_H
#define SEARCHLISTWIDGET_H

#include <QListView>
#include <QMouseEvent>
#include <QDebug>

class SearchListWidget : public QListView
{
    Q_OBJECT
public:
    SearchListWidget(QWidget* parent = nullptr) : QListView(parent)
    {
        setMouseTracking(true);
    }

protected:
    void mouseMoveEvent(QMouseEvent* event)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid())
        {
            if (currentIndex().row() != index.row())
            {
                setCurrentIndex(index);
                emit signalItemChanged(index.row());
            }
        }

        return QListView::mouseMoveEvent(event);
    }

signals:
    void signalItemChanged(int);
};

#endif // SEARCHLISTWIDGET_H
