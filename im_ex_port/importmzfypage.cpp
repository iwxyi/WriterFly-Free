#include "importmzfypage.h"

ImportMzfyPage::ImportMzfyPage(QWidget *parent) : QWidget(parent)
{
	initLayout();
	initData();
}

void ImportMzfyPage::initLayout()
{
	QVBoxLayout* main_vlayout = new QVBoxLayout(this);
    QFont font("", 12);

	// 先选择路径
    path_button = new QPushButton("点此选择作品路径", this);
    QString trans = "QPushButton{border-width:1px;border-style:solid;border-color:rgba(128,128,128,80); background-color: transparent; padding:5px}";
    path_button->setStyleSheet(trans);
    path_button->setFont(font);

    // 导入目录设置
    QHBoxLayout* dir_st_hlayout = new QHBoxLayout();
    MyLabel* dir_st_label = new MyLabel("导入目录设置", this);
    AniSwitch* dir_st_switch = new AniSwitch(import_dir_settings = us->getBool("import/import_dir_settings", true), this);
    dir_st_hlayout->addWidget(dir_st_label);
    dir_st_hlayout->addWidget(dir_st_switch);
    dir_st_switch->setMaximumWidth(width()/3);
    dir_st_switch->setResponse();
    dir_st_label->setFont(font);
    dir_st_label->setToolTip("（这个选项暂时没什么用……目录设置还没做呢）\n是否导入目录设置（例如：起始卷、起始章）");

    // 按钮
    submit_button = new QPushButton("导入", this);
    submit_button->setFont(font);
    submit_button->setIcon(QIcon(QPixmap(":/icons/import")));
    submit_button->setStyleSheet(trans);
    submit_button->setStyleSheet("padding:10px;");
    submit_button->setToolTip("仅导入目录、正文，暂不包括：大纲、细纲、名片库、设置等");

    main_vlayout->addWidget(path_button);
    main_vlayout->addLayout(dir_st_hlayout);
    main_vlayout->addWidget(submit_button);

    main_vlayout->setMargin(20);
    main_vlayout->setSpacing(20);
    main_vlayout->setAlignment(Qt::AlignTop);
    setLayout(main_vlayout);

    // ==== 选择路径后，初始化 ====
    connect(path_button, &QPushButton::clicked, [=]{
		QString recent = us->getStr("recent/file_path");
	    if (recent == "" || !isFileExist(recent))
	        recent = ".";
	    recent = getDirByFile(recent);
        //file_path = QFileDialog::getOpenFileName(this, "选择码字风云备份包（碼字風雲备份.zip）", recent, tr("码字风云备份包 (*.zip)"));
        file_path = QFileDialog::getExistingDirectory(this, "选择码字风云数据文件夹（手机存储卡/碼字風雲/）", recent);
        if (file_path.length() == 0) return ;
        us->setVal("recent/file_path", file_path);
	    QFileInfo file(file_path);
	    QString file_name = file.baseName();

        path_button->setText(file_name);
        submit_button->setText("导入");
	});
	// 导入设置
    connect(dir_st_label, &MyLabel::clicked, [=]{
		dir_st_switch->slotSwitch();
		import_dir_settings = dir_st_switch->isOn();
		us->setVal("import/import_dir_settings", import_dir_settings);
	});
    connect(dir_st_switch, &AniSwitch::signalSwitch, [=]{
		import_dir_settings = dir_st_switch->isOn();
		us->setVal("import/import_dir_settings", import_dir_settings);
	});

	// 开始导入
   connect(submit_button, SIGNAL(clicked()), this, SLOT(slotStartImport()));
}

void ImportMzfyPage::initData()
{
	file_path = "";
    if (path_button != nullptr)
		path_button->setText("选择码字风云文件夹");
    if (submit_button != nullptr)
    	submit_button->setText("导入");
}

void ImportMzfyPage::showEvent(QShowEvent* )
{
	initData();
}

