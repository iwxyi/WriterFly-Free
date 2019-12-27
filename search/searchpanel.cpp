#include "searchpanel.h"

SearchPanel::SearchPanel(QWidget *parent) : QWidget(parent), current_editor_scroll(0, 0)
{
    setWindowTitle(tr("综合搜索"));
    setMouseTracking(true);

    current_editor = nullptr;
    margin = 9;

    initView();

    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
    updateUI();

    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(slotFocusChanged(QWidget *, QWidget *)));
}

void SearchPanel::initView()
{
    key_edit = new QLineEdit(this);
    result_view = new SearchListWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(key_edit);
    layout->addWidget(result_view);
    this->setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(margin);

    //this->setMinimumSize(50, 100);
    //key_edit->setMinimumWidth(50);
    //result_view->setMinimumHeight(100);
    //result_view->setResizeMode(QListView::Adjust);
    result_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(key_edit, &QLineEdit::textChanged, [=] {
        QString text = key_edit->text();
        if (!text.trimmed().isEmpty())
        {
            search(text);
        }
        else if (preview_scrolled && current_editor)
        {
            current_editor->smoothScrollTo(current_editor_scroll);
            preview_scrolled = false;
        }
        if (current_editor)
            current_editor->setSearchKey(text);
    });

    model = new SearchModel(this);
    delegate = new SearchDelegate(this);
    result_view->setModel(model);
    result_view->setItemDelegate(delegate);

    key_edit->setMinimumHeight(key_edit->size().height());
    //    key_edit->setAlignment(Qt::AlignCenter);
    result_view->setMinimumHeight(1);
    adjustHeight(0, 1);

    connect(result_view, SIGNAL(clicked(QModelIndex)), this, SLOT(slotResultClicked(QModelIndex)));
    connect(result_view->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(adjustHeight(int, int)));
    connect(result_view, SIGNAL(signalItemChanged(int)), this, SLOT(slotResultPreview(int)));
    // connect(result_view, SIGNAL(currentChanged (const QModelIndex&, const QModelIndex&)), this, SLOT(slotResultPreview())); // 都沒有效果诶

    this->setFixedHeight(key_edit->size().height() + margin * 2);

    // 阴影效果
    // 顶部绘制会引起：UpdateLayeredWindowIndirect failed 错误，除非设置 setBlurRadius(0)
    int shadow = 2;
    shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, shadow);
    shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
    shadow_effect->setBlurRadius(0);
    this->setGraphicsEffect(shadow_effect);

    vertical_scrollbar_width = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    rt->search_panel_item_width = result_view->width() - vertical_scrollbar_width;

    // 读取数据
    commands = readTextFile(":/documents/commands").split("\n", QString::SkipEmptyParts);
    hidden_commands = readTextFile(":/documents/hidden_commands").split("\n", QString::SkipEmptyParts);
    command_orders = us->getStr("recent/commands_list", "").split(",", QString::SkipEmptyParts);

    search_type_name << "接龙" << "成语" << "成语接龙"
                     << "词典" << "字典" << "汉语词典" << "汉语字典"
                     << "同义词" << "同义" << "近义词" << "近义"
                     << "相关词" << "相关" << "词语"
                     << "句子" << "常用句"
                     << "名片" << "名片标签" << "标签" << "名片名字" << "名片内容";
    search_type_type << IDIOMS_SOLITAIRE << IDIOMS_SOLITAIRE << IDIOMS_SOLITAIRE
                     << DICTIONARY_ITEM << DICTIONARY_ITEM << DICTIONARY_ITEM << DICTIONARY_ITEM
                     << SYNONYMS << SYNONYMS << SYNONYMS << SYNONYMS
                     << RELATEDS << RELATEDS << RELATEDS
                     << SENTENCES << SENTENCES
                     << CARDLIB << CARDLIB_TAG << CARDLIB_TAG << CARDLIB_NAME << CARDLIB_CONTENT;
}

void SearchPanel::updateUI()
{
    QString add = "";
    if (us->round_view)
        add = "border-radius: 5px;";
    QString style = ""; //"QWidget{background:"+us->getFixedOpacityColor(us->mainwin_bg_color, 200)+";"+add+"}";
    style += "QListView, QLineEdit{background:transparent; border: none; }";
    this->setStyleSheet(style);

    thm->setWidgetStyleSheet(result_view->verticalScrollBar(), "scrollbar");
}

