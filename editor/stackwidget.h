#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include <QTabWidget>
#include <chaptereditor.h>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QList>
#include <QObject>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QTimer>
#include <QDateTime>
#include "stackwidgetanimation.h"
#include "editinginfo.h"
#include "globalvar.h"
#include "defines.h"
#include "zoomgeometryanimationwidget.h"
#include "editorsearchwidget.h"

/**
 *章节编辑器Tab层叠控件
 */
class StackWidget : public QStackedWidget
{
    Q_OBJECT
public:
    StackWidget(QWidget *parent);

public:
    void switchEditor(int index);
    void createEditor(QString novelName, QString chapterName, QString fullChapterName);
    ChapterEditor* getCurrentEditor();
    QList<ChapterEditor*>getEditors();
    QList<QTextEdit*> getTextEditors();
    void changeWindowTitle();
    void setTitleBar(QWidget* widget, QPixmap** p_bg);

    bool execCommand(QString command, QStringList args = QStringList());

protected:
    void resizeEvent(QResizeEvent *);

private:

signals:
    void signalEditorSwitched(QTextEdit* editor);
    void signalChapterClosed(QString novelName, QString chapterName);
    void signalAddNextChapter(QString title, QString content);
    void signalAddNextChapterFaster(QString title, QString content, bool open);
    void signalLocateOutline(QString novel_name, QString chapter_content);
    void signalLocateChapter(QString novel_name, QString chapter_name);
    void signalOpenPrevChapter(QString novel_name, QString chapter_name);
    void signalOpenNextChapter(QString novel_name, QString chapter_name);
    void signalMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text);
    void signalMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text);

    void signalChangeTitleByChapter(QString title);
    void signalEditorWordCount(int x);
    void signalSetTitleNovel(QString s);
    void signalSetTitleChapter(QString s);
    void signalSetTitleFull(QString s);
    // void signalSetTitleWc(int x);

    void signalOpenNewEditor(QPoint p);
    void signalShowSearchPanel(QString str);
    void signalEditorCountChanged(int x);

    void signalAddNewCard();
    void signalShowCardEditor(CardBean *card);

public slots:
    void slotRenameNovel(QString oldName, QString newName);
    void slotDeleteNovel(QString novelName);
    void slotOpenChapter(QString novelName, QString chapterName, QString fullChapterName);
    void slotOpenAnimation(QPoint point);
    void slotOpenAnimationFinished(int x);
    void slotDeleteChapter(QString novelName, QString chapterName);
    void slotDeleteChapters(QString novelName, QStringList chapterNames);
    void slotChangeName(bool isChapter, QString novelName, QString oldName, QString newName);
    void slotChangeFullName(QString novelName, QString chapterName, QString newFullName);

    void slotAddNextChapter(QString content);
    void slotAddNextChapterFaster(QString content);
    void slotLocateOutline(QString novel_name, QString chapter_content);
    void slotLocateChapter(QString novel_name, QString chapter_name);
    void slotOpenPrevChapter(QString novel_name, QString chapter_name);
    void slotOpenNextChapter(QString novel_name, QString chapter_name);
    void slotMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text);
    void slotMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text);

    void slotCloseEditor(int index);
    void slotCloseAllEditors();
    void slotTextChanged();
    void updateUI();
    void slotUpdateAllEditor();
    void slotEditorBottomSettingsChanged();
    void slotEditorWordCount(int x);
    void slotSavePosition();
    void slotResotrePosition();
    void slotDelayFocus();
    void slotDelayScroll();

    void slotShowEditorSearchBar(QString def, bool rep = false);
    void slotHideEditorSearchBar();
    void slotShowSearchPanel();
    void slotHidePanels();


private:
    QWidget* parent;
    QList<ChapterEditor*>editors;
    bool _flag_isNew; // 是否是本次运行第一次打开这个章节，用来显示打开动画
    int _flag_last;   // 上次（其实是本次）打开的图片
    int _flag_aim;    //
    QPixmap* _open_pixmap; // 打开的图片，切换完毕消失
    EditorSearchWidget* editor_search_bar;
    QWidget* titlebar;
    QPixmap** p_editor_blur_win_bg_pixmap;
};

#endif // MYTABWIDGET_H
