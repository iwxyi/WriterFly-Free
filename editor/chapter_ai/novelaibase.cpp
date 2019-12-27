#include "novelaibase.h"

NovelAIBase::NovelAIBase()
{
    initConstString();
}

NovelAIBase::~NovelAIBase()
{
    // 纯虚析构函数必须得实现，此处不进行任何操作
}

/**
 * 智能引号，根据语境自动添加各种形式的双引号
 * 尤其是有没有分号或者冒号
 */
void NovelAIBase::operatorSmartQuotes()
{
    // 判断是否为人物的第二句话（即双引号前面是逗号而不是冒号）
    bool is_second_said = false;
    if (isChinese(_left1) || _left1 == "“" || _right1 == "“" || _right2 == "“")
    {
        int pos = _pos;
        if (pos > 4)
        {
            if (pos == _text.length() || _text.mid(pos, 1) == "\n") pos--;
            int n_pos = _text.lastIndexOf("\n", pos);
            if (n_pos == -1) n_pos = 0;
            if (_left1 == "”") pos -= 2;
            else if (_right1 == "”") pos--;
            int q_pos = _text.lastIndexOf("”", pos);
            if (q_pos > n_pos) // 前面有对双引号
            {
                int i = q_pos;
                bool isSent = true;
                while (++i < pos)
                {
                    QString cha = _text.mid(i, 1);
                    if (isSentPunc(cha)) // 句中标点，不是连续的话，就没有必要用逗号了
                    {
                        isSent = false;
                        break;
                    }
                }
                if (isSent)
                {
                    is_second_said = true;
                }
            }
        }
    }

    // 左右前后引号的位置
    int qll, qlr, qrl, qrr, nl, nr;
    qll = _text.lastIndexOf("“", _pos > 0 ? _pos - 1 : _pos);
    qlr = _text.lastIndexOf("”", _pos > 0 ? _pos - 1 : _pos);
    qrl = _text.indexOf("“", _pos);
    qrr = _text.indexOf("”", _pos);
    nl = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
    nr = _text.indexOf("\n", _pos);
    if (nl > -1 && qll < nl) qll = -1;
    if (nl > -1 && qlr < nl) qlr = -1;
    if (nr > -1 && qrl > nr) qrl = -1;
    if (nr > -1 && qrr > nr) qrr = -1;

    // ==== 分析标点 ====
    if (_left1 == "“")            // ，“|    ：“|    汉“|    “|”    “|
    {
        if (_left2 == "，")
        {
            deleteText(_pos - 2, _pos - 1);
            insertText(_pos - 1, "：");
            ac->addUserWords();
        }
        else if (_left2 == "：")
        {
            deleteText(_pos - 2, _pos - 1);
        }
        else if (isChinese(_left2))
        {
            insertText(_pos - 1, "：");
            ac->addUserWords();
        }
        else if (_right1 == "”") // 空引号
        {
            if (isBlankChar(_left2) || isBlankChar(_right2))
                if (_left2 == "，" || _left2 == "：")
                	deleteText(_pos-2, _pos);
                else
                	deleteText(_pos-1, _pos+1);
            else
                deleteText(_pos - 1, _pos);
        }
        else if (isBlankChar(_right1))
        {
        	insertText("”");
        	moveCursor(-1);
            ac->addUserWords();
        }
        else
        {
            moveCursor(-1);
        }
    }
    else if (_left1 == "”")       // 。”|    汉”|     ”|
    {
        if (isSentPunc(_left2))
        {
            deleteText(_pos - 2, _pos - 1);
            moveCursor(-1);
        }
        else if (isChinese(_left2))
        {
            QString punc = getPunc2(_pos - 1);
            insertText(_pos - 1, punc);
            ac->addUserWords();
        }
        else
        {
            int l_pos = _pos-1;
        	bool is_operator = false;
            while (l_pos-- > 0 && _text.mid(l_pos, 1) != "\n")
                if (_text.mid(l_pos, 1) == "“") // 有前引号，则只是移动位置
        		{
        			moveCursor(-1);
        			is_operator = true;
        			break;
        		}
        		else if (_text.mid(l_pos, 1) == "”")
        			break;
            if (!is_operator) // 如果什么都没有操作，可能前面就是空的内容，补全一个前引号
            {
            	moveCursor(-1);
	            insertText("“");
                ac->addUserWords();
	        }
        }
    }
    else if (_right1 == "“")      // 汉|“    ，|“    ：|“    |“
    {
        if (isChinese(_left1))
        {
            if (is_second_said)
            {
                insertText("，");
                ac->addUserWords();
            }
            else
            {
                insertText("：");
                ac->addUserWords();
            }
            if (_right2 == "”")
            {
                moveCursor(1);
            }
        }
        else if (_left1 == "，")
        {
            deleteText(_pos - 1, _pos);
            insertText("：");
            if (_right2 == "”")
            {
                moveCursor(1);
            }
        }
        else if (_left1 == "；")
        {
            deleteText(_pos - 1, _pos);
        }
        else if (isBlankChar(_left1))
        {
            moveCursor(1);
        }
        else
        {
            moveCursor(1);
        }
    }
    else if (_right1 == "”")      // 汉|”    。|”    ，|”
    {
        if (isChinese(_left1))
        {
            bool usePunc = true;
            int q_pos = _text.lastIndexOf("“", _pos > 0 ? _pos - 1 : _pos);
            int n_pos = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
            if (q_pos > n_pos + 1) // 前引号左边是中文时不增加标签
            {
                QString cha = _text.mid(q_pos - 1, 1);
                if (isChinese(cha))
                {
                    usePunc = false;
                }
            }
            if (usePunc)   // 需要插入标点
            {
                insertText(getPunc2());
                ac->addUserWords();
            }
            moveCursor(1);
        }
        else if (_left1 == "。")
        {
            moveCursor(1);
        }
        else if (_left1 == "，")
        {
            deleteText(_pos - 1, _pos);
            insertText(getPunc2());
            moveCursor(1);
        }
        else
        {
            moveCursor(1);
        }
    }
    else if (qll <= qlr && qrl == -1 && qrr != -1)   // 缺 前引号
    {
        insertText("“");
        ac->addUserWords();
    }
    else if (qll <= qlr && qrl > -1 && qrl > qrr)    // 缺 前引号
    {
        insertText("“");
        ac->addUserWords();
    }
    else if (qll > qlr && qrl > -1 && qrr > qrl)     // 缺 后引号
    {
        if (isChinese(_left1))
        {
            if (isSentPunc(_right1) && isBlankChar(_right2))
                moveCursor(1);
            else
            {
                insertText(getPunc2());
                ac->addUserWords();
            }
        }
        insertText("”");
        ac->addUserWords();
    }
    else if (qll > qlr && qrr == -1)                 // 缺 后引号
    {
        if (isChinese(_left1))
        {
            if (isSentPunc(_right1) && isBlankChar(_right2))
                moveCursor(1);
            else
            {
                insertText(getPunc2());
                ac->addUserWords();
            }
        }
        else if (_left1 == "，")   // 句子结尾
        {
            deleteText(_pos - 1, _pos);
            insertText(getPunc2());
            ac->addUserWords();
        }
        insertText("”");
        ac->addUserWords();
    }
    else if (isInQuotes)          // 添加或者删除单引号
    {
        qll = _text.lastIndexOf("‘", _pos);
        qlr = _text.lastIndexOf("’", _pos);
        qrl = _text.indexOf("‘", _pos);
        qrr = _text.indexOf("’", _pos);
        nl = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
        nr = _text.indexOf("\n", _pos);
        if (nl > -1 && qll < nl) qll = -1;
        if (nl > -1 && qlr < nl) qlr = -1;
        if (nr > -1 && qrl > nr) qrl = -1;
        if (nr > -1 && qrr > nr) qrr = -1;

        if (_right1 == "’" || _right1 == "‘" || _right1 == "'")
        {
            moveCursor(1);
        }
        else if (_left1 == "‘" || _left1 == "’")
        {
            moveCursor(-1);
        }
        else if (qll <= qlr && qrl == -1 && qrr != -1)   // 缺 前引号
        {
            insertText("‘");
            ac->addUserWords();
        }
        else if (qll <= qlr && qrl > -1 && qrl > qrr)    // 缺 前引号
        {
            insertText("‘");
            ac->addUserWords();
        }
        else if (qll > qlr && qrl > -1 && qrr > qrl)     // 缺 后引号
        {
            insertText("’");
            ac->addUserWords();
        }
        else if (qll > qlr && qrr == -1)                 // 缺 后引号
        {
            if (_left1 == "，")
            {
                deleteText(_pos - 1, _pos);
                insertText(getPunc2());
                ac->addUserWords();
            }
            insertText("’");
            ac->addUserWords();
        }
        else
        {
            insertText("‘’");
            moveCursor(-1);
            ac->addUserWords();
        }
    }
    else if (_left1 == "，")      // ，|
    {
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords();
    }
    else if (_left1 == "：")      // ：|
    {
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords();
    }
    else if (_right1 == "，" && _right2 == "“")   // |，“
    {
        deleteText(_pos, _pos + 1);
        insertText("：");
        ac->addUserWords();
    }
    else if (_right1 == "：" && _right2 == "“")   // |：“
    {
        deleteText(_pos, _pos + 1);
    }
    else if (isChinese(_left1))   // 汉|    =>    汉：“”    汉，“”    汉“”
    {
        if (isQuoteColon(_left1))
        {
            if (is_second_said)
                insertText("，");
            else
                insertText("：");
            ac->addUserWords();

        }
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords();
    }
    else
    {
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords(2);
    }
}

/**
 * 针对选中文字情况的智能引号，增加双引号或者去掉双引号
 * @param left  选中的文字左边
 * @param right 选中的文字右边
 */