void SearchPanel::setEditors(ChapterEditor *cur_edit, QList<ChapterEditor *> edits)
{
    this->current_editor = cur_edit;
    this->editors = edits;
}

void SearchPanel::setText(QString str)
{
    key_edit->setText(str);
    key_edit->selectAll();
}

void SearchPanel::search(QString key)
{
    results.clear();
    gd->lexicons.clear();
    _flag_height_changed = false;
    bool _random_found = false;
    ResultType search_type = ResultType::NONE; // 搜索的类型，可以指定搜索类型
    int default_select = 0;                    // 默认选中项

    // 帮助信息
    if (key == "帮助" || key == "help" || key == "HELP" || key == "Help" || key == "?" || key == "？" || key == "说明" || key == "教程" || key == "用法" || key == "方法" || key == "怎么" || key == "怎么用" || key == "写作天下" || key == "碼字風雲" || key == "码字风云")
    {
        QStringList sl;
        sl << "随机取名" << "姓" << "名" << "词语" << "相关" << "人生" << "句子" << "天空"
           << "所有名片" << "所有标签"
           << "接龙 为所欲为" << "章内搜索" << "已打开的章节搜索"
           << "快捷命令" << "快捷设置（未开发）";
        results = getResultsFromStringList(sl, RANDOM_SORTS);
        _random_found = true;
    }

    // 可执行命令
    if (key == "命令" || key == "指令" || key == "操作" || key == "执行" || key == "快捷命令" || key == "快捷指令")
    {
        results += getResultsFromStringList(commands, COMMAND);
    }
    else if (commands.size())
    {
        /*if (key.startsWith(">") || key.startsWith("》"))
            key = key.right(key.length()-1)->trimmed();*/
        int start_index = results.size();
        QStringList texts;
        QString command_key = key;
        QString arg_key = "";
        if (command_key.indexOf(" ") > -1)
        {
            int pos = command_key.indexOf(" ");
            arg_key = command_key.right(command_key.length() - pos).trimmed();
            command_key = command_key.left(pos);
            arg_key = arg_key.replace("，", ",");
            arg_key = arg_key.replace(QRegExp(",\\s+"), ", ");
        }
        foreach (QString command, commands)
        {
            if (fuzzySearchByChar(command, command_key))
            {
                results.append(SearchResultBean(command + (arg_key.isEmpty() ? "" : " " + arg_key), COMMAND));
                texts.append(command);
            }
        }
        foreach (QString command, hidden_commands)
        {
            if (command == command_key)
            {
                results.append(SearchResultBean(command + (arg_key.isEmpty() ? "" : " " + arg_key), COMMAND));
                texts.append(command);
            }
        }
        // 调整默认选中的顺序
        if (texts.size())
        {
            bool find = false;
            for (int i = 0; i < command_orders.size(); i++)
            {
                for (int j = 0; j < texts.size(); j++)
                    if (command_orders.at(i) == texts.at(j))
                    {
                        default_select = start_index + j;
                        find = true;
                        break;
                    }
                if (find)
                    break;
            }
        }
    }

    // 判断种类关键词
    for (int i = 0; i < search_type_name.size(); i++)
    {
        if (key.startsWith(search_type_name.at(i)) && key.length() > search_type_name.at(i).length() + 1 &&
            (key.mid(search_type_name.at(i).length(), 1) == " " || key.mid(search_type_name.at(i).length(), 1) == ":" || key.mid(search_type_name.at(i).length(), 1) == "："))
        {
            search_type = search_type_type.at(i);
            key = key.right(key.length() - search_type_name.at(i).length() - 1);
            break;
        }
    }
    if (search_type != ResultType::NONE)
    {
        while (key.length() >= 1 && (key.startsWith(" ") || key.startsWith(":") || key.startsWith("：")))
            key = key.right(key.length() - 1);
        if (key.trimmed().isEmpty())
            return;
    }

    // 成语接龙
    if (search_type == ResultType::IDIOMS_SOLITAIRE)
    {
        QString word = key.trimmed().right(1);
        QString pinyin = PinyinUtil::ChineseConvertPinyin(word);
        QString &list = gd->lexicons.dictionary_dir;
        QStringList sl = list.split("\n", QString::SkipEmptyParts);
        foreach (QString words, sl)
        {
            if (words.length() >= 4 && PinyinUtil::ChineseConvertPinyin(words.left(1)) == pinyin)
                results.append(SearchResultBean(words, IDIOMS_SOLITAIRE));
        }
        _random_found = true;
    }

    // 名片
    if (key == "所有名片" || key == "名片所有" || key == "名片")
    {
        foreach (CardBean *card, gd->clm.currentLib()->totalCards())
        {
            addOneResult(card);
        }
    }
    else if (key == "名片标签" || key == "标签" || key == "所有标签" || key == "标签所有")
    {
        QHash<QString, int> tags = gd->clm.currentLib()->getAllTags();
        QHash<QString,int>::const_iterator i = tags.constBegin();
        while (i != tags.constEnd())
        {
            QString tag = i.key();
            int count = i.value();
            if (!tag.trimmed().isEmpty())
                results.append(SearchResultBean(tag, makeXml(count, "COUNT"), CARDLIB_TAG));
            ++i;
        }
    }
    else if ((search_type == ResultType::NONE || search_type == ResultType::CARDLIB) && gd->clm.currentLib() != nullptr)
    {
        foreach (CardBean *card, gd->clm.currentLib()->totalCards())
        {
            if (card->name.contains(key) || card->pat.contains(key) || card->tags.contains(key) /* || card->brief.contains(key) || card->detail.contains(key) */)
            {
                addOneResult(card);
            }
        }
    }
    else if (search_type == ResultType::CARDLIB_TAG && gd->clm.currentLib() != nullptr)
    {
        foreach (CardBean *card, gd->clm.currentLib()->totalCards())
        {
            if (card->tags.contains(key))
            {
                addOneResult(card);
            }
        }
    }
    else if (search_type == ResultType::CARDLIB_NAME && gd->clm.currentLib() != nullptr)
    {
        foreach (CardBean *card, gd->clm.currentLib()->totalCards())
        {
            if (card->name.contains(key))
            {
                addOneResult(card);
            }
        }
    }
    else if (search_type == ResultType::CARDLIB_CONTENT && gd->clm.currentLib() != nullptr)
    {
        foreach (CardBean *card, gd->clm.currentLib()->totalCards())
        {
            if (card->brief.contains(key) || card->detail.contains(key))
            {
                addOneResult(card);
            }
        }
    }

    // 汉语词典
    if ((search_type == ResultType::NONE || search_type == ResultType::DICTIONARY_ITEM) && !search_type_name.contains(key) && gd->lexicons.dictionary_dir.indexOf("\n" + key + "\n") > -1)
    {
        QString &text = gd->lexicons.dictionary_text;
        int pos = text.indexOf("【" + key + "】");
        if (pos > -1)
        {
            int start = pos + key.length() + 2;
            int end = text.indexOf("\n", start);
            if (end == -1)
                end = text.length();
            QString mid = text.mid(start, end - start).trimmed();
            results.append(SearchResultBean(mid, makeXml(key, "KEY"), DICTIONARY_ITEM));
        }
    }

    if (!_random_found && gd->lexicons.random_inited)
    {
        // 搜索所有词语分类
        if ((search_type == ResultType::NONE || search_type == ResultType::RELATEDS) && (key == "词语" || key == "词" || key == "相关词" || key == "相关"))
        {
            QDir dir(rt->DATA_PATH + "lexicon/related");
            dir.setFilter(QDir::Files);
            QList<QFileInfo> infos = dir.entryInfoList();
            QStringList names;
            foreach (QFileInfo info, infos)
            {
                names << info.baseName();
            }
            results += getResultsFromStringList(names, RANDOM_SORTS);
            _random_found = true;
        }

        // 列出所有近义词
        if (key == "近义词列表" || key == "同义词列表" || key == "近义列表" || key == "同义列表" || key == "所有近义词" || key == "所有同义词" || key == "所有近义" || key == "所有同义")
        {
            QString &full_text = gd->lexicons.synonym_text;
            QStringList sl = full_text.split("\n", QString::SkipEmptyParts);
            results = getResultsFromStringList(sl, RANDOM_SORTS);
            _random_found = true;
        }

        // 搜索所有句子分类
        if (key == "句子" || key == "句" || key == "常用句" || key == "例子" || key == "示例")
        {
            QDir dir(rt->DATA_PATH + "lexicon/sentence");
            dir.setFilter(QDir::Files);
            QList<QFileInfo> infos = dir.entryInfoList();
            QStringList names;
            foreach (QFileInfo info, infos)
            {
                names << info.baseName();
            }
            results += getResultsFromStringList(names, RANDOM_SORTS);
            _random_found = true;
        }

        // 特定词语文件
        if (isFileExist(rt->DATA_PATH + "lexicon/related/" + key + ".txt"))
        {
            QString text = readTextFile(rt->DATA_PATH + "lexicon/related/" + key + ".txt");
            QStringList titles = getStrMids(text, "{", "}");
            results += getResultsFromStringList(titles, RANDOM_SORTS);
            _random_found = true;
        }

        // 特定词语分类
        if ((search_type == ResultType::NONE || search_type == ResultType::RELATEDS) && gd->lexicons.related_text.indexOf("{" + key + "}") > -1)
        {
            int pos = gd->lexicons.related_text.indexOf("{" + key + "}") + key.length() + 2;

            // 找出左边
            int left_pos1 = gd->lexicons.related_text.indexOf("\n", pos);
            int left_pos2 = gd->lexicons.related_text.indexOf("[", pos);
            if (left_pos1 == -1)
                left_pos1 = pos;
            if (left_pos2 == -1)
                left_pos2 = pos;
            int left_pos = qMax(left_pos1, left_pos2) + 1;

            // 找出右边
            int right_pos1 = gd->lexicons.related_text.indexOf("]", left_pos + 1);
            int right_pos2 = gd->lexicons.related_text.indexOf("\n", left_pos + 1);
            if (right_pos1 == -1)
                right_pos1 = gd->lexicons.related_text.length();
            if (right_pos2 == -1)
                right_pos2 = gd->lexicons.related_text.length();
            int right_pos = qMin(right_pos1, right_pos2);

            QString text = gd->lexicons.related_text.mid(left_pos, right_pos - left_pos);
            QStringList list = text.split(" ", QString::SkipEmptyParts);
            results += getResultsFromStringList(list, RELATEDS);
            _random_found = true;
        }

        // 特定句子种类
        if ((search_type == ResultType::NONE || search_type == ResultType::SENTENCES) && isFileExist(rt->DATA_PATH + "lexicon/sentence/" + key + ".txt"))
        {
            QString text = readTextFile(rt->DATA_PATH + "lexicon/sentence/" + key + ".txt");
            QStringList sents = text.split("\n");
            results += getResultsFromStringList(sents, key, SENTENCES);
            _random_found = true;
        }

        // 随机种类列表
        if (key == "随机" || key == "取名" || key == "随机取名")
        {
            if (gd->lexicons.searchRandom("随机取名"))
            {
                results += getResultsFromStringList(gd->lexicons.getResult(), RANDOM_SORTS);
                _random_found = true;
            }
        }

        // 姓氏
        else if (gd->lexicons.surname_inited && (key == "姓" || key == "姓氏"))
        {
            if (gd->lexicons.searchRandom("姓氏"))
            {
                results += getResultsFromStringList(gd->lexicons.getResult(), SURNAMES);
                _random_found = true;
            }
        }

        // 人名
        else if (gd->lexicons.name_inited && (key == "人名" || key == "名字" || key == "名"))
        {
            if (gd->lexicons.searchRandom("人名"))
            {
                results += getResultsFromStringList(gd->lexicons.getResult(), NAMES);
                _random_found = true;
            }
        }

        // 姓氏触发的人名
        else if (gd->lexicons.surname_inited && gd->lexicons.name_inited && gd->lexicons.surname_text.indexOf(" " + key + " ") > -1)
        {
            if (gd->lexicons.searchRandom("人名"))
            {
                results += getResultsFromStringList(gd->lexicons.getResult(), key, NAMES);
                _random_found = true;
            }
        }

        // 随机列表
        for (QString s : gd->lexicons.random_sort_list)
        {
            if (isFileExist(gd->lexicons.lexicon_dir + "random/" + key + ".txt"))
            {
                gd->lexicons.searchRandom(key);
                results += getResultsFromStringList(gd->lexicons.getResult(), RANDOMS);
                _random_found = true;
            }
        }
    }

    if (!_random_found)
    {
        key = key.trimmed(); // 去掉首尾空白（带空白的时候不搜索随机内容）
        int key_len = key.length();

        // 搜索当前章节
        if ((search_type == ResultType::NONE || search_type == ResultType::OPENED_CHAPTERS || search_type == ResultType::CURRENT_CHAPTER) && current_editor != nullptr)
        {
            QList<SearchResultBean> list = getResultsFromEditor(current_editor, key);
            if (list.count() > 0)
                results += list;
        }

        // 搜索打开的章节
        if ((search_type == ResultType::NONE || search_type == ResultType::OPENED_CHAPTERS) && editors.count() > 0)
        {
            for (int i = 0; i < editors.count(); i++)
            {
                if (editors.at(i) != current_editor)
                {
                    QList<SearchResultBean> list = getResultsFromEditor(editors.at(i), key, i);
                    if (list.count() > 0)
                        results += list;
                }
            }
        }

        // 搜索近义词
        if (search_type == ResultType::NONE || search_type == ResultType::SYNONYMS)
        {
            QStringList synonym_list;
            QString &synonym_text = gd->lexicons.synonym_text;
            int pos = 0;
            while (1)
            {
                pos = synonym_text.indexOf(QRegExp("\\b" + key + "\\b"), pos);
                if (pos == -1)
                    break;
                int left = synonym_text.lastIndexOf("\n", pos) + 1;
                int right = synonym_text.indexOf("\n", pos);
                if (right == -1)
                    right = synonym_text.length();
                QString para = synonym_text.mid(left, right - left);
                QStringList list = para.split(" ", QString::SkipEmptyParts);
                // TODO 后期将改成 QList<CompleterItem>
                synonym_list.append(list);
                pos += key_len;
            }
            if (synonym_list.size() > 0)
            {
                while (synonym_list.removeOne(key))
                    ; // 移除全部相同的
                std::random_shuffle(synonym_list.begin(), synonym_list.end());
                results += getResultsFromStringList(synonym_list, key, SYNONYMS);
            }
        }

        // 搜索相关词
        if (search_type == ResultType::NONE || search_type == ResultType::RELATEDS)
        {
            QString &related_text = gd->lexicons.related_text;
            QStringList related_list;
            int pos = 0;
            while (1)
            {
                pos = related_text.indexOf(QRegExp("\\b" + key + "\\b"), pos);
                if (pos == -1)
                    break;                                          // 找不到了
                if (pos > 0 && related_text.mid(pos - 1, 1) == "{") // 是标题
                {
                    int left = related_text.indexOf("[", pos) + 1;
                    int right = related_text.indexOf("]", pos);
                    if (right < left - 1) // 出现了错误
                    {
                        pos = left + 1;
                        continue;
                    }
                    if (right == -1)
                        right = related_text.length();
                    QString para = related_text.mid(left, right - left);
                    QStringList list = para.split(" ", QString::SkipEmptyParts);
                    // TODO 后期将改成 QList<CompleterItem>
                    related_list.append(list);
                    pos = right;
                }
                else // 是内容
                {
                    // 如果是本程序标准格式
                    int left = related_text.lastIndexOf("[", pos) + 1;
                    int right = related_text.indexOf("]", pos);

                    // 如果只是一段一段分开的
                    int left_n = related_text.lastIndexOf("\n", pos) + 1;
                    if (left_n >= left)
                    {
                        left = left_n;
                        right = related_text.indexOf("\n", pos);
                    }
                    if (right == -1)
                        right = related_text.length();

                    QString para = related_text.mid(left, right - left);
                    QStringList list = para.split(" ", QString::SkipEmptyParts);
                    // TODO 后期将改成 QList<CompleterItem>
                    related_list.append(list);
                    pos = right;
                }
            }
            if (related_list.size() > 0)
            {
                while (related_list.removeOne(key))
                    ; // 移除全部相同的
                std::random_shuffle(related_list.begin(), related_list.end());
                results += getResultsFromStringList(related_list, key, RELATEDS);
            }
        }

        // 搜索句子
        if (search_type == ResultType::NONE || search_type == ResultType::SENTENCES)
        {
            QStringList sentence_list = gd->lexicons.getSentence(key);
            results += getResultsFromStringList(sentence_list, key, SENTENCES);
        }
    }

    // 显示到列表框
    model->setResults(results);
    if (results.count() > 0) // 设置为第一个选项
    {
        result_view->setCurrentIndex(model->index(default_select));
        slotResultPreview(default_select);
    }
    else if (preview_scrolled) // 没有结果，回滚到开始位置
    {
        if (current_editor != nullptr)
            current_editor->smoothScrollTo(current_editor_scroll);
        preview_scrolled = false;
    };
    // 搜索之后调整高度
    if (!_flag_height_changed)
    {
        this->setFixedHeight(key_edit->size().height() + margin * 2); // 通过这个触发大小改变事件，为自己的机智点个赞！(^_−)☆
    }
}

