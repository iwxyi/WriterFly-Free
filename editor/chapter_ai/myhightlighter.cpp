#include "myhightlighter.h"

MyHightlighter::MyHightlighter(QTextDocument *parent) : QSyntaxHighlighter (parent)
{
    input_manager = nullptr;
    search_key = "";
    card_underline_visible = false;
}

void MyHightlighter::setInputManager(EditorInputManager* im)
{
    this->input_manager = im;

    connect(input_manager, SIGNAL(signalRehighlight()), this, SLOT(rehighlight()));
}

void MyHightlighter::setSearchKey(QString text)
{
    search_key = text;
    rehighlight();
}

void MyHightlighter::setCardUnderlineVisible(bool show)
{
    card_underline_visible = show;
    rehighlight();
}

void MyHightlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty()) return ;

    int position = currentBlock().position();

    // 高亮人物语言（引号内彩色）
    if (us->quote_colorful)
    {
        QTextCharFormat accent_format;
        accent_format.setForeground(us->quote_colorful_color);
        QRegularExpression expression("“.*?”");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), accent_format);
        }
    }

    // 高亮章节备注
    if (us->chapter_mark)
    {
        QTextCharFormat gray_format;
        gray_format.setForeground(us->chapter_mark_color);
        QRegularExpression expression("@.*?$");
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), gray_format);
        }
    }

    // 高亮自定义正则表达式


    // 高亮名片
    if (gd->clm.currentLib() != nullptr)
    {
        CardList& cards = gd->clm.currentLib()->using_cards;
        foreach (CardBean* card, cards)
        {
            QTextCharFormat card_format;
            card_format.setForeground(card->color);
            if (card_underline_visible)
                card_format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            QRegularExpression expression(card->pattern());
            QRegularExpressionMatchIterator i = expression.globalMatch(text);
            while (i.hasNext())
            {
                QRegularExpressionMatch match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(), card_format);
            }
        }
    }

    // 高亮关键词搜索
    if (!search_key.isEmpty())
    {
        if (!us->search_regex) // 不使用正则表达式
        {
            QTextCharFormat gray_format;
            gray_format.setForeground(Qt::red);
            int len = search_key.length();
            int pos = 0;
            while ( (pos = text.indexOf(search_key, pos)) != -1)
            {
                setFormat(pos, len, gray_format);
                pos += len;
            }
        }
        else
        {
            QTextCharFormat gray_format;
            gray_format.setForeground(Qt::red);
            QRegularExpression expression(search_key);
            QRegularExpressionMatchIterator i = expression.globalMatch(text);
            while (i.hasNext())
            {
                QRegularExpressionMatch match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(), gray_format);
            }
        }
    }

    // 高亮敏感词
    if (us->sensitive_highlight)
    {
        QTextCharFormat warning_format;
        warning_format.setForeground(Qt::red);
        warning_format.setBackground(Qt::yellow);
        QRegularExpression expression(gd->lexicons.getSensitivePattern());
        QRegularExpressionMatchIterator i = expression.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), warning_format);
        }
    }

    // 高亮错词
    if (us->error_word_highlight)
    {
        QTextCharFormat warning_format;
        warning_format.setForeground(Qt::darkMagenta);
        warning_format.setBackground(Qt::yellow);
        foreach (QRegularExpression expression, gd->lexicons.getErrorWordPatterns())
        {
            QRegularExpressionMatchIterator i = expression.globalMatch(text);
            while (i.hasNext())
            {
                QRegularExpressionMatch match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(), warning_format);
            }
        }
    }

    // 设置动画中文字为透明
    // 每次输入动画开始（设置透明）和结束（回复颜色）都会调用一次 rehighligt，非常损耗性能
    if (us->input_animation && input_manager)
    {
        QTextCharFormat transparent_format;
        transparent_format.setForeground(QColor( 0, 0, 0, 0));
        for (int i = 0; i < input_manager->ani_list.count(); i++)
        {
            EditorInputCharactor* character = input_manager->ani_list[i];

            // 这个高亮是按照 block 来的，按照当前 block 的相对位置来找到动画的相对位置，判断在不在这个 block，然后设置各个字符的颜色
            int pos = character->getPosition();
            pos -= position;
            if (pos < 0 || pos > text.length()) continue;

            // 设置彩色化后的颜色
            // 因为只有开始运行的时候才会触发，所以在循环里直接设置就好了。
            if (!character->isColorSetted())
            {
//                character->setColor(format(pos).foreground().color());
                // BUG: 如果高亮的文字刚好是黑色，就会当做没有设置颜色，从而高亮色彩变为字体颜色
                character->setColor(format(pos).foreground().color() == Qt::black ? us->editor_font_color : format(pos).foreground().color());
            }

            // 设置动画中的透明颜色
            setFormat(pos, 1, transparent_format);
        }
    }

}
