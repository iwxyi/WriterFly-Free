#ifndef ENVIRONMENTPICTURES_H
#define ENVIRONMENTPICTURES_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QDateTime>
#include <math.h>
#include "fileutil.h"
#include "stringutil.h"

#define ENVIRONMENT_OPACITY_DEFAULT 32
#define ENVIRONMENT_APPEAR_DEFAULT 2
#define ENVIRONMENT_MAX_VALUE 8
#define ENVIRONMENT_MAX_QUEUE 12
#define ENVIRONMENT_OPACITY_CLOSE_INTERVAL 100
#define ENVIRONMENT_ELIMINATE_INTERVAL 180000
#define ENVIRONMENT_MIN_SHOW_TIME 30000 // 最短显示时间

class EnvironmentPictures : public QObject
{
   Q_OBJECT
public:
    EnvironmentPictures(QString path, QString dir, QString extra_dir, QWidget *parent);

    struct DictPixmap {
        DictPixmap() : value(0) {}
        QString name;
        int appear;
        int value;
        int opacity;
        QStringList dicts;
    };

    void input(QString sent);

    void draw(QPainter& painter);

private:
    void initDictPixmapsFromFile(QString path, QString extra_dir);
    void selectMaxPixmap();
    void startShowPixmap(QString name);

    qint64 getTimestamp();

signals:
    void signalRedraw();

public slots:
    void modifyOpacity();
    void eliminate();

private:
    QWidget* widget;
    QString pictures_dir; // 数据文件夹路径

    QList<DictPixmap> list;
    QList<int> queue;

    bool use_pixmap;
    int current_index;
    qint64 start_use_timetamp;
    QPixmap pixmap;
    int cur_opacity, aim_opacity;
    QTimer* opacity_timer;

    QPixmap buffer_pixmap;
    int buffer_opacity;

    QTimer* eliminate_timer;
};

#endif // ENVIRONMENTPICTURES_H
