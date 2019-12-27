#include "anitabwidget.h"

AniTabWidget::AniTabWidget(QWidget* parent) : QTabWidget(parent),
        pre_index(-1), disable_animation(false), tab_in_bottom(false), tab_eq_width(true),
        transparent_background(true), base_background_color(Qt::transparent), transparent_level(1)
{
    indicator = new QLabel(this);
    ani_indicator = new QPropertyAnimation(indicator, "geometry");

    // 设置指示器
    this->stackUnder(indicator);
    indicator->setFixedHeight(2);
    indicator->setAutoFillBackground(true);
    indicator->show();

    ani_indicator = new QPropertyAnimation(indicator, "geometry");
    connect(ani_indicator, SIGNAL(finished()), this, SLOT(moveIndicator()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));

    // 指示器默认在上面
    QPoint indi_point= this->tabBar()->geometry().bottomLeft();
    indicator->move(indi_point);
}

void AniTabWidget::updateUI()
{
    indicator->setStyleSheet("background-color:" + us->getColorString(us->accent_color));

    /*QString style = QString("QTabWidget::pane{border:none; }\
                    QTabBar::tab:hover {background-color: rgba(128, 128, 128, 64);}\
                    QTabBar::tab:pressed {background-color: rgba(128, 128, 128, 96);}\
                    QTabBar::tab:selected {border:none; border-top: 0px solid gray; border-top-color:%1; padding: 5px;}\
                    QTabBar::tab:!selected {border: none; padding: 5px;}").arg(us->getColorString(us->accent_color));
    if (tab_eq_width)
        style += QString("QTabBar::tab{width:%1px; height:%2px;}").arg(width()/(count()?count():1)-10).arg(us->widget_size/2.54);
    setStyleSheet(style);*/

    QStringList list;
    list << QStringList{"【unified_width】",
            tab_eq_width ? QString("width:%1px;").arg(width()/(count()?count():1)-10) : ""};
    list << QStringList{"【unified_height】",
            tab_eq_width ? QString("height:%2px;").arg(us->widget_size/2.54) : ""};
    thm->setWidgetStyleSheet(this, "tab_widget", list);

    tabBar()->setFixedHeight(us->widget_size);
}

void AniTabWidget::setTabInBottom()
{
    tab_in_bottom = true;

    this->setTabPosition(QTabWidget::South);

    // 指示器到下面来了，修改位置
    // 为什么设置方向后，得阻塞一下才行，不然获取到的都是原来的位置
    QTimer::singleShot(0, [=]{
        int top = this->tabBar()->geometry().top();
        indicator->move(indicator->geometry().left(), top);
        aim_rect.setTop(top);
        moveIndicator();
    });
}

void AniTabWidget::setTabEqualWidth(bool e)
{
    tab_eq_width = e;
}

void AniTabWidget::preventSwitchAnimationOnce()
{
    pre_index = -1;
}

void AniTabWidget::disableSwitchAnimation(bool d)
{
    disable_animation = d;
}

void AniTabWidget::setTransparentBackground(bool transparent, bool level)
{
    transparent_background = transparent;
    this->transparent_level = level;
}

void AniTabWidget::setBaseBacgroundColor(QColor c)
{
    base_background_color = c;
}

