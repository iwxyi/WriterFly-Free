#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QListView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include "globalvar.h"
#include "stringutil.h"
#include "searchresultbean.h"
#include "searchlistwidget.h"
#include "searchmodel.h"
#include "searchdelegate.h"
#include "chaptereditor.h"
#include "cardlibhighlight.h"

#define SHADOW_BLUR_RADIUS_MAX 10

class SearchPanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int shadow_radius WRITE setShadowRadius)
public:
    explicit SearchPanel(QWidget *parent = nullptr);

    void setEditors(ChapterEditor *cur_edit, QList<ChapterEditor *> edits);
    void setText(QString str);

protected:
    void search(QString str);
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initView();
    void toHide();

    bool fuzzySearchByChar(QString &text, QString &key) const;

    void adjustCommandOrder(QString command);

    void addOneResult(CardBean* card);
    QList<SearchResultBean> getResultsFromStringList(QStringList sl, ResultType type);
    QList<SearchResultBean> getResultsFromStringList(QStringList sl, QString key, ResultType type);
    QList<SearchResultBean> getResultsFromEditor(ChapterEditor *te, QString key, int index = -1);
    QString getKeysInText(QString key, int pos, QString &text);

    void setShadowRadius(int x);

signals:
    void signalInsertText(QString text);
    void signalInsertTextWithSuffix(QString suffix, QString text);
    void signalLocateEdit(int index, int pos, int len);
    void signalEmitCommand(QString command);
    void signalOpenCard(CardBean *card);
    void signalHide();

public slots:
    void updateUI();
    void adjustHeight(int min, int max);
    void slotResultClicked(QModelIndex index);
    void slotResultPreview(int index);
    void slotFocusChanged(QWidget *old, QWidget *now);

private:
    // 控件组
    QLineEdit *key_edit;
    SearchListWidget *result_view;
    int margin;
    bool _flag_height_changed;
    QGraphicsDropShadowEffect *shadow_effect;
    int shadow_radius;

    // 搜索对象
    ChapterEditor *current_editor;
    QList<ChapterEditor *> editors;
    EditorScrollBean current_editor_scroll; // 显示搜索面板时的滚动进度
    bool preview_scrolled;                  // 使用一个变量确定是否已经滚动，至少保证了没有滚动的话原来的位置不会变化，增强了稳定性

    // 搜索结果列表
    QList<SearchResultBean> results;
    SearchModel *model;
    SearchDelegate *delegate;
    int vertical_scrollbar_width;

    // 搜索数据
    QStringList commands;
    QStringList hidden_commands;
    QStringList command_orders;
    QStringList search_type_name;
    QList<ResultType> search_type_type;
};

#endif // SEARCHPANEL_H
