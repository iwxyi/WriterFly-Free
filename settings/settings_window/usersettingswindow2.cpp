#include "usersettingswindow2.h"

UserSettingsWindow2::UserSettingsWindow2(QWidget *parent) : QWidget(parent)
{
    // 设置布局
    bg_widget = new QWidget(this);
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    QVBoxLayout *btn_layout = new QVBoxLayout;
    InteractiveButtonBase *shortcut_btn = new InteractiveButtonBase("快捷键", this);
    group_list = new QListWidget;
    item_list = new QListWidget;
    btn_layout->addWidget(group_list);
    btn_layout->addWidget(shortcut_btn);
    hlayout->addLayout(btn_layout);
    hlayout->addWidget(item_list);
    hlayout->setStretch(0, 2);
    hlayout->setStretch(1, 5);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    group_list->setMinimumWidth(50);
    group_list->setMaximumWidth(200);
    group_list->setResizeMode(QListView::Adjust);
    group_list->setFocusPolicy(Qt::NoFocus);
    group_list->setSpacing(3);

    item_list->setMinimumWidth(100);
    item_list->setResizeMode(QListView::Adjust);
    item_list->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    item_list->verticalScrollBar()->setSingleStep(10);
    item_list->setFocusPolicy(Qt::NoFocus);

    this->setLayout(hlayout);
    group_btns.append(shortcut_btn);

    initGroups();
    initItems();
    current_group_index = 0;

    connect(shortcut_btn, &QPushButton::clicked, [=] {
        emit signalShortcutKey();
    });

    connect(thm, SIGNAL(windowChanged()), this, SLOT(updateUI()));
    connect(&gd->sm, SIGNAL(signalTriggered(QString)), this, SLOT(slotTriggerSettingItem(QString)));
    updateUI();
}

void UserSettingsWindow2::showEvent(QShowEvent *event)
{
    bg_widget->setGeometry(0,0,width(),height());
    return QWidget::showEvent(event);
}

void UserSettingsWindow2::closeEvent(QCloseEvent *event)
{
    return QWidget::closeEvent(event);
}

void UserSettingsWindow2::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        emit signalEsc();
    }

    return QWidget::keyPressEvent(event);
}

void UserSettingsWindow2::initGroups()
{
    QStringList group_names;
    group_names << "常用"
                << "主题色彩"
                << "全局外观"
                << "编辑器"
                << "特色功能"
                << "人机交互"
                << "操作动画"
                << "词典资源"
                << "一键排版";
                // << "关于";

    QFont font;
    QFontMetrics metrics(font);
    int height = metrics.lineSpacing() * 2 + WaterZoomButton::getDefaultSpacing();
    for (int i = 0; i < group_names.length(); ++i)
    {
        WaterZoomButton *btn = new WaterZoomButton(group_names.at(i), this);
        btn->setRadius(us->widget_radius, us->widget_radius / 2);
        group_btns.append(btn);

        QListWidgetItem *item = new QListWidgetItem(group_list);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setSizeHint(QSize(0, height));
        group_list->setItemWidget(item, btn);

        // 加载分组
        connect(btn, &WaterZoomButton::clicked, this, [=] {
            QPixmap pixmap_old(item_list->size());
            QPixmap pixmap_new(item_list->size());
            pixmap_new.fill(Qt::green);
            render(&pixmap_old, QPoint(0,0), item_list->geometry());
            initItems(group_names.at(i));
            updateUI();
//            item_list->render(&pixmap_new);
            render(&pixmap_new, QPoint(0,0), item_list->geometry());
            startGroupSwitchAnimation(pixmap_old, pixmap_new, i);
            current_group_index = i; // 记录当前的位置

            // 高亮显示当前分组的选项
            QColor bg_color = us->mainwin_sidebar_color;
            if (bg_color.alpha() == 0)
                bg_color.setAlpha(255);
            foreach (InteractiveButtonBase *btn, group_btns)
            {
                btn->setBgColor(bg_color);
            }
            QColor highlight_color;
            QColor c1 = bg_color;
            QColor c2 = us->getOpacityColor(us->accent_color, 64);
            int alpha_sum = c1.alpha() + c2.alpha();
            highlight_color = QColor(
                (c1.red()*c1.alpha() + c2.red()*c2.alpha()) / alpha_sum,
                (c1.green()*c1.alpha() + c2.green()*c2.alpha()) / alpha_sum,
                (c1.blue()*c1.alpha() + c2.blue()*c2.alpha()) / alpha_sum,
                c1.alpha() > c2.alpha() ? c1.alpha() : c2.alpha()
            );
            btn->setBgColor(highlight_color);
        });
    }
}

