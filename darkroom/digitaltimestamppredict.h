#ifndef DIGITALTIMESTAMPPREDICT_H
#define DIGITALTIMESTAMPPREDICT_H

#include <QList>
#include <QDateTime>
#include <QDebug>
#include <QtMath>
#include <QSet>

class DigitalTimestampPredict
{
    struct DRData {
        int words;
        qint64 timestamp;

        int year;
        int month;
        int week;
        int date;
        int month_remain;

        double prob;
    };
public:
    static QList<int> getMaxes(QString full_string)
    {
        DigitalTimestampPredict dtp(full_string);
        return dtp.words;
    }

    DigitalTimestampPredict(QString full_string)
    {
        processHistoryString(full_string);
        calculateProbilities();
        getMax3Words();
    }

    /**
     * 输入一个小黑屋字数时，判断是否需要提示
     * 条件：总次数 >= 3，字数 > 之前最大字数*2
     */
    static bool shouldAsk(QString full_string, int input)
    {
        int max_word = 0;

        QStringList list = full_string.split("\n", QString::SkipEmptyParts);
        if (list.size() < 3)
            return false;
        foreach (QString line, list)
        {
            // 从历史记录中获取字数和时间戳
            QStringList li = line.split(",", QString::SkipEmptyParts);
            if (li.size() != 5)
                continue;
            bool ok;
            int words = li.first().toInt(&ok);
            if (!ok)
                continue;

            // 进行判断
            if (words*2 >= input)
                return false;
        }
        return true;
    }

protected:
    /**
     * 根据各个属性来计算相似度
     */
    void calculateProbilities()
    {
        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        const qint64 everyday_delta = 24*60*60*1000;
        const qint64 inday_level = 15*60*1000;
        const qint64 everyweek_delta = 7*everyday_delta;
        const QDate cur_date = QDate::currentDate();
        const int cur_week = cur_date.dayOfWeek();
        const int cur_day = cur_date.day();
        const int cur_year = cur_date.year();
        const int cur_month = cur_date.month();
        const int cur_remain = cur_date.daysInMonth() - cur_day;

        // foreach (DRData data, datas)
        for (int i = 0; i < datas.size(); i++)
        {
            DRData& data = datas[i];
            qint64 delta_timestamp = timestamp - data.timestamp;
            double& prob = data.prob;

            double prob_day = 0;
            double prob_hour = 0;
            int delta_day = 0;
            int delta_hour = 0;
            // 日期上的差
            {
                // 按天数差统计
                delta_day = delta_timestamp / everyday_delta;

                if (delta_day <= 0)
                    delta_day = 1;

                // 时序概率加成
                if (delta_day > 3)
                {
                    // 按每周几统计
                    if (cur_week == data.week) // 同周几
                    {
                        int delta_week = delta_timestamp / everyweek_delta;
                        delta_day = delta_week * 4; // 天数差至少不会比周数差*4还小吧？
                    }

                    // 按月初统计
                    int delta_day_of_month = qAbs(data.date - cur_day);
                    if (delta_day_of_month <= 3) // 同几号
                    {
                        int delta_month = (cur_year*12+cur_month) - (data.year*12+data.month); // 相差的月数
                        delta_day = qMin(delta_day, delta_month * delta_day_of_month * 3);
                    }

                    // 按月末统计
                    int delta_remain_of_month = qAbs(data.date - cur_remain);
                    if (delta_remain_of_month <= 3) // 同几号
                    {
                        int delta_month = (cur_year*12+cur_month) - (data.year*12+data.month); // 相差的月数
                        delta_day = qMin(delta_day, delta_month * delta_remain_of_month * 2);
                    }
                }
                prob_day = log(1.0 / (delta_day+0.1)+1);
            }

            // 钟表上的差
            {
                qint64 delta = delta_timestamp % everyday_delta; // 一天以内的时间戳差
                delta = qAbs(qMin(delta, everyday_delta - delta));
                delta_hour = delta / inday_level + 1; // 和当前时间相差多少刻
                prob_hour = log(1.0 / (delta_hour)+1);
            }
            prob += prob_day * prob_hour;
            data.prob = prob;
//            qDebug() << "概率：" << datas.at(i).words << prob << QDateTime::fromMSecsSinceEpoch(data.timestamp).toString("MM-dd HH:mm")
//                     << "  详细：" << prob_day << " * " << prob_hour << " 刻数:" << delta_hour;
        }
    }

