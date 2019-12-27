#include "loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent) : MyDialog(parent), page(0)
{
    setWindowTitle("登录 - 码字风云");

    initView();
    initLayout();
    initStyle();
    initEvent();
    initData();
}

void LoginWindow::showEvent(QShowEvent* e)
{
    form_panel->setStyleSheet("background:"+us->getColorString(us->mainwin_bg_color)+"; border-radius:10px;");

    QString edit_style = "border:1px solid "+us->getColorString(us->accent_color)+";border-radius:10px; padding:2px; padding-left:10px; color:" + us->getColorString(us->accent_color)+";";
    username_edit->setStyleSheet(edit_style);
    password_edit->setStyleSheet(edit_style);
    password_check_edit->setStyleSheet(edit_style);
    nickname_edit->setStyleSheet(edit_style);

    return MyDialog::showEvent(e);
}

void LoginWindow::initView()
{
    login_page = new InteractiveButtonBase("登录", this);
    register_page = new InteractiveButtonBase("注册", this);
    logo_btn = new InteractiveButtonBase(QIcon(":/icons/appicon"), this);
    tip_label = new QLabel("写作天下服务端正在开发中…\n暂用码字风云账号，双向同步", this);
    divider_widget = new QWidget(this);
    form_panel = new QWidget(this);
    username_label = new WaterCircleButton(QIcon(":/icons/cloud/username"), this);
    username_edit = new QLineEdit(this);
    password_label = new WaterCircleButton(QIcon(":/icons/cloud/password"), this);
    password_edit = new QLineEdit(this);
    password_check_label = new WaterCircleButton(QIcon(":/icons/cloud/password_check"), this);
    password_check_edit = new QLineEdit(this);
    nickname_label = new WaterCircleButton(QIcon(":/icons/cloud/nickname"), this);
    nickname_edit = new QLineEdit(this);
    login_btn = new WaterFloatButton(this);

    username_edit->setPlaceholderText("用户账号");
    password_edit->setPlaceholderText("用户密码");
    password_check_edit->setPlaceholderText("确认密码");
    nickname_edit->setPlaceholderText("显示昵称");
    password_edit->setEchoMode(QLineEdit::Password);
    password_check_edit->setEchoMode(QLineEdit::Password);

    QRegExp re("([\\w@_]|[^\\x00-\\xff]){2,20}");
    QValidator *username_valid=new QRegExpValidator(re, username_edit);
    username_edit->setValidator(username_valid);
    QValidator *password_valid=new QRegExpValidator(re, password_edit);
    password_edit->setValidator(password_valid);
    QValidator *password_check_valid=new QRegExpValidator(re, password_check_edit);
    password_check_edit->setValidator(password_check_valid);
    QValidator *nickname_valid=new QRegExpValidator(re, nickname_edit);
    nickname_edit->setValidator(nickname_valid);

    logo_btn->setShowAni(true);
    logo_btn->showForeground2(QPoint(0,1));

    gray_label = new QLabel(this);
    ani_pic = new InteractiveButtonBase(QIcon(":/icons/appicon"), this);
    gray_label->hide();
    ani_pic->hide();
    ani_pic->setRadius(10);
    ani_pic->setHoverAniDuration(1000);
    ani_pic->setWaterAniDuration(2000, 1000, 10000);
    ani_pic->setBgColor(us->mainwin_bg_color, us->mainwin_bg_color);
    ani_pic->setShowAni(true);
    ani_pic->setFixedForeSize(true);
}

void LoginWindow::initLayout()
{
    int shadow = 2;

    QVBoxLayout* main_vlayout = main_layout;
    {
        QHBoxLayout* page_hlayout = new QHBoxLayout;
        page_hlayout->setSpacing(0);
        page_hlayout->setAlignment(Qt::AlignLeft);
        page_hlayout->addWidget(login_page);
        page_hlayout->addWidget(register_page);
        main_vlayout->addLayout(page_hlayout);
    }

    {
        QVBoxLayout* form_vlayout = new QVBoxLayout;
        {
            QHBoxLayout* logo_hlayout = new QHBoxLayout;
            logo_hlayout->setAlignment(Qt::AlignCenter);
            logo_hlayout->addWidget(logo_btn);
            form_vlayout->addLayout(logo_hlayout);
        }
        {
            form_vlayout->addWidget(tip_label);
            form_vlayout->addSpacing(10);
            form_vlayout->addWidget(divider_widget);
            form_vlayout->addSpacing(10);
        }

        {
            QHBoxLayout* username_hlayout = new QHBoxLayout;
            username_hlayout->addWidget(username_label);
            username_hlayout->addWidget(username_edit);
            form_vlayout->addLayout(username_hlayout);
        }
        {
            QHBoxLayout* password_hlayout = new QHBoxLayout;
            password_hlayout->addWidget(password_label);
            password_hlayout->addWidget(password_edit);
            form_vlayout->addLayout(password_hlayout);
        }
        {
            QHBoxLayout* password_check_hlayout = new QHBoxLayout;
            password_check_hlayout->addWidget(password_check_label);
            password_check_hlayout->addWidget(password_check_edit);
            form_vlayout->addLayout(password_check_hlayout);
        }
        {
            QHBoxLayout* nickname_hlayout = new QHBoxLayout;
            nickname_hlayout->addWidget(nickname_label);
            nickname_hlayout->addWidget(nickname_edit);
            form_vlayout->addLayout(nickname_hlayout);
        }
        form_panel->setLayout(form_vlayout);
        main_vlayout->addWidget(form_panel);

        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(form_panel);
        shadow_effect->setOffset(0, shadow);
        shadow_effect->setColor(QColor(0x88, 0x88, 0x88, 0x88));
        shadow_effect->setBlurRadius(10);
        form_panel->setGraphicsEffect(shadow_effect);
    }
    {
        QHBoxLayout* btn_hlayout = new QHBoxLayout;
        btn_hlayout->setAlignment(Qt::AlignCenter);
        btn_hlayout->addWidget(login_btn);
        main_vlayout->addLayout(btn_hlayout);
    }

    this->setLayout(main_vlayout);
    main_vlayout->setMargin(20);
    main_vlayout->setSpacing(10);
}

