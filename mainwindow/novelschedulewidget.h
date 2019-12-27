#ifndef NOVELINFOWIDGET_H
#define NOVELINFOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDateTime>
#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QList>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QShowEvent>
#include "globalvar.h"
#include "settings.h"
#include "fileutil.h"
#include "stringutil.h"
#include "zoomgeometryanimationwidget.h"
#include "defines.h"
#include "mylabel.h"
#include "noveldirlistview.h"
#include "im_ex_window.h"
#include "generalbuttoninterface.h"
#include "circlebutton.h"
#include "squarebutton.h"
#include "interactivebuttonbase.h"
#include "watercirclebutton.h"

/**
 * 小说一览界面
 */
class NovelScheduleWidget : public QWidget
{
    Q_OBJECT
public:
    NovelScheduleWidget(NovelDirListView* listview, QWidget *parent);

    void initLayout();
    void initStyle();
    void initEvent();
    void initData();
    void adjustGeometry(QPoint point);

    void toShow(QString name, QPoint point);
    void toHide();

protected:
    void paintEvent(QPaintEvent* event);
    //void showEvent(QShowEvent * event);
    void keyPressEvent(QKeyEvent* event);

private:
    int getWordCount(QString name);
    void getRecentChapter(QString name);
    QString getRcCount();
    QString getCreateDays(QString name);

    int max(int a, int b);
    int getTextWordCount(QString text);
    int compByTime(QFileInfo a, QFileInfo b);
    QString getFirstPara(QString text);
    QString getTimeDelta(long long timestamp0);

signals:
    void signalEsc();
    void signalToHide();
    void signalChangeNovelName(); // 修改书名
    void signalDeleteNovel(); // 删除书籍
    void signalImExPort(int kind, QString def_name);
    void signalOpenRecycle(); // 打开回收站
    void signalRecycleRecover(); // 恢复章节
    void signalOpenDirSettings(); // 打开目录设置
    void signalDirUndoOperator(); // 撤销操作（比如拖动目录）
    void signalDirSettings();
    void signalRefreshBookList(QString);

public slots:
    void updateUI();
    void slotShowMenu(QPoint point);
    void slotShowMenu();
    void slotChangeNovelNameFinished(QString old_name, QString new_name);
    void slotDuplicateBook(QString novel_name);

private:
    // 数值变量
    QString novel_name;
    Settings* ns;
    QPoint callback_point;
    int r_count, c_count;
    QString last_modify_names[10]; // 这个是文件编码后的，需要解码
    QString font_family;
    NovelDirListView* dir_listview;
    QColor bg_color;

    // 可视控件
    QImage* cover_image;
    MyLabel* cover_label;
    InteractiveButtonBase* novel_label;
    InteractiveButtonBase* type_label;
    InteractiveButtonBase* author_label;
    InteractiveButtonBase* wc_label;
    QTextEdit* desc_edit;
    MyLabel* rc_count_label;
    QWidget* recent_widget;
    MyLabel* last_label;
    InteractiveButtonBase *last_c_label[3];
    MyLabel* days_label;
    InteractiveButtonBase* operator_button;

    // 动作
    QMenu* operator_menu;
    QAction* undo_action;
    QAction* settings_action;
    QAction* export_action;
    QAction* import_action;
    QAction* import_mzfy_action;
    QAction* rename_action;
    QAction* search_action;
    QAction* duplicate_action;
    QAction* delete_action;
    QAction* recycle_action;

};

#endif // NOVELINFOWIDGET_H
