#include "usersettingswindow.h"

UserSettingsWindow::UserSettingsWindow(QWidget *parent) : QFrame(parent)
{
    //setModal(true); // 模态窗口，必须关闭
    _flag_group_scroll = true;

    // 设置布局
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    QVBoxLayout* btn_layout = new QVBoxLayout;
    QPushButton* shortcut_btn =new QPushButton("快捷键", this);
    group_list = new QListWidget;
    item_list = new QListWidget;
    btn_layout->addWidget(group_list);
    btn_layout->addWidget(shortcut_btn);
    hlayout->addLayout(btn_layout);
    hlayout->addWidget(item_list);
    hlayout->setStretch(0, 2);
    hlayout->setStretch(1, 5);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);
    group_list->setMinimumWidth(50);
    group_list->setMaximumWidth(200);
    item_list->setMinimumWidth(100);
    group_list->setResizeMode(QListView::Adjust);
    item_list->setResizeMode(QListView::Adjust);
    item_list->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    item_list->verticalScrollBar()->setSingleStep(10);
    item_list->setFocusPolicy(Qt::NoFocus);

    this->setLayout(hlayout);

    QString widget_style = "QFrame{background: "+us->getColorString(us->getNoOpacityColor(us->mainwin_bg_color))+"; border: none;}";
    widget_style += "QLabel{background:transparent;}";
    this->setStyleSheet(widget_style);

    QString list_style = "QListWidget{background: transparent; }";
    group_list->setStyleSheet(list_style);
    item_list->setStyleSheet(list_style);

    initItems();

    connect(shortcut_btn, &QPushButton::clicked, [=]{
        emit signalShortcutKey();
    });

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

void UserSettingsWindow::updateUI()
{
    QString style = "QListWidget{border:none; }\
            QListWidget::Item{padding-left:3px;}\
            QListWidget::Item:hover{background:" + us->getOpacityColorString(us->accent_color, 50) + "; }\
            QListWidget::item:selected{background:" + us->getOpacityColorString(us->accent_color, 150) + "; }";
    //QListWidget::item:selected:!active{border-width:0px;background:"+us->getOpacityColorString(us->accent_color, 200)+"; }";
    group_list->setStyleSheet(style);
    item_list->setStyleSheet(style);

    thm->setWidgetStyleSheet(this->item_list->verticalScrollBar(), "scrollbar");

    setStyleSheet("QFrame{background: "+us->getColorString(us->getNoOpacityColor(us->mainwin_bg_color))+"; border: none;} QLabel{background:transparent;}");
}

/**
 * 初始化所有设置项
 */
