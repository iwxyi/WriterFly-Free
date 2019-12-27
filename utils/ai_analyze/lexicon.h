#ifndef LEXICON_H
#define LEXICON_H

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QDebug>
#include <QTime>
#include <QList>
#include <QRegExp>
#include <QtConcurrent/QtConcurrent>
#include <QRegularExpression>
#include "fileutil.h"
#include "usettings.h"

#define COMPLETER_CONTENT_NONE 0 // 无
#define COMPLETER_CONTENT_WORD 1 // 词语
#define COMPLETER_CONTENT_SENT 2 // 句子
#define COMPLETER_CONTENT_RAND_LIST 3 // 随机种类列表
#define COMPLETER_CONTENT_SURN 4 // 姓氏
#define COMPLETER_CONTENT_NAME 5 // 名字
#define COMPLETER_CONTENT_RAND 6 // 随机具体
#define COMPLETER_PREDICT_NEXT 11 // 随机下一句
#define COMPLETER_PREDICT_SIMILAR 12 // 当前相似句

/**
 * 词库操作工具类
 */
class Lexicons
{
public:
    Lexicons(QString path);

    friend class SearchPanel; // 搜索界面直接访问私有成员变量
    friend class UserSettingsWindow; // 设置界面直接初始化私有成员变量

    void setUS(USettings *us);
    void initAll(bool syno, bool rela, bool sent, bool random);
    void uninitAll(bool syno, bool rela, bool sent, bool random); // 取消初始化所有

    void initSynonym();  // 初始化近义词
    void initRelated();  // 初始化相关词词库
    void initSentence(); // 初始化句子词库
    void initRandomName(); // 初始化随机词库
    void initSensitive();  // 敏感词
    void initStopWords();  // 停用词
    void initDictionary(); // 汉语词典
    void loadErrorWords();

    void clear();
    bool surroundSearch(QString sent, int cursor/*相对*/); // 包含光标前四个字符、后三个字母，长度每四三二都搜索过去
    bool search(QString key, bool add = false);
    bool searchRandom(QString key);
    QStringList getResult();
    QString getMatched();
    int getMatchedCase();
    void setMatchedCase(int x);
    QStringList getSentence();
    QStringList getSentence(QString key);
    QRegularExpression getSensitivePattern();
    QList<QRegularExpression> getErrorWordPatterns();
    QStringList& getSensitiveWords();
    QStringList& getStopWords();
    bool isStopWord(QString s);
    bool isSplitWord(QString c);

private:
    bool shouldRandom();

private:
    QString lexicon_dir;

    bool synonym_inited, related_inited, sentence_inited, surname_inited, name_inited, random_inited, sensitive_inited, dictionary_inited;
    QString synonym_text, related_text, sentence_text, surname_text, name_text;
    QStringList random_sort_list, random_text_list;
    QStringList stop_words;
    QString sensitive_text;
    QStringList sensitive_list;
    QRegularExpression sensitive_pattern;
    QList<QRegularExpression> error_word_patterns;
    QString dictionary_dir, dictionary_text;

    QString search_last, matched_key;
    QStringList search_result;
    int matched_case;
    bool match_sentence;

    USettings* us;
};

#endif // LEXICON_H
