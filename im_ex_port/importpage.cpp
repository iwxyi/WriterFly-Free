#include "importpage.h"

ImportPage::ImportPage(QWidget *parent) : QWidget(parent)
{
    listview = NULL;
    initLayout();
    initData();
}

void ImportPage::initLayout()
{
	QVBoxLayout* main_vlayout = new QVBoxLayout(this);
    QFont font("", 12);

    // 先选择路径
    path_button = new QPushButton("点此选择作品路径", this);
    QString trans = "QPushButton{border-width:1px;border-style:solid;border-color:rgba(128,128,128,80); background-color: transparent; padding:5px}";
    path_button->setStyleSheet(trans);
    path_button->setFont(font);

    // 导入到新书还是当前作品
    QHBoxLayout* target_hlayout = new QHBoxLayout();
    MyLabel* target_label = new MyLabel("导入至", this);
    target_combo = new QComboBox(this);
    target_hlayout->addWidget(target_label);
    target_hlayout->addWidget(target_combo);
    resetTargetCombo();
    target_label->setFont(font);
    target_label->setToolTip("是否导入到一个新的作品中（自动创建）");

    // 分卷导入
    QHBoxLayout* split_hlayout = new QHBoxLayout();
    MyLabel* split_label = new MyLabel("分卷导入", this);
    AniSwitch* split_switch = new AniSwitch(auto_split_roll = us->getBool("import/auto_split_roll", true), this);
    split_hlayout->addWidget(split_label);
    split_hlayout->addWidget(split_switch);
    split_switch->setMaximumWidth(width()/3);
    split_switch->setResponse();
    split_label->setFont(font);
    split_label->setToolTip("是否将导入的小说进行分卷（需要单行分卷标记，否则当做一卷）");

    // 当前作品的话，导入到哪一卷
    QHBoxLayout* roll_hlayout = new QHBoxLayout();
    MyLabel* roll_label = new MyLabel("开始导入的卷", this);
    roll_combo = new QComboBox(this);
    roll_hlayout->addWidget(roll_label);
    roll_hlayout->addWidget(roll_combo);
    resetRollCombo();
    roll_label->setFont(font);
    roll_label->setToolTip("从哪一卷开始导入");

    // 编码
    QHBoxLayout* codec_hlayout = new QHBoxLayout();
    MyLabel* codec_label = new MyLabel("读入编码", this);
    codec_combo = new QComboBox(this);
    codec_hlayout->addWidget(codec_label);
    codec_hlayout->addWidget(codec_combo);
    codec = us->getStr("import/codec", "GBK"); // 小在小说用GBK的比较多
    QStringList codecs{"GBK", "ANSI", "UTF-8", "UTF-8-BOM", "UCS-2 Big Endian", "UCS-2 Little Endian"};
    codec_combo->addItems(codecs);
    int codec_index = 0;
    for (int i = 0; i < codec_combo->count(); i++)
        if (codec == codecs.at(i))
            codec_index = i;
    codec_combo->setCurrentIndex(codec_index);
    codec_label->setToolTip("如果编码不对，将乱码，导致读入为空");

    // 卷名正则
    QHBoxLayout* roll_rx_hlayout = new QHBoxLayout();
    QLabel* roll_rx_label = new QLabel("卷名表达式", this);
    QLineEdit* roll_rx_edit = new QLineEdit(roll_reg = us->getStr("import/roll_regexp", "\\n\\s*(第\\S{1,5}卷) +([^\\n]+)\\n"));
    roll_rx_hlayout->addWidget(roll_rx_label);
    roll_rx_hlayout->addWidget(roll_rx_edit);
    roll_rx_label->setToolTip("匹配一行卷名的正则表达式（不懂请勿修改）\n两对圆括号分别包裹卷序、卷名\n若不自动分卷，将使用此表达式清除卷名所在行");

    // 章名正则
    QHBoxLayout* chpt_rx_hlayout = new QHBoxLayout();
    QLabel* chpt_rx_label = new QLabel("章名表达式", this);
    QLineEdit* chpt_rx_edit = new QLineEdit(chpt_reg = us->getStr("import/chpt_regexp", "\\n(?:正文)?\\s*(第\\S{1,5}章) +([^\\n]+)\\n"));
    chpt_rx_hlayout->addWidget(chpt_rx_label);
    chpt_rx_hlayout->addWidget(chpt_rx_edit);
    chpt_rx_label->setToolTip("匹配一行章名的正则表达式（不懂请勿修改）\n两对圆括号分别包裹章序、章名");

    // 按钮
    submit_button = new QPushButton("导入", this);
    submit_button->setFont(font);
    submit_button->setIcon(QIcon(QPixmap(":/icons/import")));
    submit_button->setStyleSheet(trans);
    submit_button->setStyleSheet("padding:10px;");

    main_vlayout->addWidget(path_button);
    main_vlayout->addLayout(target_hlayout);
    main_vlayout->addLayout(split_hlayout);
    main_vlayout->addLayout(roll_hlayout);
    main_vlayout->addLayout(codec_hlayout);
    main_vlayout->addLayout(roll_rx_hlayout);
    main_vlayout->addLayout(chpt_rx_hlayout);
    main_vlayout->addWidget(submit_button);

    main_vlayout->setMargin(20);
    main_vlayout->setSpacing(20);
    main_vlayout->setAlignment(Qt::AlignTop);
    setLayout(main_vlayout);


	// === 初始化信号槽连接 ====

	// 选择路径后，初始化
    connect(path_button, &QPushButton::clicked, [=]{
		QString recent = us->getStr("recent/file_path");
	    if (recent == "" || !isFileExist(recent))
	        recent = ".";
	    recent = getDirByFile(recent);
        file_path = QFileDialog::getOpenFileName(this, "选择导入的纯文本小说", recent, tr("Text files (*.txt)"));
        if (file_path.length() == 0) return ;
        us->setVal("recent/file_path", file_path);
	    QFileInfo file(file_path);
	    file_name = file.baseName();
	    resetTargetCombo();

        path_button->setText(file_name);
        submit_button->setText("导入");
	});

    // 选择导入到当前作品还是新建作品
    connect(target_combo, &QComboBox::currentTextChanged, [=]{
        if ( target_combo->currentIndex() < target_combo->count()-1) // 当前作品
            add_new_novel = false;
        else // 新建作品
            add_new_novel = true;
        resetRollCombo();
    });

	// 自动分卷
    connect(split_label, &MyLabel::clicked, [=]{
		split_switch->slotSwitch();
		auto_split_roll = split_switch->isOn();
		us->setVal("import/auto_split_roll", auto_split_roll);
		resetRollCombo();
	});
    connect(split_switch, &AniSwitch::signalSwitch, [=]{
		auto_split_roll = split_switch->isOn();
		us->setVal("import/auto_split_roll", auto_split_roll);
		resetRollCombo();
	});

	// 选择卷
	// 无

    // 选择编码
    connect(codec_combo, &QComboBox::currentTextChanged, [=]{
        codec = codec_combo->currentText();
        us->setVal("import/codec", codec);
    });

	// 修改正则
    connect(roll_rx_edit, &QLineEdit::textChanged, [=]{
        roll_reg = roll_rx_edit->text();
		us->setVal("import/roll_regexp", roll_reg);
	});
    connect(chpt_rx_edit, &QLineEdit::textChanged, [=]{
        chpt_reg = chpt_rx_edit->text();
        us->setVal("import/chpt_regexp", chpt_reg);
	});

	// 开始导入
   connect(submit_button, SIGNAL(clicked()), this, SLOT(slotStartImport()));
}

