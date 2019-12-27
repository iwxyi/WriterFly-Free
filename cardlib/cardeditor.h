#ifndef CARDEDITOR_H
#define CARDEDITOR_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include "interactivebuttonbase.h"
#include "watercirclebutton.h"
#include "waterfloatbutton.h"
#include "globalvar.h"
#include "mydialog.h"
#include "subjectcolordialog.h"
#include "generaleditor.h"
#include "waterfallbuttongroup.h"
#include "cardlibtagsai.h"

/*
 * 名片卡片编辑界面
 */

class CardEditor : public MyDialog
{
    Q_OBJECT
public:
    CardEditor(QWidget *parent = nullptr);
    CardEditor(QString name, QWidget* parent = nullptr);
    CardEditor(CardBean* card, QWidget *parent = nullptr);
    CardEditor(CardBean* card, CardEditor* from, QWidget *parent = nullptr);

    CardBean* getCard();
    CardEditor* getFrom();
    void addToEditor(CardEditor* editor);
    void delToEditor(CardEditor* editor);

    void movePos(QPoint delta);
    bool movePosFinished();
    void movePosWithChilds(QPoint delta, CardEditor* editor);

protected:
	void resizeEvent(QResizeEvent * event);
    void focusInEvent(QFocusEvent* event);
	void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent*event);

private:
    void initView();
    void initMenu();
    void initData();
    void initEvent();
    void setEditColor(QLineEdit *edit, QColor color);
    void initTags();

    void setTitleTree();

    QPoint getRelatedPos();
    void setRelatedPos(QPoint pos);

signals:
	void signalCardCreated(CardBean * card);
    void signalCardModified(CardBean * card);
    void signalShowCardEditor(CardBean *card, CardEditor *from);
    void signalLocateFromEditor(CardEditor* from);
    void signalClosed();

public slots:
    void slotSelectColor();
    void slotCreateCard();
    void slotSave();
    void slotNewCardNameChanged(const QString & s);

    void slotLocateFromEditor(CardEditor* from);

    void refreshMenu();

    void actionSearchBook();
    void actionUnuseCard();
    void actionDisableCard();
    void actionDeleteCard();

    void actionSaveSizeSelf();
    void actionFillTags();
    void actionFillColor();
    void actionAutoScroll();
    void actionSaveScroll();
    void actionSaveSize();
    void actionFollowMove();
    void actionMultiTile();

    void actionHelp();

private:
    bool modify_model;
    bool rehighlight;
    CardBean *card;
    QLineEdit* name_edit;
    InteractiveButtonBase *color_btn;
    QLineEdit *pat_edit;
    QLineEdit *tags_edit;
    WaterFallButtonGroup* tags_group;
    InteractiveButtonBase *tags_btn;
    GeneralEditor *brief_edit;
    GeneralEditor *detail_edit;
    InteractiveButtonBase *create_btn;
    QMenu* menu;
    QScrollArea* scroll_area;
    QWidget *scroll_widget;

    CardEditor *from;
    QList<CardEditor*>toes;

    // 菜单
    QAction *card_fill_tags_action;
    QAction *card_fill_color_action;
    QAction *card_auto_scroll_action;
    QAction *card_save_scroll_action;
    QAction *card_save_size_action;
    QAction *card_follow_move_action;
    QAction *card_multi_tile_action;

};

#endif // CARDEDITOR_H
