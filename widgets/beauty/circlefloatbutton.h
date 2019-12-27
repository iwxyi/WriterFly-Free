#ifndef CIRCLEFLOATBUTTON_H
#define CIRCLEFLOATBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPixmap>
#include <QBitmap>
#include "generalbuttoninterface.h"
#include "globalvar.h"

class CircleFloatButton : public GeneralButtonInterface
{
public:
    CircleFloatButton(QWidget *parent);
    CircleFloatButton(QString text, QWidget *parent);
    CircleFloatButton(QIcon icon, QString text, QWidget *parent);


    void showFore() override;
    void hideFore() override;
    void showBack() override;
    void hideBack() override;

private:
    void initView();

signals:

public slots:
    void updateUI() override;

private:
    QString icon_path; // 图标路径；为空时不设置图标
    bool wide_width;   // 是否使用宽度自由的按钮，默认为false，即圆形的
    bool border_width; // 边界宽度，默认为0
    int fore_alpha;    // 显示的前景透明度（动画？）
    int back_alpha;    // 显示的背景透明度
    QColor bg_color;   // 背景颜色
    QColor bd_color;   // 边界颜色
};

#endif // CIRCLEFLOATBUTTON_H
