#include "cardlib.h"

Cardlib::Cardlib(QString name, QString dir, QObject *parent) : QObject(parent),
        lib_name(name), dir_path(dir), cards_path(dir + "/cards/"), _flag_loading(false)
{
    ensureDirExist(dir);

    loadFromFile();
}

QString Cardlib::getName()
{
    return lib_name;
}

CardBean* Cardlib::getCard(QString id)
{
    foreach(CardBean* card, using_cards)
    {
        if (card->id == id)
            return card;
    }
    foreach (CardBean *card, unused_cards)
    {
        if (card->id == id)
            return card;
    }
    foreach (CardBean *card, disabled_cards)
    {
        if (card->id == id)
            return card;
    }
    return nullptr;
}

CardList Cardlib::totalCards()
{
    CardList cards;
    cards << using_cards;
    cards << unused_cards;
    cards << disabled_cards;
    return cards;
}

void Cardlib::loadFromFile()
{
	_flag_loading = true;
    QDir dir(cards_path);
    QStringList list = dir.entryList(QStringList{"*.card"});

    using_cards.clear();
    unused_cards.clear();
    disabled_cards.clear();
    foreach (QString s, list)
    {
        QString content = readTextFile(cards_path + s);
        CardBean *card = new CardBean;
        card->id = QFileInfo(s).baseName();
        card->name = getXml(content, "NAM");
        card->pat = getXml(content, "PAT");
        card->ctype = getXmlInt(content, "CTY");
        card->color = qvariant_cast<QColor>(getXml(content, "COL"));
        card->disabled = !getXml(content, "DIS").isEmpty();
        card->used = getXml(content, "UNU").isEmpty();
        card->tags = getXml(content, "TAGS").split(" ", QString::SkipEmptyParts);
        card->brief = getXml(content, "BRI");
        card->detail = getXml(content, "DET");

        appendCard(card);
    }
    _flag_loading = false;
    // emit signalRehighlightAll(); // 刚创建时还没有连接信号槽，这一行用不着了
}

void Cardlib::saveToFile(CardBean *card)
{
    QString full =
        makeXml(card->name, "NAM")
        + "\n" + makeXml(card->pat, "PAT")
        + "\n" + makeXml(card->ctype, "CTY")
        + "\n" + makeXml(QVariant(card->color).toString(), "COL")
        + (card->disabled ? "\n" + makeXml("1", "DIS") : "")
        + (card->used ? "" : "\n" + makeXml("1", "UNU"))
        + "\n" + makeXml(card->tags.join(" "), "TAGS")
        + "\n" + makeXml(card->brief, "BRI")
        + "\n" + makeXml(card->detail, "DET");
    writeTextFile(cards_path + card->id + ".card", full);
}

void Cardlib::addCard(QString name, QString pat, int ctype, QColor color, QStringList tags, QString brief, QString detail)
{
    CardBean *card = new CardBean;
    card->id = getRandomId();
    card->name = name;
    card->pat = pat;
    card->ctype = ctype;
    card->color = color;
    card->brief = brief;
    card->detail = detail;
    saveToFile(card);
    appendCard(card);
}

void Cardlib::addCard(CardBean *card)
{
    if (card->id.isEmpty())
        card->id = getRandomId();
    saveToFile(card);
    appendCard(card);
}

void Cardlib::appendCard(CardBean *card)
{
    if (!card->disabled)
    {
        if (card->used)
            using_cards.append(card);
        else
            unused_cards.append(card);
    }
    else
    {
        disabled_cards.append(card);
    }
    if (!_flag_loading)
        emit signalCardAppened(card);
}

void Cardlib::deleteCard(CardBean *card)
{
	log("删除名片");
    if (card->disabled)
    {
        disabled_cards.removeOne(card);
    }
    else if (card->used)
    {
        using_cards.removeOne(card);
    }
    else
    {
        unused_cards.removeOne(card);
    }
    if (isFileExist(cards_path+card->id+".card"))
        deleteFile(cards_path+card->id+".card");

    emit signalCardDeleted(card);
    delete card;
}

void Cardlib::disableCard(CardBean *card)
{
    if (card->disabled) // 禁用
    {
        if (card->used)
            using_cards.removeOne(card);
        else
            unused_cards.removeOne(card);
        disabled_cards.append(card);
        emit signalCardDisabled(card);
    }
    else // 解除禁用
    {
        disabled_cards.removeOne(card);
        if (card->used)
            using_cards.append(card);
        else
            unused_cards.append(card);
        emit signalCardEnabled(card);
    }
}

void Cardlib::useCard(CardBean *card)
{
    using_cards.append(card);
    unused_cards.removeOne(card);
    // emit signalUsingCardsModified();
    emit signalCardUsed(card);
}

void Cardlib::unuseCard(CardBean *card)
{
    using_cards.removeOne(card);
    unused_cards.append(card);
    // emit signalUsingCardsModified();
    emit signalCardUnused(card);
}

void Cardlib::addEditingCard(CardBean *card)
{
    editing_cards.append(card);
    emit signalEditingCardAdded(card);
}

QHash<QString, int> Cardlib::getAllTags()
{
	QHash<QString, int> tags;
    foreach (CardBean *card, totalCards())
    {
        foreach (QString s, card->tags)
        {
            if (s == "")
                continue;
            if (tags.contains(s))
            {
                tags[s]++;
            }
            else
            {
                tags[s] = 1;
            }
        }
    }
    return tags;
}

QString Cardlib::getRandomId()
{
    QString id;
    QString all_str = "1234567890qwertyuiopasdfghjklzxcvbnm";
    do
    {
        id = "";
        int x = 10;
        while (x--)
            id += all_str.at(rand() % all_str.length());
    } while (isFileExist(cards_path + id + ".card"));
    return id;
}

void Cardlib::log(QString s)
{
	return ;
    QString content = readTextFile(QApplication::applicationDirPath() + "/data/runtime.log");
    content = s + "\n" + content;
    writeTextFile(QApplication::applicationDirPath() + "/data/runtime.log", content);
}
