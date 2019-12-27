#include "lexicon.h"

Lexicons::Lexicons(QString path)
{
    lexicon_dir = path;
    search_last = "";
    us = nullptr;

    synonym_inited = related_inited = sentence_inited = surname_inited = name_inited = random_inited = sensitive_inited = dictionary_inited = false;
    matched_case = COMPLETER_CONTENT_NONE;
}

void Lexicons::setUS(USettings* us)
{
    this->us = us;

    initSensitive();
    initRandomName();
    initSynonym();
    initStopWords();

    QTimer::singleShot(1000, [=]{
        QtConcurrent::run(this, &Lexicons::initRelated);
    });
    QTimer::singleShot(2000, [=]{
        QtConcurrent::run(this, &Lexicons::initSentence);
    });
    QTimer::singleShot(3000, [=]{
        QtConcurrent::run(this, &Lexicons::initDictionary);
    });
}

void Lexicons::initAll(bool syno, bool rela, bool sent, bool random)
{
    Q_UNUSED(syno);
    Q_UNUSED(rela);
    Q_UNUSED(sent);
    Q_UNUSED(random);

    /*if (syno)
    {
        initSynonym();
    }

    if (rela)
    {
        initRelated();
    }

    if (sent)
    {
        initSentence();
    }

    if (random)
    {
        initRandomName();
    }*/
}

void Lexicons::initSynonym()
{
    synonym_inited = true;
    search_last = "";
    QString path = lexicon_dir + "synonym/synonym.txt";
    if (isFileExist(path))
        synonym_text = readTextFile(path);

}

void Lexicons::initRelated()
{
    related_inited = true;
    search_last = "";
    QString path = lexicon_dir + "related/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    related_text = "";
    for (QString n : files)
    {
        QString text = readTextFile(path + n);
        related_text += text;
    }
}

void Lexicons::initSentence()
{
    sentence_inited = true;
    search_last = "";
    QString path = lexicon_dir + "sentence/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name); // 获取文件名字列表（只带后缀名）

    sentence_text = "";
    for (QString f : files)
    {
        QString text = readTextFile(path+f);
        sentence_text += text + "\n";
    }
}

void Lexicons::initRandomName()
{
    random_inited = true;
    search_last = "";
    QString path = lexicon_dir + "random/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    random_sort_list.clear();
    random_text_list.clear();
    for (QString f : files)
    {
        QString text = readTextFile(path+f);
        if (f.endsWith(".txt"))
            f.chop(4);
        random_sort_list.append(f);
        random_text_list.append(text);
    }

    // 将后缀一样的内容排序，比如“地名”和“奇幻地名”
    for (int i = 0; i < random_sort_list.size()-1; i++)
        for (int j = i+1; j < random_sort_list.size(); j++)
        {
            if (random_sort_list.at(j).endsWith(random_sort_list.at(i)))
            {
                random_sort_list.swap(i, j);
                random_text_list.swap(i, j);
            }
        }

    if (isFileExist(path + "姓氏.txt"))
    {
        surname_text = " " + readTextFile(path+"姓氏.txt") + " "; // 前后空格表示有的搜索
        surname_inited = true;
    }

    if (isFileExist(path + "人名.txt"))
    {
        name_text = readTextFile(path+"人名.txt"); // 前后空格表示有的搜索
        name_inited = true;
    }

}

void Lexicons::initSensitive()
{
    if (isFileExist(lexicon_dir + "sensitive/敏感词.txt"))
    {
        sensitive_text = readTextFile(lexicon_dir+"sensitive/敏感词.txt").trimmed();
        sensitive_list = sensitive_text.split(" ", QString::SkipEmptyParts);

        sensitive_text = sensitive_text.replace(QRegExp("\\s+"), " ");
        QString pattern = sensitive_text;
        pattern = "("+pattern.replace(" ", "|")+")";
        sensitive_pattern = QRegularExpression(pattern);
    }

    sensitive_inited = true;
}

void Lexicons::initStopWords()
{
    QString text = readTextFile(":/lexicons/stop_words");
    stop_words = text.split("\n", QString::SkipEmptyParts);
}