void SearchPanel::addOneResult(CardBean* card)
{
	QString info = card->brief.isEmpty() ? card->detail : card->brief;
    QString hglt = CardlibHighlight::getHighlightedText(info);
    results.append(SearchResultBean(card->name, makeXml(card->id, "ID") + makeXml(card->name, "NAME") + makeXml(info, "INFO") + makeXml(hglt, "HGLT") + makeXml(static_cast<QVariant>(card->color).toString(), "COL"), CARDLIB));
}

void SearchPanel::adjustHeight(int min, int max)
{
    Q_UNUSED(min);
    if (max < key_edit->size().height() / 2)
        return; // 不足一行的，就感觉不对
    _flag_height_changed = true;
    max += margin * 2 + key_edit->size().height() + result_view->verticalScrollBar()->pageStep() + 5 /*这个5是为了避免窗口高度刚好比列表框矮那么一点点*/; // 窗口高度
    int max_height = parentWidget()->geometry().height() / 2;
    if (max > max_height)
        max = max_height;
    this->setFixedHeight(max);
}

void SearchPanel::showEvent(QShowEvent *event)
{
    QWidget *parent = parentWidget();
    if (parent != nullptr)
    {
        QSize p_size = parent->size();
        int width = p_size.width() / 2;
        if (width > 500)
            width = 500;
        int left = (p_size.width() - width) / 2;
        int top = p_size.height() / 10;
        int height = p_size.height() / 2;

        this->move(left, top);
        this->setMaximumHeight(height);
        this->setFixedWidth(width);
    }
    rt->search_panel_item_width = result_view->width() - vertical_scrollbar_width;

    // 保存打开时的位置，如果按的是ESC，则恢复原来的滚动位置
    if (current_editor != nullptr)
    {
        preview_scrolled = false;
        current_editor_scroll.fromEditor(current_editor);
    }

    key_edit->setFocus();
    key_edit->selectAll();

    // 显示动画
    QPropertyAnimation* eani = new QPropertyAnimation(this, "shadow_radius");
    eani->setStartValue(shadow_radius);
    eani->setEndValue(SHADOW_BLUR_RADIUS_MAX);
    eani->setDuration(300);
    eani->start();
    connect(eani, &QPropertyAnimation::finished, [=]{
        eani->deleteLater();
    });

    QRect geo = this->geometry();
    QPropertyAnimation *ani = new QPropertyAnimation(this, "pos");
    ani->setDuration(results.count() == 0 ? 300 : 400);
    ani->setStartValue(QPoint(geo.left(), -geo.height() - 5));
    ani->setEndValue(QPoint(geo.left(), geo.top()));
    ani->setEasingCurve(QEasingCurve::OutBack);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=] {
        ani->deleteLater();
        // this->setGraphicsEffect(shadow_effect);
    });

    return QWidget::showEvent(event);
}

