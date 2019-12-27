#include "wordutil.h"

WordUtil::WordUtil(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    _word = nullptr;
    _workDocument = nullptr;
#endif
    _isOpen = false;
}

bool WordUtil::replacePlainText(QString temp_path, QString save_path, QString label, QString text)
{
#ifdef Q_OS_WIN
    WordUtil wtr;
    if (!wtr.open(temp_path)) return false;
    wtr.replaceText(label, text);
    wtr.save(save_path);
#endif
    return true;
}

bool WordUtil::replacePlainText(QString temp_path, QString save_path, QStringList labels, QStringList texts)
{
#ifdef Q_OS_WIN
    WordUtil wtr;
    if (!wtr.open(temp_path)) return false;
    int len = qMin(labels.length(), texts.length());
    for (int i = 0; i < len; i++)
    {
        wtr.replaceText(labels.at(i), texts.at(i));
    }
    wtr.save(save_path);
#endif
    return true;
}

QString WordUtil::readPlainText(QString path)
{
#ifdef Q_OS_WIN

#endif
}

bool WordUtil::writePlainText(QString path, QString text)
{
#ifdef Q_OS_WIN
    //隐式的打开一个word应用程序
    QAxObject word("Word.Application");
    word.setProperty("Visible", false);

    //获取所有工作文档
    QAxObject * documents = word.querySubObject("Documents");
    //创建一个word文档
    documents->dynamicCall("Add (void)");
    //获取当前激活的文档
    QAxObject * document = word.querySubObject("ActiveDocument");
    //写入文件内容
    QAxObject *selection = word.querySubObject("Selection");
    selection->dynamicCall("TypeText(const QString&)", text);
    //设置保存
    QVariant newFileName(path); //存放位置和名称
//    QVariant fileFormat(1); //文件格式
//    QVariant LockComments(false);
//    QVariant Password("123456");     //设置打开密码
//    QVariant recent(true);
//    QVariant writePassword("");
//    QVariant ReadOnlyRecommended(false);

//    document->querySubObject("SaveAs(const QVariant&, const QVariant&,const QVariant&, const QVariant&, const QVariant&, const QVariant&,const QVariant&)", newFileName, fileFormat, LockComments, Password, recent, writePassword, ReadOnlyRecommended);
    document->querySubObject("SaveAs(const QVariant&)", newFileName);
    //关闭文档
    document->dynamicCall("Close (boolean)", true);
    //退出
    word.dynamicCall("Quit (void)");
#endif
    return true;
}

/**
 * Summary: 根据传入的模板文件地址创建新的word文档并打开
 * param:   @file .doc模板文件的绝对地址，这里只能使用绝对地址，否则找不到文件，并且建议使用QDir::toNativeSeparators()将地址中的'/'转换成'\'
 * return:  文档是否打开
 */
bool WordUtil::open(const QString& file)
{
#ifdef Q_OS_WIN
    _word = new QAxObject("word.Application");
    QAxObject *document = _word->querySubObject("Documents");//获取所有打开的文档
    if (!document)
        return false;

    document->dynamicCall("Add(QString)",file);//使用模板文件创建新的文档
    _workDocument = _word->querySubObject("ActiveDocument");//激活文档

    if (_workDocument)
        _isOpen = true;
    else
        _isOpen = false;

    if (!_isOpen)
    {
        emit signalError(QString("未找到模板文件：%0").arg(file));
    }


#endif
    return _isOpen;
}

/**
 * Summary: 将文档保存到指定地址，并关闭word程序
 * param:   @savePath word的保存地址
 */
void WordUtil::save(const QString &savePath)
{
#ifdef Q_OS_WIN
    // 模板文件是否存在
    if (!_isOpen)
        return ;

    //保存
    if (_workDocument)
    {
        _workDocument->dynamicCall("SaveAs (const QString&)",savePath);
    }

    if (_word)
    {
        _word->setProperty("DisplayAlerts",true);
    }

    //关闭文档
    if (_workDocument)
    {
        _workDocument->dynamicCall("Close(bool)",true);
        delete _workDocument;
        _workDocument = nullptr;
    }

    //推出word程序
    if (_word)
    {
        _word->dynamicCall("Quit(void)"); // 多线程下会导致报错：QAxBase: Error calling IDispatch member Quit: Unknown error
        delete _word;
        _word = nullptr;
    }

    _isOpen = false;

    emit signalSuccessed();
#endif
}

/**
 * Summary: 替换指定书签处的内容
 * param:
 *     @label 书签名称，这里的书签对应的是在word中的插入->书签
 *     @text 内容
 */
void WordUtil::replaceText(const QString &label, const QString &text)
{
#ifdef Q_OS_WIN
    if (!_workDocument)
        return ;

    //查找书签
    QAxObject *bookmark = _workDocument->querySubObject("Bookmarks(QString)",label);
    if (bookmark)
    {
        //选定书签，并替换内容
        bookmark->dynamicCall("Select(void)");
        bookmark->querySubObject("Range")->setProperty("Text",text);
        delete bookmark;
    }
#endif
}
