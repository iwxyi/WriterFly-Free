#ifndef OUTLINEGROUP_H
#define OUTLINEGROUP_H

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
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include "defines.h"
#include "noveldirlistview.h"
#include "chaptereditor.h"
#include "anifbutton.h"
#include "splitterwidget.h"
#include "settings.h"
#include "circlebutton.h"
#include "circlefloatbutton.h"
#include "generalbuttoninterface.h"
#include "pointmenubutton.h"
#include "globalvar.h"
#include "noveltools.h"
#include "fileutil.h"
#include "generaleditor.h"
#include "generalcombobox.h"
#include "watercirclebutton.h"
#include "editorsearchwidget.h"
#include "treeoutlinewindow.h"
#include "locatemutiple.h"

/**
 * 大纲列表小窗口
 * Settings存储格式： recent/outline=最近大纲名臣
 *                  progress/大纲=10  // slidePosition位置
 */
class OutlineGroup : public QWidget
{
    Q_OBJECT
public:
    explicit OutlineGroup(QWidget *parent = nullptr);

    bool execCommand(QString command);
    bool execCommand(QString command, QStringList args);

protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent* event);

private:
    void initFrame();
    void initView();
    void initStyle();
    void initData();

    void loadOutlineList(QString novel_name, QString outline_name = "");
    void loadOutlineText(QString novel_name, QString outline_name);
    void loadOutlineText(QString outline_name);

    QString inputName(QString def = "");
    void setActionsEnabled(bool e);
    void createDefaultStories();

signals:
    void signalShowCardEditor(CardBean *card);
    void signalAddNewCard();

public slots:
    void updateUI();
    void slotNovelNameChanged(QString old_name, QString new_name);
    void slotNovelDeleted();
    void slotLoadOutlineList(QString novel_name);
    void slotLoadOutlineText(QString outline_name);
    bool slotSwitchOutline(int index);
    void slotEditTextChanged();
    void slotEditSlideChanged();

    void slotLocateOutline(QString novel_name, QString chapter_content);

    void slotShowEditorSearchBar(QString def, bool rep);
    void slotHideEditorSearchBar();
    void slotHidePanels();

    void actionEdit();
    void actionRename();
    void actionDelete();
    void actionTimeline();
    void actionTreeModel();
    void actionRecycle();
    void actionHelp();

private:
    GeneralComboBox *outline_switch_combo;
    GeneralEditor *edit;
    InteractiveButtonBase* menu_button;
    QWidget *divider_line;

    QMenu *menu;
    QAction *edit_action;
    QAction *rename_action;
    QAction *delete_action;
    QAction *timeline_action;
    QAction *tree_model_action;
    QAction *recycle_action;
    QAction *help_action;

    Settings* os;
    QString novel_name;
    QString outline_name;
    QString outline_path; // 大纲文件路径

    EditorSearchWidget* editor_search_bar;

    bool used;
};

#endif // OUTLINEGROUP_H