void SearchPanel::toHide()
{
    emit signalHide();

    if (current_editor != nullptr)
        current_editor->setSearchKey("");
    for (int i = 0; i < editors.count(); i++)
        editors.at(i)->setSearchKey("");

    // 隐藏动画
    // this->setGraphicsEffect(nullptr);
    QPropertyAnimation* eani = new QPropertyAnimation(this, "shadow_radius");
    eani->setStartValue(shadow_radius);
    eani->setEndValue(-SHADOW_BLUR_RADIUS_MAX);
    eani->setDuration(100);
    eani->start();
    connect(eani, &QPropertyAnimation::finished, [=]{
        eani->deleteLater();
    });

    QRect geo = this->geometry();
    QPropertyAnimation *ani = new QPropertyAnimation(this, "pos");
    ani->setDuration(300);
    ani->setStartValue(QPoint(geo.left(), geo.top()));
    ani->setEndValue(QPoint(geo.left(), -geo.height()+5));
    ani->setEasingCurve(QEasingCurve::InOutCirc);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=] {
        ani->deleteLater();
        this->hide();
    });
}

bool SearchPanel::fuzzySearchByChar(QString &text, QString &key) const
{
    QStringList chars = key.split("", QString::SkipEmptyParts);
    foreach (QString c, chars)
    {
        if (c != " " && !text.contains(c))
            return false;
    }
    return true;
}