void ImportPage::initData()
{
	file_path = file_name = "";
    if (path_button != nullptr)
		path_button->setText("点击选择作品路径");
    if (submit_button != nullptr)
		submit_button->setText("导入");
	resetTargetCombo();
}

void ImportPage::setNovelName(QString name)
{
	novel_name = name;
}

QString ImportPage::getTargetName()
{
	return target_name;
}

void ImportPage::setNovelDirListView(NovelDirListView* listview)
{
	this->listview = listview;
}

void ImportPage::slotImportFinished(QString name)
{
    Q_UNUSED(name);
	submit_button->setText("导入成功");
}

void ImportPage::showEvent(QShowEvent* )
{
	initData();

    if (!us->getStr("import/target_path").isEmpty())
    {
        QFileInfo file(us->getStr("import/target_path"));
        file_name = file.baseName();
        resetTargetCombo();

        path_button->setText(file_name);
        submit_button->setText("导入");
        us->setVal("import/target_path", "");
    }
}

void ImportPage::resetTargetCombo()
{
	target_combo->clear();
	if (!novel_name.isEmpty())
    	target_combo->addItem(QString("当前作品(%1)").arg(novel_name));

    target_name = "自动";
    if (!file_name.isEmpty())
    {
    	if (isFileExist(rt->NOVEL_PATH+file_name))
        {
    		int index = 1;
    		while (isFileExist(rt->NOVEL_PATH+file_name+QString("(%1)").arg(index)))
    			index++;
    		target_path = rt->NOVEL_PATH+file_name+QString("(%1)").arg(index);
            target_name = file_name+QString("(%1)").arg(index);
    	}
    	else
    	{
    		target_path = rt->NOVEL_PATH+file_name;
    		target_name = file_name;
    	}
	}
    target_combo->addItem(QString("新建作品(%1)").arg(target_name));

    add_new_novel = false;
    if (file_name == novel_name)
    {
    	target_combo->setCurrentIndex(0);
    }
    else if (target_combo->count() > 1)
    {
    	target_combo->setCurrentIndex(1);
        add_new_novel = true;
    }
}