void AniTabWidget::switchTab(int index)
{
    // 界面切换的动画
    if (!disable_animation && pre_index >= 0 && pre_index < this->count())
    {
        disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));
        {
            QPoint render_pos = this->geometry().topLeft();
            QSize render_size = this->widget(index)->size();
            QWidget* render_widget = this->parentWidget();
            int level = transparent_level;
            if (!tab_in_bottom)
                render_pos += QPoint(0, height()-widget(0)->height());

            if (!transparent_background)
            {
                render_pos = tab_in_bottom ? QPoint(0, 0) : QPoint(height()-widget(0)->height(), 0);
                render_widget = this;
            }
            else // transparent_background == true
            {
                // 遍历 parentWidget 获取最上层的控件
                while (render_widget->parentWidget() != nullptr && (!transparent_level || --level))
                {
                    render_pos += render_widget->geometry().topLeft();
                    render_widget = render_widget->parentWidget();
                }
            }

            // 渲染背景色
            this->hide();
            {
                pixmap_base = QPixmap(render_size);
                // pixmap_base.fill(Qt::transparent);
                render_widget->render(&pixmap_base, QPoint(0,0), QRect(render_pos, render_size), QWidget::DrawWindowBackground); // 不带控件渲染
                // render_widget->render(&pixmap_base, QPoint(0,0), QRect(render_pos, render_size));
                if (base_background_color != Qt::transparent)
                    pixmap_base.fill(base_background_color);
            }
            this->show();

            // 渲染之前界面的图片
            this->setCurrentIndex(pre_index);
            {
                pixmap_pre = QPixmap(render_size);
                if (transparent_background)
                {
                    pixmap_pre.fill(Qt::transparent);
                    this->widget(pre_index)->render(&pixmap_pre, QPoint(0,0), QRect(QPoint(0,0), render_size), QWidget::DrawChildren);
                }
                else
                {
                    if (base_background_color != Qt::transparent)
                    {
                        pixmap_pre.fill(base_background_color);
                        render_widget->render(&pixmap_pre, QPoint(0,0), QRect(render_pos, render_size), QWidget::DrawChildren);
                    }
                    else
                    {
                        render_widget->render(&pixmap_pre, QPoint(0,0), QRect(render_pos, render_size));
                    }
                }
            }
            // 渲染新界面的图片
            this->setCurrentIndex(index);
            {
                pixmap_new = QPixmap(render_size);
                if (transparent_background)
                {
                    pixmap_new.fill(Qt::transparent);
                    this->widget(index)->render(&pixmap_new, QPoint(0,0), QRect(QPoint(0,0), render_size), QWidget::DrawChildren);
                }
                else
                {
                    if (base_background_color != Qt::transparent)
                    {
                        pixmap_new.fill(base_background_color);
                        render_widget->render(&pixmap_new, QPoint(0,0), QRect(render_pos, render_size), QWidget::DrawChildren);
                    }
                    else
                    {
                        render_widget->render(&pixmap_new, QPoint(0,0), QRect(render_pos, render_size));
                    }
                }
            }

            if (pre_index < index)
            {
                switchAnimation(DIRECT_LEFT);
            }
            else
            {
                switchAnimation(DIRECT_RIGHT);
            }
        }
        connect(this, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));
    }

    QRect tab_rect = this->tabBar()->tabRect(index);
    if (tab_in_bottom)
        tab_rect.moveTop(tab_rect.top()+tabBar()->geometry().top());
    else
//        tab_rect.moveTop(tab_rect.top()+tabBar()->geometry().top()+height()-widget(0)->height()-indicator->geometry().height());
        tab_rect.moveTop(tabBar()->tabRect(currentIndex()).bottom()-indicator->height()+1);
    aim_rect = tab_rect;
    aim_rect.setHeight(indicator->geometry().height());
    moveIndicator();

    // 设置当前index为pre_index
    pre_index = index;
}

