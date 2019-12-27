/**
 * 文本操作工具
 */

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QDebug>
#include <QUrl>

#define XML_SPLIT "5YaZ5L2c5aSp5LiL"

QStringList getStrMids(const QString &text, QString l, QString r); // 取中间文本数组
QString getStrMid(const QString &text, QString l, QString r); // 取中间文本2

QString fnEncode(QString text); // 可使用的文件名编码
QString fnDecode(QString text); // 文件名解码

bool canRegExp(const QString &str, const QString &pat); // 能否正则匹配

QString getXml(const QString &str, const QString &pat);
QStringList getXmls(const QString &str, const QString &pat);
int getXmlInt(const QString &str, const QString &pat);
QString makeXml(QString str, const QString &pat);
QString makeXml(int i, const QString &pat);
QString makeXml(qint64 i, const QString &pat);

bool isBlankChar(QString c);     // 是否为空白符
bool isBlankChar2(QString c);    // 是否为换行之外的空白符
bool isAllBlank(QString s);	// 是否全部都是空白符

QString repeatString(QString s, int i);
QString removeBlank(QString s, bool start = true, bool end = true);
QString simplifyChapter(QString chpt);

QString urlEncode(QString s);
QString urlDecode(QString s);
bool canBeNickname(QString s);

QString ArabToCN(int num);
int CNToArab(QString text);

#endif // STRINGUTIL_H
