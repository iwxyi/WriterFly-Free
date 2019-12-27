#ifndef SEARCHDELEGATE_H
#define SEARCHDELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QWidget>
#include <QRect>
#include <QLineEdit>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include "searchresultbean.h"
#include "defines.h"
#include "globalvar.h"

/**
 * 目录列表代理类
 */
class SearchDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SearchDelegate(QObject *parent = nullptr) : QItemDelegate (parent)
    {
        item_width = &(rt->search_panel_item_width);
    }

    /**
     * 绘制HTML富文本：https://www.cnblogs.com/findumars/p/5702128.html
     * @param painter
     * @param option
     * @param index
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // 获取文字
        ResultType type = static_cast<ResultType>(index.data(Qt::UserRole+1).toInt());
        QString data = index.data(Qt::UserRole).toString();
        QString text = index.data(Qt::DisplayRole).toString();
        QString title = index.data(TITLE_ROLE).toString();
        QString showed = index.data(SHOW_ROLE).toString();

        // 获取行高
        int line_height = 0;
        QFontMetrics fm(painter->font());
        line_height = fm.lineSpacing();

        // 获取样式
        QStyleOptionViewItem op(option);
        QStyle *style = op.widget ? op.widget->style() : QApplication::style();
        op.text = "";
        // style->drawControl(QStyle::CE_ItemViewItem, &op, painter, op.widget); // 绘制控制情况（例如鼠标悬浮、选中）
        // QRect drawed_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &op); // 绘制的矩形坐标区域， == op.rect
        QRect drawed_rect = op.rect; // 用这个来替换 style 方式，可以分开设置通过 style 设置的绘制选中边框（即某些 type 可以不加上选中）

        // 先绘制项目背景（如果需要）
        if (type == CARDLIB) // 名片，绘制背景卡片
        {
            // 绘制背景卡片
            int margin = line_height/4;
            drawed_rect = QRect(drawed_rect.left() + margin, drawed_rect.top() + margin, drawed_rect.width() - margin*2, drawed_rect.height() - margin*2);

            QPainterPath path;
            path.addRoundedRect(drawed_rect, us->widget_radius, us->widget_radius);
            QColor bg = us->mainwin_bg_color;
            painter->fillPath(path, bg);
            if (option.state & QStyle::State_MouseOver) // 鼠标悬浮（选中，但浅）
                painter->fillPath(path, us->getOpacityColor(us->accent_color, 64));
            else if (option.state & QStyle::State_Selected) // 选中后，再通过按键移动，才是深色
                painter->fillPath(path, us->getOpacityColor(us->accent_color, 128));

            // 缩小文字区域
            drawed_rect = QRect(drawed_rect.left() + margin + us->widget_radius, drawed_rect.top() + margin + us->widget_radius, drawed_rect.width() - margin * 2 - us->widget_radius * 2, drawed_rect.height() - margin * 2 - us->widget_radius * 2);
        }
        else
        {
            style->drawControl(QStyle::CE_ItemViewItem, &op, painter, op.widget); // 用户控制情况（例如鼠标悬浮、选中）
        }

        // 绘制纯色标题
        painter->save();
        QAbstractTextDocumentLayout::PaintContext paint_context;
        if (!title.isEmpty())
        {
            if (type == CARDLIB)
            {
                QString cs = getXml(data, "COL");
                QColor c = qvariant_cast<QColor>(cs);
                painter->setPen(c);
            }
            painter->drawText(drawed_rect, title);
            drawed_rect.adjust(0, line_height, 0, line_height);
        }
        painter->restore();

        // 绘制彩色信息
        painter->save();
        painter->translate(drawed_rect.topLeft()); // 将左上角设置为原点
        painter->setClipRect(drawed_rect.translated(-drawed_rect.topLeft())); // 设置 HTML 绘制区域
        QTextDocument doc;
        doc.setHtml(showed);
        if (type == ResultType::CURRENT_CHAPTER || type == ResultType::OPENED_CHAPTERS
                || type == ResultType::NOVEL_CHAPTERS || type == ResultType::ALL_CHAPTERS)
            ;
        else
            doc.setTextWidth(drawed_rect.width());
        doc.documentLayout()->draw(painter, paint_context);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QSize size = option.decorationSize;
        QFontMetrics fm(option.font);
        size.setHeight(size.height()+fm.lineSpacing()/2);

        ResultType type = static_cast<ResultType>(index.data(Qt::UserRole+1).toInt());
        // 固定两行文本：章节
        if (type == ResultType::CURRENT_CHAPTER || type == ResultType::OPENED_CHAPTERS
                || type == ResultType::NOVEL_CHAPTERS || type == ResultType::ALL_CHAPTERS)
            size.setHeight(size.height() + fm.lineSpacing());
        // 固定一行大小：词语
        else if (type == ResultType::NAMES || type == ResultType::SYNONYMS || type == ResultType::RELATEDS || type == ResultType::COMMAND)
            ;
        // 自动调整大小
        else
        {
            QString title = index.data(TITLE_ROLE).toString();
            QString showed = index.data(SIZE_ROLE).toString();

            int line_height = 0;
            QFontMetrics fm(option.font);
            line_height = fm.lineSpacing();

            /* QTextDocument doc;
            doc.setHtml(showed);
            doc.setTextWidth(*item_width);
            doc.adjustSize();
            int height = doc.size().height(); */
            int height = 0;
            int width = *item_width;
            if (type == CARDLIB) // 宽度不一样的
            {
                width -= line_height + us->widget_radius*2;
                height = fm.lineSpacing() * (fm.horizontalAdvance(showed)+width-1) / width;
                height += line_height;
            }
            else
            {
                height = fm.lineSpacing() * (fm.horizontalAdvance(showed)+width-1) / width;
            }
            size.setHeight(height);
        }

        return size;
    }

    const int* item_width;
};

#endif // SEARCHDELEGATE_H
