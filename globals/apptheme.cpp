#include "apptheme.h"

AppTheme::AppTheme(RuntimeInfo* rt, USettings* us)
{
    setRt(rt);
    setUs(us);
}

void AppTheme::setRt(RuntimeInfo *rt)
{
    this->rt = rt;
}

void AppTheme::setUs(USettings *us)
{
    this->us = us;
    theme_name = us->getStr("recent/theme_name");
}

void AppTheme::updateWindow()
{
    emit windowChanged();
}

void AppTheme::updateEditor()
{
    emit editorChanged();
}

void AppTheme::updateHighlight()
{
    emit hlightChanged();
}

void AppTheme::updateGlobal()
{
    QColor tc = us->global_font_color, bc = us->getNoOpacityColor(us->mainwin_bg_color);

    QPalette pa(QApplication::palette());

    pa.setColor(QPalette::WindowText, tc);
    pa.setColor(QPalette::Text, tc);
    pa.setColor(QPalette::ButtonText, tc);
    pa.setColor(QPalette::ToolTipText, tc);

    pa.setColor(QPalette::Base, bc);
    pa.setColor(QPalette::Window, bc);
    pa.setColor(QPalette::ToolTipBase, bc);
    pa.setColor(QPalette::Background, bc); // 可以修改右键菜单的背景
    pa.setColor(QPalette::Button, bc);
    pa.setColor(QPalette::AlternateBase, bc);
    pa.setColor(QPalette::BrightText, bc);

    QApplication::setPalette(pa);
}

bool AppTheme::isNight()
{
    return theme_name == NIGHT_THEME;
}

void AppTheme::setWidgetStyleSheet(QWidget *widget, QString name, QStringList values)
{
    QString ss = getStyleSheet(name, values);
    if (ss.isEmpty()) return ;
    widget->setStyleSheet(ss);
}

void AppTheme::setWidgetStyleSheet(QApplication *widget, QString name, QStringList values)
{
    QString ss = getStyleSheet(name, values);
    if (ss.isEmpty()) return ;
    widget->setStyleSheet(ss);
}

QString AppTheme::getStyleSheet(QString name/*不带后缀*/, QStringList values)
{
    QString file_path = rt->DATA_PATH+"styles/"+name+".qss";     // 用户的主题
    QString source_path = ":/styles/"+name; // 默认主题
    QString sheet = "";
    if (isFileExist(file_path))
        sheet = readTextFile(file_path);
    else if (isFileExist(source_path))
        sheet = readTextFile(source_path);
    else return "";

    // 使用主色调
    if (!sheet.isEmpty())
    {
        // 替换颜色
        QList<QString> cstrs;
        QList<QColor> colors;
        cstrs << "主色调" << "副色调" << "点缀色" << "文字色";
        colors  << us->getNoOpacityColor(us->mainwin_bg_color)
                << us->mainwin_sidebar_color
                << us->accent_color
                << us->global_font_color;
        for (int i = 0; i < cstrs.size(); i++)
        {
            QString string = cstrs.at(i);
            QColor color = colors.at(i);
            // 替换无透明度变化的
            {
                QRegExp re("\\【"+string+"\\】");
                sheet = sheet.replace(re, us->getColorString(color));
            }

            // 替换有透明度变化的
            {
                // 寻找结果
                QRegExp re("【"+string+"(\\d+)】");
                QStringList sl; // 使用字符串数组而不用整数的原因是，避免有 064 这种数字
                int pos = 0;
                while ((pos = re.indexIn(sheet, pos)) != -1)
                {
                    QString cap = re.cap(1);
                    if (!sl.contains(cap))
                        sl << cap;
                    pos += re.matchedLength();
                }

                // 开始替换
                for (int j = 0; j < sl.size(); j++)
                {
                    QRegExp r("【"+string+sl.at(j)+"】");
                    sheet = sheet.replace(r, us->getOpacityColorString(color, sl.at(j).toInt()));
                }
            }
        }

        // 替换默认值
        for (int i = 0; i < values.size()-1; i+=2)
        {
            QString string = values.at(i);
            QString value = values.at(i+1);

            QRegExp re(string);
            sheet = sheet.replace(re, value);
        }
    }

    return sheet;
}