/**
 * 初始化分组的设置项
 * @param g 分组名字
 */
void UserSettingsWindow2::initItems(QString g)
{
    foreach (InteractiveButtonBase *btn, item_btns+item_child_btns)
    {
        btn->deleteLater();
    }
    item_btns.clear();
    item_child_btns.clear();
    item_list->clear();
    item_beans.clear();
    item_value_widgets.clear();

    QList<SettingBean *> gitems = gd->sm.getItems(g);
    int hidden_count = 0;
    foreach (SettingBean *sb, gitems)
    {
        if (sb->isChild)
            continue;
        if (sb->isHidden)
        {
            hidden_count++;
            continue;
        }
        addOneItem(sb);
        item_beans.append(sb);
    }

    if (hidden_count)
    {
        addSeperator(hidden_count);
        foreach (SettingBean *sb, gitems)
        {
            if (sb->isChild)
                continue;
            if (!sb->isHidden)
            {
                hidden_count++;
                continue;
            }
            addOneItem(sb, true);
            item_beans.append(sb);
        }
    }
}

/**
 * 添加一个设置项
 * @param sb 设置项对象
 */
void UserSettingsWindow2::addOneItem(SettingBean *sb, bool hide)
{
    WaterZoomButton *btn = new WaterZoomButton(this);
    btn->setRadius(us->widget_radius, us->widget_radius / 2);
    setItemLayout(btn, sb);
    item_btns.append(btn);

    QFontMetrics metrics(this->font());
    int height = metrics.lineSpacing() + WaterZoomButton::getDefaultSpacing() / 2;
    if (!sb->description.isEmpty())
        height += metrics.lineSpacing();
    else
        height += metrics.lineSpacing() / 2;
    if (sb->relates.size())
        height += metrics.lineSpacing() / 2;
    QListWidgetItem *item = new QListWidgetItem(item_list);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(0, height * 2));
    item_list->setItemWidget(item, btn);
    if (hide)
        item->setHidden(true);

    connect(btn, &WaterZoomButton::clicked, this, [=] {
        //        gd->sm.trigger(sb); // 通过全局统一调用？
        slotTriggerSettingItem(sb);
    });
}

/**
 * 添加隐藏设置的分割线
 */
void UserSettingsWindow2::addSeperator(int count)
{
    QListWidgetItem *item = new QListWidgetItem(item_list);
    WaterZoomButton* btn = new WaterZoomButton("不常用项("+QString::number(count)+")", this);
    item_btns.append(btn);

    QFontMetrics metrics(this->font());
    int height = metrics.lineSpacing()+btn->getDefaultSpacing();
    btn->setBgColor(QColor(0x88, 0x88, 0x88, 0x88));
    btn->setChoking((height-1)/2);
    btn->setRadius(1, (height-1)/2);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(0, height));
    item_list->setItemWidget(item, btn);

    connect(btn, &WaterZoomButton::clicked, this, [=]{
        for (int i = 0; i < item_list->count(); i++)
        {
            item_list->item(i)->setHidden(false);
        }
    });
}

