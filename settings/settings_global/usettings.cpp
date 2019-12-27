#include "usettings.h"

USettings::USettings(QString filePath) : Settings(filePath),
        homonym_cover_data(HomonymCoverData::getDirFromFile(filePath)),
        dynamic_bg(nullptr)
{
    init();
}

void USettings::init()
{
    is_debug = true;

    loadFromFile();
    loadDynamicBackground();
}

void USettings::loadFromFile()
{
    /* ---- 全局 ---- */


    /* ---- 界面 ---- */
    widget_size             = getInt("us/mainwin_titlebar_height", 35);
    mainwin_bg_picture      = getStr("us/mainwin_bg_picture", "");
    mainwin_bg_color        = getColor("us/mainwin_bg_color", QColor(0xFF, 0xFF, 0xFF));
    blur_picture_radius     = getInt("us/blur_picture_radius", 80);

    mainwin_sidebar_hide    = getBool("us/mainwin_sidebar_hide", false);
    mainwin_sidebar_width   = getInt("us/mainwin_sidebar_width", 200);
    mainwin_sidebar_color   = getColor("us/mainwin_sidebar_color", QColor(0xEE, 0xEE, 0xEE, 0x0));
    mainwin_sidebar_spacing = getInt("us/mainwin_sidebar_spacing", 8);
    dir_item_bg_model       = getInt("us/dir_item_bg_model", 1);
    dir_item_text_model     = getInt("us/dir_item_text_model", 1);
    dir_item_open_model     = getInt("us/dir_item_open_model", 1);

    global_font_color       = getColor("us/global_font_color", Qt::black);
    accent_color            = getColor("us/accent_color", QColor(0x64, 0x95, 0xED)); // 点缀色，修改时要判断其他颜色，如果和点缀色一模一样，则也同时进行修改

    editor_bg_color         = getColor("us/editor_bg_color", QColor(0x00, 0x00, 0x00, 0x00));
    editor_font_size        = getInt("us/editor_font_size", 15);
    editor_font_color       = getColor("us/editor_font_color", QColor(0x00, 0x00, 0x00, 0xFF));
    editor_bg_selection     = getColor("us/editor_bg_selection", accent_color);
    editor_font_selection   = getColor("us/editor_font_selection", QColor(0xFF, 0xFF, 0xFF));
    editor_line_bg          = getBool("us/editor_line_bg", true);
    editor_line_bg_color    = getColor("us/editor_line_bg_color", QColor(0x88, 0x88, 0x88, 0x20));
    editor_cursor_width     = getInt("us/editor_cursor_width", 2);

#if !defined(Q_OS_MAC)
    integration_window      = getBool("us/integration_window", true);
#else
    integration_window      = getBool("us/integration_window", false);
#endif
    menu_bar_showed         = getBool("view/menu_bar_showed", false);
    status_bar_showed       = getBool("view/status_bar_showed", false);
    side_bar_showed         = getBool("view/side_bar_showed", true);
    win_btn_showed          = getBool("view/win_btn_showed", true);
    mainwin_radius          = getInt("us/mainwin_radius", 0);
    mainwin_border          = getInt("us/mainwin_border", 0);
    round_view              = getBool("us/round_view", true);
    widget_radius           = getInt("us/widget_radius", 10);
    mainwin_border_size     = getInt("us/mainwin_border_size", 5);

    dir_item_model          = round_view ? 1 : 2;
    editor_fuzzy_titlebar   = integration_window && getBool("us/editor_fuzzy_titlebar", false);
    split_shade             = getBool("us/split_shade", false);
    settings_in_mainwin     = getBool("us/settings_in_mainwin", true);

    quote_colorful          = getBool("us/quote_colorful", true);
    quote_colorful_color    = getColor("us/quote_colorful_color", accent_color);
    chapter_mark            = getBool("us/chapter_mark", true);
    chapter_mark_color      = getColor("us/chapter_mark_color", QColor( 0x88, 0x88, 0x88));
    editor_underline        = getBool("us/editor_underline", true);
    editor_underline_color  = getColor("us/editor_underline_color", QColor(accent_color.red(), accent_color.green(), accent_color.blue(), 30));
    editor_underline_middle = getBool("us/editor_underline_middle", true);
    editor_underline_offset = getInt("us/editor_underline_offset", 0);
    editor_line_spacing     = getInt("us/editor_line_spacing", 0);
    editor_water_effect     = getBool("us/editor_water_effect", false);
    editor_water_color      = getColor("us/editor_water_color", QColor(accent_color.red(), accent_color.green(), accent_color.blue(), 64));

    dynamic_bg_model        = (DynamicBgModel)getInt("us/dynamic_bg_model", DynamicBgModel::Dynamic_Gradient);
    emotion_filter_enabled  = getBool("us/emotion_filter_enabled",true);
    envir_picture_enabled   = getBool("us/environment_pictures_enabled", true);

    /* ---- 数据 ---- */
    save_power              = getBool("us/save_power", true);
    save_interval           = getInt("us/save_interval", 0);
    editor_recorder_max     = getInt("us/editor_recorder_max", 100);
    search_regex            = getBool("us/search_regexp", false);
    search_loop             = getBool("us/search_loop", false);
    sensitive_highlight     = false; // getBool("us/sensitive_highlight", false);
    error_word_highlight    = false;
    no_operator_mime_length = getInt("us/no_operator_mime_length", 3000);

    /* ---- 交互 ---- */
    one_click = getBool("us/one_click", true);
    tab_maxnum              = getInt("us/tab_maxnum", 20);
    open_chapter_animation  = getBool("us/open_chapter_animation", true);
    open_recent             = getBool("us/open_recent", true);
    scroll_cursor_fixed     = getBool("us/scroll_cursor_fixed", false);
    scroll_bottom_blank     = getInt("us/scroll_bottom_blank", 2);
    scroll_bottom_fixed     = getBool("us/scroll_bottom_fixed", true);
    cursor_animation        = getBool("us/cursor_animation", true);
    input_animation         = getBool("us/input_animation", true);
    board_count             = getInt("us/board_count", 100);
    clip_global             = getBool("us/clip_global", false);
    clip_limit              = getInt("us/clip_limit", 0);
    recycle_limit           = getInt("us/recycle_limit", 100);
    chapter_preview         = getBool("us/chapter_preview", true);
    smooth_scroll           = getBool("us/smooth_scroll", true);
    smooth_scroll_speed     = getInt("us/smooth_scroll_speed", 100);
    smooth_scroll_duration  = getInt("us/smooth_scroll_duration", 200);
    ban_microsoft_im        = getBool("us/ban_microsoft_im", true);
    sync_novels             = getBool("us/sync_novels", true);
    auto_sync               = getBool("us/auto_sync", true);
    auto_detect_update      = getBool("us/auto_detect_update", true);


    /* ---- 功能 ---- */
    indent_blank            = getInt("us/indent_blank", 2);
    indent_line             = getInt("us/indent_line", 1);

    smart_quotes            = getBool("us/smart_quotes", true);
    change_apos             = getBool("us/change_apos", true);
    smart_space             = getBool("us/smart_space", true);
    smart_enter             = getBool("us/smart_enter", true);
    smart_backspace         = getBool("us/smart_backspace", true);
    tab_complete            = getBool("us/tab_complete", true);
    tab_skip                = getBool("us/tab_skip", true);
    auto_punc               = getBool("us/auto_punc", true);
    auto_dqm                = getBool("us/auto_dqm", false);
    punc_cover              = getBool("us/punc_cover", true);
    space_quotes            = getBool("us/space_quotes", true);
    pair_match              = getBool("us/pair_match", true);
    pair_jump               = getBool("us/pair_jump", true);
#if defined (Q_OS_MAC)
    homonym_cover           = false; // MAC 无法进行转换，所以直接去掉此项
#else
    homonym_cover           = getBool("us/homonym_cover", true);
#endif
    smart_up_down           = getBool("us/smart_up_down", true);
    enter_force_add_para    = getBool("us/enter_force_add_para", false);
    climax_threshold        = getInt("us/climax_threshold", 9);
    climax_value            = 0;//getInt("runtime/climax_value", 0);

    para_after_quote        = getBool("us/para_after_quote", true);
    para_auto_split         = getBool("us/para_auto_split", false);
    para_max_length         = getInt("us/para_max_length", 300);
    typeset_split           = getBool("us/typeset_split", false);
    typeset_blank           = getBool("us/typeset_blank", true);
    typeset_paste           = getBool("us/typeset_paste", true);
    typeset_english         = getBool("us/typeset_english", false);

    /* ---- 名片 ---- */
    card_fill_tags          = getBool("us/card_fill_tags", true);
    card_fill_color         = getBool("us/card_fill_color", true);
    card_auto_scroll        = getBool("us/card_auto_scroll", true);
    card_save_scroll        = getBool("us/card_save_scroll", false);
    card_save_size          = getBool("us/card_save_size", true);
    card_follow_move        = getBool("us/card_follow_move", true);
    card_multi_tile         = getBool("us/card_multi_tile", false);


    /* ---- 词典 ---- */
    auto_suggestion         = getBool("us/auto_suggestion", true);
    lexicon_synonym         = getBool("us/lexicon_synonym", false);
    lexicon_related         = getBool("us/lexicon_related", false);
    lexicon_sentence        = getBool("us/lexicon_sentence", false);
    lexicon_random          = getBool("us/lexicon_random", false);
    lexicon_surname         = getBool("us/lexicon_surname", false);
    completer_input_delay   = getInt("us/completer_input_delay", 1);
    completer_cursor_delay  = getInt("us/completer_cursor_delay", 1000);
    completer_maxnum        = getInt("us/completer_maxnum", 7);
    completer_random        = getBool("us/completer_random", true);
    insert_sentence         = getBool("us/insert_sentence", false);
    auto_similar_sentence   = getBool("us/auto_similar_sentence", false);
    auto_predict_sentence   = getBool("us/auto_predict_sentence", false);

    /* ---- 自定义 ---- */


}

