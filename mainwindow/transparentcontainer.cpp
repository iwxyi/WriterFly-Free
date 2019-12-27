#include "transparentcontainer.h"

TransparentContainer::TransparentContainer(QWidget *parent) : QWidget(parent)
{
    showing = false;
    animating = false;

    setAutoFillBackground(true);
//    setStyleSheet("background: transparent; ");

    bg_pixmap = new QPixmap(parentWidget()->size());
    blur_pixmap = new QPixmap(parentWidget()->size());

    this->hide();

    connect(thm, &AppTheme::windowChanged, this, [=]{
        if (this->isHidden())
            return ;
        this->hide();
        renderPixmap(bg_pixmap);
        setPixmapBlue(bg_pixmap, blur_pixmap, MAX_BLUR_RADIUS);
        this->show();
        update();
    });
}

void TransparentContainer::toShow()
{
    // 设置透明背景后，必须要相差一个像素，否则会出现
    // UpdateLayeredWindowIndirect failed for ptDst=
    // 这样的问题
    setGeometry(QRect(QPoint(1, 1), parentWidget()->size()));
    delete bg_pixmap;
    delete blur_pixmap;
    bg_pixmap = new QPixmap(parentWidget()->size());
    blur_pixmap = new QPixmap(parentWidget()->size());

    renderPixmap(bg_pixmap);

    showing = true;
    animating = true;
    show();

    QPropertyAnimation* ani = new QPropertyAnimation(this, "blur_radius");
    ani->setDuration(300);
    ani->setStartValue(0);
    ani->setEndValue(MAX_BLUR_RADIUS);
    ani->start();
    connect(ani, SIGNAL(finished()), this, SLOT(slotAniFinished()));
}

void TransparentContainer::toHide()
{
    showing = false;
    animating = true;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "blur_radius");
    ani->setDuration(300);
    ani->setStartValue(MAX_BLUR_RADIUS);
    ani->setEndValue(0);
    ani->start();
    connect(ani, SIGNAL(finished()), this, SLOT(slotAniFinished()));

    emit signalToClose(show_type);
}

void TransparentContainer::setShowType(int t)
{
    show_type = t;
}

void TransparentContainer::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this); // 使用 paint 事件绘制背景图
    painter.drawPixmap(0, 0, this->width(), this->height(), *blur_pixmap);
}

void TransparentContainer::mousePressEvent(QMouseEvent *)
{
    if (animating || !showing) return ;
    toHide();
}

void TransparentContainer::keyPressEvent(QKeyEvent *event)
{
    deb(event->key(), "trans key");
    if (event->key() == Qt::Key_Escape)
    {
        deb("esc trans");
        if (animating || !showing) return ;
        toHide();
    }

    return QWidget::keyPressEvent(event);
}

void TransparentContainer::resizeEvent(QResizeEvent *event)
{
    if (bg_pixmap != nullptr) {
        ;
    }//delete bgmap; // 应该用的，但是闪退了。。。
    if (!showing)
        return QWidget::resizeEvent(event);
    bg_pixmap = new QPixmap(event->size());
    parentWidget()->render(bg_pixmap);
    QWidget::resizeEvent(event);
}

void TransparentContainer::renderPixmap(QPixmap *pixmap)
{
    parentWidget()->render(bg_pixmap, QPoint(0, 0), QRect(1, 1, parentWidget()->size().width(), parentWidget()->size().height()));
}

void TransparentContainer::setPixmapBlue(QPixmap *bg_pixmap, QPixmap *blur_pixmap, int radius)
{
    QT_BEGIN_NAMESPACE
        extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
    QT_END_NAMESPACE

    QImage img = (*bg_pixmap).toImage();
    QPainter painter(blur_pixmap);
    qt_blurImage(&painter, img, radius, true, false);
    update();
}

void TransparentContainer::slotAniFinished()
{

    if (showing)
    {
        ;
    }
    else
    {
        hide(); // 动画结束隐藏
        emit signalClosed(show_type);
    }
    animating = false;
}

void TransparentContainer::slotEsc()
{
    mousePressEvent(nullptr);
}

int TransparentContainer::getBlurRadius()
{
    return blur_radius;
}

void TransparentContainer::setBlurRadius(int x)
{
    blur_radius = x;
//    blur_effect->setBlurRadius(x);
    setPixmapBlue(bg_pixmap, blur_pixmap, blur_radius);

}

void TransparentContainer::resetBlurRadius()
{
    blur_radius = 0;
}