void ImportPage::resetRollCombo()
{
	roll_combo->clear();
    if (add_new_novel)
    {
        roll_combo->addItem("自动分卷");
        return ;
    }
    if (listview == nullptr)
    {
    	roll_combo->addItem("自动分卷(创建新卷)");
    	return ;
	}

    QStringList list = listview->getModel()->index(0, 0).data(Qt::UserRole+DRole_ROLL_NAMES).toStringList();
    bool last_is_blank = (listview->getModel()->getRcCountByRoll(list.size()-1) == 0); // 最后一章是否是空的
    if (list.size() == 0)
    {
    	roll_combo->addItem("无法获取分卷");
    	return ;
    }

	if (auto_split_roll) // 分卷
	{
        roll_combo->addItem(QString("最后一卷(%1)").arg(list.last()));
		roll_combo->addItem(QString("新卷（自动创建）"));
	}
	else // 不分卷
    {
        roll_combo->addItems(list);
        roll_combo->addItem("新卷（自动创建）");
    }
    if (last_is_blank) // 最后一卷是空的，说明准备导入
        roll_combo->setCurrentIndex(roll_combo->count()-2);
    else
        roll_combo->setCurrentIndex(roll_combo->count()-1);
}

void ImportPage::resetCodecCombo()
{

}

