#include "dynamicbackgroundsettings.h"

DynamicBackgroundSettings::DynamicBackgroundSettings(QWidget* parent) : QDialog(parent)
{
	initView();
    initColor();
	initEvent();
}

void DynamicBackgroundSettings::initView()
{
	/**
	 * 设置项
	 * 1、类型：无、纯色、背景
	 * 2、颜色：1 ~ 4
	 * 3、动画开关
	 * 4、动画时间
	 * 5、方向
	 */

	none_radio = new QRadioButton("静态背景", this);
	pure_radio = new QRadioButton("纯色微动", this);
	gradient_radio = new QRadioButton("渐变微动", this);
    interval_spin = new MySpinBox(20, 3600000, this);
    none_color_btn = new InteractiveButtonBase("选择颜色", this);
    pure_color1_btn = new InteractiveButtonBase("颜色边界1", this);
    pure_color2_btn = new InteractiveButtonBase("颜色边界2", this);
    gradient_color1_btn = new InteractiveButtonBase("上颜色边界1", this);
    gradient_color2_btn = new InteractiveButtonBase("上颜色边界2", this);
    gradient_color3_btn = new InteractiveButtonBase("下颜色边界1", this);
    gradient_color4_btn = new InteractiveButtonBase("下颜色边界2", this);
    tab_widget = new QTabWidget(this);
    dybg0 = new DynamicBackgroundPure(this);
    dybg1 = new DynamicBackgroundPure(this);
    dybg2 = new DynamicBackgroundGradient(this);
    page0 = new DynamicBackgroundPreviewWidget(dybg0, this);
    page1 = new DynamicBackgroundPreviewWidget(dybg1, this);
    page2 = new DynamicBackgroundPreviewWidget(dybg2, this);

    /* none_color_btn->setBgColor(QColor(128, 128, 128, 64));
    pure_color1_btn->setBgColor(QColor(128, 128, 128, 64));
    pure_color2_btn->setBgColor(QColor(128, 128, 128, 64));
    gradient_color1_btn->setBgColor(QColor(128, 128, 128, 64));
    gradient_color2_btn->setBgColor(QColor(128, 128, 128, 64));
    gradient_color3_btn->setBgColor(QColor(128, 128, 128, 64));
    gradient_color4_btn->setBgColor(QColor(128, 128, 128, 64)); */
    interval_spin->setValue(us->getInt("dybg/interval", 2000));

	QVBoxLayout* main_vlayout = new QVBoxLayout;

    // 单选按钮
	QHBoxLayout *radios_layout = new QHBoxLayout;
	{
		radios_layout->addWidget(none_radio);
		radios_layout->addWidget(pure_radio);
		radios_layout->addWidget(gradient_radio);
	}

    // 三个页面
	{ // Page0：无
		QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(none_color_btn);
        page0->setLayout(layout);
    }
	{ // Page1：纯色
		QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(pure_color1_btn);
        layout->addWidget(pure_color2_btn);
        page1->setLayout(layout);
    }
	{ // Page2：渐变
		QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(gradient_color1_btn);
        layout->addWidget(gradient_color2_btn);
        layout->addWidget(gradient_color3_btn);
        layout->addWidget(gradient_color4_btn);
        {
            QHBoxLayout* hlayout = new QHBoxLayout;
            QLabel* label = new QLabel("微动变化间隔(毫秒)：", this);
            hlayout->addWidget(label);
            hlayout->addWidget(interval_spin);
            layout->addLayout(hlayout);
        }
        page2->setLayout(layout);
	}
    tab_widget->addTab(page0, "静态");
    tab_widget->addTab(page1, "纯色");
    tab_widget->addTab(page2, "渐变");

	main_vlayout->addLayout(radios_layout);
	main_vlayout->addWidget(tab_widget);
    setLayout(main_vlayout);

    if (us->dynamic_bg_model == Dynamic_None)
    {
        none_radio->setChecked(true);
        tab_widget->setCurrentIndex(0);
    }
    else if (us->dynamic_bg_model == Dynamic_Pure)
    {
        pure_radio->setChecked(true);
        tab_widget->setCurrentIndex(1);
    }
    else if (us->dynamic_bg_model == Dynamic_Gradient)
    {
        gradient_radio->setChecked(true);
        tab_widget->setCurrentIndex(2);
    }

}