void Lexicons::initDictionary()
{
    if (isFileExist(lexicon_dir + "dictionary/目录.txt") && isFileExist(lexicon_dir + "dictionary/全文.txt"))
    {
        dictionary_dir = "\n"+readTextFile(lexicon_dir + "dictionary/目录.txt")+"\n";
        dictionary_text = readTextFile(lexicon_dir + "dictionary/全文.txt");
    }
    dictionary_inited = true;
}

void Lexicons::uninitAll(bool syno, bool rela, bool sent, bool random)
{
    if (syno)
    {
        synonym_inited = false;
    }

    if (rela)
    {
        related_inited = false;
    }

    if (sent)
    {
        sentence_inited = false;
    }

    if (random)
    {
        random_inited = false;
        surname_inited = name_inited = false;
    }
}

void Lexicons::loadErrorWords()
{
    QString filepath = lexicon_dir + "translation/词语纠错.txt";
    QString content = readTextFile(filepath);
    QStringList lines = content.split("\n", QString::SkipEmptyParts);
    QString pattern_str = "";

    int size = lines.size();
    for (int i = 0; i < size; i++)
    {
        QString line = lines.at(i);
        QStringList words = line.split("\t", QString::SkipEmptyParts);
        if (words.size() >= 2)
        {
            if (i || !pattern_str.isEmpty())
                pattern_str += "|";
            pattern_str += words.at(0);
        }
        if (i % 1000 == 0) // 因为词库太多了，必须拆成多个正则表达式才行，否则会报错
        {
            error_word_patterns.append(QRegularExpression(pattern_str));
            pattern_str = "";
        }
    }
    if (!pattern_str.isEmpty())
        error_word_patterns.append(QRegularExpression(pattern_str));
}

void Lexicons::clear()
{
    matched_key = "";
    matched_case = COMPLETER_CONTENT_NONE;
    search_result.clear();
}

/**
 * 某个句子的某个位置处进行搜索
 * @param sent   欲搜索的完整句子（短句，不包含标点）
 * @param cursor 光标在句子中的相对位置
 * @return       是否有搜索结果
 */
bool Lexicons::surroundSearch(QString sent, int cursor)
{
    int len = sent.length();
    search_result.clear();
    match_sentence = false;
    bool find = false;
    matched_key = "";
    int start_pos = 0;
    QString l1 = "", l2 = "", l4 = "";

    if (cursor >= 1)
        l1 = sent.mid(cursor-1, 1);
    if (cursor >= 2)
        l2 = sent.mid(cursor-2, 2);
    if (cursor >= 4)
        l4 = sent.mid(cursor-4, 4);

    if (random_inited)
    {
        // 随机种类列表
        if (((matched_key = l2) == "随机")
         || ((matched_key = l2) == "取名")
         || ((matched_key = l4) == "随机取名"))
            if (searchRandom("随机取名"))
            {
                matched_case = COMPLETER_CONTENT_RAND_LIST;
                return true;
            }

        // 姓氏
        if (surname_inited && (((matched_key = l1) == "姓")
         || ((matched_key = l2) == "姓氏")))
            if (searchRandom("姓氏"))
            {
                matched_case = COMPLETER_CONTENT_SURN;
                return true;
            }

        // 人名
        if (name_inited && (((matched_key = l2) == "人名")
         || ((matched_key = l2) == "名")
         || ((matched_key = l2) == "名字")))
            if (searchRandom("人名"))
            {
                matched_case = COMPLETER_CONTENT_NAME;
                return true;
            }

        // 姓氏触发的人名
        if (us->lexicon_surname && surname_inited && name_inited && cursor == len/*右边是空的*/)
        {
            if (surname_text.indexOf(" " + l1 + " ") > -1)
            {
                if (searchRandom("人名"))
                {
                    matched_key = "";
                    matched_case = COMPLETER_CONTENT_NAME;
                    return true;
                }
            }
            else if (surname_text.indexOf(" " + l2 + " ") > -1)
            {
                if (searchRandom("人名"))
                {
                    matched_key = "";
                    matched_case = COMPLETER_CONTENT_NAME;
                    return true;
                }
            }
        }

        // 随机列表
        for (QString s : random_sort_list)
        {
            if (cursor >= s.length() && sent.mid(cursor-s.length(), s.length()) == s && isFileExist(lexicon_dir + "random/" + s + ".txt"))
            {
                matched_key = s;
                matched_case = COMPLETER_CONTENT_RAND;
                searchRandom(s);
                return true;
            }
        }
    }

    // 搜索4个字
    if (!find)
    {
        start_pos = 0; // 开始搜索的位置
        if (start_pos < cursor-4) start_pos = cursor-4;
        for (int i = start_pos; i <= len-4 && i < cursor; i++)
        {
            if (search(sent.mid(i, 4), true))
            {
                find = true;
                matched_key = sent.mid(i, 4);
                break;
            }
        }
    }

    // 搜索三个字
    if (!find)
    {
        start_pos = 0;
        if (start_pos < cursor-3) start_pos = cursor-3;
        for (int i = start_pos; i <= len-3 && i < cursor; i++)
            if (search(sent.mid(i, 3), true))
            {
                find = true;
                matched_key = sent.mid(i, 3);
                break;
            }
    }

    // 搜索两个字
    if (!find)
    {
        start_pos = 0;
        if (start_pos < cursor-2) start_pos = cursor-2;
        for (int i = start_pos; i <= len-2 && i < cursor; i++)
            if (search(sent.mid(i, 2), true))
            {
                find = true;
                matched_key = sent.mid(i, 2);
                break;
            }
    }
    //qDebug() << "surround search:" << matched_key << "  result:" << search_result;
    search_last = sent;
    matched_case = COMPLETER_CONTENT_WORD;
    return find;
}

