#ifndef SHORTCUTENTRY_H
#define SHORTCUTENTRY_H

#include <QStringList>

#define NO_KEY Qt::Key_Meta // 不知道什么修饰键，随便来个点不到的

/**
 * 快捷键所在的环境
 */
enum ShortcutEnvironment{
    SCK_GLOBAL,
	SCK_EDITOR,
	SCK_DIR,
	SCK_OUTLINE,
    SCK_EDITORS
};

class ShortcutEntry
{
public:
    Qt::KeyboardModifiers modifier;
    QString keys;
    Qt::Key key;
    QStringList cmds;
    QString left, right;
    ShortcutEnvironment env;

    QString full_key;
};

#endif // SHORTCUTENTRY_H
