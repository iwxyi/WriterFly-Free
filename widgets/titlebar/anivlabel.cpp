#include "anivlabel.h"

AniVLabel::AniVLabel(QWidget* parent) : QWidget(parent)
{
    lb1 = new QLabel(this); // 1 是用来显示的
    lb2 = new QLabel(this); // 2 是用来动画的

    lb1->setGeometry(0, 0, width(), height());
    lb2->setGeometry(0, height(), width(), height());
    //lb2->setGeometry(0, -height(), width(), height());

    // 下面三项仅供测试用
//    this->setStyleSheet("background-color: purple");
    //lb1->setStyleSheet("background-color: red");
    //lb2->setStyleSheet("background-color: yellow");

    lb1->setAlignment(Qt::AlignCenter);
    lb2->setAlignment(Qt::AlignCenter);

    aniing = false;

    /*// 定时调整大小（因为初始化后，主窗口读取屏幕大小，这里的 resizeEvent 无效）
    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]{
        lb1->setGeometry(0, 0, width(), height());
        lb2->setGeometry(0, height(), width(), height());
        delete timer;
    });
    timer->start();*/

    lb1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    lb1->setFixedHeight(us->widget_size);
    lb2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    lb2->setFixedHeight(us->widget_size);
}

void AniVLabel::setMainText(QString text)
{
    if (_text == text) // 避免重复
        return ;

    if (!aniing)
        lb2->setText(_text); // 旧文本
    lb1->setText(text);  // 新文本
    _text = text;

    if (aniing) return ;

    lb1->move(0, height()*3/4);
    lb2->move(0, 0);

    QPropertyAnimation* ani1 = new QPropertyAnimation(lb1, "pos");
    ani1->setDuration(400);
    ani1->setStartValue(lb1->pos());
    ani1->setEndValue(QPoint(0, 0));
    ani1->setEasingCurve(QEasingCurve::OutCubic);
    ani1->start();
    connect(ani1, SIGNAL(finished()), this, SLOT(slotAnimationFinished()));

    QPropertyAnimation* ani2 = new QPropertyAnimation(lb2, "pos");
    ani2->setDuration(500);
    ani2->setStartValue(lb2->pos());
    ani2->setEndValue(QPoint(0, -height()/2));
    ani2->start();

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(lb2);
    effect->setOpacity(1.0);
    lb2->setGraphicsEffect(effect);

    QPropertyAnimation* ani3 = new QPropertyAnimation(effect, "opacity");
    ani3->setDuration(300);
    ani3->setStartValue(1.0);
    ani3->setEndValue(0);
    ani3->start();

    aniing = true;
}

void AniVLabel::setStaticText(QString text)
{
    lb1->setText(text);  // 新文本
    _text = text;
}

void AniVLabel::setMainSize(int size)
{
    QFont font = lb1->font();
    font.setPointSize(size);
    lb1->setFont(font);
    lb2->setFont(font);
}

void AniVLabel::setAlign(Qt::Alignment a)
{
    lb1->setAlignment(a);
    lb2->setAlignment(a);
}

void AniVLabel::setFixedWidgetHeight(int h)
{
    this->setFixedHeight(h);
    lb1->setFixedHeight(h);
    lb2->setFixedHeight(h);
}

void AniVLabel::resizeEvent(QResizeEvent *)
{
    lb1->setGeometry(0, 0, width(), height());
    lb2->setGeometry(0, -height(), width(), height());
}

void AniVLabel::slotAnimationFinished()
{
    aniing = false;
}