void AniTabWidget::switchAnimation(DirectType direct)
{
    if (disable_animation)
        return ;

    QSize size = this->currentWidget()->size();
    int top = tab_in_bottom ? 0 : height()-widget(0)->height();

    // 判断方向
    int left_pre = 0;
    switch (direct)
    {
    case DIRECT_LEFT: // 左转右
        left_pre = -size.width();
        break;
    case DIRECT_RIGHT: // 右转左
        left_pre = size.width();
        break;
    default:
        return ;
    }

    // 背景（挡住真正的前景）
    QLabel* label_base = new QLabel(this);
    label_base->setPixmap(pixmap_base);
    label_base->resize(size);
    label_base->move( 0, top );
    label_base->show();

    // 旧界面
    QLabel* label_pre = new QLabel(this);
    label_pre->setPixmap(pixmap_pre);
    label_pre->show();
    label_pre->move( 0, top );
    label_pre->resize(size);

    QPropertyAnimation* ani_pre = new QPropertyAnimation(label_pre, "geometry");
    ani_pre->setDuration(300);
    ani_pre->setStartValue(label_pre->geometry());
    ani_pre->setEndValue(QRect(left_pre/3, label_pre->geometry().top(), size.width(), size.height()));
    ani_pre->setEasingCurve(QEasingCurve::OutCubic);
    connect(ani_pre, SIGNAL(finished()), label_pre, SLOT(deleteLater()));
    connect(ani_pre, SIGNAL(finished()), ani_pre, SLOT(deleteLater()));
    ani_pre->start();

    QGraphicsOpacityEffect* effect_pre = new QGraphicsOpacityEffect(label_pre);
    effect_pre->setOpacity(1.0);
    label_pre->setGraphicsEffect(effect_pre);

    QPropertyAnimation* ani_pre_opa = new QPropertyAnimation(effect_pre, "opacity");
    ani_pre_opa->setDuration(300);
    ani_pre_opa->setEasingCurve(QEasingCurve::OutCubic);
    ani_pre_opa->setStartValue(1.0);
    ani_pre_opa->setEndValue(0);
    connect(ani_pre_opa, SIGNAL(finished()), effect_pre, SLOT(deleteLater()));
    connect(ani_pre_opa, SIGNAL(finished()), ani_pre_opa, SLOT(deleteLater()));
    ani_pre_opa->start();

    // 新界面
    QLabel* label_new = new QLabel(this);
    label_new->setPixmap(pixmap_new);
    label_new->show();
    label_new->move( 0, top );
    label_new->resize(size);

    QPropertyAnimation* ani_new = new QPropertyAnimation(label_new, "geometry");
    ani_new->setDuration(300);
    ani_new->setStartValue(QRect(-left_pre*2/3, label_new->geometry().top(), size.width()*2/3, size.height()));
    ani_new->setEndValue(label_new->geometry());
    ani_new->setEasingCurve(QEasingCurve::OutCubic);

    QGraphicsOpacityEffect* effect_new = new QGraphicsOpacityEffect(label_new);
    effect_new->setOpacity(1.0);
    label_new->setGraphicsEffect(effect_new);

    QPropertyAnimation* ani_new_opa = new QPropertyAnimation(effect_new, "opacity");
    ani_new_opa->setDuration(300);
    ani_new_opa->setStartValue(0.5);
    ani_new_opa->setEndValue(1.0);
    ani_new_opa->start();

    label_pre->stackUnder(label_new);
    label_base->stackUnder(label_pre);

    connect(ani_new, SIGNAL(finished()), label_new, SLOT(deleteLater()));
    connect(ani_new, SIGNAL(finished()), ani_new, SLOT(deleteLater()));
    connect(ani_new, SIGNAL(finished()), effect_new, SLOT(deleteLater()));
    connect(ani_new, SIGNAL(finished()), label_base, SLOT(deleteLater()));
    ani_new->start();
}

void AniTabWidget::resizeEvent(QResizeEvent *event)
{
//    this->setStyleSheet(QString("QTabBar::tab{width:%1px;}").arg((this->width()-8)*0.9/(count()?count():1))); // 8是margin
    updateUI();

    if (this->currentIndex() >= 0)
    {
        int top;
        if (tab_in_bottom) // 在底部
        {
            top = this->tabBar()->geometry().top();
            if (top == 0)
                top = this->geometry().height();
        }
        else // 在顶部
        {
            if (currentIndex() > -1)
                top = tabBar()->tabRect(currentIndex()).bottom()-indicator->height()+1;
            else
                top = tabBar()->geometry().bottom();
        }
        indicator->move(indicator->x(), top);
        QRect rect = this->tabBar()->tabRect(this->currentIndex());
        aim_rect = QRect(rect.left(), top, rect.width(), indicator->height());
        if (ani_indicator->state() == QAbstractAnimation::Running)
            ani_indicator->stop();
        indicator->setGeometry(aim_rect);
        // moveIndicator();
    }

    return QTabWidget::resizeEvent(event);
}

/**
 * 切换指示器的二段动画
 * @param aim_rect
 */
void AniTabWidget::moveIndicator()
{
    if (ani_indicator->state() == QAbstractAnimation::Running)
        ani_indicator->stop();

    QRect cur_rect = indicator->geometry(); // 之前写成 indicator->rect()，left都是 0，调了半个下午的bug，这两个方法的返回值不同
    cur_rect.moveTop(aim_rect.top());
    int min_left = min(aim_rect.left(), cur_rect.left());
    int max_right = max(aim_rect.right(), cur_rect.right());
    QRect exp_rect(min_left, cur_rect.top(), max_right-min_left, cur_rect.height());

    if (cur_rect == aim_rect) // 动画完毕
        return ;

    ani_indicator->setStartValue(cur_rect);
    ani_indicator->setDuration(150);
    if (cur_rect.left() <= exp_rect.left() && cur_rect.right() >= exp_rect.right()) // 第二段动画：收缩
    {
        ani_indicator->setEndValue(aim_rect);
    }
    else // 第一段动画：扩展
    {
        ani_indicator->setEndValue(exp_rect);
        connect(ani_indicator, SIGNAL(finished()), this, SLOT(moveIndicator()));
    }
    ani_indicator->start();
}
