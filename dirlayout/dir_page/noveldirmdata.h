#ifndef NOVELDIRDATA_H
#define NOVELDIRDATA_H

#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include "defines.h"
#include "globalvar.h"
#include "noveldiritem.h"
#include "stringutil.h"
#include "fileutil.h"
#include "noveltools.h"

class NovelDirListView;

/**
 * 小说章节目录的MVD中的模型数据类
 */
class NovelDirMData
{
public:
    NovelDirMData();
    NovelDirMData(QString novel_name);
    virtual ~NovelDirMData(){}
    friend class NovelDirListView; // 声明友元类，使 ListView 也能使用这里的内容

public:
    QString getFullText();
    QString getChapterName(int index);
    QString getFullNameByName(QString name);
    int getRcCount();
    int getRollCount();
    int getListIndexByRoll(int roll_index);
    bool isExistChapter(int roll_index, int chpt_index);
    bool isExistChapter(QString chapter_name/*未加密*/);
    bool isExistRoll(QString roll_name);
    int getRcWordCount(int index) const;
    int getRcCountByRoll(int roll_index);

    virtual void listDataChanged(); // 贼机智的使用覆盖函数来调用派生类的方法
    virtual void changeNameSuccessed(bool is_chapter, QString novel_name, QString old_name, QString new_name);
    virtual void deleteChapterSuccessed(QString novel_name, QString chapter_name);

public slots:
    void readDir(QString novel_name);                 // 读取目录列表
    void saveDir();                                   // 保存目录

    int addRoll(QString new_name);                    // 添加一个卷
    int addChapter(int list_index, QString new_name); // 在末尾添加章节
    int insertChapter(int index, QString new_name);
    int addNextChapter(int index, QString new_name);
    int insertRoll(int index, QString name);
    QString deleteChapter(int list_index);
    QStringList deleteRoll(int list_index);
    void addMutiChapter();
    int addSplitRoll(int index, QString new_name);
    bool cancelRoll(int index);

    bool tryChangeName(int index, QString new_name);  // 手动编辑的修改名字，很重要
    bool exchangeItem(int org, int aim);
    void adjustAllIndex();

    void setChapterProperty(QString name, QString key, QString val);
    void setRollProperty(QString name, QString key, QString val);
    void setRollHide(int index, bool hide);
    void setChapterWc(int index, int wc);
    void setItemProperty(int index, QString key, QString value);
    QString getItemProperty(int index, QString key);

    virtual void gotoReedit();

protected:
    mutable QList<NovelDirItem> cr_list;
    QString novel_name;
    QString dir_path;
    QString full_text;
    QStringList roll_names;  // 编码后的名字列表（实际上）
    QList<int> roll_subs;    // 每一组的章节数量

};

#endif // NOVELDIRDATA_H
