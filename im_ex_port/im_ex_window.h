#ifndef IM_EX_WINDOW
#define IM_EX_WINDOW

#include <QWidget>
#include <QObject>
#include <QTabWidget>
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"
#include "exportpage.h"
#include "importpage.h"
#include "importmzfypage.h"
#include "anitabwidget.h"

/**
 * 导入导出Tab控件
 */
class ImExWindow : public AniTabWidget
{
    Q_OBJECT
public:
    ImExWindow(QWidget* parent);
    void initLayout();
    void toShow(int kind, QString def_name);

protected:
    void paintEvent(QPaintEvent* event);

signals:
    void signalImportFinished(QString novel_name);
    void signalImportMzfyFinished(QStringList novel_name_list);

public slots:

public:
    ImportPage* import_page;
    ExportPage* export_page;
    ImportMzfyPage* import_mzfy_page;

private:
    QString current_novel_name;
};

#endif