void SearchPanel::hideEvent(QHideEvent *event)
{

    return QWidget::hideEvent(event);
}

void SearchPanel::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    QColor color = us->mainwin_sidebar_color;
    color.setAlpha(255);
    painter.setPen(color);
    QPainterPath path;

    // 画一个圆角矩形
    if (us->round_view)
    {
        path.addRoundedRect(QRect(QPoint(0, 0), geometry().size()), 5, 5);
    }
    // 画一个矩形
    else
    {
        path.addRect(QRect(QPoint(0, 0), geometry().size()));
    }

    // 减去顶边的，避免重绘报错
    /*if (!rt->isMainWindowMaxing() && us->widget_radius && parentWidget()!=nullptr)
    {
        int h = us->mainwin_border+1-this->geometry().top();
        if (h > 0 && h < height())
        {
            QPainterPath cut_path;
            cut_path.addRect(-5, 0, width()+10, h+1);
            path -= cut_path;
        }
    }*/

    painter.fillPath(path, color);
}

void SearchPanel::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up: // 上移一个选项
    {
        int current_index = result_view->currentIndex().row();
        if (current_index > 0)
            current_index--;
        else
            current_index = results.count() - 1; // 跳到最末未
        result_view->setCurrentIndex(model->index(current_index));
        slotResultPreview(current_index);
    }
    break;
    case Qt::Key_Down: // 下移一个选项
    {
        int current_index = result_view->currentIndex().row();
        if (current_index < results.count() - 1)
            current_index++;
        else
            current_index = 0; // 跳到开头的位置
        result_view->setCurrentIndex(model->index(current_index));
        slotResultPreview(current_index);
    }
    break;
    case Qt::Key_Home: // 跳转到第一个结果
    {
        int current_index = result_view->currentIndex().row();
        if (current_index < results.count() - 1)
        {
            result_view->setCurrentIndex(model->index(0));
            slotResultPreview(0);
        }
    }
    break;
    case Qt::Key_End: // 跳转到最后一个结果
    {
        int current_index = result_view->currentIndex().row();
        if (current_index < results.count() - 1)
        {
            result_view->setCurrentIndex(model->index(results.count() - 1));
            slotResultPreview(results.count() - 1);
        }
    }
    break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        if (result_view->currentIndex().isValid())
            slotResultClicked(result_view->currentIndex());
    }
    break;
    case Qt::Key_Escape: // 结束搜索
        toHide();
        if (current_editor != nullptr)
        {
            if (preview_scrolled) // 已经预览过了，回滚到原来的位置
            {
                //current_editor_scroll.setEditor(current_editor);
                current_editor->smoothScrollTo(current_editor_scroll);
                preview_scrolled = false;
            }
            current_editor->setFocus();
        }
        break;
    }

    return QWidget::keyPressEvent(event);
}

