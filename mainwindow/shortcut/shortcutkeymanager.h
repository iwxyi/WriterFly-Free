#ifndef SHORTCUTKEYMANAGER_H
#define SHORTCUTKEYMANAGER_H

#include <QObject>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "shortcutentry.h"
#include "fileutil.h"
#include "stringutil.h"

class ShortcutKeyManager
{
    friend class ShortcutWindow;
public:
    explicit ShortcutKeyManager(QString file_path);

    QStringList getShortutCmds(ShortcutEnvironment env, Qt::KeyboardModifiers modifiers, Qt::Key key);
    void saveToFile();

private:
    void initFromFile();
    void initKeyMap();

private:
    QMap<QString, Qt::Key> KEY_MAP;
    QList<ShortcutEntry>shortcuts;

    QString file_path;
    QString extra_dir;
};

#endif // SHORTCUTKEYMANAGER_H
