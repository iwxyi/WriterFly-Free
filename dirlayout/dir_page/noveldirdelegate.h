#ifndef NOVELDIRDELEGATE_H
#define NOVELDIRDELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QWidget>
#include <QRect>
#include <QLineEdit>
#include <noveldirmodel.h>
#include <noveldiritem.h>
#include "defines.h"
#include "noveldirsettings.h"

const int bubble_padding = 6;

/**
 * 目录列表代理类
 */
class NovelDirDelegate : public QItemDelegate, public NovelDirSettings
{
    Q_OBJECT
public:
    NovelDirDelegate(QObject *parent = nullptr)
    {
        Q_UNUSED(parent);

        for (int i = 0; i < 105; i++)
            sqrts[i] = static_cast<int>(sqrt(i));
    }

    QString getItemText(const QModelIndex &index, bool ten_align = true) const
    {
        QString text = "", numstr = "";
        const QString& name = index.data(Qt::UserRole+DRole_CHP_NAME).toString();
        if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 卷
        {
            if (isShowRollNum() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() != 0)
            {
                int num = getRollStartNum()+index.data(Qt::UserRole+DRole_CHP_NUM).toInt() - 1;
                if (num > 0)
                {
                    if (isUseArab()) // 阿拉伯卷序
                    {
                        // numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM).toString()+getRollChar()+QString(" ");
                        numstr = QString("第")+QString("%1").arg(num)+getRollChar()+QString(" ");
                    }
                    else // 中文卷序
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM_CN).toString()+getRollChar()+QString(" ");
                        numstr = QString("第")+ArabToCN(num)+getRollChar()+QString(" ");
                    }
                }

            }
            text = numstr + name;
        }
        else // 章
        {
            if (isShowChapterNum() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() != 0)
            {
                int num;
                if (isRecountNumByRoll())
                    num = getChapterStartNum()-1 + index.data(Qt::UserRole+DRole_CHP_CHPTINDEX).toInt();
                else
                    num = getChapterStartNum()-1 + index.data(Qt::UserRole+DRole_CHP_NUM).toInt();
                if (num > 0)
                {
                    if (isUseArab()) // 阿拉伯章序
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM).toString()+getChapterChar()+QString(" ");
                        numstr = QString("第")+QString("%1").arg(num)+getChapterChar()+QString(" ");
                    }
                    else // 中文数字章序
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM_CN).toString()+getChapterChar()+QString(" ");
                        // 整十对齐
                        QString whole_ten_align_str = "";
                        if (ten_align) // 如果开启整十对齐
                        {
                            int end_2 = num % 100;
                            if (end_2 % 10 == 0 && end_2 / 10 > 2 && end_2 / 10 <= 10) // 四十九  五十  五十一
                                whole_ten_align_str = "　";
                            if (num > 100 && end_2 == 20) // 一百一十九  一百二十  一百二十一
                                whole_ten_align_str = "　";
                            if (num > 0 && num % 100 == 0) // 一百九十九  二百  二百零一
                                whole_ten_align_str = "　　";
                        }

                        numstr = QString("第")+ArabToCN(num)+getChapterChar()+whole_ten_align_str+QString(" ");
                    }
                }
            }
            text = numstr + name;
        }
        return text;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // 如果正在编辑，则什么都不画
        if (option.state & QStyle::State_Editing)
            return ;

        QStyleOptionViewItem op(option);
        //if (!us->round_view)
        //    op.palette.setColor(QPalette::Highlight, us->getOpacityColor(us->accent_color, 128));
        //else
        //    op.palette.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
        painter->save();

        // 获取图像绘制区域
        //QRect decorationRect = QRect(option.rect.topLeft(), QSize(80,80));

        // 获取文字
        QString text= getItemText(index);
        int deviate = DIR_DEVIATE; // 初始偏移位置
        int subs = -1; // 分卷数量，章节为-1
        if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 是分卷，画数字
        {
            subs = index.data(Qt::UserRole+DRole_ROLL_SUBS).toInt();
        }
        else // 是章节
        {
            if (isNoRoll() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() > 0) // 不显示分卷 且 是正文，则取消缩进
                ;
            else
                deviate += DIR_DEVIATE_ADD; // 增加章节的缩进
        }

        // 获取文字区域并绘制
        QFontMetrics fm(painter->font());
        QRect text_rect = QRect(op.rect.topLeft()+QPoint(deviate,(op.decorationSize.height()+us->mainwin_sidebar_spacing*2-fm.height())/2),QSize(fm.width(text),fm.height()));
        /*if (!us->round_view) // 方形 绘制背景
        {
            if (option.state & QStyle::State_Active) // State_Focus/State_Active 是聚焦的效果，暂时发现是一样的
                op.palette.setColor(QPalette::Highlight, us->getOpacityColor(us->accent_color, 128));
            else // 没有聚焦
                op.palette.setColor(QPalette::Highlight, us->getOpacityColor(us->accent_color, 64));
            drawBackground(painter, op, index); // 画选中的颜色的（不加这行的话就是透明选中）
        }*/

        // 先绘制背景
        if (!index.data(Qt::UserRole+DRole_RC_ANIMATING).toBool()) // 不是在显示动画中（动画时不显示文字，即背景透明）
        {
            // 绘制选中状态的纸片形状背景
            //if (us->round_view && index.data(Qt::UserRole+DRole_RC_SELECTING).toBool())
            int hover_leave_prog = index.data(Qt::UserRole+DRole_RC_HOVERPROG).toInt(); // 悬浮背景的进度
            int unselect_prog = index.data(Qt::UserRole+DRole_RC_UNSELECT_PROG100).toInt(); // 取消选中进度。没有选中时，需要用这个方法来判断有没有选中

            // 圆形控件，并且：1鼠标悬浮；2鼠标点击；3透明度不是0
            if ( (option.state & QStyle::State_MouseOver) || (option.state & QStyle::State_Selected) || hover_leave_prog || unselect_prog)
            {
                int timer_time = 0; // 鼠标更新时间

                if (us->dir_item_bg_model == 1) // 圆形气泡
                {
                    // 获取绘制的气泡区域
                    QRect bubble_rect;
                    int bubble_radius;
                    if (us->round_view)
                    {
                        int text_padding = text_rect.height()/2-1;
                        bubble_rect = QRect(text_rect.left()-text_padding - bubble_padding, text_rect.top()-bubble_padding, text_rect.width()+text_padding*2+bubble_padding*2, text_rect.height()+bubble_padding*2);
                        bubble_radius = bubble_rect.height()/2-1;
                    }
                    else
                    {
                        bubble_rect = op.rect;
                        bubble_radius = 0;
                    }

                    int rect_max_side = bubble_rect.width() > bubble_rect.height() ? bubble_rect.width() : bubble_rect.height();
                    int rect_min_side = bubble_rect.width() < bubble_rect.height() ? bubble_rect.width() : bubble_rect.height();
                    bool horizone = bubble_rect.width() >= bubble_rect.height();

                    // 鼠标悬浮背景（和点击完全独立）
                    if (option.state & QStyle::State_MouseOver) // 鼠标悬浮，渐变出现
                    {
                        QPainterPath path; // 绘制背景的形状
                        path.addRoundedRect(bubble_rect, bubble_radius, bubble_radius);

                        // 开始绘制背景
                        painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿
                        painter->fillPath(path, us->getOpacityColor(us->accent_color, 64*(index.data(Qt::UserRole+DRole_RC_NEXT_HOVERPROG).toInt())/100));
                        painter->setRenderHint(QPainter::Antialiasing, false); // 关闭抗锯齿，避免文字变形

                        if (hover_leave_prog < 100)
                            timer_time = 10;
                    }
                    else if (hover_leave_prog > 0) // 不是悬浮状态，渐变取消
                    {
                        QPainterPath path; // 绘制背景的形状
                        path.addRoundedRect(bubble_rect, bubble_radius, bubble_radius);

                        // 开始绘制背景
                        painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿
                        painter->fillPath(path, us->getOpacityColor(us->accent_color, 64*(index.data(Qt::UserRole+DRole_RC_PREV_HOVERPROG).toInt())/100));
                        painter->setRenderHint(QPainter::Antialiasing, false); // 关闭抗锯齿，避免文字变形

                        if (hover_leave_prog > 0)
                            timer_time = 10;
                    }

                    // 选中状态
                    if (option.state & QStyle::State_Selected)
                    {
                        // 判断绘制的路径
                        QPoint click_point = index.data(Qt::UserRole+DRole_RC_CLICKPOINT).toPoint(); // 开始动画的地点
                        int select_prog = index.data(Qt::UserRole+DRole_RC_SELECT_PROG100).toInt(); // 动画的进度
                        QPainterPath path; // 绘制背景的形状

                        if (select_prog >= 100) // 动画已经结束了
                        {
                            path.addRoundedRect(bubble_rect, bubble_radius, bubble_radius); // 添加圆角矩形背景
                        }
                        else // 正在动画中
                        {
                            int ani_dis = rect_max_side * select_prog / 100; // 长边的进度
                            if (ani_dis <= rect_min_side) // 未超过小的边长（高度），画圆
                            {
                                int radius = ani_dis / 2; // 动画圆形半径

                                if (click_point.x() < bubble_rect.left() + radius)
                                    click_point.setX(bubble_rect.left()+radius);
                                if (click_point.x() > bubble_rect.right() - radius)
                                    click_point.setX(bubble_rect.right()-radius);
                                if (click_point.y() < bubble_rect.top() + radius)
                                    click_point.setY(bubble_rect.top()+radius);
                                if (click_point.y() > bubble_rect.bottom()-radius)
                                    click_point.setY(bubble_rect.bottom()-radius);

                                path.addEllipse(click_point.x()-radius, click_point.y()-radius, ani_dis, ani_dis);
                            }
                            else // 已经超过小的边长，画圆角矩形
                            {
                                int radius = rect_min_side / 2; // 短边半径
                                int half_dis = ani_dis / 2;

                                // 调整位置
                                if (horizone) // 横着的
                                {
                                    // 判断图形大小有没有超过气泡边界
                                    if (click_point.x() < bubble_rect.left() + half_dis)
                                        click_point.setX(bubble_rect.left()+half_dis);
                                    if (click_point.x() > bubble_rect.right() - half_dis)
                                        click_point.setX(bubble_rect.right()-half_dis);
                                    if (click_point.y() < bubble_rect.top() + radius)
                                        click_point.setY(bubble_rect.top()+radius);
                                    if (click_point.y() > bubble_rect.bottom()-radius)
                                        click_point.setY(bubble_rect.bottom()-radius);
                                    // path.addRoundedRect(QRect(click_point.x()-half_dis, click_point.y()-radius+(rect_min_side&1), ani_dis, rect_min_side-(rect_min_side&1)), bubble_radius, bubble_radius);
                                    path.addRoundedRect(QRect(click_point.x()-half_dis, bubble_rect.y(), ani_dis, bubble_rect.height()), bubble_radius, bubble_radius);
                                }
                                else // 竖着的
                                {
                                    if (click_point.x() < bubble_rect.left() + radius)
                                        click_point.setX(bubble_rect.left()+radius);
                                    if (click_point.x() > bubble_rect.right() - radius)
                                        click_point.setX(bubble_rect.right()-radius);
                                    if (click_point.y() < bubble_rect.top() + half_dis)
                                        click_point.setY(bubble_rect.top()+half_dis);
                                    if (click_point.y() > bubble_rect.bottom()-half_dis)
                                        click_point.setY(bubble_rect.bottom()-half_dis);
                                    path.addRoundedRect(QRect(click_point.x()-radius, click_point.y()-half_dis, rect_min_side, ani_dis), bubble_radius, bubble_radius);
                                }
                            }

                            timer_time = 10;//+sqrts[ani_distance];
                        }

                        // 开始绘制背景
                        painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿
                        painter->fillPath(path, us->getOpacityColor(us->accent_color, 128));
                        painter->setRenderHint(QPainter::Antialiasing, false); // 关闭抗锯齿，避免文字变形
                    }
                    else if (unselect_prog)
                    {
                        int select_prog = index.data(Qt::UserRole+DRole_RC_SELECT_PROG100).toInt();
                        QPoint click_point = index.data(Qt::UserRole+DRole_RC_CLICKPOINT).toPoint(); // 开始动画的地点
                        QPainterPath path; // 绘制背景的形状

                        if (us->round_view)
                        {
                            // 根据出现的进度判断消失的进度
                            int ani_dis = rect_max_side * select_prog / 100; // 长边的进度
                            if (ani_dis <= rect_min_side) // 未超过小的边长（高度），视作圆（无视可怜又微小的那一个点吧！）
                                ani_dis = rect_min_side;

                            int radius = rect_min_side / 2; // 短边半径
                            int half_dis = ani_dis / 2;

                            // 调整位置
                            if (horizone) // 横着的
                            {
                                // 判断图形大小有没有超过气泡边界
                                if (click_point.x() < bubble_rect.left() + half_dis)
                                    click_point.setX(bubble_rect.left()+half_dis);
                                if (click_point.x() > bubble_rect.right() - half_dis)
                                    click_point.setX(bubble_rect.right()-half_dis);
                                if (click_point.y() < bubble_rect.top() + radius)
                                    click_point.setY(bubble_rect.top()+radius);
                                if (click_point.y() > bubble_rect.bottom()-radius)
                                    click_point.setY(bubble_rect.bottom()-radius);
                                // 计算点击时显示出来的气泡大小
                                QRect real_bubble(click_point.x()-half_dis, bubble_rect.y(), ani_dis, bubble_rect.height());

                                // 计算显示出来的气泡距离四边的距离
                                int margin_left = real_bubble.left() - op.rect.left(), margin_top = real_bubble.top() - op.rect.top();
                                int delta_width = margin_left*2/*op.rect.width()-ani_dis*/, delta_height = op.rect.height()-bubble_rect.height();
                                int left = real_bubble.left() - margin_left * unselect_prog / 100,
                                        top = real_bubble.top() /*- margin_top * unselect_prog / 100*/,
                                        width = real_bubble.width() + delta_width * unselect_prog / 100,
                                        height = real_bubble.height() /*+ delta_height * unselect_prog / 100*/;
                                radius = radius * (100 - unselect_prog) / 100;
                                path.addRoundedRect(QRect(left, top, width, height), radius, radius);
                            }
                            else // 竖着的（无视吧，不管了）
                            {

                            }

                            // 开始绘制背景
                            painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿
                            painter->fillPath(path, us->getOpacityColor(us->accent_color, 128 * (100-unselect_prog) * select_prog / 10000));
                            painter->setRenderHint(QPainter::Antialiasing, false); // 关闭抗锯齿，避免文字变形
                        }
                        else
                        {

                        }

                        if (unselect_prog < 100)
                            timer_time = 10;
                    }
                }

                if (timer_time > 0)
                {
                    QTimer::singleShot(timer_time, [=]{ // 每隔一段时间就重新刷新，扩大范围
                        emit signalUpdateIndex(index);
                    });
                }
            }

            // 绘制文字
            if (us->dir_item_text_model == 1)
            {
                if (isNoRoll() && subs >= 0) // 不使用分卷，并且刚好是分卷，则使用灰色，弱化分卷存在感
                {
                    painter->setPen(QColor(128, 128, 128));
                }
                else
                    painter->setPen(us->global_font_color); // 设置为全局字体颜色
                painter->drawText(text_rect, text);
            }

            // 画左边的打开章节小标记
            if (subs >= 0) // 是分卷（章节默认 -1）
            {

            }
            else // 是章节，绘制打开状态
            {
                if (us->dir_item_open_model == 1)
                {
                    if (index.data(Qt::UserRole+DRole_CHP_OPENING).toBool()) // 打开状态中，画右边的小点
                    {
                        int ani_distance = index.data(Qt::UserRole+DRole_RC_SELECT_PROG100).toInt(); // 动画的进度
                        QRect state_rect = QRect(op.rect.topLeft()+QPoint(0,1),QSize(1,op.rect.height()-2));
                        if ((option.state & QStyle::State_Selected) && ani_distance < 100)
                        {
                            //state_rect.setHeight(state_rect.height() * ani_distance / 100);
                            //state_rect.moveTop(state_rect.center().y()-state_rect.height()/2);
                            painter->setPen(QPen(QColor(us->getOpacityColor(us->accent_color, 240*ani_distance/100)), Qt::SolidLine));
                        }
                        else
                        {
                            painter->setPen(QPen(QColor(us->accent_color), Qt::SolidLine));
                        }

                        //painter->setBrush(QBrush(QColor(us->accent_color), Qt::SolidPattern));
                        painter->drawRect(state_rect);
                    }
                }
            }

            // 画右边的小标记，分卷数量
            if (subs >= 0) // 是分卷（章节默认 -1）
            {
                if (isShowWordCount())
                {
                    int count = index.data(Qt::UserRole+DRole_RC_WORD_COUNT).toInt();
                    if (count >= 0)
                    {
                        painter->setPen(QColor(128, 128, 128));
                        QString text = QString::number(count);
                        QRect display_rect2 = QRect(op.rect.topRight()-QPoint(fm.width(text)+10,-2-us->mainwin_sidebar_spacing),QSize(fm.width(text),fm.height()));
                        painter->drawText(display_rect2, text);
                    }
                }
                else
                {
                    if (subs == 0) // 没有章节，灰色的数字
                    {
                        painter->setPen(QColor(128, 128, 128));
                    }
                    else // 有章节，黑偏灰色的
                    {
                        painter->setPen(QColor(64, 64, 64));
                    }
                    QString subs_text = QString("%1").arg(subs);
                    QRect display_rect2 = QRect(op.rect.topRight()-QPoint(fm.width(subs_text)+10,-2-us->mainwin_sidebar_spacing),QSize(fm.width(subs_text),fm.height()));
                    painter->drawText(display_rect2, subs_text);
                }

            }
            else // 是章节，绘制字数
            {
                if (isShowWordCount())
                {
                    int count = index.data(Qt::UserRole+DRole_RC_WORD_COUNT).toInt();
                    if (count >= 0)
                    {
                        painter->setPen(QColor(128, 128, 128));
                        QString text = QString::number(count);
                        QRect display_rect2 = QRect(op.rect.topRight()-QPoint(fm.width(text)+10,-2-us->mainwin_sidebar_spacing),QSize(fm.width(text),fm.height()));
                        painter->drawText(display_rect2, text);
                    }
                }
            }
        }

        // drawFocus(painter, option, displayRect); // 画选区虚线
        drawFocus(painter, op, QRect(0, 0, 0, 0));

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index);
        /*QStyleOptionViewItem o2(option);
        o2.rect.setHeight(option.rect.height()+10);*/
        //QSize size = option.rect.size(); // 这个是整个QListView的矩形……
        QSize size = option.decorationSize;
        int font_height = option.fontMetrics.height() + option.fontMetrics.lineSpacing()/2;
        int std_height = size.height()+us->mainwin_sidebar_spacing*2;
        size.setHeight(qMax(font_height, std_height));
        return size;//QItemDelegate::sizeHint(o2, index);
    }

    /* 编辑框修改名字 */

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const
    {
        Q_UNUSED(option);
        QLineEdit* edit = new QLineEdit(parent);
        edit->setAcceptDrops(false);
        QString style = "";//"background-color:"+us->getColorString(us->accent_color)+";";
        if (us->round_view)
        {
            QString text= getItemText(index);
            QFontMetrics fm(edit->font());
            int r = fm.height()/2;
            /*if (r < edit->height()/3) // 不知道为什么不太对？ /2 太大了，无法圆角
                r = edit->height()/3;*/
            style += "border:1px "+us->getColorString(us->accent_color)+";border-radius:"+QString::number(r)
                    +"px; padding-left: "+QString::number(r+bubble_padding)+"px;";
        }
        else
            style += "border:none;";
        edit->setStyleSheet(style);

        QPalette palette = edit->palette();
        if (us->editor_font_color.alpha() > 0)
            palette.setColor(QPalette::Text, us->editor_font_color);
        palette.setColor(QPalette::Base, us->accent_color/*editor_bg_color*/);
        if (us->editor_font_selection.alpha() > 0)
            palette.setColor(QPalette::HighlightedText, us->editor_font_selection);
        palette.setColor(QPalette::Highlight, us->editor_bg_selection);
        edit->setPalette(palette);

        return edit;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString name = index.data(Qt::UserRole+DRole_CHP_NAME).toString();
        if (index.data(Qt::UserRole+DRole_CHP_STEMP).toString() != "") // 上次编辑（编辑失败）
            name = index.data(Qt::UserRole+DRole_CHP_STEMP).toString();
        QLineEdit* edit = static_cast<QLineEdit*>(editor);
        edit->setText(name);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QLineEdit* edit = static_cast<QLineEdit*>(editor);
        QString str = edit->text();
        //closeEditor(edit, QAbstractItemDelegate::NoHint);
        if (model->setData(index, str))
            model->setData(index, str, Qt::UserRole+DRole_CHP_STEMP);
        else
            model->setData(index, str, Qt::UserRole+DRole_CHP_STEMP);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const
    {
        Q_UNUSED(index);
        int deviate = DIR_DEVIATE-2;
        // 不是分卷；或者不是（不使用分卷 && 是作品相关）
        if (!(index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) && !(isNoRoll() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() > 0))
            deviate += DIR_DEVIATE_ADD;
        if (us->round_view)
        {
            QString text= getItemText(index);
            QFontMetrics fm(editor->font());
            int delta = fm.height()/2+ bubble_padding;
            /*if (delta < editor->height()/3)
                delta = editor->height()/3;*/
            deviate -= delta;
        }
        QRect rect = option.rect;
        rect.setLeft(rect.left()+deviate);
        editor->setGeometry(rect);
    }

    QLineEdit* getEditor()
    {
        return rename_edit;
    }


private:


signals:
    void signalUpdateIndex(QModelIndex index) const; // 设置成 const 才能在 paint const 中进行 emit

private :
    //NovelDirSettings dirSettings;
    QLineEdit* rename_edit;

    int sqrts[105];
};

#endif // MYDELEGATE_H
