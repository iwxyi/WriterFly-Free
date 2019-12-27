#ifndef NOVELAIBASE_H
#define NOVELAIBASE_H

#include <QString>
#include <functional>
#include "globalvar.h"
#include "defines.h"

/**
 * 小说语气识别的AI类
 */
class NovelAIBase
{
public:
    NovelAIBase();
    virtual ~NovelAIBase() = 0;

protected:
    void operatorSmartSpace();     // 智能空格：按下空格触发
    void operatorSmartEnter();     // 智能回车：按下回车触发
    void operatorSmartQuotes();    // 智能引号：按下引号触发
    void operatorSmartBackspace(); // 智能删除，按下删除触发
    bool operatorAutoPunc();       // 自动标点：语气词自动触发
    bool operatorAutoDqm();        // 自动双引号：语言动词触发
    bool operatorSentFinish();     // 句末标点：增加或转化成结尾
    void operatorSmartQuotes2(int left, int right); // 智能引号，仅在选中文本的情况下调用
    virtual int getWordCount(QString str);  // 字数统计，交给 NovelAI

    QString getPunc(QString para/*段落*/, int pos/*光标*/);  // ☆核心：取标点(句子)
    QString getPunc(/*QString fullText, int pos*/);         // 这个是全部文本中的某一部分
    QString getPunc(int pos);  // 全部文本，特定位置的标点
    QString getPunc2(int pos); // 把"，"改成"。"
    QString getPunc2();        // 把"，"改成"。"
    int     getDescTone(QString sent); // 句子语气标点，影响语气导向
    QString getTalkTone(QString sent, QString sent2, int tone, QString left1, QString left2, bool in_quote);

    bool isChinese(QString str);              // 是否为汉字（多个字符仅取第一个）
    bool isEnglish(QString str);              // 是否为英文（多个字符仅取第一个）
    bool isNumber(QString str);               // 是否为数字（多个字符仅取第一个）
    bool isKnowFormat(QString str);           // 是否为知道系列（我知道他为什么）
    bool canRegExp(QString str, QString pat); // 能否匹配（一部分）
    bool isAllChinese(QString str);           // 是否全部为汉字

    void updateCursorChars();                           // 修改光标附近的字符
    bool canDeletePairPunc();                           // 是否能够删除成对文本（能删就删）
    bool isCursorInQuote(QString text, int pos);        // 是否在引号里面
    virtual void moveCursor(int x) = 0;                 // 移动光标
    virtual void insertText(int pos, QString text) = 0; // 插入文本
    virtual void insertText(QString text) = 0;          // 插入文本
    virtual void deleteText(int start, int end) = 0;    // 删除文本

    // bool isNextLang();              // 是否是后一句话（双引号前面多的是逗号）
    bool isSentPunc(QString str);      // 是否为句末标点（不包含引号和特殊字符，不包括逗号）
    bool isSentSplitPunc(QString str); // 是否为句子分割标点（包含逗号）
    bool isSentSplit(QString str);     // 是否为句子分割符（各类标点，包括逗号）
    bool isASCIIPunc(QString str);     // 是否为英文标点（不包含引号和特殊字符）
    bool isBlankChar(QString str);     // 是否为空白符
    bool isBlankChar2(QString str);    // 是否为换行之外的空白符
    bool isBlankString(QString str);   // 是否为空白字符串
    bool isSymPairLeft(QString str);   // 是否为对称标点左边的
    bool isSymPairRight(QString str);  // 是否为对称标点右边的
    QString getSymPairLeftByRight(QString str); // 根据右边括号获取左边的括号
    QString getSymPairRightByLeft(QString str); // 根据右边括号获取左边的括号
    QString getCursorFrontSent();      // 获取当前面的句子
    QString getCurrentChar(int x);     // 获取当前位置的附近汉字
    bool isQuoteColon(QString str);    // 汉字后面是否需要加标点

private:
    void initConstString();            // 初始化常量字符串

protected:
    QString _text, _pre_text;  // 文本
    int _pos, _pre_pos, _dif;  // 光标位置和字数差
    QString _left1, _left2, _left3, _right1, _right2; // 光标附近的文本
    bool isInQuotes;           // 是否在引号里面（用来判断是否为语言或者描述）

private:
    QString _shuo_blacklists, _dao_whitelists, _wen_blacklists;     // “说”黑名单、“道”白名单、“问”黑名单
    QString _symbol_pair_lefts, _symbol_pair_rights; // 成对符号左边/右边
    QString _quote_no_colon, _quantifiers; // 引号前面没有冒号
    QString _sent_puncs, _sent_split, _sent_split_puncs, _blank_chars; // 句末标点、句子分割符、空白符
    QString _auto_punc_whitelists; // 自动标点白名单
};

#endif // NOVELAIBASE_H