void UserSettingsWindow::initItems()
{
    /* ==== 分类 ==== */
    QStringList group_names, grounp_icons;
    group_names << tr("外观风格") << tr("章节编辑") << tr("特色功能") << tr("词典提示") << tr("交互操作") << tr("文章排版");
    grounp_icons << "appearance" << "editor" << "feature" << "lexicon" << "interaction" << "typeset";
    QFont font("", 15);
    for (int i = 0; i < group_names.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(group_list);
        //item->setTextAlignment(Qt::AlignRight);
        item->setSizeHint(QSize(80, GROUPS_HEIGHT));
        item->setText(group_names.at(i));
        item->setFont(font);

        QPixmap pixmap(":/icons/setting/" + grounp_icons.at(i));
        QIcon icon(pixmap);
        item->setIcon(icon);
    }
    connect(group_list, SIGNAL(currentRowChanged(int)), this, SLOT(slotScrollToGroup(int)));
    //connect(group_list, SIGNAL(itemPressed(QListWidgetItem *item)), this, SLOT(slotGroupClicked(QListWidgetItem *item);));

    QString text; // 备用文字

    /* ==== 界面 ==== */
    QString update_str = "";
    if (rt->has_new_version == 1) update_str = " (新版下载中，关闭后自动更新)";
    else if (rt->has_new_version == 2) update_str = " (新版已下载，关闭后自动更新)";
    addItems("about", "写作天下 " + VERSION_CODE + update_str, "一键加群：705849222", "反馈/建议/素材/合作", "", QString(""));

    addGroup("", "外观风格", "");
    //    addItems("integration_window", "一体化界面", "替代原生窗口，更加美观", "【一体化】与【原生】的区别：\n标题栏支持填充背景颜色/背景图片\n更好看的打开章节动画效果\n无法从边缘调整大小（右下角可调整）\n无法肆意移动目录位置", us->integration_window);
    addItems("mainwin_bg_picture", "窗口背景图片", "建议修改颜色透明度（鼠标悬浮查看提示）", "修改图片后缀名产生不同的效果：\npng格式图片会覆盖在背景颜色上面（如果透明的话）\njpg格式会被背景颜色覆盖（可调整颜色透明度，255时禁用纯色）\n允许jpg和png图片同时使用\n后续可能支持gif动图与可交互的动态背景", us->mainwin_bg_picture);
    addItems("blur_picture_radius", "背景图片模糊半径", "只针对 jpg 背景有效", "可用png覆盖在jpg之上，起到混合效果", us->getInt("us/blur_picture_radius", 80));
    addItems("mainwin_bg_color", "主色调", "各窗口背景颜色"+(us->dynamic_bg_model?QString("(可能会被微动背景屏蔽)"):""), "仅在微动背景关闭时生效", us->mainwin_bg_color);
    addItems("mainwin_sidebar_color", "副色调", "目录/搜索栏等小组件背景颜色", "只针对一体化界面（隐藏功能）有效", us->mainwin_sidebar_color);
    addItems("accent_color", "点缀色", "强调色，例如指示条、当前选项", "", us->accent_color);

    if (us->dynamic_bg_model == Dynamic_None)
        text = "关";
    else if (us->dynamic_bg_model == Dynamic_Pure)
        text = "纯色";
    else if (us->dynamic_bg_model == Dynamic_Gradient)
        text = "渐变";
    else if (us->dynamic_bg_model == Dynamic_Island)
        text = "移动";
    else
        text = "其他";
    addItems("dynamic_background", "微动背景", "伪静态背景，不妨碍创作又增加乐趣", "", text);
    addItems("emotion_filter_enabled", "情景滤镜", "智能分析情节中人物情绪、所处环境，为背景添加相应滤镜", "", us->emotion_filter_enabled);

    if (isFileExist(rt->IMAGE_PATH + "win_bg.jpg") || isFileExist(rt->IMAGE_PATH + "win_bg.png"))
        text = "（和窗口图片冲突）";
    else
        text = "";
    addItems("environment_pictures_enabled", "场景图片", "识别小说中的故事环境，以对应的图片作为背景图片"+text, "", us->envir_picture_enabled);

    addItems("mainwin_titlebar_height", "标题栏高度", "顶部标题栏和部分按钮的大小。重启生效", "只针对一体化界面（隐藏功能）有效，并且受到字体影响。", us->widget_size);
    hideItem();
    addItems("global_font_color", "全局字体颜色", "除正文外所有文字颜色", "", us->global_font_color);
    hideItem();
    addItems("round_view", "圆角控件", "使用全新的局部UI效果（重启生效）", "", us->round_view);
    hideItem();
    addItems("mainwin_sidebar_spacing", "目录章节列表间距", "", "", us->mainwin_sidebar_spacing);
    hideItem();
    addItems("settings_in_mainwin", "设置界面嵌入主窗口", "在主窗口中打开设置界面，显示背景模糊效果（重启生效）", "关闭此项可实时看到界面设置的改变情况", us->settings_in_mainwin);
    hideItem();
    addItems("split_shade", "明显的分割线", "显示分割线阴影、分界线线条", "", us->split_shade);
    hideItem();
    addSplitter();

    /* ==== 编辑器 ==== */
    addGroup("", "章节编辑", "");
    // addItems("editor_bg_color", "编辑框背景颜色", "", "", us->editor_bg_color);
    addItems("editor_font_size", "编辑框字体大小", "全局字体格式", "（全局重启生效）", "自定义字体方法：“font.ttf”字体文件放到“软件目录下/data/style/”", us->editor_font_size);
    addItems("editor_font_color", "编辑框字体颜色", "", "", us->editor_font_color);
    addItems("editor_line_bg", "当前行高亮", "选择颜色", "光标所在行背景颜色", "", us->editor_line_bg);
    addItems("cursor_animation", "光标移动动画", "", "", us->cursor_animation);
    addItems("input_animation", "文字输入动画", "符合自然逻辑的文字输入方式，长文章可能卡顿（不兼容微软拼音）", "后续可能支持多种动画效果", us->input_animation);
    addItems("smooth_scroll", "平滑滚动", "调整速度("+QString::number(us->smooth_scroll_speed)+")", "编辑器平滑滚动效果，负数为反向", "带动画，响应可能会慢那么一丝丝，但是看起来更加顺滑", us->smooth_scroll);
    addItems("editor_fuzzy_titlebar", "标题栏模糊", "文字模糊到标题栏，略微影响性能。", "显示菜单栏时，此功能将临时关闭", us->getBool("us/editor_fuzzy_titlebar", true) /*us->editor_fuzzy_titlebar*/);
    addItems("editor_water_effect", "鼠标点击水波", "选择颜色", "清淡的圆形水波纹动画", "后续可能支持多种动画效果", us->editor_water_effect);
    addItems("quote_colorful", "彩色人物语言", "选择颜色", "双引号里面的文字变成彩色", "", us->quote_colorful);
    addItems("chapter_mark", "章内标注", "选择颜色", "在章节内使用以“@”开头的注释，导出作品时自动屏蔽掉", "一下三种情况会自动删除标注：\n1、编辑时不选中文字而直接复制\n2、拖出章节/分卷，直接导出为文件3、作品信息中导出整本作品", us->chapter_mark);
    addItems("editor_underline", "文字下划线", "选择颜色", "每一行底部增加下划线", "", us->editor_underline);
    addItems("editor_underline_middle", "下划线不紧贴文字", "向下偏移量 ("+QString::number(us->editor_underline_offset)+")", "调整下划线与文字之间的距离", "", us->editor_underline_middle);
    addItems("save_power", "强力保存", "实时保存，安全稳定（建议开启）。关闭后需要手动Ctrl+S保存。", "别关别关别关别关别关别关别关!\n好吧……关不关随你……记得 ctrl+s 保存", us->save_power);
    hideItem();
    //addItems("save_interval", "定时保存间隔[不需要]", "单位：分钟", "", us->save_interval);
    addItems("editor_recorder_max", "撤销最大次数", "", "", us->editor_recorder_max);
    hideItem();
    addItems("scroll_cursor_fixed", "光标行固定", "打字机模式，编辑时光标高度不变，反而移动文字", "光标：我只是懒得滚", us->scroll_cursor_fixed);
    hideItem();
    addItems("scroll_bottom_fixed", "底部行固定", "在全文最后面时固定光标行位置", "得配合[底部空白]使用。可能是独有的小细节", us->scroll_bottom_fixed);
    hideItem();
    addItems("scroll_bottom_blank", "底部空白", "在编辑框底部留点空白区域，避免眼睛一直向下看", "可以自定义留白的高度\n0为关闭，1~10设置成总高度的几分之一，大于10为像素单位", us->scroll_bottom_blank);
    hideItem();
    addItems("editor_cursor_width", "编辑框光标宽度", "", "", us->editor_cursor_width);
    hideItem();
    addItems("editor_font_selection", "选中的文字颜色", "", "", us->editor_font_selection);
    hideItem();
    addItems("editor_bg_selection", "选中的文字背景颜色", "", "", us->editor_bg_selection);
    hideItem();
    addItems("editor_line_spacing", "正文行间距", "调整两行之间的距离（一定程度上导致卡顿）", "字数多时（例如上万字）将导致卡顿，请谨慎开启", us->editor_line_spacing);
    hideItem();
    addSplitter();

    /* ==== 功能 ==== */
    addGroup("", "特色功能", "");
    addItems("smart_quotes", "智能引号", "使用双引号键添加标点、移动光标", "根据语境，一键添加双引号、冒号、逗号、句末标点", us->smart_quotes);
    addItems("smart_space", "智能空格", "添加逗号、句末标点、跳过标点、跳过引号", "", us->smart_space);
    addItems("smart_enter", "智能回车", "双引号内回车、自动添加标点", "", us->smart_enter);
    addItems("smart_backspace", "高级删除", "删除成对的符号、连续的标点", "", us->smart_backspace);
    addItems("auto_punc", "自动句末标点", "在语气词或者语言动词后面自动添加上标点符号", "根据神态和语言自动分析，若有错误可直接在后面输入正确的标点来覆盖（需要开启标点覆盖）", us->auto_punc);
    addItems("auto_dqm", "自动双引号", "在“说”、“道”等动词后面自动添加冒号和双引号", "误添加的话，按空格键变为逗号", us->auto_dqm);
    addItems("space_quotes", "空格引号", "段落结尾或者新开头按空格键添加双引号", "其实在空格后面按空格键都能触发", us->space_quotes);
    addItems("tab_complete", "Tab键补全", "使用tab键补全句子结束标点", "只有句号/问号/感叹号三种，不包含逗号\nTab键功能将会覆盖原有的制表符", us->tab_complete);
    addItems("tab_skip", "Tab键跳过", "通过Tab键跳过多个标点/空格等，建议与[Tab键补全]一起食用", "常用语境：双引号内将要结束时，一键补全句末标点并跳出双引号\n也适用于中英结合的句子强制添加标点。（其实就是更懒了呀）", us->tab_skip);
#if defined(Q_OS_MAC)
    addItems("homonym_cover", "同音词覆盖（Mac无效）", QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size()), "使用相同读音的词语来覆盖光标左边已经输入的词语", "适用于手快选词出错的情况\n快捷键支持同一句子内覆盖，不必连续（关闭依旧能用）\n当前问题：无法识别多音字", us->homonym_cover);