void UserSettingsWindow2::setItemLayout(WaterZoomButton *btn, SettingBean *sb)
{
    // 设置项属性
    QHBoxLayout *hlayout = new QHBoxLayout;
    QVBoxLayout *vlayout = new QVBoxLayout;
    QLabel *title_label = new QLabel(sb->title, btn);
    QLabel *desc_label = new QLabel(sb->description, btn);
    btn->setToolTip(sb->tooltip);

    // 总体布局
    vlayout->addWidget(title_label);
    vlayout->addWidget(desc_label);
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    desc_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    if (sb->description.isEmpty())
        desc_label->hide();
    hlayout->addLayout(vlayout);
    //    hlayout->setMargin(btn->getChokingSpacing());
    hlayout->setContentsMargins(btn->getChokingSpacing() + us->widget_radius, btn->getChokingSpacing(), btn->getChokingSpacing() + us->widget_radius, btn->getChokingSpacing());
    btn->setLayout(hlayout);

    // 设置项的对应值
    switch (sb->getType())
    {
    case SVT_UNKNOW:
        item_value_widgets.append(nullptr);
        break;
    case SVT_BOOL:
    {
        AniSwitch *as = new AniSwitch(btn);
        as->setMaximumWidth(SWITCH_WIDTH);
        as->setMinimumWidth(SWITCH_WIDTH);
        bool b = sb->getValue().toBool();
        if (sb->need_restart) // 需要重启的，直接获取到的属性不一定正确
            b = us->getBool("us/"+sb->key, b);
        as->setOnOff(b);
        hlayout->addWidget(as);
        item_value_widgets.append(as);
    }
    break;
    case SVT_INTEGER:
    {
        AniNumberLabel *anl = new AniNumberLabel(btn);
        int n = sb->getValue().toInt();
        if (sb->need_restart)
            n = us->getInt("us/"+sb->key, n);
        anl->setShowNum(n);
        hlayout->addWidget(anl);
        item_value_widgets.append(anl);
    }
    break;
    case SVT_DOUBLE:
        break;
    case SVT_STRING:
    {
        QLabel *label = new QLabel(btn);
        label->setText(sb->getValue().toString());
        hlayout->addWidget(label);
        item_value_widgets.append(label);
    }
    break;
    case SVT_COLOR:
    {
        AniCircleLabel *acl = new AniCircleLabel(btn);
        acl->setMainColor(qvariant_cast<QColor>(sb->getValue()));
        QFontMetrics metrics(this->font());
        int height = metrics.lineSpacing() * 2;
        acl->setFixedSize(height, height);
        hlayout->addWidget(acl);
        item_value_widgets.append(acl);
    }
    break;
    case SVT_QINT64:
        item_value_widgets.append(nullptr);
        break;
    }

    // 设置按钮
    if (sb->relates.size() > 0)
    {
        QHBoxLayout *btn_hlayout = new QHBoxLayout;

        // 遍历每一个子项设置
        foreach (SettingBean *relate, sb->relates)
        {
            InteractiveButtonBase *rbtn = new InteractiveButtonBase(relate->tooltip, btn);
            rbtn->setToolTip(relate->description);
            rbtn->setPaddings(us->widget_radius + 2, 2);
            rbtn->setFixedForeSize();
            rbtn->setRadius(us->widget_radius / 2);
            // rbtn->setBorderColor(us->accent_color);
            rbtn->setBgColor(us->mainwin_bg_color);
            btn_hlayout->addWidget(rbtn);
            item_child_btns.append(rbtn);

            connect(rbtn, &InteractiveButtonBase::clicked, this, [=]{
                slotTriggerSettingItem(relate->key);
            });
        }
        btn_hlayout->addStretch(1);

        // 添加到主布局
        vlayout->addLayout(btn_hlayout);
    }
}

/**
 * 点击设置项，修改设置后，设置数据
 * 以及出现的动画
 */
