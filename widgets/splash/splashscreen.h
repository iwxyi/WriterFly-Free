#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QObject>
#include <QWidget>
#include <QSettings>
#include <QSplashScreen>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QTimer>
// #include <QGraphicsDropShadowEffect>
#include "settings.h"

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
    Q_PROPERTY(int angle READ getAngle WRITE setAngle)
    Q_PROPERTY(double scale READ getScale WRITE setScale)
public:
    SplashScreen();

    void toClose();
    void setSplashPixmap(QPixmap pixmap);

protected:
    void showEvent(QShowEvent* event);
    void paintEvent(QPaintEvent* event);

private:
	void calcAngle(QPoint start, QPoint mid);
    int getAngle();
    void setAngle(int angle);
    double getScale();
    void setScale(double scale);

private:
    QPixmap pixmap;
    QRect fore_geo;
    int max_angle;
    int angle;
    double scale;

    QColor bg_color;

    bool ani_finished;
    bool to_close;

    // QGraphicsDropShadowEffect *shadow_effect;
};

#endif // SPLASHSCREEN_H