void ImportMzfyPage::slotStartImport()
{
	if (file_path.isEmpty())
		return ;

	// ==== 获取 /碼字風雲/.设置 目录 ====
	QString mzfy_path = file_path;
	// deleteFile(DataPath+"temp");
	// ensureDirExist(DataPath+"temp");
	// QCUnzip(file_path, DataPath+"temp"); // 解压文件（无法使用）

	if (isFileExist(mzfy_path+"/.设置"))
		mzfy_path = mzfy_path+"/.设置";
	else if (isFileExist(mzfy_path+".设置"))
		mzfy_path = mzfy_path+".设置";
	else if (isFileExist(mzfy_path+"/碼字風雲/.设置"))
		mzfy_path = mzfy_path+"/碼字風雲/.设置";
	else if (isFileExist(mzfy_path+"碼字風雲/.设置"))
		mzfy_path = mzfy_path+"碼字風雲/.设置";
	else if (mzfy_path.endsWith("/.设置"))
		;
	else if (mzfy_path.endsWith("/.设置/"))
        mzfy_path.chop(1);
	else
	{
		QMessageBox::information(this, tr("读取失败"), tr("您打开的似乎不是一个码字风云文件夹？\n文件夹名称：碼字風雲"));
		return ;
	}

	// ==== 开始导入 ====
	QString import_tip = "";
	// mzfy_path == ../碼字風雲/.设置
	if (isFileExist(mzfy_path+"/目录") == false || isFileExist(mzfy_path+"/作品") == false)
	{
		QMessageBox::information(this, tr("读取失败"), tr("您打开的似乎不是一个码字风云文件夹？\n文件夹名称：碼字風雲"));
		return ;
	}
	QDir dir(mzfy_path+"/作品"); // 从作品那里查找，或者目录也行的
    dir.setFilter(QDir::Dirs);
    QFileInfoList list = dir.entryInfoList();
    int count = list.count();
	for (int i = 0; i < count; i++)
	{
        QFileInfo file_info = list.at(i);
        QString file_name = file_info.baseName();
        if (file_name == "." || file_name == ".." || file_name.isEmpty())
            continue;
        if (!isFileExist(mzfy_path+"/目录/"+file_name+".txt"))
        	import_tip += QString("导入《%1》失败：作品目录不存在\n").arg(file_name);

        // ==== 开始导入 ====
        QString novel_name = file_name;
        if (isFileExist(rt->NOVEL_PATH+novel_name))
        {
        	int index = 1;
    		while (isFileExist(rt->NOVEL_PATH+novel_name+QString("(%1)").arg(index)))
    			index++;
    		novel_name += QString("(%1)").arg(index);
        }

        if (importMzfyNovel(novel_name, mzfy_path+"/目录/"+file_name+".txt", file_info.filePath()))
			import_tip += QString("导入《%1》成功\n").arg(novel_name);
		else
			import_tip += QString("导入《%1》失败\n").arg(novel_name);
	}

    if (!import_tip.isEmpty())
        QMessageBox::information(this, tr("导入完毕"), import_tip);
    emit signalImportMzfyFinished("");
    submit_button->setText(QString("导入完毕(%1)").arg(count));
    file_path = "";
}

/**
 * 导入碼字風雲文件夹内的小说
 * @param  novel_name         小说名（需要先判断有没有重复存在）
 * @param  mzfy_dir_path      旧目录路径
 * @param  mzfy_chapters_path 旧章节文件夹
 * @return                    是否导入成功
 */
bool ImportMzfyPage::importMzfyNovel(QString novel_name, QString mzfy_dir_path, QString mzfy_chapters_path)
{
	QString dir_content = "<BOOK>";
    QString mzfy_content = readTextFile(mzfy_dir_path, "GBL");
    QString novel_path = rt->NOVEL_PATH+novel_name;
    QString dir_path = novel_path + "/" + NOVELDIRFILENAME;
    QString chapters_path = novel_path+"/chapters/";
    if (mzfy_chapters_path.endsWith(QString("/")) == false)
         mzfy_chapters_path += "/";


	ensureDirExist(novel_path);
	ensureDirExist(novel_path+"/chapters");
	ensureDirExist(novel_path+"/details");
	ensureDirExist(novel_path+"/outlines");
	ensureDirExist(novel_path+"/recycles");

    mzfy_content = getStrMid(mzfy_content, "<zopb", "/zopb");

	// ==== 遍历卷 ====
    QStringList roll_list = getStrMids(mzfy_content, "<jrjr", "/jrjr>");
	for (QString roll_content : roll_list)
	{
		// ==== 导入目录名字 ====
        QString roll_name = getStrMid(roll_content, "<jrmk", "/jrmk>");
		dir_content += QString("<ROLL><RINF><n:%1></RINF><LIST>").arg(roll_name);

		// ==== 遍历章节 ====
        QStringList chpt_list = getStrMids(roll_content, "<vhmk", "/vhmk>");
		for (QString chpt_name : chpt_list)
		{
            dir_content += QString("<CHPT><n:%1></CHPT>").arg(chpt_name);
			if (isFileExist(mzfy_chapters_path+chpt_name+".txt"))
			{
                QString chapter_text = readTextFile(mzfy_chapters_path+chpt_name+".txt", "GBK");
                writeTextFile(chapters_path+fnEncode(chpt_name)+".txt", chapter_text);
			}
		}

        dir_content += QString("</LIST></ROLL>");
	}

    dir_content += "</BOOK>";

	writeTextFile(dir_path, dir_content);

    return true;
}
