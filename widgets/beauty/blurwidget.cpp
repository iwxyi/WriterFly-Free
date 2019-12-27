#include "blurwidget.h"

BlurWidget::BlurWidget(int blur_radius, QWidget *parent) : QWidget(parent)
{
    this->blur_radius = blur_radius;

    back_widget = new QLabel(this);
    back_widget->setGeometry( 0, -blur_radius, width(), height()+blur_radius );
    fore_widget = new QLabel(this);
    fore_widget->setGeometry( 0, -blur_radius, width(), height()+blur_radius );

    blur_effect = new QGraphicsBlurEffect(this);
    blur_effect->setBlurRadius(blur_radius);
    blur_effect->setBlurHints(QGraphicsBlurEffect::AnimationHint);

//    fore_widget->setGraphicsEffect(blur_effect);
}

void BlurWidget::setPixmap(const QPixmap& pixmap, const QPixmap& pixmap2)
{
    setBackPixmap(pixmap);
    setForePixmap(pixmap2);
}

void BlurWidget::setBackPixmap(const QPixmap& pixmap)
{
     back_widget->setPixmap(pixmap);
}

void BlurWidget::setForePixmap(QPixmap pixmap2)
{

    // 声明图片模糊
    QT_BEGIN_NAMESPACE
        extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
    QT_END_NAMESPACE

    // 设置图片模糊
    {
        QPixmap pixmap(pixmap2.size());
        pixmap.fill(Qt::transparent);
        QImage img = pixmap2.toImage();  // img -blur-> painter(pixmap)
        QPainter painter( &pixmap );
        qt_blurImage( &painter, img, blur_radius, true, false );
        pixmap2 = pixmap;
    }

    // 设置图片透明度
    {
        QPainter p(&pixmap2);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(0,0,pixmap2);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pixmap2.rect(), QColor(0,0,0,64));
        p.end();
    }

    update();
    fore_widget->setPixmap(pixmap2);
}

void BlurWidget::paintEvent(QPaintEvent *e)
{
}

void BlurWidget::resizeEvent(QResizeEvent *e)
{
    back_widget->setGeometry( -5, 0, width(), height() ); // 不知道为什么要偏移一段

//    fore_widget->setGeometry( 0, -blur_radius, width(), height()+blur_radius ); // 实时设置前景宽度时，会报错：UpdateLayeredWindowIndirect failed for
    fore_widget->setGeometry( 0, 0, width(), height() );
}