void UserSettingsWindow2::refreshItemValue(SettingBean *sb)
{
    int index = -1;
    for (int i = 0; i < item_beans.size(); i++)
        if (item_beans.at(i) == sb)
            index = i;
    if (index == -1)
        return;
    QWidget *widget = (QWidget *)item_value_widgets.at(index);
    if (widget == nullptr)
        return;

    switch (sb->getType())
    {
    case SVT_BOOL:
    {
        AniSwitch *as = (AniSwitch *)widget;
        bool b = sb->getValue().toBool();
        if (sb->need_restart) // 需要重启的，直接获取到的属性不一定正确
            b = us->getBool("us/"+sb->key, b);
        as->slotSwitch(b);
    }
    break;
    case SVT_INTEGER:
    {
        AniNumberLabel *anl = (AniNumberLabel *)widget;
        int n = sb->getValue().toInt();
        if (sb->need_restart)
            n = us->getInt("us/"+sb->key, n);
        anl->setShowNum(n);
    }
    break;
    case SVT_STRING:
    {
        QLabel *label = (QLabel *)widget;
        label->setText(sb->getValue().toString());
    }
    break;
    case SVT_COLOR:
    {
        AniCircleLabel *acl = (AniCircleLabel *)widget;
        acl->setMainColor(qvariant_cast<QColor>(sb->getValue()));
    }
    break;
    default:
        break;
    }
}

void UserSettingsWindow2::startGroupSwitchAnimation(QPixmap pixmap_old, QPixmap pixmap_new, int index)
{
    if (current_group_index == -1 || index == current_group_index)
        return ;
    QLabel* label_old = new QLabel(this);
    QLabel* label_new = new QLabel(this);
    label_old->setGeometry(item_list->geometry());
    label_new->setGeometry(item_list->geometry());
    label_old->setPixmap(pixmap_old);
    label_new->setPixmap(pixmap_new);
    label_old->show();
    label_new->show();
    QPropertyAnimation* ani_old = new QPropertyAnimation(label_old, "pos");
    QPropertyAnimation* ani_new = new QPropertyAnimation(label_new, "pos");
    ani_old->setStartValue(label_old->pos());
    if (index > current_group_index) // 上移
    {
        ani_old->setEndValue(QPoint(item_list->geometry().left(), -item_list->height()));
        ani_new->setStartValue(QPoint(item_list->geometry().left(), this->height()));
    }
    else // 下移
    {
        ani_old->setEndValue(QPoint(item_list->geometry().left(), this->height()));
        ani_new->setStartValue(QPoint(item_list->geometry().left(), -item_list->height()));
    }
    ani_new->setEndValue(item_list->pos());
    ani_old->setDuration(300);
    ani_new->setDuration(300);
    ani_old->setEasingCurve(QEasingCurve::OutCubic);
    ani_new->setEasingCurve(QEasingCurve::OutCubic);
    ani_old->start();
    ani_new->start();
    connect(ani_old, SIGNAL(finished()), label_old, SLOT(deleteLater()));
    connect(ani_new, SIGNAL(finished()), label_new, SLOT(deleteLater()));
}

/**
 * 输入一个整数，并且设置最大值、最小值
 * @param title 标题
 * @param desc  描述
 * @param def   默认值
 * @param min   最小值
 * @param max   最大值
 * @param rst   结果（引用）
 * @return      是否成功
 */
bool UserSettingsWindow2::inputInt(QString title, QString desc, int def, int min, int max, int &rst)
{
    bool ok;
    QString text = QInputDialog::getText(this, title, desc, QLineEdit::Normal, QString("%1").arg(def), &ok);
    if (!ok)
        return false;                 // 没有输入
    if (!canRegExp(text, "^-?\\d+$")) // 不是纯数字
    {
        QMessageBox::information(this, tr("错误"), tr("请输入纯数字"));
        return false;
    }

    int x = text.toInt();
    if ((min != max) && (x < min || x > max)) // 超过范围
    {
        QMessageBox::information(this, tr("错误"), tr("请输入 %1~%2 之间的数字").arg(min).arg(max));
        return false;
    }

    rst = x;
    return true;
}

