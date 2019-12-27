#ifndef APPTHEME_H
#define APPTHEME_H

#include <QObject>
#include <QColor>
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include "usettings.h"
#include "runtimeinfo.h"
#include "fileutil.h"

#define USER_THEME "自定义"
#define NIGHT_THEME "夜间模式"

class AppTheme : public QObject
{
    Q_OBJECT
public:
    AppTheme(RuntimeInfo *rt, USettings *us);
    friend class ThemeManageWindow;

    void setRt(RuntimeInfo* rt);
    void setUs(USettings * us);

    void updateWindow();
    void updateEditor();
    void updateHighlight();
    void updateGlobal();

    bool isNight();

    void setWidgetStyleSheet(QWidget *widget, QString name, QStringList values = QStringList());
    void setWidgetStyleSheet(QApplication *widget, QString name, QStringList values = QStringList());
    QString getStyleSheet(QString name, QStringList values = QStringList());
    QString getResource(QString related_path);
    QIcon icon(QString name);
    QString iconPath(QString name);

    QString getThemeName();
    void saveCurrentTheme(QString name);
    void loadTheme(QString path, bool cover = false);

private:
    void createDefaultNightTheme();

signals:
    void windowChanged();
    void editorChanged();
    void hlightChanged();
    void signalBgPicturesChanged();
    void signalIconsChanged();

public:


private:
    RuntimeInfo* rt;
    USettings* us;
    QString theme_name; // 当前主题名字

};

#endif // APPTHEME_H
