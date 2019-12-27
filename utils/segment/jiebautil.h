#ifndef JIEBAUTIL_H
#define JIEBAUTIL_H

#include <QApplication>
#include <QStringList>
#include <QDir>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include "tools/jieba/include/Jieba.hpp"
#include "segmenttool.h"

using namespace std;

class JiebaUtil : public SegmentTool
{
public:
    JiebaUtil();
    ~JiebaUtil();

    bool init();

    QStringList WordSegment(QString text);

private:
    QString base_path;
    bool inited, initing;

    cppjieba::Jieba* jieba;
};

#endif // JIEBAUTIL_H
