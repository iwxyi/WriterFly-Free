#ifndef SUBJECTCOLORDIALOG_H
#define SUBJECTCOLORDIALOG_H

#include <QMenu>
#include <QInputDialog>
#include "colordialog.h"
#include "globalvar.h"
#include "mylabel.h"

class SubjectColorDialog : public ColorDialog
{
    Q_OBJECT
public:
    static QColor pick(QString title, int* ctype, QColor color, QWidget* parent = nullptr);
    static QColor pick(QString title, int* ctype, QColor color, bool* ok, QWidget* parent = nullptr);
    static QColor pickF(QString title, int* ctype, QColor color, QWidget* parent = nullptr);
    static QColor pickF(QString title, int* ctype, QColor color, bool* ok, QWidget* parent = nullptr);
    static QColor pick(QString key, QString title, int* ctype, QColor color, QWidget* parent = nullptr);
    static QColor pick(QString key, QString title, int* ctype, QColor color, bool* ok, QWidget* parent = nullptr);
    static QColor pickF(QString key, QString title, int* ctype, QColor color, QWidget* parent = nullptr);
    static QColor pickF(QString key, QString title, int* ctype, QColor color, bool* ok, QWidget* parent = nullptr);

private:
    SubjectColorDialog(QColor color, QWidget* parent = nullptr);
    SubjectColorDialog(QColor color, bool fore = false, QWidget* parent = nullptr);
    SubjectColorDialog(QString key, QColor color, QWidget* parent = nullptr);
    SubjectColorDialog(QString key, QColor color, bool fore = false, QWidget* parent = nullptr);

protected:
    bool selectSimilarColor(QColor color);

private:
    void initView();
    void addOneColor(QString name, QColor color);
    void showSelectAnimation(int x);

public slots:
    void loadSubjectColors();
    void slotListWidgetItemChanged(int row);
    void slotListWidgetDoubleClicked(QListWidgetItem* item);
    void slotShowItemMenu(QPoint p);
    void slotHexChanged(const QString &text);

    void actionRefreshAllSeriesItems();
    void actionInsertSeriesItem();
    void actionAppendSeriesItem();
    void actionRenameSeriesItem();
    void actionDeleteSeriesItem();
    void actionModifySeriesItem();

private:
    QListWidget* subject_list;
    QList<QColor> subjects;

    QMenu* menu;

    bool _flag_ignore_row_changed_once = false;
};

#endif // SUBJECTCOLORDIALOG_H