#else
    addItems("homonym_cover", "同音词覆盖（Ctrl+D句内手动）", QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size()), "使用相同读音的词语来覆盖光标左边已经输入的词语", "适用于手快选词出错的情况\n快捷键支持同一句子内覆盖，不必连续（关闭依旧能用）\n当前问题：无法识别多音字", us->homonym_cover);
#endif
    addItems("punc_cover", "标点覆盖", "新标点覆盖左边的标点", "专门用来处理自动添加的错误的标点", us->punc_cover);
    hideItem();
    addItems("smart_up_down", "自动上下段", "上/下箭头自动判断段落跳转，或者添加新段落", "", us->smart_up_down);
    hideItem();
    addItems("pair_match", "括号匹配", "自动添加右半部分符号", "会检索整个段落自动判断是否需要补全", us->pair_match);
    hideItem();
    addItems("pair_jump", "括号跳转", "多余的右半符号变为光标右移", "避免括号匹配后重复添加括号", us->pair_jump);
    hideItem();
    addItems("para_after_quote", "多段后引号", "连续多段语言描写，每段后面都加后引号，还是仅最后一段有后引号", "仅在双引号内回车时触发。\n别问为什么有这选项，我的语文老师说的……", us->para_after_quote);
    hideItem();
    addItems("para_auto_split", "自动分段", "在长段落（超过段落阈值）的结尾换行时，自动拆分此长段落", "只针对\"。！？\"这三种标点\n一长串的语言描写不会拆开", us->para_auto_split);
    hideItem();
    addItems("para_max_length", "段落阈值", "当段落文字数（包括标点）超过阈值时，识别为长段落", "", us->para_max_length);
    hideItem();
    addItems("climax_threshold", "高潮模式", "根据输入情况判定为情节激烈程度，此时智能标点将出现更多感叹号", "自动添加标点、按空格键添加标点时，更多的逗号、问号将变为感叹号", us->climax_threshold);
    hideItem();
    addSplitter();

    /* ==== 词典 ==== */
    addGroup("", "词典提示", "");
    addItems("lexicon_synonym", "同义词提示", "输入词语时自动提示同义词", "关闭后，综合搜索结果中也将不显示（下同）\n当然，[综合搜索]功能还未开发，不必在意", us->lexicon_synonym);
    addItems("lexicon_related", "相关词提示", "输入词语时自动提示相关词", "", us->lexicon_related);
    addItems("lexicon_sentence", "常用句提示", "输入词语时提示用到该词语的句子，从[-->]中进入", "选中的句子会复制到剪贴板，ctrl+V 粘贴到任意位置", us->lexicon_sentence);
    addItems("lexicon_random", "随机取名提示", "输入[取名][姓氏][人名]等，随机取名", "请问您叫王富贵吗？", us->lexicon_random);
    addItems("lexicon_surname", "姓氏自动取名", "输入具体的中国姓氏后自动取名字", "不建议开启，因为中国姓氏真的太多了，随便打几个字都会弹出来", us->lexicon_surname);
    addItems("completer_random", "词语乱序显示", "自动提示的词语随机排序", "只在同一类型的内部随机排序\n例如，同义词和相关词不会随机混合到一起，依旧是同义词在前", us->completer_random);
    hideItem();
    addItems("completer_maxnum", "自动提示展示数量", "词句后面自动提示允许同时显示的最大行数", "", us->completer_maxnum);
    hideItem();
    addItems("completer_cursor_delay", "移动光标提示延时", "移动光标后显示词典提示框的延迟时长（毫秒）", "", us->completer_cursor_delay);
    hideItem();
    addItems("completer_input_delay", "输入文字提示延时", "输入文字后显示词典提示框的延迟时长（毫秒）", "不建议太长，可能会挡住中文输入法的输入框", us->completer_input_delay);
    hideItem();
    addItems("insert_sentence", "直接插入句子", "在选中句子后，直接插入到光标处而不是复制到剪贴板", "默认关闭，尽量避免抄袭操作", us->insert_sentence);
    hideItem();
    addSplitter();

    /* ==== 交互 ==== */
    addGroup("", "交互操作", "");
    addItems("one_click", "单击交互", "目录单击打开章节/展开分卷", "", us->one_click);
    addItems("change_apos", "修改单引号", "单击引号实现智能引号效果（将无法输入单引号）", "", us->change_apos);
    //addItems("tab_maxnum", "打开章节个数", "", "", us->tab_maxnum);
    addItems("open_recent", "启动时打开最近一章", "", "自动聚焦到上次编辑的位置", us->open_recent);
    addItems("open_chapter_animation", "打开章节动画", "点击目录打开新章的缩放效果", "偷偷告诉你，这是傻开发者的动画强迫症", us->open_chapter_animation);
    addItems("board_count", "三板板最大数量", "剪贴板/回收板/收藏板 可记录的最大条目数量", "", us->board_count);
    addItems("clip_global", "监听全局剪贴板", "关闭后，剪贴板中只保存章节内复制的文字", "", us->clip_global);
    addItems("clip_limit", "剪贴板最少字数", "值得被记录到剪贴板的最少字数", "", us->clip_limit);
    hideItem();
    addItems("recycle_limit", "回收板最少字数", "值得被记录到回收板的最少字数", "", us->recycle_limit);
    hideItem();
    addItems("auto_detect_update", "自动检测更新", "新版自动后台下载升级，不影响任何操作", "为保证程序功能完整，即使关闭自动更新也会隔较长时间检测更新", us->getBool("us/auto_detect_update", true));
    hideItem();
    addSplitter();

    /* ==== 排版 ==== */
    addGroup("", "文章排版", "");
    addItems("indent_blank", "段首空格", "回车缩进，段首的全角空格数量", "", us->indent_blank);
    addItems("indent_line", "段落空行", "回车时段落间增加空白行", "空行不会影响网站上发布的效果", us->indent_line);
    addItems("typeset_blank", "排版插入空格", "排版（Ctrl+T）时在中文和字母数字之间自动插入一个空格", "会在所有情况下都插入空格，若用到特殊词语请关闭此项", us->typeset_blank);
    addItems("typeset_split", "排版自动分段", "排版时将长段落（超过分段阈值）自动分成多个小段", "这段落，瞎分的，不知准确性如何，请勿过于依赖此功能", us->typeset_split);
    addItems("typeset_paste", "多段粘贴排版", "粘贴多个段落时对插入的内容进行排版（Ctrl+Z撤销排版）", "附加功能：直接拖动/复制TXT文件，视作粘贴", us->typeset_paste);
    addItems("typeset_english", "英文排版", "排版时将单词首字母大写、句末增加空格等", "仅单个字母不会生效", us->typeset_english);

}

