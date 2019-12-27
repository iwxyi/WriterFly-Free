#include "usersettingsitem.h"

UserSettingsItem::UserSettingsItem(QWidget *parent) : QWidget(parent)
{
    /* setBgColor(us->mainwin_sidebar_color);
    setBgColor(us->getOpacityColor(us->accent_color, 64), us->getOpacityColor(us->accent_color, 128));
    setRadius(us->widget_radius, 3); */
}

UserSettingsItem::UserSettingsItem(QWidget *parent, QString key) : UserSettingsItem(parent)
{
    this->key = key;
}

/**
 * 添加字符串形式的设置项
 * @param key   设置项关键词
 * @param title 设置项标题
 * @param desc  设置项描述
 * @param tip   设置项提示（鼠标悬浮）
 * @param val   设置项字符串值
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString desc, QString tip, QString val)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    lb_val = new QLabel(val);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height() << 2;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        vlayout->addWidget(lb_desc);
    }

    hlayout->addLayout(vlayout);
    hlayout->addWidget(lb_val);
    lb_val->setAlignment(Qt::AlignRight);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

/**
 * 添加整数形式的设置项
 * @param key   设置项关键词
 * @param title 设置项标题
 * @param desc  设置项描述
 * @param tip   设置项提示（鼠标悬浮）
 * @param val   设置项整型值
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString desc, QString tip, int val)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    nb_val = new AniNumberLabel(this);
    nb_val->setAlignment(Qt::AlignRight);
    nb_val->setShowNum(val);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height() << 2;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        vlayout->addWidget(lb_desc);
    }
    hlayout->addLayout(vlayout);
    hlayout->addWidget(nb_val);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

/**
 * 添加布尔型的设置项
 * @param key   设置项关键词
 * @param title 设置项标题
 * @param desc  设置项描述
 * @param tip   设置项提示（鼠标悬浮）
 * @param val   设置项布尔值
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString desc, QString tip, bool val)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    as = new AniSwitch(val);
    as->setMaximumWidth(SWITCH_WIDTH);
    as->setMinimumWidth(SWITCH_WIDTH);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height() << 2;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        vlayout->addWidget(lb_desc);
    }
    hlayout->addLayout(vlayout);
    hlayout->addWidget(as);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString btn, QString desc, QString tip, bool val)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    ev_btn = new QPushButton(btn);
    ev_btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(ev_btn, &QPushButton::clicked, [=]{
        emit signalBtnClicked(this, key);
    });

    as = new AniSwitch(val);
    as->setMaximumWidth(SWITCH_WIDTH);
    as->setMinimumWidth(SWITCH_WIDTH);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height()*5;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        QHBoxLayout* btn_hlayout = new QHBoxLayout;
        btn_hlayout->addWidget(ev_btn);
        btn_hlayout->addWidget(lb_desc);
        vlayout->addLayout(btn_hlayout);
    }
    else
    {
        vlayout->addWidget(ev_btn);
    }
    hlayout->addLayout(vlayout);
    hlayout->addWidget(as);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString btn, QString desc, QString tip, QString v)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    ev_btn = new QPushButton(btn);
    ev_btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(ev_btn, &QPushButton::clicked, [=]{
        emit signalBtnClicked(this, key);
    });

    if (v != "")
    {

    }

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height()*5;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        QHBoxLayout* btn_hlayout = new QHBoxLayout;
        btn_hlayout->addWidget(ev_btn);
        btn_hlayout->addWidget(lb_desc);
        vlayout->addLayout(btn_hlayout);
    }
    else
    {
        vlayout->addWidget(ev_btn);
    }
    hlayout->addLayout(vlayout);
    //hlayout->addWidget(as);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString btn, QString desc, QString tip, int v)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    ev_btn = new QPushButton(btn);
    ev_btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(ev_btn, &QPushButton::clicked, [=]{
        emit signalBtnClicked(this, key);
    });

    nb_val = new AniNumberLabel(this);
    nb_val->setAlignment(Qt::AlignRight);
    nb_val->setShowNum(v);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height()*5;

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        QHBoxLayout* btn_hlayout = new QHBoxLayout;
        btn_hlayout->addWidget(ev_btn);
        btn_hlayout->addWidget(lb_desc);
        vlayout->addLayout(btn_hlayout);
    }
    else
    {
        vlayout->addWidget(ev_btn);
    }
    hlayout->addLayout(vlayout);
    hlayout->addWidget(nb_val);

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

/**
 * 添加颜色形式的设置项
 * @param key   设置项关键词
 * @param title 设置项标题
 * @param desc  设置项描述
 * @param tip   设置项提示（鼠标悬浮）
 * @param val   设置项颜色值
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString desc, QString tip, QColor c)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QHBoxLayout* hlayout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);
    rb_color = new AniCircleLabel(this);
    rb_color->setMaximumWidth(SWITCH_WIDTH);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height() << 2;

    /*QPalette palette = lb_val->palette();
    palette.setColor(QPalette::Background, QColor(100, 2, 3, 255));
    lb_val->setAutoFillBackground(true);  //一定要这句，否则不行（实测加了也不行……）
    lb_val->setPalette(palette);
    lb_val->update();*/
    rb_color->setMainColor(c);

    vlayout->addWidget(lb_title);
    if (desc != "")
    {
        vlayout->addWidget(lb_desc);
    }
    hlayout->addLayout(vlayout);
    hlayout->addWidget(rb_color);
    //hLayout->addWidget(lb_val); // 因为改变不了颜色，所以就不显示了吧……

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(hlayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

/**
 * 添加不可更改的设置项
 * @param key   设置项关键词
 * @param title 设置项标题
 * @param desc  设置项描述
 * @param tip   设置项提示（鼠标悬浮）
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString key, QString title, QString desc, QString tip)
    : UserSettingsItem(parent, key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    QVBoxLayout* vLayout = new QVBoxLayout;

    lb_title = new QLabel(title);
    lb_desc = new QLabel(desc);

    // lb_desc->setStyleSheet(DESC_STYLESHEET);
    item_height = lb_title->fontMetrics().height() << 2;

    vLayout->addWidget(lb_title);
    if (desc != "")
    {
        vLayout->addWidget(lb_desc);
    }

    if (tip != "")
    {
        this->setToolTip(tip);
    }

    this->setLayout(vLayout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

/**
 * 添加设置项分组标题
 * @param key   关键词
 * @param title 标题
 * @param desc  描述
 */
UserSettingsItem::UserSettingsItem(QWidget *parent, QString group_key, QString title, QString desc)
    : UserSettingsItem(parent, group_key)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    Q_UNUSED(desc);
    lb_title = new QLabel(title);
    lb_title->setFont(QFont(lb_title->font().family(), 15));
    lb_title->setStyleSheet("color: "+us->getColorString(us->global_font_color)+";");

    lb_title->setMargin(10);
    item_height = lb_title->fontMetrics().height()*5;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(lb_title);
    layout->setMargin(0);
    setLayout(layout);

    updateUI();
    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
}

