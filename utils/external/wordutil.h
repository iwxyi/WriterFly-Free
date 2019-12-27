#ifndef WORDUTIL_H
#define WORDUTIL_H

#include <QObject>
#include <QMap>
#include <QString>
#ifdef Q_OS_WIN
    #include <QAxObject>
    #include <QAxWidget>
    #include "qt_windows.h"
#endif
class WordUtil : public QObject
{
    Q_OBJECT
public:
    WordUtil(QObject *parent = nullptr);

    static bool replacePlainText(QString temp_path, QString save_path, QString label, QString text);
    static bool replacePlainText(QString temp_path, QString save_path, QStringList labels, QStringList texts);

    static QString readPlainText(QString path);

    static bool writePlainText(QString path, QString text);

signals:
    void signalSuccessed();
    void signalError(QString content);

public slots:
    bool open(const QString &file);//打开文档
    void save(const QString &savePath);//保存并关闭文档

    void replaceText(const QString &label,const QString &text);//替换文档中标签出的文字

private:
#ifdef Q_OS_WIN
    QAxObject *_word;         //word主程序，其实是 QAxWidget*，为了支持多线程才使用的 QAxObject*
    QAxObject *_workDocument; //工作簿
#endif
    bool _isOpen;//文档打开状态
};

#endif // WORDUTIL_H
