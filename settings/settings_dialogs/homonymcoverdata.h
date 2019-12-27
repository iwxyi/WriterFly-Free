#ifndef HOMONYMCOVERDATA_H
#define HOMONYMCOVERDATA_H

#include <QStringList>
#include "fileutil.h"
#include "stringutil.h"

class HomonymCoverData
{
public:
    QString file_path;      // 存储的文件路径
    QStringList black_list; // 黑名单
    QStringList white_list; // 白名单

    QString blacklist_chinese_left;  // 黑名单_中文_左边
    QString blacklist_chinese_right; // 黑名单_中文_右边
    QString blacklist_chinese_both;  // 黑名单_中文_两边
    QString blacklist_pinyin_left;   // 黑名单_拼音_左边
    QString blacklist_pinyin_right;  // 黑名单_拼音_右边
    QString blacklist_pinyin_both;   // 黑名单_拼音_两边

    QString whitelist_chinese_left;  // 白名单_中文_左边
    QString whitelist_chinese_right; // 白名单_中文_右边
    QString whitelist_chinese_both;  // 白名单_中文_两边
    QString whitelist_pinyin_left;   // 白名单_拼音_左边
    QString whitelist_pinyin_right;  // 白名单_拼音_右边
    QString whitelist_pinyin_both;   // 白名单_拼音_两边

    HomonymCoverData(QString dir)
    {
        initFromDataDir(dir);
    }

    void initFromDataDir(QString dir)
    {
        this->file_path = dir;
        if (!dir.endsWith("/")) dir += "/";
        file_path = dir+"homonym_cover.txt";
        readFromFile(file_path);
    }

    void setBlack(QString str)
    {
        black_list = str.split("\n", QString::SkipEmptyParts);
        save();
    }

    QString blackToString()
    {
        QString res = "";
        for (int i = 0; i < black_list.size(); i++)
            res += black_list.at(i) + "\n";
        return res;
    }

    /**
     * @brief canCover 黑白名单内判断
     * @param left     旧的文本：中文，或者拼音
     * @param right    新的文本：中文，或者拼音
     * @return         是否应该被覆盖（不做拼音判断）
     */
    bool canCover(QString left, QString right)
    {
        for (int i = 0; i < black_list.size(); i++)
        {
            if (canRegExp(left, black_list.at(i)) || canRegExp(right, black_list.at(i)))
                return false;
        }
        return true;
    }

    static QString getDirFromFile(QString file_path)
    {
        int pos = file_path.lastIndexOf("/", file_path.length()-2);
        if (pos == -1) return file_path;
        return file_path.left(pos+1);
    }

private:

    void save()
    {
        writeToFile(file_path);
    }

    void readFromFile(QString path)
    {
        if (!isFileExist(path)) return ;
        QString str = readTextFile(path);
        QString b = getXml(str, "BLACK");
        QString w = getXml(str, "WHITE");

        black_list = b.split("\n", QString::SkipEmptyParts);
        white_list = w.split("\n", QString::SkipEmptyParts);
    }

    void writeToFile(QString path)
    {
        QString full = "";

        full += "<BLACK>";
        for (int i = 0; i < black_list.size(); i++)
            full += black_list.at(i) + "\n";
        full += "</BLACK>";

        full += "<WHITE>";
        for (int i = 0; i < white_list.size(); i++)
            full += white_list.at(i) + "\n";
        full += "</WHITE>";

        writeTextFile(path, full);
    }
};

#endif // HOMONYMCOVERDATA_H
