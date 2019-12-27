#include "dirsettingswidget.h"

DirSettingsWidget::DirSettingsWidget(QWidget *parent) : QDialog(parent)
{
	ns = nullptr;
    setAttribute(Qt::WA_DeleteOnClose);

    initLayout();
}

void DirSettingsWidget::setNS(NovelDirSettings* ns)
{
	this->ns = ns;
}

void DirSettingsWidget::showEvent(QShowEvent* )
{
	initData();
}

void DirSettingsWidget::initLayout()
{
	QVBoxLayout* main_vlayout = new QVBoxLayout(this);
    QFont font("", 12);
    QRegExp re("-?\\d+");
    QRegExpValidator *pReg = new QRegExpValidator(re, this);

    setMinimumSize(300, 480);
	setWindowTitle(tr("目录设置"));

	// 书籍名称
    QHBoxLayout* novel_name_hlayout = new QHBoxLayout();
    // QLabel* novel_name_label = new QLabel(tr("作品名称"), this);
    novel_name_edit = new QLabel(this);
    // novel_name_hlayout->addWidget(novel_name_label);
    novel_name_hlayout->addWidget(novel_name_edit);
    // novel_name_label->setFont(font);
    novel_name_edit->setFont(font);
    novel_name_edit->setAlignment(Qt::AlignCenter);
    // novel_name_edit->setMaximumWidth(width()/2);

	// 起始章序号
    QHBoxLayout* start_chapter_hlayout = new QHBoxLayout();
    QLabel* start_chapter_label = new QLabel(tr("起始章序"), this);
    start_chapter_edit = new QLineEdit;
    start_chapter_hlayout->addWidget(start_chapter_label);
    start_chapter_hlayout->addWidget(start_chapter_edit);
    start_chapter_label->setFont(font);
    start_chapter_label->setToolTip("第一个章节的序号，默认为 1\n序号小于等于0的章节将不显示章序，可用来存放 楔子、序章 等");
    start_chapter_edit->setValidator(pReg);
    start_chapter_edit->setMaximumWidth(width()/5);
    connect(start_chapter_edit, &QLineEdit::textChanged, [=]{
        QString text = start_chapter_edit->text();
    	if (text.isEmpty()) return ;
    	if (canRegExp(text, "^-?\\d+$"))
	    	setVal("start_chapter", text);
    });

	// 起始卷序号
    QHBoxLayout* start_roll_hlayout = new QHBoxLayout();
    QLabel* start_roll_label = new QLabel(tr("起始卷序"), this);
    start_roll_edit = new QLineEdit;
    start_roll_hlayout->addWidget(start_roll_label);
    start_roll_hlayout->addWidget(start_roll_edit);
    start_roll_label->setFont(font);
    start_roll_label->setToolTip("第一个分卷的序号，默认为 1\n序号小于等于0的分卷将不显示卷序，可用来存放 额外的设定");
    start_roll_edit->setValidator(pReg);
    start_roll_edit->setMaximumWidth(width()/5);
    connect(start_roll_edit, &QLineEdit::textChanged, [=]{
        QString text = start_roll_edit->text();
    	if (text.isEmpty()) return ;
    	if (canRegExp(text, "^-?\\d+$"))
	    	setVal("start_roll", text);
    });


	// 章类名
    QHBoxLayout* chapter_char_hlayout = new QHBoxLayout();
    QLabel* chapter_char_label = new QLabel(tr("章节标记"), this);
    char_chapter_edit = new QLineEdit;
    chapter_char_hlayout->addWidget(chapter_char_label);
    chapter_char_hlayout->addWidget(char_chapter_edit);
    chapter_char_label->setFont(font);
    chapter_char_label->setToolTip("这决定了目录序号显示的是 第一章，还是第一回，又或者是第一帅");
    char_chapter_edit->setMaximumWidth(width()/5);
    connect(char_chapter_edit, &QLineEdit::textChanged, [=]{
        QString text = char_chapter_edit->text();
    	if (text.isEmpty()) return ;
    	if (canRegExp(text, "^\\S+$"))
	    	setVal("char_chapter", text);
    });

	// 卷类名
    QHBoxLayout* roll_char_hlayout = new QHBoxLayout();
    QLabel* roll_char_label = new QLabel(tr("分卷标记"), this);
    char_roll_edit = new QLineEdit;
    roll_char_hlayout->addWidget(roll_char_label);
    roll_char_hlayout->addWidget(char_roll_edit);
    roll_char_label->setFont(font);
    roll_char_label->setToolTip("同上，改成“帅”字，那“第一卷 正文”就变成“第一帅 正文”");
    char_roll_edit->setMaximumWidth(width()/5);
    connect(char_roll_edit, &QLineEdit::textChanged, [=]{
        QString text = char_roll_edit->text();
    	if (text.isEmpty()) return ;
    	if (canRegExp(text, "^\\S+$"))
	    	setVal("char_roll", text);
    });


	// 显示章序
    QHBoxLayout* show_chapter_num_hlayout = new QHBoxLayout();
    MyLabel* show_chapter_num_label = new MyLabel("显示章序", this);
    show_chapter_num_switch = new AniSwitch(false, this);
    show_chapter_num_hlayout->addWidget(show_chapter_num_label);
    show_chapter_num_hlayout->addWidget(show_chapter_num_switch);
    show_chapter_num_label->setFont(font);
    show_chapter_num_label->setToolTip("显示章节前面的“第x章”");
    show_chapter_num_switch->setMaximumWidth(width()/5);
    show_chapter_num_switch->setResponse();
    connect(show_chapter_num_label, &MyLabel::clicked, [=]{
    	show_chapter_num_switch->slotSwitch();
    	setVal("show_chapter_num", show_chapter_num_switch->isOn());
    });
    connect(show_chapter_num_switch, &AniSwitch::signalSwitch, [=]{
        setVal("show_chapter_num", show_chapter_num_switch->isOn());
    });


	// 显示卷序
    QHBoxLayout* show_roll_num_hlayout = new QHBoxLayout();
    MyLabel* show_roll_num_label = new MyLabel("显示卷序", this);
    show_roll_num_switch = new AniSwitch(false, this);
    show_roll_num_hlayout->addWidget(show_roll_num_label);
    show_roll_num_hlayout->addWidget(show_roll_num_switch);
    show_roll_num_label->setFont(font);
    show_roll_num_label->setToolTip("显示分卷前面的“第x卷”");
    show_roll_num_switch->setMaximumWidth(width()/5);
    show_roll_num_switch->setResponse();
    connect(show_roll_num_label, &MyLabel::clicked, [=]{
    	show_roll_num_switch->slotSwitch();
    	setVal("show_roll_num", show_roll_num_switch->isOn());
    });
    connect(show_roll_num_switch, &AniSwitch::signalSwitch, [=]{
        setVal("show_roll_num", show_roll_num_switch->isOn());
    });


	// 显示章节字数（影响加载速度）
    QHBoxLayout* show_chapter_words_hlayout = new QHBoxLayout();
    MyLabel* show_chapter_words_label = new MyLabel("显示章节字数 [未开发]", this);
    show_chapter_words_switch = new AniSwitch(false, this);
    show_chapter_words_hlayout->addWidget(show_chapter_words_label);
    show_chapter_words_hlayout->addWidget(show_chapter_words_switch);
    show_chapter_words_label->setFont(font);
    show_chapter_words_label->setToolTip("在每一章节的后面显示章节字数 [暂时不支持]");
    show_chapter_words_switch->setMaximumWidth(width()/5);
    show_chapter_words_switch->setResponse();
    connect(show_chapter_words_label, &MyLabel::clicked, [=]{
    	show_chapter_words_switch->slotSwitch();
    	setVal("show_chapter_words", show_chapter_words_switch->isOn());
    });
    connect(show_chapter_words_switch, &AniSwitch::signalSwitch, [=]{
        setVal("show_chapter_words", show_chapter_words_switch->isOn());
    });


    /*// 鼠标悬浮预览
    QHBoxLayout* chapter_preview_hlayout = new QHBoxLayout();
    MyLabel* chapter_preview_label = new MyLabel("鼠标悬浮预览章节", this);
    chapter_preview_switch = new AniSwitch(false, this);
    chapter_preview_hlayout->addWidget(chapter_preview_label);
    chapter_preview_hlayout->addWidget(chapter_preview_switch);
    chapter_preview_label->setFont(font);
    chapter_preview_label->setToolTip("目录章节处悬浮鼠标进行预览（前100字）");
    chapter_preview_switch->setMaximumWidth(width()/5);
    chapter_preview_switch->setResponse();
    connect(chapter_preview_label, &MyLabel::clicked, [=]{
        chapter_preview_switch->slotSwitch();
        setVal("chapter_preview", chapter_preview_switch->isOn());
        us->setVal("us/chapter_preview", us->chapter_preview = chapter_preview_switch->isOn());
    });
    connect(chapter_preview_switch, &AniSwitch::signalSwitch, [=]{
        setVal("chapter_preview", chapter_preview_switch->isOn());
        us->setVal("us/chapter_preview", us->chapter_preview = chapter_preview_switch->isOn());
    });*/


	// 使用阿利伯数字（否则中文）
    QHBoxLayout* use_arab_hlayout = new QHBoxLayout();
    MyLabel* use_arab_label = new MyLabel("使用阿拉伯数字", this);
    use_arab_switch = new AniSwitch(false, this);
    use_arab_hlayout->addWidget(use_arab_label);
    use_arab_hlayout->addWidget(use_arab_switch);
    use_arab_label->setFont(font);
    use_arab_label->setToolTip("“第一章”还是“第1章”，任您选");
    use_arab_switch->setMaximumWidth(width()/5);
    use_arab_switch->setResponse();
    connect(use_arab_label, &MyLabel::clicked, [=]{
    	use_arab_switch->slotSwitch();
    	setVal("use_arab", use_arab_switch->isOn());
    });
    connect(use_arab_switch, &AniSwitch::signalSwitch, [=]{
        setVal("use_arab", use_arab_switch->isOn());
    });


	// 每卷重计章序
    QHBoxLayout* recount_num_by_roll_hlayout = new QHBoxLayout();
    MyLabel* recount_num_by_roll_label = new MyLabel("每卷重计章序", this);
    recount_num_by_roll_switch = new AniSwitch(false, this);
    recount_num_by_roll_hlayout->addWidget(recount_num_by_roll_label);
    recount_num_by_roll_hlayout->addWidget(recount_num_by_roll_switch);
    recount_num_by_roll_label->setFont(font);
    recount_num_by_roll_label->setToolTip("下一卷重新从起始章序开始计算，而不延续上一卷\n用来撰写多部作品时使用");
    recount_num_by_roll_switch->setMaximumWidth(width()/5);
    recount_num_by_roll_switch->setResponse();
    connect(recount_num_by_roll_label, &MyLabel::clicked, [=]{
    	recount_num_by_roll_switch->slotSwitch();
    	setVal("recount_num_by_roll", recount_num_by_roll_switch->isOn());
    });
    connect(recount_num_by_roll_switch, &AniSwitch::signalSwitch, [=]{
        setVal("recount_num_by_roll", recount_num_by_roll_switch->isOn());
    });


	// 不使用卷（即正文靠正左对齐）
    QHBoxLayout* no_roll_hlayout = new QHBoxLayout();
    MyLabel* no_roll_label = new MyLabel("不使用分卷", this);
    no_roll_switch = new AniSwitch(false, this);
    no_roll_hlayout->addWidget(no_roll_label);
    no_roll_hlayout->addWidget(no_roll_switch);
    no_roll_label->setFont(font);
    no_roll_label->setToolTip("删除章节标题左边的空白缩进，与卷对齐");
    no_roll_switch->setMaximumWidth(width()/5);
    no_roll_switch->setResponse();
    connect(no_roll_label, &MyLabel::clicked, [=]{
    	no_roll_switch->slotSwitch();
    	setVal("no_roll", no_roll_switch->isOn());
    });
    connect(no_roll_switch, &AniSwitch::signalSwitch, [=]{
        setVal("no_roll", no_roll_switch->isOn());
    });

    main_vlayout->addLayout(novel_name_hlayout);
    main_vlayout->addLayout(start_chapter_hlayout);
    main_vlayout->addLayout(start_roll_hlayout);
    main_vlayout->addLayout(chapter_char_hlayout);
    main_vlayout->addLayout(roll_char_hlayout);
    main_vlayout->addLayout(show_chapter_num_hlayout);
    main_vlayout->addLayout(show_roll_num_hlayout);
    main_vlayout->addLayout(show_chapter_words_hlayout);
//    main_vlayout->addLayout(chapter_preview_hlayout);
    main_vlayout->addLayout(use_arab_hlayout);
    main_vlayout->addLayout(recount_num_by_roll_hlayout);
    main_vlayout->addLayout(no_roll_hlayout);

    main_vlayout->setMargin(20);
    main_vlayout->setSpacing(20);
    main_vlayout->setAlignment(Qt::AlignTop);
}