QIcon AppTheme::icon(QString name)
{
    if (isFileExist(rt->STYLE_PATH + "icons/" + name + ".png"))
        return QIcon(rt->STYLE_PATH + "icons/" + name + ".png");
    return QIcon(":/icons/" + name);
}

QString AppTheme::iconPath(QString name)
{
    if (isFileExist(rt->STYLE_PATH + "icons/" + name + ".png"))
        return QString(rt->STYLE_PATH + "icons/" + name + ".png");
    return QString(":/icons/" + name);
}

QString AppTheme::getResource(QString related_path)
{
    if (isFileExist(QApplication::applicationDirPath() + "data/" + related_path))
        return QApplication::applicationDirPath() + "data/" + related_path;
    return ":/" + related_path;
}

/**
 * 当前主题的名字，默认为空
 * @return 主题名字
 */
QString AppTheme::getThemeName()
{
    return theme_name;
}

/**
 * 保存主题到自己的文件夹，从而可以读取其他的主题
 */
void AppTheme::saveCurrentTheme(QString name)
{
    if (name.isEmpty())
        name = "自定义";
    QString dir = rt->THEME_PATH + name + "/";
    QString path = dir+"theme.ini";
    ensureDirExist(dir);
    Settings ss(path);

    ss.setColor("mainwin_bg_color", us->mainwin_bg_color);
    ss.setColor("mainwin_sidebar_color", us->mainwin_sidebar_color);
    ss.setColor("global_font_color", us->global_font_color);
    ss.setColor("accent_color", us->accent_color);
    ss.setColor("editor_font_color", us->editor_font_color);
    ss.setColor("editor_bg_selection", us->editor_bg_selection);
    ss.setColor("editor_font_selection", us->editor_font_selection);
    ss.setColor("editor_line_bg_color", us->editor_line_bg_color);
    ss.setColor("editor_underline_color", us->editor_underline_color);
    ss.setColor("editor_water_color", us->editor_water_color);
    ss.setColor("quote_colorful_color", us->quote_colorful_color);
    ss.setColor("chapter_mark_color", us->chapter_mark_color);
    ss.setVal("dybg/model", us->dynamic_bg_model);
    ss.setVal("split_shade", us->split_shade);
    if (us->dynamic_bg_model == Dynamic_Pure)
    {
        ss.setColor("dybg/pure_color1", us->getColor("dybg/pure_color1",QColor(255, 250, 240)/*花卉白*/));
        ss.setColor("dybg/pure_color2", us->getColor("dybg/pure_color2",QColor(253, 245, 230)/*舊蕾絲色*/));
    }
    else if (us->dynamic_bg_model == Dynamic_Gradient)
    {
        ss.setColor("dybg/gradient_color1", us->getColor("dybg/gradient_color1", QColor(230, 230, 250)/*薰衣草紫*/));
        ss.setColor("dybg/gradient_color2", us->getColor("dybg/gradient_color2", QColor(204, 204, 255)/*长春花色*/));
        ss.setColor("dybg/gradient_color3", us->getColor("dybg/gradient_color3", QColor(240, 248, 255)/*爱丽丝蓝*/));
        ss.setColor("dybg/gradient_color4", us->getColor("dybg/gradient_color4", QColor(248, 248, 253)/*幽灵白*/));
        ss.setVal("dybg/gradient_horizone", us->getBool("dybg/gradient_horizone", false));
        ss.setVal("dybg/interval", us->getInt("dybg/interval", 2000));
    }

    // 删除原来的主题素材文件夹
    if (isFileExist(dir+"images"))
        deleteDir(dir+"images");
    if (isFileExist(dir+"icons"))
        deleteDir(dir+"icons");
    if (isFileExist(dir+"styles"))
        deleteDir(dir+"styles");

    // 复制当前的到主题文件夹中
    if (isFileExist(rt->IMAGE_PATH))
        copyDir(rt->IMAGE_PATH, dir+"images", true);
    if (isFileExist(rt->ICON_PATH))
        copyDir(rt->ICON_PATH, dir+"icons", true);
    if (isFileExist(rt->STYLE_PATH))
        copyDir(rt->STYLE_PATH, dir+"styles", true);
}