void LoginWindow::initStyle()
{
    switchPage(0);

    this->setStyleSheet("border-radius:10px;");

    login_page->setMinimumSize(fontMetrics().width("登录") * 2.3, fontMetrics().lineSpacing() * 2.3);
    register_page->setMinimumSize(fontMetrics().width("登录") * 2.3, fontMetrics().lineSpacing() * 2.3);

    logo_btn->setFixedSize(this->width()/2, this->width()/2);
    logo_btn->setPaddings(-10);
    tip_label->setAlignment(Qt::AlignCenter);
    tip_label->setStyleSheet("margin-left:20px; margin-right:20px");
    divider_widget->setFixedHeight(1);
    divider_widget->setStyleSheet("background-color:rgba(88,88,88,64); margin:10px;");

    int si = username_edit->height();
    username_label->setFixedSize(si, si);
    password_label->setFixedSize(si, si);
    password_check_label->setFixedSize(si, si);
    nickname_label->setFixedSize(si, si);
    username_label->setPaddings(0);
    password_label->setPaddings(0);
    password_check_label->setPaddings(0);
    nickname_label->setPaddings(0);

    login_btn->setFontSize(font().pointSize()*1.5);
    login_btn->setFixedSize(login_btn->fontMetrics().width("登录") * 3, login_btn->fontMetrics().lineSpacing()*1.5);
    login_btn->setBgColor(us->getOpacityColor(us->accent_color, 64));
    login_btn->setIconColor(us->getOpacityColor(us->accent_color, 64));
    login_btn->setBgColor(us->getOpacityColor(us->accent_color, 128), us->accent_color);
    login_btn->setTextColor(us->global_font_color);

    gray_label->setStyleSheet("background:rgba(88,88,88,64); border-radius:0px;");
}

void LoginWindow::initEvent()
{
    /*setTabOrder(username_edit, password_check_edit);
    setTabOrder(password_edit, password_check_edit);
    setTabOrder(password_check_edit, nickname_edit);
    setTabOrder(nickname_edit, login_btn);*/

    /*connect(qApp, &QApplication::focusChanged, [=](QWidget* o, QWidget* n){
        qDebug() << o << n;
    });*/

    connect(this, SIGNAL(signalConfirm()), this, SLOT(loginBtnClicked()));

    connect(ac, &UserAccount::signalLoginFinished, [=]{
        this->close();
    });
    connect(login_page, &InteractiveButtonBase::clicked, [=]{
        switchPage(0);
    });
    connect(register_page, &InteractiveButtonBase::clicked, [=]{
        switchPage(1);
    });

    connect(login_btn, SIGNAL(clicked()), this, SLOT(loginBtnClicked()));
}

void LoginWindow::initData()
{
    username_edit->setText(ac->getUsername());
    password_edit->setText(ac->getPassword());

    switchPage(0);
}

void LoginWindow::switchPage(int p)
{
    page = p;

    InteractiveButtonBase* current_page, * another_page;
    if (!p) // 登录页
    {
        current_page = login_page;
        another_page = register_page;

        password_check_edit->hide();
        password_check_label->hide();
        nickname_edit->hide();
        nickname_label->hide();

        login_btn->setText("登录");
    }
    else // 注册页
    {
        current_page = register_page;
        another_page = login_page;

        password_check_edit->show();
        password_check_label->show();
        nickname_edit->show();
        nickname_label->show();

        login_btn->setText("注册");
    }
    current_page->setTextColor(us->accent_color);
    current_page->setFontSize(font().pointSize() * 1.5);
    another_page->setTextColor(Qt::gray);
    another_page->setFontSize(font().pointSize());
}

