#ifndef CARDBEAN_H
#define CARDBEAN_H

#include <QStringList>
#include <QColor>

/**
 * 一个名片的Bean
 * 存储名片的所有信息
 */

struct CardBean
{
    QString id;
    QString name;        // 名片名字
    QString pat;         // 匹配表达式
    int ctype;           // 颜色种类：0自定义，其余整数表示对应下标
    QColor color;        // 颜色值
    bool disabled;       // 禁用标记
    bool used;           // 最近正在被使用（提升性能）
    QStringList tags;    // 标签
    QString brief;       // 简单介绍（出现在名片上）
    QString detail;      // 全部介绍
    qint64 time = 0;     // 上次用到的时间戳（非遍历时使用）
    int sort_data = 0;   // 用户数据
    bool expand = false; // 卡片控件展开

    /**
     * 返回正则表达式
     * 本来应该弄一个缓冲正则对象，但是嫌麻烦……
     */
    QString pattern()
    {
        return pat.isEmpty() ? name : pat;
    }
};

#endif // CARDBEAN_H
