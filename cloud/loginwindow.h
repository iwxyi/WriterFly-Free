#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegExpValidator>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include "interactivebuttonbase.h"
#include "waterfloatbutton.h"
#include "watercirclebutton.h"
#include "globalvar.h"
#include "dynamicbackgroundgradient.h"
#include "netutil.h"
#include "mydialog.h"

class LoginWindow : public MyDialog
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent* e);

private:
    void initView();
    void initLayout();
    void initStyle();
    void initEvent();
    void initData();
    void initSignal();

    void switchPage(int p = 0);
    QPropertyAnimation* startHeightAnimation(QWidget* widget, int start_value, int end_value);

    void showLoading();
    void hideLoading();

public slots:
    void loginBtnClicked();


private:
	InteractiveButtonBase* login_page;
	InteractiveButtonBase* register_page;
	InteractiveButtonBase* logo_btn;
    QLabel* tip_label;
    QWidget* divider_widget;
    QWidget* form_panel;
    InteractiveButtonBase* username_label;
    InteractiveButtonBase* password_label;
    InteractiveButtonBase* password_check_label;
    InteractiveButtonBase* nickname_label;
    QLineEdit* username_edit;
    QLineEdit* password_edit;
    QLineEdit* password_check_edit;
    QLineEdit* nickname_edit;
    InteractiveButtonBase* login_btn;

    QLabel* gray_label;
    QLabel* movie_label;
    QMovie* loading_movie;
    InteractiveButtonBase* ani_pic;

    int page;
};

#endif // LOGINWINDOW_H
