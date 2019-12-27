#ifndef EMOTIONFILTER_H
#define EMOTIONFILTER_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <math.h>
#include "fileutil.h"
#include "stringutil.h"

#define EMOTION_COLOR_DEFAULT_ALPHA 32 // 默认的透明度
#define EMOTION_MAX_VALUE 8 // 最多只取8次出现，低于此数值时，按比例来，最高不超过此
#define EMOTION_MAX_QUEUE 12 // 所有颜色数值加起来的和
#define EMOTION_COLOR_CLOSE_INTERVAL 100 // 色彩变化间隔
#define EMOTION_ELIMINATE_INTERVAL 180000 // 最多半分钟消除一个，最快10秒钟

class EmotionFilter : public QObject
{
    Q_OBJECT
public:
    explicit EmotionFilter(QString path, QString extra_dir, QWidget *parent);

    struct DictColor {
        DictColor() : value( 0 ) {}
        QString name;
        int value;
        QColor color;
        QStringList dicts;
    };

    void input(QString sent);

    void draw(QPainter& painter);

private:
    void initDictColorsFromFile(QString path, QString extra_dir);
    void refreshColor();
    void startColorAnimation();

signals:
    void signalRedraw();

public slots:
    void closeToColor();
    void eliminate();

private:
    QWidget* widget;

    int max_sum;            // 队列长度上限
    QList<DictColor>list;   // 情绪滤镜的信息
    QList<int>queue;        // 情绪滤镜的队列，每一个的索引

    QColor cur_color;
    QColor aim_color;
    QTimer* color_timer;

    QTimer* eliminate_timer;
};

#endif // EMOTIONFILTER_H