void UserSettingsWindow2::updateUI()
{
    setStyleSheet("QLabel{background:transparent;}");
    bg_widget->setStyleSheet("QWidget{background: " + us->getColorString(us->getNoOpacityColor(us->mainwin_bg_color)) + "; border: 0px; border-radius:"+QString::number(us->widget_radius)+"px; }");

    QString style = "QListWidget{background:transparent; border:none; padding: 5px 5px; }\
            QListWidget::Item{}\
            QListWidget::Item:hover{background:transparent; }\
            QListWidget::item:selected{background:transparent; }";
    group_list->setStyleSheet(style);
    item_list->setStyleSheet(style);

    thm->setWidgetStyleSheet(this->item_list->verticalScrollBar(), "scrollbar");

    QList<InteractiveButtonBase *> btns = group_btns + item_btns + item_child_btns;

    QColor bg_color = us->mainwin_sidebar_color;
    if (bg_color.alpha() == 0)
        bg_color.setAlpha(255);
    foreach (InteractiveButtonBase *button, btns)
    {
        button->setTextColor(us->global_font_color);
        button->setBgColor(bg_color);
        button->setBgColor(us->getOpacityColor(us->accent_color, 32), us->getOpacityColor(us->accent_color, 64));
    }
    foreach (InteractiveButtonBase* button, item_child_btns)
    {
        button->setBgColor(us->getNoOpacityColor(us->mainwin_bg_color));
    }
}

void UserSettingsWindow2::slotTriggerSettingItem(QString key)
{
    QList<SettingBean*> items = gd->sm.getItems();
    for (int i = 0; i < items.size(); i++)
    {
        if (items.at(i)->key == key)
        {
            slotTriggerSettingItem(items.at(i));
            break;
        }
    }
}