void AppTheme::loadTheme(QString name, bool cover)
{
    QString dir = rt->THEME_PATH + name + "/";
    QString path = dir+"theme.ini";
    if (!isFileExist(path))
    {
        if (name == NIGHT_THEME)
        {
            createDefaultNightTheme();
        }
        else
        {
            // QMessageBox::information(nullptr, QObject::tr("找不到主题："+name+"。可能是主题文件残缺，无法使用。"));
            // qDebug() << "找不到主题：" << name << path;
            return ;
        }
    }

    if (name == NIGHT_THEME && theme_name != name) // 表示是切换到夜间，并且避免连续两次切换到夜间，使自定义也变成夜间
    {
        saveCurrentTheme(USER_THEME); // 保存到自定义，夜间取消的时候恢复这个主题
    }
    // 如果不是夜间模式，就不在切换的时候保存到“自定义”主题上了

    us->setVal("recent/theme_name", theme_name = name); // 保存新的主题到设置，下次打开为这个

    Settings ss(path); // 从这个文件中读取主题配置
    us->mainwin_bg_color = ss.getColor("mainwin_bg_color", us->mainwin_bg_color);
    us->mainwin_sidebar_color = ss.getColor("mainwin_sidebar_color", us->mainwin_sidebar_color);
    us->global_font_color = ss.getColor("global_font_color", us->global_font_color);
    us->accent_color = ss.getColor("accent_color", us->accent_color);
    us->editor_font_color = ss.getColor("editor_font_color", us->editor_font_color);
    us->editor_bg_selection = ss.getColor("editor_bg_selection", us->editor_bg_selection);
    us->editor_font_selection = ss.getColor("editor_font_selection", us->editor_font_selection);
    us->editor_line_bg_color = ss.getColor("editor_line_bg_color", us->editor_line_bg_color);
    us->editor_underline_color = ss.getColor("editor_underline_color", us->editor_underline_color);
    us->editor_water_color = ss.getColor("editor_water_color", us->editor_water_color);
    us->quote_colorful_color = ss.getColor("quote_colorful_color", us->quote_colorful_color);
    us->chapter_mark_color = ss.getColor("chapter_mark_color", us->chapter_mark_color);
    us->dynamic_bg_model = (DynamicBgModel)ss.getInt("dybg/model", us->dynamic_bg_model);

    us->setColor("us/mainwin_bg_color", us->mainwin_bg_color);
    us->setColor("us/mainwin_sidebar_color", us->mainwin_sidebar_color);
    us->setColor("us/global_font_color", us->global_font_color);
    us->setColor("us/accent_color", us->accent_color);
    us->setColor("us/editor_font_color", us->editor_font_color);
    us->setColor("us/editor_bg_selection", us->editor_bg_selection);
    us->setColor("us/editor_font_selection", us->editor_font_selection);
    us->setColor("us/editor_line_bg_color", us->editor_line_bg_color);
    us->setColor("us/editor_water_color", us->editor_water_color);
    us->setColor("us/quote_colorful_color", us->quote_colorful_color);
    us->setColor("us/chapter_mark_color", us->chapter_mark_color);
    us->setVal("dybg/model", us->dynamic_bg_model);
    if (us->dynamic_bg_model == DynamicBgModel::Dynamic_Pure)
    {
        us->setColor("dybg/pure_color1", ss.getColor("dybg/pure_color1", QColor(255, 250, 240)/*花卉白*/));
        us->setColor("dybg/pure_color2", ss.getColor("dybg/pure_color2", QColor(253, 245, 230)/*舊蕾絲色*/));
    }
    else if (us->dynamic_bg_model == DynamicBgModel::Dynamic_Gradient)
    {
        us->setColor("dybg/gradient_color1", ss.getColor("dybg/gradient_color1", QColor(230, 230, 250)/*薰衣草紫*/));
        us->setColor("dybg/gradient_color2", ss.getColor("dybg/gradient_color2", QColor(204, 204, 255)/*长春花色*/));
        us->setColor("dybg/gradient_color3", ss.getColor("dybg/gradient_color3", QColor(240, 248, 255)/*爱丽丝蓝*/));
        us->setColor("dybg/gradient_color4", ss.getColor("dybg/gradient_color4", QColor(248, 248, 253)/*幽灵白*/));
    }

    // 加载图标
    bool icon_changed = false;
    if (!cover && isFileExist(rt->ICON_PATH))
    {
        deleteDir(rt->ICON_PATH);
        icon_changed = true;
    }
    if (isFileExist(dir + "icons"))
    {
        copyDir(dir + "icons", rt->ICON_PATH, true);
        icon_changed = true;
    }
    if (icon_changed)
        emit signalIconsChanged();

    // 加载图片
    bool image_changed = false;
    if (!cover && isFileExist(rt->IMAGE_PATH))
    {
        deleteDir(rt->IMAGE_PATH);
        image_changed = true;
    }
    if (isFileExist(dir + "images"))
    {
        copyDir(dir + "images", rt->IMAGE_PATH, true);
        image_changed = true;
    }
    if (image_changed)
        emit signalBgPicturesChanged();

    // 加载样式表
    if (!cover && isFileExist(rt->STYLE_PATH))
    {
        deleteDir(rt->STYLE_PATH);
    }
    if (isFileExist(dir + "styles"))
    {
        copyDir(dir + "styles", rt->STYLE_PATH, true);
    }

    // 加载情绪滤镜
    if (isFileExist(dir + "emotion_filter.txt"))
    {
        ensureDirExist(rt->DBASE_PATH + "emotion_filter_extra");
        copyFile(dir + "emotion_filter.txt", rt->DBASE_PATH + "emotion_filter_extra/" + theme_name + ".txt", true);
    }

    // 加载场景图片
    if (isFileExist(dir + "environment_pictures.txt") && isFileExist(dir + "environment_pictures/"))
    {
        ensureDirExist(rt->DBASE_PATH + "environment_pictures_extra");
        copyDir(dir + "environment_pictures/", rt->DBASE_PATH + "environment_pictures/");
        copyFile(dir + "environment_pictures.txt", rt->DBASE_PATH + "environment_pictures_extra/" + theme_name + ".txt", true);
    }

    // 加载定时祝福语
    if (isFileExist(dir + "warm_wish.txt"))
    {
        ensureDirExist(rt->DBASE_PATH + "warm_wish_extra");
        copyFile(dir + "warm_wish.txt", rt->DBASE_PATH + "warm_wish_extra/" + theme_name + ".txt", true);
    }

    us->loadDynamicBackground();
    updateGlobal();
    updateWindow();
    updateEditor();
}

