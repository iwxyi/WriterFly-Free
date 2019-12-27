#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QColor>
#include <QDebug>
//#include "globalvar.h"

/**
 * 自定义设置基础类
 */
class Settings : public QSettings
{
public :
    Settings(QString filePath) : QSettings(filePath, QSettings::IniFormat)
    {
        this->setIniCodec("GBK");
    }

    /*Settings(QString section = "default", QString fileName = "settings.ini")
            : QSettings(DataPath+fileName, QSettings::IniFormat)
    {
        if (section != "" && !section.endsWith("/"))
            section += "/";
        this->section = section;
        this->setIniCodec("GBK");
    }*/

    void setSection(QString section = "")
    {
        if (section != "" && !section.endsWith("/"))
            section += "/";
        this->section = section;
    }

    QString getSection()
    {
        return section;
    }

    void setVal(QString key, QVariant val)
    {
        setValue(section+key, val);
        sync();
    }

    QString getStr(QString key)
    {
        QVariant var = value(section+key);
        QString val = var.toString();
        return val;
    }

    QString getStr(QString key, QVariant def)
    {
        if (!contains(section+key))
            return def.toString();
        QString val = value(section+key).toString();
        return val;
    }

    int getInt(QString key)
    {
        QVariant var = value(section+key);
        int val = var.toInt();
        return val;
    }

    int getInt(QString key, QVariant def)
    {
        if (!contains(section+key))
            return def.toInt();
        int val = value(section+key).toInt();
        return val;
    }

    bool getBool(QString key)
    {
        QVariant var = value(section+key);
        bool val = var.toBool();
        return val;
    }

    bool getBool(QString key, QVariant def)
    {
        if (!contains(section+key))
            return def.toBool();
        bool val = value(section+key).toBool();
        return val;
    }

    long long getLongLong(QString key)
    {
        QVariant var = value(section+key);
        long long val = var.toLongLong();
        return val;
    }

    long long getLongLong(QString key, QVariant def)
    {
        if (!contains(section+key))
            return def.toLongLong();
        long long val = value(section+key).toLongLong();
        return val;
    }

    void setColor(QString key, QColor val)
    {
        setValue(key, QColorToInt(val));
    }

    QColor getColor(QString key)
    {
        QVariant var = value(section+key);
        int val = var.toInt();
        return IntToQColor(val);
    }

    QColor getColor(QString key, QColor def)
    {
        if (!contains(section+key))
            return def;
        int val = value(section+key).toInt();
        return IntToQColor(val);
    }

    QVariant getVar(QString key)
    {
        QVariant val = value(section+key);
        return val;
    }

    QVariant getVar(QString key, QVariant def)
    {
        if (!contains(section+key))
            return def;
        QVariant val = value(section+key);
        return val;
    }

    bool have(QString key)
    {
        return contains(section+key);
    }

    /**
     * 根据给定的颜色，返回自定义透明度（相对现有）的相同颜色
     */
    QString getOpacityColorString(QColor color, int transparent)
    {
        int r = color.red(), g = color.green(), b = color.blue();
        return QString("rgba(%1,%2,%3,%4)").arg(r).arg(g).arg(b).arg(transparent);
    }

    QColor getOpacityColor(QColor color, int alpha)
    {
        color.setAlpha(color.alpha()*alpha/255);
        return color;
    }

    /**
     * QColor颜色值转换为HTML/CSS颜色值，例如：#FFFFFF
     */
    QString getColorString(QColor color)
    {
        QString r = QString(QByteArray::number(color.red(), 16));
        QString g = QString(QByteArray::number(color.green(), 16));
        QString b = QString(QByteArray::number(color.blue(), 16));
        QString a = QString(QByteArray::number(color.alpha(), 16));
        if (r.length() == 1)
            r = "0"+r;
        if (g.length() == 1)
            g = "0"+g;
        if (b.length() == 1)
            b = "0"+b;
        if (a.length() == 1)
            a = "0"+a;
        if (color.alpha() == 255)
            return "#"+r+g+b;
        else
            return "#"+a+r+g+b;
    }

    /**
     * 无视颜色自带的透明度（如果小于min），使用绝对0~255
     * 一般用到使用主背景色作控件背景色，但是太透明又太难看了
     * @param color    颜色
     * @param opacity  绝对透明度，0~255
     * @param min      如果颜色小于min，就是用绝对透明度，否则使用相对透明度
     * @return         颜色值 #88888888
     */
    QString getFixedOpacityColor(QColor color, int opacity, int min = 250)
    {
        if (min > 0 && color.alpha() >= min)
            return getColorString(color);
        color.setAlpha(opacity);
        return getColorString(color);
    }

    /**
     * 获取一个不透明的图片
     * @param color
     * @return
     */
    QColor getNoOpacityColor(QColor color)
    {
        color.setAlpha(255);
        return color;
    }

    QColor getReverseColor(QColor color, bool highlight = true)
    {
        // ==== 保证颜色突出显示，和背景颜色不一样 ====
        if (highlight)
        {
            color.setRed(getHihglightColor(color.red()));
            color.setGreen(getHihglightColor(color.green()));
            color.setBlue(getHihglightColor(color.blue()));
        }
        else
        {
            color.setRed(255 - color.red());
            color.setGreen(255 - color.green());
            color.setBlue(255 - color.blue());
        }

        return color;
    }

private:
    int QColorToInt(const QColor &color)
    {
        //return (int)(((unsigned int)color.alpha()<< 24) | ((unsigned int)color.blue()<< 16) | (unsigned short)(((unsigned short)color.green()<< 8) | color.red()));
        return static_cast<int>(
                    (  static_cast<unsigned int>(color.alpha()<< 24)
                     | static_cast<unsigned int>(color.blue()<< 16)
                     | static_cast<unsigned int>(color.green()<< 8)
                     | static_cast<unsigned int>(color.red())
                    ));
    }

    QColor IntToQColor(const int &int_color)
    {
        int red = int_color & 255;
        int green = int_color >> 8 & 255;
        int blue = int_color >> 16 & 255;
        int alpha = int_color >> 24 & 255;
        return QColor(red, green, blue, alpha);
    }

    int getHihglightColor(int x)
    {
        if (x <= 96 || x >= 160)
        {
            return 255 - x;
        }
        return x < 128 ? 255 : 0;
    }

private :
    QString section;
};

#endif // SETTINGS_H
