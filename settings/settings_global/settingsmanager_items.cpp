#include "settingsmanager.h"

/**
 * 初始化所有的项目
 */
void SettingsManager::initItems(QString g)
{
    if (!g.isEmpty())
    {
        for (int i = 0; i < items.size(); ++i)
        {
            SettingBean* sb = items.at(i);
            if (sb->group == g)
                items.removeAt(i--);
        }
    }
    else
    {
        items.clear();
    }

    QString text;

    group_name = "常用";
    if (g.isEmpty() || g == group_name)
    {
        addItem("about", "写作天下", "反馈/建议/素材/合作", "");
        addItemChild(addItem("QQ_qun", "一键加群", "QQ群：705849222", "一键加群：705849222"));
    }

    group_name = "主题色彩";
    if (g.isEmpty() || g == group_name)
    {
        addItem("mainwin_bg_picture", "窗口背景图片", "建议修改颜色透明度（鼠标悬浮查看提示）", "修改图片后缀名产生不同的效果：\npng格式图片会覆盖在背景颜色上面（如果透明的话）\njpg格式会被背景颜色覆盖（可调整颜色透明度，255时禁用纯色）\n允许jpg和png图片同时使用\n后续可能支持gif动图与可交互的动态背景", &us->mainwin_bg_picture);
        addItem("blur_picture_radius", "背景图片模糊半径", "只针对 jpg 背景有效", "可用png覆盖在jpg之上，起到混合效果", &us->blur_picture_radius);
        setItemRange(0, 500);
        addItem("mainwin_bg_color", "主色调", "各窗口背景颜色" + (us->dynamic_bg_model ? QString("(可能会被微动背景屏蔽)") : ""), "仅在微动背景关闭时生效", &us->mainwin_bg_color);
        addItem("mainwin_sidebar_color", "副色调", "目录/搜索栏等小组件背景颜色", "只针对一体化界面（隐藏功能）有效", &us->mainwin_sidebar_color);
        addItem("accent_color", "点缀色", "强调色，例如指示条、当前选项", "", &us->accent_color);
        addItem("dynamic_background", "微动背景", "伪静态背景，不妨碍创作又增加乐趣", "", (int*)&us->dynamic_bg_model, QStringList{"关", "纯色", "渐变", "移动", "其他"});
        addItem("emotion_filter_enabled", "情景滤镜", "智能分析情节中人物情绪、所处环境，为背景添加相应滤镜", "", &us->emotion_filter_enabled);
        text = (isFileExist(rt->IMAGE_PATH + "win_bg.jpg") || isFileExist(rt->IMAGE_PATH + "win_bg.png")) ? "（和窗口图片冲突）" : "";
        addItem("environment_pictures_enabled", "场景图片", "识别小说中的故事环境，以对应的图片作为背景图片" + text, "", &us->envir_picture_enabled);
    }

    group_name = "全局外观";
    if (g.isEmpty() || g == group_name)
    {
        addItem("mainwin_titlebar_height", "标题栏高度", "顶部标题栏和部分按钮的大小。重启生效", "只针对一体化界面（隐藏功能）有效，并且受到字体影响。", &us->widget_size);
        setItemRange(10, 100);
        setItemRestart();
        addItem("global_font", "全局字体", "", "");
        setItemRestart();
        addItemChild(addItem("global_font_color", "全局字体颜色", "除正文外所有文字颜色", "全局字体颜色", &us->global_font_color));
        addItem("round_view", "圆角控件", "使用全新的局部UI效果（重启生效）", "", &us->round_view);
        setItemRestart();
        addItem("mainwin_sidebar_spacing", "目录章节列表间距", "", "", &us->mainwin_sidebar_spacing);
        setItemRange(0, 30);
        setItemHideen();
        addItem("settings_in_mainwin", "设置界面嵌入主窗口", "在主窗口中打开设置界面，显示背景模糊效果（重启生效）", "关闭此项可实时看到界面设置的改变情况", &us->settings_in_mainwin);
        setItemRestart();
        setItemHideen();
        addItem("split_shade", "明显的分割线", "显示分割线阴影、分界线线条", "", &us->split_shade);
        // addItem("integration_window", "一体化界面", "替代原生窗口，更加美观", "【一体化】与【原生】的区别：\n标题栏支持填充背景颜色/背景图片\n更好看的打开章节动画效果\n无法从边缘调整大小（右下角可调整）\n无法肆意移动目录位置", &us->integration_window);
    }

    group_name = "编辑器";
    if (g.isEmpty() || g == group_name)
    {
        addItem("editor_font_size", "编辑框字体大小", "", "自定义字体方法：“font.ttf”字体文件放到“软件目录下/data/style/”", &us->editor_font_size);
        setItemRange(1, 250);
        addItem("editor_font_color", "编辑框字体颜色", "", "", &us->editor_font_color);
        addItem("editor_line_bg", "当前行高亮", "光标所在行背景颜色(修改内容后生效)", "", &us->editor_line_bg);
        addItemChild(addItem("editor_line_bg_color", "当前行背景颜色", "", "当前行背景颜色", &us->editor_line_bg_color));
        addItem("quote_colorful", "彩色人物语言", "双引号里面的文字变成彩色", "", &us->quote_colorful);
        addItemChild(addItem("quote_colorful_color", "人物语言颜色", "", "语言颜色", &us->quote_colorful_color));
        addItem("chapter_mark", "章内标注", "在章节内使用以“@”开头的注释，导出作品时自动屏蔽掉", "一下三种情况会自动删除标注：\n1、编辑时不选中文字而直接复制\n2、拖出章节/分卷，直接导出为文件3、作品信息中导出整本作品", &us->chapter_mark);
        addItemChild(addItem("chapter_mark_color", "章内标注颜色", "", "标注颜色", &us->chapter_mark_color));
        addItem("editor_underline", "文字下划线", "每一行底部增加下划线", "", &us->editor_underline);
        addItemChild(addItem("editor_underline_color", "下划线颜色", "", "下划线颜色", &us->editor_underline_color));
        addItem("editor_underline_middle", "下划线不紧贴文字", "调整下划线与文字之间的距离", "", &us->editor_underline_middle);
        setItemRange(-25, 25);
        addItemChild(addItem("editor_underline_offset", "下划线偏移", "", "向下偏移量 (" + QString::number(us->editor_underline_offset) + ")", &us->editor_underline_offset));
        addItem("cursor_animation", "光标移动动画", "", "", &us->cursor_animation);
        addItem("input_animation", "文字输入动画", "符合自然逻辑的文字输入方式，长文章可能卡顿（不兼容微软拼音）", "后续可能支持多种动画效果", &us->input_animation);
        addItem("save_power", "强力保存", "实时保存，安全稳定（建议开启）。关闭后需要手动Ctrl+S保存。", "别关别关别关别关别关别关别关!\n好吧……关不关随你……记得 ctrl+s 保存", &us->save_power);
        setItemHideen();
        addItem("editor_recorder_max", "撤销最大次数", "", "", &us->editor_recorder_max);
        setItemRange(10, 999);
        setItemHideen();
        addItem("scroll_cursor_fixed", "光标行固定", "打字机模式，编辑时光标高度不变，反而移动文字", "光标：我只是懒得滚", &us->scroll_cursor_fixed);
        addItem("scroll_bottom_fixed", "底部行固定", "在全文最后面时固定光标行位置", "得配合[底部空白]使用。可能是独有的小细节", &us->scroll_bottom_fixed);
        addItem("scroll_bottom_blank", "底部空白", "在编辑框底部留点空白区域，避免眼睛一直向下看", "可以自定义留白的高度\n0为关闭，1~10设置成总高度的几分之一，大于10为像素单位", &us->scroll_bottom_blank);
        addItem("editor_cursor_width", "编辑框光标宽度", "", "", &us->editor_cursor_width);
        setItemRange(1, 5);
        addItem("editor_font_selection", "选中的文字颜色", "", "", &us->editor_font_selection);
        addItem("editor_bg_selection", "选中的文字背景颜色", "", "", &us->editor_bg_selection);
        addItem("editor_line_spacing", "正文行间距", "调整两行之间的距离（一定程度上导致卡顿）", "字数多时（例如上万字）将导致卡顿，请谨慎开启", &us->editor_line_spacing);
        setItemRange(0, 50);
        setItemHideen();
        // addItem("editor_bg_color", "编辑框背景颜色", "", "", &us->editor_bg_color);
        // addItem("save_interval", "定时保存间隔[不需要]", "单位：分钟", "", &us->save_interval);
        // setItemRange(1, 999);
    }

    group_name = "特色功能";
    if (g.isEmpty() || g == group_name)
    {
        addItem("smart_quotes", "智能引号", "使用双引号键添加标点、移动光标", "根据语境，一键添加双引号、冒号、逗号、句末标点", &us->smart_quotes);
        addItem("smart_space", "智能空格", "添加逗号、句末标点、跳过标点、跳过引号", "", &us->smart_space);
        addItem("smart_enter", "智能回车", "双引号内回车、自动添加标点", "", &us->smart_enter);
        addItem("smart_backspace", "高级删除", "删除成对的符号、连续的标点", "", &us->smart_backspace);
        addItem("auto_punc", "自动句末标点", "在语气词或者语言动词后面自动添加上标点符号", "根据神态和语言自动分析，若有错误可直接在后面输入正确的标点来覆盖（需要开启标点覆盖）", &us->auto_punc);
        addItem("auto_dqm", "自动双引号", "在“说”、“道”等动词后面自动添加冒号和双引号", "误添加的话，按空格键变为逗号", &us->auto_dqm);
        addItem("space_quotes", "空格引号", "段落结尾或者新开头按空格键添加双引号", "其实在空格后面按空格键都能触发", &us->space_quotes);
        addItem("tab_complete", "Tab键补全", "使用tab键补全句子结束标点", "只有句号/问号/感叹号三种，不包含逗号\nTab键功能将会覆盖原有的制表符", &us->tab_complete);
        addItem("tab_skip", "Tab键跳过", "通过Tab键跳过多个标点/空格等，建议与[Tab键补全]一起食用", "常用语境：双引号内将要结束时，一键补全句末标点并跳出双引号\n也适用于中英结合的句子强制添加标点。（其实就是更懒了呀）", &us->tab_skip);
#if defined(Q_OS_MAC)
        addItem("homonym_cover", "同音词覆盖（Mac无效）", QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size()), "使用相同读音的词语来覆盖光标左边已经输入的词语", "适用于手快选词出错的情况\n快捷键支持同一句子内覆盖，不必连续（关闭依旧能用）\n当前问题：无法识别多音字", &us->homonym_cover);
#else
        addItem("homonym_cover", "同音词覆盖（Ctrl+D句内手动）", "使用相同读音的词语来覆盖光标左边已经输入的词语", "适用于手快选词出错的情况\n快捷键支持同一句子内覆盖，不必连续（关闭依旧能用）\n当前问题：无法识别多音字", &us->homonym_cover);
        addItem("homonym_cover_blacklist", "同音词覆盖黑名单", QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size()), "");