QList<SearchResultBean> SearchPanel::getResultsFromStringList(QStringList sl, ResultType type)
{
    QList<SearchResultBean> list;
    for (QString l : sl)
    {
        list.append(SearchResultBean(l, type));
    }
    return list;
}

QList<SearchResultBean> SearchPanel::getResultsFromStringList(QStringList sl, QString key, ResultType type)
{
    QList<SearchResultBean> list;
    QString data = makeXml(key, "KEY");
    for (QString l : sl)
    {
        list.append(SearchResultBean(l, data, type));
    }
    return list;
}

QList<SearchResultBean> SearchPanel::getResultsFromEditor(ChapterEditor *te, QString key, int index)
{
    QList<SearchResultBean> list;
    QString text = te->toPlainText();
    int key_len = key.length();
    int find_pos = 0;
    EditingInfo info = te->getInfo();
    while ((find_pos = text.indexOf(key, find_pos)) != -1)
    {
        QString data = getKeysInText(key, find_pos, text) + makeXml(index, "INDEX") + makeXml(info.getNovelName(), "NOVEL") + makeXml(info.getChapterName(), "CHAPTER");
        QString str = getXml(data, "TEXT");
        list.append(SearchResultBean(str, data, index == -1 ? CURRENT_CHAPTER : OPENED_CHAPTERS));
        find_pos += key_len;
    }
    return list;
}