QPropertyAnimation *LoginWindow::startHeightAnimation(QWidget *widget, int start_value, int end_value)
{
    QPropertyAnimation* ani = new QPropertyAnimation(widget, "size");
    ani->setDuration(300);
    ani->setStartValue(QSize(widget->width(), start_value));
    ani->setEndValue(QSize(widget->width(), end_value));
    ani->start();

    connect(ani, &QPropertyAnimation::finished, [=]{
        if (end_value == 0)
            widget->hide();
        ani->deleteLater();
    });

    return ani;
}

void LoginWindow::showLoading()
{
    gray_label->setGeometry(0,0,width(),height());

    ani_pic->setGeometry(form_panel->geometry());
    ani_pic->setHoverAniDuration(1000);
    ani_pic->showForeground2(QPoint(0,1)); // 显示前景动画
//    ani_pic->simulateStatePress();

    logo_btn->setIcon(QIcon());

    QTimer::singleShot(300, [=]{
        if (!ani_pic->isHidden())
            ani_pic->simulateStatePress();
    });

    gray_label->show();
    ani_pic->show();

    QGraphicsOpacityEffect *opacity_effect = new QGraphicsOpacityEffect(this);
    opacity_effect->setOpacity(0);
    gray_label->setGraphicsEffect(opacity_effect);

    QPropertyAnimation* ani = new QPropertyAnimation(opacity_effect, "opacity");
    ani->setStartValue(0);
    ani->setEndValue(1);
    ani->setDuration(300);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=]{
        ani->deleteLater();
        opacity_effect->deleteLater();
    });
}

void LoginWindow::hideLoading()
{
    gray_label->hide();
    ani_pic->hide();
    ani_pic->hideForeground();

    logo_btn->setIcon(QIcon(":/icons/appicon"));

    QGraphicsOpacityEffect *opacity_effect = new QGraphicsOpacityEffect(this);
    opacity_effect->setOpacity(1);
    gray_label->setGraphicsEffect(opacity_effect);

    QPropertyAnimation* ani = new QPropertyAnimation(opacity_effect, "opacity");
    ani->setStartValue(0);
    ani->setEndValue(1);
    ani->setDuration(300);
    ani->start();
    connect(ani, &QPropertyAnimation::finished, [=]{
        ani->deleteLater();
        opacity_effect->deleteLater();
    });
}

void LoginWindow::loginBtnClicked()
{
    QString username = username_edit->text();
    QString password = password_edit->text();
    QString password2 = password_check_edit->text();
    QString nickname = nickname_edit->text();

    if (username.isEmpty())
        username_label->simulateStatePress();
    if (password.isEmpty())
        password_label->simulateStatePress();
    if (password2.isEmpty() || password != password2)
        password_check_label->simulateStatePress();
    if (nickname.isEmpty())
        nickname_label->simulateStatePress();
    if (username.isEmpty() || password.isEmpty())
        return ;


    if (!page) // 登录
    {
        NetUtil* n = ac->login(username, password);
        if (n != nullptr)
        {
            showLoading();
            connect(n, &NetUtil::finished, [=]{
                hideLoading();
            });
        }
    }
    else // 注册
    {
        if (password2.isEmpty() || nickname.isEmpty() || password != password2)
            return ;
        showLoading();
        // 进行注册
        connect(new NetUtil(rt->SERVER_PATH+"account_register.php", QStringList{
                                "version", ac->version_s, "vericode", ac->enVerify(), "imei", ac->CPU_ID, "username", username, "password", password, "nickname", nickname,
                                "allwords", QString::number(ac->getUserWords()), "alltimes", QString::number(ac->getUserTimes()), "allbonus", "0"
                            }), &NetUtil::finished, [=](QString s){
           hideLoading();
           if (!ac->deVerify(getXml(s, "verify")))
           {
               QMessageBox::information(this, "注册失败", "为保证云同步稳定性与安全性，请修改设备时间为标准北京时间");
               return ;
           }
           QString error = getXml(s, "ERROR");
           if (!error.isEmpty())
           {
               QMessageBox::information(this, "注册失败", error);
               return ;
           }
           ac->userID = getXml(s, "userID");
           ac->username = username;
           ac->password = password;
           ac->nickname = nickname;
           ac->rank = 9999;
           ac->save();

           us->setVal("account/nickname", ac->nickname);
           us->setVal("account/prev_userID", ac->userID);
           us->setVal("sync/last_download_pull_timestamp", 0);
           us->setVal("sync/last_upload_added_timetamp", 0);
           us->setVal("sync/prev_full_upload_timestamp", 0);
           us->setVal("sync/lastest_downloaded_timstamp", 0);

           ac->loginFinished();
        });
    }
}

