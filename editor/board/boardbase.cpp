#include "boardbase.h"

BoardBase::BoardBase(QString path)
{
	file_path = path;
    max_count = 100;

    readFromFile();
}

void BoardBase::addItem(QString str, qint64 timestamp)
{
	deleteItem(str); // 去掉重复的文字
    contents.append(BoardItem(str, timestamp));

    // 去掉多余的内容
    while (contents.count() > max_count)
        contents.removeFirst();

	writeToFile();
}

void BoardBase::deleteItem(QString str)
{
	for (int i = 0; i < contents.count(); i++)
		if (str == contents.at(i) .text)
		{
			contents.removeAt(i);
			break;
		}
}

void BoardBase::deleteItem(int index)
{
    contents.removeAt(index);

    writeToFile();
}

QList<BoardItem> BoardBase::getRecent()
{
	QList<BoardItem> res;
	int count = contents.count();
	if (count > 10)
		count = 10;
	for (int i = 0; i < count; i++)
		res << contents.at(i);

	return res;
}

QList<BoardItem> BoardBase::getAll()
{
	return contents;
}

QStringList BoardBase::getRecentSting()
{
    QStringList res;
	int count = contents.count();
	if (count > 10)
		count = 10;
	for (int i = 0; i < count; i++)
		res << contents.at(i).text;

	return res;
}

QStringList BoardBase::getAllSting()
{
	QStringList ss;
	for (int i = 0; i < contents.count(); i++)
	{
		ss << contents.at(i).text;
	}
	return ss;
}

void BoardBase::clear()
{
	contents.clear();

	writeToFile();
}

void BoardBase::readFromFile()
{
	if (file_path.isEmpty() || !isFileExist(file_path))
		return ;

	contents.clear();
	QString str = readTextFile(file_path);
	QStringList texts = getXmls(str, "__ITEM__");
	for (int i = 0; i < texts.count(); i++)
		contents.append(BoardItem(texts.at(i)));
}

void BoardBase::writeToFile()
{
	if (file_path.isEmpty())
		return ;

	QString text = "";
	for (int i = 0; i < contents.count(); i++)
        text += makeXml((contents.at(i)).toString(), "__ITEM__");
	writeTextFile(file_path, text);
}

int BoardBase::count()
{
	return contents.count();
}

void BoardBase::setCount(int x)
{
	this->max_count = x;

    // 去掉多余的内容
    while (contents.count() > max_count)
        contents.removeFirst();
}
