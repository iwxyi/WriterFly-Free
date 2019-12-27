#ifndef GENERALEDITOR_H
#define GENERALEDITOR_H

#include <QObject>
#include "chaptereditor.h"

class GeneralEditor : public ChapterEditor
{
public:
    GeneralEditor(QWidget *parent);
    GeneralEditor(QWidget *parent, EditType type);
    GeneralEditor(QWidget* parent, QString path);
    GeneralEditor(QWidget* parent, EditType type, QString path);
    GeneralEditor(QWidget* parent, EditType type, QString path, bool sm, bool cm, bool nm);

    void initView();
    void updateUI();

    void setModels(bool sm, bool cm, bool nm);
    void setType(EditType type);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void saveFontSize() override;

private:
    bool single_mode;  // 单行模式
    bool compact_mode; // 紧凑模式
    bool normal_mode;  // 关闭AI操作（普通的编辑器）
};

#endif // GENERALEDITOR_H
