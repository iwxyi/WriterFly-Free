#ifndef MYHIGHTLIGHTER_H
#define MYHIGHTLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "globalvar.h"
#include "editorinputmanager.h"

class MyHightlighter : public QSyntaxHighlighter
{
public:
    MyHightlighter(QTextDocument *parent);

    void setInputManager(EditorInputManager* im);
    void setSearchKey(QString text);
    void setCardUnderlineVisible(bool show);

protected:
    void highlightBlock(const QString &text);

private:
    QString search_key;
    EditorInputManager* input_manager;
    bool card_underline_visible;
};

#endif // MYHIGHTLIGHTER_H
