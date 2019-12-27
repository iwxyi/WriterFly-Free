#ifndef CHAPTEREDITOR_H
#define CHAPTEREDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <novelai.h>
#include <QPropertyAnimation>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QInputMethodEvent>
#include <QFont>
#include <QMimeData>
#include <QTextBlockFormat>
#include <QTextBlock>
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QListView>
#include <QClipboard>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QToolTip>
#include "settings.h"
#include "defines.h"
#include "globalvar.h"
#include "fileutil.h"
#include "editorrecorderitem.h"
#include "editorcursor.h"
#include "generallistview.h"
#include "editorscrollbean.h"
#include "myhightlighter.h"
#include "noveltools.h"
#include "boardwidget.h"
#include "zoomgeometryanimationwidget.h"
#include "smoothscrollbean.h"
#include "blurwidget.h"
#include "inputdialog.h"
#include "cardlibhighlight.h"
#include "subjectcolordialog.h"
#include "motioncontentnumber.h"

/**
 * 小说章节编辑器类
 */
class ChapterEditor : public QTextEdit, public NovelAI
{
    Q_OBJECT
    Q_PROPERTY(int smooth_scroll READ getScrollSlider WRITE setScrollSlider RESET resetScrollSlider)
    Q_PROPERTY(int underline_progress READ getUnderlineProgress WRITE setUnderlineProgress RESET resetUnderlineProgress)
public:
    ChapterEditor(QWidget *parent);
    ChapterEditor(EditType type, QWidget *parent);
    ChapterEditor(QWidget *parent, QString novel_name, QString chapter_name, QString full_chapter_name);
    ChapterEditor(QWidget *parent, EditType type, QString novel_name, QString chapter_name, QString show_name = "");
    ~ChapterEditor();

public:
    // 用户最近使用的操作（结合 last_operator）
    enum UserOperator {
        OPERATOR_NULL,
        OPERATOR_KEY,
        OPERATOR_MOUSE,
        OPERATOR_CN,
        OPERATOR_WHEEL, // 这个用不到
        OPERATOR_NOT_COMPLETER = 11, // 专门用来不提示的
        OPERATOR_IMD_COMPLETER // 专门用来马上提示
    };

    // 撤销重做的操作情况（比如：一次撤销可撤销多次删除）
    enum UndoRedoCase {
        UNDOREDO_CASE_NONE,
        UNDOREDO_CASE_INPUTONE,
        UNDOREDO_CASE_BACKSPACEONE,
        UNDOREDO_CASE_DELETEONE
    };

    // 点击水波纹动画
    struct Water {
        Water(QPoint p, qint64 t) : point(p), progress(0), press_timestamp(t),
            release_timestamp(0), finish_timestamp(0), finished(false) {}
        QPoint point;
        int progress;
        qint64 press_timestamp;
        qint64 release_timestamp;
        qint64 finish_timestamp;
        bool finished;
    };

public:
	void setInitialText(QString text);
    void openChapter(QString novel_name, QString chapter_name, QString full_chapter_name); // 单纯的打开章节
    void readFromFile(QString filePath);
    void removeOpeningFile();
    bool isSaved();
    bool save();
    void reloadContent();
    EditingInfo &getInfo();

    void surroundWordSearch();
    void closeCompleteTimerOnce();

    void setSearchKey(QString key, bool regexp = false);
    void setReplaceKey(QString key);
    void smoothScrollTo(int pos, int sel = -1, int slider = -1);
    void smoothScrollTo(EditorScrollBean ecb);

    void setTitleBar(QWidget* widget = nullptr, QPixmap** p_bg = nullptr);
    void blurTitleBar();
    void blurTitleBarFaster();

    bool execCommand(QString command);
    bool execCommand(QString command, QStringList args);

protected:
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event); // 按键响应
    void keyReleaseEvent(QKeyEvent* event);
    void inputMethodEvent(QInputMethodEvent *event); // 输入中文字符
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent* e);
    void showEvent(QShowEvent* e);
    void resizeEvent(QResizeEvent* e);
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);
    void insertFromMimeData(const QMimeData *source); // 粘贴槽
    void contextMenuEvent(QContextMenuEvent* event);
    bool event(QEvent *event);

    void cursorRealChanged(int pos = -1);
    void cutRecorderPosition();

    void setTextInputColor();

    virtual void saveFontSize();

