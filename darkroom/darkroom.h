#ifndef DARKROOM_H
#define DARKROOM_H

#include <QObject>
#include "QMessageBox"
#include "useraccount.h"
#include "escapedialog.h"

class DarkRoom : public QObject
{
    Q_OBJECT
public:
    DarkRoom(QString data_dir, UserAccount* ac = nullptr);
    friend class DarkRoomWidget;

    enum ForceType {
        ForceType_None,
        ForceType_Warning,
        ForceType_Force
    };

    enum ComboType {
        ComboType_None,
        ComboType_Word,
        ComboType_Time,
        ComboType_Either,
        ComboType_Both
    };

    bool isInDarkRoom();
    void startDarkRoom(int delta_word, int delta_time, ComboType combo, ForceType force);
    bool exitDarkRoom(QWidget* w = nullptr);
    void restoreDarkRoom(QWidget* w);
    void finishDarkRoomForce();
    QString getRestString();
    QString getStateString();

    QString getHistories();

private:
    bool warningExit(QWidget* w = nullptr);

    void setAutoStart(bool bAutoStart);

signals:
    void signalOpenDarkRoom();
    void signalCloseDarkRoom();

private:
    QString file_path;
    UserAccount* ac;

    ComboType ct;
    ForceType ft;
    qint64 start_timstamp; // 开始时间

    int delta_word; // 目标字数
    int finish_word; // 最终字数

    int delta_time; // 目标分钟数
    int finish_time; // 最终时间
};

#endif // DARKROOM_H
