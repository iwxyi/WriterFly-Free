#ifndef DYNAMICBACKGROUNDSETTINGS_H
#define DYNAMICBACKGROUNDSETTINGS_H

#include <QDialog>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QColorDialog>
#include <QSpinBox>
#include "anicirclelabel.h"
#include "mylabel.h"
#include "myspinbox.h"
#include "interactivebuttonbase.h"
#include "globalvar.h"
#include "dynamicbackgroundpreviewwidget.h"

class DynamicBackgroundSettings : public QDialog
{
    Q_OBJECT
public:
    DynamicBackgroundSettings(QWidget* parent = nullptr);

private:
	void initView();
    void initEvent();
    void initColor();

    int getHihglightColor(int x);
    QColor getReverseColor(QColor color, bool highlight = true);

public slots:
    void gradientSpinChanged(int val);

private:
	QRadioButton *none_radio, *pure_radio, *gradient_radio;
    QColor none_color, pure_color1, pure_color2,
        gradient_color1, gradient_color2, gradient_color3, gradient_color4;
    InteractiveButtonBase *none_color_btn, *pure_color1_btn, *pure_color2_btn,
        *gradient_color1_btn, *gradient_color2_btn, *gradient_color3_btn, *gradient_color4_btn;
    MySpinBox* interval_spin;
    QTabWidget *tab_widget;
    DynamicBackgroundPreviewWidget *page0, *page1, *page2;
    DynamicBackgroundInterface *dybg0, *dybg1, *dybg2;
};

#endif // DYNAMICBACKGROUNDSETTINGS_H