void DynamicBackgroundSettings::initEvent()
{
    // ==== 三个单选按钮 ====
	connect(none_radio, &QRadioButton::clicked, [=]{
        if (none_radio->isChecked() && tab_widget->currentIndex() != 0)
            tab_widget->setCurrentIndex(0);
        if (none_radio->isChecked() && us->dynamic_bg_model != Dynamic_None)
        {
            us->setVal("dybg/model", 0);
            us->loadDynamicBackground();
            thm->updateWindow();
        }
	});
	connect(pure_radio, &QRadioButton::clicked, [=]{
        if (pure_radio->isChecked() && tab_widget->currentIndex() != 1)
            tab_widget->setCurrentIndex(1);
        if (pure_radio->isChecked() && us->dynamic_bg_model != Dynamic_Pure)
        {
            us->setVal("dybg/model", 1);
            us->loadDynamicBackground();
        }
	});
	connect(gradient_radio, &QRadioButton::clicked, [=]{
        if (gradient_radio->isChecked() && tab_widget->currentIndex() != 2)
            tab_widget->setCurrentIndex(2);
        if (gradient_radio->isChecked() && us->dynamic_bg_model != Dynamic_Gradient)
        {
            us->setVal("dybg/model", 2);
            us->loadDynamicBackground();
        }
	});

    // ==== 各种设置颜色 ====
    connect(none_color_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(none_color, this, "", QColorDialog::ShowAlphaChannel);
		if (!color.isValid()) return ;
        none_color_btn->setTextColor(getReverseColor(none_color));
        us->setColor("us/mainwin_bg_color", us->mainwin_bg_color = none_color = color);
        dybg0->setColor(none_color, none_color);
        if (us->dynamic_bg_model == Dynamic_None)
        {
            us->loadDynamicBackground();
            thm->updateWindow();
        }
    });
    connect(pure_color1_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(pure_color1, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        pure_color1_btn->setTextColor(getReverseColor(pure_color1));
        us->setColor("dybg/pure_color1", pure_color1 = color);
        dybg1->setColor(pure_color1, pure_color2);
        if (us->dynamic_bg_model == Dynamic_Pure)
            us->loadDynamicBackground();
    });
    connect(pure_color2_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(pure_color2, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        pure_color2_btn->setTextColor(getReverseColor(pure_color2));
        us->setColor("dybg/pure_color2", pure_color2 = color);
        dybg1->setColor(pure_color1, pure_color2);
        if (us->dynamic_bg_model == Dynamic_Pure)
            us->loadDynamicBackground();
    });
    connect(gradient_color1_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(gradient_color1, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        gradient_color1_btn->setTextColor(getReverseColor(gradient_color1));
        us->setColor("dybg/gradient_color1", gradient_color1 = color);
	    dybg2->setColor(gradient_color1, gradient_color2, gradient_color3, gradient_color4);
        if (us->dynamic_bg_model == Dynamic_Gradient)
            us->loadDynamicBackground();
    });
    connect(gradient_color2_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(gradient_color2, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        gradient_color2_btn->setTextColor(getReverseColor(gradient_color2));
        us->setColor("dybg/gradient_color2", gradient_color2 = color);
	    dybg2->setColor(gradient_color1, gradient_color2, gradient_color3, gradient_color4);
        if (us->dynamic_bg_model == Dynamic_Gradient)
            us->loadDynamicBackground();
    });
    connect(gradient_color3_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(gradient_color3, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        gradient_color3_btn->setTextColor(getReverseColor(gradient_color3));
        us->setColor("dybg/gradient_color3", gradient_color3 = color);
	    dybg2->setColor(gradient_color1, gradient_color2, gradient_color3, gradient_color4);
        if (us->dynamic_bg_model == Dynamic_Gradient)
            us->loadDynamicBackground();
    });
    connect(gradient_color4_btn, &InteractiveButtonBase::clicked, [=]{
        QColor color = QColorDialog::getColor(gradient_color4, this, "", QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) return ;
        gradient_color4_btn->setTextColor(getReverseColor(gradient_color4));
        us->setColor("dybg/gradient_color4", gradient_color4 = color);
	    dybg2->setColor(gradient_color1, gradient_color2, gradient_color3, gradient_color4);
        if (us->dynamic_bg_model == Dynamic_Gradient)
            us->loadDynamicBackground();
    });

    // ==== SPin 事件 ====
    connect(interval_spin, SIGNAL(valueChanged(int)), this, SLOT(gradientSpinChanged(int)));
}

void DynamicBackgroundSettings::initColor()
{
    none_color = us->getColor("us/mainwin_bg_color", Qt::white);
    pure_color1 = us->getColor("dybg/pure_color1", QColor(255, 250, 240)/*花卉白*/);
    pure_color2 = us->getColor("dybg/pure_color2", QColor(253, 245, 230)/*舊蕾絲色*/);
    gradient_color1 = us->getColor("dybg/gradient_color1", QColor(230, 230, 250)/*薰衣草紫*/);
    gradient_color2 = us->getColor("dybg/gradient_color2", QColor(204, 204, 255)/*长春花色*/);
    gradient_color3 = us->getColor("dybg/gradient_color3", QColor(240, 248, 255)/*爱丽丝蓝*/);
    gradient_color4 = us->getColor("dybg/gradient_color4", QColor(248, 248, 253)/*幽灵白*/);

    none_color_btn->setTextColor(getReverseColor(none_color));
    pure_color1_btn->setTextColor(getReverseColor(pure_color1));
    pure_color2_btn->setTextColor(getReverseColor(pure_color2));
    gradient_color1_btn->setTextColor(getReverseColor(gradient_color1));
    gradient_color2_btn->setTextColor(getReverseColor(gradient_color2));
    gradient_color3_btn->setTextColor(getReverseColor(gradient_color3));
    gradient_color4_btn->setTextColor(getReverseColor(gradient_color4));

    dybg0->setColor(none_color, none_color);
    dybg1->setColor(pure_color1, pure_color2);
    dybg2->setColor(gradient_color1, gradient_color2, gradient_color3, gradient_color4);
}

int DynamicBackgroundSettings::getHihglightColor(int x)
{
    if (x <= 96 || x >= 160)
    {
        return 255 - x;
    }
    return x < 128 ? 255 : 0;
}

QColor DynamicBackgroundSettings::getReverseColor(QColor color, bool highlight)
{
    // ==== 保证颜色突出显示，和背景颜色不一样 ====
    if (highlight)
    {
        color.setRed(getHihglightColor(color.red()));
        color.setGreen(getHihglightColor(color.green()));
        color.setBlue(getHihglightColor(color.blue()));
    }
    else
    {
        color.setRed(255 - color.red());
        color.setGreen(255 - color.green());
        color.setBlue(255 - color.blue());
    }

    return color;
}

/**
 * 渐变周期变化
 */
void DynamicBackgroundSettings::gradientSpinChanged(int val)
{
    // 保存到设置
    us->setVal("dybg/interval", val);
    us->dynamic_bg->setInterval(val);
}
