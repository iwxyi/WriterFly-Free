#include "exportpage.h"

ExportPage::ExportPage(QWidget *parent) : QWidget(parent)
{
	initLayout();
	initData();
}

void ExportPage::initLayout()
{
	QVBoxLayout* main_vlayout = new QVBoxLayout(this);
    QFont font("", 12);

    // 导出作品相关
    QHBoxLayout* about_hlayout = new QHBoxLayout();
    MyLabel* about_label = new MyLabel("导出作品相关", this);
    AniSwitch* about_switch = new AniSwitch(false, this);
    about_hlayout->addWidget(about_label);
    about_hlayout->addWidget(about_switch);
    about_label->setFont(font);
    about_label->setToolTip("开启后，将在全文最开始的地方显示作品相关，以及作品相关里面的内容\n注意：作品相关没有章节序号，无法导入回来");
    about_switch->setMaximumWidth(width()/3);
    about_switch->setResponse();
    connect(about_label, &MyLabel::clicked, [=]{
    	about_switch->slotSwitch();
    	us->setVal("export/about", about_switch->isOn());
    });
    connect(about_switch, &AniSwitch::signalSwitch, [=]{
        us->setVal("export/about", about_switch->isOn());
    });

    // 每卷第一章上方显示卷名
    QHBoxLayout* split_hlayout = new QHBoxLayout();
    MyLabel* split_label = new MyLabel("每卷第一章上方显示卷名", this);
    AniSwitch* split_switch = new AniSwitch(true, this);
    split_hlayout->addWidget(split_label);
    split_hlayout->addWidget(split_switch);
    split_label->setFont(font);
    split_label->setToolTip("开启后，每卷第一章上方会显示这一卷的卷名（根据目录设置选择带不带卷序）");
    split_switch->setMaximumWidth(width()/3);
    split_switch->setResponse();
    connect(split_label, &MyLabel::clicked, [=]{
    	split_switch->slotSwitch();
    	us->setVal("export/split", split_switch->isOn());
    });
    connect(split_switch, &AniSwitch::signalSwitch, [=]{
        us->setVal("export/split", split_switch->isOn());
    });

    // 章名前添加卷名
    QHBoxLayout* roll_hlayout = new QHBoxLayout();
    MyLabel* roll_label = new MyLabel("章名前添加卷名", this);
    AniSwitch* roll_switch = new AniSwitch(false, this);
    roll_hlayout->addWidget(roll_label);
    roll_hlayout->addWidget(roll_switch);
    roll_label->setFont(font);
    roll_label->setToolTip("每一章前面都带有这一卷的卷名（根据目录设置选择带不带卷序/章序）");
    roll_switch->setMaximumWidth(width()/3);
    roll_switch->setResponse();
    connect(roll_label, &MyLabel::clicked, [=]{
    	roll_switch->slotSwitch();
    	us->setVal("export/roll", roll_switch->isOn());
    });
    connect(roll_switch, &AniSwitch::signalSwitch, [=]{
        us->setVal("export/roll", roll_switch->isOn());
    });

    // 显示设备（换行符）
    QHBoxLayout* newline_hlayout = new QHBoxLayout();
    MyLabel* newline_label = new MyLabel("显示平台", this);
    QComboBox* newline_combo = new QComboBox(this);
    newline_hlayout->addWidget(newline_label);
    newline_hlayout->addWidget(newline_combo);
    QStringList newline_codecs{"自动", "Window", "Mac", "Unix"};
    newline_combo->addItems(newline_codecs);
#if defined(Q_OS_WIN)
//    newline_combo->setCurrentIndex(0);
#elif defined(Q_OS_MAC)
//    newline_combo->setCurrentIndex(1);
#elif defined(Q_OS_UNIX)
//    newline_combo->setCurrentIndex(2);
#else
//    newline_combo->setCurrenIndex(0);
#endif
    newline_combo->setCurrentIndex(0);
    newline_label->setToolTip("不同平台换行符处理方式不同（自动即可）");
    connect(newline_combo, &QComboBox::currentTextChanged, [=]{
        us->setVal("export/newline", newline_combo->currentIndex());
    });

    // 按钮
    submit_button = new QPushButton("导出", this);
    submit_button->setFont(font);
    submit_button->setIcon(QIcon(QPixmap(":/icons/export")));
    QString trans = "QPushButton{border-width:1px;border-style:solid;border-color:rgba(128,128,128,80); background-color: transparent; padding:5px;}";
    submit_button->setStyleSheet(trans);
    connect(submit_button, &QPushButton::clicked, [=]{
    	emit signalExport();
    });

    QLabel* tip_label = new QLabel("可在【目录设置】中调整 章名/卷名 的显示方式", this);

    main_vlayout->addLayout(about_hlayout);
    main_vlayout->addLayout(split_hlayout);
    main_vlayout->addLayout(roll_hlayout);
    main_vlayout->addLayout(newline_hlayout);
    main_vlayout->addWidget(tip_label);
    main_vlayout->addWidget(submit_button);

    main_vlayout->setMargin(20);
    main_vlayout->setSpacing(20);
    main_vlayout->setAlignment(Qt::AlignTop);
	setLayout(main_vlayout);
}

void ExportPage::initData()
{
    if (submit_button != nullptr)
        submit_button->setText("导出");
}

void ExportPage::slotExportFinished()
{
	submit_button->setText("导出完成");
}

void ExportPage::showEvent(QShowEvent* )
{
	initData();
}
