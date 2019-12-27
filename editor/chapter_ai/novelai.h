#ifndef NOVELAI_H
#define NOVELAI_H

#include <QString>
#include <QTextCursor>
#include <QCursor>
#include <QTextEdit>
#include <QScrollBar>
#include <QRegExp>
#include <QStringList>
#include "novelaibase.h"
#include "pinyinutil.h"
#include "selectionitemlist.h"
#include "editorinputmanager.h"
#include "myhightlighter.h"
#include "editinginfo.h"

#define INPUT_SHRESHOLD 20 // 超过20字则判断为粘贴

/**
 * 小说AI操作类
 */
class NovelAI : public NovelAIBase
{
public:
    NovelAI();

    void initAnalyze(QTextEdit* edit);
    void textAnalyze();  // 文本改变时进行传值和初始化
    void isNotInput();   // 非用户输入标记
    void isNotMove();
    bool isMove();

    // 调用 NovelAIBase 的函数
    void activeSmartQuotes();
    void activeSmartQuotes2();
    void activeSmartSpace();
    void activeSmartEnter();
    void activeSmartBackspace();
    void activeNormalEnterInsertByFirstPara();
    void activeRetrevEnter();
    void activeNormalEnter();
    void activeSentFinish();
    void activeExpandSelection();
    void activeShrinkSelection();

    // 自身 AI 函数
    void operatorAddPrevPara();
    void operatorAddNextPara();
    void operatorTypeset();
    void operatorTypesetPaste(int x, QString text);
    void operatorTypesetPart(int start, int end);
    bool activeParaSplit(int x = -1);
    bool operatorParaSplit(int x, bool indent);
    void operatorSmartDelete();
    void operatorWordReplace(int start, int end, QString word);
    void operatorExpandSelection(int start, int end, int &callback_start, int &callback_end);
    void operatorShrinkSelection(int start, int end, int &callback_start, int &callback_end);
    bool operatorTabComplete();
    void operatorTabSkip(int has_changed);
    void operatorReverseTabSkip();
    bool activeHomonymCover();
    bool operatorHomonymCover(int end_pos, int diff);
    bool activeHomonymCoverForce();
    bool operatorHomonymCoverForce(int start, QString text);
    bool operatorPuncCover();      // 标点覆盖：输入特定标点触发
    bool operatorPairMatch();      // 括号匹配：左半符号自动触发
    bool operatorPairJump();       // 括号跳转：右半括号跳转到右边
    void operatorHomeKey(bool shift = false);  // 智能Home键
    bool operatorSmartKeyUp(bool shift = false);      // 智能上一段
    bool operatorSmartKeyDown(bool shift = false);    // 智能下一段
    bool operatorMultiReplace(QStringList old_words, QStringList new_words);
    void operatorMultiEliminate(QStringList words);

    // 获取文本与光标事件
    int     getPreWordPos(int pos);                               // 上一个单词的位置
    int     getNextWordPos(int pos);                              // 下一个单词的位置
    int     getWordCount(QString text);                           // 字数统计
    int     getWordCount(QString text, int& ch, int& num, int& en, int& punc, int& blank, int& line);
    QString getWord(QString text, int pos, int& start, int& end); // 选择单词
    QString getSent(QString text, int pos, int& start, int& end); // 选择短句
    QString getSent2(QString text, int pos, int& start, int& end);// 选择长句
    QString getSent3(QString text, int pos, int& start, int& end);// 选择最近句正文（不包括标点）
    QString getPair(QString text, int pos, int& start, int& end); // 选择成对符号（如果没有则返回空）
    QString getPara(QString text, int pos, int& start, int& end); // 选择段落

    // 自身光标处理事件
    void backspaceWord(int x = -1);
    void deleteWord(int x = -1);
    void backspaceSent(int x = -1);
    void deleteSent(int x = -1);
    void backspacePara(int x = -1);
    void deletePara(int x = -1);
    void moveToPreWord(int x);
    void moveToNextWord(int x);
    void moveToSentStart(int pos);
    void moveToSentEnd(int pos);
    void moveToParaStart(int x);
    void moveToParaStart2(int x); // 换行后面，或者0
    void moveToParaEnd(int x);
    void moveToParaPrevStart(int x);
    void moveToParaPrevEnd(int x);
    void moveToParaNextStart(int x);
    void moveToParaNextEnd(int x);
    void moveCursorFinished(int pos);

protected:
    virtual void recorderOperator();
    virtual void updateRecorderPosition(int x);
    virtual void updateRecorderScroll();
    virtual void updateRecorderSelection();
    virtual void cutRecorderPosition();
    void onlyDeleteText(int start, int end);
    virtual void scrollToEditEnd();
    virtual void scrollCursorFix();
    virtual void cursorRealChanged(int pos = -1);

    bool isAtEditStart();
    bool isAtEditEnd(); // 是否在编辑最后一行正文（无视空白符）
    bool isAtBlankLine();
    bool isAtBlankLine2();
    bool isAtParaEnd();
    void makeInputAnimation();
    virtual void setTextInputColor(); // 设置输入动画的字体颜色

    bool prepareAnalyze(); // 准备分析，预处理
    void finishAnalyze();  // 结束修改，设置新的文本或光标
    void moveCursor(int x);                 // 移动光标
    void insertText(int pos, QString text); // 插入文本
    void insertText(QString text);          // 插入文本
    void deleteText(int start, int end);    // 删除文本
    void setSelection(int start, int end, int pos = -1); // 设置光标

protected:
    // 文件交互
    EditingInfo editing;
    QTextEdit *_edit;

    QTextCursor _text_cursor;
    int _scroll_pos;
    bool _flag_is_not_move;
    bool _flag_is_not_scrolled;
    bool _change_text, _change_pos; // 标记是否改变
    bool is_editing_end, is_editing_end_backup;

    int pre_scroll_position, pre_scroll_bottom_delta;
    int pre_scroll_bottom_delta_by_analyze; // 文字改变前的位置，避免文字改变导致的变化
    int pre_scroll_viewpoer_top_by_analyze, pre_scroll_viewport_top;
    SelectionItemList selection_list;

    EditorInputManager* input_manager;
    bool _flag_color_changing; // 颜色改变期间无视任何 textchange
    bool _flag_is_cursor_change; // 是 setTextCursor 改变引起的 textChange 事件
    bool _flag_is_not_selection;
    int edit_range_start, edit_range_end;

    // 高亮
    MyHightlighter* highlighter;

    bool _flag_is_analyzing;
    int _input_chinese_pos; // 输入的位置
    QString _input_chinese_text; // 输入的文字（仅限中文）
    bool _input_chinese_buffer; // 是否输入中文
    int key_pressed_pos;    // 按下按下时的光标位置

private:
    int _pre_changed_pos; // 上一次改变的光标位置
    bool _flag_is_not_input;  // 非用户手动输入标记，如果为true，则本次不分析文本，并且置为false
};

#endif // NOVELAI_H
