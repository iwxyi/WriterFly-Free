#ifndef EDITORSEARCHWIDGET_H
#define EDITORSEARCHWIDGET_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <chaptereditor.h>
#include "circlebutton.h"
#include "globalvar.h"
#include "watercirclebutton.h"

class EditorSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EditorSearchWidget(QWidget *parent = nullptr);

    void setAimEditor(ChapterEditor* editor);
    void setText(QString def="");
    void showReplace(bool show = true);
    void setReplaceKey(QString str);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void setCursorPosition(int position, int sel = 0);

private:
    void adjustFocus(bool aim_focus = false);

signals:
    void signalSearchPrev(QString);
    void signalSearchNext(QString);
    void signalClosed();

public slots:
    void updateUI();

    void slotSearchKeyChanged();
    void slotSearchPrev(bool select = true);
    void slotSearchNext(bool select = true);
    void slotSearchFirst(bool select = true);
    void slotSearchLast(bool select = true);
    void slotSearchThis(bool select = false);

    void slotReplacePrev();
    void slotReplaceNext();
    void slotReplaceAll();

private:
    ChapterEditor* aim_editor;
    QWidget* bg;
    QWidget* separator;

    int shadow;
    int search_height, replace_height;

    QLineEdit* key_editor;
    WaterCircleButton* prev_btn;
    WaterCircleButton* next_btn;
    WaterCircleButton* close_btn;

    QLineEdit* replace_key_editor;
    WaterCircleButton* replace_prev_btn;
    WaterCircleButton* replace_next_btn;
    WaterCircleButton* replace_all_btn;
};

#endif // EDITORSEARCHWIDGET_H