void NovelAIBase::operatorSmartQuotes2(int left, int right)
{
    // 确保左边小于右边
    if (left > right)
    {
        int temp = left;
        left = right;
        right = temp;
    }
    if (left < 0 || right > _text.length() || left == right) return ;

    // 将旁边的空白收缩
    while (left < right && isBlankChar(_text.mid(left,1)))
        left++;
    while (right > left && isBlankChar(_text.mid(right-1,1)))
        right--;

    if (left == right) // 是空的，插入双引号
    {
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords(2);
        return ;
    }

    // 是否在选区里面的 function封装+Lambda表达式
    /*auto isInSelect = [=](QString str) { // function<bool(QString)>
        QString text = _text.mid(left, right-left);
        int pos = text.indexOf(str);
        return pos > -1;
    };*/

    QString text = _text.mid(left, right-left);
    if (text.indexOf("\n") == -1) // 选中单行
    {
        QString ll = left<= 0 ? "" : _text.mid(left-1, 1);
        QString lr = _text.mid(left, 1);
        QString rl = _text.mid(right-1, 1);
        QString rr = right >= _text.length() ? "" : _text.mid(right, 1);

        if ( (ll=="“" || lr=="“") && (rl=="”" || rr=="”") )   // 删除两边双引号
        {
            if (left < right-2) // 中间有文字
            {
                if (lr == "“")
                    deleteText(left, left+1);
                else
                    deleteText(left-1, left);
                if (rl == "”")
                    deleteText(right-1-1, right-1);
                else
                    deleteText(right, right+1);
            }
            else // 连续的
            {
                deleteText(left, right);
            }
        }
        else if (text.indexOf("“")>-1 && text.indexOf("”")>-1) // 里面有完整的双引号，删除里面的双引号
        {
            int pos = text.indexOf("“");
            deleteText(left+pos, left+pos+1);
            right--;
            text = _text.mid(left, right-left);
            pos = text.indexOf("”", pos);
            if (pos == -1)
                pos = text.indexOf("”");
            deleteText(left+pos, left+pos+1);
        }
        else if (_text.lastIndexOf("“", left) > -1 && _text.lastIndexOf("“", right) <= left && _text.indexOf("”", left) >= right) // 选中的区域内没有引号，且在引号里面
        {
            if (left == right)
            {
                insertText("‘’");
                moveCursor(-1);
                return ;
            }
            insertText(right, "’");
            insertText(left, "‘");
            _pos = right+1; // 右单引号前面
            ac->addUserWords(2);
        }
        else
        {
            insertText(left, "“");
            insertText(right+1, "”");
            _pos = right+2;
            ac->addUserWords(2);
        }
    }
    else // 选中多行
    {
        int have_quetos = 0, blank_para = 0;
        QStringList sp = text.split(QRegExp("\n"));
        for (QString str : sp)
        {
            if (isBlankString(str))
                blank_para++;
            else if (str.indexOf("“") > -1 && str.indexOf("”")>-1)
                have_quetos++;
        }
        if (have_quetos == sp.size() - blank_para ) // 每一个非空段都有双引号，去掉
        {
            QString result = "";
            for (QString str : sp)
            {
                result += str.replace(QRegExp("“|”"), "") + "\n";
            }
            if (result.size() > 0)
                result = result.mid(0, result.size()-1);
            deleteText(left, right);
            insertText(left, result);
        }
        else // 每段增加引号，从段首非空位置开始
        {
            QString result = "";
            for (QString str : sp)
            {
                if (!isBlankString(str) && str.indexOf("“")==-1 && str.indexOf("”")==-1) // 每一段增加一个双引号
                {
                    // 去掉左边的空格
                    int pos = 0, len = str.length();
                    while (pos < len && isBlankChar(str.mid(pos, 1)))
                        pos++;
                    if (pos == len)
                    {
                        result += str + "\n";
                        continue;
                    }
                    str = str.mid(0, pos) + "“" + str.mid(pos, len-pos);

                    // 去掉右边的空格
                    pos = len = str.length();
                    while (pos > 0 && isBlankChar(str.mid(pos-1, 1)))
                        pos--;
                    if (pos == 0)
                    {
                        result += str + "”" + "\n";
                        continue;
                    }
                    str = str.mid(0, pos) + "”" + str.mid(pos, len-pos);

                    result += str + "\n";
                }
                else // 空段落只或者已经有双引号的段落只恢复换行
                    result += str + "\n";
            }
            if (result.size() > 0)
                result = result.mid(0, result.size()-1);
            deleteText(left, right);
            insertText(left, result);
        }
    }
}



int NovelAIBase::getWordCount(QString str)
{
    return str.length();
}

/**
 * 智能删除：删除成对的标点（内部、右边）
 */
void NovelAIBase::operatorSmartBackspace()
{
    if (canDeletePairPunc()) ;
    else if (_left1 == _left2 && (_left1=="…" || _left1=="—" || isSentPunc(_left1)))
        deleteText(_pos - 2, _pos);
    /*else if (_left1 == _left2 && _left2 != _left3 && !isChinese(_left1) && _left1 != "\n") // 连续的字符
    {
        deleteText(_pos - 2, _pos);
    }*/
    else
    {
        deleteText(_pos - 1, _pos);
    }
}

/**
 * 智能空格：添加或者修改句末标点；
 * 标点的左右移动光标到右边；
 * 消除自动添加的双引号
 */
void NovelAIBase::operatorSmartSpace()
{
    if (_left1 == "" || _left1 == "\n")   // 增加缩进
    {
        QString insText = "";
        for (int i = 0; i < us->indent_blank; i++)
            insText += "　";
        insertText(insText);
    }
    else if (_left1 == "“" && _right1 == "”")   // 空的引号中间
    {
        deleteText(_pos - 1, _pos + 1);
        if (_left2 == "：")
        {
            deleteText(_pos - 1, _pos);
        }
        updateCursorChars();
        if (isChinese(_left1))
        {
            QString punc = getPunc();
            insertText(punc);
            ac->addUserWords();
        }
    }
    else if (_right1 == "，")
    {
        moveCursor(1);
    }
    else if (_left1 == "，")   // 逗号变句号
    {
        if (_right1 == "“")
        {
            moveCursor(1);
        }
        else
        {
            deleteText(_pos - 1, _pos);
            QString punc = getPunc2();
            insertText(punc);
            ac->addUserWords();

            if (punc == "！")
            	us->addClimaxValue(true);
            else if (punc == "！")
            	us->addClimaxValue(false);
        }
    }
    else if (isSentPunc(_right1))   // 跳过标点
    {
        moveCursor(1);
    }
    else if (isEnglish(_left1) || isNumber(_left1) || isEnglish(_right1) || isNumber(_right1))
    {
        insertText(" ");
    }
    else if (isASCIIPunc(_left1))
    {
        insertText(" ");
    }
    else if (isChinese(_left1))   // 添加标点
    {
        if (_right1 == "”")   // 判断需不需要插入一个标点
        {
            bool use_punc = true;
            int q_pos = _text.lastIndexOf("“", _pos);
            int n_pos = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
            if (q_pos > n_pos + 1) // 前引号左边是中文时不增加
            {
                QString cha = _text.mid(q_pos - 1, 1);
                if (isChinese(cha))
                {
                    use_punc = false;
                }
            }
            if (use_punc)   // 需要插入标点
            {
                QString punc = getPunc();
                insertText(punc);
                ac->addUserWords();

                if (punc == "！")
                	us->addClimaxValue(true);
                else if (punc == "！")
                	us->addClimaxValue(false);
            }
            else   // 直接跳过引号
            {
                moveCursor(1);
            }
        }
        else   // 插入一个标点
        {
            QString punc = getPunc();
            insertText(punc);
            ac->addUserWords();

            if (punc == "！")
            	us->addClimaxValue(true);
            else if (punc == "！")
            	us->addClimaxValue(false);
        }
    }
    else if (_right1 == "”")
    {
        moveCursor(1);
    }
    else if (isSentPunc(_left1) && _left1 != "，" && (_right1 != "" && _symbol_pair_rights.indexOf(_right1) > -1) )
    {
        moveCursor(1);
    }
    else if (us->space_quotes && (_left1 == "　" || isSentPunc(_left1)) && _right1 != "”")   // 空格引号
    {
        insertText("“”");
        moveCursor(-1);
        ac->addUserWords(2);
    }
    else if (isSentPunc(_left1) && _left1 != "，") // 句末标点 变成 逗号，或者跳转
    {
        deleteText(_pos - 1, _pos);
        insertText("，");
        ac->addUserWords();
    }
    else if (_left1 == "　")
    {
        insertText("　");
    }
    else if (_left1 == " ")
    {
        insertText(" ");
    }
    else   // 普通空格
    {
        insertText(" ");
    }
}

/**
 * 智能回车：句末标点左边右移一位；
 * 自动添加双引号；
 * 跳过双引号的右半部分；
 * 双引号里面句子换行则自动补全双引号
 */
void NovelAIBase::operatorSmartEnter()
{
    bool blank_line_cut = false; // 空行后面暂时减少一行

    // ==== 删除前后空白 ====
    int blank_start = _pos, blank_end = _pos;
    while (blank_start > 0 && isBlankChar2(_text.mid(blank_start-1, 1)))
        blank_start--;
    while (blank_end < _text.length() && isBlankChar2(_text.mid(blank_end, 1)))
        blank_end++;
    if (blank_end > blank_start)
        deleteText(blank_start, blank_end);

    // ==== 智能引号 ====
    if (_left1 == "“" && _right1 == "”")   // 空的双引号中间，删除
    {
        deleteText(_pos - 1, _pos + 1);
        updateCursorChars();
        if (_left1 == "，" || _left1 == "：")
        {
            deleteText(_pos - 1, _pos);
        }
        updateCursorChars();
        if (isChinese(_left1))
        {
            QString punc = getPunc2();
            insertText(punc);
            ac->addUserWords();
        }
    }
    else if (_left1 == "“" || _right1 == "“")   // 左1 是 前引号
    {
        if (_left1 == "“")
        {
            moveCursor(-1);
            updateCursorChars();
        }
        if (isChinese(_left1))    // 中文|”“
        {
            insertText(":");
            ac->addUserWords();
        }
        else if (_left1 == "，")   // 中文，|”“
        {
            deleteText(_pos - 1, _pos);
            insertText(":");
            ac->addUserWords();
        }
    }
    else if (isChinese(_left1) || (_left1 == "，" && isChinese(_left2))) // 左1 是中文
    {
        if (_left1 == "，") // 逗号变成句末标点，先删除
        {
            deleteText(_pos - 1, _pos);
            updateCursorChars();
        }
        if (isSentPunc(_right1))   // 右1是句末标点，移动一位
        {
            moveCursor(1);
            if (QString("-—…~").indexOf(_right1) > -1 && _right1 == _right2)   // 双标点，继续移动一位
            {
                moveCursor(1);
            }
            updateCursorChars();
        }
        else   // if (isBlankChar(_right1)) // 添加一个标点
        {
            QString punc = getPunc2();
            insertText(punc);
            ac->addUserWords();

            if (punc == "！")
            	us->addClimaxValue(true);
            else if (punc == "！")
            	us->addClimaxValue(false);
        }
    }
    else if (_left2 != "\n" &&  _left1 == "\n" && isBlankChar(_right1))    // 段落下一行的空行，很可能是不小心点错了位置
    {
        blank_line_cut = true;
    }
    else if (_right1 != "" && _symbol_pair_rights.indexOf(_right1) > -1 && (_right2 == "" || _right2 == "\n"))
    {
        moveCursor(1);
        updateCursorChars();
    }

    // ==== 跳过还是填充引号 ====
    if (_right1 == "”")   // 右1 是 右引号
    {
        moveCursor(1);
    }
    else if (isCursorInQuote(_text, _pos))   // 自动填充双引号
    {
        if (us->para_after_quote) // 多段后引号：插入后引号
            insertText("”“");
        else // 只插入前引号
            insertText("“");
        moveCursor(-1);
        ac->addUserWords();
    }

    // ==== 修改缩进 ====
    QString ins_text = "";
    int blank_line_num = us->indent_line;
    if (blank_line_cut) blank_line_num--;
    for (int i = 0; i <= blank_line_num; i++)
        ins_text += "\n";
    for (int i = 0; i < us->indent_blank; i++)
        ins_text += "　";
    insertText(ins_text);
}

