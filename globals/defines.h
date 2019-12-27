/**
 * 全局宏定义
 */

#ifndef DEFINES_H
#define DEFINES_H

#include <QString>


// ====标题栏 ====
#define BUTTON_HEIGHT 30 // 按钮高度
#define BUTTON_WIDTH 30  // 按钮宽度
#define TITLE_HEIGHT 30  // 标题栏高度

// ==== 布局 ====
#define CUTTING_SPACE 0  // 切掉的布局控件间宽度

// ==== 目录 Role ====
enum DRole {
    DRole_CHP_NAME = 0,
    DRole_CHP_NUM = 1,
    DRole_CHP_NUM_CN = 2,
    DRole_CHP_DETAIL = 3,
    DRole_CHP_ISROLL = 10,
    DRole_CHP_ISHIDE = 4,
    DRole_CHP_STEMP = 50,
    DRole_CHP_OPENING = 100,
    DRole_ROLL_SUBS = 11,
    DRole_ROLL_COUNT = 12,
    DRole_CHP_FULLNAME = 13,
    DRole_RC_ANIMATING = 14,
    DRole_RC_SEARCH = 15,
    DRole_CR_COUNT = 16,
    DRole_ROLL_NAMES = 17,
    DRole_CHP_ROLLINDEX = 18,
    DRole_CHP_CHPTINDEX = 19,
    DRole_RC_SELECTING = 20,
    DRole_RC_CLICKPOINT,
    DRole_RC_SELECT_PROG100,
    DRole_RC_UNSELECT_PROG100,
    DRole_RC_HOVERPROG,
    DRole_RC_PREV_HOVERPROG,
    DRole_RC_NEXT_HOVERPROG,
    DRole_RC_WORD_COUNT,
};

#define SHOW_ROLE Qt::UserRole+26
#define TITLE_ROLE Qt::UserRole+58
#define SIZE_ROLE Qt::UserRole + 84

#define DIR_DEVIATE 14      // 目录左边的距离
#define DIR_DEVIATE_ADD 20  // 章节比分卷靠后的距离

// ==== 文件 ====
#define NOVELDIRFILENAME "dir.txt"
#define OUTLINE_DIR_FILENAME "outline_dir.txt"


// ==== 编辑 ====
enum EditType {
    EDIT_TYPE_NONE,   // 没有编辑
    EDIT_TYPE_CHAPTER, // 章节
    EDIT_TYPE_OUTLINE, // 大纲
    EDIT_TYPE_FINE_OL, // 细纲
    EDIT_TYPE_BRIEF,   // 简介
    EDIT_TYPE_DETAIL,  // 细节
    EDIT_TYPE_UNKNOW = 9,  // 未知
};


// ==== 方向 ====
enum DirectType {
    DIRECT_NONE,
    DIRECT_TOP,
    DIRECT_LEFT,
    DIRECT_RIGHT,
    DIRECT_BOTTOM
};


// ==== 控件 ====
#define TRANSPARENT_SHOW_KIND_NOVEL_INFO 3


// ==== 引导 ====
enum LINE_GUIDE {
    LINE_GUIDE_NONE,
    LINE_GUIDE_CREATE_BOOK
};


#endif // DEFINES_H