UserSettingsItem::UserSettingsItem(QWidget *parent, int kind) : UserSettingsItem(parent)
{
    // DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 128)+";";

    if (kind == 2) // 分割线
    {
        key = "splitter"; // 设置特定的分割线关键词，展开隐藏的项目

        lb_title = new QLabel();
        lb_title->setStyleSheet("QWidget{background: rgba(128,128,128,30);}");
        //lb_title->setGeometry(0, 15, 10000, 1);
        lb_title->setMaximumHeight(2);
        lb_title->setToolTip("点击查看隐藏的设置项");
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(lb_title);
        setLayout(layout);
    }
}

void UserSettingsItem::mouseReleaseEvent(QMouseEvent* e)
{
    QWidget::mouseReleaseEvent(e);
    if ((e->pos()-press_point).manhattanLength() > QApplication::startDragDistance()) // 不算点击
        return ;
    if (e->button() == Qt::LeftButton)
        emit signalClicked(this, key);
}

void UserSettingsItem::mousePressEvent(QMouseEvent *e)
{
    press_point = e->pos();
}

void UserSettingsItem::enterEvent(QEvent *e)
{
    emit signalMouseEntered(group);
    return QWidget::enterEvent(e);
}

void UserSettingsItem::setVal(QString v)
{
    lb_val->setText(v);
}

void UserSettingsItem::setVal(int    v)
{
    nb_val->setShowNum(v);
}

void UserSettingsItem::setVal(bool   v)
{
    as->slotSwitch(v);
}

void UserSettingsItem::setVal(QColor v)
{
    /*lb_val->clear();
    QPalette palette;
    palette.setColor(QPalette::Background, v);
    lb_val->setAutoFillBackground(true);  //一定要这句，否则不行
    lb_val->setPalette(palette);
    lb_val->update();*/
    rb_color->setMainColor(v);
}

void UserSettingsItem::setBtn(QString s)
{
    ev_btn->setText(s);
}

void UserSettingsItem::setDesc(QString s)
{
    lb_desc->setText(s);
}

void UserSettingsItem::setGroup(int g)
{
    this->group = g;
}

int UserSettingsItem::getGroup()
{
    return this->group;
}

int UserSettingsItem::getHeight()
{
    return item_height + 3; // 这个 3 是为了避免有些无法显示完全的地方
}

/**
 * 只有主题色变化
 */
void UserSettingsItem::updateUI()
{
    DESC_STYLESHEET = "color:"+us->getOpacityColorString(us->global_font_color, 192)+";";
    if (lb_desc != nullptr)
        lb_desc->setStyleSheet(DESC_STYLESHEET);

    if (lb_title != nullptr)
        lb_title->setStyleSheet("color: "+us->getColorString(us->global_font_color)+";");
}