void UserSettingsWindow::addItems(QString key, QString title, QString desc, QString tip, QString val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString desc, QString tip, int val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString desc, QString tip, bool val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString btn, QString desc, QString tip, bool val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, btn, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    connect(widget, SIGNAL(signalBtnClicked(UserSettingsItem *, QString)), this, SLOT(slotBtnClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString btn, QString desc, QString tip, QString val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, btn, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    connect(widget, SIGNAL(signalBtnClicked(UserSettingsItem *, QString)), this, SLOT(slotBtnClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString btn, QString desc, QString tip, int val)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, btn, desc, tip, val);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    connect(widget, SIGNAL(signalBtnClicked(UserSettingsItem *, QString)), this, SLOT(slotBtnClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString desc, QString tip, QColor c)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, desc, tip, c);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addItems(QString key, QString title, QString desc, QString tip)
{
    UserSettingsItem *widget = new UserSettingsItem(item_list, key, title, desc, tip);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setSizeHint(QSize(80, widget->getHeight()));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addGroup(QString group_key, QString title, QString desc)
{
    // 点击左边的列表切换到对应的子项
    group2item_indexs.append(item_list->count());

    // 显示的控件
    UserSettingsItem *widget = new UserSettingsItem(item_list, group_key, title, desc);
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setFlags(Qt::ItemIsEnabled);   // Qt::ItemIsUserCheckable 不可选、变灰（影响颜色）      Qt::ItemIsEnabled  仅不可选
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(80, 10 + (widget->getHeight() >> 1)));
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

void UserSettingsWindow::addSplitter()
{
    // 显示的分割线
    UserSettingsItem *widget = new UserSettingsItem(item_list, 2);
    connect(widget, SIGNAL(signalClicked(UserSettingsItem *, QString)), this, SLOT(slotItemClicked(UserSettingsItem *, QString)));
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(100, 20));
    item_list->setItemWidget(item, widget);

    item2group_indexs.append(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    widget->setGroup(group2item_indexs.count() - 1 < 0 ? 0 : group2item_indexs.count() - 1);
    connect(widget, SIGNAL(signalMouseEntered(int)), this, SLOT(slotFocuGroup(int)));
}

/**
 * 隐藏最后一个 item
 */
void UserSettingsWindow::hideItem()
{
    item_list->item(item_list->count() - 1)->setHidden(true);
    //group_indexs.removeLast();
}

void UserSettingsWindow::showEvent(QShowEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(200);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    //QDialog::showEvent(e);
}

void UserSettingsWindow::closeEvent(QCloseEvent *)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(200);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();

    //QDialog::closeEvent(e);
}

void UserSettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        emit signalEsc();
    }

    return QFrame::keyPressEvent(event);
}

/**
 * 滚动到某一项
 * @param x 分组索引
 */
void UserSettingsWindow::slotScrollToGroup(int x)
{
    if (!_flag_group_scroll)
    {
        _flag_group_scroll = true;
        return ;
    }
    if (x >= group_list->count()) return ;
    int index = group2item_indexs.at(x); // 分组对应的下标
    if (index >= item_list->count()) return ;

    int vi = this->height() / ITEMS_HEIGHT; // 能看见的数量
    int index2 = index;
    while (vi-- && index2 < item_list->count())
    {
        if (item_list->item(index2)->isHidden())
            vi++;
        index2++;
    }
    if (index2 >= item_list->count())
        index2 = item_list->count() - 1;
    item_list->scrollTo(item_list->model()->index(index2, 0));
    //itemsList->scrollTo(itemsList->model()->index(index, 0));
    item_list->setCurrentRow(index); // 相比上面的，这个多了选中项
}

void UserSettingsWindow::slotFocuGroup(int g)
{
    _flag_group_scroll = false;
    group_list->setCurrentRow(g);
    _flag_group_scroll = true;
}

/**
 * 项目单击事件
 * @param widget 被单击的控件，用于设置修改后的数据
 * @param key    设置项关键词
 */
void UserSettingsWindow::slotItemClicked(UserSettingsItem *item, QString key)
{
    if (key == "splitter")
    {
        int group = item->getGroup();
        for (int i = 0; i < item_list->count(); i++)
        {
            QWidget *item_widget = item_list->itemWidget(item_list->item(i));
            UserSettingsItem *item = static_cast<UserSettingsItem *>(item_widget);
            if (item->getGroup() == group)
                item_list->item(i)->setHidden(false);
        }
        /*int current_index = item_list->currentRow();
        int current_group = 0; // 所在的分组
        while (current_group < group2item_indexs.size() && current_index >= group2item_indexs[current_group])
            current_group++;
        int end_pos = group2item_indexs[current_group];
        int start_pos = --current_group < 0 ? 0 : group2item_indexs[current_group];
        for (int i = start_pos; i < end_pos; i++)
            item_list->setItemHidden(item_list->item(i), false);*/
    }
    else if (key == "integration_window")
    {
        bool b = us->getBool("us/%1", us->integration_window); // 避免本次点击无效
        b = !b; // 下次重启生效
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "mainwin_titlebar_height")
    {
        int x;
        if (!inputInt(tr("窗口标题栏高度"), tr("请输入主窗口标题栏的高度（10~100），重启生效"), us->widget_size, 10, 100, x))
            return ;
        //us->mainwin_titlebar_height = x; // 重启生效
        us->setVal("us/" + key, x);
        item->setVal(x);
    }
    else if (key == "mainwin_bg_picture")
    {
        // 背景图存在，删除
        if (isFileExist(rt->IMAGE_PATH + "win_bg.jpg") || isFileExist(rt->IMAGE_PATH + "win_bg.png"))
        {
            if (QMessageBox::question(this, QObject::tr("是否删除背景图？"), QObject::tr("删除之后您可以重新设置成其他图片"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
                if (isFileExist(rt->IMAGE_PATH + "win_bg.png"))
                    deleteFile(rt->IMAGE_PATH + "win_bg.png");
                if (isFileExist(rt->IMAGE_PATH + "win_bg.jpg"))
                    deleteFile(rt->IMAGE_PATH + "win_bg.jpg");
                item->setVal(QString(""));

                if (us->dynamic_bg_model == Dynamic_None && us->mainwin_bg_color.alpha() != 255) // 可能是被之前颜色覆盖掉的
                {
                    us->mainwin_bg_color.setAlpha(255); // 恢复完全不透明色
                    us->setColor("us/mainwin_bg_color", us->mainwin_bg_color);
                }

                emit thm->signalBgPicturesChanged();
            }
            return ;
        }

        QString recent = us->getStr("recent/file_path");
        if (recent == "" || !isFileExist(recent))
            recent = ".";
        QString path = QFileDialog::getOpenFileName(this, tr("选择背景图片"), recent, tr("Image Files(*.jpg *.png)"));
        if (path.length() == 0) return ;

        us->mainwin_bg_picture = path; // 这个path好像是带双引号的？（至少debug的时候是带有双引号的）
        item->setVal(tr("清除图片"));
        us->setVal("recent/file_path", path);

        ensureDirExist(rt->IMAGE_PATH);

        if (path.endsWith("png"))
            copyFile(path, rt->IMAGE_PATH + "win_bg.png");
        else if (path.endsWith("jpg"))
        {
            copyFile(path, rt->IMAGE_PATH + "win_bg.jpg");

            if (us->dynamic_bg_model != Dynamic_None) // 先去掉微动背景
            {
                us->dynamic_bg_model = Dynamic_None;
                us->setVal("dybg/model", 0);
                us->loadDynamicBackground();
            }
            if (us->mainwin_bg_color.alpha() == 255) // 如果不透明背景的话，会覆盖背景色
            {
                us->mainwin_bg_color.setAlpha(128); // 设置成50%透明
                us->setColor("us/mainwin_bg_color", us->mainwin_bg_color);
            }
        }
        else
            copyFile(path, rt->IMAGE_PATH + "win_bg.jpg");

        emit thm->signalBgPicturesChanged();
    }
    else if (key == "blur_picture_radius")
    {
        int x;
        if (!inputInt(tr("背景图片模糊半径"), tr("请输入背景图片模糊半径（0~500）"), us->blur_picture_radius, 0, 500, x))
            return;

        us->blur_picture_radius = x;
        us->setVal("us/"+key, x);
        item->setVal(x);

        emit thm->signalBgPicturesChanged();
    }
    else if (key == "mainwin_bg_color")
    {
        QColor color = QColorDialog::getColor(us->mainwin_bg_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid())
            return;
        us->mainwin_bg_color = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        // this->updateUI();
        // parentWidget()->update();
        thm->updateGlobal();
        thm->updateWindow();
    }
    else if (key == "dynamic_background")
    {
        DynamicBackgroundSettings dbs(this);
        dbs.setModal(true);
        dbs.exec();

        QString text;
        if (us->dynamic_bg_model == Dynamic_None)
            text = "关";
        else if (us->dynamic_bg_model == Dynamic_Pure)
            text = "纯色";
        else if (us->dynamic_bg_model == Dynamic_Gradient)
            text = "渐变";
        else if (us->dynamic_bg_model == Dynamic_Island)
            text = "移动";
        else
            text = "其他";

        item->setVal(text);
    }
    else if (key == "emotion_filter_enabled")
    {
        bool &b = us->emotion_filter_enabled;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        emit us->signalEmotionFilterChanged();
    }
    else if (key == "environment_pictures_enabled")
    {
        bool &b = us->envir_picture_enabled;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        emit us->signalEnvironmentPicturesChanged();
    }
    else if (key == "mainwin_sidebar_color")
    {
        QColor color = QColorDialog::getColor(us->mainwin_sidebar_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->mainwin_sidebar_color = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        parentWidget()->update();
        thm->updateWindow();
    }
    else if (key == "mainwin_sidebar_spacing")
    {
        int x;
        if (!inputInt(tr("章节间间距"), tr("请输入章节之间的间距（0~30），重启生效"), us->mainwin_sidebar_spacing, 0, 30, x))
            return ;
        us->mainwin_sidebar_spacing = x;
        us->setVal("us/" + key, x);
        item->setVal(x);
    }
    else if (key == "accent_color")
    {
        QColor color = QColorDialog::getColor(us->accent_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;

        // 修改其他相对应的点缀色
        us->modifyToAccentColorIfIsAccent(us->editor_underline_color, color, "us/editor_underline_color");
        us->modifyToAccentColorIfIsAccent(us->quote_colorful_color, color, "us/quote_colorful_color");
        us->modifyToAccentColorIfIsAccent(us->editor_bg_selection, color, "us/editor_bg_selection");

        us->accent_color = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        this->update();
        parentWidget()->update();

        thm->updateWindow();
    }
    else if (key == "global_font_color")
    {
        QColor color = QColorDialog::getColor(us->global_font_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;

        us->global_font_color = color;
        us->setColor("us/" + key, color);
        thm->updateGlobal();

        item->setVal(color);
        this->updateUI();

        thm->updateWindow();

        for (int i = 0; i < item_list->count(); i++)
        {
            QWidget* item_widget = item_list->itemWidget(item_list->item(i));
            UserSettingsItem *item = static_cast<UserSettingsItem *>(item_widget);
            item->updateUI();
        }
    }
    else if (key == "editor_bg_color")
    {
        QColor color = QColorDialog::getColor(us->editor_bg_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_bg_color = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "editor_bg_selection")
    {
        QColor color = QColorDialog::getColor(us->editor_bg_selection, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_bg_selection = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "editor_font_size")
    {
        int x;
        if (!inputInt(tr("字体大小"), tr("请输入编辑框字体大小（1~250）"), us->editor_font_size, 1, 250, x))
            return ;

        us->editor_font_size = x;
        us->setVal("us/" + key, x);
        item->setVal(x);

        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "editor_font_color")
    {
        QColor color = QColorDialog::getColor(us->editor_font_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_font_color = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "editor_font_selection")
    {
        QColor color = QColorDialog::getColor(us->editor_font_selection, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_font_selection = color;
        us->setColor("us/" + key, color);

        item->setVal(color);
        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "editor_line_bg")
    {
        bool &b = us->editor_line_bg;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (!b) // 关闭，先全部设置成透明
        {
            b = true;
            QColor color(us->editor_line_bg_color);
            us->editor_line_bg_color = QColor(0,0,0,0);
            thm->updateEditor();
            us->editor_line_bg_color = color;
            b = false;
        }
        else
        {
            thm->updateEditor();
        }
    }
    else if (key == "editor_water_effect")
    {
        bool &b = us->editor_water_effect;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "editor_fuzzy_titlebar")
    {
        bool& b = us->editor_fuzzy_titlebar; // 重启生效
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
        emit us->signalBlurTitlebarChanged(b);
    }
    else if (key == "editor_cursor_width")
    {
        int x;
        if (!inputInt(tr("光标宽度"), tr("请输入编辑框光标宽度（1~5）"), us->editor_cursor_width, 1, 5, x))
            return ;

        us->editor_cursor_width = x;
        us->setVal("us/" + key, x);
        item->setVal(x);

        parentWidget()->update();

        thm->updateEditor();
    }
    else if (key == "quote_colorful")
    {
        bool &b = us->quote_colorful;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
        thm->updateHighlight();
    }
    else if (key == "chapter_mark")
    {
        bool &b = us->chapter_mark;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
        thm->updateHighlight();
    }
    else if (key == "settings_in_mainwin")
    {
        bool b = us->getBool("us/settings_in_mainwin", us->settings_in_mainwin); //us->settings_in_mainwin; // 避免本次点击无效
        b = !b; // 下次重启生效
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "round_view")
    {
        bool &b = us->round_view;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "split_shade")
    {
        bool &b = us->split_shade;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "one_click")
    {
        bool &b = us->one_click;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "tab_maxnum")
    {
        int x;
        if (!inputInt(tr("打开数量"), tr("请输入打开章节的最大数量（1~999）"), us->tab_maxnum, 1, 999, x))
            return ;

        us->tab_maxnum = x;
        us->setVal(QString("us/%1").arg(key), x);
        item->setVal(x);
    }
    else if (key == "open_chapter_animation")
    {
        bool &b = us->open_chapter_animation;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "clip_global")
    {
        bool &b = us->clip_global;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->boards.startConnectClipboard();
        else
            gd->boards.stopConnectClipboard();
    }
    else if (key == "board_count")
    {
        int x;
        if (!inputInt(tr("三板板最大数量"), tr("请输入剪贴板/回收板/收藏板的最大条目数量（5~999）"), us->board_count, 5, 999, x))
            return ;

        us->board_count = x;
        us->setVal("us/board_count", x);
        item->setVal(x);
        gd->boards.setBoardCount(x);
    }
    else if (key == "clip_limit")
    {
        int x;
        if (!inputInt(tr("剪贴板最少字数"), tr("请输入记录到剪贴板的最少字数（0~100000）"), us->clip_limit, 1, 100000, x))
            return ;

        us->clip_limit = x;
        us->setVal("clip_limit", x);
        item->setVal(x);
        gd->boards.setLimits(us->clip_limit, us->recycle_limit);
    }
    else if (key == "recycle_limit")
    {
        int x;
        if (!inputInt(tr("回收板最少字数"), tr("请输入记录到回收板的最少字数（0~100000）"), us->recycle_limit, 1, 100000, x))
            return ;

        us->recycle_limit = x;
        us->setVal("recycle_limit", x);
        item->setVal(x);
        gd->boards.setLimits(us->clip_limit, us->recycle_limit);
    }
    else if (key == "open_recent")
    {
        bool &b = us->open_recent;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "save_power")
    {
        bool &b = us->save_power;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "save_interval")
    {
        int x;
        if (!inputInt(tr("定时保存"), tr("请输入定时保存的间隔（1~999），单位：分钟。0为关闭定时保存。"), us->save_interval, 0, 999, x))
            return ;

        us->save_interval = x;
        us->setVal(QString("us/%1").arg(key), x);
        item->setVal(x);
    }
    else if (key == "smooth_scroll")
    {
        bool &b = us->smooth_scroll;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "editor_recorder_max")
    {
        int x;
        if (!inputInt(tr("撤销次数"), tr("请输入撤销的最大次数（10~999）"), us->editor_recorder_max, 10, 999, x))
            return ;

        us->editor_recorder_max = x;
        us->setVal(QString("us/%1").arg(key), x);
        item->setVal(x);
    }
    else if (key == "scroll_bottom_blank")
    {
        int x;
        if (!inputInt(tr("底部空白"), tr("请输入空白高度（0~9999），0为关闭，1~10设置成总高度的几分之一，大于10为像素单位"), us->scroll_bottom_blank, 0, 9999, x))
            return ;

        us->scroll_bottom_blank = x;
        us->setVal(QString("us/%1").arg(key), x);
        item->setVal(x);

        emit signalEditorBottomSettingsChanged();
    }
    else if (key == "scroll_bottom_fixed")
    {
        bool &b = us->scroll_bottom_fixed;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        emit signalEditorBottomSettingsChanged();
    }
    else if (key == "scroll_cursor_fixed")
    {
        bool &b = us->scroll_cursor_fixed;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        emit signalEditorBottomSettingsChanged();
    }
    else if (key == "cursor_animation")
    {
        bool &b = us->cursor_animation;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "input_animation")
    {
        bool &b = us->input_animation;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "auto_detect_update")
    {
        bool b = us->getBool("us/auto_detect_update");
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
        if (b)
        {
            us->setVal("runtime/check_update", 0);
            emit signalDetectUpdate();
        }
    }
    else if (key == "indent_blank")
    {
        int x;
        if (!inputInt(tr("段首缩进"), tr("请输入段落开头的空格缩进量（0~10）"), us->indent_blank, 0, 10, x))
            return ;

        us->indent_blank = x;
        us->setVal("us/indent_blank", x);
        item->setVal(x);
    }
    else if (key == "indent_line")
    {
        int x;
        if (!inputInt(tr("段间空行"), tr("请输入两段之间的空白行数量（0~10）"), us->indent_line, 0, 10, x))
            return ;

        us->indent_line = x;
        us->setVal("us/indent_line", x);
        item->setVal(x);
    }
    else if (key == "editor_line_spacing")
    {
        int x;
        if (!inputInt(tr("行间距（若出现卡顿请清零）"), tr("请输入两段之间的空白行数量（0~50）"), us->editor_line_spacing, 0, 50, x))
            return ;

        if (x == 0) // 关闭：先设置为1，再关闭（只差1应该不怎么看得出来的吧？）
        {
            item->setDesc("已关闭，修改任意内容后应用修改");
        }
        else
        {
            item->setDesc("若在编辑或者打开章节时出现明显卡顿，请关闭此项");
        }

        us->editor_line_spacing = x;
        us->setVal("us/editor_line_spacing", x);
        item->setVal(x);

        thm->updateEditor();
    }
    else if (key == "editor_underline")
    {
        bool &b = us->editor_underline;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "editor_underline_middle")
    {
        bool &b = us->editor_underline_middle;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "smart_quotes")
    {
        bool &b = us->smart_quotes;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "change_apos")
    {
        bool &b = us->change_apos;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "smart_space")
    {
        bool &b = us->smart_space;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "smart_enter")
    {
        bool &b = us->smart_enter;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "smart_backspace")
    {
        bool &b = us->smart_backspace;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "auto_punc")
    {
        bool &b = us->auto_punc;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "auto_dqm")
    {
        bool &b = us->auto_dqm;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "punc_cover")
    {
        bool &b = us->punc_cover;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "space_quotes")
    {
        bool &b = us->space_quotes;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "smart_up_down")
    {
        bool &b = us->smart_up_down;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "pair_match")
    {
        bool &b = us->pair_match;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "pair_jump")
    {
        bool &b = us->pair_jump;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "tab_complete")
    {
        bool &b = us->tab_complete;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "tab_skip")
    {
        bool &b = us->tab_skip;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "homonym_cover")
    {
        bool &b = us->homonym_cover;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "para_after_quote")
    {
        bool &b = us->para_after_quote;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "para_auto_split")
    {
        bool &b = us->para_auto_split;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "para_max_length")
    {
        int x;
        if (!inputInt(tr("自动分段阈值"), tr("当段落长度超过这个次数时进行分段（100~500）"), us->para_max_length, 100, 500, x))
            return ;

        us->para_max_length = x;
        us->setVal("us/para_max_length", x);
        item->setVal(x);
    }
    else if (key == "climax_threshold")
    {
        int x;
        if (!inputInt(tr("高潮阈值"), tr("判断为激烈情节的阈值，越低越易触发（0~1000）"), us->climax_threshold, 0, 1000, x))
            return ;

        us->climax_threshold = x;
        us->setVal("us/climax_threshold", x);
        item->setVal(x);

        if (us->climax_value > us->climax_threshold)
        {
            us->climax_value = us->climax_threshold;
            us->setVal("us/climax_value", us->climax_value);
        }
    }
    else if (key == "typeset_split")
    {
        bool &b = us->typeset_split;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "typeset_blank")
    {
        bool &b = us->typeset_blank;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "typeset_paste")
    {
        bool &b = us->typeset_paste;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "typeset_english")
    {
        bool &b = us->typeset_english;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "lexicon_synonym")
    {
        bool &b = us->lexicon_synonym;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->lexicons.initSynonym();
        else
            gd->lexicons.uninitAll(true, false, false, false);
    }
    else if (key == "lexicon_related")
    {
        bool &b = us->lexicon_related;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->lexicons.initRelated();
        else
            gd->lexicons.uninitAll(false, true, false, false);
    }
    else if (key == "lexicon_sentence")
    {
        bool &b = us->lexicon_sentence;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->lexicons.initSentence();
        else
            gd->lexicons.uninitAll(false, false, true, false);
    }
    else if (key == "lexicon_random")
    {
        bool &b = us->lexicon_random;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->lexicons.initRandomName();
        else
            gd->lexicons.uninitAll(false, false, false, true);
    }
    else if (key == "lexicon_surname")
    {
        bool &b = us->lexicon_surname;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);

        if (b)
            gd->lexicons.initRandomName();
        else
            gd->lexicons.uninitAll(false, false, false, true);
    }
    else if (key == "completer_input_delay")
    {
        int x;
        if (!inputInt(tr("延迟时长"), tr("输入文字后出现词语提示框的延迟时长，毫秒（1~10000）"), us->completer_input_delay, 1, 10000, x))
            return ;

        us->completer_input_delay = x;
        us->setVal("us/" + key, x);
        item->setVal(x);
    }
    else if (key == "completer_cursor_delay")
    {
        int x;
        if (!inputInt(tr("延迟时长"), tr("移动光标后出现词语提示框的延迟时长，毫秒（1~10000）"), us->completer_cursor_delay, 1, 10000, x))
            return ;

        us->completer_cursor_delay = x;
        us->setVal("us/" + key, x);
        item->setVal(x);
    }
    else if (key == "completer_maxnum")
    {
        int x;
        if (!inputInt(tr("显示行数"), tr("自动提示框同时显示的最大项目数（1~10）"), us->completer_maxnum, 1, 10, x))
            return ;

        us->completer_maxnum = x;
        us->setVal("us/" + key, x);
        item->setVal(x);
        thm->updateEditor();
    }
    else if (key == "completer_random")
    {
        bool &b = us->completer_random;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    else if (key == "insert_sentence")
    {
        bool &b = us->insert_sentence;
        b = !b;
        us->setVal(QString("us/%1").arg(key), b);
        item->setVal(b);
    }
    us->sync();
}

void UserSettingsWindow::slotBtnClicked(UserSettingsItem *item, QString key)
{
    if (key == "homonym_cover")
    {
        HomonymCoverWindow hcw(this);
        hcw.setModal(true);
        hcw.exec();

        item->setBtn(QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size()));
    }
    else if (key == "about")
    {
        QDesktopServices::openUrl(QUrl(QString("https://shang.qq.com/wpa/qunwpa?idkey=cb424b14c6897cca5c0c10b5cd63e9db61502be5fcadbea5c8151add07689c9e")));
    }
    else if (key == "editor_font_size")
    {
        bool ok;
        QFont font(this->font());
        font = QFontDialog::getFont(&ok, font, this);
        if (ok)
        {
            QApplication::setFont(font);
            us->setVal("us/font", font.toString());
            item->setDesc("强烈建议重启，避免字体显示问题。");
            log("设置字体", font.toString());
        }
        else
        {
            log("设置字体失败", font.toString());
        }
    }
    else if (key == "editor_water_effect")
    {
        QColor color = QColorDialog::getColor(us->editor_water_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_water_color = color;
        us->setColor("us/editor_water_color", color);
    }
    else if (key == "quote_colorful")
    {
        QColor color = QColorDialog::getColor(us->quote_colorful_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->quote_colorful_color = color;
        us->setColor("us/quote_colorful_color", color);

        thm->updateHighlight();
    }
    else if (key == "chapter_mark")
    {
        QColor color = QColorDialog::getColor(us->chapter_mark_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->chapter_mark_color = color;
        us->setColor("us/chapter_mark_color", color);

        thm->updateHighlight();
    }
    else if (key == "editor_underline")
    {
        QColor color = QColorDialog::getColor(us->editor_underline_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_underline_color = color;
        us->setColor("us/editor_underline_color", color);

        thm->updateHighlight();
    }
    else if (key == "editor_underline_middle")
    {
        int x;
        if (!inputInt(tr("偏移量"), tr("请输下划线向下偏移的距离（-25~+25）"), us->editor_underline_offset, -25, 25, x))
            return ;

        us->editor_underline_offset = x;
        us->setVal(QString("us/editor_underline_offset"), x);
        item->setBtn("向下偏移量 ("+QString::number(x)+")");
    }
    else if (key == "editor_line_bg")
    {
        QColor color = QColorDialog::getColor(us->editor_line_bg_color, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        us->editor_line_bg_color = color;
        us->setColor("us/editor_line_bg_color", color);

        thm->updateEditor();
    }
    else if (key == "smooth_scroll")
    {
        int x;
        if (!inputInt(tr("滚动速度"), tr("一次平滑滚动的像素数量（-1000~+1000）"), us->smooth_scroll_speed, -1000, 1000, x))
            return ;

        us->smooth_scroll_speed = x;
        us->setVal(QString("us/smooth_scroll_speed"), x);
        item->setBtn("滚动速度 ("+QString::number(x)+")");
    }
}

/**
 * 输入一个整数，并且设置最大值、最小值
 * @param title 标题
 * @param desc  描述
 * @param def   默认值
 * @param min   最小值
 * @param max   最大值
 * @param rst   结果（引用）
 * @return      是否成功
 */
bool UserSettingsWindow::inputInt(QString title, QString desc, int def, int min, int max, int &rst)
{
    bool ok;
    QString text = QInputDialog::getText(this, title, desc, QLineEdit::Normal, QString("%1").arg(def), &ok);
    if (!ok) return false; // 没有输入
    if (!canRegExp(text, "^-?\\d+$")) // 不是纯数字
    {
        QMessageBox::information(this, tr("错误"), tr("请输入纯数字"));
        return false;
    }

    int x = text.toInt();
    if ((min != max) && (x < min || x > max)) // 超过范围
    {
        QMessageBox::information(this, tr("错误"), tr("请输入 %1~%2 之间的数字").arg(min).arg(max));
        return false;
    }

    rst = x;
    return true;
}
