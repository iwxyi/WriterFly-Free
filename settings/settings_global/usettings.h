#ifndef USETTINGS_H
#define USETTINGS_H

#include "settings.h"
#include <QColor>
#include "homonymcoverdata.h"
#include "boardmanager.h"
#include "dynamicbackgroundpure.h"
#include "dynamicbackgroundgradient.h"
#include "emotionfilter.h"
#include "environmentpictures.h"

enum DynamicBgModel {
    Dynamic_None,      // 无微动背景
    Dynamic_Pure,      // 纯色背景
    Dynamic_Gradient,  // 渐变背景
    Dynamic_Island     // 小物件
};

/**
 * 程序总设置类
 */
class USettings : public Settings
{
    Q_OBJECT
public:

    USettings(QString filePath);

    void init();
    void loadFromFile();

    bool modifyToAccentColorIfIsAccent(QColor& color, QColor aim, QString key);
    void loadDynamicBackground();
    DynamicBackgroundInterface* createDynamicBackground(DynamicBgModel model = DynamicBgModel::Dynamic_None);
    void addClimaxValue(bool add);

public:
    /* ==== 全局 ==== */
    bool is_debug;                   // 调试模式
    bool deep_debug;                 // 深度调试模式（记录每一个步骤，可能会引起卡顿）

    QColor predominant_color;        // 主色调：背景颜色
    QColor auxiliary_color;          // 辅助色：各类面板颜色
    QColor decoration_color;         // 点缀色
    QColor foreground_color;         // 文字色

    /* ==== 界面 ==== */
    int     widget_size;             // 标题栏高度
    int     mainwin_border_size;     // 边框宽度（尺寸）
    QString mainwin_bg_picture;      // 背景图片，如果是文件夹则随机
    QColor  mainwin_bg_color;        // 窗口背景颜色，用于在图片背后，-1为无
    int     blur_picture_radius;     // 背景图片模糊半径

    bool    mainwin_sidebar_hide;    // 侧边栏是否折叠
    int     mainwin_sidebar_width;   // 侧边栏的固定宽度，为0则自适应
    QColor  mainwin_sidebar_color;   // 侧边栏的颜色，为了和背景颜色适配，要调整透明度
    int     mainwin_sidebar_spacing; // 目录章节间的间隔
    int     dir_item_bg_model;       // 目录背景动画
    int     dir_item_text_model;     // 目录文字动画
    int     dir_item_open_model;     // 目录打开动画

    QColor  global_font_color;       // 全局字体颜色（除了编辑框外的所有字体颜色）
    QColor  accent_color;            // 强调色

    QColor  editor_bg_color;         // 编辑框的背景颜色
    int     editor_font_size;        // 编辑框的字体大小
    QColor  editor_font_color;       // 编辑框的字体颜色
    QColor  editor_bg_selection;     // 选中的背景颜色
    QColor  editor_font_selection;   // 选中的字体颜色
    bool    editor_line_bg;          // 当前行高亮
    QColor  editor_line_bg_color;    // 光标所在行背景颜色
    int     editor_cursor_width;     // 光标宽度
    int     editor_line_spacing;     // 行间距
    bool    editor_underline;        // 下划线
    QColor  editor_underline_color;  // 下划线颜色
    bool    editor_underline_middle; // 下划线间居中
    int     editor_underline_offset; // 下划线向下偏移
    bool    editor_water_effect;     // 编辑框点击动画
    QColor  editor_water_color;      // 编辑框点击动画颜色

    bool    integration_window;      // 一体化界面。替代原生窗口边框、浮动窗口
    bool    menu_bar_showed;         // 菜单栏
    bool    status_bar_showed;       // 状态栏
    bool    side_bar_showed;         // 侧边栏
    bool    win_btn_showed;          // 窗口按钮
    int     mainwin_radius;          // 主窗口圆角程度
    int     mainwin_border;          // 主窗口调整大小的阴影边界
    bool    round_view;              // 控件圆角效果开关
    int     widget_radius;           // 矩形控件圆角（弹窗主体、名片卡片）
    int     dir_item_model;          // 目录样式：0自动，1方形，2椭圆，3半椭圆，4边框，5箭头
    bool    editor_fuzzy_titlebar;   // 编辑框模糊到标题栏
    bool    split_shade;             // 分割线阴影效果
    bool    settings_in_mainwin;     // 在主窗口打开设置（背景模糊效果）

    DynamicBgModel dynamic_bg_model; // 微动背景
    bool    emotion_filter_enabled;  // 情景滤镜
    bool    envir_picture_enabled;   // 情境图片

