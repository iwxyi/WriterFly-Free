#ifndef CARDLIBTAGSAI_H
#define CARDLIBTAGSAI_H

#include "cardlib.h"
#include "locatemutiple.h"
#include "globalvar.h"

#define TGSDEB if (0) qDebug()
#define DOUBLEIGNORE 0.0001

struct TagInfo {
    QString tag;   // 名字
    QColor color;  // 标签颜色
    bool selected; // 选中
};

class CardlibTagsAI : public QObject
{
    Q_OBJECT
public:
    QList<TagInfo> static getAllTagsOrderName(QString name, bool auto_select = false, QObject* parent = nullptr)
    {
        CardlibTagsAI* ai = new CardlibTagsAI(parent);
        return ai->startAnalyze(name, QStringList(), auto_select);
    }
    QList<TagInfo> static getAllTagsOrderName(QString name, QStringList selected_list, bool auto_select = false, QObject* parent = nullptr)
    {
        CardlibTagsAI* ai = new CardlibTagsAI(parent);
        return ai->startAnalyze(name, selected_list, auto_select);
    }

protected:
    CardlibTagsAI(QObject* parent = nullptr) : QObject(parent)
    {
        this->cardlib = gd->clm.currentLib();
        tf_idf = new LocateMultiple(gd->participle, gd->lexicons.getStopWords());
    }

    /**
     * 综合所有的可能性
     * 词向量、开头、结尾三种结果累加
     * 然后再进行排序
     */
    QList<TagInfo> startAnalyze(QString name, QStringList selected_list, bool auto_select = false)
    {
        cards = cardlib->totalCards();
        all_tags = cardlib->getAllTags();

        // 计算每个标签的概率
        if (!name.isEmpty())
        {
            orderByWordVector(name);
            orderByHeader(name);
            orderByTailer(name);
            TGSDEB << "tag_posss" << tag_posss;
        }

        // 对其进行插入排序
        QStringList select_list = selected_list;
        QList<QPair<QString,double>> order_poss = orderByPoss(select_list, auto_select);

        // 没有用到的标签，按出现次数，继续插入排序到后面
        QList<QPair<QString,int>> order_count = orderByCount();

        // 获取所有标签的颜色
        QHash<QString,QColor> tags_color = getTagsColor();

        // 整合所有的标签
        QColor default_color = us->global_font_color;
        QList<TagInfo> results;
        for (int i = 0; i < order_poss.size(); i++)
        {
            results.append(TagInfo{order_poss.at(i).first, tags_color.value(order_poss.at(i).first, default_color), select_list.contains(order_poss.at(i).first)});
        }
        for (int i = 0; i < order_count.size(); i++)
        {
            results.append(TagInfo{order_count.at(i).first, tags_color.value(order_count.at(i).first, default_color), select_list.contains(order_count.at(i).first)});
        }
        TGSDEB << "---->>>>最终结果：" << results.size();

        return results;
    }

    /**
     * 根据词向量判断可能性
     */
    void orderByWordVector(QString name)
    {
        WordVec vec = tf_idf->stringToVector(name);
        // 每个分段转换为向量
        QList<WordVec> vecs;
        QStringList tags = all_tags.keys();
        foreach(QString tag, tags)
        {
            vecs.append(tf_idf->stringToVector(tag.trimmed(), true));
        }
        // 获取每个词语的重要性。 // 只检查正文词语，如果不包含大纲中的词汇，那么就是0，没必要算了
        WordFrq TIs = tf_idf->getTF_IDF(vec.keys(), tags);
        foreach (WordVec vec, vecs)
        {
            tf_idf->addTF_IDF(TIs, vec.keys(), tags);
        }

        foreach (CardBean* card, cards)
        {
            QString card_name = card->name;
            double simi = tf_idf->getVectorSimilarity(vec, tf_idf->stringToVector(card->name), TIs);
//            TGSDEB << "vector simi:" << simi << name << card_name;
            if (simi > 0.0001)
            {
                foreach (QString tag, card->tags)
                {
                    addTagPoss(tag, simi);
                }
            }
        }
    }