    /**
     * 获取前三个合适的字体大小
     * 如果没有合适的，则不进行提示
     */
    void getMax3Words()
    {
        // 先获取最大的概率
        double max_prob = 0;
        for (int i = 0; i < datas.size(); i++)
        {
            if (datas.at(i).prob > max_prob)
                max_prob = datas.at(i).prob;
        }
        if (max_prob <= 0.0001)
            return ;

        // 因为set不好用结构体，所以先用 hash 合并，转成 list 排序
        // 将重复的字数合并，概率增加
        QHash<int, double>hash;
        for (int i = 0; i < datas.size(); i++)
        {
            hash[datas.at(i).words] += datas.at(i).prob;
        }
//        qDebug() << "hash" << hash;

        struct IntDouble {
            int i;
            int ali; // 用来4字节对齐的（莫名wanring，难道int不就是4的倍数？）
            double d;
        };
        QList<IntDouble> list;
        for (auto it = hash.begin(); it != hash.end(); ++it)
        {
            list.append(IntDouble{it.key(), 0, it.value()});
        }
        std::sort(list.begin(), list.end(), [=](const IntDouble a, const IntDouble b){ return a.d > b.d || a.i < b.i; });

        words.clear();
        foreach (IntDouble id, list)
        {
            words.append(id.i);
        }
    }

private:

    /**
     * 处理从文件中读取的字符串
     * @param full_string 文本格式历史记录
     */
    void processHistoryString(const QString& full_string)
    {
        // 格式：字数,时间,运算,强制,时间戳
        QStringList list = full_string.split("\n", QString::SkipEmptyParts);
        foreach (QString line, list) {
            // 从历史记录中获取字数和时间戳
            QStringList li = line.split(",", QString::SkipEmptyParts);
            if (li.size() != 5) continue;
            bool ok;
            int words = li.first().toInt(&ok);
            if (!ok) continue;
            qint64 timestamp = li.last().toLongLong(&ok);
            if (!ok) continue;

            // 将时间戳转换成日期
            addOneData(words, timestamp);
        }
    }

    /**
     * 根据时间戳来确定具体日期，并添加一条记录数据
     * @param words     字数
     * @param timestamp 时间戳，用来生成具体日期
     */
    void addOneData(int words, qint64 timestamp)
    {
        // 获取星期、日期、距离月末
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
        QDate da = dt.date();
        int year = da.year();
        int month = da.month();
        int week = da.dayOfWeek();
        int day = da.day();
        int month_remain = da.daysInMonth() - day; // 这个月剩下几天

        if (day < 3) month_remain = -day; // 负数的月末，同样参与统计
        if (month_remain > 3) month_remain = 0; // 月末3天前，归零

        datas.append(DRData{words, timestamp, year, month, week, day, month_remain, 0});
    }

    /**
     * 获取一个月份剩下几天（已被 QDate 内置函数代替）
     * @param  year  年份（计算闰年、平年）
     * @param  month 月份
     * @return       剩下几天
     */
    int getMonthRemain(int year, int month, int day)
    {
        static int days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        // 判断是不是闰年
        if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
            days[2] = 29;
        else
            days[2] = 28;

        return days[month] - day;
    }

private:
    QList<DRData>datas;
    QList<int> words;
};

#endif // DIGITALTIMESTAMPPREDICT_H
