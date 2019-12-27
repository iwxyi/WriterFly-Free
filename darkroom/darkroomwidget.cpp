#include "darkroomwidget.h"

DarkRoomWidget::DarkRoomWidget(QWidget *parent) : MyDialog(parent)
{
    setWindowTitle("小黑屋");

    initView();
    initData();
    initMenu();
}

void DarkRoomWidget::initView()
{
    desc_label = new QLabel("摒除杂念，抛下红尘，\n强制码字，狠人专用。", this);
	word_label = new QLabel("目标字数", this);
    word_spin = new MySpinBox(0, 100000, this);
    time_label = new QLabel("分钟时长", this);
    time_spin = new MySpinBox(0, 600, this);
    combo_label = new QLabel("解除条件", this);
	either_radio = new QRadioButton("之一", this);
	both_radio = new QRadioButton("两者", this);
    type_label = new QLabel("黑屋类型", this);
	warning_radio = new QRadioButton("警告", this);
	force_radio = new QRadioButton("强制", this);
    start_btn = new InteractiveButtonBase("开始闯小黑屋", this);
    either_radio->setToolTip("完成字数或者时间其中之一即可解除小黑屋（0将会忽略）");
    both_radio->setToolTip("同时完成目标字数、并且到达目标时间方可解除小黑屋（0将忽略）");

	warning_radio->setToolTip("在小黑屋中依旧可退出，仅弹出警告、扣除积分");
	force_radio->setToolTip("必须解除小黑屋后才能退出程序");

	connect(this, SIGNAL(signalConfirm()), this, SLOT(startDarkRoom()));
	connect(start_btn, SIGNAL(clicked()), this, SLOT(startDarkRoom()));

	QVBoxLayout* main_vlayout = main_layout;
    main_vlayout->addSpacing(close_btn->height() / 2);
    main_vlayout->setSpacing(16);
    main_vlayout->setMargin(20);
	{ // 说明
		QHBoxLayout* desc_hlayout = new QHBoxLayout;
		desc_hlayout->addStretch();
        desc_hlayout->addWidget(desc_label);
        desc_hlayout->addStretch();
        main_vlayout->addLayout(desc_hlayout);
	}
	{ // 字数
		QHBoxLayout* word_hlayout = new QHBoxLayout;
		word_hlayout->addWidget(word_label);
		word_hlayout->addWidget(word_spin);
		main_vlayout->addLayout(word_hlayout);
	}
	{ // 时间
		QHBoxLayout* time_hlayout = new QHBoxLayout;
		time_hlayout->addWidget(time_label);
		time_hlayout->addWidget(time_spin);
		main_vlayout->addLayout(time_hlayout);
	}
	{ // 组合条件
        combo_group = new QButtonGroup(this);
        combo_group->addButton(either_radio, DarkRoom::ComboType_Either);
        combo_group->addButton(both_radio, DarkRoom::ComboType_Both);

        QHBoxLayout* combo_hlayout = new QHBoxLayout;
        combo_hlayout->addWidget(combo_label);
        combo_hlayout->addWidget(either_radio);
        combo_hlayout->addWidget(both_radio);

        main_vlayout->addLayout(combo_hlayout);
	}
	{ // 禁止类型
		force_group = new QButtonGroup(this);
        force_group->addButton(warning_radio, DarkRoom::ForceType_Warning);
        force_group->addButton(force_radio, DarkRoom::ForceType_Force);

		QHBoxLayout* type_hlayout = new QHBoxLayout;
		type_hlayout->addWidget(type_label);
		type_hlayout->addWidget(warning_radio);
		type_hlayout->addWidget(force_radio);
		type_hlayout->addStretch();
		main_vlayout->addLayout(type_hlayout);
	}
    {
        suggest_hlayout = new QHBoxLayout;
        main_vlayout->addLayout(suggest_hlayout);
    }
	main_vlayout->addWidget(start_btn);
}

