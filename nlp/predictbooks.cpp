#include "predictbooks.h"

PredictBooks::PredictBooks(const QStringList &stop_words, QString books_dir, QObject *parent)
    : QObject(parent), jieba(nullptr), stop_words(stop_words),
      books_dir_path(books_dir), initing(false), min_similar(0.6)
{

}

void PredictBooks::setSegment(SegmentTool *segment)
{
    this->jieba = segment;
}

void PredictBooks::runSplit()
{
    if (jieba == nullptr || !jieba->init())
        return ;
    initing = true;
    if (type_books.size() == 0)
    {
        readByType("测试");
    }

    LocateMultiple lm(jieba, stop_words);
    foreach (QStringList books, type_books) // 遍历每一个类型
    {
        foreach (QString book, books) // 遍历每一本书
        {
            PDTDEB << "分割book：" << book.length() << book.split("\n", QString::SkipEmptyParts).size();
//            if (book.length() > 1000000) qDebug() << book.mid(10, 300); // 查看是否乱码
            foreach (QString para, book.split("\n", QString::SkipEmptyParts)) // 遍历每一个分段
            {
                // 获取这个词在分段中的词频
                books_para_vec_list.append(lm.stringToVector(para));
                books_para_str_list.append(para);

                // 遍历每一个句子
                QStringList sent_list = para.split(QRegExp("[，。？！“”]+"), QString::SkipEmptyParts);
                foreach (QString sent, sent_list)
                {
                    if (sent.length() < 3 || sent.length() > 100/*可能乱码出错*/)
                        continue;
                    // 获取句子中各个词的词频
                    sent = sent.trimmed(); // 去除首尾空
                    books_sent_vec_list.append(lm.stringToVector(sent, true));
                    books_sent_str_list.append(sent);
                }
            }
            PDTDEB << "书籍分割结束" << books_para_vec_list.size() << books_sent_vec_list.size();
        }
    }
    full_word_count = lm.full_word_count;
    PDTDEB << "full_word_count" << full_word_count;
    initing = false;
}

QStringList PredictBooks::getNextSentence(QString para)
{
    PDTDEB << "预测下一句：" << para;
    LocateMultiple lm(jieba, stop_words);
    lm.full_word_count = full_word_count;
    WordVec para_vec = lm.stringToVector(para);
    WordFrq TIs = lm.getTF_IDF(para_vec.keys(), getRandomBookSentList());
    PDTDEB << "TIs" << TIs;
    // 随机遍历1000条，获取最高几率或者0.8及以上
    double max_simi = 0;
    int all_count = books_sent_vec_list.count();
    int loop_count = qMin(1000, all_count / 10);
    for (int i = 0; i < loop_count; i++)
    {
        int r = rand() % all_count;
        WordVec cmp_vec = books_sent_vec_list.at(r);
        double simi = lm.getVectorSimilarity(para_vec, cmp_vec, TIs);
        if (simi > max_simi)
        {
            max_simi = simi;
            PDTDEB << simi << books_sent_str_list.at(r) ;
        }
    }
    PDTDEB << "测试最大值：" << max_simi;
PDTDEB << "--------------------------------------------------------------------";
    // 随机遍历最多10000条，直到出现100条或遍历结束
    if (max_simi > 0.8)
        max_simi = 0.8;
    if (max_simi < min_similar)
        max_simi = min_similar;
    int found_count = 0;
    loop_count = qMin(100000, all_count / 10);
    QStringList result;
    _PDTTEST << "循环次数：" << loop_count;
    for (int i = 0; i < all_count; i++)
    {
        int r = rand() % all_count;
//        int r = i;
        WordVec cmp_vec = books_sent_vec_list.at(r);
        double simi = lm.getVectorSimilarity(para_vec, cmp_vec, TIs);
        if (simi >= max_simi)
        {
            QString similar = books_sent_str_list.at(r);
            QString next = (r<all_count-1?books_sent_str_list.at(r+1):"");
            PDTDEB << simi << similar << next;
            found_count++;
            if (similar.trimmed().size() >= 3)
                result.append(next);
        }
        if (found_count >= 100)
            break;
    }
    result.removeDuplicates();
    _PDTTEST << "循环结束" << result.size();
    return result;
    // emit completer->signalPredictResult(result);
}

