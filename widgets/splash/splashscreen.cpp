#include "splashscreen.h"

SplashScreen::SplashScreen()
{
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    max_angle = angle = 0;
    scale = 0.618;
    ani_finished = false;
    to_close = false;
    bg_color = Qt::white;

    /* shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(4);
    setGraphicsEffect(shadow_effect); */
}

void SplashScreen::toClose()
{
    to_close = true;

    if (ani_finished) // 如果动画结束了，直接关闭
    {
        this->close();
        this->deleteLater();
    }
}

void SplashScreen::setSplashPixmap(QPixmap pixmap)
{
    this->pixmap = pixmap;
}

void SplashScreen::showEvent(QShowEvent *event)
{
    QSplashScreen::showEvent(event);

    // 设置真正窗口的大小
    QRect win_geo(geometry());
    QString path = QApplication::applicationDirPath()+"/data/settings.ini";
    if (QFile::exists(path))
    {
        Settings se(path);
        if (se.contains("layout/win_pos"))
        {
            QVariant var = se.value("layout/win_pos");
            win_geo.moveTo(var.toPoint());
        }
        if (se.contains("layout/win_size"))
        {
            QVariant var = se.value("layout/win_size");
            win_geo.setSize(var.toSize());
            if (win_geo.width() < 300)
                win_geo.setWidth(300);
            if (win_geo.height() < 300)
                win_geo.setHeight(300);
        }
        bg_color = se.getColor("us/mainwin_bg_color", Qt::white);
        bg_color.setAlpha(255);
    }

    // 计算启动图标的大小
    int bian = qMin(win_geo.width()/3, 300);
    bian = qMin(win_geo.height()/3, bian);
    QSize size(bian, bian);
    QPoint pos(win_geo.center() - QPoint(size.width()/2, size.height()/2));

    // 获取鼠标的位置
    QPoint cursor_pos = QCursor::pos();
    calcAngle(cursor_pos, win_geo.center());
    angle = max_angle/2;

    // shadow_effect->setOffset(-5, 5);

    // 窗口出现动画
    QPropertyAnimation* appear_ani = new QPropertyAnimation(this, "geometry");
    appear_ani->setStartValue(QRect(cursor_pos.x() - 16, cursor_pos.y() - 16, 32, 32));
    appear_ani->setEndValue(QRect(pos, size));
    appear_ani->setEasingCurve(QEasingCurve::InQuint);
    appear_ani->setDuration(300);
    appear_ani->start();
    connect(appear_ani, &QPropertyAnimation::finished, [=]{
        appear_ani->deleteLater();
    });

    /*// 图标放大动画
    QPropertyAnimation *scale_ani = new QPropertyAnimation(this, "scale");
    scale_ani->setStartValue(0.5);
    scale_ani->setEndValue(0.618);
    scale_ani->setEasingCurve(QEasingCurve::InQuint);
        scale_ani->setDuration(500);
    scale_ani->start();
    connect(scale_ani, &QPropertyAnimation::finished, [=] {
        scale_ani->deleteLater();
    });*/

    // 角度增大动画
    QPropertyAnimation *angle_ani = new QPropertyAnimation(this, "angle");
    angle_ani->setStartValue(0);
    angle_ani->setEndValue(max_angle);
    angle_ani->setEasingCurve(QEasingCurve::InQuint);
    angle_ani->setDuration(600);
    angle_ani->start();
    connect(angle_ani, &QPropertyAnimation::finished, [=] {
        angle_ani->deleteLater();

        // 角度回退动画
        QPropertyAnimation *angle_ani2 = new QPropertyAnimation(this, "angle");
        angle_ani2->setStartValue(angle);
        angle_ani2->setEndValue(0);
        angle_ani2->setEasingCurve(QEasingCurve::OutBack);
        angle_ani2->setDuration(300);
        angle_ani2->start();
        connect(angle_ani2, &QPropertyAnimation::finished, [=] {
            angle_ani2->deleteLater();

            // QTimer::singleShot(200, [=] {
                // 窗口消失动画
                QPropertyAnimation *narrow_ani = new QPropertyAnimation(this, "geometry");
                narrow_ani->setStartValue(geometry());
                narrow_ani->setEndValue((QRect(geometry().center(), QSize(1, 1))));
                narrow_ani->setEasingCurve(QEasingCurve::InQuint);
                    narrow_ani->setDuration(300);
                narrow_ani->start();
                connect(narrow_ani, &QPropertyAnimation::finished, [=] {
                    narrow_ani->deleteLater();
                });

                // 图标消失动画
                QPropertyAnimation *scale_ani = new QPropertyAnimation(this, "scale");
                scale_ani->setStartValue(scale);
                scale_ani->setEndValue(1.218);
                scale_ani->setEasingCurve(QEasingCurve::InQuint);
                    scale_ani->setDuration(300);
                scale_ani->start();
                connect(scale_ani, &QPropertyAnimation::finished, [=] {
                    scale_ani->deleteLater();
                });

                // 关闭整个 SplashScreen
                QTimer::singleShot(500, [=] {
                    if (to_close) // 需要关闭时动画没有结束，所以在动画结束后直接关闭
                    {
                        this->close();
                        this->deleteLater();
                    }
                    ani_finished = true;
                });
            // });
        });
    });
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    // QSplashScreen::paintEvent(event);

    QPainter painter(this);

    // 画阴影
    int shade = this->width() / 64;
    if (shade > 5)
        shade = 5;
    for (int i = 0; i < shade-1; i++)
    {
//        int alpha = 32 * i;
//        painter.setPen(QColor(0x88, 0x88, 0x88, alpha));
//        painter.drawEllipse(i, shade, width()-i*2, height()-i*2);
        QPainterPath path;
        path.addEllipse(i, shade+1, width()-i*2, height()-i-shade);
        painter.fillPath(path, QColor(0x88, 0x88, 0x88, 16));
    }

    // 画背景
    // painter.setBrush(QBrush(bg_color));
    // painter.drawRect( -1, -1, width()+2, height()+2 );
    QPainterPath path;
    path.addEllipse(shade, shade, width() - shade * 2, height() - shade*2);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(path, bg_color);

    // 画图标
    int trans_x = width() * ((1 - scale)/2 + scale / 4), trans_y = height() * ( (1 - scale)/2 + scale );
    painter.translate(trans_x, trans_y);
    painter.rotate(angle);
    fore_geo = QRect(width()*(1 - scale)/2-trans_x, height()*(1 - scale)/2-trans_y, width()*scale, height()*scale);
    painter.drawPixmap(fore_geo, pixmap);
}

/*
 * 根据坐标差计算动画后需要摇晃的角度
 */
void SplashScreen::calcAngle(QPoint start, QPoint mid)
{
    int angle_min = 5, angle_max = 20; // 角度的范围
    int delta_x = mid.x() - start.x(),
        delta_y = mid.y() - start.y();
    if (delta_y == 0)
        delta_y = 1;
    double prop = abs(delta_x) / (double)abs(delta_y);
    if (prop >= 1)
        max_angle = angle_max;
    else
        max_angle = angle_min + (angle_max - angle_min)*prop;

    if (delta_x < 0)
        max_angle = -max_angle;
}

int SplashScreen::getAngle()
{
    return this->angle;
}

void SplashScreen::setAngle(int angle)
{
    this->angle = angle;
    update();
}

double SplashScreen::getScale()
{
    return scale;
}

void SplashScreen::setScale(double scale)
{
    this->scale = scale;
}
