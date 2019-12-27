#ifndef NOVELDIRSETTING_H
#define NOVELDIRSETTING_H

#include "settings.h"
#include "globalvar.h"

/**
 * 目录设置读取与存储类
 */
class NovelDirSettings
{
public:
    NovelDirSettings();
    NovelDirSettings(QString novel_name);

    void readSettings();

public:
    void readSettings(QString novel_name);
    void setVal(QString key, QVariant val);

    void setStartChapterNum(int a);
    void setStartRollNum(int a);
    void setChapterChar(QString a);
    void setRollChar(QString a);
    void setRecountNumByRoll(bool a);
    void setUseArab(bool a);
    void setShowChapterNum(bool a);
    void setShowRollNum(bool a);
    void setShowChapterWords(bool a);
    void setShowChangeTime(bool a);
    void setChapterPreview(bool a);
    void setShowWordCount(bool a);

    Settings* getSettings();
    QString getNovelName();
    QString getChapterChar()  const;
    QString getRollChar()     const;
    int  getChapterStartNum() const;
    int  getRollStartNum()    const;
    bool isShowChapterNum()   const;
    bool isShowRollNum()      const;
    bool isShowChapterWords() const;
    bool isShowChangeTime()   const;
    bool isUseArab()          const;
    bool isRecountNumByRoll() const;
    bool isNoRoll()           const;
    bool isChapterPreview()   const;
    bool isShowWordCount()    const;

private:
    QString novel_name;       // 书籍名称
    Settings* s;

    int start_chapter;        // 起始章序号
    int start_roll;           // 起始卷序号
    QString char_chapter;     // 章类名
    QString char_roll;        // 卷类名
    bool show_chapter_num;    // 显示章序
    bool show_roll_num;       // 显示卷序
    bool show_chapter_words;  // 显示章节字数（影响加载速度）
    bool show_change_time;    // 显示章节时间
    bool chapter_preview;     // 预览章节
    bool use_arab;            // 使用阿利伯数字（否则中文）
    bool recount_num_by_roll; // 每卷重计章序
    bool no_roll;             // 不使用卷（即正文靠正左对齐）
    bool show_word_count;     // 右侧显示字数
};

#endif // NOVELDIRSETTING_H
