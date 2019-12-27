#ifndef CARDLIBMANAGER_H
#define CARDLIBMANAGER_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "cardlib.h"
#include "subjectcolorseries.h"
#include "runtimeinfo.h"
#include "cardlibloadthread.h"

/**
 * 名片库管理类
 * 针对多个名片库进行管理
 * 名片库本身的操作在名片库类中
 */

class CardlibManager : public QObject
{
    Q_OBJECT
public:
    CardlibManager(RuntimeInfo *rt, QObject *parent = nullptr);
    ~CardlibManager();

    QString currentName();
    Cardlib *currentLib();
    SubjectColorSeries &colorSeries();

    void setSubjectColors(QColor def, QString path);
    QColor getSubjectColor(int x);
    void refreshAllSeriesItems();

    QString getSuitableCardlibName();

private:
	void log(QString s);

signals:
    void signalRehighlight();
    void signalReloaded();
    void signalRefreshCardColors();
    void signalCardlibClosed();

    void signalUsingCardsModified();
    void signalEditingCardsModified();
    void signalCardColorChanged(CardBean *card);
    void signalCardAppened(CardBean *card);
    void signalCardDeleted(CardBean *card);
    void signalCardDisabled(CardBean *card);
    void signalCardEnabled(CardBean *card);
    void signalCardUsed(CardBean *card);
    void signalCardUnused(CardBean *card);
    void signalEditingCardAdded(CardBean* card);

    void signalUseCardByEdit(CardBean *card);
    void signalEditCardByEdit(CardBean *card);

public slots:
    void loadCardlib(QString name);
    void addCardlib(QString name);
    bool renameCardlib(QString name);
    void duplicateCardlib(QString name);
    bool deleteCardlib(QString name);

public:
    RuntimeInfo *rt;
    QString cardlibs_dir;
    Cardlib *cardlib;
    QColor default_color; // 没有主题名片色的默认色（应该为点缀色）
    SubjectColorSeries color_series;
};

#endif // CARDLIBMANAGER_H
