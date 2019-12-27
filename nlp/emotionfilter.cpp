#include "emotionfilter.h"

EmotionFilter::EmotionFilter(QString path, QString extra_dir, QWidget *parent) : QObject(parent), widget(parent)
{
    initDictColorsFromFile(path, extra_dir);

    cur_color = aim_color = QColor(128,128,128,0);

    color_timer = new QTimer(this);
    color_timer->setInterval(EMOTION_COLOR_CLOSE_INTERVAL);
    connect(color_timer, SIGNAL(timeout()), this, SLOT(closeToColor()));

    eliminate_timer = new QTimer(this);
    connect(eliminate_timer, SIGNAL(timeout()), this, SLOT(eliminate()));
}

void EmotionFilter::initDictColorsFromFile(QString path, QString extra_dir)
{
	QString content = readTextFileWithFolder(path, extra_dir);
	QStringList items = getXmls(content, "FILTER");
	foreach (QString item, items)
	{
		DictColor dc;
		dc.name = getXml(item, "NAME").trimmed();

		QStringList rgba = getXml(item, "COLOR").trimmed().split(",", QString::SkipEmptyParts);
		if (rgba.size() >= 3) // 只有3
		{
			dc.color.setRed((rgba.at(0)).toInt());
			dc.color.setGreen((rgba.at(1)).toInt());
			dc.color.setBlue((rgba.at(2)).toInt());
			if (rgba.size() >= 4)
				dc.color.setAlpha((rgba.at(3)).toInt());
			else
				dc.color.setAlpha(EMOTION_COLOR_DEFAULT_ALPHA);
		}

		dc.dicts = getXml(item, "DICT").trimmed().split(" ");

        list << dc;
	}
}

void EmotionFilter::input(QString sent)
{
	bool changed = false;
	for (int i = 0; i < list.size(); i++)
	{
		DictColor& dc = list[i];
        QStringList& dicts = list[i].dicts;
        for (int j = 0; j < dicts.size(); j++)
		{
            if ( sent.indexOf(dicts.at(j)) > -1 ) // 找到这个词语
			{
				queue << i;
				dc.value++;
				changed = true;
			}
		}
	}

	while (queue.size() > EMOTION_MAX_QUEUE)
	{
        int index = queue.first();
        queue.removeFirst();
        list[index].value--;
	}

	if (changed)
    {
        refreshColor();

        if (!eliminate_timer->isActive() && queue.size() > 0)
        {
            int sq = sqrt(queue.size());
            int interval = EMOTION_ELIMINATE_INTERVAL / (sq+2); // 数量越多，消除越快
            eliminate_timer->setInterval(interval);
            eliminate_timer->start();
        }
    }
}

void EmotionFilter::draw(QPainter &painter)
{
	if (!widget) return;

    painter.save();
	{
        painter.setBrush(cur_color);
        painter.drawRect(QRect( -1, -1, widget->size().width()+2, widget->size().height()+2));
	}
	painter.restore();
}

void EmotionFilter::refreshColor()
{
	// 统计总和及其分布
    int sum = 0;
	QList<int>values;
	for (int i = 0; i < list.size(); i++)
	{
        int value = list.at(i).value;
		if (value == 0)
		{
			values << 0;
			continue;
		}
		if (value <= EMOTION_MAX_VALUE)
		{
			values << value;
            sum += value;
		}
		else
		{
			values << EMOTION_MAX_VALUE;
            sum += EMOTION_MAX_VALUE;
        }
	}

    if (sum <= 0)
	{
		aim_color = QColor(128, 128, 128, 0);
	}
	else
	{
        int alpha_sum = sum;
        if (alpha_sum < 10) // 一开始不能变化太大
            alpha_sum = EMOTION_MAX_VALUE;

		int red = 0, green = 0, blue = 0, alpha = 0;
		// 按比例分配
		for (int i = 0; i < list.size(); i++)
		{
            if (values.at(i) == 0) continue;

            double prop = static_cast<double>(values.at(i)) / sum;
            double alpha_prop = static_cast<double>(values.at(i)) / alpha_sum;
			QColor color = list.at(i).color;

			red += color.red() * prop;
			green += color.green() * prop;
			blue += color.blue() * prop;
            alpha += color.alpha() * alpha_prop;

//            qDebug() << list[i].name << values.at(i) << prop << alpha_prop;
		}

		aim_color = QColor(red, green, blue, alpha);

	}

    if (!color_timer->isActive())
        color_timer->start();
//    qDebug() << "aim_color:" << aim_color;
}


void EmotionFilter::closeToColor()
{
    if (cur_color != aim_color)
    {
        if (cur_color.alpha() == 0)
        {
            cur_color.setRed(aim_color.red());
            cur_color.setGreen(aim_color.green());
            cur_color.setBlue(aim_color.blue());
        }

        if (cur_color.red() < aim_color.red())
            cur_color.setRed(cur_color.red()+1);
        else if (cur_color.red() > aim_color.red())
        	cur_color.setRed(cur_color.red()-1);
        if (cur_color.green() < aim_color.green())
            cur_color.setGreen(cur_color.green()+1);
        else if (cur_color.green() > aim_color.green())
        	cur_color.setGreen(cur_color.green()-1);
        if (cur_color.blue() < aim_color.blue())
            cur_color.setBlue(cur_color.blue()+1);
        else if (cur_color.blue() > aim_color.blue())
        	cur_color.setBlue(cur_color.blue()-1);
        if (cur_color.alpha() < aim_color.alpha())
            cur_color.setAlpha(cur_color.alpha()+1);
        else if (cur_color.alpha() > aim_color.blue())
            cur_color.setAlpha(cur_color.alpha()-1);
    }
    else
    {
        color_timer->stop();
    }
//    qDebug() << cur_color << aim_color;

    emit signalRedraw();
}

void EmotionFilter::eliminate()
{
    // 消除第一个
    if (queue.size() > 0)
    {
        int index = queue.first();
        list[index].value--;
        queue.removeFirst();

        int sq = sqrt(queue.size());
        int interval = EMOTION_ELIMINATE_INTERVAL / (sq+2); // 数量越多，消除越快
        eliminate_timer->setInterval(interval);

        refreshColor();
    }
    else
    {
        eliminate_timer->stop();
    }
}