void UserSettingsWindow2::slotTriggerSettingItem(SettingBean *sb)
{
    QString key = sb->key;
    // 一、按类型统一判断
    if (sb->getType() == SVT_BOOL)
    {
        bool* p = (bool *)(sb->point);
        // 非即时生效的（需要重启）
        if (sb->need_restart)
        {
            bool b = *p;
            b = us->getBool("us/"+sb->key, b);
            us->setVal("us/" + key, !b);
        }
        // 不需要重启的
        else
        {
            *p = !*p;
            us->setVal("us/" + key, *p);
        }
    }
    else if (sb->getType() == SVT_INTEGER)
    {
        int* p = (int*)(sb->point);
        int x;
        QString desc = sb->tooltip.isEmpty()?sb->description:sb->tooltip;
        desc += QString("(%1~%2)").arg(sb->min).arg(sb->max);
        if (sb->need_restart)
            desc += " 重启生效";
        if (!inputInt(sb->title, desc, *p, sb->min, sb->max, x))
            return;

        if (!sb->need_restart)
        {
            *p = x;
        }
        us->setVal("us/" + key, x);
    }
    else if (sb->getType() == SVT_COLOR)
    {
        QColor* p = (QColor*)sb->point;
        QColor color = QColorDialog::getColor(*p, this, sb->title, QColorDialog::ShowAlphaChannel);
        if (!color.isValid())
            return;
        if (sb->need_restart)
        {
            us->setColor("us/" + key, color);
        }
        else
        {
            *p = color;
            us->setColor("us/" + key, color);
        }
    }
    else if (sb->getType() == SVT_STRING)
    {

    }
    else
    {

    }

    // 二、按照key进行个体操作
    if (key == "mainwin_bg_picture")
    {
        // 背景图存在，删除
        if (isFileExist(rt->IMAGE_PATH + "win_bg.jpg") || isFileExist(rt->IMAGE_PATH + "win_bg.png"))
        {
            if (QMessageBox::question(this, QObject::tr("是否删除背景图？"), QObject::tr("删除之后您可以重新设置成其他图片"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
                if (isFileExist(rt->IMAGE_PATH + "win_bg.png"))
                    deleteFile(rt->IMAGE_PATH + "win_bg.png");
                if (isFileExist(rt->IMAGE_PATH + "win_bg.jpg"))
                    deleteFile(rt->IMAGE_PATH + "win_bg.jpg");

                if (us->dynamic_bg_model == Dynamic_None && us->mainwin_bg_color.alpha() != 255) // 可能是被之前颜色覆盖掉的
                {
                    us->mainwin_bg_color.setAlpha(255); // 恢复完全不透明色
                    us->setColor("us/mainwin_bg_color", us->mainwin_bg_color);
                }

                emit thm->signalBgPicturesChanged();
            }
            return;
        }

        QString recent = us->getStr("recent/file_path");
        if (recent == "" || !isFileExist(recent))
            recent = ".";
        QString path = QFileDialog::getOpenFileName(this, tr("选择背景图片"), recent, tr("Image Files(*.jpg *.png)"));
        if (path.length() == 0)
            return;

        us->mainwin_bg_picture = path; // 这个path好像是带双引号的？（至少debug的时候是带有双引号的）
        us->setVal("recent/file_path", path);

        ensureDirExist(rt->IMAGE_PATH);

        if (path.endsWith("png"))
            copyFile(path, rt->IMAGE_PATH + "win_bg.png");
        else if (path.endsWith("jpg"))
        {
            copyFile(path, rt->IMAGE_PATH + "win_bg.jpg");

            if (us->dynamic_bg_model != Dynamic_None) // 先去掉微动背景
            {
                us->dynamic_bg_model = Dynamic_None;
                us->setVal("dybg/model", 0);
                us->loadDynamicBackground();
            }
            if (us->mainwin_bg_color.alpha() == 255) // 如果不透明背景的话，会覆盖背景色
            {
                us->mainwin_bg_color.setAlpha(128); // 设置成50%透明
                us->setColor("us/mainwin_bg_color", us->mainwin_bg_color);
            }
        }
        else
            copyFile(path, rt->IMAGE_PATH + "win_bg.jpg");
    }
    else if (key == "dynamic_background")
    {
        DynamicBackgroundSettings dbs(this);
        dbs.setModal(true);
        dbs.exec();
    }
    else if (key == "accent_color")
    {
        // 修改其他相对应的点缀色
        QColor color = us->accent_color;
        us->modifyToAccentColorIfIsAccent(us->editor_underline_color, color, "us/editor_underline_color");
        us->modifyToAccentColorIfIsAccent(us->quote_colorful_color, color, "us/quote_colorful_color");
        us->modifyToAccentColorIfIsAccent(us->editor_bg_selection, color, "us/editor_bg_selection");
    }
    if (key == "editor_line_bg")
    {
        bool &b = us->editor_line_bg;
        if (!b) // 关闭，先全部设置成透明
        {
            b = true;
            QColor color(us->editor_line_bg_color);
            us->editor_line_bg_color = QColor(0, 0, 0, 0);
            thm->updateEditor();
            us->editor_line_bg_color = color;
            b = false;
        }
        else
        {
            thm->updateEditor();
        }
    }
    else if (key == "clip_global")
    {
        if (us->clip_global)
            gd->boards.startConnectClipboard();
        else
            gd->boards.stopConnectClipboard();
    }
    else if (key == "board_count")
    {
        gd->boards.setBoardCount(us->board_count);
    }
    else if (key == "clip_limit")
    {
        gd->boards.setLimits(us->clip_limit, us->recycle_limit);
    }
    else if (key == "recycle_limit")
    {
        gd->boards.setLimits(us->clip_limit, us->recycle_limit);
    }
    else if (key == "auto_detect_update")
    {
        if (us->auto_detect_update)
        {
            us->setVal("runtime/check_update", 0);
            emit signalDetectUpdate();
        }
    }
    else if (key == "editor_line_spacing")
    {
        if (us->editor_line_spacing == 0) // 关闭：先设置为1，再关闭（只差1应该不怎么看得出来的吧？）
        {
            sb->description = "已关闭，修改任意内容后应用修改";
        }
        else
        {
            sb->description = "若在编辑或者打开章节时出现明显卡顿，请关闭此项";
        }
    }
    else if (key == "climax_threshold")
    {
        if (us->climax_value > us->climax_threshold)
        {
            us->climax_value = us->climax_threshold;
            us->setVal("us/climax_value", us->climax_value);
        }
    }
    else if (key == "lexicon_synonym")
    {
        if (us->lexicon_synonym)
            gd->lexicons.initSynonym();
        else
            gd->lexicons.uninitAll(true, false, false, false);
    }
    else if (key == "lexicon_related")
    {
        if (us->lexicon_related)
            gd->lexicons.initRelated();
        else
            gd->lexicons.uninitAll(false, true, false, false);
    }
    else if (key == "lexicon_sentence")
    {
        if (us->lexicon_sentence)
            gd->lexicons.initSentence();
        else
            gd->lexicons.uninitAll(false, false, true, false);
    }
    else if (key == "lexicon_random")
    {
        if (us->lexicon_random)
            gd->lexicons.initRandomName();
        else
            gd->lexicons.uninitAll(false, false, false, true);
    }
    else if (key == "lexicon_surname")
    {
        if (us->lexicon_surname)
            gd->lexicons.initRandomName();
        else
            gd->lexicons.uninitAll(false, false, false, true);
    }
    else if (key == "homonym_cover")
    {
        HomonymCoverWindow hcw(this);
        hcw.setModal(true);
        hcw.exec();

        sb->tooltip = QString("黑名单(%1)").arg(us->homonym_cover_data.black_list.size());
    }
    else if (key == "about")
    {
        QDesktopServices::openUrl(QUrl(QString("https://shang.qq.com/wpa/qunwpa?idkey=cb424b14c6897cca5c0c10b5cd63e9db61502be5fcadbea5c8151add07689c9e")));
    }
    else if (key == "QQ_qun")
    {
        QDesktopServices::openUrl(QUrl(QString("https://shang.qq.com/wpa/qunwpa?idkey=cb424b14c6897cca5c0c10b5cd63e9db61502be5fcadbea5c8151add07689c9e")));
    }
    else if (key == "global_font")
    {
        bool ok;
        QFont font(this->font());
        font = QFontDialog::getFont(&ok, font, this);
        if (ok)
        {
            QApplication::setFont(font);
            us->setVal("us/font", font.toString());
            sb->description = "强烈建议重启，避免字体显示问题。";
            log("设置字体", font.toString());
        }
        else
        {
            log("设置字体失败", font.toString());
        }
    }
    else if (key == "editor_underline_offset")
    {
        sb->tooltip = "向下偏移量 (" + QString::number(us->editor_underline_offset) + ")";
    }

    // 三、判断影响的信号槽
    if (key == "blur_picture_radius" || key == "mainwin_bg_picture" || key == "blur_picture_radius")
    {
        emit thm->signalBgPicturesChanged();
    }
    else if (key == "mainwin_bg_color" || key == "mainwin_sidebar_color" || key == "accent_color"
        || key == "global_font_color")
    {
        thm->updateGlobal();
        thm->updateWindow();
    }
    else if (key == "editor_bg_color" || key == "editor_bg_selection" || key == "editor_font_size"
        || key == "editor_font_color" || key == "editor_font_selection" || key == "editor_cursor_width"
        || key == "editor_line_spacing" || key == "completer_maxnum" || key == "editor_line_bg")
    {
        thm->updateEditor();
    }
    else if (key == "emotion_filter_enabled")
    {
        emit us->signalEmotionFilterChanged();
    }
    else if (key == "environment_pictures_enabled")
    {
        emit us->signalEnvironmentPicturesChanged();
    }
    else if (key == "editor_fuzzy_titlebar")
    {
        emit us->signalBlurTitlebarChanged(us->editor_fuzzy_titlebar);
    }
    else if (key == "quote_colorful" || key == "chapter_mark" || key == "editor_underline_color"
            || key == "quote_colorful_color" || key == "chapter_mark_color")
    {
        thm->updateHighlight();
    }
    else if (key == "scroll_bottom_blank" || key == "scroll_bottom_fixed" || key == "scroll_cursor_fixed")
    {
        emit signalEditorBottomSettingsChanged();
    }

    // 四、设置界面显示的数据
    us->sync();
    refreshItemValue(sb);
}