QStringList PredictBooks::getSimiSentence(QString para)
{
    PDTDEB << "求相似：" << para;
    LocateMultiple lm(jieba, stop_words);
    lm.full_word_count = full_word_count;
    WordVec para_vec = lm.stringToVector(para);
    WordFrq TIs = lm.getTF_IDF(para_vec.keys(), getRandomBookSentList());
    PDTDEB << "TIs" << TIs;
    // 随机遍历1000条，获取最高几率或者0.8及以上
    double max_simi = 0;
    int all_count = books_sent_vec_list.count();
    int loop_count = qMin(1000, all_count / 10);
    for (int i = 0; i < loop_count; i++)
    {
        int r = rand() % all_count;
        WordVec cmp_vec = books_sent_vec_list.at(r);
        double simi = lm.getVectorSimilarity(para_vec, cmp_vec, TIs);
        if (simi > max_simi)
        {
            max_simi = simi;
            PDTDEB << simi << books_sent_str_list.at(r) ;
        }
    }
    PDTDEB << "测试最大值：" << max_simi;
PDTDEB << "--------------------------------------------------------------------";
    // 随机遍历最多10000条，直到出现100条或遍历结束
    if (max_simi > 0.8 && min_similar < 0.8)
        max_simi = 0.8;
    if (max_simi < min_similar)
        max_simi = min_similar;
    int found_count = 0;
    loop_count = qMin(100000, all_count / 10);
    _PDTTEST << "循环次数" << loop_count;
    QStringList result;
    for (int i = 0; i < all_count; i++)
    {
        int r = rand() % all_count;
//        int r = i;
        WordVec cmp_vec = books_sent_vec_list.at(r);
        double simi = lm.getVectorSimilarity(para_vec, cmp_vec, TIs);
        if (simi >= max_simi)
        {
            QString similar = books_sent_str_list.at(r);
            PDTDEB << simi << similar;
            found_count++;
            // emit completer->signalPredictSentence(books_sent_str_list.at(r), similar);
            if (similar.trimmed().size() >= 3)
                result.append(similar);
        }
        if (found_count >= 100)
            break;
    }
    _PDTTEST << "遍历结束";
    result.removeDuplicates();
    return result;
    // emit completer->signalPredictResult(result);
}

QStringList PredictBooks::getRandomBookSentList()
{
    int all_count = books_sent_str_list.size();
    if (all_count < 100)
        return books_sent_str_list;
    int count = qMin(1000, all_count / 10);
    int r = rand() % (all_count - count);
    QStringList list;
    for (int i = 0; i < count; i++)
        list.append(books_sent_str_list.at(r + i));
    return list;
}

void PredictBooks::loadByType(QString type)
{
    if (!jieba) // 未初始化完成
        return ;

    if (type_books.contains(type)) // 已经加载了这个类型了
        return ;

    QtConcurrent::run([=]{
        initing = false;
        readByType(type);
        initing = true;
    });
}

void PredictBooks::loadByType(QStringList types)
{
    foreach (QString type, types)
    {
        loadByType(type);
    }
}

bool PredictBooks::isIniting()
{
    return initing;
}

void PredictBooks::startPredict(QString para, PredictCompleter *completer)
{
    if (isIniting()) // 正在初始化，禁止使用
        return ;
    QtConcurrent::run([=]{
        if (!splited())
            runSplit();
        _PDTTEST << "33333";
        QStringList list = getNextSentence(para);
        _PDTTEST << "444444";
        if (!completer->isValid())
            return ;
        _PDTTEST << "5555555";
        emit completer->signalPredictResult(list);
        _PDTTEST << "66666666";
        delete completer; // 允许多线程出现多个Completer了
        _PDTTEST << "7777777777";
    });
}

void PredictBooks::startSimilar(QString para, PredictCompleter *completer)
{
    if (isIniting()) // 正在初始化，禁止使用
        return ;
    QtConcurrent::run([=]{
        if (!splited())
            runSplit();
        _PDTTEST << "aaaaaaaaaa";
        QStringList list = getSimiSentence(para);
        _PDTTEST << "bbbbbbbb";
        if (!completer->isValid())
            return ;
        _PDTTEST << "ccccccccccccc";
        emit completer->signalPredictResult(list);
        _PDTTEST << "ddddddddddd";
        delete completer; // 允许多线程出现多个Completer了
        _PDTTEST << "eeeeeeeeeee";
    });
}

bool PredictBooks::splited()
{
    return books_para_vec_list.count() > 0;
}

void PredictBooks::readByType(QString type)
{
    QString path = books_dir_path + type + "/";
    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList file_names = dir.entryList();
    QStringList books;
    foreach (QString file_name, file_names)
    {
        QString file_path = path + file_name;
        QString text = readTextFile(file_path, "GBK");
        books.append(text);
        PDTDEB << "加载" << file_name << "完毕";
    }
    type_books.insert(type, books);
}