/**
 * 获取当前所在的句子（仅限光标前面，后面的文字没什么用）
 * @return 当前句子
 */
QString NovelAIBase::getCursorFrontSent()
{
    int left = _pos;
    while (left > 0 && !isSentSplit(_text.mid(left-1, 1)))
        left--;
    return _text.mid(left, _pos-left);
}

/**
 * 在说话或者语气词后面自动添加标点
 * @return 是否自动添加
 */
bool NovelAIBase::operatorAutoPunc()
{
    //if (isChinese(_left1)) return false; // 运行时已经判断了，可注释掉
    if (_auto_punc_whitelists.indexOf(_left1) == -1) return false; // 首先要在白名单内
    if (_right1 != "”" && _right1 != " " && _right1 != "\n" && _right1 != "") return false; // 右1需要是空内容

    QString sent = getCursorFrontSent();
    QString punc;

    if (_left1 == "么")
    {
        if (QString("那这怎什多么").indexOf(_left2) > -1) return false;
        punc = getPunc();
    }
    else if (_left1 == "呵")
    {
        if (_left2 == "呵" && !isChinese(_left3))
            punc = "！";
        else
            return false;
    }
    else if (_left1 == "哈")
    {
        if (_left2 == "哈")
        {
            punc = "！";
        }
        else return false;
    }
    else if (_left1 == "诶")
    {
        if (!isChinese(_left2))
        {
            punc = "？";
        }
        else return false;
    }
    else if (_left1 == "呸")
    {
        if (!isChinese(_left2) || (_left2 == "我" && !isChinese(_left3)) )
        {
            punc = "！";
        }
        else return false;
    }
    else if (_left1 == "滚")
    {
        if (!isChinese(_left2))
        {
            punc = "！";
        }
        else return false;
    }
    else // 白名单内的其他词
    {
        punc = getPunc();
    }
    if (!punc.isEmpty())
    {
    	insertText(punc);
	    ac->addUserWords();

	    if (punc == "！")
	    	us->addClimaxValue(true);
	    else if (punc == "！")
	    	us->addClimaxValue(false);

	    return true;
    }
    return false;
}

bool NovelAIBase::operatorAutoDqm()
{
    //if (isChinese(_left1)) return false; // 运行时已经判断了，可注释掉
    //if (_auto_punc_whitelists.indexOf(_left1) == -1) return false; // 首先要在白名单内
    if (_right1 != "”" && _right1 != " " && _right1 != "\n" && _right1 != "") return false; // 右1需要是空内容

    QString sent = getCursorFrontSent();
    bool in_quote = isCursorInQuote(_text, _pos);

    if (_left1 == "说")
    {
        if (in_quote) return false;
        if (canRegExp(sent, _shuo_blacklists) /*_shuo_blacklists.indexOf(_left2) > -1*/ || isInQuotes) return false; // 黑白名单
        if (sent.indexOf("看") > -1 && sent.indexOf("看着") == -1) // 黑名单外的特殊判断
            return false;
        if (_text.lastIndexOf("“", _pos) > _text.lastIndexOf("”", _pos) && _text.lastIndexOf("“", _pos) > _text.lastIndexOf("\n", _pos)) return false; // 不在引号(同一段)外面
        operatorSmartQuotes();
        return true;
    }
    else if (_left1 == "道")
    {
        if (in_quote) return false;
        if (_dao_whitelists.indexOf(_left2) == -1 || isInQuotes) return false;
        if (_text.lastIndexOf("“", _pos) > _text.lastIndexOf("”", _pos) && _text.lastIndexOf("“", _pos) > _text.lastIndexOf("\n", _pos)) return false;
        operatorSmartQuotes();
        return true;
    }
    else if (_left1 == "问")
    {
        if (in_quote) return false;
        if (_wen_blacklists.indexOf(_left2) == -1 || isInQuotes) return false;
        if (_text.lastIndexOf("“", _pos) > _text.lastIndexOf("”", _pos) && _text.lastIndexOf("“", _pos) > _text.lastIndexOf("\n", _pos)) return false;
        operatorSmartQuotes();
        return true;
    }
    return false;
}

bool NovelAIBase::operatorSentFinish()
{
    if (isSentSplit(_left1) && _left1 != "，") return false; // 除了逗号以外的
    if (isSentPunc(_right1)) return false; // 标点（不包括逗号）
    if (isBlankChar(_left1)) return false; // 空白符
    if (_left1 == "，")
    {
        deleteText(_pos-1, _pos);
    }
    else if (_right1 == "，")
    {
        deleteText(_pos, _pos+1);
    }
    insertText(getPunc2());
    ac->addUserWords();
    return true;
}

/**
 * 获取句子标点的函数
 * @param  para 段落文本（包含光标所在的句子）
 * @param  pos  光标位置（相对于段落起始点）
 * @return      应该添加的标点
 */
QString NovelAIBase::getPunc(QString para, int pos)
{
    QString sent, punc = "，";
    int state = 0, doubt = 0, sigh = 0; // 陈述、疑问、感叹 的值
    Q_UNUSED(state);
    Q_UNUSED(doubt);
    Q_UNUSED(sigh);
    int tone = -1; // 语气导向，0陈述，1感叹，2问号

    // ==== 欲分析位置附近的标点 ====
    QString left1, left2, left3, right1, right2;
    if (pos > 0) left1 = para.mid(pos - 1, 1);
    else return punc;
    if (pos > 1) left1 = para.mid(pos - 2, 1);
    if (pos > 2) left1 = para.mid(pos - 3, 1);
    if (pos < para.length() - 1) left1 = para.mid(pos, 1);
    if (pos < para.length() - 2) left1 = para.mid(pos + 1, 1);
    if (isSentPunc(left1)) return punc;

    // === 如果在引号内部，则判断引号左边的语气 ===
    int quo_l = para.lastIndexOf("“", pos - 1), quo_r = para.lastIndexOf("”", pos - 1);
    if (quo_l > quo_r)
    {
        // 寻找语气所在的句子
        if (quo_r < 0) quo_r = 0;
        int desc_right = quo_l, desc_left = quo_l - 1;
        while (desc_left > quo_r && !isSentPunc(para.mid(desc_left - 1, 1)) )
            desc_left--;

        // 判断语气词
        tone = getDescTone(para.mid(desc_left, desc_right - desc_left));
    }

    // 获取当前短句子的位置
    int sent_right = pos, sent_left = pos - 1;
    while (sent_left > 0 && isChinese(para.mid(sent_left - 1, 1)) )
        sent_left--;

    // 获取当前长句子的位置
    int sent2_left = pos - 1;
    while (sent2_left > 0 && !isSentPunc(para.mid(sent_left - 1, 1)) )
        sent2_left--;

    // 根据短内容和长内容获取标点
    punc = getTalkTone(para.mid(sent_left, sent_right - sent_left), para.mid(sent2_left, sent_right - sent2_left), tone, left1, left2, isCursorInQuote(_text, _pos));

    return punc;
}

/**
 * 获取人物描写的语气（即语言描写最前面的句子）
 * @param  sent 人物语气动作神态描写的句子
 * @return      人物语气，-1默认，0陈述句，1感叹句，2疑问句
 */
int NovelAIBase::getDescTone(QString sent)
{
    int tone = -1;

    if (us->climax_value >= us->climax_threshold)
        tone = 1;
    else if ( sent.indexOf("轻") > -1)
        tone = 0;
    else if ( sent.indexOf("温") > -1)
        tone = 0;
    else if ( sent.indexOf("柔") > -1)
        tone = 0;
    else if ( sent.indexOf("悄") > -1)
        tone = 0;
    else if ( sent.indexOf("淡") > -1)
        tone = 0;
    else if ( sent.indexOf("静") > -1)
        tone = 0;
    else if ( sent.indexOf("小") > -1)
        tone = 0;
    else if ( sent.indexOf("问") > -1)
        tone = 2;
    else if ( sent.indexOf("疑") > -1)
        tone = 2;
    else if ( sent.indexOf("惑") > -1)
        tone = 2;
    else if ( sent.indexOf("不解") > -1)
        tone = 2;
    else if ( sent.indexOf("迷") > -1)
        tone = 2;
    else if ( sent.indexOf("好奇") > -1)
        tone = 2;
    else if ( sent.indexOf("试") > -1)
        tone = 2;
    else if ( sent.indexOf("探") > -1)
        tone = 2;
    else if ( sent.indexOf("询") > -1)
        tone = 2;
    else if ( sent.indexOf("诧") > -1)
        tone = 2;
    else if ( sent.indexOf("愤") > -1)
        tone = 1;
    else if ( sent.indexOf("恼") > -1)
        tone = 1;
    else if ( sent.indexOf("咬") > -1)
        tone = 1;
    else if ( sent.indexOf("怒") > -1)
        tone = 1;
    else if ( sent.indexOf("骂") > -1)
        tone = 1;
    else if ( sent.indexOf("狠") > -1)
        tone = 1;
    else if ( sent.indexOf("火") > -1)
        tone = 1;
    else if ( sent.indexOf("重") > -1)
        tone = 1;
    else if ( sent.indexOf("抓") > -1)
        tone = 1;
    else if ( sent.indexOf("狂") > -1)
        tone = 1;
    else if ( sent.indexOf("叫") > -1)
        tone = 1;
    else if ( sent.indexOf("喊") > -1)
        tone = 1;
    else if ( sent.indexOf("力") > -1)
        tone = 1;
    else if ( sent.indexOf("大") > -1)
        tone = 1;
    else if ( sent.indexOf("哮") > -1)
        tone = 1;
    else if ( sent.indexOf("厉") > -1)
        tone = 1;
    else if ( sent.indexOf("斥") > -1)
        tone = 1;
    else if ( sent.indexOf("吼") > -1)
        tone = 1;
    else if ( sent.indexOf("气") > -1)
        tone = 1;
    else if ( sent.indexOf("震") > -1)
        tone = 1;
    else if ( sent.indexOf("喜") > -1)
        tone = 1;
    else if ( sent.indexOf("惊") > -1)
        tone = 1;
    else if ( sent.indexOf("忙") > -1)
        tone = 1;
    else if ( sent.indexOf("瞪") > -1)
        tone = 1;
    else if ( sent.indexOf("嗔") > -1)
        tone = 1;
    else if ( sent.indexOf("暴") > -1)
        tone = 1;
    else if ( sent.indexOf("咒") > -1)
        tone = 1;
    else if ( sent.indexOf("红") > -1)
        tone = 1;
    else if ( sent.indexOf("痛") > -1)
        tone = 1;
    else if ( sent.indexOf("恐") > -1)
        tone = 1;
    else if ( sent.indexOf("憎") > -1)
        tone = 1;
    else if ( sent.indexOf("眦") > -1)
        tone = 1;
    else if ( sent.indexOf("悲") > -1)
        tone = 1;
    else if ( sent.indexOf("狂") > -1)
        tone = 1;
    else if ( sent.indexOf("重") > -1)
        tone = 1;
    else if ( sent.indexOf("躁") > -1)
        tone = 1;
    else if ( sent.indexOf("铁青") > -1)
        tone = 1;
    else if ( sent.indexOf("狠") > -1)
        tone = 1;
    else if ( sent.indexOf("恨") > -1)
        tone = 1;
    else if ( sent.indexOf("齿") > -1)
        tone = 1;
    else if ( sent.indexOf("急") > -1)
        tone = 1;
    else if ( sent.indexOf("变") > -1)
        tone = 1;
    else if ( sent.indexOf("冲") > -1)
        tone = 1;
    else if ( sent.indexOf("激") > -1)
        tone = 1;
    else if ( sent.indexOf("恶") > -1)
        tone = 1;
    else if ( sent.indexOf("绝") > -1)
        tone = 1;
    else if ( sent.indexOf("瞪") > -1)
        tone = 1;
    else if ( sent.indexOf("愁") > -1)
        tone = 1;
    else if ( sent.indexOf("羞") > -1)
        tone = 1;
    else if ( sent.indexOf("恼") > -1)
        tone = 1;
    else if ( sent.indexOf("忿") > -1)
        tone = 1;
    else if ( sent.indexOf("凶") > -1)
        tone = 1;
    else if ( sent.indexOf("连") > -1)
        tone = 1;
    else if ( sent.indexOf("热") > -1)
        tone = 1;
    else if ( sent.indexOf("欢") > -1)
        tone = 1;
    else if ( sent.indexOf("万") > -1)
        tone = 1;
    else if ( sent.indexOf("得") > -1)
        tone = 1;
    else if ( sent.indexOf("叹") > -1)
        tone = 1;
    else if ( sent.indexOf("兴") > -1)
        tone = 1;
    else if ( sent.indexOf("不已") > -1)
        tone = 1;
    else if ( sent.indexOf("舞") > -1)
        tone = 1;
    else if ( sent.indexOf("天天") > -1)
        tone = 1;
    else if ( sent.indexOf("高") > -1)
        tone = 1;
    else if ( sent.indexOf("昂") > -1)
        tone = 1;
    else if ( sent.indexOf("澎湃") > -1)
        tone = 1;
    else if ( sent.indexOf("颤") > -1)
        tone = 1;
    else if ( sent.indexOf("慌") > -1)
        tone = 1;
    else if ( sent.indexOf("骇") > -1)
        tone = 1;
    else if ( sent.indexOf("跳") > -1)
        tone = 1;
    else if ( sent.indexOf("皆") > -1)
        tone = 1;
    else if ( sent.indexOf("怵") > -1)
        tone = 1;
    else if ( sent.indexOf("霹雳") > -1)
        tone = 1;
    else if ( sent.indexOf("急") > -1)
        tone = 1;
    else if ( sent.indexOf("忙") > -1)
        tone = 1;
    else
        tone = -1;

    return tone;
}

