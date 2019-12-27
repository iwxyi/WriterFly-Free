#include "syncstatewindow.h"

SyncStateWindow::SyncStateWindow(QWidget *parent) : MyDialog(parent)
{
    initView();
    initMenu();
}

void SyncStateWindow::showEvent(QShowEvent *event)
{
    initData();
    refreshMenu();

    if (!ac->download_seq.isEmpty())    // 最优先的：下载队列
    {
        if (tab_widget->currentIndex() != 0)
        {
            tab_widget->preventSwitchAnimationOnce();
            tab_widget->setCurrentIndex(0);
        }
    }
    else if (!ac->upload_seq.isEmpty()) // 上传队列
    {
        if (tab_widget->currentIndex() != 1)
        {
            tab_widget->preventSwitchAnimationOnce();
            tab_widget->setCurrentIndex(1);
        }
    }
    else                                // 没有队列：已完成
    {
        if (tab_widget->currentIndex() != 2)
        {
            tab_widget->preventSwitchAnimationOnce();
            tab_widget->setCurrentIndex(2);
        }
    }

    return MyDialog::showEvent(event);
}

void SyncStateWindow::initView()
{
    // ==== 云同步队列 ====
    tab_widget = new AniTabWidget(this);
	download_listview = new QListView(this);
	upload_listview = new QListView(this);
	// download_finished_listview = new QListView(this);
	// upload_finished_listview = new QListView(this);
    finished_listview = new QListView(this);

//    tab_widget->setTabInBottom();
    tab_widget->addTab(download_listview, "下载中");
    tab_widget->addTab(upload_listview, "上传中");
    // sync_state_tab->addTab(download_finished_listview, "下载完成");
    // sync_state_tab->addTab(upload_finished_listview
    tab_widget->addTab(finished_listview, "已完成");
    download_listview->setStyleSheet("background:transparent; border:1px solid rgba(88,88,88,30);");
    upload_listview->setStyleSheet("background:transparent; border:1px solid rgba(88,88,88,30);");
    finished_listview->setStyleSheet("background:transparent; border:1px solid rgba(88,88,88,30);");
    tab_widget->setTransparentBackground(true, 1);

    // ==== 布局 ====

	QVBoxLayout* main_vlayout = main_layout;
    addTitleSpacing();
    {
        QHBoxLayout* account_hlayout = new QHBoxLayout;
        account_hlayout->setAlignment(Qt::AlignRight);
        // TODO 加入显示用户信息的控件
        main_vlayout->addLayout(account_hlayout);
    }
    main_vlayout->addWidget(tab_widget);
    setLayout(main_vlayout);
    int mw = width() + 250;
    if (parentWidget() != nullptr && mw > parentWidget()->width()*2/3)
        mw = parentWidget()->width()*2/3;
    setMinimumWidth(mw);

    // ==== 设置列表数据 ====

    download_model = new SyncSequeueModel(&(ac->download_seq), this);
    download_listview->setModel(download_model);

    upload_model = new SyncSequeueModel(&(ac->upload_seq), this);
	upload_listview->setModel(upload_model);

    finished_model = new SyncFinishedSequeueModel(&(ac->finished_seq), this);
    finished_delegate = new SyncFinishedSequeueDelegate(this);
    finished_listview->setModel( finished_model );
    finished_listview->setItemDelegate( finished_delegate );

    connect(ac, &SyncThread::signalSyncChanged, [=](int, int){
        if (!this->isHidden())
        {
            initData();
        }
    });
}

void SyncStateWindow::initData()
{
    download_model->listDataChanged();
    upload_model->listDataChanged();
    finished_model->listDataChanged();

    tab_widget->setTabText(0, QString("下载中(%1)").arg(ac->download_seq.size()));
    tab_widget->setTabText(1, QString("上传中(%1)").arg(ac->upload_seq.size()));
    tab_widget->setTabText(2, QString("已完成(%1)").arg(ac->finished_seq.size()));
}

void SyncStateWindow::initMenu()
{
    sync_menu = new QMenu("云同步", this);
    modify_nickname_action = new QAction(QIcon(":/icons/menu/smile2"), "修改昵称", this);
    modify_username_action = new QAction(QIcon(":/icons/menu/smile1"), "修改账号", this);
    modify_password_action = new QAction(QIcon(":/icons/menu/password"), "修改密码", this);
    sync_novels_action = new QAction(QIcon(":/icons/menu/cloud_sync"), "同步小说", this);
    logout_action = new QAction(QIcon(":/icons/menu/cry"), "退出登录", this);

    sync_novels_action->setCheckable(true);
    sync_novels_action->setChecked(true);
    sync_novels_action->setEnabled(false);
    modify_username_action->setEnabled(false);

    sync_menu->addAction(modify_nickname_action);
    sync_menu->addAction(modify_username_action);
    sync_menu->addAction(modify_password_action);
    sync_menu->addSeparator();
    sync_menu->addAction(sync_novels_action);
    sync_menu->addSeparator();
    sync_menu->addAction(logout_action);

    setDialogMenu(sync_menu);

    connect(modify_nickname_action, SIGNAL(triggered()), this, SLOT(actionModifyNickname()));
    connect(modify_username_action, SIGNAL(triggered()), this, SLOT(actionModifyUsername()));
    connect(modify_password_action, SIGNAL(triggered()), this, SLOT(actionModifyPassword()));
    connect(sync_novels_action, SIGNAL(triggered()), this, SLOT(actionSyncNovels()));
    connect(logout_action, SIGNAL(triggered()), this, SLOT(actionLogout()));
}

