#ifndef LOCATEMUTIPLE_H
#define LOCATEMUTIPLE_H

#include <QStringList>
#include <QMap>
#include <QDebug>
#include <QtMath>
#include "segmenttool.h"

#define ALLDBG if (0) qDebug()
#define PERDBG if (0) qDebug()
#define OUTDBG if (0) qDebug()

typedef QMap<QString, int> WordVec;
typedef QMap<QString, double> WordFrq;

class LocateMultiple
{
    friend class PredictBooks;
    friend class CardlibTagsAI;
public:
    static void locateOutlineByChapter(SegmentTool* jieba, const QStringList& stop_words,
        const QString& one, const QString& outline, int& start, int& end);
    static int locateOneInMul(SegmentTool* jieba, const QStringList& stop_words,
        const QString& one, const QStringList& mul_list);

protected:
    LocateMultiple(SegmentTool*& jieba, const QStringList& stop_words);

    QStringList stringToArray(const QString& text, bool stst = false);
    WordVec stringToVector(const QString& text, bool stst = false);

    WordFrq getTF_IDF(const QList<QString> words, const QStringList& list);
    void addTF_IDF(WordFrq& frq, const QList<QString> words, const QStringList& list);
    double getTF(const QString& s, const QString &string, int full_word_count);
    double getIDF(const QString& s, const QStringList& list, int full_word_count);

    int getStringAppearCount(const QString& s, const QString& full);
    int getStringAppearCount(const QString& s, const QStringList& full);
    int getStringContainCount(const QString& s, const QStringList& full);

    double getVectorSimilarity(const WordVec& a, const WordVec& b, const WordFrq tf_idf);
    double getVectorSimilarity(const WordVec& a, const WordVec& b);

private:
    SegmentTool* jieba;    // Jieba 分词工具
    QStringList stop_words; // 停用词（这里需要加上比较专业的特殊停用词）
    int full_word_count;    // list总词数，用来统计词频
};

#endif // LOCATEMUTIPLE_H