/**
 * 取标点
 * @param  sent  短句子（逗号分隔）
 * @param  sent2 完整句子（逗号不分隔）
 * @param  tone  影响的语气（-1默认，0陈述，1感叹，2疑问）
 * @param  left1 左1
 * @param  left2 左2
 * @return       应该添加的标点
 */
QString NovelAIBase::getTalkTone(QString sent, QString sent2, int tone, QString left1, QString left2, bool in_quote)
{
    Q_UNUSED(sent2);
    Q_UNUSED(left1);
    Q_UNUSED(left2);

    QString punc = "，";

    if (sent.indexOf("是不") > -1)
        if (tone == 0)
            punc = "，";
        else if (tone == 1)
            punc = "！";
        else if (canRegExp(sent, "是不.的") == true)
            ;
        else if (sent.indexOf("本来是不") > -1)
            ;
        else
            punc = "？";
    else if (sent.indexOf("是么") > -1)
        punc = "？";
    else if (sent.indexOf("管不管") > -1)
        punc = "？";
    else if (sent.indexOf("不管") > -1)
        ;
    else if (sent.indexOf("反正") > -1)
        ;
    else if (sent.indexOf("怎么知") > -1)
        punc = "？";
    else if (sent.indexOf("么会") > -1)
        punc = "？";
    else if (sent.indexOf("真") > -1 && sent.indexOf("真理") == -1 && sent.indexOf("真假") == -1 && sent.indexOf("真事") == -1 && !canRegExp(sent, "真.{0,3}不是"))
        if (sent.indexOf("真的") > -1 || sent.indexOf("真是") > -1)
            if (sent.indexOf("真的是") > -1 || sent.indexOf("真是") > -1)
                if (sent.indexOf("啊") > -1)
                    punc = "！";
                else if (sent.indexOf("吗") > -1)
                    punc = "？";
                else if (sent.indexOf("啦") > -1)
                    punc = "！";
                else if (sent.indexOf("呀") > -1)
                    punc = "！";
                else if (sent.indexOf("了") > -1)
                    if (sent.indexOf("太") > -1 || sent.indexOf("好"))
                        punc = "！";
                    else
                        punc = "？";
                else if (tone == 1)
                    punc = "！";
                else if (tone == 2)
                    punc = "？";
                else if (sent.indexOf("真是不") > -1)
                    punc = "！";
                else
                {}
            else if (sent == "真的")
                if (tone == 2)
                    punc = "？";
                else if (sent.indexOf("好吗"))
                    punc = "？";
                else if (sent.indexOf("太") > -1 || sent.indexOf("好"))
                    punc = "！";
                else
                {}
            else if (sent.indexOf("真的好") > -1 || sent.indexOf("真的很") > -1 || sent.indexOf("真的非常") > -1 || sent.indexOf("真的太") > -1 || sent.indexOf("真的特") > -1)
                if (sent.indexOf("吗") > -1)
                    punc = "？";
                else if (tone == 0)
                    ;
                else if (tone == 2)
                    punc = "？";
                else
                    punc = "！";
            else if (sent.indexOf("怎么") > -1)
                if (canRegExp(sent, "怎么也.*不"))
                    ;
                else
                    punc = "？";
            else if (sent.indexOf("难道") > -1)
                punc = "？";
            else if (isKnowFormat(sent) == true)
                ;
            else if (sent.indexOf("真的是我") > -1)
                ;
            else if (sent.indexOf("如何") > -1)
                punc = "？";
            else if (sent.endsWith("啊") || sent.endsWith("啦") || sent.endsWith("呀") || sent.endsWith("呢"))
                punc = "！";
            else if (sent.endsWith("吗"))
                punc = "？";
            else if (tone == 1)
                punc = "！";
            else if (tone == 2)
                punc = "？";
            else if (sent.indexOf("真的不是") > -1)
                ;
            else if (sent.indexOf("不是") > -1)
                punc = "？";
            else
            {}
        else if (sent.indexOf("你真") > -1)
            if (sent.indexOf("不") > -1 || sent.indexOf("了") > -1)
                if (tone == 1)
                    punc = "！";
                else if (tone == 0)
                    ;
                else
                    punc = "？";
            else if (tone == 2 || sent.endsWith("吗"))
                punc = "？";
            else if (tone == 0)
                ;
            else if (sent.endsWith("啊") || sent.endsWith("啦") || sent.endsWith("呀") || sent.endsWith("呢"))
                punc = "！";
            else if (sent.endsWith("吗"))
                punc = "？";
            else
                punc = "！";
        else if (sent.indexOf("他真") > -1)
            if (sent.indexOf("不") > -1 || sent.indexOf("了") > -1)
                if (tone == 1)
                    punc = "！";
                else if (tone == 0)
                    ;
                else
                    punc = "？";
            else if (tone == 2)
                punc = "？";
            else if (tone == 0)
                ;
            else if (sent.endsWith("啊") || sent.endsWith("啦") || sent.endsWith("呀") || sent.endsWith("呢"))
                punc = "！";
            else if (sent.endsWith("吗"))
                punc = "？";
            else
                punc = "！";
        else if (sent.indexOf("她真") > -1)
            if (sent.indexOf("不") > -1 || sent.indexOf("了") > -1)
                if (tone == 1)
                    punc = "！";
                else if (tone == 0)
                    ;
                else
                    punc = "？";
            else if (tone == 2)
                punc = "？";
            else if (tone == 0)
                ;
            else if (sent.endsWith("啊") || sent.endsWith("啦") || sent.endsWith("呀") || sent.endsWith("呢"))
                punc = "！";
            else if (sent.endsWith("吗"))
                punc = "？";
            else
                punc = "！";
        else if (isKnowFormat(sent) == true)
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 2)
            punc = "？";
        else if (sent.endsWith("啊") || sent.endsWith("啦") || sent.endsWith("呀") || sent.endsWith("呢"))
            punc = "！";
        else if (sent.endsWith("吗"))
            punc = "？";
        else
        {}
    else if (sent.indexOf("太") > -1 && QString("了啦吧").indexOf(sent.right(1)) > -1) // 带“太”的都是厉害的，除了部分名词，应该都是感叹号
        punc = "！";
    else if (sent.indexOf("是否") > -1)
        if (isKnowFormat(sent) == true)
            ;
        else
            punc = "？";
    else if (sent.indexOf("是不是") > -1)
        if (isKnowFormat(sent) == true)
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else
            punc = "？";
    else if (sent.indexOf("可能是") > -1 && canRegExp(sent, ".*[特别|真的|格外|非常].*") == true)
        punc = "！";
    else if (sent.indexOf("知不知") > -1)
        punc = "？";
    else if (sent.indexOf("需不需") > -1)
        if (isKnowFormat(sent) == true && sent.indexOf("怎么") == -1 && sent.indexOf("为什么") == -1)
            ;
        else
            punc = "？";
    else if (sent.indexOf("要不要") > -1)
        if (isKnowFormat(sent) == true)
            ;
        else if (sent.indexOf("你") == -1)
            punc = "？";
        else if (sent.indexOf("犹豫要") > -1 || sent.indexOf("在想") > -1 || sent.indexOf("思考") > -1)
            ;
        else
            punc = "？";
    else if (sent.indexOf("要不是") > -1)
        ;
    else if (canRegExp(sent, "(.{1,2})不\\1") == true)
        if (isKnowFormat(sent) == true && sent.indexOf("怎么") == -1 && sent.indexOf("为什么") == -1 && sent.indexOf("难道") == -1)
            ;
        else if (sent.indexOf("时不时") > -1)
            ;
        else
            punc = "？";
    else if (sent.length()  > 2 && sent.right(1) == "不" && sent.indexOf("不不") == -1)
        punc = "？";
    else if ( sent.indexOf("还不") > -1 || canRegExp(sent, "^(你他她)们?(怎么|为什么|干嘛|为何)还(没|不)"))
        if (sent.indexOf("给我") > -1)
            punc = "！";
        else if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else if (sent.contains("甚至"))
            ;
        else if (sent.indexOf("吧") > -1)
            punc = "？";
        else if (sent.indexOf("吗") > -1)
            punc = "？";
        else if (sent.indexOf("嘛") > -1)
            punc = "？";
        else if (sent.indexOf("啊") > -1)
            punc = "！";
        else if (sent.indexOf("不至于") > -1)
            ;
        else if (sent.indexOf("不如") > -1)
            ;
        else
            punc = "？";
    else if (sent.indexOf("不要") > -1 && sent.indexOf("吗") == -1 && sent.indexOf("吧") == -1 && sent.indexOf("呢") == -1 && sent.indexOf("了") == -1)
        if (in_quote)
            punc = "！";
        else
            ;
    else if (sent.indexOf("不可思议") > -1)
        punc = "！";
    else if (sent.indexOf("誓不") > -1)
        punc = "！";
    else if (sent.indexOf("都要") > -1 && sent.indexOf("吗") == -1 && sent.indexOf("吧") == -1 && sent.indexOf("呢") == -1 && sent.indexOf("了") == -1)
        if (sent.indexOf("你都要") > -1 && sent.indexOf("这") > -1 && sent.indexOf("这") > sent.indexOf("你都要"))
            punc = "？";
        else
            punc = "！";
    else if (sent == "我要")
        punc = "！";
    else if (sent == "不要")
        punc = "！";
    else if (sent == "要")
        ;
    else if (sent == "反正")
        ;
    else if (sent == "那就是说" && sent.length()  > 6 && sent.right(1) == "了")
        if (tone == 1)
            punc = "！";
        else if (tone == 0)
            punc = "。";
        else
            punc = "？";
    else if (sent.indexOf("绝对") > -1)
        punc = "！";
    else if (sent.indexOf("一定") > -1 && sent.indexOf("有一定") == -1 && sent.indexOf("一定的") == -1 && sent.indexOf("不一定") == -1 && sent.indexOf("一定的") == -1)
        punc = "！";
    else if (sent.indexOf("你居然") > -1)
        punc = "！";
    else if (sent.indexOf("甚至") > -1)
        punc = "！";
    else if (sent.indexOf("简直") > -1)
        punc = "！";
    else if (sent.indexOf("必定") > -1)
        punc = "！";
    else if (sent.indexOf("要不") > -1)
        if (sent == "要不")
            punc = "，";
        else
            punc = "？";
    else if (sent.indexOf("可不") > -1 && (sent.contains("可不可") || sent.endsWith("可不")))
        punc = "？";
    else if (sent.indexOf("行不") > -1)
        punc = "？";
    else if (sent.indexOf("不就") > -1)
        punc = "？";
    else if (sent.indexOf("多少") > -1)
        if (sent.indexOf("没") > -1)
            ;
        else if (sent.indexOf("多少") < sent.indexOf("是"))
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else if (isKnowFormat(sent) == true)
            ;
        else
            punc = "？";
    else if (sent.indexOf("多久") > -1)
        if (sent.indexOf("没") > -1 && sent.indexOf("没") < sent.indexOf("多久"))
            ;
        else if (sent.indexOf("没多久") > -1)
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else if (isKnowFormat(sent) == true)
            ;
        else
            punc = "？";
    else if (sent.indexOf("有多") > -1)
        if (tone == 1)
            punc = "！";
        else if (tone == 0)
            punc = "。";
        else if (isKnowFormat(sent) == true)
            ;
        else
            punc = "？";
    else if (sent.indexOf("都要") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (canRegExp(sent, ".{1,3}(无论|不管)(怎么|什么|任意|任何)") && !sent.contains("真的"))
        ;
    else if (sent.indexOf("么") > -1)
        if (sent.indexOf("什么") > -1)
        {
            int shenme_pos = sent.indexOf("什么");
            if (tone == 1)
                punc = "！";
            else if (tone == 2)
                punc = "？";
            else if (tone == 0)
                ;
            else if (sent.indexOf("还") > -1)
                if (isKnowFormat(sent) == true)
                    ;
                else if (sent.indexOf("以为") > -1)
                    ;
                else
                    punc = "？";
            else if (sent.indexOf("似乎") > -1)
                ;
            else if (sent.indexOf("好像") > -1)
                ;
            else if (sent.contains("不") && sent.indexOf("不") < sent.indexOf("什么"))
                ;
            else if (sent.indexOf("不了") > -1)
                if (tone == 1)
                    punc = "！";
                else
                {}
            else if (sent.indexOf("怎么") == -1 && canRegExp(sent, "什么.*都"))
                ;
            else if (sent.indexOf("都不") > -1)
                ;
            else if (sent.indexOf("都") > -1 && sent.indexOf("来") > -1)
                ;
            else if (sent.indexOf("或许") > -1)
                ;
            else if (sent.indexOf("说不定") > -1)
                ;
            else if (sent.indexOf("可能") > -1)
                ;
            else if (sent.indexOf("仿佛") > -1)
                ;
            else if (sent.indexOf("要发生什么") > -1)
                ;
            else if (sent.indexOf("没") > -1 && sent.indexOf("没") < shenme_pos)
                ;
            else if (sent.indexOf("肯定") > -1 && sent.indexOf("肯定") < shenme_pos)
                ;
            else if (sent.indexOf("决定") > -1)
                ;
            else if (sent.indexOf("不出") > -1)
                ;
            else if (sent.indexOf("不是") > -1 && sent.indexOf("不是") < shenme_pos)
                ;
            else if (sent.indexOf("不到") > -1 && sent.indexOf("不到") < shenme_pos)
                ;
            else if (sent.indexOf("不了") > -1)
                ;
            else if (sent.indexOf("多么") > -1)
                if (sent.indexOf("啊") > -1)
                    punc = "！";
                else
                {}
            else if (sent.indexOf("什么都") > -1)
                if (sent.indexOf("怎么") > -1)
                    if (sent.indexOf("不") > -1)
                        ;
                    else
                        punc = "？";
                else if (sent.indexOf("什么都要") > -1)
                    punc = "！";
                else
                {}
            else if (sent.indexOf("什么的") > -1)
                ;
            else if (sent.indexOf("的什么东西") > -1)
                ;
            else if (sent.indexOf("说了什么") > -1 && sent.indexOf("的", sent.indexOf("说了什么") + 1) > -1)
                ;
            else if (sent.indexOf("到底") > -1)
                punc = "！";
            else if (sent.indexOf("情况") > -1)
                punc = "？";
            else if (isKnowFormat(sent) == true)
                ;
            else
                punc = "？";
        }
        else if (sent.indexOf("怎么") > -1 && sent.indexOf("不怎么") == -1 && sent.indexOf("怎么也") == -1)
            if (isKnowFormat(sent) == true && sent.indexOf("都知道") == -1)
                if (sent.indexOf("怎么知") > -1)
                    punc = "？";
                else if (sent.indexOf("怎么懂") > -1)
                    punc = "？";
                else if (canRegExp(sent, "我.*[教|告|诉|帮].*怎么"))
                    ;
                else if (tone == 2)
                    punc = "？";
                else if (tone == 1)
                    punc = "！";
                else
                {}
            else if (tone == 1)
                punc = "！";
            else if (tone == 0)
                ;
            else if (sent == "怎么")
                punc = "，";
            else if (sent.indexOf("吗") > -1)
                punc = "？";
            else if (sent.indexOf("啊") > -1)
                punc = "？";
            else if (sent.indexOf("吧") > -1)
                punc = "？";
            else if (sent.indexOf("呢") > -1)
                punc = "？";
            else if (sent.indexOf("嘛") > -1)
                punc = "？";
            else if (sent.indexOf("看到") > -1)
                ;
            else if (sent.indexOf("其实") > -1)
                ;
            else if (sent.indexOf("发现") > -1)
                ;
            else if (sent.indexOf("怎么就") > -1)
                punc = "！";
            else
                punc = "？";
        else if (sent.indexOf("要么") > -1)
            punc = "，";
        else if (sent.indexOf("不怎") > -1)
            ;
        else if (sent.indexOf("怎样") > -1)
            if (isKnowFormat(sent) == true || sent.indexOf("不怎样") > -1)
                ;
            else
                punc = "？";
        else if (sent.indexOf("么么") > -1)
            punc = "~";
        else if (sent.indexOf("么又") > -1)
            if (tone == 1)
                punc = "！";
            else if (tone == 0)
                ;
            else
                punc = "？";
        else if (sent.indexOf("这么") > -1 && tone == 2)
            punc = "？";
        else if (sent.indexOf("这么") > -1)
            punc = "！";
        else if (sent.indexOf("那么") > -1 && (tone == 1 || sent.endsWith("吧")) )
            punc = "！";
        else if (sent.indexOf("那么") > -1 && (tone == 2 || sent.endsWith("吗")))
            punc = "？";
        else if (sent.indexOf("多么") > -1)
            if (sent.indexOf("啊") > -1 || sent.indexOf("呀") > -1 || sent.indexOf("呢") > -1 || sent.indexOf("诶") > -1)
                punc = "！";
            else
            {}
        else if (sent.indexOf("饿了么") > -1)
            ;
        else
            punc = "？";
    else if (sent.indexOf("难道") > -1)
        if (sent == "难道")
            punc = "，";
        else
            punc = "？";
    else if (sent.indexOf("怎样") > -1 && sent.indexOf("不怎样") == -1)
        if (isKnowFormat(sent) == true && sent.indexOf("我") > -1 && sent.indexOf("怎么") == -1 && sent.indexOf("知") < sent.indexOf("怎样") && sent.indexOf("明") < sent.indexOf("怎样") && sent.indexOf("懂", 0) < sent.indexOf("怎样"))
            ;
        else
            punc = "？";
    else if (sent.indexOf("何") > -1)
        if (sent.indexOf("如何") > -1)
            if (sent.indexOf("无论") > -1 || sent.indexOf("不管") > -1 || isKnowFormat(sent) == true)
                if (tone == 1)
                    punc = "！";
                else
                {}
            else
                punc = "？";
        else if (sent.indexOf("任何") > -1)
            if (tone == 2 && isKnowFormat(sent) == false)
                punc = "？";
            else if (tone == 1)
                punc = "！";
            else
            {}
        else if (sent.indexOf("为何") > -1)
            if (isKnowFormat(sent) == true)
                if (sent.indexOf("何不知") > -1)
                    punc = "？";
                else
                {}
            else
                punc = "？";
        else if (sent.indexOf("何况") > -1 || sent.indexOf("何人") > -1 || sent.indexOf("何事") > -1 || sent.indexOf("何时") > -1 || sent.indexOf("何且") > -1)
            punc = "？";
        else if (sent.indexOf("何等") > -1)
            if (sent.indexOf("啊") > -1)
                punc = "！";
            else
            {}
        else if (tone == 0)
            ;
        else if (tone == 1)
            punc = "！";
        else if (sent.indexOf("几何") > -1)
            ;
        else if (sent.indexOf("何来") > -1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("几") > -1 && ( tone == 2 || sent.contains("几点") || sent.contains("几时") || sent.contains("样了") ||
        (sent.startsWith("几") && sent.endsWith("了")) || (sent.contains("你") && sent.indexOf("你") < sent.indexOf("几")) ) )
        punc = "？";
    else if (sent.indexOf("谁") > -1 && sent.indexOf("谁也") == -1)
        if (sent.indexOf("谁知") > -1 && sent.indexOf("谁知") < sent.indexOf("就"))
            punc = "！";
        else if (sent.indexOf("谁知") > -1 && sent.contains("然"))
            ;
        else if (tone == 1)
            punc = "！";
        else if (sent.indexOf("谁说") > -1 || sent.indexOf("谁让") > -1)
            punc = "？";
        else
            punc = "？";
    else if (canRegExp(sent, "当.+时") == true)
        if (sent.indexOf("难道") > -1)
            punc = "？";
        else
        {}
    else if (sent.indexOf("啥") > -1)
        if (isKnowFormat(sent) == true)
            punc = "。";
        else
            punc = "？";
    else if (sent.indexOf("哪") > -1)
        if (tone == 0)
            punc = "。";
        else if (tone == 1)
            punc = "！";
        else if (sent.indexOf("天哪") > -1)
            punc = "！";
        else if (sent.indexOf("哪怕") > -1)
            ;
        else if (sent == "哪里")
            ;
        else
            punc = "？";
    else if (sent.indexOf("居然") > -1 && sent.length()  >= 3 && sent.right(3).indexOf("居然") > -1)
        punc = "……";
    else if (sent.indexOf("居然") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            if (sent.length() > 5)
                punc = "。";
            else
            {}
        else if (sent.indexOf("知") > -1 || sent.indexOf("发") > -1 || sent.indexOf("到") > -1)
            ;
        else
            punc = "！";
    else if ((sent.startsWith("虽然") || sent.startsWith("然而") || sent.startsWith("但") || sent.startsWith("最后") || sent.indexOf("接着") > -1 || sent.indexOf("然后") > -1 || sent.indexOf("之后") > -1 || sent.indexOf("至少") > -1)
             && QString("吗呢吧呀啊").indexOf(sent.right(1)) == -1)
        ;
    else if (sent.indexOf("也不能") > -1 && sent.indexOf("也不能")<= 2 && QString("吗呢").indexOf(sent.right(1)) == -1)
        ;
    else if (sent.indexOf("听说") > -1)
        if (tone == 0 || sent.indexOf("就听说") > -1 || sent.indexOf("一些") || sent.indexOf("还没") || sent.indexOf("不是") > sent.indexOf("听说")
                || sent == "听说" || sent.indexOf("前") > sent.indexOf("听说") || sent.indexOf("时") > -1 || sent.indexOf("听说过") > -1 || sent.indexOf("却") > -1
                || (sent.indexOf("没听说") > -1 && QString("吧吗啊").indexOf(sent.right(1)) > -1) || sent.indexOf("都听说") > -1
                || sent.indexOf("她") > sent.indexOf("听说")|| sent.indexOf("他") > sent.indexOf("听说")|| sent.indexOf("它") > sent.indexOf("听说")
                || sent.endsWith("听说") || sent.indexOf("这") > -1)
            ;
        else if (tone == 1)
            punc = "！";
        else if (canRegExp(sent, "听说你.*?什么都.+"))
            punc = "?";
        else
            punc = "？";
    else if (sent.indexOf("谢谢") > -1)
        if (sent == "谢谢" || sent == "谢谢你" || sent == "谢谢您") // 为了效率才特判
            punc = "！";
        else if (tone == 2 || sent.endsWith("吗"))
            punc = "？";
        else if (sent.endsWith("呦"))
            punc = "~";
        else if (canRegExp(sent, "谢谢.*[他她它]"))
            ;
        else
            punc = "！";
    else if (sent.indexOf("多谢") > -1)
        punc = "！";
    else if (sent.indexOf("貌似") > -1)
        if (tone == 0)
            ;
        else if (tone == 1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("有没有") > -1 || sent.indexOf("有木有") > -1)
        if (sent.indexOf("知道") > -1)
            ;
        else if (tone == 1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("至少") > -1)
        if (sent == "至少")
            punc = "，";
        else if (tone == 0)
            ;
        else if (tone == 2)
            punc = "？";
        else
            punc = "！";
    else if (sent.indexOf("想必") > -1)
        if (tone == 0)
            ;
        else
            punc = "？";
    else if (sent.indexOf("站住") > -1)
        punc = "！";
    else if (sent.indexOf("然又") > -1 || sent.indexOf("又来") > -1)
        if (tone == 2 || sent.indexOf("吗") > -1)
            punc = "？";
        else
            punc = "！";
    else if (sent.indexOf("了没") > -1 && isKnowFormat(sent) == false)
        punc = "？";
    else if (sent.indexOf("了什") > -1 && isKnowFormat(sent) == false )
        punc = "？";
    else if (sent.indexOf("不知") > -1)
        if (isKnowFormat(sent) == true && sent.right(2) == "知道")
            punc = "，";
        else if (tone == 0)
            ;
        else if (tone == 1)
            punc = "！";
        else if (sent.indexOf("知过") > -1)
            ;
        else if (sent.length()  > 7 && sent.left(3) == "不知道" && sent.right(1) == "的")
            punc = "？";
        else if (sent.indexOf("然不知") > -1)
            punc = "，";
        else if (sent.indexOf("知所") > -1)
            punc = "，";
        else if (sent.indexOf("知不") > -1)
            punc = "，";
        else if (sent.indexOf("知者") > -1)
            punc = "，";
        else if (sent.indexOf("知火") > -1)
            punc = "，";
        else if (sent.indexOf("知之") > -1)
            punc = "，";
        else if (sent.indexOf("知的") > -1)
            punc = "，";
        else if (sent.indexOf("还不") > -1 && sent.indexOf("呢") > -1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("干嘛") > -1)
        if ( sent.indexOf("你") == -1 && isKnowFormat(sent) == true)
            punc = "，";
        else
            punc = "？";
    else if (sent.indexOf("也算") > -1 || sent.indexOf("算是") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 1)
            punc = "！";
        else if (sent.indexOf("吗") > -1)
            punc = "？";
        else if (sent.indexOf("呢") > -1)
            punc = "！";
        else if (sent.indexOf("嘛") > -1)
            punc = "！";
        else if (sent.indexOf("吧") > -1)
            if (sent.indexOf("这") > -1 || sent.indexOf("那") > -1 )
                if (sent.indexOf("也算") > -1)
                    punc = "？";
                else
                    punc = "！";
            else
                punc = "！";
        else
        {}
    else if (sent.indexOf("百分百") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (sent.indexOf("听说你") > -1 || sent.indexOf("听说他") > -1 || sent.indexOf("听说她") > -1 || sent.indexOf("听说这") > -1 || sent.indexOf("听说那") > -1)
        if (tone == 2 || sent.indexOf("啊") > -1 || sent.indexOf("吗") > -1 || sent.indexOf("吧") > -1)
            punc = "？";
        else if (tone == 0)
            ;
        else if (tone == 1 || sent.indexOf("呢") > -1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("彻底") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (sent.indexOf("到底") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (sent.endsWith("极了"))
        punc = "！";
    else if (sent.indexOf("岂有此理") > -1)
        punc = "！";
    else if (sent.indexOf("恐怖如此") > -1)
        punc = "！";
    else if (sent.indexOf("岂") > -1)
        punc = "？";
    else if (sent.indexOf("真的") > -1)
        punc = "？";
    else if (sent.indexOf("而且是") > -1)
        punc = "？";
    else if (sent.indexOf("多久") > -1)
        if (isKnowFormat(sent) == true)
            ;
        else
            punc = "？";
    else if (sent.indexOf("莫非") > -1)
        if (sent == "莫非")
            punc = "……";
        else
            punc = "？";
    else if (sent.indexOf("其实") > -1)
        if (sent == "其实")
            ;
        else
        {}
    else if (sent == "当心" || sent == "小心")
        punc = "！";
    else if (sent.indexOf("当真") > -1)
        punc = "？";
    else if (sent.indexOf("你敢") > -1)
        if (sent == "你敢")
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("你确定") > -1)
        punc = "？";
    else if (sent.indexOf("你肯定") > -1)
        punc = "？";
    else if (sent.indexOf("定") > -1 && canRegExp(sent, "[必定|一定|肯定|定要|定把|定将|定会|定能|定可|定是|定非]") == true && sent.indexOf("不一定") == -1 && sent.indexOf("不") == -1 && sent.indexOf("确定") == -1 && sent.indexOf("稳定") == -1 && sent.indexOf("待定") == -1 && sent.indexOf("定理") == -1 && sent.indexOf("定义") == -1 && sent.indexOf("不定") == -1 && sent.indexOf("没") == -1 && sent.indexOf("定时") == -1 && sent.indexOf("定期") == -1 && sent.indexOf("安定") == -1 && sent.indexOf("设定") == -1 && sent.indexOf("定点") == -1 && sent.indexOf("平定") == -1 && sent.indexOf("定力") == -1)
        if (sent.indexOf("你确定") > -1)
            punc = "？";
        else if (sent.indexOf("确定") > -1)
            ;
        else
            punc = "！";
    else if (sent.indexOf("滚") > -1 && sent.indexOf("打滚") == -1 && sent.indexOf("翻滚") == -1 && sent.indexOf("滚动") == -1 && sent.indexOf("靠滚") == -1)
        punc = "！";
    else if (sent.indexOf("混账") > -1 || sent.indexOf("混蛋") > -1 || sent.indexOf("可恶") > -1 || sent.indexOf("变态") > -1 || sent.indexOf("难以置信") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (sent.indexOf("相信") > -1)
        if (sent == "我相信" || sent == "相信")
            punc = "，";
        else
            punc = "！";
    else if (sent.indexOf("不信") > -1)
        punc = "！";
    else if (sent.indexOf("加油") > -1)
        punc = "！";
    else if (sent.indexOf("还是") > -1)
        if (tone == 0)
            ;
        else if (isInQuotes == true)
            if (tone == 2)
                punc = "？";
            else
                punc = "！";
        else if (sent == "还是")
            punc = "？";
        else if (sent.right(1) == "的" || sent.indexOf("果然") > -1)
            ;
        else if (sent.indexOf("居然") > -1 || sent.indexOf("竟然") > -1)
            punc = "！";
        else if (sent.indexOf("还是没有") > -1 && sent.indexOf("你还是") == -1)
            ;
        else
        {
            if (QString("吗嘛呢").indexOf(_left1) > -1)
                punc = "？";
            else if (_left1 == "吧")
                punc = "！";
            else if (sent.indexOf("我还是") > -1)
                punc = "！";
            else if (_left1 == "但还是")
                ;
            else if (QString("吧啊嘛哈").indexOf(_left1) > -1)
                punc = "！";
            else
            {}
        }
    else if (sent.indexOf("不可能") > -1)
        if (tone == 1)
            punc = "！";
        else if (tone == 2)
            punc = "？";
        else
        {}
    else if (sent.startsWith("你还在") || sent.startsWith("你也在"))
        if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else
            punc = "？";
    else if (sent.startsWith("还是先") && sent2.indexOf("先") > -1)
        punc = "？";
    else if (sent.indexOf("不就") > -1)
        if (tone == 0)
            ;
        else if (tone == 1)
            punc = "！";
        else
            punc = "？";
    else if (sent.indexOf("有点") > -1 && QString("啊吗呢吧呀么").indexOf(sent.right(1)) == -1)
        ;
    else if (sent.indexOf("斩") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else
            punc = "！";
    else if (sent.indexOf("简直") > -1)
        punc = "!";
    else if (sent.startsWith("巴不得") && (sent.endsWith("呢")))
        punc = "！";
    else if (sent.indexOf("不成") > -1 && (
                 canRegExp(sent, "还.{1,5}不成") || sent.indexOf("成不成") > -1))
        punc = "？";
    else if (canRegExp(sent, "你[更最].+的.+"))
        punc = "？";
    else if (sent.indexOf("杀") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else if (sent.length()  >= 2 && sent.left(1) == "杀")
            ;
        else if (sent.length()  >= 2 && sent.right(1) == "杀")
            ;
        else if (sent.indexOf("被") > -1)
            ;
        else if (sent.indexOf("我不") > -1)
            ;
        else if (sent.indexOf("杀了你") > -1)
            punc = "！";
        else if (sent.indexOf("我杀") > -1)
            ;
        else if (sent.indexOf("杀掉") > -1)
            punc = "！";
        else
        {}
    else if (sent.indexOf("死") > -1)
        if (tone == 2)
            punc = "？";
        else if (tone == 0)
            ;
        else if (sent.indexOf("濒死") > -1)
            ;
        else if (sent.indexOf("死活") > -1)
            ;
        else if (sent.indexOf("死寂") > -1)
            ;
        else if (sent.indexOf("死不") > -1)
            ;
        else if (sent.indexOf("不死") > -1)
            if (sent.indexOf("啊") > -1)
                punc = "？";
            else
            {}
        else
            punc = "！";
    else if (sent.length()  >= 4 && sent.left(1) == "快")
        if (tone == 0)
            ;
        else
            punc = "！";
    else if (tone == -1 && sent.indexOf("貌似") > -1 && sent.indexOf("要") > -1)
        ;
    else if (_left1 == "了")
        if (tone == 1)
            punc = "！";
        else if (tone == 2)
            punc = "？";
        else if (tone == 3)
            punc = "~";
        else
        {}
    else if (_left1 == "吗")
        if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else if (tone == 3)
            punc = "~";
        else
            punc = "？";
    else if (_left1 == "吧")
        if (tone == 1)
            punc = "！";
        else if (tone == 0)
            ;
        else if (tone == 3)
            punc = "~";
        else if (sent.indexOf("没") > -1)
            punc = "？";
        else if (_left2 == "心")
            punc = "，";
        else if (sent.indexOf("你还") > -1)
            punc = "？";
        else if (sent.indexOf("似乎") > -1)
            punc = "？";
        else if (sent.indexOf("会") > -1 && sent.indexOf("我") > -1 && sent.indexOf("我") < sent.indexOf("会"))
            punc = "？";
        else if (sent.indexOf("不说") > -1)
            ;
        else if (sent.indexOf("还") > -1)
            ;
        else if (sent.indexOf("酒吧") > -1)
            ;
        else if (sent.indexOf("网吧") > -1)
            ;
        else if (sent.indexOf("咖啡吧") > -1)
            ;
        else if (sent.indexOf("第") > -1 && sent.indexOf("次") > -1)
            if (tone == 2)
                punc = "？";
            else if (in_quote)
                punc = "？";
            else if (sent.indexOf("你"))
                punc = "？";
            else
                punc = "！";
        else if (sent.indexOf("再") > -1)
            punc = "！";
        else if (sent.indexOf("因为") > -1)
            punc = "！";
        else if (sent.indexOf("就是") > -1)
            punc = "！";
        else if (sent.startsWith("这是"))
            punc = "？";
        else if (sent.indexOf("是") > -1)
            punc = "？";
        else if (sent.indexOf("这也") > -1)
            punc = "？";
        else if (_left2 == "的")
            punc = "？";
        else if (canRegExp(sent, "就.{1,3}了吧"))
            punc = "？";
        else
            punc = "！";
    else if (_left1 == "啊")
        if (sent.indexOf("还是") > -1)
            punc = "！";
        else if (sent.indexOf("你还") > -1)
            punc = "？";
        else if (sent == "不过啊")
            punc = "，";
        else if (sent == "但是啊")
            punc = "，";
        else if (sent == "你想啊")
            punc = "，";
        else if (sent.right(3).contains("后"))
            punc = "，";
        else if ((sent.indexOf("啊") < sent.length() - 1 && sent.mid(sent.length()-2, 1) != "啊") || tone == 0)
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 2)
            punc = "？";
        else if (isChinese(_left2) == true)
            punc = "！";
        else
            punc = "！";
    else if (_left1 == "呢")
        if (sent.indexOf("如果") > -1 || sent.indexOf("要是") > -1 || sent.startsWith("而") || sent.indexOf("哪") > -1)
            punc = "？";
        else if (sent.contains("还是") || sent.contains("就是") || sent.contains("毕竟"))
            punc = "！";
        else if (sent .contains("起来") && sent .indexOf("起来") <= 2)
            punc = "！";
        else if (sent.startsWith("我") || sent.contains("很") || sent.contains("特别") || sent.contains("格外")
            || sent.contains("非常") || sent.contains("太") || sent.contains("真") || sent.contains("确"))
            punc = "！";
        else if (canRegExp(sent, "还.+了呢$") || canRegExp(sent, ".{1,3}着.*呢$"))
            punc = "！";
        else
            punc = "？";
    else if (_left1 == "呀")
        if (sent == "什么呀")
            punc = "，";
        else if (sent.indexOf("什么") > -1)
            punc = "？";
        else if (sent == "哎呀")
            punc = "，";
        else if (canRegExp(sent, "不.{1,3}呀"))
            punc = "！";
        else if (sent.indexOf("呀呀") > -1)
            ;
        else if (sent.indexOf("来呀") > -1)
            ;
        else if (tone == 1)
            punc = "！";
        else if (tone == 2)
            punc = "？";
        else
            punc = "！";
    else if (_left1 == "哦")
        if (isChinese(_left2) == true)
            punc = "！";
        else
            punc = "？";
    else if (_left1 == "哈")
        if (_left1 == "哈" || isChinese(_left2) == false)
            punc = "！";
        else
        {}
    else if (_left1 == "哼")
        punc = "！";
    else if (_left1 == "唉")
        if (tone == 0)
            ;
        else if (tone == 2)
            punc = "？";
        else
            punc = "！";
    else if (_left1 == "嘛")
        if (sent.indexOf("就") > -1)
            punc = "！";
        else if (isKnowFormat(sent) == true)
            ;
        else if (sent.indexOf("你") > -1)
            punc = "！";
        else
            punc = "！";
    else if (_left1 == "额")
        if (!isChinese(_left2))
            punc = "……";
        else
        {}
    else if (_left1 == "呃")
        punc = "……";
    else if (_left1 == "啦")
        if (_left2 == "的")
            punc = "～";
        else
            punc = "！";
    else if (_left1 == "嘻")
        punc = "！";
    else if (_left1 == "诶")
        punc = "！";
    else if (_left1 == "嘭")
        punc = "！";
    else if (_left1 == "咚")
        punc = "！";
    else if (_left1 == "咦")
        punc = "？";
    else if (_left1 == "呜")
        if (_left1 ==  "嗷")
            punc = "～～";
        else
            punc = "……";
    else if (_left1 == "开")
        if (isChinese(_left2) == false)
            punc = "！";
        else
        {}
    else if (_left1 == "嗷")
        punc = "～～";
    else if (_left1 == "呦")
        punc = "～";
    else if (_left1 == "呸")
        punc = "！";
    else if (_left1 == "嘿")
        punc = "！";
    else if (_left1 == "嗨")
        punc = "！";
    else if (_left1 == "哩" && _left2 != "哔")
        punc = "！";
    else if (_left1 == "靠")
        punc = "！";
    else if (_left1 == "艹")
        punc = "！";
    else if (_left1 == "要")
        punc = "！";
    else if (_left1 == "轰")
        punc = "！";
    else if (_left1 == "隆")
        punc = "！";
    else if (_left1 == "砰")
        punc = "！";
    else if (_left1 == "哇")
        punc = "！";
    else if (_left1 == "当")
        punc = "！";
    else if (_left1 == "喽")
        if (_left2 == "的")
            punc = "？";
        else if (tone == 0)
            punc = "。";
        else if (tone == 2)
            punc = "？";
        else
            punc = "！";
    else if (_left1 == "呵")
        if (!isChinese(_left2) || (_left2=="呵"&&_left3=="呵"))
            punc = "！";
        else
        {}
    else
    {}

    return punc;
}

/**
 * 取标点
 * @param  pos 光标位置
 * @return     标点
 */
QString NovelAIBase::getPunc(int pos)
{
    if (pos < 0 || pos > _text.length()) return "";
    int l_pos = _text.lastIndexOf("\n", pos > 0 ? pos - 1 : pos);
    int r_pos = _text.indexOf("\n", pos);
    if (l_pos == -1) l_pos = 0;
    if (r_pos == -1) r_pos = _text.length();
    QString para = _text.mid(l_pos, r_pos - l_pos); // 当前段落
    return (getPunc(para, pos-l_pos));
}

QString NovelAIBase::getPunc(/*QString fullText, int pos*/)
{
    return getPunc(_pos);
}

/**
 * 强行完成句子
 * @param pos
 * @return
 */
QString NovelAIBase::getPunc2(int pos)
{
    QString punc = getPunc(pos);
    if (punc == "，") punc = "。";
    return punc;
}

QString NovelAIBase::getPunc2()
{
    QString punc = getPunc(_pos);
    if (punc == "，") punc = "。";
    return punc;
}



/**
 * 初始化所有常量字符串（可能会移动到全局变量）
 */
void NovelAIBase::initConstString()
{
    _auto_punc_whitelists = "吗哼呀呢么嘛啦呵吧啊哦哪呸滚"; // 说问道
    _shuo_blacklists = "[白|在|再|再有|接|一直|人|没|持|道|还|样|以|理|明|里|来|么|用|要|一|想|都|是|虽|听|听我|据|再|小|说|是|不|传|如|胡|话|乱|且|假|游|按|邪|述|数|陈|成|称|浅|图|谬|定|解|瞎|劝|妄|解|叙|絮|评|照|论|申|言|演|学|明|好|难|枉|接|实|者|上|一会儿?|被.*|听|不|无.*|让.*|怎么.*|说.*]说";
    _dao_whitelists = "，说口嘴的着回答地释述笑哭叫喊吼脸骂";
    _wen_blacklists = "一请想去若试不莫问想要你我敢来";
    _symbol_pair_lefts  = "([{（［｛‘“<〈《〔【「『︵︷︹︻︽︿﹁";
    _symbol_pair_rights = ")]}）］｝’”>〉》〕】」』︶︸︺︼︾﹀﹂";
    _sent_puncs = "。？！；~—…?!";
    _sent_split_puncs = "，。？！；~—…?!;:\"“”,.";
    _sent_split = "\n\t，。？！；~—…?!;:\"“”,.";
    _quote_no_colon = "很常点击倒真是成为其乃就能的作些称作之被有和及选择在会用不起么上下出入与和及跟叫并且可以要非来去离知何啥意一百千万亿为到拿以多少点做为";
	_quantifiers = "局团坨滩根排列匹张座回场尾条个首阙阵网炮顶丘棵只支袭辆挑担颗壳窠曲墙群腔砣座客贯扎捆刀令打手罗坡山岭江溪钟队单双对出口头脚板跳枝件贴针线管名位身堂课本页丝毫厘分钱两斤担铢石钧锱忽毫厘分寸尺丈里寻常铺程撮勺合升斗石盘碗碟叠桶笼盆盒杯钟斛锅簋篮盘桶罐瓶壶卮盏箩箱煲啖袋钵年月日季刻时周天秒分旬纪岁世更夜春夏秋冬代伏辈丸泡粒颗幢堆";
    _blank_chars = "\n\r \t　";
}

/**
 * 是否为知道系列，若是，则取标点函数返回的问号将有可能变成句号
 * @param  str 句子文本
 * @return     是否是知道系列
 */
bool NovelAIBase::isKnowFormat(QString str)
{
    if (str.indexOf("怎么知") > -1) return false;
    if (str.indexOf("知道我") > -1) return false;

    if (canRegExp(str, "知道我.*[怎|什|何|吗|吧]"))
    	return false;

    QStringList knows;
    knows << "知道" << "我明" << "问我" << "明白" << "我理" << "我懂" << "至于" << "对于" << "问我" << "问他" << "问她"
          << "问它" << "才知" << "才明" << "才理" << "才懂" << "于知" << "于明" << "于理" << "于懂" << "在知" << "在明"
          << "在理解" << "在咚" << "我知" << "问了" << "说了" << "问了" << "我告诉" << "告诉了" << "教会";

    for (int i = 0; i < knows.size(); i++)
        if (str.indexOf(knows.at(i)) > -1)
            return true;

    return false;
}

/**
 * 能否正则匹配，一部分（总的可以加上 ^ $ ）
 * @param  str [description]
 * @param  pat [description]
 * @return     [description]
 */
bool NovelAIBase::canRegExp(QString str, QString pat)
{
    return QRegExp(pat).indexIn(str) > -1;

    /*QRegExp rxlen(pat);
    int pos = rxlen.indexIn(str);
    return pos > -1;*/

    /*if (pos > -1) {
        QString value = rxlen.cap(1);
        QString unit = rxlen.cap(2);
    }*/
}

bool NovelAIBase::isAllChinese(QString str)
{
    int len = str.length();
    if (len == 0) return false;
    for (int i = 0; i < len; i++)
    {
        ushort uni = str.at(0).unicode();
        if (!((uni >= 0x4E00 && uni <= 0x9FA5)))
            return false;
    }
    return true;
}

bool NovelAIBase::isChinese(QString str)
{
    if (str == "") return false;
    QChar ch = str.at(0);
    ushort uni = ch.unicode();
    return (uni >= 0x4E00 && uni <= 0x9FA5);
}

bool NovelAIBase::isEnglish(QString str)
{
    if (str == "") return false;
    QByteArray ba = str.toLatin1();
    const char *s = ba.data();
    return ( (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') );
}

bool NovelAIBase::isNumber(QString str)
{
    if (str == "") return false;
    QByteArray ba = str.toLatin1();
    const char *s = ba.data();
    return ( *s >= '0' && *s <= '9' );
}

bool NovelAIBase::isSentPunc(QString str)
{
    if (str == "") return false;
    if (str.length() > 1) str = str.left(1);
    return (_sent_puncs.indexOf(str) > -1);
}

bool NovelAIBase::isSentSplitPunc(QString str)
{
    if (str == "") return false;
    if (str.length() > 1) str = str.left(1);
    return (_sent_split_puncs.indexOf(str) > -1);
}

bool NovelAIBase::isSentSplit(QString str)
{
    if (str == "") return false;
    if (str.length() > 1) str = str.left(1);
    return (_sent_split.indexOf(str) > -1);
}

bool NovelAIBase::isASCIIPunc(QString str)
{
    if (str == "") return false;
    QChar ch = str.at(0);
    ushort uni = ch.unicode();
    return ( (uni <= 47) || (uni >= 58 && uni <= 63) || (uni >= 91 && uni <= 95) || (uni >= 123 && uni <= 127) );
}

bool NovelAIBase::isBlankChar(QString str)
{
    if (str == "") return true; // 这个空文本是返回true的
    if (str.length() > 1) str = str.left(1);
    return (_blank_chars.indexOf(str) > -1);
}

bool NovelAIBase::isBlankChar2(QString str)
{
    return isBlankChar(str) && str != "\n" && str != "\r";
}

bool NovelAIBase::isBlankString(QString str)
{
    int len = str.length();
    for (int i = 0; i < len; i++)
        if (!isBlankChar(str.mid(i, 1)))
            return false;
    return true;
}

bool NovelAIBase::isSymPairLeft(QString str)
{
    if (str == "") return false;
    return _symbol_pair_lefts.indexOf(str) > -1;
}

bool NovelAIBase::isSymPairRight(QString str)
{
    if (str == "") return false;
    return _symbol_pair_rights.indexOf(str) > -1;
}

QString NovelAIBase::getSymPairLeftByRight(QString str)
{
    int index = _symbol_pair_rights.indexOf(str);
    if (index == -1) return "";
    return _symbol_pair_lefts.mid(index, 1);
}

QString NovelAIBase::getSymPairRightByLeft(QString str)
{
    int index = _symbol_pair_lefts.indexOf(str);
    if (index == -1) return "";
    return _symbol_pair_rights.mid(index, 1);
}

bool NovelAIBase::isQuoteColon(QString str)
{
    if (str == "") return false;
    if (str.length() > 1) str = str.left(1);
    return (_quote_no_colon.indexOf(str) == -1 && _quantifiers.indexOf(str) == -1);
}

/**
 * 是否在光标内（只针对当前段落，无视其他段落）
 * @param text 要判断的文本，一般是 _text
 * @param pos  要判断的光标位置
 * @return     是否在光标内
 */
bool NovelAIBase::isCursorInQuote(QString text, int pos)
{
    if (pos > 0 && text.mid(pos-1,1 ) == "“")
        return true;
    if (pos < text.length() && text.mid(pos, 1)=="”")
        return true;

    // 获取段落的文本
    int l_pos = text.lastIndexOf("\n", pos > 0 ? pos - 1 : pos);
    int r_pos = text.indexOf("\n", pos);
    // if (l_pos == -1) l_pos = 0;
    l_pos++; // 不包括第一个前引号
    if (r_pos == -1) r_pos = text.length();
    if (r_pos - l_pos < 2) return false;
    text = text.mid(l_pos, r_pos - l_pos);
    pos -= l_pos;

    // 搜索左右两边的最近的前后引号
    int ll_pos = text.lastIndexOf("“", pos?pos-1:pos);
    int lr_pos = text.lastIndexOf("”", pos?pos-1:pos);
    int rl_pos = text.indexOf("“", pos);
    int rr_pos = text.indexOf("”", pos);
    if (ll_pos == -1) return false;                         // 没有前引号
    if (rr_pos == -1) return false;                         // 没有后引号
    if (ll_pos <= lr_pos) return false;                     // “”|
    if (rl_pos > -1 && rl_pos < rr_pos) return false;       // |“”
    //if (lr_pos < ll_pos && rl_pos <= rr_pos) return true; // 接着前引号，但是没有后引号

    return true;
}

/**
 * 插入或者删除文本之后，修改左右的字符串（class中的全局变量）
 */
void NovelAIBase::updateCursorChars()
{
    if (_pos >= 1) _left1 = _text.mid(_pos - 1, 1);
    else _left1 = "";
    if (_pos >= 2) _left2 = _text.mid(_pos - 2, 1);
    else _left2 = "";
    if (_pos >= 3) _left3 = _text.mid(_pos - 3, 1);
    else _left3 = "";
    if (_pos < _text.length()) _right1 = _text.mid(_pos, 1);
    else _right1 = "";
    if (_pos < _text.length() - 1) _right2 = _text.mid(_pos + 1, 1);
    else _right2 = "";
}

/**
 * 能否删除成对符号（使用栈进行判断）
 * 能删就删
 * 注意：用之前需要 prepareAnalyze()，用完后需要 finishAnalyze()
 * @return 是否删除了
 */
bool NovelAIBase::canDeletePairPunc()
{
	// 可能1：【|】
	int l_pos = _symbol_pair_lefts.indexOf(_left1);
	int r_pos = _symbol_pair_rights.indexOf(_right1);
	// 可能2：【】|
	int l_pos2 = _symbol_pair_lefts.indexOf(_left2);
	int r_pos2 = _symbol_pair_rights.indexOf(_left1);

	int nl_pos = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
	int nr_pos = _text.indexOf("\n", _pos);
	if (nl_pos == -1) nl_pos = 0;
	if (nr_pos == -1) nr_pos = _text.length();
	QString para = _text.mid(nl_pos + 1, nr_pos - nl_pos - 1); // 段落文本，用来判断括号堆栈
	int para_pos = _pos - nl_pos - 1;                     // 光标相对于段落的位置

	if (l_pos > -1 && l_pos == r_pos) // 【|】
	{
	    // 左边括号多余的数量
	    int l_count = 0;
	    for (int i = 0; i < para_pos; i++)
	        if (para.mid(i, 1) == _left1)
	            l_count++;
	        else if (para.mid(i, 1) == _right1)
	            l_count--;

	    // 右边括号剩余的数量
	    int r_count = 0, len = para.length();
	    for (int i = para_pos; i < len; i++)
	        if (para.mid(i, 1) == _left1)
	            r_count--;
	        else if (para.mid(i, 1) == _right1)
	            r_count++;
	    if (r_count >= l_count) // 这个段落右括号的数量比左括号多，或者数量一样，则一起删除
	        deleteText(_pos - 1, _pos + 1);
	    else
	        deleteText(_pos - 1, _pos);
	    return true;
	}
	else if (l_pos2 > -1 && l_pos2 == r_pos2) // 【】|
	{
	    // 左边括号多余的数量
	    int l_count = 0;
	    for (int i = 0; i < para_pos; i++)
	        if (para.mid(i, 1) == _left2)
	            l_count++;
	        else if (para.mid(i, 1) == _left1)
	            l_count--;

	    // 右边括号剩余的数量
	    int r_count = 0, len = para.length();
	    for (int i = para_pos; i < len; i++)
	        if (para.mid(i, 1) == _left2)
	            r_count--;
	        else if (para.mid(i, 1) == _left1)
	            r_count++;

	    if (l_count >= r_count) // 这个段落右括号的数量比左括号多，或者数量一样，则一起删除
	     	deleteText(_pos - 2, _pos);
	    else
	    	deleteText(_pos - 1, _pos);
	    return true;
	}

	return false;
}

/**
 * 获取相对于现行光标的字符
 * @param  x [description]
 * @return   [description]
 */
QString NovelAIBase::getCurrentChar(int x)
{
    int pos = _pos - x;
    if (pos < 0 || pos >= _text.length())
        return "";
    return _text.mid(pos, 1);
}