void SyncStateWindow::refreshMenu()
{
    sync_novels_action->setChecked(us->sync_novels);
}

void SyncStateWindow::actionModifyNickname()
{
    if (!ac->isLogin()) return ;

    bool ok;
    QString nickname = "";
    QString tip = "请输入新昵称，所有人可见，2~20 位字母/数字/汉字。\n24小时可修改一次";
    while (1)
    {
        nickname = QInputDialog::getText(this, "修改昵称", tip, QLineEdit::Normal, ac->nickname, &ok);
        if (!ok || nickname.isEmpty()) return ;
        if (canBeNickname(nickname))
            break;
        else
            tip = "昵称格式错误，应当为 2~20 位汉字/字母/数字";
    }

    // 到服务器上更改密码
    connect(new NetUtil(rt->SERVER_PATH+"account_bind.php", ac->getIdentity() << "nickname" << nickname), &NetUtil::finished, [=](QString s){
        if (getXml(s, "state") == "OK")
        {
            QMessageBox::information(this, "修改昵称", "修改成功，部分界面重启后生效");
            ac->nickname = nickname;
            ac->save();
            us->setVal("account/nickname", nickname);
        }
        else
        {
            QString error = getXml(s, "REASON");
            if (error.isEmpty()) error = "修改失败";
            QMessageBox::information(this, "修改失败", error);
            return ;
        }
    });
}

void SyncStateWindow::actionModifyUsername()
{
    if (!ac->isLogin()) return ;
    // 进制修改用户名
}

void SyncStateWindow::actionModifyPassword()
{
    if (!ac->isLogin()) return ;
    // 输入旧密码
    bool ok;
    QString checked_pwd = QInputDialog::getText(this, "修改密码(1/2)", "请输入旧密码以验证", QLineEdit::Password, "", &ok);
    if (!ok || checked_pwd.isEmpty()) return ;
    if (checked_pwd != ac->password)
    {
        QMessageBox::critical(this, "修改密码", "密码错误，请重试");
        return ;
    }

    // 输入新密码
    QString new_pwd = "";
    QString tip = "请输入新密码，2~20 位字母/数字/汉字（牢记）";
    while (1)
    {
        new_pwd = QInputDialog::getText(this, "修改密码(2/2)", tip, QLineEdit::Normal, "", &ok);
        if (!ok || new_pwd.isEmpty()) return ;
        if (canBeNickname(new_pwd))
            break;
        else
            tip = "新密码格式错误，应当为 2~20 位字母/数字/汉字";
    }

    // 到服务器上更改密码
    connect(new NetUtil(rt->SERVER_PATH+"account_bind.php", ac->getIdentity() << "newpassword" << new_pwd), &NetUtil::finished, [=](QString s){
        if (getXml(s, "state") == "OK")
        {
            QMessageBox::information(this, "修改密码", "修改成功，请牢记您的新密码");
            ac->password = new_pwd;
            ac->save();
        }
        else
        {
            QString error = getXml(s, "REASON");
            if (error.isEmpty()) error = "修改失败";
            QMessageBox::information(this, "修改失败", error);
            return ;
        }
    });
}

void SyncStateWindow::actionSyncNovels()
{
    if (!ac->isLogin()) return ;
    us->sync_novels = !us->sync_novels;
    us->setVal("us/sync_novels", us->sync_novels);
}

void SyncStateWindow::actionLogout()
{
    if (!ac->isLogin()) return ;
    QMessageBox mess(QMessageBox::Warning,tr("提示"),tr("是否退出云同步账号？\n重新登录后将继续之前未完成的云同步"));
    QPushButton *ok_btn = (mess.addButton(tr("确认退出"), QMessageBox::AcceptRole));
    QPushButton *cancel_btn = (mess.addButton(tr("取消退出"), QMessageBox::RejectRole));
    mess.setIconPixmap(QPixmap(":/icons/cry2"));
    mess.exec();
    if (mess.clickedButton()==ok_btn)
    {
        ac->logout();
        this->hide();
    }
}
