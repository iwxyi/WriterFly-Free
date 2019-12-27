#ifndef CARDLIB_H
#define CARDLIB_H

#include <QObject>
#include <QApplication>
#include "cardbean.h"
#include "fileutil.h"
#include "stringutil.h"

/**
 * 名片库类
 * 针对本身的各种方法
 * 不包括和外界对象打交道
 */

typedef QList<CardBean *> CardList;

class Cardlib : public QObject
{
    Q_OBJECT
    friend class CardlibManager;
public:
    Cardlib(QString name, QString dir, QObject *parent = nullptr);

    QString getName();
    CardBean* getCard(QString id);
    CardList totalCards();

    void addCard(QString name, QString pat, int ctype, QColor color, QStringList tags, QString brief, QString detail);
    void addCard(CardBean *card);
    void deleteCard(CardBean *card);
    void disableCard(CardBean *card);
    void useCard(CardBean *card);
    void unuseCard(CardBean *card);
    void addEditingCard(CardBean *card);

    QHash<QString, int> getAllTags();
    QStringList getAllTagsOrderByName(QString name);

private:
    void appendCard(CardBean *card);
    QString getRandomId();

    void log(QString s);

signals:
    void signalRehighlightAll();
    void signalUsingCardsModified();
    void signalEditingCardsModified();
    void signalCardAppened(CardBean *card);
    void signalCardDeleted(CardBean *card);
    void signalCardDisabled(CardBean *card);
    void signalCardEnabled(CardBean *card);
    void signalCardUsed(CardBean *card);
    void signalCardUnused(CardBean *card);
    void signalEditingCardAdded(CardBean *card);

public slots:
    void saveToFile(CardBean *card);
    void loadFromFile();

public:
    CardList editing_cards;  // 编辑中的，不参与以下分类
    CardList using_cards;    // 1、正在使用的名片（近期使用）
    CardList unused_cards;   // 2、已加载却并未使用到的名片
    CardList disabled_cards; // 3、已经暂停使用的名片

private:
    QString lib_name;
    QString dir_path;
    QString cards_path;

	bool _flag_loading;
};

/* 名片格式：
<NAME>名字</NAME>
<PAT>表达式</PAT>
<CTYPE>颜色类型</CTYPE>
<COLOR>颜色值</COLOR>
<ENABLE>启用<ENABLE>
<USED>用到<USED>
<BRIEF>简介<BRIEF>
<DETAIL>详情</DETAIL>
*/

#endif // CARDLIB_H
