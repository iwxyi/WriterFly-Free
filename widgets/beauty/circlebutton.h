#ifndef CIRCLEBUTTON_H
#define CIRCLEBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QIcon>
#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QLabel>
#include "globalvar.h"
#include "defines.h"
#include "generalbuttoninterface.h"

class CircleButton : public GeneralButtonInterface
{
    Q_OBJECT
public:
    CircleButton(QWidget *parent = nullptr);
    CircleButton(QString icon, QWidget *parent);
    CircleButton(QIcon icon, QWidget *parent);
    CircleButton(QIcon icon, QString text, QWidget *parent);
    CircleButton(QString icon, QString text, QWidget* parent);
    CircleButton(bool squ, QString icon, QWidget* parent);

    void updateUI() override;

    void showFore() override;
    void hideFore() override;
    void showBack() override;
    void hideBack() override;

    void disableFixed() override;
    void setFixed() override;

protected:
    virtual void initView();

signals:

public slots:

protected:
    QString icon_path; // 图标路径；为空时不设置图标
    bool border_width; // 边界宽度，默认为0
    bool square;       // 方形
    int fore_alpha;    // 显示的前景透明度（动画？）
    int back_alpha;    // 显示的背景透明度
    QColor bg_color;   // 背景颜色
    QColor bd_color;   // 边界颜色

    QPropertyAnimation* ani;
    QLabel* ani_label;
    bool m_pixmap_setted;
    QPixmap m_pixmap;
    DirectType store_direct; // 隐藏时的存放位置
};

#endif // CIRCLEBUTTON_H
