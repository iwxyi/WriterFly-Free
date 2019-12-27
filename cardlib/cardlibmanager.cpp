#include "cardlibmanager.h"

CardlibManager::CardlibManager(RuntimeInfo *rt, QObject *parent) : QObject(parent),
	rt(rt), cardlibs_dir(rt->CARDLIB_PATH), cardlib(nullptr), color_series(rt->DATA_PATH + "theme_color.txt")
{
    ensureDirExist(cardlibs_dir);

    connect(this, &CardlibManager::signalUseCardByEdit, this, [=](CardBean* card){
        if (cardlib == nullptr) return;
        cardlib->useCard(card);
    });
    connect(this, &CardlibManager::signalEditCardByEdit, this, [=](CardBean *card) {
        if (cardlib == nullptr) return;
        cardlib->addEditingCard(card);
    });

    // 修改一个主题色
    connect(&color_series, &SubjectColorSeries::signalListModified, this, [=](int index) {
        if (cardlib == nullptr)
            return;
        QColor color = getSubjectColor(index);
        foreach (CardBean *card, cardlib->totalCards())
        {
            if (card->ctype ==  index) // 是修改的颜色
            {
                card->color = color;
                cardlib->saveToFile(card);
            }
        }
        emit signalRefreshCardColors();
        emit signalRehighlight();
    });
    // 插入一个主题色
    connect(&color_series, &SubjectColorSeries::signalListInserted, this, [=](int index) {
        if (cardlib == nullptr)
            return;
        foreach (CardBean *card, cardlib->totalCards())
        {
            if (card->ctype >= index) // 索引+1
            {
                card->ctype++;
                cardlib->saveToFile(card);
            }
        }
    });
    // 删除一个主题色
    connect(&color_series, &SubjectColorSeries::signalListDeleted, this, [=](int index) {
        if (cardlib == nullptr)
            return;
        foreach (CardBean *card, cardlib->totalCards())
        {
            if (card->ctype == index) // 删除的颜色，变成自定义
            {
                card->ctype = 0;
                cardlib->saveToFile(card);
            }
            else if (card->ctype > index)
            {
                card->ctype--;
                cardlib->saveToFile(card);
            }
        }
    });
}

CardlibManager::~CardlibManager()
{
    if (cardlib != nullptr)
        cardlib->deleteLater();
}

QString CardlibManager::currentName()
{
    if (cardlib == nullptr)
        return "";
    else
        return cardlib->getName();
}

Cardlib *CardlibManager::currentLib()
{
    return cardlib;
}

SubjectColorSeries &CardlibManager::colorSeries()
{
    return color_series;
}

