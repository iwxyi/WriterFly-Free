#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QWidget>
#include "customtitlebar.h"

/**
 * 自定义标题栏窗口[模板]
 */
class CustomTitleBarWindow : public QWidget
{
    Q_OBJECT

public:
    CustomTitleBarWindow(QWidget *parent = nullptr);
    ~CustomTitleBarWindow();

private:
    void initTitleBar();
    void paintEvent(QPaintEvent *event);
    void loadStyleSheet(const QString &sheet_name);

private slots:
    void onButtonMinClicked();
    void onButtonRestoreClicked();
    void onButtonMaxClicked();
    void onButtonCloseClicked();

protected:
    CustomTitleBar* m_titlebar;

};

#endif // BASEWINDOW_H