private:
    void initEditor();
    void initMenu();

    void delayCompleter(int ms);

    // 数据相关
    QString getPrecedingParaText();
    QString getPrecedingSentText();

    // 高亮相关
    void drawUnderLine();
    void highlightCurrentLine();
    void highlightAll();

    // 显示名片信息
    bool showCardlibToolTip(QPoint pos);
    bool showCardlibEditor();
    int getPositionByMouseHover(QPoint point);
    CardBean* getCardByTextCursor(int pos = -1);

    // 空白与光标固定
    void scrollToEditEnd();
    void scrollCursorFix();

    // 平滑滚动动画
    void setScrollSlider(int x);
    int getScrollSlider();
    void resetScrollSlider();
    void addSmoothScrollThread(int distance, int duration);

    // 下划线出现动画
    void setUnderlineProgress(int x);
    int getUnderlineProgress();
    void resetUnderlineProgress();

    // 水波纹动画
    void paintWaterRipple(QPainter &painter);

signals:
    void signalWordCount(int x);
    void signalUpdateAllEditor();

    // 菜单
    void signalShowSearchBar(QString, bool);
    void signalShowSearchPanel();
    void signalHidePanels();
    void signalAddNextChapter(QString text);
    void signalAddNextChapterFaster(QString text);
    void signalLocateOutline(QString novel_name, QString chapter_content);
    void signalLocateChapter(QString novel_name, QString chapter_name);
    void signalOpenPrevChapter(QString novel_name, QString chapter_name);
    void signalOpenNextChapter(QString novel_name, QString chapter_name);
    void signalMoveToPrevChapterEnd(QString novel_name, QString chapter_name, QString text);
    void signalMoveToNextChapterStart(QString novel_name, QString chapter_name, QString text);

    // 名片
    void signalShowCardEditor(CardBean *card);
    void signalAddNewCard();

public slots:
    // ==== edit槽 ====
    void beforeClose();         // 关闭前询问是否保存
    void textChange();          // 文本改变槽
    void cursorChange();        // 光标位置改变槽
    void copyAva();             // 允许复制槽
    void cursorChangedByUser(); // 用户手动移动光标槽
    void scrollChange();        // 进度条滚动槽
    void resetScrollBarRange(); // 重置进度条高度
    void beforeInputted(QString in); // 输入前调用，例如监视选中文字被覆盖

    // ==== 撤销重做 ====
    void initUndoRedo();
    void recorderOperator();
    void undoOperator();
    void redoOperator();
    void readRecorder(int x);
    void updateRecorderPosition(int x);
    void updateRecorderScroll();
    void updateRecorderSelection();

    // ==== 更新信息 ====
    void updateUI();
    void updateWordCount();
    void slotEditorBottomSettingsChanged();
    void updateEditorMarked();
    void updateEditor();

    // ==== 自动提示 ====
    void slotCompleterShowed();
    void slotOnCompleterActived(const QString& completion);

    // ==== 名片 ====
    void slotShowCardUnderline(bool show);

    // ==== 控件 ====
    void slotSubWidgetHidden(QPixmap pixmap, QRect rect);
    void slotShowMotion();

    // ==== 动画槽 ====
    void slotSmoothScrollDistance(SmoothScrollBean *bean, int dis);

    // ==== 搜索替换 ====
    int actionSearchPrev(bool select = true);
    int actionSearchNext(bool select = true);
    int actionSearchFirst(bool select = true);
    int actionSearchLast(bool select = true);
    int actionReplacePrev(bool select = true);
    int actionReplaceNext(bool select = true);
    int actionReplaceAll(bool select = true);

    // ==== 各种快捷键操作 ====
    void actionScrollContentByLines(int line = 1);
    void actionScrollContentByHeight(int part = 3);
    void actionMoveTextCursorToPrevPara(bool select = false);
    void actionMoveTextCursorToNextPara(bool select = false);
    void actionMoveTextCursorToLeft(bool word = false, bool sent = false, bool select = false);
    void actionMoveTextCursorToRight(bool word = false, bool sent = false, bool select = false);

    // ==== 菜单 ====
    void refreshMenu(); // 根据选中的内容和程序配置刷新菜单（显示时）
    void actionComprehensive();
    void actionUndo();
    void actionRedo();
    void actionCut();
    void actionCopy();
    void actionCopyWithTitle();
    void actionCopyWithChapterMark();
    void actionPaste();
    void actionPasteWithTypeset();
    void actionPasteWithoutTypeset();
    void actionBoards();
    void actionSearch();
    void actionReplace();
    void actionSelectAll();
    void actionWordCount();
    void actionTypeset();

    // 章节
    void actionLocateOutline();
    void actionLocateChapter();
    void actionOpenPrevChapter();
    void actionOpenNextChapter();
    void actionAddNextChapterFaster();
    void actionMoveToPrevChapterEnd();
    void actionMoveToNextChapterStart();
    void actionSplitToNextChapter();
    void actionSplitToMultiChapters();

    // 名片
    void actionAddCard();
    void actionSeeCard();
    void actionChangeCardColor();
    void actionCardAllAppear();
    void actionJumpCardAppear();
    void actionJumpCardDisappear();

    // 敏感词
    void actionSensitiveHighlight();
    void actionSensitiveEliminate();
    void actionSensitiveReplace();
    void actionSensitiveReplacePinyin();
    void actionSensitiveReplaceFirst();
    void actionSensitiveEditList();
    void actionSensitiveEditReplace();

    // 转换
    void actionSimplifiedToTraditional();
    void actionTraditionalToSimplified();
    void actionErrorWordHighlight();
    void actionWordCorrect();
    void actionSimpleBeauty();
    void actionSimilarSentence();
    void actionPredictSentence();

    // ==== 其他操作 ====
    void msgLostParam(QString cmd, QString s);

