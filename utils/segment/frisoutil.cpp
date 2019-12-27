#include "frisoutil.h"


FrisoUtil::FrisoUtil() : SegmentTool ()
{
    inited = false;
    initing = false;
    valid = true;

    QtConcurrent::run(this, &FrisoUtil::init);
}

FrisoUtil::~FrisoUtil()
{
    Destructor();
}

QStringList FrisoUtil::WordSegment(QString _text)
{
    Q_UNUSED(_text);
#if defined(Q_OS_WIN) && defined(FRISO)
    if (!valid)
        return sList;

    if (!inited)
    {
        init();
        if (inited == false) // 初始化失败
        {
            QStringList list;
            int len = _text.length();
            for (int i = 0; i < len; i++)
                list.append(_text.mid(i, 1));
            return list;
        }
    }

    if (_text == _recent) return sList;
    _recent = _text;

    friso_task_t task = friso_new_task();
    fstring text = _text.toUtf8().data();
    friso_set_text(task, text);

    sList.clear();
    while ( (friso_next(friso, config, task)) != nullptr )
    {
        sList.append(task->hits->word);
    }

    friso_free_task(task);
#endif
    return sList;
}

void FrisoUtil::Destructor()
{
#if defined(Q_OS_WIN) && defined(FRISO)
    if (inited)
    {
        try {
//            friso_free_config(config); // 这行在 win7（32位）上会导致崩溃
            friso_free(friso);
        } catch (int e) {
            qDebug() << "析构分析出错:" << e;
        } catch (std::string e) {
            qDebug() << "析构分析出错:" << QString::fromStdString(e);
        }
    }
#endif
}

bool FrisoUtil::init()
{
#if defined(Q_OS_Android)
    initing = true;
    valid = false;
#elif defined(Q_OS_WIN) && defined(FRISO)
    if (initing) return false;
    initing = true;

    char pa[1000] = "";
    strcpy(pa, QApplication::applicationDirPath().toLocal8Bit().data());
    strcat(pa, "/tools/friso/friso.ini");
    fstring _ifile = pa;
    friso = friso_new();
    config = friso_new_config();
    if (friso_init_from_ifile(friso, config, _ifile) != 1) {
        qDebug() << "fail to initialize friso and config.";
        return initing = false;
    }
    initing = false;
    inited = true;
#endif
    return inited;
}
