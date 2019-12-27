#include "jiebautil.h"

JiebaUtil::JiebaUtil() : SegmentTool(), inited(false), initing(false)
{
    QTimer::singleShot(3000, [=]{
        QtConcurrent::run(this, &JiebaUtil::init);
    });
}

JiebaUtil::~JiebaUtil()
{
    // delete jieba;
}

bool JiebaUtil::init()
{
    if (inited || initing) return true;
    initing = true;

    QString base_path = "";
    if (QDir("./tools/jieba/dict").exists())
        base_path = "./tools/jieba/dict/";
    else if (QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tools/jieba/dict").exists())
        base_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tools/jieba/dict/";
    else if (QDir("./release/tools/jieba/dict").exists())
        base_path = "./release/tools/jieba/dict/";
    else if (QDir("./debug/tools/jieba/dict").exists())
        base_path = "./debug/tools/jieba/dict/";

    if (base_path.isEmpty())
    {
        qDebug() << "Jieba 分词路径不存在，暂时停用此功能";
        initing = false;
        return false;
    }

    const QString DICT_PATH = base_path+"jieba.dict.utf8";
    const QString HMM_PATH = base_path+"hmm_model.utf8";
    const QString USER_DICT_PATH = base_path+"user.dict.utf8";
    const QString IDF_PATH = base_path+"idf.utf8";
    const QString STOP_WORD_PATH = base_path+"stop_words.utf8";

    jieba = new cppjieba::Jieba(
                DICT_PATH.toLatin1().data(),
                HMM_PATH.toLatin1().data(),
                USER_DICT_PATH.toLatin1().data(),
                IDF_PATH.toLatin1().data(),
                STOP_WORD_PATH.toLatin1().data()
            );

    inited = true;
    initing = false;
    return true;
}

QStringList JiebaUtil::WordSegment(QString text)
{
    if (!inited)
        return text.split("", QString::SkipEmptyParts);

    vector<string> words;
    vector<cppjieba::Word> jiebawords;

    jieba->Cut(text.toStdString(), words, true);

    vector<string>::iterator it;
    QStringList sl;
    for (it = words.begin(); it != words.end(); it++)
    {
        sl << QString::fromStdString(*it);
    }

    return sl;
}