    /**
     * 根据开头判断可能性
     */
    void orderByHeader(QString name)
    {
        for (int i = 0; i < cards.size(); i++)
        {
            QString card_name = cards.at(i)->name;
            QStringList card_tags = cards.at(i)->tags;

            int simi_pos = 0;
            while (simi_pos < name.length() && simi_pos < card_name.length() && name.mid(simi_pos, 1) == card_name.mid(simi_pos, 1) )
                simi_pos++;
//            TGSDEB << "比较：" << simi_pos << name << card_name;
            if (simi_pos == 0) // 没有相似的，直接退出后面的
                continue;
            double poss = simi_pos / (double)(name.length()+card_name.length()-simi_pos);
//            TGSDEB << "header simi:" << poss << name << card_name;
            for (int j = 0; j < card_tags.size(); j++)
            {
                addTagPoss(card_tags.at(j), poss);
            }
        }
    }

    /**
     * 根据结尾进行判断可能性
     */
    void orderByTailer(QString name)
    {
        for (int i = 0; i < cards.size(); i++)
        {
            QString card_name = cards.at(i)->name;
            QStringList card_tags = cards.at(i)->tags;

            int simi_pos = 0;
            while (simi_pos < name.length() && simi_pos < card_name.length() && name.mid(name.length()-simi_pos-1, 1) == card_name.mid(card_name.length()-simi_pos-1, 1) )
                simi_pos++;
//            TGSDEB << "比较：" << simi_pos << name << card_name;
            if (simi_pos == 0) // 没有相似的，直接退出后面的
                continue;
            double poss = simi_pos / (double)(name.length()+card_name.length()-simi_pos);
//            TGSDEB << "tailer simi:" << poss << name << card_name;
            for (int j = 0; j < card_tags.size(); j++)
            {
                addTagPoss(card_tags.at(j), poss);
            }
        }
    }

    /**
     * 添加一个tag的可能性
     */
    void addTagPoss(QString tag, double poss)
    {
        TGSDEB << "addTagPoss" << tag << poss;
        if (tag_posss.contains(tag))
            tag_posss[tag] += poss;
        else
            tag_posss.insert(tag, poss);
    }

    QList<QPair<QString,double>> orderByPoss(QStringList& select_list, bool auto_select)
    {
        QList<QString>order_poss_strs;
        QList<double>order_poss_vals;
        QList<bool>selecteds;
        TGSDEB << "排序前" << tag_posss;
        for (QHash<QString,double>::iterator i = tag_posss.begin(); i != tag_posss.end(); ++i)
        {
            QString s = i.key();
            double p = i.value();

            // 按照可能性从高到低二分插入排序
            if (order_poss_vals.size() == 0) // 要用元素进行判断，所以没有元素时要进行特判
            {
                order_poss_strs.append(s);
                order_poss_vals.append(p);
            }
            else
            {
                int left = 0, right = order_poss_strs.size(), mid = (left+right)/2;
                while (left < right) // 确定相等或者之间的位置
                {
                    mid = (left+right)/2;
                    if (order_poss_vals.at(mid) > mid + DOUBLEIGNORE)
                    {
                        right = mid-1;
                    }
                    else  if (order_poss_vals.at(mid) < mid - DOUBLEIGNORE)
                    {
                        left = mid+1;
                    }
                    else
                        break;
                }
                if (order_poss_vals.at(mid) < p) // 插入到前方
                {
                    order_poss_strs.insert(mid, s);
                    order_poss_vals.insert(mid, p);
                }
                else
                {
                    order_poss_strs.insert(mid+1, s);
                    order_poss_vals.insert(mid+1, p);
                }
            }
            selecteds.append(false);
        }
        TGSDEB << "排序后键：" << order_poss_strs;
        TGSDEB << "排序后值：" << order_poss_vals;

        // 针对可能性，添加对应的标签
        // 算法：从大的开始往后，判断是否有可以一起的，若有，则继续选中下面这个
        // 如果遇到可能性骤然降低一半的，也截止
        if (auto_select)
        {
            QList<QStringList> tags_lists; // 把所有的可能性标签组列出来
            foreach (CardBean* card, cards)
            {
                if (tags_lists.contains(card->tags))
                    continue;
                tags_lists.append(card->tags);
            }
            int index = 0;
            double prev_poss = 0.8; // 0.4 起步
            while (index < order_poss_strs.size())
            {
                QString tag = order_poss_strs.at(index);
                double poss = order_poss_vals.at(index);
                index++;
                if (poss < prev_poss / 2) // 可能性直降一半
                    break;
                // 判断能否有一起的
                bool together = false;
                for (int i = 0; i < tags_lists.size(); i++)
                {
                    if (tags_lists.at(i).contains(tag, Qt::CaseInsensitive))
                    {
                        together = true;
                    }
                }

                if (!together)
                    continue;

                // 可以在一起，剔除不包含本标签的组
                for (int i = 0; i < tags_lists.size(); i++)
                {
                    if (!tags_lists.at(i).contains(tag, Qt::CaseInsensitive))
                    {
                        tags_lists.removeAt(i--);
                    }
                }
                select_list.append(tag);
                prev_poss = poss;
            }
            TGSDEB << "选中结果：" << select_list;
        }

        TGSDEB << "排序结束";

        QList<QPair<QString,double>> result;
        for (int i = 0; i < order_poss_strs.size(); i++)
            result.append(QPair<QString,double>(order_poss_strs.at(i), order_poss_vals.at(i)));
        return result;
    }

