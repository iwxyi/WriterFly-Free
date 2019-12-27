#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include "globalvar.h"
#include "interactivebuttonbase.h"
#include "watercirclebutton.h"

#define DEFAULT_SPACING 4

class MainMenuWidget : public QWidget
{
    Q_OBJECT
public:
    MainMenuWidget(QWidget* parent = nullptr);

    void toShow();
    void toHide();

    void setOpenedChapterCount(int x);

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    void initView();
    void initStyle();
    void initEvent();
    void refreshData();
    void emitHide();

public slots:
    void updateUI();
    void slotFocusChanged(QWidget*, QWidget* cur);

signals:
    void signalHidded(); // Popup 被动隐藏

    void signalHide(); // 主动发出要隐藏的信号

    void signalProfileClicked();
    void signalCloudClicked();
    void signalWordClicked();
    void signalTimeClicked();
    void signalRankClicked();
    void signalSpeedClicked();
    void signalTodayClicked();
    void signalThemeClicked();
    void signalNightClicked();
    void signalDarkRoomClicked();
    void signalViewportClicked();
    void signalCloseOpenedClicked();
    void signalFullScreenClicked();
    void signalSettingsClicked();
    void signalHelpClicked();

private:
    InteractiveButtonBase* state_panel;
    InteractiveButtonBase* profile_label;
    QLabel* nickname_label;
    QLabel* sync_state_label;
    QLabel* level_label;
    InteractiveButtonBase* right_arrow;

    InteractiveButtonBase* word_button;
    QLabel* word_count_label;
    QLabel* word_label;
    InteractiveButtonBase* time_button;
    QLabel* time_count_label;
    QLabel* time_label;
    InteractiveButtonBase* rank_button;
    QLabel* rank_count_label;
    QLabel* rank_label;

    QWidget* speed_today_widget;
    InteractiveButtonBase* speed_today_panel;
    QLabel* speed_label;
    QLabel* today_label;

    InteractiveButtonBase* theme_button;
    InteractiveButtonBase* dark_room_button;
    InteractiveButtonBase* viewport_button;
    InteractiveButtonBase* close_opened_button;
    InteractiveButtonBase* theme_label;

    InteractiveButtonBase* full_screen_button;
    InteractiveButtonBase* settings_button;
    InteractiveButtonBase* help_button;

    QLabel* hsplit1, *hsplit2, *vsplit1, *vsplit2;

    bool is_showing;
    bool is_hidding;
    QGraphicsOpacityEffect* effect;
};

#endif // MAINMENUWIDGET_H