void ImportPage::slotStartImport()
{
    if (file_name.isEmpty() || roll_reg.trimmed().isEmpty() || chpt_reg.trimmed().isEmpty()) return ;

	// ==== 初始化字符串变量 ====

	bool keep_cr_number = false;

	us->setVal("import/add_new_novel", add_new_novel);
	us->setVal("import/auto_split_roll", auto_split_roll);
    us->setVal("import/roll_regexp", roll_reg);
    us->setVal("import/chpt_regexp", chpt_reg);

	if (!add_new_novel) // 保存到当前作品，则交给 listview 处理
	{
		emit signalImport(file_path, auto_split_roll, roll_combo->currentIndex());
		return ;
	}

    QString novel_name = target_name;              // 小说名字（和全局变量冲突了，但好像问题不大）
    QString novel_path = rt->NOVEL_PATH+novel_name; // 小说文件夹路径
    QString dir_path = novel_path+"/"+NOVELDIRFILENAME;  // 小说目录
    QString chapter_path = novel_path+"/chapters/";      // 章节目录

	// ==== 创建小说目录 ====

	if (isFileExist(novel_path))
	{
		QMessageBox::information(this, tr("导入失败"), QString("作品《%1》的存储路径已经存在：%2").arg(novel_name).arg(novel_path));
		return ;
	}
    if (!canBeFileName(novel_name))
	{
		QMessageBox::information(this, tr("导入失败"), QString("作品《%1》的包含特殊字符").arg(novel_name));
		return ;
	}
	ensureDirExist(novel_path);
	ensureDirExist(novel_path+"/chapters");
	ensureDirExist(novel_path+"/details");
	ensureDirExist(novel_path+"/outlines");
	ensureDirExist(novel_path+"/recycles");


	// ==== 开始导入到新书 ====

    QString total_content = readTextFile(file_path, codec); // 导入的内容（全部）
	QString dir_text = "<BOOK>"; // 新书的目录

	QRegExp roll_rx(roll_reg);
	QRegExp chpt_rx(chpt_reg);

	if (auto_split_roll) // 分卷
	{
		int roll_count = 0, chpt_count = 0;
		// 不导入作品相关，默认生成一个作品相关卷
		dir_text += "<ROLL><RINF><n:作品相关></RINF><LIST></LIST></ROLL>";

		// 遍历每一卷

		int roll_pos = 0;
		while ( 1 )
		{
			// 当前卷的起始位置
			roll_pos = roll_rx.indexIn(total_content, roll_pos);
			if (roll_pos == -1) break;
			roll_pos += roll_rx.matchedLength();

			// 目录内容加上卷名
			QString roll_numb_str = roll_rx.cap(1); // 卷序号
			QString roll_name_str = roll_rx.cap(2); // 卷名字
			if (keep_cr_number)
			{
				dir_text += QString("<ROLL><RINF><n:%1 %2></RINF><LIST>").arg(roll_numb_str).arg(fnEncode(roll_name_str));
			}
			else
			{
				dir_text += QString("<ROLL><RINF><n:%1></RINF><LIST>").arg(fnEncode(roll_name_str));
			}

			// 下一卷的位置
			int next_roll_pos = roll_rx.indexIn(total_content, roll_pos);
			if (next_roll_pos == -1)
				next_roll_pos = total_content.length();

			// 解析章节
			// 从 roll_pos ~ nextroll_pos 中间的都是章节
			QString roll_content = total_content.mid(roll_pos, next_roll_pos-roll_pos);

			int chpt_pos = 0;
			while ( 1 )
			{
				// 这一章名的位置
				chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
				if (chpt_pos == -1) break;
				chpt_pos += chpt_rx.matchedLength();

				// 目录加上章节的名字
				QString chpt_numb_str = chpt_rx.cap(1);
				QString chpt_name_str = chpt_rx.cap(2);
				chpt_name_str = fnEncode(chpt_name_str);
				// 判断章名同名情况（本书同名）
				if (dir_text.indexOf(QString("<n:%1>").arg(chpt_name_str)) > -1)
	            {
	                int index = 1;
	                while (dir_text.indexOf(QString("<n:%1(%2)>").arg(chpt_name_str).arg(index)) > -1)
	                    index++;
	                chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
	            }
				if (keep_cr_number)
				{
					dir_text += QString("<CHPT><n:%1 %2></CHPT>").arg(chpt_numb_str).arg(chpt_name_str);
				}
				else
				{
					dir_text += QString("<CHPT><n:%1></CHPT>").arg(chpt_name_str);
				}

				// 下一章名的位置
				int next_chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
				if (next_chpt_pos == -1)
					next_chpt_pos = roll_content.length();

				// 获取章节正文
				QString chpt_content = roll_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
				chpt_content = simplifyChapter(chpt_content);
				QString chpt_file_path = chapter_path + chpt_name_str + ".txt";
				writeTextFile(chpt_file_path, chpt_content);

				chpt_count++;
			}

			dir_text += "</LIST></ROLL>";

			roll_count++;
		}

		dir_text += "</BOOK>";
		writeTextFile(dir_path, dir_text);

		QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n卷数：%2\n章数：%3").arg(target_name).arg(roll_count).arg(chpt_count));
	}
	else // 不分卷
	{
		// ==== 清除卷名行 ====
		int roll_pos = 0;
        while ( (roll_pos=roll_rx.indexIn(total_content, roll_pos)) != -1 )
            total_content = total_content.left(roll_pos) + total_content.right(total_content.length()-roll_pos-roll_rx.matchedLength());

		int chpt_count = 0;
		// 自动生成一个作品相关
		dir_text += "<ROLL><RINF><n:作品相关></RINF><LIST></LIST></ROLL>";
		// 自动生成正文卷
		dir_text += "<ROLL><RINF><n:正文></RINF><LIST>";

		// ==== 开始导入 ====
		int chpt_pos = 0;
		while ( 1 )
		{
			// 章名位置
			chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
			if (chpt_pos == -1) break;
			chpt_pos += chpt_rx.matchedLength();

			QString chpt_numb_str = chpt_rx.cap(1);
			QString chpt_name_str = chpt_rx.cap(2);
			chpt_name_str = fnEncode(chpt_name_str);
			// 判断章名同名情况（本书同名）
			if (dir_text.indexOf(QString("<n:%1>").arg(chpt_name_str)) > -1)
            {
                int index = 1;
                while (dir_text.indexOf(QString("<n:%1(%2)>").arg(chpt_name_str).arg(index)) > -1)
                    index++;
                chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
            }
			if (keep_cr_number)
			{
				dir_text += QString("<CHPT><n:%1 %2></CHPT>").arg(chpt_numb_str).arg(chpt_name_str);
			}
			else
			{
				dir_text += QString("<CHPT><n:%1></CHPT>").arg(chpt_name_str);
			}

			// 下一章名的位置
			int next_chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
			if (next_chpt_pos == -1)
                next_chpt_pos = total_content.length();

			// 获取章节正文
			QString chpt_content = total_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
			chpt_content = simplifyChapter(chpt_content);
			QString chpt_file_path = chapter_path + chpt_name_str + ".txt";
			writeTextFile(chpt_file_path, chpt_content);

			chpt_count++;
		}

		dir_text += "</LIST></ROLL>"; // 闭合正文卷标签
		dir_text += "</BOOK>";        // 闭合全书标签
		writeTextFile(dir_path, dir_text);

		QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n章数：%2").arg(target_name).arg(chpt_count));
	}


	// ==== 导入结束，保存变量并且传递信号 ====

	us->setVal("recent/novel", novel_name);
	us->setVal("recent/chapter", "");
	us->setVal("recent/fullChapterName", "");
	us->setVal("recent/index", -1);
	us->setVal("recent/roll_index", -1);
	us->setVal("recent/chapter_index", -1);

	emit signalImportFinished(novel_name);
	submit_button->setText("导入完成");

	file_path = file_name = "";
}

/**
 * 精简章节，去掉首尾多余的空行
 * 如果可以，再进行排版
 * @param  chpt 未精简的章节
 * @return      精简过的章节
 */
QString ImportPage::simplifyChapter(QString chpt)
{
	int pos = 0, len = chpt.length();

	// 去除前面的空白
    while (pos < len && isBlankChar(chpt.mid(pos, 1)))
		pos++;
	while (pos > 0 && chpt.mid(pos-1, 1) != "\n")
		pos--;
	int start_pos = pos;

	pos = len;
	while (pos > 0 && isBlankChar(chpt.mid(pos-1, 1)))
		pos--;
	while (pos < len && chpt.mid(pos, 1) != "\n")
		pos++;
	int end_pos = pos;

	return chpt.mid(start_pos, end_pos-start_pos);
}

bool ImportPage::isBlankChar(QString c)
{
	if (c == " " || c == "\n" || c == "　" || c == "\t")
		return true;
	return false;
}