    QList<QPair<QString,int>> orderByCount()
    {
        QList<QString>order_count_strs;
        QList<int>order_count_vals;
        TGSDEB << "排序前" << all_tags;
        for (QHash<QString, int>::iterator i = all_tags.begin(); i != all_tags.end(); ++i)
        {
            QString tag = i.key();
            int count = i.value();
            if (tag_posss.contains(tag)) // 已经在可能性的那组了
                continue;

            // 按照出现次数从大到小排序
            if (order_count_vals.size() == 0) // 要用元素进行判断，所以没有元素时要进行特判
            {
                order_count_strs.append(tag);
                order_count_vals.append(count);
            }
            else
            {
                int left = 0, right = order_count_strs.size(), mid = (left+right)/2;
                while (left < right) // 确定相等或者之间的位置
                {
                    mid = (left+right)/2;
                    if (order_count_vals.at(mid) > mid + DOUBLEIGNORE)
                    {
                        right = mid-1;
                    }
                    else  if (order_count_vals.at(mid) < mid - DOUBLEIGNORE)
                    {
                        left = mid+1;
                    }
                    else
                        break;
                }
                if (order_count_vals.at(mid) < count) // 插入到前方
                {
                    order_count_strs.insert(mid, tag);
                    order_count_vals.insert(mid, count);
                }
                else
                {
                    order_count_strs.insert(mid+1, tag);
                    order_count_vals.insert(mid+1, count);
                }
            }
        }
        TGSDEB << "排序后键：" << order_count_strs;
        TGSDEB << "排序后值：" << order_count_vals;

        QList<QPair<QString,int>> result;
        for (int i = 0; i < order_count_strs.size(); i++)
            result.append(QPair<QString,int>(order_count_strs.at(i), order_count_vals.at(i)));
        return result;
    }

    QHash<QString,QColor> getTagsColor()
    {
        QHash<QString,QColor> result;
        QStringList tags = all_tags.keys();
        foreach (QString tag_name, tags)
        {
            // 根据标签名，获取改名字数量最多的颜色
            QHash<int, int> counts;
            int max = 0;
            QColor max_color = us->global_font_color;
            foreach (CardBean* card, cards)
            {
                if (card->tags.contains(tag_name))
                {
                    QColor c = card->color;
                    int c_val = c.value();
                    if (counts.contains(c_val))
                    {
                        counts[c_val]++;
                        if (counts[c_val] > max)
                        {
                            max = counts[c_val];
                            max_color = c;
                        }
                    }
                    else
                        counts.insert(c_val, 1);
                }
            }
            result.insert(tag_name, max_color);
        }
        return result;
    }

private:
    Cardlib* cardlib;
    LocateMultiple* tf_idf;
    CardList cards;
    QHash<QString, int> all_tags;
    QHash<QString, double> tag_posss;
};

#endif // CARDLIBTAGSAI_H