void CardlibManager::loadCardlib(QString name)
{
    // 取消之前的名片库
    if (cardlib != nullptr)
    {
        emit signalCardlibClosed();
        delete cardlib;
    }

    // 先确保文件存在
    if (!isFileExist(cardlibs_dir + name))
    {
        ensureDirExist(cardlibs_dir + name);
        ensureDirExist(cardlibs_dir + name + "/cards");
    }

    cardlib = nullptr;

    /* QtConcurrent::run([&](QString name, QString dir, QObject *parent) {
        // 大量名片时必须在多线程内进行操作（但是会导致加载时关键词高亮闪一下，所以用了另一种带有信号槽的多线程加载方式）
        Cardlib *cardlib = new Cardlib(name, dir, parent);
        // cardlib->setParent(parent);

        connect(cardlib, &Cardlib::signalUsingCardsModified, [=] { emit signalUsingCardsModified(); });
        connect(cardlib, &Cardlib::signalCardAppened, [=](CardBean *card) { emit signalCardAppened(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardDeleted, [=](CardBean *card) { emit signalCardDeleted(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardDisabled, [=](CardBean *card) { emit signalCardDisabled(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardEnabled, [=](CardBean *card) { emit signalCardEnabled(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardUsed, [=](CardBean *card) { emit signalCardUsed(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardUnused, [=](CardBean *card) { emit signalCardUnused(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalEditingCardAdded, [=](CardBean *card) { emit signalEditingCardAdded(card); emit signalRehighlight(); });

        this->cardlib = cardlib;
        emit signalRehighlight(); // 载入名片库后需要重新高亮
        emit signalReloaded();    // 重新加载名片库完成，通知视图控件
    }, name, cardlibs_dir + name + "/", nullptr); */

    /* CardlibLoadThread *thread = new CardlibLoadThread(name, cardlibs_dir + name + "/");
    connect(thread, &CardlibLoadThread::signalLoadFinished, [=](Cardlib* cardlib) {
        this->cardlib = cardlib;
        connect(cardlib, &Cardlib::signalUsingCardsModified, [=] { emit signalUsingCardsModified(); });
        connect(cardlib, &Cardlib::signalCardAppened, [=](CardBean *card) { emit signalCardAppened(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardDeleted, [=](CardBean *card) { emit signalCardDeleted(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardDisabled, [=](CardBean *card) { emit signalCardDisabled(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardEnabled, [=](CardBean *card) { emit signalCardEnabled(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardUsed, [=](CardBean *card) { emit signalCardUsed(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalCardUnused, [=](CardBean *card) { emit signalCardUnused(card); emit signalRehighlight(); });
        connect(cardlib, &Cardlib::signalEditingCardAdded, [=](CardBean *card) { emit signalEditingCardAdded(card); emit signalRehighlight(); });
        emit signalRehighlight(); // 载入名片库后需要重新高亮
        emit signalReloaded();    // 重新加载名片库完成，通知视图控件
    });
    thread->start(); */

    cardlib = new Cardlib(name, cardlibs_dir+name+"/", this);
    emit signalRehighlight(); // 载入名片库后需要重新高亮
    emit signalReloaded();    // 重新加载名片库完成，通知视图控件
    connect(cardlib, &Cardlib::signalUsingCardsModified, [=]{ emit signalUsingCardsModified(); });
    connect(cardlib, &Cardlib::signalCardAppened, [=](CardBean * card){ emit signalCardAppened(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalCardDeleted, [=](CardBean * card){ emit signalCardDeleted(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalCardDisabled, [=](CardBean * card){ emit signalCardDisabled(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalCardEnabled, [=](CardBean * card){ emit signalCardEnabled(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalCardUsed, [=](CardBean * card){ emit signalCardUsed(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalCardUnused, [=](CardBean * card){ emit signalCardUnused(card); emit signalRehighlight(); });
    connect(cardlib, &Cardlib::signalEditingCardAdded, [=](CardBean * card){ emit signalEditingCardAdded(card); emit signalRehighlight(); });
}

void CardlibManager::addCardlib(QString name)
{
    ensureDirExist(cardlibs_dir + name);
    ensureDirExist(cardlibs_dir + name + "/cards");
}

bool CardlibManager::renameCardlib(QString name)
{
    if (cardlib == nullptr)
        return false;
    QDir cardlib_dir(rt->CARDLIB_PATH + cardlib->getName());
    if (!cardlib_dir.rename(rt->CARDLIB_PATH + cardlib->getName(), rt->CARDLIB_PATH + name))
        return false;
    cardlib->lib_name = name;
    cardlib->dir_path = rt->CARDLIB_PATH + cardlib->getName();
    cardlib->cards_path = cardlib->dir_path + "/cards/";
    return true;
}

void CardlibManager::duplicateCardlib(QString name)
{

}

bool CardlibManager::deleteCardlib(QString name)
{
	if (cardlib == nullptr)
        return false;
    if (!deleteDir(rt->CARDLIB_PATH + name))
        ; // return false; // 删除文件夹，不管有没有成功都当做成功
    if (cardlib->getName() == name)
    {
        delete cardlib;
        cardlib = nullptr;
        emit signalCardlibClosed();
    }
    return true;
}

void CardlibManager::setSubjectColors(QColor def, QString path)
{
    this->default_color = def;
}

QColor CardlibManager::getSubjectColor(int index)
{
    if (index < 0 || index > color_series.count())
        return default_color;
    return color_series.getColor(index);
}

void CardlibManager::refreshAllSeriesItems()
{
    color_series.loadFromFile();

    // 刷新所有名片
    if (cardlib == nullptr)
        return ;
    foreach (CardBean *card, cardlib->totalCards())
    {
        if (card->ctype > 0)
        {
            if (card->ctype <= 0)
                continue;
            if (card->ctype > color_series.count()) // 超过范围
            {
                // card->ctype = 0; // 设置成自定义还是继续保留，都行吧
                // cardlib->saveToFile(card);
            }
            else // card->ctype <= color_series.count()
            {
                if (card->color != getSubjectColor(card->ctype)) // 颜色不一样
                {
                    card->color = getSubjectColor(card->ctype);
                    cardlib->saveToFile(card);
                }
            }
        }
    }
}

void CardlibManager::log(QString s)
{
	return ;
    QString content = readTextFile(QApplication::applicationDirPath() + "/data/runtime.log");
    content = s + "\n" + content;
    writeTextFile(QApplication::applicationDirPath() + "/data/runtime.log", content);
}

QString CardlibManager::getSuitableCardlibName()
{
	QString name;
    if (rt->current_novel.isEmpty())
        name = "名片库";
    else
        name = rt->current_novel;

    int index = 1;
    if (isFileExist(cardlibs_dir + name))
    {
        while (isFileExist(cardlibs_dir + name + QString::number(++index)));
    }
    if (index <= 1)
        return name;
    else
        return name + QString::number(index);
}
