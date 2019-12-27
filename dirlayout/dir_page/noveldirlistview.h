#ifndef NOVELDIR_H
#define NOVELDIR_H

#include <QWidget>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QAction>
#include <QLineEdit>
#include <QMimeData>
#include <QDrag>
#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>
#include <QFileDialog>
#include <QScrollBar>
#include <QDesktopServices>
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"
#include "globalvar.h"
#include "noveldirmodel.h"
#include "noveldirdelegate.h"
#include "noveldiritem.h"
#include "noveldirsettings.h"
#include "stackwidgetanimation.h"
#include "dirsettingswidget.h"
#include "noveltools.h"
#include "smoothscrollbean.h"
#if defined(Q_OS_WIN)
    #include <windows.h>
#endif

#define CHAPTER_FILEINFO_SIZE 30720
#define NOVEL_FILEINFO_SIZE 10485760

/**
 * 小说章节目录列表类
 */
class NovelDirListView : public QListView
{
    Q_OBJECT
public:
    NovelDirListView(QWidget *parent);

    void initStyle();
    void initRecent();
    void restoreDirectory();
    void initMenu();
    void initRowHidden();
    void initRowHidden(QModelIndex index);

    NovelDirModel* getModel();
    NovelDirDelegate* getDelegate();
    QString getNovelName();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyboardSearch(const QString &search);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void currentChange(const QModelIndex &current, const QModelIndex &previous);

    bool exportNovel(QString path);
    void setCurrentIndex(const QModelIndex &index);

private:
    int addRoll(QString newName);
    int addChapter(int index, QString newName);
    void editItem(int index, bool use_ani = true);
    void selectIndex(int index);

    QUrl getIndexFileTemp(QModelIndex index);
    QString getSuitableChapterName(int index, bool move_to_end = false);
    QString getChapterPath(QString name);
    bool processDropUrl(QString str);
    bool processDropText(QString str);
    bool processDropUrls(QStringList list);

    void addSmoothScrollThread(int distance, int duration);

signals:
    void signalCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void signalOpenChapter(QString novel_name, QString chapterName, QString fullChapterName);
    void signalOpenAnimationStartPos(QPoint point);

    void signalAddItemRect(QRect rect, QPixmap pixmap);
    void signalDeleteChapter(QString novel_name, QString chapterName);
    void signalDeleteChapters(QString novel_name, QStringList chapterNames);
    void signalChangeFullName(QString novel_name, QString chapter_name, QString full_name);

    void signalImExPort(int kind, QString def_name); // 和一览一样的信号
    void signalExportFinished();
    void signalImportFinished(QString novel_name);

    void signalNeedLineGuide();

public slots:
    void updateUI();

    void readNovel(QString novel_name); // Delegate 读取 settings，Model 读取 ChapterList
    void slotOpenChapter(int index);
    void slotOpenCurrentChapter();
    void slotOpenChapter(QString name);
    void slotChapterClosed(QString novel_name, QString chapter_name);

    void slotInsertRoll();
    void slotInsertRoll(QString new_name);
    void slotDeleteRoll();
    void slotRenameRoll();
    void slotRollWordCount();

    void slotInsertChapter();
    void slotInsertChapter(QString new_name, QString content);
    int slotAddNextChapter(QString new_name, QString content);
    void slotAddNextChapterFaster(QString new_name, QString content, bool open = false);
    void slotDeleteChapter();
    void slotRenameChapter();
    void slotChapterWordCount();
    void slotChangeFullName(QString novel_name, QString chapter_name);

    void slotLocateChapter(QString novel_name, QString chapter_name);
    void slotOpenPrevChapter(QString novel_name, QString chapter_name);
    void slotOpenNextChapter(QString novel_name, QString chapter_name);
    void slotMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text);
    void slotMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text);

    void slotAddRoll();
    void slotAddRoll(QString new_name);
    void slotAddChapter();
    void slotAddChapter(QString new_name, QString content);
    void slotInitRowHidden();

    void slotExternRect(QRect r);
    void slotExternPoint(QPoint p);
    void slotAddAnimationFinished(int index);

    void slotReedit();
    void slotFocuItem(int row);
    void slotUpdateIndex(QModelIndex index);

    void emitItemEvent(int index);
    void slotExport();
    void slotImport(QString file_path, bool split_roll, int index);

    void slotSmoothScrollDistance(SmoothScrollBean* bean, int dis);

    void refreshMenu();
    void actionInsert();
    void actionDelete();
    void actionRename();
    void actionWordCount();
    void actionBookSearch();
    void actionBookReplace();
    void actionReycle();

    void actionRollSplit();
    void actionRollCancel();

    void actionImmediateExport();
    void actionExport();
    void actionImport();
    void actionImportMzfy();

    void slotDirSettings();
    void actionShowWordCount();
    void actionChapterPreview();

private:
    // 主要内容
    QString novel_name;
    NovelDirModel *novel_dir_model;
    NovelDirDelegate *novel_dir_delegate;
    QList<NovelDirItem> roll_position_list; // 记录分组的位置

    // 各种标记
    NovelDirItem *current_item;
    QPoint drag_start_pos, drag_end_pos;
    QString drag_file_path;
    QRect extern_rect;
    QPoint extern_point;
    int pre_index;
    qint64 pressed_time1, pressed_time2;
    qint64 released_time;
    bool _flag_no_rename_by_added;

    // 右键菜单
    QMenu* menu;
    QAction* insert_action;
    QAction* delete_action;
    QAction* rename_action;
    QAction* word_count_action;
    QAction* book_search_action;
    QAction* book_replace_action;
    QAction* recycle_action;

    QMenu* directory_menu;
    QAction* roll_split_action;
    QAction* roll_cancel_action;

    QMenu* settings_menu;
    QAction* dir_settings_action;
    QAction* show_word_count_action;
    QAction* chapter_preview_action;

    QMenu* im_ex_menu;
    QAction* immediate_export_action;
    QAction* export_action;
    QAction* import_action;
    QAction* import_mzfy_action;

    // 动画相关
    QList<SmoothScrollBean*> smooth_scrolls;
};

#endif // NOVELDIR_H
