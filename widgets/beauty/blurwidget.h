#ifndef BLURWIDGET_H
#define BLURWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QTimer>

#define BLUR_TITLEBAR_RADIUS 15

class BlurWidget : public QWidget
{
    Q_OBJECT
public:
    BlurWidget(int blur_radius, QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pixmap, const QPixmap &pixmap2);
    void setBackPixmap(const QPixmap &pixmap);
    void setForePixmap(QPixmap pixmap2);

protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);

signals:

public slots:

private:
    QPixmap pixmap, pixmap2;
    QGraphicsBlurEffect* blur_effect;
    QLabel* fore_widget, *back_widget;
    int blur_radius;
};

#endif // BLURWIDGET_H