/**
 * 在词库中搜索某一个词语
 * @param key 欲搜索的词语
 * @param add 是否为添加模式。如果不是，则先清空已经找到的列表
 * @return    是否有搜索结果
 */
bool Lexicons::search(QString key, bool add)
{
    if (key.isEmpty()) return false;

    /* // 上次搜索的缓冲区，但是没必要了，因为每次surroundSearch的时候
     * // 都把上次的搜索结果清空了，key还在但是result没了
    if (key == search_last)
    {
        return true;
    }*/

    if (!add) search_result.clear();
    bool find = false;
    int key_len = key.length();

    // 搜索近义词
    if (synonym_inited)
    {
        QStringList synonym_list;
        int pos = 0;
        while (1)
        {
            //pos = synonym_text.indexOf(key, pos);
            pos = synonym_text.indexOf(QRegExp("\\b"+key+"\\b"), pos);
            if (pos == -1) break;
            int left = synonym_text.lastIndexOf("\n", pos)+1;
            int right = synonym_text.indexOf("\n", pos);
            if (right == -1) right = synonym_text.length();
            QString para = synonym_text.mid(left, right-left);
            QStringList list = para.split(" ", QString::SkipEmptyParts);
            // TODO 后期将改成 QList<CompleterItem>
            synonym_list.append(list);
            pos += key_len;
            find = true;
        }
        if (synonym_list.size() > 0 && shouldRandom())
        {
            if (shouldRandom())
            {
                std::random_shuffle(synonym_list.begin(), synonym_list.end());
            }
            search_result.append(synonym_list);
        }
    }

    // 搜索相关词
    if (related_inited)
    {
        QStringList related_list;
        int pos = 0;
        while (1)
        {
            pos = related_text.indexOf(QRegExp("\\b"+key+"\\b"), pos);
            if (pos == -1) break; // 找不到了
            if (pos > 0 && related_text.mid(pos-1, 1) == "{") // 是标题
            {
                int left = related_text.indexOf("[", pos)+1;
                int right = related_text.indexOf("]", pos);
                if (right < left-1) // 出现了错误
                {
                    pos = left+1;
                    continue;
                }
                if (right == -1) right = related_text.length();
                QString para = related_text.mid(left, right-left);
                QStringList list = para.split(" ", QString::SkipEmptyParts);
                // TODO 后期将改成 QList<CompleterItem>
                related_list.append(list);
                pos = right;
            }
            else // 是内容
            {
                // 如果是本程序标准格式
                int left = related_text.lastIndexOf("[", pos)+1;
                int right = related_text.indexOf("]", pos);

                // 如果只是一段一段分开的
                int left_n = related_text.lastIndexOf("\n", pos)+1;
                if (left_n >= left)
                {
                    left = left_n;
                    right = related_text.indexOf("\n", pos);
                }
                if (right == -1) right = related_text.length();

                QString para = related_text.mid(left, right-left);
                QStringList list = para.split(" ", QString::SkipEmptyParts);
                // TODO 后期将改成 QList<CompleterItem>
                related_list.append(list);
                pos = right;
            }

            find = true;
        }
        if (related_list.size() > 0)
        {
            if (shouldRandom())
            {
                std::random_shuffle(related_list.begin(), related_list.end());
            }
            search_result.append(related_list);
        }
    }

    // 搜索句子（只显示是否搜索到了）
    if (sentence_inited && find)
    {
        if (sentence_text.indexOf(key) > -1)
        {
            match_sentence = true;
            search_result.append("-->");
        }
    }
    //qDebug() << "search:" << key << "  result:" << search_result;
    return find;
}

