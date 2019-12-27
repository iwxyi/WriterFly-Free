#include "locatemutiple.h"

/************************************************************************
 * 从章节定位到故事线的静态函数，返回段落的首尾位置
 ************************************************************************/
void LocateMultiple::locateOutlineByChapter(SegmentTool* jieba, const QStringList& stop_words,
    const QString& artical, const QString& outline, int& start, int& end)
{
    QStringList paras = outline.split("\n", QString::SkipEmptyParts);
    OUTDBG << QStringLiteral("正文长度:") << artical.length();
    OUTDBG << QStringLiteral("段落数量:") << paras.size();
    int max_simi_index = locateOneInMul(jieba, stop_words, artical, paras);
    if (max_simi_index > -1)
    {
        QString para = paras.at(max_simi_index);
        start = outline.indexOf(para);
        end = start + para.length();
        OUTDBG << QStringLiteral("最终结果：") << para;
    }
}

/************************************************************************
 * 从多个定位到一个，并返回最适配的段落下标
 ************************************************************************/
int LocateMultiple::locateOneInMul(SegmentTool * jieba, const QStringList & stop_words,
    const QString & one, const QStringList & mul_list)
{
    // 段落文本转向量
    LocateMultiple pl(jieba, stop_words);
    WordVec artical_vector = pl.stringToVector(one);

    // 每个分段转换为向量
    QList<WordVec> mul_vectors;
    foreach(QString mul, mul_list)
    {
        mul_vectors.append(pl.stringToVector(mul.trimmed(), true));
    }

    // 获取每个词语的重要性。 // 只检查正文词语，如果不包含大纲中的词汇，那么就是0，没必要算了
    WordFrq TIs = pl.getTF_IDF(artical_vector.keys(), mul_list);
    foreach (WordVec vec, mul_vectors)
    {
        pl.addTF_IDF(TIs, vec.keys(), mul_list);
    }
    ALLDBG << "---------------------------";
    ALLDBG << TIs;
    ALLDBG << "---------------------------";

    // 遍历每个分段，获取相似度最大的一段
    double max_simi = 0; // 最大相似度
    int max_simi_index = -1; // 最大相似度的位置
    for (int i = 0; i < mul_vectors.size(); i++)
    {
        double simi = pl.getVectorSimilarity(artical_vector, mul_vectors.at(i), TIs);
        if (simi > max_simi)
        {
            max_simi = simi;
            max_simi_index = i;
        }
    }
    return max_simi_index;
}

/************************************************************************
 * 构造函数
 ************************************************************************/
LocateMultiple::LocateMultiple(SegmentTool*& jieba, const QStringList& stop_words) :
    jieba(jieba), stop_words(stop_words), full_word_count(0)
{
}

/************************************************************************
 * 将字符串转换成词语词组
 ************************************************************************/
QStringList LocateMultiple::stringToArray(const QString& text, bool stst)
{
    QStringList words = jieba->WordSegment(text);
    if (stst)
        full_word_count += stst;
    for (int i = 0; i < words.size(); i++) // 遍历章节词组的每一个
    {
        if (stop_words.contains(words.at(i)))
        {
            words.removeAt(i--);
        }
    }
    return words;
}

/************************************************************************
 * 将字符串转换成词语向量
 * map 格式：["名称" => 出现次数]
 ************************************************************************/
WordVec LocateMultiple::stringToVector(const QString& text, bool stst)
{
    WordVec map;
    WordVec::iterator it;
    QStringList list = stringToArray(text, stst);
    foreach(QString word, list)
    {
        if (word.trimmed().isEmpty())
            continue; // 去除空白
        if ((it = map.find(word)) != map.end()) // 如果这个词语存在，值+1
        {
            (*it)++;
        }
        else // 如果这个词语不存在，添加并设置值为1
        {
            map.insert(word, 1);
        }
    }

    return map;
}


/************************************************************************
 * 获取某一个词的重要程度（出现次数越大、而包含的文档数越少，越重要）
 * TF-IDF = 计算的词频(TF)*计算的反文档频率(IDF)
 ************************************************************************/
WordFrq LocateMultiple::getTF_IDF(const QList<QString> words, const QStringList& list)
{
    WordFrq TIs;
    QString words_string = words.join(" ");
    foreach (QString word, words)
    {
//        TIs.insert(word, getTF(word, list, full_word_count) * getIDF(word, list, full_word_count));
        TIs.insert(word, getTF(word, words_string, words.size()) * getIDF(word, list, full_word_count));
    }
    return TIs;
}

/************************************************************************
 * 在原来的数据基础上，添加其他的词重要程度（针对list）
 ************************************************************************/
void LocateMultiple::addTF_IDF(WordFrq & TIs, const QList<QString> words, const QStringList & list)
{
    QString words_string = words.join(" ");
    foreach (QString word, words)
    {
        if (TIs.contains(word)) // 已经存在这个词了
            continue;
//        TIs.insert(word, getTF(word, list, full_word_count) * getIDF(word, list, full_word_count));
        TIs.insert(word, getTF(word, words_string, words.size()) * getIDF(word, list, full_word_count));
    }
}

/************************************************************************
 * 获取被搜索文章中的各词的词频（TF）
 * 词频 = 某个词在文章中出现的次数 / 文章的总词数
 ************************************************************************/