QString SearchPanel::getKeysInText(QString key, int pos, QString &text)
{
    // 获取段落内容
    int para_left = text.lastIndexOf("\n", pos - 1) + 1;
    int para_right = text.indexOf("\n", pos);
    // 去掉首尾空（包括所有空白符，不知道trim行不行，所以就没用）
    while (para_left < pos)
    {
        QString c = text.mid(para_left, 1);
        if (c == " " || c == "　" || c == "\n" || c == "\r" || c == "\t")
            para_left++;
        else
            break;
    }
    if (para_right < 0)
        para_right = text.length();
    while (para_right > para_left)
    {
        QString c = text.mid(para_right - 1, 1);
        if (c == " " || c == "　" || c == "\n" || c == "\r" || c == "\t")
            para_right--;
        else
            break;
    }
    QString para = text.mid(para_left, para_right - para_left);
    return makeXml(para, "TEXT") + makeXml(pos - para_left, "START") + makeXml(key.length(), "LEN") + makeXml(pos, "POS");
}

void SearchPanel::setShadowRadius(int x)
{
    this->shadow_radius = x;
    if (x < 0)
        x = 0;
    shadow_effect->setBlurRadius(0);
}

/**
 * 搜索结果直接被单击
 */
void SearchPanel::slotResultClicked(QModelIndex index)
{
    QString key = key_edit->text();
    SearchResultBean result = results.at(index.row());
    QString text = result.text;
    QString data = result.data;

    switch (result.type)
    {
    case NONE:
        /* UNKNOW */
        break;
    // ==== 文本定位 ====
    case CURRENT_CHAPTER:
    case OPENED_CHAPTERS:
    {
        int index = getXml(data, "INDEX").toInt();
        int pos = getXml(data, "POS").toInt();
        int len = getXml(data, "LEN").toInt();
        emit signalLocateEdit(index, pos, len);

        if (result.type == CURRENT_CHAPTER)
            current_editor->setFocus();
    }
    break;
    case NOVEL_CHAPTERS:
        /* TODO */
        break;
    case ALL_CHAPTERS:
        /* TODO */
        break;
    // ==== 直接插入 ====
    case SYNONYMS:
    case RELATEDS:
    case SENTENCES:
    case RANDOMS:
    case SURNAMES:
    case DICTIONARY_ITEM:
        emit signalInsertText(text);
        break;
    case NAMES:
    {
        QString surname = getXml(data, "KEY");
        emit signalInsertTextWithSuffix(surname, text);
    }
    break;
    case RANDOM_SORTS:
        key_edit->setText(text);
        return;
    case IDIOMS_SOLITAIRE:
        key_edit->setText("接龙 " + text);
        return;
    case COMMAND:
        adjustCommandOrder(text);
        QTimer::singleShot(0, [=] { // 避免线程太长导致出错
            emit signalEmitCommand(text);
        });
        break;
    case CARDLIB:
    {
        CardBean *card = gd->clm.currentLib()->getCard(getXml(data, "ID"));
        emit signalOpenCard(card);
        break;
    }
    case CARDLIB_TAG:
        key_edit->setText("名片标签 " + text);
        return;
    default:
        break;
    }

    toHide();
}