/**
 * 通过一个词语，来获取应该显示的随机取名提示列表
 * @param key 欲搜索的词语
 * @return    是否找到结果
 */
bool Lexicons::searchRandom(QString key)
{
	// ==== 随机取名列表 ====
	if (key == "随机取名" || key == "随机" || key == "取名")
	{
		for (QString s : random_sort_list)
            search_result.append(s);
        std::random_shuffle(search_result.begin(), search_result.end());
		return true;
	}

	// ==== 随机取名具体 ====
    for (int i = 0; i < random_sort_list.size(); i++)
        if (random_sort_list.at(i) == key)
		{
            QStringList list = random_text_list.at(i).split(" ", QString::SkipEmptyParts);
            search_result = list;
            std::random_shuffle(search_result.begin(), search_result.end());
            search_result = search_result.mid(0, 100);
	        return true;
		}

	return false;
}

/**
 * 搜索后返回结果
 * @return 搜索结果
 */
QStringList Lexicons::getResult()
{
    return search_result;
}

QString Lexicons::getMatched()
{
    return matched_key;
}

int Lexicons::getMatchedCase()
{
    return matched_case;
}

void Lexicons::setMatchedCase(int x)
{
    matched_case = x;
}

QStringList Lexicons::getSentence()
{
    return getSentence(matched_key);
}

QStringList Lexicons::getSentence(QString key)
{
    search_result.clear();
    if (key.isEmpty()) return search_result;
    matched_case = COMPLETER_CONTENT_SENT;

    int pos = 0;
    while (1)
    {
        pos = sentence_text.indexOf(key, pos);
        if (pos == -1) break;
        int left = sentence_text.lastIndexOf("\n", pos)+1;
        int right = sentence_text.indexOf("\n", pos);
        if (right == -1) right = sentence_text.length();
        QString para = sentence_text.mid(left, right-left);
        // TODO 后期将改成 QList<CompleterItem>
        search_result.append(para);
        pos = right+1;
    }
    if (shouldRandom())
    {
        std::random_shuffle(search_result.begin(), search_result.end());
    }
    return search_result;
}

QRegularExpression Lexicons::getSensitivePattern()
{
    return sensitive_pattern;
}

QList<QRegularExpression> Lexicons::getErrorWordPatterns()
{
    return error_word_patterns;
}

QStringList& Lexicons::getSensitiveWords()
{
    return sensitive_list;
}

QStringList& Lexicons::getStopWords()
{
    return stop_words;
}

bool Lexicons::isStopWord(QString s)
{
    return stop_words.contains(s);
}

bool Lexicons::isSplitWord(QString c)
{
    static QString split = "的地着是把了在叫为就对说看向中到来去上有称作很乃和跟要做";
    static QString quantifiers = "局团坨滩根排列匹张座回场尾条个首阙阵网炮顶丘棵只支袭辆挑担颗壳窠曲墙群腔砣座客贯扎捆刀令打手罗坡山岭江溪钟队单双对出口头脚板跳枝件贴针线管名位身堂课本页丝毫厘分钱两斤担铢石钧锱忽毫厘分寸尺丈里寻常铺程撮勺合升斗石盘碗碟叠桶笼盆盒杯钟斛锅簋篮盘桶罐瓶壶卮盏箩箱煲啖袋钵年月日季刻时周天秒分旬纪岁世更夜春夏秋冬代伏辈丸泡粒颗幢堆";

    return split.contains(c) || quantifiers.contains(c);
}

bool Lexicons::shouldRandom()
{
    if (us == nullptr)
        return true;
    return us->completer_random;
}
