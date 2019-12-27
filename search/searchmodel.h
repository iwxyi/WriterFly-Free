#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractItemModel>
#include <QWidget>
#include <QTimer>
#include <QFontMetrics>
#include "stringutil.h"
#include "globalvar.h"
#include "searchresultbean.h"
#include "defines.h"
#include "noveldirmodel.h"
#include "noveldiritem.h"

/**
 * 小说章节目录的MVD模式中的Model
 */
class SearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SearchModel(QObject *parent = nullptr) : QAbstractListModel(parent)
    {
        item_width = &(rt->search_panel_item_width);
    }

    void setResults(QList<SearchResultBean>rs)
    {
        this->results = rs;
        beginResetModel();
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return results.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        int row = index.row();
        SearchResultBean item = results.at(row);

        if (role == Qt::DisplayRole)
            return results.at(row).text;
        else if (role == Qt::UserRole)
        {
            return results.at(row).getText();
        }
        else if (role == Qt::UserRole+1)
        {
            return results.at(row).type;
        }
        else if (role == Qt::UserRole+2)
        {
            return results.at(row).data;
        }
        // ==== 要显示的文字 ====
        // 如果有标题，则为第二行的信息
        else if (role == SHOW_ROLE)
        {
            ResultType type = static_cast<ResultType>(index.data(Qt::UserRole+1).toInt());
            QString text = index.data(Qt::DisplayRole).toString();
            QString data = index.data(Qt::UserRole).toString();
            QString showed = text;

            if (type == ResultType::CURRENT_CHAPTER || type == ResultType::OPENED_CHAPTERS
                    || type == ResultType::NOVEL_CHAPTERS || type == ResultType::ALL_CHAPTERS) // 章节搜索结果
            {
                int index = getXml(data, "INDEX").toInt();
                int start = getXml(data, "START").toInt();
                int len = getXml(data, "LEN").toInt();
                QString novel = getXml(data, "NOVEL");
                QString chapter = getXml(data, "CHAPTER");
                int pos = getXml(data, "POS").toInt();

                if (start >= 0 && len > 0)
                {
                    text = getXml(data, "TEXT");

                    // 智能截取段落文字位置
                    QFontMetrics fm(qApp->font());
                    int charactor_width = fm.maxWidth();
                    int count = *item_width / charactor_width; // 一行最大字数（以中文为准）

                    int left = start, right = start+len;
                    int lenth = text.length();
                    while (right-left < count)
                    {
                        // 如果刚好停到一个句子的间隔的地方，则先扩展出来
                        if (left > 0 && isSentPunc(text.mid(left-1, 1)))
                            left--;
                        if (right < lenth && isSentPunc(text.mid(right, 1)))
                            right++;

                        // 向两边扩展句子
                        while (left > 0 && !isSentPunc(text.mid(left-1, 1)) && left>right-count)
                            left--;
                        while (right < lenth && !isSentPunc(text.mid(++right, 1)))
                            ;
                        if ((left == 0 || left <= right-count) && right == lenth)
                            break;
                    }

                    text = text.mid(left, right-left);
                    start -= left;
                    showed = text.left(start) + "<font color="+us->getColorString(us->accent_color)+">" + text.mid(start, len) + "</font>" + text.right(text.length()-start-len);
                }
            }
            else if (type == ResultType::NAMES) // 随机取名结果
            {
                QString key = getXml(data, "KEY");
                if (!key.isEmpty()) // 可能直接搜索名字，就不高亮姓氏了
                    showed = "<font color="+us->getColorString(us->accent_color)+">" + key + "</font>" + text;
            }
            else if (type == ResultType::SYNONYMS) // 同义词
            {
                // 同义词啊，直接高亮出来就好了
                showed = "<font color="+us->getColorString(us->accent_color)+">" + text + "</font>";
            }
            else if (type == ResultType::RELATEDS) // 相关词，不进行高亮，一般也用不到
            {

            }
            else if (type == ResultType::SENTENCES)
            {
                QString key = getXml(data, "KEY");
                showed = text.replace(key, "<font color="+us->getColorString(us->accent_color)+">" + key + "</font>");
            }
            else if (type == ResultType::DICTIONARY_ITEM)
            {

            }
            else if (type == ResultType::COMMAND)
            {
                showed = "> <font color="+us->getColorString(us->accent_color)+">" + text + "</font>";
            }
            else if (type == ResultType::CARDLIB)
            {
                showed = getXml(data, "HGLT");
            }
            else if (type == ResultType::CARDLIB_TAG)
            {
                showed = QString("%1 (%2)").arg(text).arg(getXml(data, "COUNT"));
            }
            return showed;
        }
        // ==== 尺寸，默认SHOW_ROLE ====
        else if (role == SIZE_ROLE)
        {
            ResultType type = static_cast<ResultType>(index.data(Qt::UserRole + 1).toInt());
            QString data = index.data(Qt::UserRole).toString();
            if (type == CARDLIB) // 不带高亮的
            {
                return getXml(data, "INFO");
            }
            return this->data(index, SHOW_ROLE);
        }
        // ==== 多行显示的话，标题 ====
        else if (role == TITLE_ROLE)
        {
            ResultType type = static_cast<ResultType>(index.data(Qt::UserRole+1).toInt());
            QString data = index.data(Qt::UserRole).toString();
            QString title = "";

            if (type == ResultType::CURRENT_CHAPTER || type == ResultType::OPENED_CHAPTERS
                    || type == ResultType::NOVEL_CHAPTERS || type == ResultType::ALL_CHAPTERS) // 章节搜索结果
            {
                int index = getXml(data, "INDEX").toInt();
                int start = getXml(data, "START").toInt();
                int len = getXml(data, "LEN").toInt();
                QString novel = getXml(data, "NOVEL");
                QString chapter = getXml(data, "CHAPTER");
                int pos = getXml(data, "POS").toInt();

                if (index == -1)
                {
                    title = "当前章节 - ";
                }
                else
                {
                    if (novel == rt->current_novel)
                        title = chapter + " - ";
                    else
                        title = novel + " / " + chapter + " - ";
                }
                title += QString::number(pos);
            }
            else if (type == ResultType::CARDLIB)
            {
                title = getXml(data, "NAME");
                /* QString cs = getXml(data, "COL");
                // QColor c = qvariant_cast<QColor>(cs);
                title = QString("<font color='%2'>%1</font>").arg(title).arg(cs); */
            }
            return title;
        }
        else if (role == Qt::ToolTipRole)
        {
            QString tip = "";
            switch(item.type)
            {
            case NONE:
                tip = "未知来源";
                break;
            case CURRENT_CHAPTER:
                tip = "当前章节搜索结果";
                break;
            case OPENED_CHAPTERS:
                tip = "已打开的章节搜索结果";
                break;
            case NOVEL_CHAPTERS:
                tip = "全书查找结果";
                break;
            case ALL_CHAPTERS:
                tip = "所有作品查找结果";
                break;
            case SYNONYMS:
                tip = "同义词";
                break;
            case RELATEDS:
                tip = "相关词";
                break;
            case SENTENCES:
                tip = index.data(Qt::DisplayRole).toString();
                tip += "\n\n常用句";
                break;
            case RANDOM_SORTS:
                tip = "随机种类";
                break;
            case RANDOMS:
                tip = "随机取名";
                break;
            case SURNAMES:
                tip = "随机姓氏";
                break;
            case NAMES:
                tip = "随机人名";
                break;
            case DICTIONARY_ITEM:
                tip = "汉语词典：" + getXml(index.data(Qt::UserRole+2).toString(), "KEY") + "\n" +index.data(Qt::DisplayRole).toString();
                break;
            case CARDLIB:
                tip = "名片：" + getXml(index.data(Qt::UserRole+2).toString(), "INFO");
                break;
            default:
                break;
            }
            if (!tip.isEmpty())
            {
                tip += "  (单击插入到正文)";
                return tip;
            }
        }

        return QVariant();
    }

    static bool isSentPunc(QString c)
    {
        return QString(",!?，。！？…—\n\r“”").indexOf(c) > -1;
    }

signals:


public:
    const int* item_width;

private:
    QList<SearchResultBean>results;
};

#endif // SEARCHMODEL_H