void DarkRoomWidget::initMenu()
{
    QMenu* main_menu = new QMenu(this);
    suggest_words_action = new QAction(thm->icon("menu/habit"), "根据使用习惯预测字数", this);
    record_times_action = new QAction(thm->icon("menu/time_lock"), "还原上次小黑屋时间", this);
    main_menu->addAction(suggest_words_action);
    main_menu->addAction(record_times_action);
    setDialogMenu(main_menu);

    suggest_words_action->setCheckable(true);
    record_times_action->setCheckable(true);
    suggest_words_action->setChecked(us->getBool("us/suggest_words", true));
    record_times_action->setChecked(us->getBool("us/record_times", true));

    connect(suggest_words_action, &QAction::triggered, [=] {
        bool b = !us->getBool("us/suggest_words", true);
        us->setVal("us/suggest_words", b);
        suggest_words_action->setChecked(b);
    });

    connect(record_times_action, &QAction::triggered, [=] {
        bool b = !us->getBool("us/record_times", true);
        us->setVal("us/record_times", b);
        record_times_action->setChecked(b);
    });
}

void DarkRoomWidget::initData()
{
	word_spin->setValue(us->getInt("dr/word", 0));
    if (us->getBool("us/record_times", true))
        time_spin->setValue(us->getInt("dr/time", 0));

    if (us->getInt("dr/combo", DarkRoom::ComboType_Either) == DarkRoom::ComboType_Both)
		both_radio->setChecked(true);
	else
	    either_radio->setChecked(true);

    if (us->getInt("dr/force", DarkRoom::ForceType_Force) == DarkRoom::ForceType_Warning)
		warning_radio->setChecked(true);
	else
	    force_radio->setChecked(true);

    if (us->getBool("us/suggest_words", true))
        addSuggestWordsList(DigitalTimestampPredict::getMaxes(gd->dr.getHistories()));
}

void DarkRoomWidget::showEvent(QShowEvent * e)
{
    start_btn->setTextColor(us->accent_color);
    return MyDialog::showEvent(e);
}

void DarkRoomWidget::startDarkRoom()
{
    int word = word_spin->value();
    int time = time_spin->value();
    DarkRoom::ComboType ct = static_cast<DarkRoom::ComboType>(combo_group->checkedId());
    DarkRoom::ForceType ft = static_cast<DarkRoom::ForceType>(force_group->checkedId());

    if (word == 0 && time == 0)
        return ;

    // 如果字数过多，先进行询问
    if (word > 3000)
    {
        // 获取之前的最大字数
        if (DigitalTimestampPredict::shouldAsk(gd->dr.getHistories(), word))
        {
            if (QMessageBox::information(this, "提示", "该字数已远远打破您的小黑屋记录，是否坚持进入小黑屋？", "开启小黑屋", "返回") != QMessageBox::NoButton)
            {
                return;
            }
        }
    }

    us->setVal("dr/word", word);
    us->setVal("dr/time", time);
    us->setVal("dr/combo", ct);
    us->setVal("dr/force", ft);

    /**
     * 注意：必须要先退出这个窗口，再：开启小黑屋>>全屏显示
     * 否则此窗口关闭的时机在全屏化之后
     * 结束全屏时，将无法进行任何鼠标操作，会有一个 stackWidget挡住整个程序
     */
    this->close();

    gd->dr.startDarkRoom(word, time, ct, ft);
}

/**
 * 添加计算结束后的建议，并把第一个建议加入输入框
 */
void DarkRoomWidget::addSuggestWordsList(QList<int> list)
{
     // qDebug() << "添加建议字数："  << list;
    if (!list.size())
        return;
    word_spin->setValue(list.first());
    list.removeFirst();

    int size = qMin(list.size(), 3);
    for (int i = 0; i < size; i++)
    {
        WaterFloatButton* btn = new WaterFloatButton(QString("%1字").arg(list.at(i)), this);
        btn->setBgColor(us->getOpacityColor(us->accent_color, 128)); // 设置背景之后就不进行画边框线条了
        btn->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);
        btn->setTextColor(us->global_font_color);
        connect(btn, &WaterFloatButton::clicked, [=]{
            word_spin->setValue(list.at(i));
            word_spin->setFocus();
            word_spin->selectAll();
        });
        suggest_hlayout->addWidget(btn);
    }
}
