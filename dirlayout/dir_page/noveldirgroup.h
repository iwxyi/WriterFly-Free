#ifndef NOVELDIRLAYOUT_H
#define NOVELDIRLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>
#include <QCursor>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include "defines.h"
#include "noveldirlistview.h"
#include "anifbutton.h"
#include "splitterwidget.h"
#include "settings.h"
#include "circlebutton.h"
#include "circlefloatbutton.h"
#include "generalbuttoninterface.h"
#include "noveltools.h"
#include "generalcombobox.h"
#include "interactivebuttonbase.h"
#include "watercirclebutton.h"
#include "waterfloatbutton.h"

/**
 * 完整的小说目录控件，包括作品列表、章节列表、交互按钮、更改界面宽高等
 */
class NovelDirGroup : public QWidget
{
    Q_OBJECT
public:
    NovelDirGroup(QWidget *parent = nullptr);

public:
    NovelDirListView *getListView();

    void initRecent();
    bool execCommand(QString command);
    bool execCommand(QString command, QStringList args);

protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent* event);

private:
    void initView();
    void initStyle();
    void initData();

signals:
    void signalOpenNovel(QString novelName);
    void signalRenameNovel(QString oldName, QString newName);
    void signalDeleteNovel(QString novelName);
    void signalShowInfo(QString novelName, QPoint pos);

public slots:
    void updateUI();
    bool switchBook(int index);
    bool createBook();
    void readBookList(QString recent);
    void slotRenameNovel();
    void slotDeleteNovel();
    void slotNeedLineGuide();

    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void slotInfoClicked();
    void slotAddRollButtonClicked();
    void slotAddChapterButtonClicked();

    void slotImportFinished(QString name);

private:
    GeneralComboBox *book_switch_combo;
    NovelDirListView *novel_dir_listview;
    InteractiveButtonBase *add_roll_button;
    InteractiveButtonBase *add_chapter_button;
    InteractiveButtonBase *info_button;
    QHBoxLayout *btn_layout;
    QWidget *divider_line;

    QString novel_new_name_temp;
    bool novel_had_deleted_temp;
};

#endif // NOVELDIRECTORYLAYOUT_H