protected:
    // 撤销重做
    QList<EditorRecorderItem> recorders;
    int recorders_pos; //  索引从1开始
    int last_operator, completer_case;
    bool changed_by_redo_undo;
    int last_pressed_key;

    // 按键交互
    bool ctrling, alting, shifting;
    QTimer* mouse_press_timer, *completer_timer;
    bool mouse_press_timer_timeout, completer_timer_timeout;
    bool focusing;
    QTimer* document_height_timer;

    // 自动补全
    QCompleter* completer;
    GeneralListView* completer_listview;
    PredictCompleter* predict_completer;

    // 光标行固定
    int editor_height, document_height;
    int viewport_margin_bottom;
    int pre_scrollbar_max_range;     // 上一次的最大值，判断有没有改变高度

    // 各种标识
    bool _flag_dont_save; // 本次修改是否不保存：打开新章导致文本改变
    bool _flag_is_opening;
    bool _flag_is_not_range_changed; // rangeChanged 事件中，避免死循环
    bool _flag_should_update;
    bool _flag_user_change_cursor;
    bool _flag_highlighting; // 正在高亮中，不进行文本改变判断
    qint64 edit_timestamp;

    // 动画
    EditorCursor* editor_cursor;
    QPropertyAnimation* smooth_ani;
    int _smooth_scroll_slider, _smooth_scroll_speed;
    QList<SmoothScrollBean*> smooth_scrolls;
    int underline_progress;

    bool blur_effect;
    BlurWidget* blur_title;
    QPixmap* blur_pixmap;
    QPixmap** p_blur_win_bg;

    QList<Water>waters;
    QTimer* update_timer;

    // 搜索
    QString search_key;
    QString replace_key;
    bool search_regex;

    // 名片
    QTimer* ctrl_underline_timer;
    bool is_ctrl_underline;

    // 菜单
    QMenu* pop_menu;
    QAction* comprehansive_action;
    QAction* undo_action;
    QAction* redo_action;
    QAction* cut_action;
    QAction* copy_action;
    QAction* paste_action;
    QAction* copy_all_action;
    QAction* boards_action;
    QAction* search_action;
    QAction* replace_action;
    QAction* select_all_action;
    QAction* word_count_action;
    QAction* typeset_action;

    /*QMenu* boards_menu;
    QAction* clip_board_action;
    QAction* recycle_board_action;
    QAction* collection_board_action;*/

    QMenu* chapter_menu;
    QAction* locate_outline_action;
    QAction* locate_chapter_action;
    QAction* open_prev_chapter_action;
    QAction* open_next_chapter_action;
    QAction* add_next_chapter_faster_action;
    QAction* move_to_prev_chapter_end_action;
    QAction* move_to_next_chapter_start_action;
    QAction* split_to_next_chapter_action;
    QAction* split_to_multi_chapters_action;

    // QMenu* cardlib_menu;
    QAction* add_card_action;
    QAction* see_card_action;
    QMenu* cardlib_menu;
    QAction* card_color_action;
    QAction* card_all_appear_action;
    QAction* jump_appear_action;
    QAction* jump_disappear_action;

    QMenu* conversion_menu;
    QAction* error_word_highlight_action;
    QAction* word_correct_action;
    QAction* simple_beauty_action;
    QAction* similar_sentence_action;
    QAction* predict_sentence_action;
    QAction* simplified_to_traditional_action;
    QAction* traditional_to_simplified_action;

    QMenu* sensitive_menu;
    QAction* sensitive_action;
    QAction* sensitive_eliminate_action;
    QAction* sensitive_replace_action;
    QAction* sensitive_replace_pinyin_action;
    QAction* sensitive_replace_first_action;
    QAction* sensitive_edit_list_action;
    QAction* sensitive_edit_replace_action;

    QMenu* function_menu;
};

#endif // CHAPTEREDITOR_H
