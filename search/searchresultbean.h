#ifndef SEARCHRESULTBEAN_H
#define SEARCHRESULTBEAN_H

#include <QString>
#include <QList>

enum ResultType
{
    NONE,               // 未知
    CURRENT_CHAPTER,    // 当前章节
    OPENED_CHAPTERS,    // 打开的章节
    NOVEL_CHAPTERS,     // 本书章节
    ALL_CHAPTERS,       // 全书章节
    SYNONYMS,           // 同义词
    RELATEDS,           // 相关词
    SENTENCES,          // 常用句
    RANDOM_SORTS,       // 随机种类
    RANDOMS,            // 随机
    SURNAMES,           // 姓氏
    NAMES,              // 名字
    HELP,               // 帮助
    COMMAND,            // 命令
    RETURN,             // 返回上一级
    LIST,               // 列表
    CHAPTER_NAME,       // 章节名
    ROLL_NAME,          // 分卷名
    NOVEL_NAME,         // 小说名
    NOVEL_CHAPTER_NAME, // 其他作品章节名
    NOVEL_ROLL_NAME,    // 其他作品分卷名
    DICTIONARY_ITEM,    // 汉语词典
    IDIOMS_SOLITAIRE,   // 成语接龙
    CARDLIB,            // 名片
    CARDLIB_NAME,       // 名片名字
    CARDLIB_TAG,        // 名片标签
    CARDLIB_CONTENT,    // 名片内容
};

struct SearchResultBean
{

    SearchResultBean(QString text, ResultType type)
    {
        this->text = text;
        this->type = type;
    }
    SearchResultBean(QString text, QString data, ResultType type)
    {
        this->text = text;
        this->data = data;
        this->type = type;
    }

    QString getText() const /* 要声明为 const 才能在 model 中用 */
    {
        if (data.isEmpty())
            return text;
        return data;
    }

    ResultType type;
    QString text;
    QString data;
};

#endif // SEARCHRESULTBEAN_H
