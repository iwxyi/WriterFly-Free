#include "subjectcolorseries.h"

SubjectColorSeries::SubjectColorSeries(QString path, QObject *parent) : QObject(parent), file_path(path)
{
    loadFromFile();
}

void SubjectColorSeries::loadFromFile()
{
    cnames.clear();
    colors.clear();
    QString content = readTextFileIfExist(file_path);
    if (content.isEmpty())
    {
        createDefaultSeries();
        saveToFile();
        return;
    }
    QStringList lines = content.split("\n", QString::SkipEmptyParts);
    QRegExp rx("^#([0-9a-fA-Z]{6}|[0-9a-fA-Z]{8})$");
    foreach (QString line, lines)
    {
        if (rx.exactMatch(line)) // 是颜色
            colors.append(qvariant_cast<QColor>(line));
        else // 是名字
            cnames.append(line);
        if (cnames.length() < colors.length())
            cnames.append("");
    }
}

void SubjectColorSeries::saveToFile()
{
    QStringList full;
    for (int i = 0; i < colors.size(); i++)
    {
        if (!cnames.at(i).isEmpty())
            full.append(cnames.at(i));
        full.append(static_cast<QVariant>(colors.at(i)).toString());
    }
    writeTextFile(file_path, full.join("\n"));
}

void SubjectColorSeries::createDefaultSeries()
{
    // 来自：科学上最令人舒服的十种颜色
    // https://blog.csdn.net/orange_man/article/details/38490429
    colors.append(QColor(25, 202, 173));
    colors.append(QColor(140, 199, 181));
    colors.append(QColor(160, 238, 225));
    colors.append(QColor(190, 231, 233));
    colors.append(QColor(190, 237, 199));
    colors.append(QColor(214, 213, 183));
    colors.append(QColor(209, 186, 116));
    colors.append(QColor(230, 206, 172));
    colors.append(QColor(236, 173, 158));
    colors.append(QColor(244, 96, 108));

    // 自定义
    // https://tieba.baidu.com/p/6054687532?red_tag=1129954826
    colors.append(qvariant_cast<QColor>(QString("#ff00ff")));
    colors.append(qvariant_cast<QColor>(QString("#008000")));
    colors.append(qvariant_cast<QColor>(QString("#70db93")));
    colors.append(qvariant_cast<QColor>(QString("#5c3317")));
    colors.append(qvariant_cast<QColor>(QString("#9f5f9f")));
    colors.append(qvariant_cast<QColor>(QString("#b5a642")));
    colors.append(qvariant_cast<QColor>(QString("#d9d919")));
    colors.append(qvariant_cast<QColor>(QString("#a67d3d")));
    colors.append(qvariant_cast<QColor>(QString("#8c7853")));
    colors.append(qvariant_cast<QColor>(QString("#a67d3d")));

    // 物名
    colors.append(qvariant_cast<QColor>(QString("#5c4033")));
    colors.append(qvariant_cast<QColor>(QString("#2f4f2f")));
    colors.append(qvariant_cast<QColor>(QString("#4a766e")));
    colors.append(qvariant_cast<QColor>(QString("#4f4f2f")));
    colors.append(qvariant_cast<QColor>(QString("#5f9f9f")));

    // 地名
    colors.append(qvariant_cast<QColor>(QString("#9932cd")));
    colors.append(qvariant_cast<QColor>(QString("#d98719")));
    colors.append(qvariant_cast<QColor>(QString("#b87333")));
    colors.append(qvariant_cast<QColor>(QString("#42426f")));
    colors.append(qvariant_cast<QColor>(QString("#ff7f00")));

    /* for (int i = 0; i < 10; i++)
        colors.append(QColor(100, 149, 237)); // 矢车菊蓝
    for (int i = 0; i < 5; i++)
        colors.append(QColor(106, 90, 205)); // 岩蓝
    for (int i = 0; i < 5; i++)
        colors.append(QColor(112, 128, 144)); // 岩灰 */

    for (int i = 0; i < colors.size(); i++)
    {
        cnames.append("");
    }
    cnames[0] = "默认色";
    cnames[10] = "人名";
    cnames[20] = "物名";
    cnames[25] = "地名";
}

int SubjectColorSeries::count()
{
    return colors.count();
}

QList<QColor> SubjectColorSeries::getColors()
{
    return colors;
}

QStringList SubjectColorSeries::getNames()
{
    QStringList names;
    int index = 1; // 第一个就是空的话，就是默认 1
    QString serie = "主题色";
    foreach (const QString cname, cnames)
    {
        if (cname.isEmpty())
        {
            names.append(QString("%1 %2").arg(serie).arg(index++));
        }
        else
        {
            names.append(serie = cname);
            index = 2;
        }
    }
    return names;
}

QColor SubjectColorSeries::getColor(int index)
{
    if (index <= 0 || index > colors.count())
        index = 1;
    return colors.at(index - 1);
}

QString SubjectColorSeries::getColorName(int index)
{
    return cnames.at(index);
}

void SubjectColorSeries::insertItem(int x)
{
    QColor c(Qt::blue);
    if (x < colors.count()) // 插入到上面
        c = colors.at(x);
    else if (x > 0)
        c = colors.at(x - 1);
    colors.insert(x, c);
    cnames.insert(x, "");
    emit signalListInserted(x + 1);
    saveToFile();
}

void SubjectColorSeries::appendItem(int x)
{
    QColor c(Qt::blue);
    if (x >= colors.count() - 1 && colors.count()) // 添加到最后一个
    {
        c = colors.at(x);
        colors.append(c);
        cnames.append("");
    }
    else
    {
        insertItem(x + 1);
        return;
    }
    saveToFile();
}

void SubjectColorSeries::renameItem(int x, QString name)
{
    cnames[x] = name;
    saveToFile();
}

void SubjectColorSeries::modifyItem(int x, QColor color)
{
    colors[x] = color;
    emit signalListModified(x + 1);
    saveToFile();
}

void SubjectColorSeries::deleteItem(int x)
{
    colors.removeAt(x);
    cnames.removeAt(x);
    emit signalListDeleted(x + 1);
    saveToFile();
}