/**
 * 搜索结果预览
 * 允许通过ESC键撤销回原来的位置
 */
void SearchPanel::slotResultPreview(int index)
{
    SearchResultBean result = results.at(index);

    if (result.type == CURRENT_CHAPTER && current_editor != nullptr) // 如果是当前章节的话
    {
        preview_scrolled = true;

        int pos = getXml(result.data, "POS").toInt();
        int len = getXml(result.data, "LEN").toInt();
        current_editor->smoothScrollTo(pos, len, -2);
        current_editor->setSearchKey(key_edit->text());
    }
    else if (preview_scrolled) // 上一个选项是当前章节
    {
        // 恢复原来的位置
        if (current_editor != nullptr)
            current_editor->smoothScrollTo(current_editor_scroll);
        preview_scrolled = false;
    }
    else if (result.type == OPENED_CHAPTERS) // 如果是打开的章节
    {
    }
}

void SearchPanel::slotFocusChanged(QWidget * /*old*/, QWidget *now)
{
    if (this->isHidden() || now == key_edit || now == result_view || now == this)
        return;
    else
    {
        toHide(); // 如果失去了焦点，就隐藏起来
    }
}

void SearchPanel::adjustCommandOrder(QString command)
{
    command_orders.removeOne(command);
    command_orders.insert(0, command);
    QString str = command_orders.join(",");
    us->setVal("recent/commands_list", str);
}
