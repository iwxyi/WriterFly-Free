#ifndef PREDICTBOOKS_H
#define PREDICTBOOKS_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>
#include "fileutil.h"
#include "segmenttool.h"
#include "locatemutiple.h"

#define PDTDEB if ( 0) qDebug()
#define _PDTTEST if ( 0) qDebug()

class PredictCompleter : public QObject
{
    Q_OBJECT
public:
    PredictCompleter(QObject* parent = nullptr): QObject(parent), valid(true)
    {
    }
    PredictCompleter(int pos, QObject* parent = nullptr): QObject(parent), pos(pos), valid(true)
    {
    }

    void disable()
    {
        valid = false;
    }

    bool isValid()
    {
        return valid;
    }

    int getPosition()
    {
        return pos;
    }

signals:
    void signalPredictSentence(QString similar, QString next);
    void signalSimilarSentence(QString similar);
    void signalPredictResult(QStringList list);

private:
    bool valid;
    int pos;
};

class PredictBooks : public QObject
{
    Q_OBJECT
public:
    PredictBooks(const QStringList& stop_words, QString books_dir, QObject *parent = nullptr);

    void setSegment(SegmentTool* segment);

    void loadByType(QString type);
    void loadByType(QStringList types);

    bool isIniting();
    void startPredict(QString para, PredictCompleter* completer);
    void startSimilar(QString para, PredictCompleter* completer);

private:
    void readByType(QString type);

    bool splited();
    void runSplit();
    QStringList getNextSentence(QString para);
    QStringList getSimiSentence(QString para);
    QStringList getRandomBookSentList();

signals:

public slots:

private:
    SegmentTool* jieba;    // Jieba 分词工具
    QStringList stop_words; // 停用词（这里需要加上比较专业的特殊停用词）

    QMap<QString, QList<QString> >type_books;
//    QMap<QString, QList<QList<QList<QString> > > > para_splits;
//    QMap<QString, QList<QList<QList<QList<QString> > > > > sent_splits;

    QList<WordVec>books_para_vec_list; // 按段落分的词频
    QList<QString>books_para_str_list; // 按词语分的整段
    QList<WordVec>books_sent_vec_list; // 按词语分的词频
    QList<QString>books_sent_str_list; // 按词语分的整句
    int full_word_count;

    QString books_dir_path;
    bool initing;

    // PredictCompleter* completer;
    double min_similar; // 最小允许的匹配度
};

#endif // PREDICTBOOKS_H