/**
 * 修改点缀色时，修改其他相同的颜色（保留透明度）
 * // TODO 有bug
 * @param color
 * @param key
 */
bool USettings::modifyToAccentColorIfIsAccent(QColor &color, QColor aim, QString key)
{
    if (color.red()!=accent_color.red() || color.green()!=accent_color.green() || color.blue()!=accent_color.blue())
        return false;
    int alpha = color.alpha();
    color = aim;
    color.setAlpha(alpha);
    setColor(key, color);
    return true;
}

void USettings::loadDynamicBackground()
{
    int model = getInt("dybg/model", 2);
    dynamic_bg_model = (DynamicBgModel)model;

    if (dynamic_bg)
        delete dynamic_bg;
    dynamic_bg = createDynamicBackground(dynamic_bg_model);

    emit signalDynamicBackgroundChanged(); // 第一次加载还没有连接信号槽，这个设置还没有什么用
}

DynamicBackgroundInterface* USettings::createDynamicBackground(DynamicBgModel model)
{
    if (model == DynamicBgModel::Dynamic_None)
        model = dynamic_bg_model;

    DynamicBackgroundInterface* bg = nullptr;

    if (model == DynamicBgModel::Dynamic_None) // 无
    {
        return nullptr;
    }
    else if (model == DynamicBgModel::Dynamic_Pure) // 纯色
    {
        bg = new DynamicBackgroundPure(nullptr,
                                               getColor("dybg/pure_color1", QColor(255, 250, 240)/*花卉白*/),
                                               getColor("dybg/pure_color2", QColor(253, 245, 230)/*舊蕾絲色*/));
    }
    else if (model == DynamicBgModel::Dynamic_Gradient) // 渐变
    {
        bg = new DynamicBackgroundGradient(nullptr,
                                                   getColor("dybg/gradient_color1", QColor(230, 230, 250)/*薰衣草紫*/),
                                                   getColor("dybg/gradient_color2", QColor(204, 204, 255)/*长春花色*/),
                                                   getColor("dybg/gradient_color3", QColor(240, 248, 255)/*爱丽丝蓝*/),
                                                   getColor("dybg/gradient_color4", QColor(248, 248, 253)/*幽灵白*/));
        bg->setHorizone(getBool("dybg/gradient_horizone", false));
        bg->showAni();
    }
    else if (model == DynamicBgModel::Dynamic_Island) // 小物件
    {
        return nullptr;
    }
    else // 其他类型的
    {
        return nullptr;
    }

    bg->setInterval(getInt("dybg/interval", 2000));
    return bg;
}

void USettings::addClimaxValue(bool add)
{
    if (dynamic_bg_model == DynamicBgModel::Dynamic_Gradient)
    {
        if (add)
            ++climax_value;
//            climax_value=20;
        else if (climax_value > - climax_threshold)
            --climax_value;
//            climax_value=-20;
        dynamic_bg->setAccumulate(climax_value);
//        setVal("runtime/climax_value", climax_value);
        dynamic_bg->redraw();
    }
}