#endif
        addItem("punc_cover", "标点覆盖", "新标点覆盖左边的标点", "专门用来处理自动添加的错误的标点", &us->punc_cover);
        addItem("smart_up_down", "自动上下段", "上/下箭头自动判断段落跳转，或者添加新段落", "", &us->smart_up_down);
        addItem("pair_match", "括号匹配", "自动添加右半部分符号", "会检索整个段落自动判断是否需要补全", &us->pair_match);
        addItem("pair_jump", "括号跳转", "多余的右半符号变为光标右移", "避免括号匹配后重复添加括号", &us->pair_jump);
        addItem("para_after_quote", "多段后引号", "连续多段语言描写，每段后面都加后引号，还是仅最后一段有后引号", "仅在双引号内回车时触发。\n别问为什么有这选项，我的语文老师说的……", &us->para_after_quote);
        addItem("para_auto_split", "自动分段", "在长段落（超过段落阈值）的结尾换行时，自动拆分此长段落", "只针对\"。！？\"这三种标点\n一长串的语言描写不会拆开", &us->para_auto_split);
        addItem("para_max_length", "段落阈值", "当段落文字数（包括标点）超过阈值时，识别为长段落", "", &us->para_max_length);
        setItemRange(100, 1000);
        addItem("climax_threshold", "高潮模式", "根据输入情况判定为情节激烈程度，此时智能标点将出现更多感叹号", "自动添加标点、按空格键添加标点时，更多的逗号、问号将变为感叹号", &us->climax_threshold);
        setItemRange(0, 1000);
    }

    group_name = "人机交互";
    if (g.isEmpty() || g == group_name)
    {
        addItem("one_click", "单击交互", "目录单击打开章节/展开分卷", "", &us->one_click);
        addItem("change_apos", "修改单引号", "单击引号实现智能引号效果（将无法输入单引号）", "", &us->change_apos);
        //addItem("tab_maxnum", "打开章节个数", "", "", &us->tab_maxnum);
        //setItemRange(1, 999);
        //setItemHideen();
        addItem("open_recent", "启动时打开最近一章", "", "自动聚焦到上次编辑的位置", &us->open_recent);
        addItem("board_count", "三板板最大数量", "剪贴板/回收板/收藏板 可记录的最大条目数量", "", &us->board_count);
        setItemRange(5, 999);
        setItemHideen();
        addItem("clip_global", "监听全局剪贴板", "关闭后，剪贴板中只保存章节内复制的文字", "", &us->clip_global);
        addItem("clip_limit", "剪贴板最少字数", "值得被记录到剪贴板的最少字数", "", &us->clip_limit);
        setItemRange(0, 100000);
        setItemHideen();
        addItem("recycle_limit", "回收板最少字数", "值得被记录到回收板的最少字数", "", &us->recycle_limit);
        setItemRange(0, 100000);
        setItemHideen();
        addItem("auto_detect_update", "自动检测更新", "新版自动后台下载升级，不影响任何操作", "为保证程序功能完整，即使关闭自动更新也会隔较长时间检测更新", &us->auto_detect_update);
        setItemHideen();
    }

    group_name = "操作动画";
    if (g.isEmpty() || g == group_name)
    {
        addItem("smooth_scroll", "平滑滚动", "编辑器平滑滚动效果，负数为反向", "带动画，响应可能会慢那么一丝丝，但是看起来更加顺滑", &us->smooth_scroll);
        addItemChild(addItem("smooth_scroll_speed", "平滑滚动速度", "", "调整速度(" + QString::number(us->smooth_scroll_speed) + ")", &us->smooth_scroll_speed));
        setItemRange(-1000, 1000);
        addItem("editor_fuzzy_titlebar", "标题栏模糊", "文字模糊到标题栏，略微影响性能。", "显示菜单栏时，此功能将临时关闭", &us->editor_fuzzy_titlebar);
        addItem("editor_water_effect", "鼠标点击水波", "清淡的圆形水波纹动画", "后续可能支持多种动画效果", &us->editor_water_effect);
        addItemChild(addItem("editor_water_color", "鼠标点击水波纹颜色", "", "水波纹颜色", &us->editor_water_color));
        addItem("open_chapter_animation", "打开章节动画", "点击目录打开新章的缩放效果", "偷偷告诉你，这是傻开发者的动画强迫症", &us->open_chapter_animation);
    }

    group_name = "词典资源";
    if (g.isEmpty() || g == group_name)
    {
        addItem("auto_suggestion", "自动提示开关", "输入内容后是否开启自动提示", "", &us->auto_suggestion);
        addItem("lexicon_synonym", "同义词提示", "输入词语时自动提示同义词", "关闭后，综合搜索结果中也将不显示（下同）\n当然，[综合搜索]功能还未开发，不必在意", &us->lexicon_synonym);
        addItem("lexicon_related", "相关词提示", "输入词语时自动提示相关词", "", &us->lexicon_related);
        addItem("lexicon_sentence", "常用句提示", "输入词语时提示用到该词语的句子，从[-->]中进入", "选中的句子会复制到剪贴板，ctrl+V 粘贴到任意位置", &us->lexicon_sentence);
        addItem("lexicon_random", "随机取名提示", "输入[取名][姓氏][人名]等，随机取名", "请问您叫王富贵吗？", &us->lexicon_random);
        addItem("lexicon_surname", "姓氏自动取名", "输入具体的中国姓氏后自动取名字", "不建议开启，因为中国姓氏真的太多了，随便打几个字都会弹出来", &us->lexicon_surname);
        addItem("completer_random", "词语乱序显示", "自动提示的词语随机排序", "只在同一类型的内部随机排序\n例如，同义词和相关词不会随机混合到一起，依旧是同义词在前", &us->completer_random);
        addItem("completer_maxnum", "自动提示展示数量", "词句后面自动提示允许同时显示的最大行数", "", &us->completer_maxnum);
        setItemRange(1, 10);
        setItemHideen();
        addItem("completer_cursor_delay", "移动光标提示延时", "移动光标后显示词典提示框的延迟时长（毫秒）", "", &us->completer_cursor_delay);
        setItemRange(1, 10000);
        setItemHideen();
        addItem("completer_input_delay", "输入文字提示延时", "输入文字后显示词典提示框的延迟时长（毫秒）", "不建议太长，可能会挡住中文输入法的输入框", &us->completer_input_delay);
        setItemRange(1, 10000);
        setItemHideen();
        addItem("insert_sentence", "直接插入句子", "在选中句子后，直接插入到光标处而不是复制到剪贴板", "默认关闭，尽量避免抄袭操作", &us->insert_sentence);
        setItemHideen();
    }

    group_name = "一键排版";
    if (g.isEmpty() || g == group_name)
    {
        addItem("indent_blank", "段首空格", "回车缩进，段首的全角空格数量", "", &us->indent_blank);
        setItemRange(0, 10);
        addItem("indent_line", "段落空行", "回车时段落间增加空白行", "空行不会影响网站上发布的效果", &us->indent_line);
        setItemRange(0, 10);
        addItem("typeset_blank", "排版插入空格", "排版（Ctrl+T）时在中文和字母数字之间自动插入一个空格", "会在所有情况下都插入空格，若用到特殊词语请关闭此项", &us->typeset_blank);
        addItem("typeset_split", "排版自动分段", "排版时将长段落（超过分段阈值）自动分成多个小段", "这段落，瞎分的，不知准确性如何，请勿过于依赖此功能", &us->typeset_split);
        addItem("typeset_paste", "多段粘贴排版", "粘贴多个段落时对插入的内容进行排版（Ctrl+Z撤销排版）", "附加功能：直接拖动/复制TXT文件，视作粘贴", &us->typeset_paste);
        addItem("typeset_english", "英文排版", "排版时将单词首字母大写、句末增加空格等", "仅单个字母不会生效", &us->typeset_english);
    }

    group_name = "关于";
    if (g.isEmpty() || g == group_name)
    {

    }
}

/**
 * 通过key来触发设置项修改事件
 * @param key    设置项
 * @param widget 触发的控件（用户界面）
 * @return 是否进行修改了（可能会取消）
 */
bool SettingsManager::trigger(QString key, QWidget* widget)
{
    // 根据key设置每一项的内容


    // 设置项修改完毕的信号槽
    emit signalTriggered(key);
    return true;
}