    /* ==== 交互 ==== */
    bool    one_click;               // 单击操作：单击打开章节分组+双击重命名、双击打开章节分组+菜单重命名
    int     tab_maxnum;              // 打开的 tab widget 个数
    bool    open_chapter_animation;  // 打开章节的动画
    bool    open_recent;             // 启动时自动打开最后一次的章节
    bool    scroll_cursor_fixed;     // 全文光标固定
    int     scroll_bottom_blank;     // 底部空白（滚动超过最后一行）
    bool    scroll_bottom_fixed;     // 末尾固定位置
    bool    scroll_suspension;       // 悬浮滚动
    bool    cursor_animation;        // 光标移动动画
    bool    input_animation;         // 输入文字动画
    int     board_count;             // 三板的最大数量
    bool    clip_global;             // 监听全局剪贴板
    int     clip_limit;              // 剪贴板的最少字数
    int     recycle_limit;           // 回收版的最少字数
    bool    chapter_preview;         // 目录鼠标悬浮预览
    bool    smooth_scroll;           // 平滑滚动
    int     smooth_scroll_speed;     // 平滑滚动速度
    int     smooth_scroll_duration;  // 平滑滚动周期
    bool    ban_microsoft_im;        // 禁止微软拼音字母
    bool    sync_novels;             // 是否开启云同步工具
    bool    auto_sync;               // 是否开启自动云同步
    bool    auto_detect_update;      // 是否开启检测更新


    /* ==== 数据 ==== */
    bool    save_power;              // 强力保存：每写一个字就进行保存
    int     save_interval;           // 自动保存的时间间隔(单位分钟)，仅在 powerSave 关闭时有效
    int     editor_recorder_max;     // 撤销的最大次次数
    bool    quote_colorful;          // 引号内彩色
    QColor  quote_colorful_color;    // 引号内彩色颜色
    bool    chapter_mark;            // 章内标注
    QColor  chapter_mark_color;      // 章内标注颜色
    bool    search_regex;            // 使用正则表达式替换
    bool    search_loop;             // 循环搜索
    bool    error_word_highlight;    // 错词高亮
    int     no_operator_mime_length; // 粘贴/插入时超过这个字数就不进行自动化操作

    /* ==== 功能 ==== */
    int     indent_blank;            // 段首空格
    int     indent_line;             // 段落空行
    bool    smart_quotes;            // 双引号变成智能引号
    bool    change_apos;             // 单击引号实现智能引号效果(针对双引号按键修复)
    bool    smart_space;             // 智能空格
    bool    smart_enter;             // 智能回车
    bool    smart_backspace;         // 智能删除
    bool    tab_complete;            // tab完成
    bool    tab_skip;                // tab跳过
    bool    auto_punc;               // 自动标点
    bool    auto_dqm;                // 自动双引号
    bool    punc_cover;              // 标点覆盖
    bool    space_quotes;            // 空格引号
    bool    pair_match;              // 括号匹配
    bool    pair_jump;               // 括号跳转
    bool    homonym_cover;           // 同音词覆盖
    HomonymCoverData homonym_cover_data; // 同音词覆盖的数据
    bool    smart_up_down;           // 上下箭头进行段落跳转或者
    bool    enter_force_add_para;    // 回车键强制另起一段
    int     climax_threshold;        // 高潮模式阈值
    int     climax_value;            // 当前高潮程度

    bool    para_after_quote;        // 多段后引号
    bool    para_auto_split;         // 回车自动分段
    int     para_max_length;         // 自动分段的上限
    bool    typeset_split;           // 排版时长段落自动分段
    bool    typeset_blank;           // 中文和字母数字间增加空格
    bool    typeset_paste;           // 多段粘贴排版
    bool    typeset_english;         // 排版时首字母大写、标点后空格

    bool    sensitive_highlight;     // 敏感词高亮

    /* ==== 名片 ==== */
    bool    card_fill_tags;          // 自动填充新名片标签
    bool    card_fill_color;         // 自动填充新名片颜色
    bool    card_auto_scroll;        // 自动滚动至内容
    bool    card_save_scroll;        // 保存滚动历史
    bool    card_save_size;          // 保存窗口大小
    bool    card_follow_move;        // 跟随主窗口移动
    bool    card_multi_tile;         // 多张名片平铺

    /* ==== 词典 ==== */
    bool    auto_suggestion;         // 自动提示总开关
    bool    lexicon_synonym;         // 同义词词典
    bool    lexicon_related;         // 相关词词典
    bool    lexicon_sentence;        // 句子词典
    bool    lexicon_random;          // 随机取名
    int     completer_input_delay;   // 输入后提示延时
    int     completer_cursor_delay;  // 移动后提示延时
    int     completer_maxnum;        // 自动提示最大显示项数
    bool    completer_random;        // 词典乱序显示
    bool    lexicon_surname;         // 姓氏提示名字
    bool    insert_sentence;         // 句子是否直接插入而不复制
    bool    auto_similar_sentence;   // 是否自动提示相似句子
    bool    auto_predict_sentence;   // 是否自动预测下一条句子

    /* ==== 自定义 ==== */
    DynamicBackgroundInterface* dynamic_bg;
    EmotionFilter* emotion_filter;
    EnvironmentPictures* envir_picture;

    /* ==== 常量 ==== */

signals:
    void signalDynamicBackgroundChanged();
    void signalEmotionFilterChanged();
    void signalEnvironmentPicturesChanged();

    void signalTitlebarHeightChanged(int x);
    void signalBlurTitlebarChanged(bool b);
};


#endif // USETTINGS_H