void DirSettingsWidget::initData()
{
	Settings* s = ns->getSettings();

	novel_name = ns->getNovelName();
	if (novel_name.startsWith("《") && novel_name.endsWith("》"))
        novel_name_edit->setText(ns->getNovelName());
	else
        novel_name_edit->setText(QString("《%1》").arg(novel_name));
    start_chapter_edit->setText(QString("%1").arg(s->getInt("start_chapter", 1)));
    start_roll_edit->setText(QString ("%1").arg(s->getInt("start_roll", 1)));
    char_chapter_edit->setText(s->getStr("char_chapter", "章"));
    char_roll_edit->setText(s->getStr("char_roll", "卷"));
    show_chapter_num_switch->setOnOff(s->getBool("show_chapter_num", true));
    show_roll_num_switch->setOnOff(s->getBool("show_roll_num", true));
    show_chapter_words_switch->setOnOff(s->getBool("show_chapter_words", false));
//    chapter_preview_switch->setOnOff(s->getBool("chapter_preview", false));
    use_arab_switch->setOnOff(s->getBool("use_arab_switch", false));
    recount_num_by_roll_switch->setOnOff(s->getBool("recount_num_by_roll", false));
    no_roll_switch->setOnOff(s->getBool("no_roll", false));
}

void DirSettingsWidget::setVal(QString key, QVariant val)
{
	if (ns == nullptr)
		return ;
    ns->setVal(key, val);

}

void DirSettingsWidget::toShow(QString n, QPoint p)
{
    Q_UNUSED(n);
    Q_UNUSED(p);
}

void DirSettingsWidget::toHide()
{

}