void AppTheme::createDefaultNightTheme()
{
    QString dir = rt->THEME_PATH+NIGHT_THEME;
    QString path = rt->THEME_PATH+NIGHT_THEME+"/theme.ini";
    ensureDirExist(dir);
    Settings ss(path);

    ss.setColor("mainwin_bg_color", QColor(54,52,55));
    ss.setColor("mainwin_sidebar_color", QColor(54,52,55, 0));
    ss.setColor("global_font_color", QColor(255, 255, 255, 140));
    ss.setColor("accent_color", us->accent_color);
    ss.setColor("editor_font_color", QColor(255, 255, 255, 178));
    ss.setColor("editor_bg_selection", QColor(105, 105, 105));
    ss.setColor("editor_font_selection", QColor(255, 255, 255, 178));
    ss.setColor("editor_line_bg_color", QColor(150, 150, 150, 32));
    ss.setColor("editor_underline_color", QColor(150, 150, 150, 32));
    ss.setColor("quote_colorful_color", us->accent_color);
    ss.setColor("chapter_mark_color", QColor(255, 255, 255, 140));
    ss.setVal("dybg_model", 0);
    ss.setColor("dybg/pure_color1", QColor(54,52,55));
    ss.setColor("dybg/pure_color2", QColor(54,52,55));
    ss.setColor("dybg/gradient_color1", QColor(54,52,55));
    ss.setColor("dybg/gradient_color2", QColor(54,52,55));
    ss.setColor("dybg/gradient_color3", QColor(54,52,55));
    ss.setColor("dybg/gradient_color4", QColor(54,52,55));
    ss.setVal("dybg/gradient_horizone", QColor(54,52,55));
    ss.setVal("dybg/interval", 2000);
}