double LocateMultiple::getTF(const QString & s, const QString & string, int full_word_count)
{
    int appear_count = getStringAppearCount(s, string);
    return (double)appear_count / full_word_count;
}

/************************************************************************
 * 获取翻文档频率（IDF）
 * 反文档频率 = log( 语料库的文章总数 / (包含该词的文档数+1) )
 ************************************************************************/
double LocateMultiple::getIDF(const QString & s, const QStringList & list, int full_word_count)
{
    int contain_count = getStringContainCount(s, list);
    return log((double)list.count() / (contain_count+1));
}

/************************************************************************
 * 获取字符串在长文本中出现的次数
 ************************************************************************/
int LocateMultiple::getStringAppearCount(const QString & s, const QString & full)
{
    int count = 0, len = s.length(), find = -len;
    while ((find = full.indexOf(s, find + len)) != -1)
    {
        count++;
    }
    return count;
}

/************************************************************************
 * 获取字符串在多个长文本中出现的总次数
 ************************************************************************/
int LocateMultiple::getStringAppearCount(const QString & s, const QStringList & full)
{
    int count = 0;
    foreach (QString f, full)
    {
        count += getStringAppearCount(s, f);
    }
    return count;
}

/************************************************************************
 * 计算数组中含有某个字符串的总数
 ************************************************************************/
int LocateMultiple::getStringContainCount(const QString & s, const QStringList & full)
{
    int count = 0;
    foreach (QString f, full)
    {
        if (f.contains(s))
            count++;
    }
    return count;
}

/************************************************************************
 * 单纯计算两个向量之间的相似度
 * 根据TF-IDF（词频-反文档频率）计算两个向量之间的相似度
 ************************************************************************/
double LocateMultiple::getVectorSimilarity(const WordVec& a, const WordVec& b, const WordFrq frq)
{
    // 将 a 和 b 统一词包
    QList<int> v1, v2;
    QStringList word_orders;
    for (WordVec::const_iterator it = a.begin(); it != a.end(); it++)
    {
        v1.append(it.value());
        v2.append(b.contains(it.key()) ? b[it.key()] : 0);
        word_orders.append(it.key());
    }
    for (WordVec::const_iterator it = b.begin(); it != b.end(); it++)
    {
        if (a.contains(it.key())) // 这个词语已经在前面出现过了
            continue;
        v1.append(0);
        v2.append(it.value());
        word_orders.append(it.key());
    }

    PERDBG << b.keys();
    ALLDBG << word_orders;
    ALLDBG << v1;
    ALLDBG << v2;

    // 如果不在里面，则重要性是最弱的一半
    double importance_none = frq.size()?frq.first():0;
    foreach (double d, frq.values())
        if (importance_none > d/2)
            importance_none = d/2;

    // 计算向量余弦值
    double numerator = 0, denominator1 = 0, denominator2 = 0; // 分子和分母
    for (int i = 0; i < v1.size(); i++)
    {
        QString word = word_orders.at(i);
        double importance = frq.contains(word) ? frq[word] : importance_none;
        importance *= importance; // 平方值（后面开根号）
        numerator += v1.at(i) * v2.at(i) * importance;
        denominator1 += v1.at(i) * v1.at(i) * importance;
        denominator2 += v2.at(i) * v2.at(i) * importance;
    }
    double ans = numerator / sqrt(denominator1 * denominator2);
    PERDBG << "ans:" << ans;
    //PERDBG <<"\t\t"<< numerator<<" / "<<QStringLiteral("√(")<<denominator1<<"*"<<denominator2<<")";
    PERDBG << "-------------------";

    return ans;
}

/************************************************************************
 * 单纯计算两个向量之间的相似度
 * 不使用 TF-IDF 算法
 ************************************************************************/
double LocateMultiple::getVectorSimilarity(const WordVec &a, const WordVec &b)
{
    // 将 a 和 b 统一词包
    QList<int> v1, v2;
    QStringList word_orders;
    for (WordVec::const_iterator it = a.begin(); it != a.end(); it++)
    {
        v1.append(it.value());
        v2.append(b.contains(it.key()) ? b[it.key()] : 0);
        word_orders.append(it.key());
    }
    for (WordVec::const_iterator it = b.begin(); it != b.end(); it++)
    {
        if (a.contains(it.key())) // 这个词语已经在前面出现过了
            continue;
        v1.append(0);
        v2.append(it.value());
        word_orders.append(it.key());
    }

    PERDBG << b.keys();
    ALLDBG << word_orders;
    ALLDBG << v1;
    ALLDBG << v2;

    // 计算向量余弦值
    double numerator = 0, denominator1 = 0, denominator2 = 0; // 分子和分母
    for (int i = 0; i < v1.size(); i++)
    {
        QString word = word_orders.at(i);
        numerator += v1.at(i) * v2.at(i);
        denominator1 += v1.at(i) * v1.at(i);
        denominator2 += v2.at(i) * v2.at(i);
    }
    double ans = numerator / sqrt(denominator1 * denominator2);
    PERDBG << "ans:" << ans;
    //PERDBG <<"\t\t"<< numerator<<" / "<<QStringLiteral("√(")<<denominator1<<"*"<<denominator2<<")";
    PERDBG << "-------------------";

    return ans;
}
