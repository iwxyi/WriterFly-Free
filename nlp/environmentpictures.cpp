#include "environmentpictures.h"

EnvironmentPictures::EnvironmentPictures(QString path, QString dir, QString extra_dir, QWidget* parent) : QObject(parent),
		widget(parent), pictures_dir(dir),
        use_pixmap(false), current_index(-1), start_use_timetamp(0), cur_opacity(0), aim_opacity(0),
        buffer_opacity(-1)
{
    initDictPixmapsFromFile(path, extra_dir);

	opacity_timer = new QTimer(this);
	opacity_timer->setInterval(ENVIRONMENT_OPACITY_CLOSE_INTERVAL);
	connect(opacity_timer, SIGNAL(timeout()), this, SLOT(modifyOpacity()));

    eliminate_timer = new QTimer(this);
	connect(eliminate_timer, SIGNAL(timeout()), this, SLOT(eliminate()));
}

void EnvironmentPictures::initDictPixmapsFromFile(QString path, QString extra_dir)
{
    QString content = readTextFileWithFolder(path, extra_dir);
    QStringList items = getXmls(content, "PICTURE");
    foreach (QString item, items)
    {
    	DictPixmap dp;
    	dp.name = getXml(item, "NAME").trimmed();
    	dp.dicts = getXml(item, "DICT").trimmed().split(" ");
    	dp.opacity = getXmlInt(item, "OPACITY");
    	if (dp.opacity <= 0) dp.opacity = ENVIRONMENT_OPACITY_DEFAULT;
        dp.appear = getXmlInt(item, "APPEAR");
        if (dp.appear <= 0) dp.appear = ENVIRONMENT_APPEAR_DEFAULT;
    	list << dp;
//        qDebug() << dp.name << dp.dicts;
    }
}

void EnvironmentPictures::input(QString sent)
{
	bool changed = false;
	for (int i = 0; i < list.size(); i++)
	{
		DictPixmap& dp = list[i];
		QStringList& dicts = list[i].dicts;
		for (int j = 0; j < dicts.size(); j++)
		{
			if ( sent.indexOf(dicts.at(j)) > -1 )
			{
				queue << i;
				dp.value++;
				changed = true;
			}
		}
	}

	while (queue.size() > ENVIRONMENT_MAX_QUEUE)
	{
		int index = queue.first();
		queue.removeFirst();
		list[index].value--;
	}

	if (changed)
	{
		selectMaxPixmap();

        if (!eliminate_timer->isActive() && queue.size() > 0)
        {
            int sq = static_cast<int>(sqrt(queue.size()));
            int interval = ENVIRONMENT_ELIMINATE_INTERVAL / (sq+2);
            eliminate_timer->setInterval(interval);
            eliminate_timer->start();
        }
	}
}

void EnvironmentPictures::selectMaxPixmap()
{
    qint64 timestamp = getTimestamp();

	// 寻找最大的
	int max_value = 0, max_index = -1;
	for (int i = 0; i < list.size(); i++)
	{
		int value = list.at(i).value;
        if (value && i == current_index) // 当前图片的值
            value++; // 当前的图片再加一，坚挺一些
        if (value > max_value && value >= list.at(i).appear) // 超过最大值且超过显示的值
		{
			max_value = value;
			max_index = i;
		}
	}

	// 和原来的进行比较
    if (max_value > 1 && max_index > -1) // 至少出现2次，或者当前图片至少出现1次
	{
//        qDebug() << "选择：" << list[max_index].name;
		if (max_value > ENVIRONMENT_MAX_VALUE)
			max_value = ENVIRONMENT_MAX_VALUE;

        if (max_index != current_index && cur_opacity==0) // 当前图片已经结束了，并且使用其他图片
		{
            startShowPixmap(list[max_index].name);
            current_index = max_index;
            aim_opacity = list[max_index].opacity * max_value / ENVIRONMENT_MAX_VALUE;
//            qDebug() << "新图片：" << aim_opacity;
		}
        else if (max_index == current_index) // 依旧是这一张图片
        {
            aim_opacity = list[max_index].opacity * max_value / ENVIRONMENT_MAX_VALUE;
//            qDebug() << "opacity:" << aim_opacity;
        }
        else // if (max_index != current_index) // 更换图片
        {
            if (timestamp - start_use_timetamp < ENVIRONMENT_MIN_SHOW_TIME) // 当前图片没有结束，等待结束
                ;//qDebug() << "等待结束";
            else
            {
//                qDebug() << "开始结束";
                aim_opacity = 0;
            }
        }
	}
	else // 没有最大的，全部恢复初始化
	{
        aim_opacity = 0;
	}

	if (cur_opacity == 0 && aim_opacity == 0)
		;
	else if (!opacity_timer->isActive())
		opacity_timer->start();
}

void EnvironmentPictures::startShowPixmap(QString name)
{
    QFileInfo info(pictures_dir + name);
    if (info.exists() && info.isDir())
    {
        // 随机获取一张图片

    }
    else if (isFileExist(pictures_dir + name + ".jpg"))
    {
        pixmap = QPixmap(pictures_dir + name + ".jpg");
//        qDebug() << "选择了 jpg 图片";
    }
    else if (isFileExist(pictures_dir + name + ".png"))
    {
        pixmap = QPixmap(pictures_dir + name + ".png");
    }
    else
    {
        return ;
    }

    buffer_opacity = -1;
}

void EnvironmentPictures::draw(QPainter &painter)
{
    if (!widget || !cur_opacity) return;

    if (buffer_opacity != cur_opacity) // 使用缓存的透明图片
    {
        buffer_pixmap = QPixmap(pixmap.size());
        buffer_pixmap.fill(Qt::transparent); // 支持 png

        QPainter p(&buffer_pixmap);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(0,0,pixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(buffer_pixmap.rect(), QColor(0,0,0,cur_opacity));
        p.end();

        buffer_opacity = cur_opacity;
    }

    painter.drawPixmap(QRect( -1, -1, widget->size().width()+2, widget->size().height()+2), buffer_pixmap);
}

qint64 EnvironmentPictures::getTimestamp()
{
	return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void EnvironmentPictures::modifyOpacity()
{
    if (cur_opacity != aim_opacity)
    {
        if (cur_opacity < aim_opacity)
        {
            cur_opacity++;
        }
        else // if (cur_opacity > aim_opacity)
        {
            cur_opacity--;
            if (cur_opacity == 0) // 当前图片切换结束了，开始判断下一张图片
            {
//                qDebug() << "一张图片显示结束，开始切换下一张图片";
                selectMaxPixmap();
            }
        }

        emit signalRedraw();
//        qDebug() << "opacity_timer:" << cur_opacity << " / " << aim_opacity;
    }
}

void EnvironmentPictures::eliminate()
{
    // 消除第一个
    if (queue.size() > 0)
    {
        int index = queue.first();
        list[index].value--;
        queue.removeFirst();

        int sq = sqrt(queue.size());
        int interval = ENVIRONMENT_ELIMINATE_INTERVAL / (sq+2); // 数量越多，消除越快
        eliminate_timer->setInterval(interval);

        selectMaxPixmap();
    }
    else
    {
        eliminate_timer->stop();
    }
}
